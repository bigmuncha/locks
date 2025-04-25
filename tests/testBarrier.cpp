#include <plainBarrier.h>
#include <cassert>
#include "doctest/doctest/doctest.h"

TEST_CASE("testBarrierOneUse")
{
    auto  waitTime = std::chrono::seconds{2};
    plainBarrier barrier(2);
    auto start =  std::chrono::steady_clock::now();
    std::thread t([&barrier, start, waitTime](){
	barrier.WaitAll();
	auto stop =  std::chrono::steady_clock::now();
	auto time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	assert(time_elapsed > waitTime);
    });
    std::this_thread::sleep_for(waitTime);
    barrier.WaitAll();
    t.join();
    std::cout << "SUCCES pass throught";
}


TEST_CASE("testBarrierReuse")
{
    auto  waitTime = std::chrono::seconds{2};
    plainBarrier barrier(2);
    auto start =  std::chrono::steady_clock::now();
    std::thread t([&barrier, start, waitTime](){
	barrier.WaitAll();
	auto stop =  std::chrono::steady_clock::now();
	auto time_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	assert(time_elapsed > waitTime);

	auto second_start =  std::chrono::steady_clock::now();

	barrier.WaitAll();
	auto second_stop =  std::chrono::steady_clock::now();
	auto second_time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(second_stop - second_start);
	assert(time_elapsed > waitTime);
    });
    std::this_thread::sleep_for(waitTime);
    barrier.WaitAll();
    std::this_thread::sleep_for(waitTime);
    barrier.WaitAll();
    t.join();
    std::cout << "SUCCES pass throught";
}
