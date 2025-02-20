#include "EmuPowerMeter.h"

#if SUPPORT_EMUPOWERMETER

#include <Hardware/IoPorts.h>

#include <Platform/RepRap.h>
#include "Heating/Heat.h"

#include "Heating/Sensors/EmulatedNoise.h"

extern void debugPowerMeter(const char* fmt, ...) noexcept;

EmuPowerMeter::EmuPowerMeter() noexcept {

}

void EmuPowerMeter::Poll() noexcept
{
	debugPowerMeter("EmuPowerMeter Device Present\r\n") ;

	const auto millisSinceLast = millis() - whenLastRead ;

	// Basic values
	values.voltage_V = 220.0f + EmulatedNoise::PeakToPeak(1.0f);
	values.current_A = 0.7f + EmulatedNoise::PeakToPeak(0.15f);

	// Most of the power is due to the bed and the CBC
	const int BedHeater = reprap.GetHeat().GetBedHeater(0);
	constexpr float BedCurrent { 5.0f };
	if (BedHeater >= 0)
	{
		values.current_A += reprap.GetHeat().GetAveragePWM(BedHeater) * BedCurrent ;
	}
	const int CbCHeater = reprap.GetHeat().GetChamberHeater(0);
	constexpr float CbcCurrent { 3.7f };
	if (CbCHeater >= 0)
	{
		values.current_A += reprap.GetHeat().GetAveragePWM(CbCHeater) * CbcCurrent ;
	}
	values.energy_W  = values.voltage_V * values.current_A;
	if(millisSinceLast > 0){
		totalPowerWatts += static_cast<uint64_t>(values.energy_W * 1000.0f ) / ( millisSinceLast);
		const float kWh = static_cast<float>(totalPowerWatts) / (1000.f * 3600.0f);
		values.energyPartial_kWh = kWh ;
		values.energyTotal_kWh_P = kWh ;
	}

	totalMillis += millisSinceLast;
	values.hours_H = static_cast<float>((totalMillis/1000u) & 0x7FFFFFFF) / 3600.0f ;

	whenLastRead = millis();
}

#endif

// End
