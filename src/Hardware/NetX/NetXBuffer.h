/*
 * NetXBuffer.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */


#ifndef SRC_HARDWARE_NETX_NETXBUFFER_H_
#define SRC_HARDWARE_NETX_NETXBUFFER_H_

#include <stdio.h>                  /** Include C standard library input/output header */
#include <string.h>
#include <RepRapFirmware.h>

#if NETX_ENABLE

struct NetXBufferData
{
	uint8_t * const pData;
	size_t const length;
	NetXBufferData(uint8_t * const pBuffer, const size_t bufferLength) : pData(pBuffer), length(bufferLength){};
};

class NetXQueue
{
public:

	NetXQueue() = delete;
	NetXQueue(const NetXBufferData bufferDescription) :
		pData(bufferDescription.pData),
		length(bufferDescription.length){};

	bool Add (const uint8_t *pDataToAppend, const size_t bytesToAppend) noexcept;
	bool Poll (uint8_t * pDataRead, const size_t bytesToRead = 1) noexcept;

	size_t Size () const noexcept;
	size_t Capacity () const noexcept { return length; };
	inline size_t FreeSpace() const noexcept
	{
		return length - Size();
	}

	void Clear () noexcept
	{
		indexGet = 0;
		indexAppend = 0;
	};

private:
	uint8_t *pData = nullptr;
	size_t length = 0;
	volatile size_t indexGet = 0 ;
	volatile size_t indexAppend = 0;
};

struct NetXBidirectionalBuffer
{
	NetXBidirectionalBuffer() = delete;
	NetXBidirectionalBuffer(const NetXBufferData rxBufferDescription, const NetXBufferData txBufferDescription) :
		rxQueue(rxBufferDescription), txQueue(rxBufferDescription) {};

	NetXQueue rxQueue, txQueue;

};

#endif

#endif
