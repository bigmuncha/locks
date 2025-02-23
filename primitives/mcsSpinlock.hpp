#include <atomic>
#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <thread>

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

class ErrorMyImplementation
{
    struct qnode
    {
	std::atomic<qnode*> next;
	std::atomic<bool> locked;
    };

    using  MSCLock = std::atomic<qnode*>;

public:
    [[gnu::noinline]]void lock()	{
	    qnode* const node = getThreadLocalNode();
	    //sanity_check(node);
	    node->next = nullptr;
	    qnode* predecessor = L.exchange(node);
	    //sanity_check(node);
	    if(predecessor == nullptr) //somebody acquire this lock
		return;
	    //sanity_check(node);
	    //std::cout << "have_predecessor deadlock"<< std::endl;
	    node->locked.store(true);
	    //sanity_check(node);
	    predecessor->next.store(node);
	    //sanity_check(node);
	    while(node->locked.load() == true)
	    {
		PAUSE();
	    }
	}

    [[gnu::noinline]] void unlock()
	{

	    qnode* node = getThreadLocalNode();
	    auto cmpxch_pointer = node;
	    //sanity_check(node);
	    if(node->next.load() == nullptr) // no known successor
	    {
		if(L.compare_exchange_strong(cmpxch_pointer,nullptr))
		{
		    //sanity_check(node);
		    // std::cout << "cas successed deadlock"<< std::endl;
		    return;
		}
		//std::cout << "cas error deadlock"<< std::endl;
		while(node->next.load() == nullptr)
		{
		    PAUSE();
		}

	    }
	    //sanity_check(node);
	    node->next.load()->locked.store(0);
	}
    void sanity_check(qnode * noda)
	{
	    if(thread_local_holder.size() == 2)
	    {
		auto first_thread = thread_local_holder.begin();
		auto second_thread = thread_local_holder.rbegin();
		assert(first_thread->first != second_thread->first);
		assert(first_thread->second != second_thread->second);
		auto index = std::this_thread::get_id();
		assert(thread_local_holder[index] == noda);
		std::cout <<"success check\n";
	    }
	}

    ~ErrorMyImplementation()
	{
	    for(auto &node: thread_local_holder)
	    {
		delete node.second;
	    }
	}
private:
    MSCLock L{};
    std::atomic<int> flager{};
    std::map<std::thread::id, qnode*> thread_local_holder{};
    /* qnode* getThreadLocalNode() */
    /* 	{ */
    /* 	    auto index = std::this_thread::get_id(); */
    /* 	    auto iter = thread_local_holder.find(index); */
    /* 	    if(iter != thread_local_holder.end()) */
    /* 	    { */
    /* 		return iter->second; */
    /* 	    } */
    /* 	    else */
    /* 	    { */
    /* 		qnode* node = new qnode; */
    /* 		node->locked.store(0); */
    /* 		node->next.store(nullptr); */
    /* 		//std::cout << "thread id is: " << index << " new node address: " << node << std::endl; */
    /* 		thread_local_holder.insert({index, node}); */
    /* 		return node; */
    /* 	    } */
    /* 	} */
    qnode* getThreadLocalNode()
	{
	    static thread_local qnode thread_holder;
	    //std::atomic<qnode*> addres_of = &thread_holder;
	    if(flager == 0)
	    {
		++flager;
		//std::cout << " node 1 address: "<< &thread_holder<< " thread is "<< std::this_thread::get_id() <<std::endl;
	    }
	    else if(flager == 1)
	    {
		++flager;
		//std::cout << " node 2 address: " << &thread_holder << " thread is "<< std::this_thread::get_id() <<std::endl;
	    }
	    return &thread_holder;
	}

};
