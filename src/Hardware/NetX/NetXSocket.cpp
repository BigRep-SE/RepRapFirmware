/*
 * NetXSocket.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#include "Platform/RepRap.h"
#include "NetXSocket.h"

#if NETX_ENABLE

const unsigned int MaxBuffersPerSocket = 8;

NetXSocket::NetXSocket() noexcept :
	Socket(nullptr)
{
	protocol = HttpProtocol;
}

void NetXSocket::Close() noexcept
{
	if(state == SocketState::connected || state == SocketState::clientDisconnecting)
	{
		state = SocketState::inactive;
		return ;
	}
#ifdef NETX_DEBUG
	debugPrintf("Closing with error\r\n");
#endif
	Terminate();	// something is not right, so terminate the socket for safety
}

// Terminate a connection immediately
// We can call this after any sort of error on a socket as long as it is in use.
void NetXSocket::Terminate() noexcept
{
	if (state != SocketState::inactive)
	{
		//const int32_t reply = GetInterface()->SendCommand(NetworkCommand::connAbort, socketNum, 0, 0, nullptr, 0, nullptr, 0);
		//state = (reply != 0) ? SocketState::broken : SocketState::inactive;
		protocolECAT.socketChannel.txQueue.Clear();	// Clear the Tx buffer.
		char errorMsg[] = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
		Send((uint8_t *)errorMsg,(unsigned int)strlen(errorMsg));
#ifdef NETX_DEBUG
		debugPrintf("Terminate inactive\r\n");
#endif
		state = SocketState::inactive;
	}
#ifdef NETX_DEBUG
	debugPrintf("Terminate\r\n");
#endif
	DiscardReceivedData();
}

// Return true if there is or may soon be more data to read
bool NetXSocket::CanRead() const noexcept
{
	int counterLoop = 25;
	bool ret = (state == SocketState::connected) || (state == SocketState::clientDisconnecting && receivedData != nullptr && receivedData->TotalRemaining() != 0);
	// Let's wait until it is ready.
	while(!ret && counterLoop != 0)
	{
		delay(10);
		ret = (state == SocketState::connected) || (state == SocketState::clientDisconnecting && receivedData != nullptr && receivedData->TotalRemaining() != 0);
		--counterLoop;
	}
	return ret;
}

// Return true if we can send data to this socket
bool NetXSocket::CanSend() const noexcept
{
	return state == SocketState::connected;
}

// Read 1 character from the receive buffers, returning true if successful
bool NetXSocket::ReadChar(char& c) noexcept
{
	if (receivedData != nullptr)
	{
		const bool ret = receivedData->ReadChar(c);
		if (receivedData->IsEmpty())
		{
			receivedData = receivedData->Release();
		}
		return ret;
	}
	c = 0;
	return false;
}

// Return a pointer to data in a buffer and a length available, and mark the data as taken
bool NetXSocket::ReadBuffer(const uint8_t *&buffer, size_t &len) noexcept
{
	if (receivedData != nullptr)
	{
		len = receivedData->Remaining();
		buffer = receivedData->UnreadData();
		return true;
	}

	return false;
}

// Flag some data as taken from the receive buffers. We never take data from more than one buffer at a time.
void NetXSocket::Taken(size_t len) noexcept
{
	if (receivedData != nullptr)
	{
		receivedData->Taken(len);
		if (receivedData->IsEmpty())
		{
			receivedData = receivedData->Release();		// discard empty buffer at head of chain
		}
	}
}

// Poll a socket to see if it needs to be serviced
void NetXSocket::Poll() noexcept
{
	if (state != SocketState::connected)
	{
		// It's a new connection
		localPort = 1;
		remotePort = 1;
		remoteIPAddress.SetV4LittleEndian(123456789);
		DiscardReceivedData();
		if (state != SocketState::waitingForResponder)
		{
			whenConnected = millis();
			state = SocketState::waitingForResponder;
		}
		// We will try to get a Responder
		if (reprap.GetNetwork().FindResponder(this, protocol))
		{
			state = SocketState::connected;
		}
		else if (millis() - whenConnected >= FindResponderTimeout)
		{
#ifdef NETX_DEBUG
			debugPrintf("No responder, new conn %u terminated\r\n", socketNum);
#endif
			Terminate(); // Timeout
		}
	}
	else
	{
		ReceiveData();
	}

	needsPolling = false;
}

// Try to receive more incoming data from the socket.
void NetXSocket::ReceiveData() noexcept
{
	const size_t availableInBuffer = protocolECAT.socketChannel.rxQueue.Size();
	if (availableInBuffer > 0)
	{
		// timeoutLoop = millis();
		// First see if we already have a buffer with enough room
		NetworkBuffer *const lastBuffer = NetworkBuffer::FindLast(receivedData);

		if (lastBuffer != nullptr && (availableInBuffer <= lastBuffer->SpaceLeft() || (lastBuffer->SpaceLeft() != 0 && NetworkBuffer::Count(receivedData) >= MaxBuffersPerSocket)))
		{
#ifdef NETX_DEBUG
			debugPrintf("Using Existing buffer\r\n");
#endif
			// Read data into the existing buffer
			size_t maxToRead = min<size_t>(lastBuffer->SpaceLeft(), availableInBuffer);
			// Here read from NetX
			if( protocolECAT.socketChannel.rxQueue.Poll( lastBuffer->UnwrittenData(), maxToRead) )
			{
				lastBuffer->dataLength += maxToRead;
				rxAccumBytes += maxToRead;
			}
		}
		else if (NetworkBuffer::Count(receivedData) < MaxBuffersPerSocket)
		{
			// New buffer receivedData needed.
			NetworkBuffer * buf = NetworkBuffer::Allocate();
			// Let's make sure it is a new buffer
			if(prevBuffer == buf && prevBuffer != nullptr)
			{
				buf = NetworkBuffer::Allocate();
				prevBuffer->Release();
			}
			prevBuffer = buf;
			if (buf != nullptr)
			{
				const size_t maxToRead = min<size_t>(NetworkBuffer::bufferSize, availableInBuffer);
				const uint8_t auxBool = protocolECAT.socketChannel.rxQueue.Poll( buf->Data(), maxToRead );
				if (auxBool > 0)
				{
					buf->dataLength = maxToRead;
					rxAccumBytes += maxToRead;
					NetworkBuffer::AppendToList(&receivedData, buf);
				}
				else
				{
					buf->Release();
				}
			}
		}
#ifdef NETX_DEBUG
		else
		{
			debugPrintf("No Buffers Available\r\n");
		}
#endif
	}
}

// Discard any received data for this transaction
void NetXSocket::DiscardReceivedData() noexcept
{
	while (receivedData != nullptr)
	{
		receivedData = receivedData->Release();
	}
}

// Send the data, returning the length buffered
size_t NetXSocket::Send(const uint8_t *data, size_t length) noexcept
{
	//if (state == SocketState::connected && NetXSocketBufferTx_freeSpace() != 0)
	const size_t freeSpace = protocolECAT.socketChannel.txQueue.FreeSpace();
	if ( freeSpace > 0)
	{
		if( freeSpace < MinFreeSpaceToAddDelay )
		{
			delay( MinFreeSpaceDelayMs );
		}

		const size_t lengthToSend = min<size_t>( length, freeSpace );
		if ( protocolECAT.socketChannel.txQueue.Add( (unsigned char*)data, lengthToSend ) > 0 )
		{
			txAccumBytes += lengthToSend;
			return lengthToSend;
		}
#ifdef NETX_DEBUG
		debugPrintf("Send failed, terminating\r\n");
#endif
		state = SocketState::broken;							// something is not right, terminate the socket soon
	}
	return 0;
}

// Tell the interface to send the outstanding data
void NetXSocket::Send() noexcept
{
	/*
	// TODO: TIMEOUT!!!!
	while( protocolECAT.socketChannel.txQueue.Size() )
	{
		delay(1);
	}
	*/
}

// Return true if we need to poll this socket
bool NetXSocket::NeedsPolling() const noexcept
{
	return state != SocketState::inactive || needsPolling || protocolECAT.socketChannel.rxQueue.Size();
}

/*
 * Return the total amount of bytes that were received from the master
 * and clears the accumulator.
 */
uint32_t NetXSocket::GetTotalBytesReceivedAndClear() noexcept
{
	uint32_t ret = rxAccumBytes;
	rxAccumBytes = 0;
	return ret;
}

/*
 * Return the total amount of bytes that were transmitted to the master
 * and clears the accumulator.
 */
uint32_t NetXSocket::GetTotalBytesTransmittedAndClear() noexcept
{
	uint32_t ret = txAccumBytes;
	txAccumBytes = 0;
	return ret;
}

#endif
