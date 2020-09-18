#include "TestRunner.h"

#include "OctreeTests/TestOctreeOld.h"
#include "OctreeTests/TestOctreeAlt.h"
#include "OctreeTests/TestOctreeNew.h"
#include "OctreeTests/TestOctreeJensB.h"

#include "Branches/TestAABB.h"

#include "MultiThreading/MutexLockTest.h"

#include <vectorclass/vectorclass.h>

#include "Core/JobScheduler/JobScheduler.h"
#include <algorithm>
#include <Systems/GameTime.h>

int main()
{
    const size_t numTests = 1000;
    TestRunner<numTests> testRunner;

    // testRunner.Add(new TestOctreeOldInsert());
    // testRunner.Add(new TestOctreeOldSearch());
    // testRunner.Add(new TestOctreeAltInsert());
    // testRunner.Add(new TestOctreeAltSearch());
    // testRunner.Add(new TestOctreeNewInsert());
    // testRunner.Add(new TestOctreeNewSearch());
    // testRunner.Add(new TestOctreeJensBInsert());
    // testRunner.Add(new TestOctreeJensBSearch());

    testRunner.Add(new StdMutexLockTest());
    // testRunner.Add(new CustomMutexLockTest());

    // testRunner.Add(new TestAABB(), new TestAABBNoBranch());

    // testRunner.RunTests();
    // testRunner.RunBenchs();

    auto glmTest = [](int workTime) {
        glm::vec3 p{ 0.0f, 0.0f, 0.0f };
        glm::vec3 d{ 0.0f, 1.0f, 0.0f };
        glm::vec3 p2{ 10.0f, 1.0f, 10.0f };

        size_t i = 0;
        size_t max = 5000;
        while (i < max) {
            ++i;

            glm::vec3 toTarget = p2 - p;
            float dist = glm::length(toTarget);
        }
    };

    auto glm4Test = []() {
        glm::vec4 p{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 d{ 0.0f, 1.0f, 0.0f, 0.0f };
        glm::vec4 p2{ 10.0f, 1.0f, 10.0f, 0.0f };

        size_t i = 0;
        size_t max = 5000;
        while (i < max) {
            ++i;

            glm::vec4 toTarget = p2 - p;
            float dist = glm::length(toTarget);
        }
    };

    auto vclTest = []() {
        Vec4f p(0.0f, 0.0f, 0.0f, 0.0f);
        Vec4f d(0.0f, 1.0f, 0.0f, 0.0f);
        Vec4f p2(10.0f, 1.0f, 10.0f, 0.0f);

        size_t i = 0;
        size_t max = 5000;
        while (i < max) {
            ++i;

            Vec4f toTarget = p2 - p;
            Vec4f len(toTarget[0] * toTarget[0], toTarget[1] * toTarget[1]
                , toTarget[2] * toTarget[2], toTarget[3] * toTarget[3]);
            float dist = sqrt(horizontal_add(len));

        }

    };

    auto& jobber = JobScheduler::GetInstance();

    jobber.Init();

    // GenericJob job(glmTest);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 1, 0);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 2, 1);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 3, 2);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 4, 3);
	jobber.AddBehavior(glmTest, 5, 4);
	jobber.AddBehavior(glmTest, 5, 4);
	jobber.AddBehavior(glmTest, 5, 4);
	jobber.AddBehavior(glmTest, 5, 4);
	jobber.AddBehavior(glmTest, 5, 4);
	jobber.AddBehavior(glmTest, 5, 4);

    float worktime = 2; // seconds
    
    GameTime timer;
    timer.Init();
    float freq = 1.0f / 1.0f;
    size_t frame = 0u;
    while (timer.GetTotalTime() < 10.0f)
    {
        timer.Tick();
		const float frameTime = timer.GetElapsed();

        printf("frame: (%d), frametime: (%f s), time: (%f s), jobs: (%d)\n", 
            frame, frameTime, timer.GetTotalTime(), jobber.Size());
        jobber.Run((int)frameTime);

        float deltaT = freq - frameTime;
        int time = static_cast<int>(deltaT * 1000); // second to ms

        ++frame;
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    jobber.Cleanup();

    //testRunner.RunQuick(glmTest, vclTest);
    //testRunner.RunQuick(glmTest, glm4Test);

    getchar();
    return 0;
}
