
#include "Launcher.h"
#include "Engine/Game.h"
#include "BoidSystem/BoidSystemState.h"

#include <chrono>
#include <set>

#define RANDOM_STUFF 0

#if RANDOM_STUFF
#define _AMD64_
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#pragma warning(push, 0)
#include <windows.h>
//#include <windef.h>
//#include <fileapi.h>
//#include <synchapi.h>
//#include <debugapi.h>
#pragma warning(pop)
#endif

int main(int argc, char* argv[])
{
#if RANDOM_STUFF
	unsigned int count = 10000;
	HANDLE mutex = CreateMutex(NULL, FALSE, NULL);
	CRITICAL_SECTION critSec;
	InitializeCriticalSection(&critSec);

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER start, end;

	// Force code into memory, so we don't see any effects of paging.
	EnterCriticalSection(&critSec);
	LeaveCriticalSection(&critSec);
	QueryPerformanceCounter(&start);
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++) 
		{
			EnterCriticalSection(&critSec);
			LeaveCriticalSection(&critSec);
		}
	}

	QueryPerformanceCounter(&end);

	int totalTimeCS = (int)((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);

	// Force code into memory, so we don't see any effects of paging.
	WaitForSingleObject(mutex, INFINITE);
	ReleaseMutex(mutex);

	QueryPerformanceCounter(&start);
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++) {
			WaitForSingleObject(mutex, INFINITE);
			ReleaseMutex(mutex);
		}
	}

	QueryPerformanceCounter(&end);

	int totalTime = (int)((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);

	printf("Mutex: %d CritSec: %d\n", totalTime, totalTimeCS);

	getchar();
	return 0;
#else
	BoidSystemState stubState;
	Game game(&stubState);
	return game.Execute();
#endif
}