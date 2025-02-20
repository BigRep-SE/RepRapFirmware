/*
 * RtdSensor31865.cpp
 *
 *  Created on: 8 Jun 2017
 *      Author: David
 */

#include "RtdSensor31865.h"

#if SUPPORT_SPI_SENSORS

#include <Platform/RepRap.h>
#include <GCodes/GCodeBuffer/GCodeBuffer.h>

#if SUPPORT_REMOTE_COMMANDS
# include <CanMessageGenericParser.h>
#endif

const uint32_t MAX31865_Frequency = 4000000;	// maximum for MAX31865 is 5MHz

// SPI modes:
// If the inactive state of SCL is LOW (CPOL = 0) (in the case of the MAX31865, this is sampled on the falling edge of CS):
// The MAX31865 changes data after the rising edge of CLK, and samples input data on the falling edge.
// This requires NCPHA = 0.
const SpiMode MAX31865_SpiMode = SPI_MODE_1;

// Define the minimum interval between readings. The MAX31865 needs 62.5ms in 50Hz filter mode.
const uint32_t MinimumReadInterval = 100;		// minimum interval between reads, in milliseconds

// Default configuration register
// Note that to get the MAX31865 to do continuous conversions, we need to set the bias bit as well as the continuous-conversion bit
#if SUPPORT_3WIRES_RTD
//  Vbias=1
//  Conversion mode=1
//	1shot = 0
//	3wire=1
//	Fault detection=00 no action
//	Fault status=1 clear any existing fault
//	50/60Hz reject=1 for 50Hz (0 for 60Hz)
const uint8_t DefaultCr0 = 0b11010011;
#else
//  Vbias=1
//  Conversion mode=1
//	1shot = 0
//	3wire=0
//	Fault detection=00 no action
//	Fault status=1 clear any existing fault
//	50/60Hz reject=1 for 50Hz (0 for 60Hz)
const uint8_t DefaultCr0 = 0b11000011;
#endif
const uint8_t Cr0ReadMask = 0b11011101;		// bits 1 and 5 auto clear, so ignore the value read

const uint32_t DefaultRef = 400;

// Macro to build a standard lambda function that includes the necessary type conversions
#define OBJECT_MODEL_FUNC(...)					OBJECT_MODEL_FUNC_BODY(RtdSensor31865, __VA_ARGS__)
#define OBJECT_MODEL_FUNC_IF(_condition, ...)	OBJECT_MODEL_FUNC_IF_BODY(RtdSensor31865, _condition, __VA_ARGS__)

constexpr ObjectModelTableEntry RtdSensor31865::objectModelTable[] =
{
	{ "rRef",	OBJECT_MODEL_FUNC(self->rrefTimes100/100), 	ObjectModelEntryFlags::none }
};

constexpr uint8_t RtdSensor31865::objectModelTableDescriptor[] = { 1, 1 };

DEFINE_GET_OBJECT_MODEL_TABLE_WITH_PARENT(RtdSensor31865, SensorWithPort)

// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor RtdSensor31865::typeDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new RtdSensor31865(sensorNum); } );

RtdSensor31865::RtdSensor31865(unsigned int sensorNum) noexcept
	: SpiTemperatureSensor(sensorNum, "PT100 (MAX31865)", MAX31865_SpiMode, MAX31865_Frequency),
	  rrefTimes100(DefaultRef * 100), cr0(DefaultCr0)
{
	sum = 0;
	index = 0;
	windowSize = 0;
	reqWindowSize = 0;
	for (size_t i = 0; i < Pt100MaxAverageReadings; ++i)
	{
		readings[i] = 0;
	}
}

// Configure this temperature sensor
GCodeResult RtdSensor31865::Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException)
{
	if (!ConfigurePort(gb, reply, changed))
	{
		return GCodeResult::error;
	}
	ConfigureCommonParameters(gb, changed);
	if (gb.Seen('F'))
	{
		changed = true;
		if (gb.GetIValue() == 60)
		{
			cr0 &= ~0x01;		// set 60Hz rejection
		}
		else
		{
			cr0 |= 0x01;		// default to 50Hz rejection
		}
	}

	if (gb.Seen('W'))
	{
		changed = true;
		if (gb.GetUIValue() == 3)
		{
			cr0 |= 0x10;		// 3 wire configuration
		}
		else
		{
			cr0 &= ~0x10;		// 2 or 4 wire configuration
		}
	}

	if (gb.Seen('R'))
	{
		changed = true;
		rrefTimes100 = lrintf(gb.GetPositiveFValue() * 100);
	}

	if (gb.Seen('K'))
	{
		reqWindowSize = GetSamplesFromSeconds(gb.GetLimitedUIValue('K', GetSecondsFromSamples(Pt100MaxAverageReadings)+1));
	}

	return FinishConfiguring(changed, reply);
}

#if SUPPORT_REMOTE_COMMANDS

GCodeResult RtdSensor31865::Configure(const CanMessageGenericParser& parser, const StringRef& reply) noexcept
{
	bool seen = false;
	if (!ConfigurePort(parser, reply, seen))
	{
		return GCodeResult::error;
	}

	uint8_t paramF;
	if (parser.GetUintParam('F', paramF))
	{
		seen = true;
		if (paramF == 60)
		{
			cr0 &= ~0x01;		// set 60Hz rejection
		}
		else
		{
			cr0 |= 0x01;		// default to 50Hz rejection
		}
	}

	uint8_t paramW;
	if (parser.GetUintParam('W', paramW))
	{
		seen = true;
		if (paramW == 3)
		{
			cr0 |= 0x10;		// 3 wire configuration
		}
		else
		{
			cr0 &= ~0x10;		// 2 or 4 wire configuration
		}
	}

	float paramR;
	if (parser.GetFloatParam('R', paramR))
	{
		seen = true;
		rrefTimes100 = lrintf(paramR * 100);
	}

	uint32_t paramK;
	if (parser.GetUintParam('K', paramK))
	{
		paramK = GetSamplesFromSeconds(paramK);
		reqWindowSize = (paramK > Pt100MaxAverageReadings) ? Pt100MaxAverageReadings : paramK;
	}

	return FinishConfiguring(seen, reply);
}

#endif

GCodeResult RtdSensor31865::FinishConfiguring(bool changed, const StringRef& reply) noexcept
{
	if (changed)
	{
		// Initialise the sensor
		InitSpi();

		TemperatureError rslt(TemperatureError::unknownError);
		for (unsigned int i = 0; i < 3; ++i)		// try 3 times
		{
			rslt = TryInitRtd();
			if (rslt == TemperatureError::ok)
			{
				break;
			}
			delay(MinimumReadInterval);
		}

		SetResult(0.0, rslt);
		if (rslt != TemperatureError::ok)
		{
			reply.printf("Failed to initialise RTD: %s", rslt.ToString());
			return GCodeResult::error;
		}
	}
	else
	{
		CopyBasicDetails(reply);
		reply.catf(", %s wires, reject %dHz, reference resistor %.2f ohms", ((cr0 & 0x10u) != 0) ? "3" : "2/4", ((cr0 & 0x01u) != 0) ? 50 : 60, (double)((float)rrefTimes100 * 0.01));
		if (reqWindowSize > 0)
		{
			reply.catf(", moving average filter: %lu sec", GetSecondsFromSamples(reqWindowSize));
		}
		else
		{
			reply.catf(", moving average filter: disabled");
		}
	}
	return GCodeResult::ok;
}

// Try to initialise the RTD
TemperatureError RtdSensor31865::TryInitRtd() const noexcept
{
	const uint8_t modeData[2] = { 0x80, cr0 };			// write register 0
	uint32_t rawVal;
	TemperatureError sts = DoSpiTransaction(modeData, ARRAY_SIZE(modeData), rawVal);

	if (sts == TemperatureError::ok)
	{
		delayMicroseconds(1);
		static const uint8_t readData[2] = { 0x00, 0x00 };	// read register 0
		sts = DoSpiTransaction(readData, ARRAY_SIZE(readData), rawVal);
	}

	//debugPrintf("Status %d data %04x\n", (int)sts, (uint16_t)rawVal);
	if (sts == TemperatureError::ok && (rawVal & Cr0ReadMask) != ((uint32_t)cr0 & Cr0ReadMask))
	{
		sts = TemperatureError::badResponse;
	}

	return sts;
}

uint32_t RtdSensor31865::GetSamplesFromSeconds(uint32_t s) const noexcept
{
    return (s*1000)/HeatSampleIntervalMillis;
}

uint32_t RtdSensor31865::GetSecondsFromSamples(uint32_t s) const noexcept
{
    return (s*HeatSampleIntervalMillis)/1000;
}

// Call this to put a new reading into the filter and update the sum
uint16_t RtdSensor31865::UpdateFilter(uint16_t r, uint16_t w) noexcept
{
	size_t head = index;		// Avoid reloading volatile variable

	// Filter is disabled
	if (w == 0)
	{
		if (windowSize != 0)	// Reset filter only if before was activated
		{
			sum = 0;
			index = 0;
			windowSize = 0;
			for (size_t i = 0; i < Pt100MaxAverageReadings; ++i)
			{
				readings[i] = 0;
			}
		}
		return r;
    }

	// Reducing window
    if (w <= windowSize)
    {
    	size_t lastWindowElementIndex = head + (Pt100MaxAverageReadings - windowSize);
    	size_t diffWindow = windowSize - w;
        windowSize = w;
        // Remove from the sum the oldest values of the window up to the new size (or just the oldest one if the size doesn't change)
        for (size_t i = 0; i <= diffWindow; ++i)
        {
            sum -= readings[(lastWindowElementIndex + i) % Pt100MaxAverageReadings];
        }
    }

	// Increasing window
	if (windowSize < w)
	{
		windowSize++;
	}

	// Insert the new value, update sum and move index
	readings[head] = r;
	sum += r;
	index = (head + 1) % Pt100MaxAverageReadings;

	// Compute and return the average when buffer is full
	return (windowSize == w) ? (sum / w) : r;
}

void RtdSensor31865::Poll() noexcept
{
	static const uint8_t dataOut[4] = {0, 0x55, 0x55, 0x55};			// read registers 0 (control), 1 (MSB) and 2 (LSB)
	uint32_t rawVal;
	TemperatureError sts = DoSpiTransaction(dataOut, ARRAY_SIZE(dataOut), rawVal);

	if (sts != TemperatureError::ok)
	{
		SetResult(sts);
	}
	else
	{
		if (   (((rawVal >> 16) & Cr0ReadMask) != ((uint32_t)cr0 & Cr0ReadMask))	// if control register not as expected
			|| (rawVal & 1) != 0										// or fault bit set
		   )
		{
			static const uint8_t faultDataOut[2] = {0x07, 0x55};
			if (DoSpiTransaction(faultDataOut, ARRAY_SIZE(faultDataOut), rawVal) == TemperatureError::ok)	// read the fault register
			{
				sts = ((rawVal & 0x04) != 0) ? TemperatureError::overOrUnderVoltage
							: ((rawVal & 0x18) != 0) ? TemperatureError::openCircuit
								: TemperatureError::hardwareError;
			}
			else
			{
				sts = TemperatureError::hardwareError;
			}
			SetResult(sts);
			delayMicroseconds(1);										// MAX31865 requires CS to be high for 400ns minimum
			TryInitRtd();												// clear the fault and hope for better luck next time
		}
		else
		{
			uint16_t ohmsx100 = (uint16_t)((((rawVal >> 1) & 0x7FFF) * rrefTimes100) >> 15);
			ohmsx100 = UpdateFilter(ohmsx100, reqWindowSize);
			float t;
			sts = GetPT100Temperature(t, ohmsx100);
			SetResult(t, sts);
		}
	}
}

#endif // SUPPORT_SPI_SENSORS

// End
