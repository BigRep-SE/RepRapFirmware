/*
 * NetXBuffer.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */


#include "NetXBuffer.h"

#if NETX_ENABLE

bool NetXQueue::Add (const uint8_t *pDataToAppend, const size_t bytesToAppend) noexcept
{
	bool ret = false;

	const size_t appendIdx = indexAppend;
	const size_t getIdx = indexGet;
	const size_t elements = (appendIdx >= getIdx) ? (appendIdx - getIdx) : (appendIdx + length - getIdx);

	if( (elements + bytesToAppend) < length )
	{
		// Copy data
		if( ( appendIdx + bytesToAppend ) <= length )
		{
			memcpy( &pData[appendIdx], pDataToAppend, bytesToAppend );
		}
		else
		{
			uint16_t inLastPartOfTheBuffer = length - appendIdx;
			memcpy( &pData[appendIdx], pDataToAppend, inLastPartOfTheBuffer );
			memcpy( pData, &pDataToAppend[inLastPartOfTheBuffer], bytesToAppend - inLastPartOfTheBuffer );
		}
		// Update Index
		indexAppend = ( appendIdx + bytesToAppend ) % length;

		ret = true;
	}
	else
	{
		// debugPrintf("Buffer Overflow");
	}

	return ret;
}

bool NetXQueue::Poll (uint8_t *pDataRead, const size_t bytesToRead) noexcept
{
	bool ret = false;
	const size_t appendIdx = indexAppend;
	const size_t getIdx = indexGet;
	const size_t elements = (appendIdx >= getIdx) ? (appendIdx - getIdx) : (appendIdx + length - getIdx);
	if( bytesToRead <= elements )
	{
		// Copy data
		if( (getIdx + bytesToRead) <= length )
		{
			memcpy( pDataRead, &pData[getIdx], bytesToRead);
		}
		else
		{
			uint16_t inLastPartOfTheBuffer = length - getIdx;
			memcpy( pDataRead, &pData[getIdx], inLastPartOfTheBuffer);
			memcpy( &pDataRead[inLastPartOfTheBuffer], pData, bytesToRead - inLastPartOfTheBuffer);
		}
		// Update Index
		indexGet = (getIdx + bytesToRead) % length;

		ret = true;
	}

	return ret;
}

size_t NetXQueue::Size () const noexcept
{
	const size_t appendIdx = indexAppend;
	const size_t getIdx = indexGet;
	return (appendIdx >= getIdx) ? (appendIdx - getIdx) : (appendIdx + length - getIdx);
}

#endif
