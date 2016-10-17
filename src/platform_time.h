#pragma once
#include <chrono>

typedef std::chrono::high_resolution_clock::time_point Time;
typedef std::chrono::high_resolution_clock::duration Duration;

auto now() -> Time{
	Time n = std::chrono::high_resolution_clock::now();
	return n;
}

