#ifndef SRC_POWERMETER_EMUPOWERMETER_H_
#define SRC_POWERMETER_EMUPOWERMETER_H_

#include <RepRapFirmware.h>

#if SUPPORT_EMUPOWERMETER

#include "EnergyMeter.h"

class EmuPowerMeter : public EnergyMeter
{
private:
	static const uint32_t DefaultPowerMeterBaudRate = 9600;		// Default baudrate
	static const uint8_t DefaultSlaveAddress = 1;				// Default slave address

	static const uint16_t DeviceIdentificationCode 	= 120;		// ET112-DIN AV0 1 x S1 X
	static const uint16_t MaxResponseTime_Ms 		= 510;		// [msec] Max Response time
	static const uint16_t MinDelayBetweenFrames_Ms 	= 5;		// [msec] Min Delay between frames

public:
	EmuPowerMeter() noexcept;
	// Do a quick test to check whether the power meter is present, returning true if it is
	void Init() noexcept override {};

	bool IsConnected() const noexcept override { return true; };

	void Poll() noexcept override;

	const char *GetMeterType() const noexcept override {return "EmuPowerMeter";};
private:
	uint64_t totalMillis {0};
	uint64_t totalPowerWatts {0};
};

#endif

#endif /* SRC_POWERMETER_EMUPOWERMETER_H_ */
