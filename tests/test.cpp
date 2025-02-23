#include <cassert>
#include <iostream>
#include <mcsSpinlock.hpp>
#include <thread>

static bool testPASS() { return true; }
static bool testFAIL() { return false; }


void testLockInterference()
{
    //std::cout <<" start empty\n";
    MCSspinlock lock;
    std::atomic<bool> flag = 0;
    lock.lock();

    std::thread t([&flag, &lock](){
	flag.store(1);
	lock.lock();
	lock.unlock();
    });
    while(flag == 0) {
    };
    lock.unlock();
    t.join();
    //lock.checkEmpty();
    //std::cout << "TEST INTERFERENCE REUSE\n";
}
#include <github_mcs.hpp>

void testLockInterferenceGithub()
{
    //std::cout <<" start GITHUB\n";
    GithubMCS lock;
    std::atomic<bool> flag = 0;
    lock.lock();

    std::thread t([&flag, &lock](){
	flag.store(1);
	lock.lock();
	lock.unlock();
    });
    while(flag == 0) {
    };
    lock.unlock();
    t.join();
}

void testLockREUSE()
{
    MCSspinlock lock{};
    int thread1;
    lock.lock();
    lock.unlock();
    lock.lock();
    lock.unlock();
    lock.checkEmpty();
    std::cout << "TEST PASS REUSE\n";
}



void testLock10()
{
    static constexpr int operation_num = 2000;
    MCSspinlock lock{};

    volatile unsigned int current_num = 0;
    std::array<std::thread, 10> thread_arr;
    std::atomic< bool> barrier = 0;
    for(int thread_I =0; thread_I < thread_arr.size(); thread_I++)
    {
	thread_arr[thread_I] = std::thread([&barrier, thread_I, &lock, &current_num](){
	    while(barrier.load() == 0)
	    {
		//busy wait until ;
	    }
	    std::cout << "SPAWN THREAD WITH ID: " << thread_I << std::endl;
	    for(int i = 0; i < operation_num; i++)
	    {
		//std::cout << "try lock on thread: " << thread_I << std::endl;
		lock.lock();
		//std::cout << "make work in thread: " << thread_I << std::endl;
		auto now = current_num;
		if(now == current_num)
		    current_num = current_num + 1;
		assert(now == current_num - 1);
		//std::cout << "try unlock on thread: " << thread_I << std::endl;
		lock.unlock();
	    }
	});
    }
    barrier.store(true);
    for(auto &t: thread_arr)
    {
	t.join();
    }
    assert(current_num == operation_num * thread_arr.size());
    std::cout << "TEST PASS\n";
}

void RUNTESTS()
{
    assert(testPASS() == true);
    assert(testFAIL() == false);
    testLockREUSE();
    for(int i =0; i < 1000; i++)
    {
	testLockInterference();
    }
    for(int i =0; i < 1000; i++)
    {
	testLockInterferenceGithub();
    }
    //testLockInterference();
    //testLock10();
}


int main()
{
    std::cout <<"LOCKFREE TEST RUN" << std::endl;
    RUNTESTS();
    return 0;
}
