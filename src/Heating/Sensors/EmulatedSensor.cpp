#include "EmulatedSensor.h"
#include <GCodes/GCodeBuffer/GCodeBuffer.h>
#include <Platform/RepRap.h>
#include <Platform/Platform.h>
#include "EmulatedNoise.h"
#include "CanMessageGenericParser.h"


// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor EmulatedSensor::emulatedSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new EmulatedSensor(sensorNum); } );

EmulatedSensor::EmulatedSensor(unsigned int sensorNum) noexcept
	: TemperatureSensor(sensorNum, "emusensor"){
}

void EmulatedSensor::UpdateEmulatedTemperature()noexcept
{
	const uint32_t currentTime = millis();
	const uint32_t sinceLastCall = currentTime - prevTime; // the time period since the last calculation

	const float pwmTon = (currentPwm * ((float)sinceLastCall))/1000.0f  ;
	const float pwmToff = (((float)sinceLastCall) - pwmTon)/1000.0f;
	const float tempCurRise = currentTemp + (tempMax - currentTemp ) *(1-(float)exp((double)(-(pwmTon)/tauRise)));

	currentTemp = TempAmb + (tempCurRise - TempAmb)*((float)exp((double)(-(pwmToff)/tauFall)));

	prevTime = currentTime;
}

void EmulatedSensor::SetCurrentPwm(float pwm) noexcept
{
	currentPwm = pwm;
}

void EmulatedSensor::Poll() noexcept
{
	UpdateEmulatedTemperature();
	//if (currentTemp > 24.0f) {
	SetResult( currentTemp  + EmulatedNoise::PeakToPeak(0.15f), TemperatureError::ok);
	//}

}

GCodeResult EmulatedSensor::Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException)
{
	gb.TryGetFValue('R', tauRise, changed);
	gb.TryGetFValue('F', tauFall, changed);
	gb.TryGetFValue('C', tempMax, changed);

	ConfigureCommonParameters(gb, changed);

	if (!changed)
	{
		CopyBasicDetails(reply);
		reply.catf(" R:%f F:%f C:%f", (double)tauRise, (double)tauFall, (double)tempMax);
	}

	return GCodeResult::ok;

}
