#include "doctest/doctest/doctest.h"
#include "scspring.hpp"
#include <thread>
#include <random>

TEST_CASE("testScSpRing")
{
    {
	scspRing<int> ringOfInt(8);
	{
	    REQUIRE(ringOfInt.empty());
	    for(int i =0; i < 5; i++)
	    {
		bool ret = ringOfInt.enqueue(i);
		REQUIRE(ret != false);
	    }

	    REQUIRE(ringOfInt.size() == 5);
	    for(int i=0; i < 5; i++)
	    {
		int val;
		bool ret = ringOfInt.dequeue(&val);
		REQUIRE(ret != false);
		REQUIRE(val == i);
	    }
	    REQUIRE(ringOfInt.empty());
	    REQUIRE(ringOfInt.size() == 0);
	}

	{
	    REQUIRE(ringOfInt.empty());
	    for(int i =0; i < 8; i++)
	    {
		bool ret = ringOfInt.enqueue(i);
		REQUIRE(ret != false);
	    }
	    REQUIRE(ringOfInt.size() == 8);

	    for(int i=0; i < 8; i++)
	    {
		int val;
		bool ret = ringOfInt.dequeue(&val);
		REQUIRE(ret != false);
		REQUIRE(val == i);
	    }
	}

	{
	    for(int i =10; i < 18; i++)
	    {
		bool ret = ringOfInt.enqueue(i);
		REQUIRE(ret != false);
	    }

	    for(int i=10; i < 18; i++)
	    {
		int val;
		bool ret = ringOfInt.dequeue(&val);
		REQUIRE(ret != false);
		REQUIRE(val == i);
	    }
	    REQUIRE(ringOfInt.empty());
	    REQUIRE(ringOfInt.size() == 0);
	}
    }

    {
	CHECK_THROWS(scspRing<int>{9});
    }

    {
	scspRing<int> ringOfInt1(16);
	for(int i =0; i < 16; i++)
	{
	    ringOfInt1.enqueue(i);
	}
	REQUIRE(ringOfInt1.size() == 16);
	bool ret = ringOfInt1.enqueue(1);
	REQUIRE(ret == false);
	REQUIRE(ringOfInt1.size() == 16);
	for(int i =0; i < 16; i++)
	{
	    int val;
	    ringOfInt1.dequeue(&val);
	}
	REQUIRE(ringOfInt1.empty());
	int v;
	ret = ringOfInt1.dequeue(&v);
	REQUIRE(ret == false);
	REQUIRE(ringOfInt1.empty());
    }
}

TEST_CASE("testScSpRingAtomic")
{
    auto startTime = std::chrono::steady_clock::now();
    scspRingAtomic<unsigned long long> ringOfInt(1 << 8);
    const auto  workTime = std::chrono::seconds{10};
    std::srand(std::time({}));
    unsigned long long random_value = std::rand();

    std::atomic_flag flag = {};
    std::cout << "Start omar "<< random_value << '\n';

    std::thread t([&ringOfInt, startTime, workTime, random_value, &flag](){
	std::cout << "second start\n";
	decltype(random_value) retVal;
	flag.test_and_set();
	flag.notify_one();
	while(true)
	{
	    for(int i =0; i < (1 << 8) ; i++)
	    {
		if(ringOfInt.dequeue(&retVal) == true)
		    REQUIRE(retVal == random_value);
	    }
	    auto end = std::chrono::steady_clock::now();
	    auto time = startTime - end;
	    if(std::chrono::duration_cast<std::chrono::seconds>(end - startTime) > workTime)
		break;
	}
	std::cout << "first end";
    });
    std::cout << "third start\n";
    flag.wait(false);
    while(true)
    {
	for(int i =0; i < (1 << 8) ; i++)
	{
	    ringOfInt.enqueue(random_value);
	}
	auto end = std::chrono::steady_clock::now();
	auto time = startTime - end;
	if(std::chrono::duration_cast<std::chrono::seconds>(end - startTime) > workTime)
	    break;
    }
    std::cout << "second end\n";
    t.join();
}

TEST_CASE("testScSpRingAtomicAlignas")
{
    auto startTime = std::chrono::steady_clock::now();
    scspRingAtomicAligned<unsigned long long> ringOfInt(1 << 8);
    const auto  workTime = std::chrono::seconds{10};
    std::srand(std::time({}));
    unsigned long long random_value = std::rand();

    std::atomic_flag flag = {};
    std::cout << "Start omar "<< random_value << '\n';

    std::thread t([&ringOfInt, startTime, workTime, random_value, &flag](){
	std::cout << "second start\n";
	decltype(random_value) retVal;
	flag.test_and_set();
	flag.notify_one();
	while(true)
	{
	    for(int i =0; i < (1 << 8) ; i++)
	    {
		if(ringOfInt.dequeue(&retVal) == true)
		    REQUIRE(retVal == random_value);
	    }
	    auto end = std::chrono::steady_clock::now();
	    auto time = startTime - end;
	    if(std::chrono::duration_cast<std::chrono::seconds>(end - startTime) > workTime)
		break;
	}
	std::cout << "first end";
    });
    std::cout << "third start\n";
    flag.wait(false);
    while(true)
    {
	for(int i =0; i < (1 << 8) ; i++)
	{
	    ringOfInt.enqueue(random_value);
	}
	auto end = std::chrono::steady_clock::now();
	auto time = startTime - end;
	if(std::chrono::duration_cast<std::chrono::seconds>(end - startTime) > workTime)
	    break;
    }
    std::cout << "second end\n";
    t.join();
}
