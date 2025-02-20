#include "EmulatedBedSensor.h"
#include <GCodes/GCodeBuffer/GCodeBuffer.h>
#include "GCodes/GCodes.h"
#include <Platform/RepRap.h>
#include <Platform/Platform.h>
#include "CanMessageGenericParser.h"
#include <Movement/Move.h>
#include <Tools/Tool.h>
#include <Endstops/ZProbe.h>
#include <Platform/TaskPriorities.h>
#include <math.h>

// Sensor type descriptors
TemperatureSensor::SensorTypeDescriptor EmulatedBedSensor::emulatedBedSensorDescriptor(TypeName, [](unsigned int sensorNum) noexcept -> TemperatureSensor *_ecv_from { return new EmulatedBedSensor(sensorNum); } );

EmulatedBedSensor::EmulatedBedSensor(unsigned int sensorNum) noexcept
	: TemperatureSensor(sensorNum, "emulinear"){
}

void EmulatedBedSensor::Poll() noexcept
{
	const float ScaleToUm = 1000.0f;		// Scale to um
	float positions[MaxAxes];
	reprap.GetMove().GetCurrentMachinePosition(positions, reprap.GetGCodes().GetPrimaryMovementState().GetNumber());

	// Positions referenced to the offset
	const float XPosition = positions[0] - xOffset;
	const float YPosition = positions[1] - yOffset;

	// Calculation of the offsets from the plane and parabola
	const float PlaneOffset = ( xInclination*XPosition + yInclination*YPosition) / sqrt( xInclination*xInclination + yInclination*yInclination + 1);
	const float ParabolaOffset = ( XPosition*XPosition ) / ( aParabola*aParabola ) + ( YPosition*YPosition ) / ( bParabola*bParabola );

	const float BedShapeOffset = PlaneOffset + ParabolaOffset;

	if( !sensorReferenced && reprap.GetGCodes().IsAxisHomed(2) == true && positions[2] < 0 ) {
		sensorReferenced = true;
		// We may need a delay in the homing with the emulator to make sure this value is set!
		zOffset = positions[2] + BedShapeOffset;
	}

	// Z Position referenced to the offset
	const float ZPosition = positions[2] - zOffset;

	// Sensor value
	float sensorValue = ScaleToUm * ( BedShapeOffset + ZPosition ) ;

	// Setting the limits!
	sensorValue = min( sensorValue, maxValue );
	sensorValue = max( sensorValue, minValue );
	if(!sensorReferenced && sensorValue == minValue){
		sensorValue =  maxValue ;
	}

	// Adding noise
	sensorValue += NoiseGenerator(noiseAmplitude);

	// Reporting!
	SetResult( sensorValue , TemperatureError::ok );
}

int32_t EmulatedBedSensor::RandomGenerator(uint32_t range) noexcept
{
	// From Numerical Recipes
	constexpr uint32_t A = 1664525;
	constexpr uint32_t C = 1013904223;

	seed = (A*seed) + C;
	return static_cast<int>( seed % (range*2) ) - range;
}


float EmulatedBedSensor::NoiseGenerator(float amplitude) noexcept
{
	constexpr uint32_t Range = 10000;
	return amplitude * static_cast<float>(RandomGenerator(Range)) / static_cast<float>(Range);
}

GCodeResult EmulatedBedSensor::Configure(GCodeBuffer& gb, const StringRef& reply, bool& changed) THROWS(GCodeException)
{

	gb.TryGetFValue('H', xInclination, changed);
	gb.TryGetFValue('V', yInclination, changed);
	gb.TryGetFValue('W', xOffset, changed);
	gb.TryGetFValue('D', yOffset, changed);
	bool zChanged = false;
	gb.TryGetFValue('Z', zOffset, zChanged);  // This value may change when the machine is homed
	if(zChanged) {
		zOffsetDef = zOffset;
		changed = true;
	}
	gb.TryGetFValue('J', aParabola, changed);
	gb.TryGetFValue('K', bParabola, changed);
	gb.TryGetFValue('N', noiseAmplitude, changed);
	gb.TryGetFValue('T', maxValue, changed);
	gb.TryGetFValue('B', minValue, changed);
	ConfigureCommonParameters(gb, changed);

	if (!changed)
	{
		zOffset = zOffsetDef; // Reseting the offset value
		sensorReferenced = false;
		CopyBasicDetails(reply);
		reply.catf(" H%f V%f W%f D%f Z%f A(J):%f B(K):%f Noise:%f Max:%f Min:%f, ",
				(double)xInclination, (double)yInclination,
				(double)xOffset, (double)yOffset, (double)zOffset,
				(double)aParabola, (double)bParabola,
				(double)noiseAmplitude,
				(double)maxValue,(double)minValue);
	}

	return GCodeResult::ok;

}
