#include "PowerMeter.h"

#if SUPPORT_POWERMETER || SUPPORT_EMUPOWERMETER

#include <Platform/Platform.h>
#include <Platform/RepRap.h>
#include <RTOSIface/RTOSIface.h>
#include <Platform/TaskPriorities.h>
#include "ET112.h"
#include "EmuPowerMeter.h"

constexpr size_t PowerMeterTaskStackWords = 400;			// big enough to handle printf and file writes
static Task<PowerMeterTaskStackWords> *powerMeterTask;

constexpr uint32_t PowerMeterSleep = 1000;					// [ms] Sleep between reads.

EnergyMeter * energyMeter = nullptr;

void debugPowerMeter(const char* fmt, ...) noexcept
{
/*
#ifdef DEBUG
	// Calls to debugPrintf() from with ISRs are unsafe, both because of timing issues and because the call to Platform::MessageF tries to acquire a mutex.
	// So ignore the call if we are coming from within an ISR.
	if (!inInterrupt())
	{
		va_list vargs;
		va_start(vargs, fmt);
		reprap.GetPlatform().DebugMessage(fmt, vargs);
		va_end(vargs);
	}
#endif
*/
}

[[noreturn]] void PowerMeterTaskCode(void*) noexcept
{
	// Creation of the ET112 power Meter
#if SUPPORT_POWERMETER
	energyMeter = new ET112(&SERIAL_RS485_DEVICE);
#elif SUPPORT_EMUPOWERMETER
	energyMeter = new EmuPowerMeter();
#endif

	debugPowerMeter("PowerMeter Init\r\n");
	energyMeter->Init();

	debugPowerMeter("PowerMeter Starting\r\n");

	while(1)
	{
		energyMeter->Poll();

		delay(PowerMeterSleep);
	}

}

bool PowerMeter::IsConnected() noexcept
{
	return energyMeter->IsConnected();
}

uint32_t PowerMeter::GetWhenLastRead() noexcept
{
	return energyMeter->GetWhenLastRead();
}


const EnergyMeter::Values & PowerMeter::GetValues() noexcept
{
	return energyMeter->GetValues();
}

void PowerMeter::ConfigurePowerMeter() noexcept
{
	powerMeterTask = new Task<PowerMeterTaskStackWords>;
	powerMeterTask->Create(PowerMeterTaskCode, "POWMET", nullptr, TaskPriority::PowerMeterPriority);
}

#endif

// End
