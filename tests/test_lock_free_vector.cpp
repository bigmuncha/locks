#include <lockFreeVector.hpp>
#include <thread>
#include "doctest/doctest/doctest.h"

namespace lcvector{

TEST_CASE("test push vector")
{
    vectorLF<int> lockFree;
    CHECK(lockFree.size() == 0);
    lockFree.pushBack(12);
    lockFree.pushBack(14);
    CHECK(lockFree.at(0) == 12);
    CHECK(lockFree.at(1) == 14);
    CHECK(lockFree.size() == 2);
}

TEST_CASE("testPop")
{
    vectorLF<int> lockFree;
    lockFree.pushBack(12);
    lockFree.pushBack(14);
    lockFree.pushBack(15);
    CHECK(lockFree.at(0) == 12);
    CHECK(lockFree.at(1) == 14);
    CHECK(lockFree.at(2) == 15);
    CHECK(lockFree.popBack() == 15);
    CHECK(lockFree.popBack() == 14);
    CHECK(lockFree.popBack() == 12);
    CHECK(lockFree.size() == 0);
}

TEST_CASE("testTwoThread")
{
    vectorLF<int> lockFree;
    std::atomic_flag flag = {};
    std::thread t([&flag, &lockFree](){
	INFO("TRY TO PUt");
	lockFree.pushBack(124);
	flag.test_and_set();
	flag.notify_one();
	flag.wait(true);
	INFO("CHECK SIZE");
	CHECK(lockFree.size() == 0);
    });
    flag.wait(false);
    INFO("TRY TO POP");
    CHECK(lockFree.popBack() == 124);
    flag.clear();
    flag.notify_one();
    t.join();
    CHECK(lockFree.size() == 0);
}

TEST_CASE("testThreeThread")
{
    vectorLF<int> lockFree;
    std::atomic_flag flag = {};
    std::atomic_flag flag_second = {};
    std::thread t([&flag, &lockFree](){
	lockFree.pushBack(333);
	flag.test_and_set();
	flag.notify_one();
	flag.wait(true);
	CHECK(lockFree.size() == 0);
    });
    std::thread t2([&flag_second, &lockFree](){
	lockFree.pushBack(666);
	flag_second.test_and_set();
	flag_second.notify_one();
	flag_second.wait(true);
	CHECK(lockFree.size() == 0);
    });
    
    flag.wait(false);
    flag_second.wait(false);
    CHECK(lockFree.size() == 2);
    auto first = lockFree.popBack();
    auto second = lockFree.popBack();
    CHECK(first + second == 999);
    flag.clear();
    flag.notify_one();
    flag_second.clear();
    flag_second.notify_one();
    t.join();
    t2.join();
    CHECK(lockFree.size() == 0);
}

TEST_CASE("fullness")
{
    // i expect that vector can be full for 8^3 == 512 elements
    vectorLF<int,3> lockFree;
    for(int i=0; i < 512; i++)
    {
	lockFree.pushBack(i);
    }
    CHECK(lockFree.size() == 512);

    for(int i =512; i >0; i--)
    {
	auto current = lockFree.popBack();
	CHECK(current == i);
    }
}
};
