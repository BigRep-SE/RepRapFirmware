/*
 * ProtocolOverEtherCAT.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_PROTOCOLOVERETHERCAT_H_
#define SRC_HARDWARE_NETX_PROTOCOLOVERETHERCAT_H_

#include <RepRapFirmware.h>

#if NETX_ENABLE

#include <Platform/RepRap.h>
#include <Platform/Platform.h>
#include <Hardware/NetX/NetXStructs.h>
#include <Hardware/NetX/NetXBuffer.h>

class ProtocolOverEtherCAT
{
private:
	uint8_t 	lastIdxSerialMOSI = NETX_UART_RESET_INDEX;
	uint8_t 	lastIdxSerialMISO = NETX_UART_RESET_INDEX;
	uint8_t 	lastIdxSocketMOSI = NETX_UART_RESET_INDEX;
	uint8_t 	lastIdxSocketMISO = NETX_UART_RESET_INDEX;



public:

	NetXBidirectionalBuffer serialChannel, socketChannel;

	static ProtocolOverEtherCAT& GetInstance() noexcept
	{
		static ProtocolOverEtherCAT instance;
		return instance;
	}

	void ProcessMOSIData(EtherCATApplicationMOSIData &dataMOSI, EtherCATApplicationMISOData & dataMISO) noexcept;

	void PrepareMISOData(EtherCATApplicationMOSIData &dataMOSI, EtherCATApplicationMISOData & dataMISO) noexcept;

	void Reset() noexcept
	{
		lastIdxSerialMOSI = NETX_UART_RESET_INDEX;
		lastIdxSerialMISO = NETX_UART_RESET_INDEX;
		lastIdxSocketMOSI = NETX_UART_RESET_INDEX;
		lastIdxSocketMISO = NETX_UART_RESET_INDEX;
	}

private:
	ProtocolOverEtherCAT() noexcept;

	template<typename T>
	EtherCATCRC_t CalculateCRC(T &data, const size_t offset = 0) noexcept
	{
		EtherCATCRC_t crc = 0;
		uint8_t *p = (uint8_t *) &data;
		for (uint16_t i = (uint16_t)offset; i < sizeof(T);i++)
		{
			crc += (EtherCATCRC_t)p[i];
		}
		return crc;
	};

	template<typename T>
	EtherCATCRC_t CalculateCRC(T &data, const size_t offset = 0, const size_t length = 0) noexcept
	{
		EtherCATCRC_t crc = 0;
		uint8_t *p = (uint8_t *) &data;
		for (uint16_t i = (uint16_t)offset; i < (uint16_t)(offset+length) ;i++)
		{
			crc += (EtherCATCRC_t)p[i];
		}
		return crc;
	};

};
#endif // NETX_ENABLE

#endif /* SRC_HARDWARE_NETX_NETX_H_ */
