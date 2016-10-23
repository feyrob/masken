#pragma once

#include "platform.h"

static String str(Duration d){
	U64 nanos = d.count();
	U64 seconds = nanos / 1000 / 1000 / 1000;
	U64 nanos_without_seconds = nanos % (1000 * 1000 * 1000);
	String nano_str = tfm::format("%s", nanos_without_seconds);
	while(nano_str.size() < 9){
		nano_str = String("0") + nano_str;
	}
	String seconds_str = tfm::format("%s", seconds);
	while(seconds_str.size() < 3){
		seconds_str = String("0") + seconds_str;
	}
	String m = seconds_str + "." + nano_str;
	return m;
}

static String str(Time t){
	Duration d = t.time_since_epoch();
	String s = str(d);
	return s;
}

static String str(int i, int min_width){
	String m = tfm::format("%i",i);
	while(m.size() < min_width){
		m = String("0") + m;
	}
	return m;
}

static String str(int i){
	String m = tfm::format("%i",i);
	return m;
}

static String str(float f){
	String m = tfm::format("%f",f);
	return m;
}

static String str(const char* str){
	if(str){
		String m = "\"";
		m += str;
		m += "\"";
		return m;
	}else{
		return String("nil");
	}
}

