/*
 * NetXSerial.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_NETXSERIAL_H_
#define SRC_HARDWARE_NETX_NETXSERIAL_H_

#include "Core.h"
#include "Stream.h"
#include "Hardware/NetX/ProtocolOverEtherCAT.h"
#include "Hardware/NetX/NetXBuffer.h"

#if NETX_ENABLE

class NetXSerial : public Stream
{
private:
	bool 	isConnected = false;							// TODO: Support check connected.

	uint32_t txAccumBytes = 0;								// Total amount of bytes transfered using the NetX (header bytes are not considered)
	uint32_t rxAccumBytes = 0;								// Total amount of bytes received using the NetX (header bytes are not considered)


public:

	static NetXSerial& GetInstance() noexcept
	{
		static NetXSerial instance; // Guaranteed to be destroyed.
		  // Instantiated on first use.
		return instance;
	};

private:
	NetXSerial() noexcept {};
	ProtocolOverEtherCAT &protocol = ProtocolOverEtherCAT::GetInstance();

public:

	// static const uint16_t	BUFFER_LENGHT = 500;

	void Init() noexcept;

	int available() noexcept override;
	int read() noexcept override;
	size_t readBytes(char *buffer, size_t length) noexcept override;
	void flush() noexcept override;
	size_t write(uint8_t) noexcept override;
	size_t write(const uint8_t *buffer, size_t size) noexcept override;

	size_t write(const char *str) noexcept { return write((const uint8_t *)str, strlen(str)); }
    size_t write(const char *buffer, size_t size) noexcept { return write((const uint8_t *)buffer, size); }

    size_t canWrite() noexcept override;
	bool IsConnected() const noexcept;

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

};

#endif

#endif /* SRC_HARDWARE_NETX_NETXSERIAL_H_ */

