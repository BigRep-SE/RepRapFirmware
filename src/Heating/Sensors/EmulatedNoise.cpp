#include "EmulatedNoise.h"
#include <cstdint>

inline int32_t RandomInt(int32_t min, int32_t max) {
	constexpr uint32_t A = 1664525;
	constexpr uint32_t C = 1013904223;
	static uint32_t seed = 134775813;

	if(max <= min) {
		return 0;
	}

	// From Numerical Recipes
	seed = (A*seed) + C;  // Updating the seed

	uint32_t range =  static_cast<uint32_t>(max - min + 1);

	return static_cast<int32_t>(seed % range) + min;
}

float EmulatedNoise::PeakToPeak(float peak){

	return  ( static_cast<float>(RandomInt(-1000, 1000)) * peak ) / 1000.0f   ;
}

