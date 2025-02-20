#ifndef SRC_POWERMETER_ENERGYMETER_H_
#define SRC_POWERMETER_ENERGYMETER_H_

#include <RepRapFirmware.h>
#include <Devices.h>

#if SUPPORT_POWERMETER || SUPPORT_EMUPOWERMETER

class EnergyMeter{

public:
	EnergyMeter() : whenLastRead(millis()) {};
	virtual ~EnergyMeter() = default;

	// Try to get a temperature reading
	virtual void Poll() noexcept = 0;
	virtual void Init() noexcept = 0;
	virtual bool IsConnected() const { return true; };
	virtual const char *GetMeterType() const noexcept = 0;

	// Expected values of a EnergyMeter.
	struct Values{
		float hours_H = 0.0, voltage_V = 0.0, current_A = 0.0, energy_W = 0.0;
		float energyPartial_kWh = 0.0, energyPartial_kVARh = 0.0, energyPartial_kWhT1 = 0.0, energyPartial_kWhT2 = 0.0;
		float energyTotal_kWh_P = 0.0, energyTotal_kWh_N = 0.0, energyTotal_kVARh_P = 0.0, energyTotal_kVARh_N = 0.0;
	} ;

	const Values& GetValues() const noexcept { return values; };
	uint32_t GetWhenLastRead() const noexcept { return whenLastRead; };

protected :
	uint32_t whenLastRead;
	struct Values values;
};

#endif

#endif /* SRC_POWERMETER_ENERGYMETER_H_ */
