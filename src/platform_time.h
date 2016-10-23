#pragma once
#include "platform_basic_types.h"
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point Time;
typedef std::chrono::high_resolution_clock::duration Duration;

auto now() -> Time{
	Time n = std::chrono::high_resolution_clock::now();
	return n;
}

Duration seconds(double count){
	U64 nano_count = count * 1000 * 1000 * 1000;
	auto nano_duration = std::chrono::nanoseconds(nano_count);
	return nano_duration;
}

