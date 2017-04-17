#pragma once
#include "platform_basic_types.h"
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point TTime;
typedef std::chrono::high_resolution_clock::duration TDuration;

auto Now() -> TTime{

	TTime n = std::chrono::high_resolution_clock::now();
	return n;
}

auto Seconds(double s) -> TDuration{
	U64 NanoCount = s * 1000 * 1000 * 1000;
	auto Duration = std::chrono::nanoseconds(NanoCount);
	return Duration;
}

