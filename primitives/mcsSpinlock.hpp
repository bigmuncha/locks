#include <atomic>
#include <iostream>
#include <vector>

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

struct qnode
{
    std::atomic<qnode*> next;
    std::atomic<bool> locked;
};

using  MSCLock = std::atomic<qnode*>;

static void release_lock_no_cas(MSCLock &L, qnode* I)
{
    if(I->next.load() == nullptr)
    {
	auto old_tail = L.exchange(nullptr);
	if(old_tail == I) 
	{
	    return;
	}
	auto usurper = L.exchange(old_tail);
	while(I->next.load() == nullptr)
	{
	    //SPIN
	}
	if(usurper != nullptr)
	{
	    usurper->next.store(I->next.load());
	}
	else
	{
	    I->next.load()->locked.store(false);
	}
    }
    else
	I->next.load()->locked.store(false);
}


class MCSspinlock
{
public:
    [[gnu::noinline]]void lock()	{
	    auto node = getThreadLocalNode();
	    node->next = nullptr;
	    auto predecessor = L.exchange(node, std::memory_order_acquire);
	    if(predecessor == nullptr) //somebody acquire this lock
		return;
	    node->locked.store(true, std::memory_order_relaxed);
	    predecessor->next.store(node, std::memory_order_release);
	    while(node->locked.load(std::memory_order_acquire))
	    {
		PAUSE();
	    }
	}

    [[gnu::noinline]] void unlock()
	{
	    auto node = getThreadLocalNode();
	    if(node->next.load(std::memory_order_relaxed) == nullptr) // no known successor
	    {
		if(L.compare_exchange_strong(node,nullptr, std::memory_order_release))
		    return;
		while(node->next.load(std::memory_order_acquire) == nullptr)
		{
		    PAUSE();
		}
	    }
	    node->next.load()->locked.store(0, std::memory_order_release);
	}
    bool checkEmpty()
	{
	    auto queue = L.load();
	    int res = 0;
	    while(queue)
	    {
		res = 1;
		std::cout <<"NOT EMPTY " << " locked? " << (int)queue->locked;
		queue = queue->next.load();
	    }
	    std::cout << "queue empty" << std::endl;
	     return res;
	}
private:
    MSCLock L{};
    qnode* getThreadLocalNode()
	{
	    static __thread qnode thread_holder;
	    return &thread_holder;
	}
};
