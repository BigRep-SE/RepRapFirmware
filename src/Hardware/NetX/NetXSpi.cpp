/*
 * NetXSpi.cpp
 *
 *  Created on: 16 Jun 2020
 *      Author: David
 */

#include <Hardware/IoPorts.h>
#include <Hardware/NetX/NetXSpi.h>

#if NETX_ENABLE

#if !defined(DUET3_MB6HC)

# include <pmc/pmc.h>
# include <spi/spi.h>

#if defined(DUET3) || defined(SAME70XPLD)
#ifndef USE_XDMAC
# define USE_DMAC			0		// use general DMA controller
# define USE_XDMAC			1		// use XDMA controller
# define USE_DMAC_MANAGER	0		// use SAME5x DmacManager module
#endif // USE_XDMAC
#endif // defined(DUET3) || defined(SAME70XPLD)


#if USE_XDMAC
# include <xdmac/xdmac.h>
#endif // USE_XDMAC

#if USE_DMAC_MANAGER || SAME70
# include <DmacManager.h>
#endif // USE_DMAC_MANAGER || SAME70

// constexpr uint32_t DefaultNetXSpiClockFrequency = 2000000;
constexpr uint32_t SpiTimeout = 1000000;
constexpr uint32_t NetXUartBufferLength = 500;
constexpr uint32_t NetXUartHeaderBufferLength = 11; // TODO: Use size of

#if USE_XDMAC

// Set up the PDC or DMAC to send a register and receive the status, but don't enable it yet
// sendData and rcvData must be aligned.
static void SetupDMA(const uint8_t* tx_data, uint8_t* rx_data, size_t len) noexcept
{
	// Receive
	{
		xdmac_channel_disable(XDMAC, DmacChanNetXSPIRx);
		xdmac_channel_config_t p_cfg = {0, 0, 0, 0, 0, 0, 0, 0};

		p_cfg.mbr_ubc = len;
		p_cfg.mbr_da = reinterpret_cast<uint32_t>(rx_data);
		p_cfg.mbr_sa = reinterpret_cast<uint32_t>(&(NETX_SPI->SPI_RDR));
		p_cfg.mbr_cfg =   XDMAC_CC_TYPE_PER_TRAN
						| XDMAC_CC_MBSIZE_SINGLE
						| XDMAC_CC_DSYNC_PER2MEM
						| XDMAC_CC_CSIZE_CHK_1
						| XDMAC_CC_DWIDTH_BYTE
						| XDMAC_CC_SIF_AHB_IF1
						| XDMAC_CC_DIF_AHB_IF0
						| XDMAC_CC_SAM_FIXED_AM
						| XDMAC_CC_DAM_INCREMENTED_AM
						| XDMAC_CC_PERID(NetXSPI_DmaRxPerid);
		xdmac_configure_transfer(XDMAC, DmacChanNetXSPIRx, &p_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, DmacChanSbcRx, 0);
		xdmac_disable_interrupt(XDMAC, DmacChanSbcRx);
	}

	// Transmit
	{
		xdmac_channel_disable(XDMAC, DmacChanNetXSPITx);
		xdmac_channel_config_t p_cfg = {0, 0, 0, 0, 0, 0, 0, 0};
		p_cfg.mbr_ubc = len;
		p_cfg.mbr_sa = reinterpret_cast<uint32_t>(tx_data);
		p_cfg.mbr_da = reinterpret_cast<uint32_t>(&(NETX_SPI->SPI_TDR));
		p_cfg.mbr_cfg =   XDMAC_CC_TYPE_PER_TRAN
						| XDMAC_CC_MBSIZE_SINGLE
						| XDMAC_CC_DSYNC_MEM2PER
						| XDMAC_CC_CSIZE_CHK_1
						| XDMAC_CC_DWIDTH_BYTE
						| XDMAC_CC_SIF_AHB_IF0
						| XDMAC_CC_DIF_AHB_IF1
						| XDMAC_CC_SAM_INCREMENTED_AM
						| XDMAC_CC_DAM_FIXED_AM
						| XDMAC_CC_PERID(NetXSPI_DmaTxPerid);

		xdmac_configure_transfer(XDMAC, DmacChanNetXSPITx, &p_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, DmacChanNetXSPITx, 0);
		xdmac_disable_interrupt(XDMAC, DmacChanNetXSPITx);
	}

}

static inline void EnableDmaRx() noexcept
{
	xdmac_channel_enable_interrupt(XDMAC, DmacChanNetXSPIRx, XDMAC_CIE_BIE);
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPITx, 0xFFFFFFFF);
	xdmac_channel_enable(XDMAC, DmacChanNetXSPIRx);
	xdmac_channel_enable(XDMAC, DmacChanNetXSPITx);
	xdmac_enable_interrupt(XDMAC, DmacChanNetXSPIRx);
}

static inline void EnableDmaTx() noexcept
{
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPIRx, 0xFFFFFFFF);
	xdmac_channel_disable(XDMAC, DmacChanNetXSPIRx);
	xdmac_channel_enable_interrupt(XDMAC, DmacChanNetXSPITx, XDMAC_CIE_BIE);
	xdmac_channel_enable(XDMAC, DmacChanNetXSPITx);
	xdmac_enable_interrupt(XDMAC, DmacChanNetXSPITx);
}


static inline void DisableDma() noexcept
{
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPITx, 0xFFFFFFFF);
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPIRx, 0xFFFFFFFF);
	xdmac_disable_interrupt(XDMAC, DmacChanNetXSPIRx);
	xdmac_disable_interrupt(XDMAC, DmacChanNetXSPITx);
	xdmac_channel_disable(XDMAC, DmacChanNetXSPITx);
	xdmac_channel_disable(XDMAC, DmacChanNetXSPIRx);
}

// DMA complete callback
void NetXSPIRxDmaCompleteCallback(CallbackParameter param, DmaCallbackReason reason) noexcept
{
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPIRx, 0xFFFFFFFF);
	NetXSpi::GetInstance().Interrupt();
}

void NetXSPITxDmaCompleteCallback(CallbackParameter param, DmaCallbackReason reason) noexcept
{
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPITx, 0xFFFFFFFF);
	NetXSpi::GetInstance().Interrupt();
}


#endif // USE_XDMAC

#endif // !defined(DUET3_MB6HC)

#ifndef NETX_SPI_SPI_HANDLER
# error NETX_SPI_SPI_HANDLER undefined
#endif // NETX_SPI_SPI_HANDLER

/*
extern "C" void NETX_SPI_SPI_HANDLER() noexcept
{
	NetXSpi::GetInstance().Interrupt();
}
*/
void NetXSpi::Interrupt() noexcept
{
#if !defined(DUET3_MB6HC)
	DisableDma();
#endif
	TaskBase::GiveFromISR(taskWaiting);
}

// NetXSpi members
NetXSpi::NetXSpi() noexcept
#if defined(DUET3_MB6HC)
	: device(SharedSpiDevice::GetMainSharedSpiDevice())
#else
	: hardware(NETX_SPI)
#endif // defined(DUET3_MB6HC)
{
#if !defined(DUET3_MB6HC)
	mutex.Create("NetXSpi");
#endif
}

void NetXSpi::Release() noexcept {
	delayMicroseconds(2);
#if defined(DUET3_MB6HC)
	device.Disable();
	device.Release();
#else
	Disable();
	mutex.Release();
#endif
	delayMicroseconds(2);
};

// SharedSpiClient members
#if !defined(DUET3_MB6HC)
void NetXSpi::Disable() const noexcept
{
	spi_disable(hardware);
}

void NetXSpi::Enable() const noexcept
{
	spi_enable(hardware);
}

// Wait for transmitter ready returning true if timed out
inline bool NetXSpi::waitForTxReady() const noexcept
{
	uint32_t timeout = SpiTimeout;
	while (!spi_is_tx_ready(hardware))
	{
		if (--timeout == 0)
		{
			return true;
		}
	}
	return false;
}

// Wait for transmitter empty returning true if timed out
inline bool NetXSpi::waitForTxEmpty() const noexcept
{
	uint32_t timeout = SpiTimeout;
	while (!spi_is_tx_empty(hardware))
	{
		if (!timeout--)
		{
			return true;
		}
	}
	return false;
}

// Wait for receive data available returning true if timed out
inline bool NetXSpi::waitForRxReady() const noexcept
{
	uint32_t timeout = SpiTimeout;
	while (!spi_is_rx_ready(hardware))
	{
		if (--timeout == 0)
		{
			return true;
		}
	}
	return false;
}
#endif
void NetXSpi::SetClockFrequencyAndMode(uint32_t freq, SpiMode mode) const noexcept
{
#if  defined(DUET3_MB6HC)
	device.SetClockFrequencyAndMode(freq, mode);
#else
	// We have to disable SPI device in order to change the baud rate and mode
	spi_disable(hardware);
	hardware->SPI_MR = SPI_MR_MSTR | SPI_MR_MODFDIS;

	spi_disable_mode_fault_detect(hardware);
	spi_disable_loopback(hardware);
	spi_configure_cs_behavior(hardware, 0, SPI_CS_RISE_FORCED);

	// Set SPI mode, clock frequency, CS not active after transfer, delay between transfers
	const uint16_t baud_div = (uint16_t)spi_calc_baudrate_div(freq, SystemPeripheralClock());

	uint32_t csr = SPI_CSR_SCBR(baud_div)				// Baud rate
					| SPI_CSR_BITS_8_BIT				// Transfer bit width
					| SPI_CSR_DLYBCT(0);      			// Transfer delay

	if (((uint8_t)mode & 0x01) == 0)
	{
		csr |= SPI_CSR_NCPHA;
	}
	if ((uint8_t)mode & 0x02)
	{
		csr |= SPI_CSR_CPOL;
	}
	hardware->SPI_CSR[0] = csr;
	//spi_set_transfer_delay(hardware,0,0xC0,0xC0);
	spi_enable(hardware);
#endif
}

// Send and receive data returning true if successful
bool NetXSpi::TransceivePacket(const uint8_t* tx_data, uint8_t* rx_data, size_t len) noexcept
{
	return TransceivePacketWithOffset(tx_data, rx_data, len,0);
}

// Send and receive data returning true if successful
bool NetXSpi::TransceiveData(const uint8_t* tx_data, uint8_t* rx_data, size_t len) noexcept
{
	return TransceiveWithOffset(tx_data, rx_data, len,0);
}

bool NetXSpi::TransceiveWithOffset(const uint8_t* tx_data, uint8_t* rx_data, size_t len, size_t offset) noexcept
{
	bool ret = false;
	if(Take(DefaultTakeTime_ms))
	{
		SetClockFrequencyAndMode();
		WriteCS(false);
#if  defined(DUET3_MB6HC)
		ret = device.TransceivePacketWithOffset(tx_data, rx_data, len, offset);
#else
		ret = TransceivePacketWithOffset(tx_data, rx_data, len, offset);
#endif
		WriteCS(true);
	}
	Release();
	return ret;
}


// We need this function to support the NetX.
bool NetXSpi::TransceivePacketWithOffset(const uint8_t* tx_data, uint8_t* rx_data, size_t len, uint32_t offset) noexcept
{
#if defined(DUET3_MB6HC)
	device.TransceivePacketWithOffset(tx_data, rx_data, len, offset);
#else
	uint32_t offsetCount = 0;

	// Clear any existing data
	(void)hardware->SPI_RDR;

	for (uint32_t i = 0; i < len; ++i)
	{
		uint32_t dOut = (tx_data == nullptr) ? 0x000000FF : (uint32_t)*tx_data++;
		if (waitForTxReady())			// we have to write the first byte after enabling the device without waiting for DRE to be set
		{
			return false;
		}

		// Write to transmit register
		if (i + 1 == len)
		{
			dOut |= SPI_TDR_LASTXFER;
		}
		hardware->SPI_TDR = dOut;

		// MF: Next delay was added in the SAMMY version to control the NetX52
		// delayMicroseconds(10);

		// Some devices are transmit-only e.g. 12864 display, so don't wait for received data if we don't need to
		if (rx_data != nullptr)
		{
			// Wait for receive register
			if (waitForRxReady())
			{
				return false;
			}

			// Get data from receive register
			const uint8_t dIn =
					(uint8_t)hardware->SPI_RDR;
			offsetCount++;
			if(offsetCount > offset)
			{
				*rx_data++ = dIn;
			}
		}
	}

	// Wait for transmitter empty, to make sure that the last clock pulse has finished
	waitForTxEmpty();

	// If we were not receiving, clear data from the receive buffer
	if (rx_data == nullptr)
	{
		(void)hardware->SPI_RDR;
	}
#endif // NETX_SHARED_SPI
	return true;	// success

}

// We need this function to support the NetX.

bool NetXSpi::WriteNonBlocking(const uint8_t *tx_data, const size_t len) const noexcept
{
#if defined(DUET3_MB6HC)
	return device.WriteNonBlocking(tx_data, len);
#else
	Disable();	// Disable SPI

	{
		TaskCriticalSectionLocker lock;
		SetupDMA(tx_data,nullptr,len); // Set up the DMA to with the RX data

		InterruptCriticalSectionLocker lock2;
		EnableDmaTx();
		Enable();
		(void)hardware->SPI_SR; // Clear pending interrupts.

	}
	return true;	// success
#endif

}

bool NetXSpi::ReadNonBlocking(uint8_t *rx_data, const size_t len) const noexcept
{
#if defined(DUET3_MB6HC)
	return device.ReadNonBlocking(rx_data, len);
#else

	Disable();	// Disable SPI

	{
		TaskCriticalSectionLocker lock;
		SetupDMA(nullptr,rx_data,len); // Set up the DMA to with the RX data
		InterruptCriticalSectionLocker lock2;
		EnableDmaRx();
		Enable();
		(void)hardware->SPI_SR; // Clear pending interrupts.
	}

	return true;	// success
#endif
}

bool NetXSpi::SendPacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, uint8_t* tx_data, size_t len_data, bool debugEnable) noexcept
{
	bool ret = false;
	if(Take(DefaultTakeTime_ms))
	{
		SetClockFrequencyAndMode();
		WriteCS(false);
		ret = TransceivePacket(address.GetWriteHeader(), nullptr, address.GetWriteHeaderSize());
		// We need to reconfigure again the SPI
		if(ret)
		{
			SetClockFrequencyAndMode();
			TaskBase::ClearCurrentTaskNotifyCount();
			taskWaiting = task;//TaskBase::GetCallerTaskHandle();
			ret = WriteNonBlocking(tx_data,len_data);
			if(ret)
			{
				ret = task->Take(DefaultTimeout_ms);
#if  defined(DUET3_MB6HC)
				device.
#endif
				DisableDma();
				if(!ret)
				{
					debugPrintf("\nERROR: TIMEOUT\n");
				}
			}
		}
		WriteCS(true);
	}
	Release();

#ifdef NETX_DEBUG
	if (ret && debugEnable)
	{
		debugPrintf("Writing [%06lX]: ", address.GetAddress() );

		for(uint32_t i = 0; i < (uint32_t)len_data; ++i )
		{
			if((i % 32) == 0 && i != 0)
			{
				debugPrintf("\n                  " );
			}
			debugPrintf("%02X ", tx_data[i] );
		}
		debugPrintf("\n" );
	}
#endif
	return ret;
}

bool NetXSpi::ReceivePacketWithHeaderNonBlocking(TaskBase *task, NetXAddress &address, uint8_t* rx_data, size_t len_data, bool debugEnable) noexcept
{
	bool ret = false;
	if(Take(DefaultTakeTime_ms))
	{
		SetClockFrequencyAndMode();
		WriteCS(false);
		ret = TransceivePacket(address.GetReadHeader(len_data), nullptr, address.GetReadHeaderSize());
		// We need to reconfigure again the SPI
		if(ret)
		{
			SetClockFrequencyAndMode();
			TaskBase::ClearCurrentTaskNotifyCount();
			taskWaiting = task;//TaskBase::GetCallerTaskHandle();
			ret = ReadNonBlocking(rx_data,len_data);
			if(ret)
			{
				ret = task->Take(DefaultTimeout_ms);
#if  defined(DUET3_MB6HC)
				device.
#endif
				DisableDma();
				if(!ret)
				{
					debugPrintf("\nERROR: TIMEOUT\n");
				}
			}
		}
		WriteCS(true);
	}
	Release();

#ifdef NETX_DEBUG
	if (ret && debugEnable)
	{
		debugPrintf("Reading [%06lX]: ", address.GetAddress() );

		for(uint32_t i = 0; i < (uint32_t)len_data; ++i )
		{
			if((i % 32) == 0 && i != 0)
			{
				debugPrintf("\n                  " );
			}
			debugPrintf("%02X ", rx_data[i] );
		}
		debugPrintf("\n" );
	}
#endif
	return ret;
}



bool NetXSpi::SendPacketWithHeader(const uint8_t* tx_header,size_t len_header, uint8_t* tx_data, size_t len_data) noexcept
{
	bool ret = false;
	if(Take(DefaultTakeTime_ms))
	{
		SetClockFrequencyAndMode();
		WriteCS(false);
		ret = TransceivePacket(tx_header, nullptr, len_header);
		ret = TransceivePacket(tx_data, nullptr, len_data);
		WriteCS(true);
	}
	Release();
	return ret;
}

void NetXSpi::WriteCS(bool state) const noexcept
{
	delayMicroseconds(2);
#if defined(DUET3_MB6HC)
	device.WriteCS(state);
#else
	IoPort::WriteDigital(APIN_NETX_SPI_CS,state);
#endif
	delayMicroseconds(2);
}

void NetXSpi::Init() noexcept
{
#if !defined(DUET3_MB6HC)
	SetPinFunction(APIN_NETX_SPI_MOSI, NetXSPIPeriphMode);
	SetPinFunction(APIN_NETX_SPI_MISO, NetXSPIPeriphMode);
	SetPinFunction(APIN_NETX_SPI_SCK, NetXSPIPeriphMode);
	IoPort::SetPinMode(APIN_NETX_SPI_CS, OUTPUT_HIGH);		// We make sure that the NetX CS is configured
	IoPort::WriteDigital(APIN_NETX_SPI_CS,true);

	pmc_enable_periph_clk(NETX_SPI_INTERFACE_ID);
#if SAME70
	xdmac_channel_disable_interrupt(XDMAC, DmacChanNetXSPIRx, 0xFFFFFFFF);
	DmacManager::SetInterruptCallback(DmacChanNetXSPITx, NetXSPITxDmaCompleteCallback, CallbackParameter());
	DmacManager::SetInterruptCallback(DmacChanNetXSPIRx, NetXSPIRxDmaCompleteCallback, CallbackParameter());
	xdmac_disable_interrupt(XDMAC, DmacChanNetXSPIRx);
	xdmac_disable_interrupt(XDMAC, DmacChanNetXSPITx);
#endif // SAME70
#else
	device.WriteCS(true);
#endif
}

#endif // NETX_ENABLE
// End
