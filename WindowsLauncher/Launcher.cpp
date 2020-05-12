
#include "Launcher.h"
#include "Engine/Game.h"

#include "BoidSystem/BoidSystemState.h"
#include "BoidSystem/ThreadedState.h"

#include "Engine/Systems/Octree.h"
#include "Engine/Core/exp_Octree.h"
#define OCTREE_TEST 1

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
	LARGE_INTEGER freq;					\
	QueryPerformanceFrequency(&freq);	\
	LARGE_INTEGER start, end;			\
	QueryPerformanceCounter(&start);	\

#define TIME_END() \
	QueryPerformanceCounter(&end);													\
	int totalTime = (int)((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);	\
	printf("Time %d (ms)\n", totalTime);													\


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
#elif OCTREE_TEST

	float range = 2.0f;
	glm::vec3 qPoint = { 0.0f, 0.0f, 0.0f };
	size_t nPoints = 1000;
	size_t nTests = 10000;

	std::vector<glm::vec3> points(nPoints);
	for (size_t i = 0; i < nPoints; i++)
	{
		points[i] = MathUtils::RandomInUnitSphere() * 10.0f;
	}

	std::vector<size_t> indices;
	{
		Octree oct;
		TIME_START();
		oct.Initialize(points);
		
		for (size_t i = 0; i < nTests; i++)
		{
			indices.clear();
			oct.FindNeighborsAlt(qPoint, range, indices);
		}
		TIME_END();
	}

	std::vector<OcNode> result;
	{
		Octree oct = Octree(glm::vec3(0.0f), 10.0f);
		TIME_START();
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}

		for (size_t i = 0; i < nTests; i++)
		{
			result.clear();
			oct.Search(AABB(qPoint, range), result);
			std::remove_if(result.begin(), result.end(), [&](const OcNode& n) {
				return glm::length2(qPoint - points[n.m_data]) > range * range;
				});
		}
		TIME_END();
	}
	
	std::vector<size_t> indices2;
	{
		unibn::OctreeParams oParams;
		oParams.bucketSize = 4;
		unibn::Octree<glm::vec3> oct2;

		TIME_START();
		oct2.initialize(points, oParams);

		for (size_t i = 0; i < nTests; i++)
		{
			indices2.clear();
			oct2.radiusNeighbors<unibn::L2Distance<glm::vec3>>(qPoint, range, indices2);
		}
		TIME_END();
	}
	
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