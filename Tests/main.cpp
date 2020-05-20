#include "TestRunner.h"

#include "OctreeTests/TestOctreeOld.h"
#include "OctreeTests/TestOctreeAlt.h"
#include "OctreeTests/TestOctreeNew.h"
#include "OctreeTests/TestOctreeJensB.h"

#include "Branches/TestAABB.h"

#include "MultiThreading/MutexLockTest.h"

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

    testRunner.RunTests();
    testRunner.RunBenchs();

    getchar();
    return 0;
}
