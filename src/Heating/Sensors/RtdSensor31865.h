/*
 * RtdSensor31865.h
 *
 *  Created on: 8 Jun 2017
 *      Author: David
 */

#ifndef SRC_HEATING_RTDSENSOR31865_H_
#define SRC_HEATING_RTDSENSOR31865_H_

#include "SpiTemperatureSensor.h"
#include <Platform/Platform.h>

#if SUPPORT_SPI_SENSORS

class RtdSensor31865 : public SpiTemperatureSensor
{
public:
	explicit RtdSensor31865(unsigned int sensorNum) noexcept;

	GCodeResult Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException) override;

#if SUPPORT_REMOTE_COMMANDS
	GCodeResult Configure(const CanMessageGenericParser& parser, const StringRef& reply) noexcept override; // configure the sensor from M308 parameters
#endif

	void Poll() noexcept override;
	const char *_ecv_array GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *_ecv_array TypeName = "rtdmax31865";

protected:
	DECLARE_OBJECT_MODEL

private:
	static SensorTypeDescriptor typeDescriptor;

	TemperatureError TryInitRtd() const noexcept;
	GCodeResult FinishConfiguring(bool changed, const StringRef& reply) noexcept;

	uint32_t rrefTimes100;				// reference resistor in units of 0.01 ohms
	uint8_t cr0;

	// Filter
	uint16_t UpdateFilter(uint16_t r, uint16_t s) noexcept;
	uint32_t GetSamplesFromSeconds(uint32_t s) const noexcept;
	uint32_t GetSecondsFromSamples(uint32_t s) const noexcept;

	size_t index;
	uint32_t windowSize;											// Current number of samples use in the average
	uint32_t reqWindowSize;											// New size of the window requested by M308 K parameter
	uint32_t sum;													// Sum of all the elements in the buffer
	uint16_t readings[Pt100MaxAverageReadings];						// Filter buffer
};

#endif //SUPPORT_SPI_SENSORS

#endif /* SRC_HEATING_RTDSENSOR31865_H_ */
