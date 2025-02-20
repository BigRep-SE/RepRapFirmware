/*
 * EtherCATMonitor.cpp
 *
 *  Created on: 29 May 2022
 *      Author: BigRep
 */

#include "EtherCATMonitor.h"
#include <Platform/Platform.h>
#include <Platform/RepRap.h>

#if HAS_ETHERCAT_MONITOR
#include "NetX.h"
#include "NetXSerial.h"
#include "NetXSocket.h"

// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor EtherCATTxMonitor::etherCATTxMonitorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new EtherCATTxMonitor(sensorNum); } );
TemperatureSensor::SensorTypeDescriptor EtherCATRxMonitor::etherCATRxMonitorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new EtherCATRxMonitor(sensorNum); } );


constexpr float EtherCATMaxAmountOfBytesPerSec = 125000.0f;

using Ticks = StepTimer::Ticks;

EtherCATTxMonitor::EtherCATTxMonitor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "EtherCAT Monitor Tx Buffer sensor")
{

}

void EtherCATTxMonitor::Poll() noexcept
{
	float perc = 0.0f;
	const TimeMs currentTimeMs = millis();
	const float timeSinceLastPollSec = ((float)(currentTimeMs - lastPollTimeMs)) / 1000.0f  ;
	if(timeSinceLastPollSec > 0.0f)
	{
		const float totalBytes = (float)(NetXSerial::GetInstance().GetTotalBytesTransmittedAndClear() + NetXSocket::GetInstance().GetTotalBytesTransmittedAndClear());
		const float totalBytesPerSec = totalBytes / timeSinceLastPollSec;
		perc = (totalBytesPerSec * 100.0f) / EtherCATMaxAmountOfBytesPerSec;
		lastPollTimeMs = currentTimeMs;	// Update the last poll time.
	}
	SetResult(perc, TemperatureError::success);
}

EtherCATRxMonitor::EtherCATRxMonitor(unsigned int sensorNum) noexcept : TemperatureSensor(sensorNum, "EtherCAT Monitor Rx Buffer sensor")
{

}

void EtherCATRxMonitor::Poll() noexcept
{
	float perc = 0.0f;
	const TimeMs currentTimeMs = millis();
	const float timeSinceLastPollSec = ((float)(currentTimeMs - lastPollTimeMs)) / 1000.0f  ;
	if(timeSinceLastPollSec > 0.0f)
	{
		const float totalBytes = (float)(NetXSerial::GetInstance().GetTotalBytesReceivedAndClear() + NetXSocket::GetInstance().GetTotalBytesReceivedAndClear());
		const float totalBytesPerSec = totalBytes / timeSinceLastPollSec;
		perc = (totalBytesPerSec * 100.0)/(EtherCATMaxAmountOfBytesPerSec);
		lastPollTimeMs = currentTimeMs; // Update the last poll time.
	}
	SetResult(perc, TemperatureError::success);
}

#endif

// End
