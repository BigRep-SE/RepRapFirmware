/*
 * NetXSpi.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_NETXSPI_H_
#define SRC_HARDWARE_NETX_NETXSPI_H_

#include <RepRapFirmware.h>
#include <RTOSIface/RTOSIface.h>

#if NETX_ENABLE
#include <Platform/RepRap.h>
#include "Hardware/Spi/SpiMode.h"

#if defined(DUET3_MB6HC)
#include <Hardware/SharedSpi/SharedSpiDevice.h>
#endif // defined(DUET3_MB6HC)

class NetXAddress {
public:
    NetXAddress() {};
    NetXAddress(const uint32_t baseAddress) : currentAddress(baseAddress){};

    template<typename T>
    NetXAddress(const T baseAddress) : currentAddress(static_cast<uint32_t>(baseAddress)){};

    template<typename T>
    NetXAddress& Append(T addressOffset)
	{
		currentAddress += static_cast<uint32_t>(addressOffset);
		return *this;
	};

    NetXAddress& Append(const uint32_t addressOffset)
    {
        currentAddress += addressOffset;
        return *this;
    };
    NetXAddress& Append(const NetXAddress &baseAddress)
	{
		currentAddress += baseAddress.GetAddress();
		return *this;
	};

    inline const uint32_t GetAddress() const { return currentAddress; };

    void Print() const{
#ifdef NETX_DEBUG
    	debugPrintf("0x%06lX", currentAddress);
#endif
    };

    const uint8_t* GetReadHeader(const size_t byteToRead)
    {
        header[0] = SetReadBit((uint8_t)((currentAddress >> 16) & 0x0F)); 	// Byte[0] Enable Read bit and use bits 23 to 16 bits
        header[1] = (uint8_t)(currentAddress >> 8);							// Byte[1] Address 15 to 8 bits
        header[2] = (uint8_t)(currentAddress >> 0);							// Byte[2] Address 7 to 0 bits
        header[3] = (((uint16_t)byteToRead <= 255) ? (uint8_t)byteToRead : (uint8_t)0);			// Byte[3] Amount of bytes to read from memory
        return header;
    };

    const uint8_t* GetWriteHeader()
    {
        header[0] = ((uint8_t)((currentAddress >> 16) & 0x0F)); 	// Byte[0] Address 23 to 16 bits
        header[1] = (uint8_t)(currentAddress >> 8);					// Byte[1] Address 15 to 8 bits
        header[2] = (uint8_t)(currentAddress >> 0);					// Byte[2] Address 7 to 0 bits
        return header;
    };

    static const size_t GetReadHeaderSize() { return 4; };
    static const size_t GetWriteHeaderSize() { return 3; };

private:

    uint32_t currentAddress = 0;
    uint8_t header[4] = {0,0,0,0};

    inline uint8_t SetReadBit(uint8_t val) { return val | 0x80; }
};

class NetXSpi
{
public:
	using  DPMAddress 		= uint32_t;

	void SetClockFrequencyAndMode(uint32_t freq = DefaultSpiClockFrequency, SpiMode mode = DefaultSpiMode) const noexcept;

	bool TransceiveData(const uint8_t* tx_data, uint8_t* rx_data, size_t len) noexcept;

	bool TransceiveWithOffset(const uint8_t* tx_data, uint8_t* rx_data, size_t len, size_t offset) noexcept;

	// Get ownership of this SPI, return true if successful
	bool Take(uint32_t timeout) noexcept {
#if defined(DUET3_MB6HC)
		return device.Take(timeout);
#else
		return mutex.Take(timeout);
#endif // defined(DUET3_MB6HC)
	}

	void Init() noexcept;

	static NetXSpi& GetInstance() noexcept
	{
		static NetXSpi instance;
		return instance;
	}

	NetXSpi(NetXSpi const&)     = delete;
	void operator=(NetXSpi const&)  = delete;

	// Needed for the NetX
	bool ReadNonBlocking(uint8_t *rx_data, const size_t len) const noexcept;
	bool WriteNonBlocking(const uint8_t *tx_data, const size_t len) const noexcept;

	bool TransceivePacket(const uint8_t* tx_data, uint8_t* rx_data, size_t len) noexcept;

	bool SendPacketWithHeader(const uint8_t* tx_header,size_t len_header, uint8_t* tx_data, size_t len_data) noexcept;

	template<typename T>
	bool ReceivePacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, T &data, bool debugEnable = false) noexcept
	{
		return ReceivePacketWithHeaderNonBlocking(task, address, (uint8_t*)(&data), sizeof(T), debugEnable);
	};

	template<typename T>
	bool SendPacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, T &data, bool debugEnable = false) noexcept
	{
		return SendPacketWithHeaderNonBlocking(task, address, (uint8_t*)(&data), sizeof(T), debugEnable);
	};

	void WriteCS(bool state) const noexcept;

	template<typename Taddress, typename T>
	bool ReadAddress(Taddress addressDPM, T&data, bool enableDebug = false) noexcept
	{
		NetXAddress address{addressDPM};
		return ReadAddress(address,data, enableDebug);
	}

	template<typename T>
	bool ReadAddress(NetXAddress &address, T&data, bool enableDebug = false) noexcept
	{
		const size_t offset = NetXAddress::GetReadHeaderSize();
		const size_t bytesToRead = sizeof(T) ;
		uint8_t *pData = (uint8_t*)&data;
		bool ret = TransceiveWithOffset(address.GetReadHeader(bytesToRead), pData, bytesToRead + offset, offset);

#ifdef NETX_DEBUG
		if (ret && enableDebug)
		{
			debugPrintf("Reading [%06lX]: ", address.GetAddress() );
			for(int i = 0; i < (int)bytesToRead; ++i )
			{
				if((i % 32) == 0 && i != 0)
				{
					debugPrintf("\n                  " );
				}
				debugPrintf("%02X ", pData[i] );
			}
			debugPrintf("\n" );
		}
#endif
		return ret;
	}

	/*
	template<typename Taddress, typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
	T GetIntegerFromAddress(Taddress addressDPM, bool enableDebug = false) noexcept
	{
		NetXAddress addressToRead{addressDPM};
		return GetIntegerFromAddress<T>(addressToRead,enableDebug);
	};
	*/

	template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
	T GetIntegerFromAddress(NetXAddress &&addressToRead, bool enableDebug = false) noexcept
	{
		return GetIntegerFromAddress<T>(addressToRead, enableDebug);
	};

	template<typename T, typename = typename std::enable_if<std::is_integral<T>::value, T>::type>
	T GetIntegerFromAddress(NetXAddress &addressToRead, bool enableDebug = false) noexcept
	{
		T var;
		if(ReadAddress(addressToRead, var, enableDebug))
		{
			return var;
		}
		else
		{
			return 0;
		}
	};

	void Interrupt() noexcept;
	volatile TaskHandle taskWaiting = nullptr;  // Just for DMA

private:

#if defined(DUET3_MB6HC)
	static constexpr uint32_t 	DefaultSpiClockFrequency 	= 24000000; // [Hz]
#else
	static constexpr uint32_t 	DefaultSpiClockFrequency 	= 40000000; // [Hz]
#endif // defined(DUET3_MB6HC)
	static constexpr SpiMode 	DefaultSpiMode 				= SpiMode::mode3;
	static constexpr uint32_t 	DefaultTakeTime_ms 			= 500;
	static constexpr uint32_t 	DefaultTimeout_ms 			= 5;		// [ms]
	NetXSpi() noexcept;

	void Release() noexcept;

#if !defined(DUET3_MB6HC)
	// Release ownership of this SPI and Disables it
	void Enable() const noexcept;
	void Disable() const noexcept;
	bool waitForTxReady() const noexcept;
	bool waitForTxEmpty() const noexcept;
	bool waitForRxReady() const noexcept;
#endif

	bool TransceivePacketWithOffset(const uint8_t *tx_data, uint8_t *rx_data, size_t len, uint32_t offset) noexcept;
	bool ReceivePacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, uint8_t* rx_data, size_t len_data, bool debugEnable = false) noexcept;
	bool SendPacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, uint8_t* tx_data, size_t len_data, bool debugEnable) noexcept;

#if defined(DUET3_MB6HC)
	SharedSpiDevice& device;
#else
	Spi * const hardware;
	Mutex mutex;
#endif // defined(DUET3_MB6HC)
	static NetXSpi *mainNetXSpi;
};

#endif // NETX_ENABLED

#endif /* SRC_HARDWARE_NETX_NETXSPI_H_ */
