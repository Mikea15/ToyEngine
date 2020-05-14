
#include "Engine/Game.h"

#include "BoidSystem/BoidSystemState.h"
#include "BoidSystem/ThreadedState.h"

#include <chrono>
#include <set>

#define RANDOM_STUFF 0
#define INC_WIN 1

#if INC_WIN || RANDOM_STUFF
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define SIMD_TESTS 0
#if SIMD_TESTS
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <time.h>

#define SIMD_ON 0
#if SIMD_ON
#define GLM_FORCE_SSE2
#define GLM_MESSAGES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define NUM_VERT 1000000
#define PI 3.14159f
#endif

#define TIME_START() \
	{ \
	LARGE_INTEGER start, end;			\
	QueryPerformanceCounter(&start);	\

#define TIME_END2(n) \
	QueryPerformanceCounter(&end);													\
	int totalTime = (int)((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);	\
	printf(" / avg %.4f (ms)\n", static_cast<float>(totalTime)/static_cast<float>(n));	\
	}					\

#define TIME_END() TIME_END2(1)

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

#elif SIMD_TESTS

	float* buf = new float[NUM_VERT + 4];

	std::srand(std::time(nullptr));
	for (size_t i = 0; i < NUM_VERT + 4; i++)
		buf[i] = float(std::rand() % 100) * PI;

	glm::mat4 Model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f));
	Model[3][2] = 0.11f;
	Model[2][0] = -0.1f;
	Model[0][1] = 0.04f;
	Model[1][3] = -0.2f;

	double sum = 0.0;
	
	glm::vec4 Pos, Trans;
	
	TIME_START();

	for (size_t i = 0; i < NUM_VERT; i++)
	{
		Pos.x = buf[i];
		Pos.y = buf[i + 1];
		Pos.z = buf[i + 2];
		Pos.w = buf[i + 3];
		Trans = Model * Pos;
		sum += glm::length(Trans);
	}

	TIME_END();

	std::cout << "  sum = " << sum << std::endl;

	delete[] buf;
	getchar();
	return 0;
#else

#if MULTITHREAD
	ThreadedState state;
#else
	BoidSystemState state;
#endif

	Game game(&state);
	return game.Execute();
#endif

}