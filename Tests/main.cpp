
#include "TestRunner.h"

#include "OctreeTests/TestOctreeOld.h"
#include "OctreeTests/TestOctreeAlt.h"
#include "OctreeTests/TestOctreeNew.h"
#include "OctreeTests/TestOctreeJensB.h"

int main()
{
    TestRunner testRunner;

    testRunner.Add(new TestOctreeOldInsert());
    testRunner.Add(new TestOctreeOldSearch());
    testRunner.Add(new TestOctreeAltInsert());
    testRunner.Add(new TestOctreeAltSearch());
    testRunner.Add(new TestOctreeNewInsert());
    testRunner.Add(new TestOctreeNewSearch());
    testRunner.Add(new TestOctreeJensBInsert());
    testRunner.Add(new TestOctreeJensBSearch());

    testRunner.RunAll();

    getchar();
    return 0;
}
