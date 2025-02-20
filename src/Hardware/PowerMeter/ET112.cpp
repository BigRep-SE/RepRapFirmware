#include "ET112.h"

#if SUPPORT_POWERMETER

#include <Hardware/IoPorts.h>

extern void debugPowerMeter(const char* fmt, ...) noexcept;

ET112::ET112(AsyncSerial485 * rs485,uint8_t deviceAddress) noexcept : _rs485(rs485), _deviceAddress(deviceAddress)
{
}

void ET112::Init() noexcept
{
	// Let's make sure the RS485 output is disabled
	IoPort::SetPinMode(SERIAL_RS485_SELECT, OUTPUT_HIGH);		// We make sure that the NetX CS is configured
	IoPort::WriteDigital(SERIAL_RS485_SELECT,false);

	_rs485->begin(DefaultPowerMeterBaudRate);
	_rs485->setInterruptPriority(NvicPriorityAuxUart);
}

void ET112::Poll() noexcept
{
	isConnected = CheckPresent();
	if(!isConnected)
	{
		debugPowerMeter("ET112 Not Found\r\n") ;
	}
	else
	{
		debugPowerMeter("ET112 Device Present\r\n") ;
		if(ReadHours(values.hours_H)) {
			debugPowerMeter("Hours: %.2f\r\n",(double)values.hours_H);
		}
		/*
		if(powerMeterET112->GetVoltage(voltage_V)) {
			debugPowerMeter("Volts: %.1f V\r\n",(double)voltage_V);
		}
		if(powerMeterET112->GetCurrent(current_mA)) {
			debugPowerMeter("Amps:  %.3f A\r\n",(double)current_mA/(double)1000.0);
		}
		if(powerMeterET112->GetWattage(wattage_W)) {
			debugPowerMeter("Watts: %.1f W\r\n",(double)wattage_W);
		}
		*/
		if(ReadVoltageCurrentWattage(values.voltage_V, values.current_A, values.energy_W))
		{
			debugPowerMeter("%.1f V | %.3f A | %.1f W\r\n",
					(double)values.voltage_V,
					(double)values.current_A,
					(double)values.energy_W);
		}
		if(ReadPartialEnergiers(values.energyPartial_kWh, values.energyPartial_kVARh, values.energyPartial_kWhT1, values.energyPartial_kWhT2))
		{
			debugPowerMeter("%.1f kWh | %.1f kVARh | %.1f kWh T1 | %.1f kWh T2\r\n",
					(double)values.energyPartial_kWh,
					(double)values.energyPartial_kVARh,
					(double)values.energyPartial_kWhT1,
					(double)values.energyPartial_kWhT2);
		}
		if(ReadTotalEnergiers(values.energyTotal_kWh_P, values.energyTotal_kWh_N, values.energyTotal_kVARh_P, values.energyTotal_kVARh_N))
		{
			debugPowerMeter("%.1f kWh (+) | %.1f kWh (-) | %.1f kVARh (+) | %.1f kVARh (-)\r\n",
					(double)values.energyTotal_kWh_P,
					(double)values.energyTotal_kWh_N,
					(double)values.energyTotal_kVARh_P,
					(double)values.energyTotal_kVARh_N);
		}
		whenLastRead = millis();
	}
}

// Do a quick test to check whether the accelerometer is present, returning true if it is
bool ET112::CheckPresent() noexcept
{
	bool ret = false;
	debugPowerMeter("Check Present | ");
	uint16_t val;
	if(ReadRegister(AddressType::IdentificationCode, val))
	{
		ret = (val == DeviceIdentificationCode);
	}
	if(isConnected != ret)
	{
		isConnected = ret;
	}
	return ret;
}

bool ET112::ReadHours(float &hours_H) noexcept
{
	debugPowerMeter("Get Hours     | ");
	uint32_t val;
	if(ReadRegister(AddressType::HourCounter, val))
	{
		hours_H = ((float)val)/100.0;
		return true;
	}
	return false;
}

bool ET112::ReadVoltage(float &voltage_V) noexcept
{
	debugPowerMeter("Get Volts     | ");
	uint32_t val;
	if(ReadRegister(AddressType::VoltageLN, val))
	{
		voltage_V = ((float)val)/10.0;
		return true;
	}
	return false;
}

bool ET112::ReadCurrent(float &current_mA) noexcept
{
	debugPowerMeter("Get Current   | ");
	uint32_t val;
	if(ReadRegister(AddressType::Current, val))
	{
		current_mA = ((float)val);
		return true;
	}
	return false;
}

bool ET112::ReadWattage(float &wattage_W) noexcept
{
	debugPowerMeter("Get Wattage   | ");
	uint32_t data;
	if(ReadRegister(AddressType::Wattage, data))
	{
		wattage_W = ((float)data)/10.0;
		return true;
	}
	return false;
}

bool ET112::ReadVoltageCurrentWattage(float &voltage_V, float &current_A, float &wattage_W) noexcept
{
	debugPowerMeter("Get V I W     | ");
	uint32_t data[3];
	if(ReadRegister(AddressType::VoltageLN, data, sizeof(data)/sizeof(data[0])))
	{
		voltage_V 	= ((float)data[0]) / 10.0;
		current_A 	= ((float)data[1]) / 1000.0; // The data is in mA
		wattage_W 	= ((float)data[2]) / 10.0;
		return true;
	}
	return false;
}


bool ET112::ReadPartialEnergiers(float &kWhPartial, float &kVARh, float &kWhT1, float &kWhT2) noexcept
{
	debugPowerMeter("Get Partial E | ");
	uint32_t data[4];
	if(ReadRegister(AddressType::kWhPartial, data, sizeof(data)/sizeof(data[0])))
	{
		kWhPartial 	= ((float)data[0])/10.0;
		kVARh	 	= ((float)data[1])/10.0;
		kWhT1	 	= ((float)data[2])/10.0;
		kWhT2	 	= ((float)data[3])/10.0;
		return true;
	}
	return false;
}

bool ET112::ReadTotalEnergiers(float &kWh_P, float &kWh_N, float &kVARh_P, float &kVARh_N) noexcept
{
	debugPowerMeter("Get Total E   | ");
	uint32_t data[4];
	if(ReadRegister(AddressType::kWhPositiveTotal, data, sizeof(data)/sizeof(data[0])))
	{
		kWh_P	 	= ((float)data[0])/10.0;
		kVARh_P	 	= ((float)data[1])/10.0;
		kWh_N	 	= ((float)data[2])/10.0;
		kVARh_N	 	= ((float)data[3])/10.0;
		return true;
	}
	return false;
}


bool ET112::CheckCRC(uint8_t *buf, uint8_t lengthFullBuffer)
{
	uint16_t crc = CalcCRC(buf, lengthFullBuffer-2);
	uint8_t crcLsb =  (uint8_t)(crc & 0x00FF) ;
	uint8_t crcMsb =  (uint8_t)(crc >> 8) ;

	if( buf[lengthFullBuffer-2] != crcLsb || buf[lengthFullBuffer-1] != crcMsb  )
	{
		return false;
	}
	return true;
}

uint16_t ET112::CalcCRC(uint8_t *buf, uint8_t length)
{
	uint16_t crc = 0xFFFF;
	for (uint8_t index = 0; index < length; index++)
	{
		crc ^= (uint16_t)buf[index];
		for (int i = 8; i != 0; i--)
		{
			if ((crc & 0x0001) != 0)
			{
				// Shift right and XOR 0xA001
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				// Just shift right
				crc >>= 1;
			}
		}
	}
	return crc;
}

bool ET112::ReadRegister(AddressType addressToRead, uint16_t &responseWords) noexcept
{
	uint8_t buf[2];
	if(ReadAddress(addressToRead,buf,sizeof(buf)))
	{
		responseWords = (((uint16_t)buf[0]) << 8)  + ((uint16_t)buf[1]) ;	// (MSB LSB)
		return true;
	}
	return false;

}

bool ET112::ReadRegister(AddressType addressToRead, uint32_t &responseWords) noexcept
{
	uint8_t buf[4];
	if(ReadAddress(addressToRead,buf,sizeof(buf)))
	{
		responseWords =
				(((uint32_t)buf[0]) << 8)  +  ((uint32_t)buf[1])       + 	// LSW (MSB LSB)
				(((uint32_t)buf[2]) << 24) + (((uint32_t)buf[3]) << 16) ;	// LSW (MSB LSB)
		return true;
	}
	return false;
}

bool ET112::ReadRegister(AddressType addressToRead, uint32_t *buffer, size_t length) noexcept
{
	uint8_t bufferBytes[length*4];  // Let's create a buffer with the right size.
	if(ReadAddress(addressToRead,bufferBytes,sizeof(bufferBytes)))
	{
		for(int indexResponse = 0; indexResponse < (int)length ; ++indexResponse)
		{
			buffer[indexResponse] =
					(((uint32_t)bufferBytes[indexResponse*4 + 0]) << 8)  +	// LSW (MSB)
					 ((uint32_t)bufferBytes[indexResponse*4 + 1])        + 	// LSW (LSB)
					(((uint32_t)bufferBytes[indexResponse*4 + 2]) << 24) +	// LSW (MSB)
					(((uint32_t)bufferBytes[indexResponse*4 + 3]) << 16) ;	// LSW (LSB)
		}
		return true;
	}
	return false;
}

bool ET112::ReadAddress(AddressType addressToRead, uint8_t *buf, size_t length) noexcept
{
	uint16_t amountOfWords = (uint16_t)length/2;
	uint8_t bufToSend[sizeof(RequestMessage)];
	bufToSend[0] = _deviceAddress;
	bufToSend[1] = (uint8_t)MessagesType::readHoldingRegister;
	bufToSend[2] = (uint8_t)( (uint16_t)addressToRead >> 8);		// MSB First for memory Address
	bufToSend[3] = (uint8_t)( (uint16_t)addressToRead & 0x00FF);	// LSB Last for memory Address
	bufToSend[4] = (uint8_t)( amountOfWords >> 8);				// MSB First for Amount of words
	bufToSend[5] = (uint8_t)( amountOfWords & 0x00FF);			// LSB Last for Amount of words
	uint16_t crc = CalcCRC(bufToSend,6);
	bufToSend[6] = (uint8_t)(crc & 0x00FF);					// LSB First for CRC
	bufToSend[7] = (uint8_t)(crc >> 8);						// MSB Last for CRC

	// Send the Requests
	_rs485->write(bufToSend, sizeof(bufToSend));

	int8_t bufToReadIndex = 0;
	uint8_t bufToRead[length + 5];			// 5 bytes for CRC and Header

	// TODO: Improve the RX with Callback
	uint32_t startTime = millis();
	// For the timeout it is important to consider the length of the frame. As each byte needs almost 1ms:
	uint32_t timeout_Ms = (uint32_t)MaxResponseTime_Ms + (uint32_t)length + 5;
	do{
		delay(5);
		while(_rs485->available())
		{
			bufToRead[bufToReadIndex] = (uint8_t) _rs485->read();
			//debugPowerMeter("0x%02X ",bufToRead[bufToReadIndex]);
			bufToReadIndex++;
		}
	}while( ((millis() - startTime) < timeout_Ms) && (bufToReadIndex != ((int8_t)length + 5)) );
	//debugPowerMeter("\r\n");
	if((millis() - startTime >= timeout_Ms) && bufToReadIndex != ((int8_t)length + 5))
	{
		debugPowerMeter("Timeout\r\n");
		delay(100);
		return false;
	}
	else if(bufToReadIndex != ((int8_t)length + 5))
	{
		debugPowerMeter("Response Length not valid\r\n");
		delay(100);
		return false;
	}
	else if(!CheckCRC(bufToRead,sizeof(bufToRead)))
	{
		debugPowerMeter("Not valid CRC\r\n");
		delay(100);
		return false;
	}

	// Copy the data into the return buffer
	memcpy(buf, &bufToRead[3],length);
	// std::copy_n(&bufToRead[3],length,buf);

	delay(MinDelayBetweenFrames_Ms);	// Minimum Delay between frames
	return true;
}

#endif

// End
