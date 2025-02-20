#ifndef SRC_HEATING_SENSORS_EMULATEDSENSOR_H_
#define SRC_HEATING_SENSORS_EMULATEDSENSOR_H_

/**
 * This class implements the emulated temperature for bed, cbc, and the extruders
 */

#include "TemperatureSensor.h"
#include "CanMessageGenericParser.h"

class EmulatedSensor : public TemperatureSensor
{
public:

	static constexpr const char *_ecv_array TypeName = "emu-sensor";

	explicit EmulatedSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;

	const char *_ecv_array GetShortSensorType() const noexcept override { return TypeName; };

	GCodeResult Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException) override;

	void SetCurrentPwm(float pwm) noexcept;

private:

	static SensorTypeDescriptor emulatedSensorDescriptor;

	void UpdateEmulatedTemperature()noexcept;

	float currentPwm 	= 0.0f;
	float currentTemp 	= 25.0f;
	float tempMax 		= 100.0f;
	inline static constexpr float TempAmb = 25.0f;
	float tauRise 		= 20.0f;
	float tauFall 		= 60.0f;

	uint32_t prevTime = 0;

};
#endif
