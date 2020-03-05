
#pragma once

#include <stdarg.h>
#include <stdio.h>

//#define NOMINMAX
// #include <windows.h>

enum class LEVEL : int
{
	NORMAL = 0,
	INFO,
	WARNING,
	ERR,
	FATAL
};

#define LOG(text, ...)			Logger::Log(LEVEL::NORMAL,	text, __VA_ARGS__);
#define LOG_INFO(text, ...)		Logger::Log(LEVEL::INFO,	text, __VA_ARGS__);
#define LOG_WARNING(text, ...)	Logger::Log(LEVEL::WARNING, text, __VA_ARGS__);
#define LOG_ERROR(text, ...)	Logger::Log(LEVEL::ERR,		text, __VA_ARGS__);
#define LOG_FATAL(text, ...)	Logger::Log(LEVEL::FATAL,	text, __VA_ARGS__);

class Logger
{
private:
	Logger() = delete;

public:
	static void Log(LEVEL level, const char* szFormat, ...)
	{
		static const unsigned int s_plotBufferSize = 1024;

		char szBuff[s_plotBufferSize];
		va_list arg;
		va_start(arg, szFormat);
		_vsnprintf_s(szBuff, s_plotBufferSize, szFormat, arg);
		va_end(arg);

		char warning[s_plotBufferSize];
		switch (level)
		{
		case LEVEL::INFO:
			strcpy_s(warning, "[Info] ");
			break;
		case LEVEL::WARNING:
			strcpy_s(warning, "[Warning] ");
			break;
		case LEVEL::ERR:
			strcpy_s(warning, "[Error] ");
			break;
		case LEVEL::FATAL:
			strcpy_s(warning, "[Fatal Error] ");
			break;
		case LEVEL::NORMAL:
		default:
			strcpy_s(warning, "[Log] ");
			break;
		}

		char finalBuffer[s_plotBufferSize];
		strcpy_s(finalBuffer, warning);
		strcat_s(finalBuffer, szBuff);
		strcat_s(finalBuffer, "\n");

		printf(finalBuffer);
		// OutputDebugString(finalBuffer);
	}
};
