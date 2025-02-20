#ifndef SRC_POWERMETER_POWERMETERSENSOR_H_
#define SRC_POWERMETER_POWERMETERSENSOR_H_

#include "Heating/Sensors/TemperatureSensor.h"

#if SUPPORT_POWERMETER || SUPPORT_EMUPOWERMETER

#include "GCodes/GCodeBuffer/GCodeBuffer.h"

// TODO: Create a single class with all the sensors.

class PowerMeterFilteredSensor : public TemperatureSensor { // IIR First order Low-Pass filter

public:
	PowerMeterFilteredSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "") {}

	GCodeResult Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException) override;

protected:
	float FilterValue( float input )
	{
		value = ( (kIIR >= MaxK) ?
				input : // Filter is disabled
				value * (1.0f - kIIR) + input * kIIR );
		return value ;
	};
	float FilterValue( float input, uint32_t whenRead )
	{
		if(lastUpdated == 0 && value == 0.0f)
		{
			// For faster init
			value = input;
			lastUpdated = whenRead;
		}
		else if(lastUpdated != whenRead)
		{
			value = ( (kIIR >= MaxK) ?
					input : // Filter is disabled
					value * (1.0f - kIIR) + input * kIIR );
			lastUpdated = whenRead;
		}
		return value ;
	};
private:

	static constexpr float MinK = 0.01f;
	static constexpr float MaxK = 0.99f;

	GCodeResult ConfigureFilter(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException);

	void SetK(float k)
	{
		if(k > MaxK) {
			k = 1.0f; // No filter
		}
		else if( k < MinK)
		{
			k = MinK;
		}
		kIIR = k;
	};

	float kIIR = 1.0f; 	// k value of First Order IIR filter. Default is IIR filter is disabled.
	float value = 0.0f;
	uint32_t lastUpdated = 0;
};

class PowerMeterVoltageSensor : public PowerMeterFilteredSensor
{
public:
	PowerMeterVoltageSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmvolt";

private:
	static SensorTypeDescriptor powerMeterVoltageSensorDescriptor;
};

class PowerMeterCurrentSensor : public PowerMeterFilteredSensor
{
public:
	PowerMeterCurrentSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmamp";

private:
	static SensorTypeDescriptor powerMeterCurrentSensorDescriptor;
};

class PowerMeterPowerWSensor : public PowerMeterFilteredSensor
{
public:
	PowerMeterPowerWSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmwatt";

private:
	static SensorTypeDescriptor powerMeterPowerWSensorDescriptor;
};

class PowerMeterEnergyPartialkWhSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyPartialkWhSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkwh";

private:
	static SensorTypeDescriptor powerMeterEnergyPartialkWhSensorDescriptor;
};

class PowerMeterEnergyPartialkVARhSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyPartialkVARhSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkvarh";

private:
	static SensorTypeDescriptor powerMeterEnergyPartialkVARhSensorDescriptor;
};

class PowerMeterEnergyPartialkWhT1Sensor : public TemperatureSensor
{
public:
	PowerMeterEnergyPartialkWhT1Sensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkwh1";

private:
	static SensorTypeDescriptor powerMeterEnergyPartialkWhT1SensorDescriptor;
};

class PowerMeterEnergyPartialkWhT2Sensor : public TemperatureSensor
{
public:
	PowerMeterEnergyPartialkWhT2Sensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkwh2";

private:
	static SensorTypeDescriptor powerMeterEnergyPartialkWhT2SensorDescriptor;
};

class PowerMeterEnergyTotalkWhPositiveSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyTotalkWhPositiveSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkwhp";

private:
	static SensorTypeDescriptor powerMeterEnergyTotalkWhPositiveSensorDescriptor;
};

class PowerMeterEnergyTotalkWhNegativeSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyTotalkWhNegativeSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkwhn";

private:
	static SensorTypeDescriptor powerMeterEnergyTotalkWhNegativeSensorDescriptor;
};

class PowerMeterEnergyTotalkVARhPositiveSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyTotalkVARhPositiveSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkvarhp";

private:
	static SensorTypeDescriptor powerMeterEnergyTotalkVARhPositiveSensorDescriptor;
};

class PowerMeterEnergyTotalkVARhNegativeSensor : public TemperatureSensor
{
public:
	PowerMeterEnergyTotalkVARhNegativeSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmkvarhn";

private:
	static SensorTypeDescriptor powerMeterEnergyTotalkVARhNegativeSensorDescriptor;
};

class PowerMeterHoursSensor : public TemperatureSensor
{
public:
	PowerMeterHoursSensor(unsigned int sensorNum) noexcept;

	void Poll() noexcept override;
	const char *GetShortSensorType() const noexcept override { return TypeName; }

	static constexpr const char *TypeName = "pmhours";

private:
	static SensorTypeDescriptor powerMeterHoursSensorDescriptor;
};


#endif

#endif /* SRC_POWERMETER_POWERMETERSENSOR_H_ */
