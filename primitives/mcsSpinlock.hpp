#include <atomic>
#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <thread>
#include <source_location>

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")
#define MEM_BARRIER() __sync_synchronize()

#define CAS(address,oldValue,newValue) __sync_bool_compare_and_swap(address,oldValue,newValue)
#define CAS_RET_VAL(address,oldValue,newValue) __sync_val_compare_and_swap(address,oldValue,newValue)
#define ADD_AND_FETCH(address,offset) __sync_add_and_fetch(address,offset)
#define FETCH_AND_ADD(address,offset) __sync_fetch_and_add(address,offset)

#define ATOMIC_LOAD(x) ({COMPILER_BARRIER(); *(x);})
#define ATOMIC_STORE(x, v) ({COMPILER_BARRIER(); *(x) = v; MEM_BARRIER(); })
#define ATOMIC_EXCHANGE(address, val) __atomic_exchange_n(address, val, __ATOMIC_SEQ_CST)
#define PAUSE() __asm__ __volatile__("pause\n" : : : "memory")
#define CACHE_ALIGN_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_ALIGN_SIZE)))

#define SPINLOCK_DEFINE(spin_name)                                             \
  struct spin_holder##spin_name##_FILE_##__FUNCTION__##__LINE__ {              \
    qnode *getLocalNode() {                                                    \
      alignas(std::hardware_destructive_interference_size) thread_local qnode  \
          thread_holder_;                                                      \
      return &thread_holder_;                                                  \
    }                                                                          \
    MCSspinLock spinner;                                                       \
  }                                                                            \
  spin_holder##spin_name;




#define ACQUIRE_LOCK(spin_name)                                                \
  {                                                                            \
    spin_holder##spin_name.spinner.lock(                                       \
        spin_holder##spin_name.getLocalNode());                                \
  }


#define RELEASE_LOCK(spin_name)                                                \
  {                                                                            \
    spin_holder##spin_name.spinner.unlock(                                     \
        spin_holder##spin_name.getLocalNode());                                \
  }

#define THREAD_LOCAL_ADDRESS(spin_name)			\
    spin_holder##spin_name.getLocalNode()			\


struct qnode
{
    std::atomic<qnode*> next;
    std::atomic<bool> locked;
};


class MCSspinLock
{
    using  MSCLock = std::atomic<qnode*>;

public:
    [[gnu::noinline]]void lock(qnode* thread_holder)	{
	    thread_holder->next = nullptr;
	    qnode* predecessor = L.exchange(thread_holder);
	    if(predecessor == nullptr) //somebody acquire this lock
		return;
	    thread_holder->locked.store(true);
	    predecessor->next.store(thread_holder);
	    while(thread_holder->locked.load() == true)
	    {
		PAUSE();
	    }
	}

    [[gnu::noinline]] void unlock(qnode* thread_holder)
	{
	    auto cmpxch_pointer = thread_holder;
	    if(thread_holder->next.load() == nullptr) // no known successor
	    {
		if(L.compare_exchange_strong(cmpxch_pointer,nullptr))
		{
		    return;
		}
		while(thread_holder->next.load() == nullptr)
		{
		    PAUSE();
		}

	    }
	    //sanity_check(node);
	    thread_holder->next.load()->locked.store(0);
	}
    [[gnu::noinline]] void unlock_no_cas(qnode* thread_holder)
	{
	    auto cmpxch_pointer = thread_holder;
	    if(thread_holder->next.load() == nullptr) // no known successor
	    {
		auto breaker = L.exchange(nullptr);
		if(breaker == thread_holder)
		{
		    return;
		}
		auto newer = L.exchange(breaker);
		while(thread_holder->next.load() == nullptr)
		{
		    PAUSE();
		}

		if(newer != nullptr)
		{
		    newer->next.store(thread_holder->next.load());
		}
		else
		{
		    thread_holder->next.load()->locked.store(0);
		}
	    }
	    else
	    {
		thread_holder->next.load()->locked.store(0);
	    }
	}
public:
    void lock()
	{
	    lock(getThreadLocalNode());
	}
    void unlock()
	{
	    unlock(getThreadLocalNode());
	}
private:
    MSCLock L{};
    std::atomic<int> flager{};

    qnode* getThreadLocalNode()
	{
	    alignas(std::hardware_destructive_interference_size) thread_local qnode thread_holder; //avoid false sharing
	    if(flager == 0)
	    {
		std::cout << "create node for 1 thread: "<< std::this_thread::get_id() << " node: "<< &thread_holder <<  std::endl;
		++flager;
	    }
	    else if(flager == 1)
	    {
		std::cout << "create node for 2 thread: "<< std::this_thread::get_id() << " node: "<< &thread_holder <<  std::endl;
		++flager;
	    }
	    return &thread_holder;
	}

};
