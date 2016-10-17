#include "ae.h"


auto main(int argc, char** argv) -> int{
	auto start_time = now();
	Log::verbose("","message 1");
	Log::debug("","message 2");
	auto init_end_time = now();

	auto init_duration = init_end_time - start_time;

	auto end_time = now();
	auto run_duration = end_time - start_time;
	String m = "run_duration: " + str(run_duration);
	Log::info("",m);

	return 0;
}

