#include <cassert>
#include <iostream>
#include <mcsSpinlock.hpp>
#include <thread>

static bool testPASS() { return true; }
static bool testFAIL() { return false; }

#include <github_mcs.hpp>

template <typename spinType>
int testLockInterference(spinType& spinner)
{

    std::atomic<bool> flag = 0;
    spinner.lock();

    std::thread t([&flag, &spinner](){
	flag.store(1);
	flag.notify_one();
	spinner.lock();
	spinner.unlock();
    });
    flag.wait(false);
    spinner.unlock();
    t.join();
    return flag;
}


void testLockREUSE()
{
    ErrorMyImplementation lock{};
    int thread1;
    lock.lock();
    lock.unlock();
    lock.lock();
    lock.unlock();
    std::cout << "TEST PASS REUSE\n";
}



void testLock10()
{
    static constexpr int operation_num = 2000;
    ErrorMyImplementation lock{};

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

int RUNTESTS()
{
    assert(testPASS() == true);
    assert(testFAIL() == false);
    //testLockREUSE();
    int* flag  = new int;
    for(int i = 0; i < 1000; i++)
    {
	WorkingGithubImplementation lock;
	*flag += testLockInterference(lock);
    }
    std::cout << "NEW TEST START";
    for( int i=0; i < 1000; i++)
    {
	ErrorMyImplementation lock;
	*flag+= testLockInterference(lock);
    }
    return *flag;
}


int main()
{
    std::cout <<"LOCKFREE TEST RUN" << std::endl;
    RUNTESTS();
    return 0;
}
