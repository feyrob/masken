#pragma once
#include "platform.h"
#include "ae_string.h"

enum ELevel {
	e_debug,
	e_verbose,
	e_info,
	e_attention,
	e_warning,
	e_error
};

const char* styleResetAll = "\033[0m";
//const char* styleResetColors = "\033[39;49m";
const char* styleRed = "\033[31m";
const char* styleGreen = "\033[32m";
const char* styleGreenBright = "\033[1;32m";
const char* styleYellow = "\033[33m";
const char* styleYellowBright = "\033[1;33m";
const char* styleBlue = "\033[34m";
const char* styleBlueBright = "\033[1;34m";
const char* styleMagenta = "\033[35m";
const char* styleMagentaBright = "\033[1;35m";
const char* styleCyan = "\033[36m";
const char* styleCyanBright = "\033[1;36m";
const char* styleWhite = "\033[37m";
const char* styleWhiteBright = "\033[1;37m";
const char* styleBlackOnWhite= "\033[30;47m";
const char* styleGrayDark = "\033[1;30m";

char const* c_levelStr[] = {
	"debug", 
	"verbose", 
	"info", 
	"attention", 
	"warning", 
	"error"
};

char const* c_levelStyle[] = {
	// debug
	styleWhiteBright,
	// verbose
	styleMagentaBright,
	// info
	styleCyan,
	// attention 
	styleGreen,
	// warning
	styleYellow,
	// error
	styleRed
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

struct ILoggerBackend{
	virtual void log(LogEntry& log_entry) = 0;
};

struct ConsoleLoggerBackend : public ILoggerBackend{
	Time m_start_time;
	ConsoleLoggerBackend(Time start_time){
		m_start_time = start_time;
	}

	void log(LogEntry& log_entry){

		auto duration_since_start = log_entry.m_time - m_start_time;

		String levelStr(
			c_levelStr[log_entry.m_level]
		);
		while(levelStr.size() < 9){
			levelStr += " ";
		}
		auto style = c_levelStyle[log_entry.m_level];

		String duration_since_start_str = str(duration_since_start);

		String styledLevelStr = style + levelStr + styleResetAll; 

		String m = duration_since_start_str + " " + styledLevelStr + " [" + log_entry.m_ndc + "] " + log_entry.m_message;
		
		printf(
			"%s\n",
			m.c_str()
		);
	}

};

struct Log {
	std::vector<ILoggerBackend*> m_backends;
	Log() {}
	~Log() {}

	void add_backend(ILoggerBackend* backend){
		m_backends.push_back(backend);
	}

	static Log& instance() {
		static Log s;
		return s;
	}

	void log(
		LogEntry entry) {
		for(auto backend:m_backends){
			backend->log(entry);
		}
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
