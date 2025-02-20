#ifndef SRC_POWERMETER_POWERMETER_H_
#define SRC_POWERMETER_POWERMETER_H_

#include <RepRapFirmware.h>

#if SUPPORT_POWERMETER || SUPPORT_EMUPOWERMETER

#include "EnergyMeter.h"

namespace PowerMeter
{
	void ConfigurePowerMeter() noexcept;
	bool IsConnected() noexcept;
	uint32_t GetWhenLastRead()  noexcept;
	const EnergyMeter::Values & GetValues() noexcept;
}

#endif

#endif /* SRC_ACCELEROMETERS_ACCELEROMETERS_H_ */
