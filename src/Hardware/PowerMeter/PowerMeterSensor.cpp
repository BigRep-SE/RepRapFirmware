#include <Platform/Platform.h>
#include <Platform/RepRap.h>
#include <Hardware/PowerMeter/PowerMeterSensor.h>

#if SUPPORT_POWERMETER || SUPPORT_EMUPOWERMETER
#include <Hardware/PowerMeter/PowerMeter.h>
#include <GCodes/GCodeBuffer/GCodeBuffer.h>

// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor PowerMeterVoltageSensor::powerMeterVoltageSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterVoltageSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterCurrentSensor::powerMeterCurrentSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterCurrentSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterPowerWSensor::powerMeterPowerWSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterPowerWSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyPartialkWhSensor::powerMeterEnergyPartialkWhSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyPartialkWhSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyPartialkVARhSensor::powerMeterEnergyPartialkVARhSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyPartialkVARhSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyPartialkWhT1Sensor::powerMeterEnergyPartialkWhT1SensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyPartialkWhT1Sensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyPartialkWhT2Sensor::powerMeterEnergyPartialkWhT2SensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyPartialkWhT2Sensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyTotalkWhPositiveSensor::powerMeterEnergyTotalkWhPositiveSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyTotalkWhPositiveSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyTotalkWhNegativeSensor::powerMeterEnergyTotalkWhNegativeSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyTotalkWhNegativeSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyTotalkVARhPositiveSensor::powerMeterEnergyTotalkVARhPositiveSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyTotalkVARhPositiveSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterEnergyTotalkVARhNegativeSensor::powerMeterEnergyTotalkVARhNegativeSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterEnergyTotalkVARhNegativeSensor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor PowerMeterHoursSensor::powerMeterHoursSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new PowerMeterHoursSensor(sensorNum); } );


GCodeResult PowerMeterFilteredSensor::Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed)
{
	GCodeResult ret = TemperatureSensor::Configure(gb,reply, changed);
	if(ret == GCodeResult::ok)
	{
		ret = ConfigureFilter(gb, reply, changed);
		if(!changed  && !gb.Seen('Y'))
		{
			reply.catf(", %sfiltered, C = %.2f", (kIIR <= MaxK) ? "" : "un", (double)kIIR);
		}
	}
	return ret;
};

GCodeResult PowerMeterFilteredSensor::ConfigureFilter(GCodeBuffer& gb, const StringRef& reply, bool& changed)
{
	float k = 1.0f;
	gb.TryGetFValue('C', k, changed);
	if (changed)
	{
		SetK(k);
	}
	return GCodeResult::ok;
}

PowerMeterVoltageSensor::PowerMeterVoltageSensor(unsigned int sensorNum) noexcept : PowerMeterFilteredSensor(sensorNum)
{
}

void PowerMeterVoltageSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(FilterValue(PowerMeter::GetValues().voltage_V, PowerMeter::GetWhenLastRead()), TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterCurrentSensor::PowerMeterCurrentSensor(unsigned int sensorNum) noexcept : PowerMeterFilteredSensor(sensorNum)
{
}

void PowerMeterCurrentSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(FilterValue(PowerMeter::GetValues().current_A, PowerMeter::GetWhenLastRead()), TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterPowerWSensor::PowerMeterPowerWSensor(unsigned int sensorNum) noexcept : PowerMeterFilteredSensor(sensorNum)
{
}

void PowerMeterPowerWSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(FilterValue(PowerMeter::GetValues().energy_W, PowerMeter::GetWhenLastRead()), TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyPartialkWhSensor::PowerMeterEnergyPartialkWhSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyPartialkWhSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyPartial_kWh, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyPartialkVARhSensor::PowerMeterEnergyPartialkVARhSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyPartialkVARhSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyPartial_kVARh, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyPartialkWhT1Sensor::PowerMeterEnergyPartialkWhT1Sensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyPartialkWhT1Sensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyPartial_kWhT1, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyPartialkWhT2Sensor::PowerMeterEnergyPartialkWhT2Sensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyPartialkWhT2Sensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyPartial_kWhT2, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyTotalkWhPositiveSensor::PowerMeterEnergyTotalkWhPositiveSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyTotalkWhPositiveSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyTotal_kWh_P, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyTotalkWhNegativeSensor::PowerMeterEnergyTotalkWhNegativeSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyTotalkWhNegativeSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyTotal_kWh_N, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyTotalkVARhPositiveSensor::PowerMeterEnergyTotalkVARhPositiveSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyTotalkVARhPositiveSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyTotal_kVARh_P, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterEnergyTotalkVARhNegativeSensor::PowerMeterEnergyTotalkVARhNegativeSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterEnergyTotalkVARhNegativeSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().energyTotal_kVARh_N, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

PowerMeterHoursSensor::PowerMeterHoursSensor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "")
{
}

void PowerMeterHoursSensor::Poll() noexcept
{
	if(PowerMeter::IsConnected())
	{
		SetResult(PowerMeter::GetValues().hours_H, TemperatureError::ok);
	}
	else
	{
		SetResult( 0.0f , TemperatureError::timeout);
	}
}

#endif

// End
