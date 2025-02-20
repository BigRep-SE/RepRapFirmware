/*
 * EtherCATMonitor.h
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#ifndef SRC_HARDWARE_NETX_ETHERCATMONITOR_H_
#define SRC_HARDWARE_NETX_ETHERCATMONITOR_H_

#include "Heating/Sensors/TemperatureSensor.h"

#if HAS_ETHERCAT_MONITOR
#include "Movement/StepTimer.h"

class EtherCATTxMonitor : public TemperatureSensor
{
public:
	EtherCATTxMonitor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "ecattx";
private:
	using TimeMs = uint32_t;
	TimeMs lastPollTimeMs = 0;

	static SensorTypeDescriptor etherCATTxMonitorDescriptor;
};

class EtherCATRxMonitor : public TemperatureSensor
{
public:
	EtherCATRxMonitor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "ecatrx";
private:
	using TimeMs = uint32_t;
	TimeMs lastPollTimeMs = 0;

	static SensorTypeDescriptor etherCATRxMonitorDescriptor;

};


#endif

#endif /* SRC_HEATING_SENSORS_CPUTEMPERATURESENSOR_H_ */
