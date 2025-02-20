/*
 * ProtocolOverEtherCAT.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */


#include <Hardware/NetX/ProtocolOverEtherCAT.h>

#if NETX_ENABLE

#include <Movement/StepTimer.h>


constexpr size_t NETX_BUFFER_SERIAL_RX_LENGTH = 1500;
constexpr size_t NETX_BUFFER_SERIAL_TX_LENGTH = 3500;
constexpr size_t NETX_BUFFER_SOCKET_RX_LENGTH = 5000;
constexpr size_t NETX_BUFFER_SOCKET_TX_LENGTH = 2000;

// Buffers
uint8_t	 serialRxBuffer[NETX_BUFFER_SERIAL_RX_LENGTH];		///< Rx buffer
// TODO: Once we move the NetX to we need to reduce this and
//		 when calling NetXSerial::GetInstance().write() there should
//		 be a delay if there is no more space.
//       Or asign the space dinamically.
uint8_t	 serialTxBuffer[NETX_BUFFER_SERIAL_TX_LENGTH];		///< Tx buffer

uint8_t	 socketRxBuffer[NETX_BUFFER_SOCKET_RX_LENGTH];		///< Rx buffer
uint8_t	 socketTxBuffer[NETX_BUFFER_SOCKET_TX_LENGTH];		///< Tx buffer


void DebugBuffer(const char *data, const size_t dataLength)
{
#ifdef NETX_DEBUG
	{
		for(uint32_t i = 0 ; i < (uint32_t) dataLength; ++i)
		{
			debugPrintf("%c", data[i]);
		}
	}
#endif
}


ProtocolOverEtherCAT::ProtocolOverEtherCAT() noexcept :
		serialChannel( NetXBufferData(serialRxBuffer, NETX_BUFFER_SERIAL_RX_LENGTH), NetXBufferData(serialTxBuffer, NETX_BUFFER_SERIAL_TX_LENGTH)),
		socketChannel( NetXBufferData(socketRxBuffer, NETX_BUFFER_SOCKET_RX_LENGTH), NetXBufferData(socketTxBuffer, NETX_BUFFER_SOCKET_RX_LENGTH))
{}

// Return true if data was processed
void ProtocolOverEtherCAT::ProcessMOSIData(EtherCATApplicationMOSIData &dataMOSI, EtherCATApplicationMISOData & dataMISO) noexcept
{
	const size_t HeardersSize = sizeof(EtherCATApplicationMOSIData::SerialHeader) + sizeof(EtherCATApplicationMOSIData::SocketHeader);
	const size_t LengthMOSISharedBuffer = (size_t)(dataMOSI.serialHeader.length + dataMOSI.socketHeader.length) ;
	const size_t BytesInMOSICRC = HeardersSize + LengthMOSISharedBuffer;
	const size_t OffsetMOSICRC = sizeof(EtherCATApplicationMOSIData) - ( HeardersSize + sizeof(dataMOSI.sharedBuffer) );

	if( CalculateCRC(dataMOSI, OffsetMOSICRC, BytesInMOSICRC) != dataMOSI.crc16bit )
	{
		// The data is fine.
		// There is no new input data available
		return;
	}
	uint8_t errorSerialBusy = 0;
	uint8_t errorSocketBusy = 0;

	// Process the Serial Data
	if(dataMOSI.serialHeader.idxMOSI == NETX_UART_RESET_INDEX)
	{
		lastIdxSerialMISO = NETX_UART_RESET_INDEX;
		lastIdxSerialMOSI = NETX_UART_RESET_INDEX;
		dataMISO.serialHeader.idxTx = NETX_UART_RESET_INDEX;
		dataMISO.serialHeader.idxRx = NETX_UART_RESET_INDEX;

		serialChannel.rxQueue.Clear();
		serialChannel.txQueue.Clear();

		dataMISO.serialHeader.idxRx = dataMOSI.serialHeader.idxMOSI;

	}
	else if(serialChannel.rxQueue.FreeSpace() >= EtherCATSharedBufferLenght)
	{

		if(dataMOSI.serialHeader.idxMOSI != lastIdxSerialMOSI && dataMOSI.serialHeader.length > 0)
		{
			// Copy the values received.
			// As the Serial bytes are in the first part of the shared buffer:
			serialChannel.rxQueue.Add(
					dataMOSI.sharedBuffer,
					dataMOSI.serialHeader.length);
			/*
			if (reprap.Debug(moduleNetX))
			{
				debugPrintf("SERIAL >> ");
				DebugBuffer((char*)dataMOSI.sharedBuffer, dataMOSI.serialHeader.length);
			}
			*/
		}
		lastIdxSerialMOSI = dataMOSI.serialHeader.idxMOSI;
		dataMISO.serialHeader.idxRx = dataMOSI.serialHeader.idxMOSI;
	}
	else
	{
		// We block for receiving more data.
		errorSerialBusy = 1;
	}


	// Process the Socket Data
	if(dataMOSI.socketHeader.idxMOSI == NETX_UART_RESET_INDEX)
	{
		lastIdxSocketMISO = NETX_UART_RESET_INDEX;
		lastIdxSocketMOSI = NETX_UART_RESET_INDEX;
		dataMISO.socketHeader.idxTx = NETX_UART_RESET_INDEX;
		dataMISO.socketHeader.idxRx = NETX_UART_RESET_INDEX;

		socketChannel.rxQueue.Clear();
		socketChannel.txQueue.Clear();

		dataMISO.socketHeader.idxRx = dataMOSI.socketHeader.idxMOSI;
	}
	else  if(socketChannel.rxQueue.FreeSpace() >= EtherCATSharedBufferLenght)
	{
		if(dataMOSI.socketHeader.idxMOSI != lastIdxSocketMOSI && dataMOSI.socketHeader.length > 0)
		{
			// Copy the values received.
			socketChannel.rxQueue.Add(
					&dataMOSI.sharedBuffer[dataMOSI.serialHeader.length],
					dataMOSI.socketHeader.length);
			/*
			if (reprap.Debug(moduleNetX))
			{
				debugPrintf("SOCKET >> ");
				DebugBuffer((char*)&dataMOSI.sharedBuffer[dataMOSI.serialHeader.length], dataMOSI.socketHeader.length);
				debugPrintf("\n");
			}
			*/
		}
		lastIdxSocketMOSI = dataMOSI.socketHeader.idxMOSI;
		dataMISO.socketHeader.idxRx = dataMOSI.socketHeader.idxMOSI;
	}
	else
	{
		// We block for receiving more data.
		errorSocketBusy = 1;
	}

	// We have new data available.
	EtherCATLength_t serialBytesToSend = 0;
	EtherCATLength_t socketBytesToSend = 0;
	// Let's calculate how the shared buffer will be divided.
	EtherCATLength_t serialChannelTxLength = errorSerialBusy ? 0 : (EtherCATLength_t)serialChannel.txQueue.Size() ;
	EtherCATLength_t socketChannelTxLength = errorSocketBusy ? 0 : (EtherCATLength_t)socketChannel.txQueue.Size();

	if((serialChannelTxLength + socketChannelTxLength) > EtherCATSharedBufferLenght)
	{
		// We can't send all the information. We have to devide it.
		// TODO: Set priorities
		if(serialChannelTxLength <= EtherCATSingleBufferLenght)
		{
			// We can send all the serial data now.
			serialBytesToSend = serialChannelTxLength;
			// The rest of the buffer is used for the socket.
			socketBytesToSend = EtherCATSharedBufferLenght-serialBytesToSend;
		}
		else if(socketChannelTxLength <= EtherCATSingleBufferLenght)
		{
			// We can send all the socket data now.
			socketBytesToSend = socketChannelTxLength;
			// The rest of the buffer is used for the serial.
			serialBytesToSend = EtherCATSharedBufferLenght-socketBytesToSend;
		}
		else
		{
			// Both are too big. Equal parts.
			serialBytesToSend = EtherCATSingleBufferLenght;
			socketBytesToSend = EtherCATSingleBufferLenght;
		}
	}
	else
	{
		// we can send all.
		serialBytesToSend = serialChannelTxLength;
		socketBytesToSend = socketChannelTxLength;
	}

	if(dataMOSI.serialHeader.idxMISO == lastIdxSerialMISO && dataMOSI.socketHeader.idxMISO == lastIdxSocketMISO)
	{
		if(serialBytesToSend)
		{
			// It got it! Lets send the next one!
			lastIdxSerialMISO++; // Update the index
			if(lastIdxSerialMISO == NETX_UART_RESET_INDEX)
			{
				lastIdxSerialMISO++;
			}
			dataMISO.serialHeader.idxTx = lastIdxSerialMISO;	// update the MISO
			dataMISO.serialHeader.length = serialBytesToSend; // Update the length
			// Update the shared buffer.
			serialChannel.txQueue.Poll(
				dataMISO.sharedBuffer,
				serialBytesToSend);
			/*
			if (reprap.Debug(moduleNetX))
			{
				debugPrintf("SERIAL << ");
				DebugBuffer((char*)dataMISO.sharedBuffer, serialBytesToSend);
				debugPrintf("\n");
			}
			*/
		}
		else if(!errorSerialBusy)
		{
			// Lets send an empty packet.
			if(dataMISO.serialHeader.length != 0)
			{
				// It got it! Lets send the next one!
				lastIdxSerialMISO++; // Update the index
				if(lastIdxSerialMISO == NETX_UART_RESET_INDEX)
				{
					lastIdxSerialMISO++;
				}
				dataMISO.serialHeader.idxTx = lastIdxSerialMISO;	// update the MISO
				dataMISO.serialHeader.length = serialBytesToSend; // Update the length
			}
		}
		if(socketBytesToSend)
		{
			lastIdxSocketMISO++; // Update the index
			if(lastIdxSocketMISO == NETX_UART_RESET_INDEX)
			{
				lastIdxSocketMISO++;
			}
			dataMISO.socketHeader.idxTx = lastIdxSocketMISO; 	// update the MISO
			dataMISO.socketHeader.length = socketBytesToSend; // Update the length
			socketChannel.txQueue.Poll(
				&dataMISO.sharedBuffer[dataMISO.serialHeader.length], // The serial data length is the offset.
				socketBytesToSend);
			/*
			if (reprap.Debug(moduleNetX))
			{
				debugPrintf("SOCKET << ");
				DebugBuffer((char*)&dataMISO.sharedBuffer[dataMISO.serialHeader.length], socketBytesToSend);
				debugPrintf("\n");
			}
			*/
		}
		else if(!errorSocketBusy)
		{
			// Lets send an empty packet.
			if(dataMISO.socketHeader.length != 0)
			{
				// It got it! Lets send the next one!
				lastIdxSocketMISO++; // Update the index
				if(lastIdxSocketMISO == NETX_UART_RESET_INDEX)
				{
					lastIdxSocketMISO++;
				}
				dataMISO.socketHeader.idxTx = lastIdxSocketMISO;	// update the MISO
				dataMISO.socketHeader.length = socketBytesToSend; // Update the length
			}
		}
	}
	// Finally lets update the CRC.
}

// Return true if there is new data to send.
void ProtocolOverEtherCAT::PrepareMISOData(EtherCATApplicationMOSIData &dataMOSI, EtherCATApplicationMISOData & dataMISO) noexcept
{
	const size_t HeardersSize = sizeof(EtherCATApplicationMISOData::SerialHeader) + sizeof(EtherCATApplicationMISOData::SocketHeader);
	const size_t LengthMISOSharedBuffer = (size_t)(dataMISO.serialHeader.length + dataMISO.socketHeader.length) ;
	const size_t BytesInMISOCRC = HeardersSize + LengthMISOSharedBuffer;

	const size_t OffsetMISOCRC = sizeof(EtherCATApplicationMISOData) - (HeardersSize + sizeof(dataMISO.sharedBuffer));

	dataMISO.crc16bit = CalculateCRC(dataMISO, OffsetMISOCRC, BytesInMISOCRC);

	// Alive signal in sync with the Diag-LED.
	dataMISO.alive =  (uint8_t)( (StepTimer::GetTimerTicks() & (1u << 19)) != 0 );

}

#endif // NETX_ENABLE
