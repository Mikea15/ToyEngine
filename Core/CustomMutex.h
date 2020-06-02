#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

class CustomMutex
{
public:
	CustomMutex() 
	{
		InitializeCriticalSection((CRITICAL_SECTION*)mMutexBuffer);
	}

	~CustomMutex() 
	{
		DeleteCriticalSection((CRITICAL_SECTION*)mMutexBuffer);
	}

	void lock() 
	{
		EnterCriticalSection((CRITICAL_SECTION*)mMutexBuffer);
	}

	bool try_lock()
	{
		lock();
		return true;
	}

	void unlock() 
	{
		LeaveCriticalSection((CRITICAL_SECTION*)mMutexBuffer);
	}

protected:
#if defined(_WIN64)
	uint64_t mMutexBuffer[40 / sizeof(uint64_t)]; // CRITICAL_SECTION is 40 bytes on Win64.
#elif defined(_WIN32)
	uint32_t mMutexBuffer[24 / sizeof(uint32_t)]; // CRITICAL_SECTION is 24 bytes on Win32.
#endif
};
