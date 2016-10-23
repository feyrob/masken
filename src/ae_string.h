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

static String str2(int i, int min_width){
	U64 u2 = i;
	String m = tfm::format("%llu",u2);
	while(m.size() < min_width -1){
		m = String("0") + m;
	}
	if(i < 0){
		m = String("-") + m;
	}else{
		m = String("+") + m;
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

static String str3(float f, int min_leading, int trailing){
	float pf = f;
	if(f < 0.0f){
		pf = -pf;
	}
	String format = tfm::format("%%.%if",trailing);
	String m = tfm::format(format.c_str(),pf);
	int min_width = min_leading + 1 + trailing;
	while(m.size() < min_width){
		m = String(0) + m;
	}
	if(f < 0.0f){
		m = String("-") + m;
	}else{
		m = String("+") + m;
	}
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

