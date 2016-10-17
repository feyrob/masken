#pragma once
#include "platform.h"
#include "tinyformat/tinyformat.h"

static String str(Duration d){
	U64 nanos = d.count();
	double seconds = static_cast<double>(nanos) / 1000.0 / 1000.0 / 1000.0;
	String time_str = tfm::format("%10f", seconds);
	int len = 17;
	//int len = 14;
	while(time_str.size() < len){
		time_str += "0";
	}
	while(time_str.size() > len){
		time_str.pop_back();
	}
	return time_str;
}

static String str(Time t){
	Duration d = t.time_since_epoch();
	String s = str(d);
	return s;
}

enum ELevel {
	e_debug,
	e_verbose,
	e_info,
	e_attention,
	e_warning,
	e_error
};

char const* level_cstr[] = {
	"debug", 
	"verbose", 
	"info", 
	"attention", 
	"warning", 
	"error"
};

struct LogEntry {
	Time m_time;
	ELevel m_level;
	String m_ndc;
	String m_message;

	LogEntry(){}

	LogEntry(
		Time time,
		ELevel level,
		String ndc,
		String message
	) :
			m_time(time),
			m_level(level),
			m_ndc(ndc),
			m_message(message)
	{}

	String to_string() {
		String level_str(
			level_cstr[m_level]
		);

		String now_str = str(m_time);

		String m = now_str + " [" + m_ndc + "] " + level_str + ": " + m_message;
		return m;
	}

	static LogEntry create(
		ELevel level,
		String ndc,
		String message
	){
		Time now = std::chrono::high_resolution_clock::now();
		LogEntry log_entry(
			now,
			level,
			ndc,
			message
		);
		return log_entry;
	}
};

struct Log {
	Log() {}
	~Log() {}

	static Log& instance() {
		static Log s;
		return s;
	}

	void log(
		LogEntry entry) {
		String s = entry.to_string();
		printf(
		    "%s\n",
		    s.c_str()
		);
	}

	static void slog(
		LogEntry entry) {
		auto logger = instance();
		logger.log(entry);
	}

	static void debug(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_debug,
			ndc,
			message
		);
		slog(entry);
	}

	static void verbose(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_verbose,
			ndc,
			message
		);
		slog(entry);
	}

	static void info(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_info,
			ndc,
			message
		);
		slog(entry);
	}

	static void attention(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_attention,
			ndc,
			message
		);
		slog(entry);
	}

	static void warning(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_warning,
			ndc,
			message
		);
		slog(entry);
	}

	static void error(
		String ndc,
		String message
	){
		auto entry = LogEntry::create(
			e_error,
			ndc,
			message
		);
		slog(entry);
	}
	
};
