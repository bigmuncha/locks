#include <atomic>
#include <array>

class Spinlock
{
    struct alignas(std::hardware_destructive_interference_size) node {
	std::atomic<bool> locked;
	std::atomic<node*> next;
    };
    std::array<node,10> node_arr;
    alignas(std::hardware_destructive_interference_size) std::atomic<node*> main_lock;
public:
    Spinlock()
	{
	    for(auto &a:node_arr)
	    {
		a.locked.store(0);
		a.next.store(nullptr);
		main_lock.store(nullptr);
	    }
	}

    void lock(int thread)
	{
	    auto node = &node_arr[thread];
	    auto previous = main_lock.exchange(node);
	    if(previous != nullptr)
	    {
		node->locked.store(1);
		previous->next.store(node);
		while(node->locked.load())
		{
		    //spining
		}
	    }
	}
    void unlock(int thread)
	{
	    auto node = &node_arr[thread];

	    if(node->next.load() == nullptr)
	    {
		if(main_lock.compare_exchange_strong(node, nullptr))
		    return; //succecs switch head
		//else, then somebody switch head and we wait till next will be available
		while(node->next.load() == nullptr)
		{
		    //spin while next node;
		}
	    }
	    auto next = node->next.exchange(nullptr); // take my next node
	    next->locked.store(false);
	}
};
