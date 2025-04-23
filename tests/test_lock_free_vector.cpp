#include <lockFreeVector.hpp>
#include "test_lock_free_vector.h"

namespace lcvector{
    int makeLockFreeTest()
    {
	testPush();
	testPop();
	testEmptyness();
	testTwoThread();
	testContention();
	return 0;
    }
int testPush()
{
    vectorLF<int> lockFree;
    lockFree.pushBack(12);
    lockFree.pushBack(14);
    assert(lockFree.size() == 2);
    return 0;
}
int testPop()
{
    return 0;
}
int testEmptyness()
{
    return 0;
}
int testTwoThread()
{
    return 0;
}
int testContention()
{
    return 0;
}
};
