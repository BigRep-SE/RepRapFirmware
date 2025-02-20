#ifndef SRC_POWERMETER_ET112_H_
#define SRC_POWERMETER_ET112_H_

#include <RepRapFirmware.h>

#if SUPPORT_POWERMETER

#include "EnergyMeter.h"

class ET112 : public EnergyMeter
{
private:
	static const uint32_t DefaultPowerMeterBaudRate = 9600;		// Default baudrate
	static const uint8_t DefaultSlaveAddress = 1;				// Default slave address

	static const uint16_t DeviceIdentificationCode 	= 120;		// ET112-DIN AV0 1 x S1 X
	static const uint16_t MaxResponseTime_Ms 		= 510;		// [msec] Max Response time
	static const uint16_t MinDelayBetweenFrames_Ms 	= 5;		// [msec] Min Delay between frames

	AsyncSerial485 * _rs485;
	uint8_t _deviceAddress;
public:
	ET112(AsyncSerial485 * rs485, uint8_t deviceAddress = DefaultSlaveAddress) noexcept;
	// Do a quick test to check whether the power meter is present, returning true if it is
	bool CheckPresent() noexcept;

	void Init() noexcept override;

	bool IsConnected() const noexcept override { return isConnected; };

	void Poll() noexcept override;

	const char *GetMeterType() const noexcept override {return "ET112";};

private:

	bool isConnected = false;

	// Get single parameters (slower)
	bool ReadHours(float &hours_H) noexcept;
	bool ReadVoltage(float &voltage_V) noexcept;
	bool ReadCurrent(float &current_mA) noexcept;
	bool ReadWattage(float &wattage_W) noexcept;

	// Reading multiple parameters with one request. (faster)
	bool ReadVoltageCurrentWattage(float &voltage_V, float &current_mA, float &wattage_W) noexcept;
	bool ReadPartialEnergiers(float &kWhPartial, float &kVARh, float &kWhT1, float &kWhT2) noexcept;
	bool ReadTotalEnergiers(float &kWh_P, float &kWh_N, float &kVARh_P, float &kVARh_N) noexcept;

	// Get a status byte
	uint8_t ReadStatus() noexcept;

	enum class MessagesType : uint8_t
	{
		readHoldingRegister 		= 0x03,
		readInputRegister 			= 0x04,
		writeSingleHoldingRegister 	= 0x06,
		diagnostic 					= 0x08
	};

	enum class AddressType : uint16_t {
			VoltageLN				= 0x0000,		// (INT32) Volt*10
			Current					= 0x0002,		// (INT32) mA
			Wattage					= 0x0004,		// (INT32) Watt*10
			IdentificationCode      = 0x000B,		// (UINT16)
			kWhPartial			  	= 0x0014,		// (INT32) Partial kWh*10
			kvarhPartial			= 0x0016,		// (INT32) Partial kvarh*10
			kWhT1					= 0x0018,		// (INT32) t1 kWh*10
			kWhT2					= 0x0018,		// (INT32) t2 kWh*10
			HourCounter  			= 0x002C,		// (INT32) Hours*100
			kWhPositiveTotal		= 0x0112,		// (INT32) kWh Total (+) kWh*10
			kvarhPositiveTotal		= 0x0114,		// (INT32) kvarh Total (+) kWh*10
			kWhNegativeTotal		= 0x0116,		// (INT32) kWh Total (-) kWh*10
			kvarhNegativeTotal		= 0x0118		// (INT32) kvarh Total (-) kWh*10
	  };


	struct __attribute__((packed)) RequestMessage{
		uint8_t 			slaveAddress;		// Slave address
		MessagesType 		messageType;		// Read, write, diagnostic.
		AddressType			memoryAddress;		// Memory address to read or write
		uint16_t			amountOfWords;		// Amount of words to read or write.
		uint16_t			crc16;				// CRC
	};

	bool CheckCRC(uint8_t *buf, uint8_t lengthFullBuffer);
	uint16_t CalcCRC(uint8_t *buf, uint8_t length);

	bool ReadRegister(AddressType addressToRead, uint16_t &responseWords) noexcept;
	bool ReadRegister(AddressType addressToRead, uint32_t &responseWords) noexcept;
	bool ReadRegister(AddressType addressToRead, uint32_t *buffer, size_t length) noexcept;
	bool ReadAddress(AddressType addressToRead, uint8_t *buf, size_t length) noexcept;
	//bool WriteRegisters(ET112Register reg, size_t numToWrite) noexcept;

};

#endif

#endif /* SRC_POWERMETER_ET112_H_ */
