#pragma once

#include "platform.h"
#include "ae_string.h"
#include "ae_logging.h"
#include "ae_threading_spsc_queue.h"
#include "ae_input.h"

void busy_sleep(Duration duration){
	Time start_time = now();
	Time target_time = start_time + duration;
	while(true){
		Time current_time = now(); 
		if(current_time < target_time){
			// keep sleeping
		}else{
			return;
		}
	}
}
