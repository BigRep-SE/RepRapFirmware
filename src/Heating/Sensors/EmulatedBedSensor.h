#ifndef SRC_HEATING_SENSORS_EMULATEDBEDSENSOR_H_
#define SRC_HEATING_SENSORS_EMULATEDBEDSENSOR_H_

/**
 * This class implements the emulated temperature for bed, cbc, and the extruders
 */

#include "TemperatureSensor.h"
#include "CanMessageGenericParser.h"
#include <Platform/Platform.h>
#include <GCodes/GCodeBuffer/GCodeBuffer.h>
#include <Tools/Tool.h>
#include <Endstops/ZProbe.h>
#include <Platform/TaskPriorities.h>
#include <Platform/RepRap.h>
#include <Movement/Move.h>

class EmulatedBedSensor : public TemperatureSensor
{
public:

	static constexpr const char *_ecv_array TypeName = "emulinear";

	explicit EmulatedBedSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;

	const char *_ecv_array GetShortSensorType() const noexcept override { return TypeName; };

	GCodeResult Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException) override;

private:

	static SensorTypeDescriptor emulatedBedSensorDescriptor;

	int32_t RandomGenerator(uint32_t range) noexcept;
	float NoiseGenerator(float amplitude) noexcept;

	void UpdateEmulatedTemperature()noexcept;

	inline static uint32_t seed = 134775813; // Static to shared the seed when multiple declarations

	float noiseAmplitude = 10.0f;

	// Define the normal vector of the non-flat bed plane
	float xInclination = 0.0005;
	float yInclination = 0.0003;
	float xOffset = 500;	// [mm] Offset in X
	float yOffset = 250;	// [mm] Offset in X
	float zOffset = -4;		// [mm] Offset in Z
	float zOffsetDef = -4;		// [mm] Offset in Z
	float aParabola = 1000;
	float bParabola = 1200;
	float maxValue = 1800;	// [um] Max value that the sensor may return
	float minValue = -2400;	// [um] Min value that the sensor may return
	bool sensorReferenced = false;
};
#endif
