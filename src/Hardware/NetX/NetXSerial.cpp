/*
 * NetXSerial.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */


#include "NetXSerial.h"
#include <algorithm>

#if NETX_ENABLE

void NetXSerial::Init() noexcept
{
	// TODO: Check if it is actually connected.
	isConnected = true ;
}

int NetXSerial::available() noexcept
{
	return (isConnected);
}

int NetXSerial::read() noexcept
{
	uint8_t aux;
	int ret = -1;
	if( protocol.serialChannel.rxQueue.Size() )
	{
		if(protocol.serialChannel.rxQueue.Poll(&aux))
		{
			rxAccumBytes++;
			ret = (int)aux;
		}
	}
	return ret ;
}

size_t NetXSerial::readBytes(char *buffer, size_t length) noexcept
{
	size_t ret = 0;
	const size_t bytesAvilable = protocol.serialChannel.rxQueue.Size();

	if(bytesAvilable)
	{
		const size_t toRead = min<size_t>(bytesAvilable,length);
		protocol.serialChannel.rxQueue.Poll((uint8_t *)buffer, toRead);
		rxAccumBytes += toRead;
		ret = toRead;
	}
	return ret;
}

void NetXSerial::flush(void) noexcept
{
	protocol.serialChannel.rxQueue.Clear();
	protocol.serialChannel.txQueue.Clear();
}

size_t NetXSerial::write(uint8_t c) noexcept
{
	return write(&c, 1);
}

// Non-blocking write to USB. Returns number of bytes written. If we are not connected, pretend that all bytes have been written.
size_t NetXSerial::write(const uint8_t *buffer, size_t size) noexcept
{
	if (isConnected && size != 0)
	{
		// Check if there is enough space or if we need to write just some bytes.
		size = min<size_t>(protocol.serialChannel.txQueue.FreeSpace(),size);
		// Write in the buffer.
		if(protocol.serialChannel.txQueue.Add((uint8_t *)buffer, size))
		{
			txAccumBytes += size;
		}
		else
		{
			size = 0;	// We could not write.
		}
	}
	return size;
}

size_t NetXSerial::canWrite() noexcept
{
	return (isConnected) ? protocol.serialChannel.txQueue.FreeSpace() : 0;
}

bool NetXSerial::IsConnected() const noexcept
{
	return isConnected;
}

/*
 * Return the total amount of bytes that were received from the master
 * and clears the accumulator.
 */
uint32_t NetXSerial::GetTotalBytesReceivedAndClear() noexcept
{
	uint32_t ret = rxAccumBytes;
	rxAccumBytes = 0;
	return ret;
}

/*
 * Return the total amount of bytes that were transmitted to the master
 * and clears the accumulator.
 */
uint32_t NetXSerial::GetTotalBytesTransmittedAndClear() noexcept
{
	uint32_t ret = txAccumBytes;
	txAccumBytes = 0;
	return ret;
}

#endif
// End
