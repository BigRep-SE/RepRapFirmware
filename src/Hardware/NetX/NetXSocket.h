/*
 * NetXSocket.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_NETXSOCKET_H_
#define SRC_HARDWARE_NETX_NETXSOCKET_H_

#include "RepRapFirmware.h"
#include "Network.h"
#include "NetworkBuffer.h"
#include "Networking/NetworkDefs.h"
#include "Networking/Socket.h"
#include "Hardware/NetX/ProtocolOverEtherCAT.h"

#if NETX_ENABLE

class NetXSocket : public Socket
{
public:

	static NetXSocket& GetInstance() noexcept
	{
		static NetXSocket instance; // Guaranteed to be destroyed.
		  // Instantiated on first use.
		return instance;
	};
private:
	NetXSocket() noexcept ;

public:

	int State() const noexcept { return (int)state; }
	void Poll() noexcept override;
	void Close() noexcept override;
	bool IsClosing() const noexcept { return (state == SocketState::closing); }
	void Terminate() noexcept override;
	void TerminateAndDisable() noexcept override { Terminate(); }
	bool ReadChar(char& c) noexcept override;
	bool ReadBuffer(const uint8_t *&buffer, size_t &len) noexcept override;
	void Taken(size_t len) noexcept override;
	bool CanRead() const noexcept override;
	bool CanSend() const noexcept override;
	size_t Send(const uint8_t *data, size_t length) noexcept override;
	void Send() noexcept override;
	void SetNeedsPolling() noexcept { needsPolling = true; }
	bool NeedsPolling() const noexcept;
	void Connected() noexcept;

	/*
	 * Return the total amount of bytes that were received from the master
	 * and clears the accumulator.
	 */
	uint32_t GetTotalBytesReceivedAndClear() noexcept;

	/*
	 * Return the total amount of bytes that were transmitted to the master
	 * and clears the accumulator.
	 */
	uint32_t GetTotalBytesTransmittedAndClear() noexcept;
private:

	// If the buffer is almost full we add a delay to give time to the NetX to send the data.
	static constexpr size_t MinFreeSpaceToAddDelay = 1000; ///< Min. Amount of bytes available in the queue to trigger the delay.
	static constexpr size_t MinFreeSpaceDelayMs 	= 15;	///< Delay time

	enum class SocketState : uint8_t
	{
		inactive,
		waitingForResponder,
		connected,
		clientDisconnecting,
		closing,
		broken
	};

	void ReceiveData() noexcept;
	void DiscardReceivedData() noexcept;

	NetworkBuffer *receivedData = nullptr;					// List of buffers holding received data
	uint32_t whenConnected = 0;
	SocketNumber socketNum = 0;								// The socket number we are using
	SocketState state = SocketState::inactive;
	bool needsPolling = false;
	uint32_t timeoutLoop = 0 ;
	uint32_t txAccumBytes = 0;								// Total amount of bytes transfered using the NetX (header bytes are not considered)
	uint32_t rxAccumBytes = 0;								// Total amount of bytes received using the NetX (header bytes are not considered)
	NetworkBuffer * prevBuffer = nullptr;

	ProtocolOverEtherCAT &protocolECAT = ProtocolOverEtherCAT::GetInstance();
};

#endif

#endif
