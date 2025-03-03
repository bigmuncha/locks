#include <cassert>
#include <iostream>
#include <mcsSpinlock.hpp>
#include <thread>

static bool testPASS() { return true; }
static bool testFAIL() { return false; }

#include <github_mcs.hpp>


void testTwoLocks()
{
    std::cout << "TEST PASS 2locks start\n";
    MCSspinLock lock1;
    MCSspinLock lock2;
    lock1.lock();
    lock2.lock();
    lock1.unlock();
    lock2.unlock();
    std::cout << "TEST PASS 2locks\n";
}

class classWithSpinlock
{
    SPINLOCK_DEFINE(spin1);
public:

    void make_work()
	{
	    ACQUIRE_LOCK(spin1);
	    RELEASE_LOCK(spin1);
	}
    void* getId()
	{
	    return THREAD_LOCAL_ADDRESS(spin1);
	}
};

void testSpinInClass()
{
    classWithSpinlock class1;
    classWithSpinlock class2;
    class1.make_work();
    class2.make_work();
    std::cout <<"first class spin addr"<< class1.getId() << '\n';
    std::cout <<"second class spin addr"<< class2.getId() << '\n';
};
void testTwoLocksMACRO()
{
    std::cout << "TEST PASS 2locks MACRO start\n";
    {

	SPINLOCK_DEFINE(spin1);
	SPINLOCK_DEFINE(spin2);
	ACQUIRE_LOCK(spin1);
	ACQUIRE_LOCK(spin2);
	std::cout <<"first spin addr"<< THREAD_LOCAL_ADDRESS(spin1) << '\n';
	std::cout <<"second spin addr"<< THREAD_LOCAL_ADDRESS(spin2) << '\n';
	RELEASE_LOCK(spin1);
	RELEASE_LOCK(spin2);
    }
    {
	SPINLOCK_DEFINE(spin1);
	SPINLOCK_DEFINE(spin2);
	ACQUIRE_LOCK(spin1);
	ACQUIRE_LOCK(spin2);
	std::cout <<"first spin addr"<< THREAD_LOCAL_ADDRESS(spin1) << '\n';
	std::cout <<"second spin addr"<< THREAD_LOCAL_ADDRESS(spin2) << '\n';
	RELEASE_LOCK(spin1);
	RELEASE_LOCK(spin2);
    }
    std::cout << "TEST PASS 2locks macro\n";
}

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
    MCSspinLock lock{};
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
    MCSspinLock lock{};

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
    testLockREUSE();
    testTwoLocks();

    int* flag  = new int;
    std::cout << "NEW TEST START";
    for( int i=0; i < 1000; i++)
    {
	MCSspinLock lock;
	*flag+= testLockInterference(lock);
    }

    testTwoLocks();
    testTwoLocksMACRO();
    testSpinInClass();
    return *flag;
}


int main()
{
    std::cout <<"LOCKFREE TEST RUN" << std::endl;
    RUNTESTS();
    return 0;
}
