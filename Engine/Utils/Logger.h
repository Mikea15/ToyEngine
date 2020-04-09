#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <time.h>

#if 1
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
// #pragma warning(push, 0)
#include <windows.h>
// #include <intrin.h>
//#include <windef.h>
//#include <fileapi.h>
//#include <synchapi.h>
//#include <debugapi.h>
//#pragma warning(pop)
#endif

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


#define strcpy strcpy_s
#define strcat strcat_s
#define _vsnprintf _vsnprintf_s
#define gmtime gmtime_s

class Logger
{
public:
	static void Log(LEVEL level, const char* szFormat, ...)
	{
		static const unsigned int s_plotBufferSize = 1024;

		char formattedBuffer[s_plotBufferSize];
		va_list arg;
		va_start(arg, szFormat);
		_vsnprintf(formattedBuffer, s_plotBufferSize, szFormat, arg);
		va_end(arg);

		std::string messageLog = "<" + GetLogTime() + "> ";
		messageLog += GetLogLevel(level);
		messageLog += formattedBuffer;
		messageLog += "\n";

		printf(messageLog.c_str());
		OutputDebugStringA(messageLog.c_str());
	}

private:
	Logger()
	{
		m_currentTime = time(0);
	};

	static std::string GetLogTime()
	{
		time_t rawtime;
		struct tm timeinfo;

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		// Must be static, otherwise won't work
		static char buffer[20];
		strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);

		return { buffer };
	}

	static std::string GetLogLevel(LEVEL lvl)
	{
		std::string levelStr = " ";

		switch (lvl)
		{
		case LEVEL::INFO:		levelStr += "[Info] "; break;
		case LEVEL::WARNING:	levelStr += "[Warning] "; break;
		case LEVEL::ERR:		levelStr += "[Error] "; break;
		case LEVEL::FATAL:		levelStr += "[Fatal Error] "; break;
		case LEVEL::NORMAL:
		default: break;
		}

		return levelStr;
	}

private:
	static time_t m_currentTime;
};
