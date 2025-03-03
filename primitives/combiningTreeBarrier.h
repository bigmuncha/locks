#include <atomic>
#include <array>

template <int procCount>
class combiningBarrier
{
    struct alignas(std::hardware_destructive_interference_size) qnode
    {
	int k;
	int counter;
	bool locksense;
	qnode* parent{};
    };


public:

    combiningBarrier()
    {
	arr[0].parent = nullptr;
	for(int i =1 ; i < procCount; i++)
	{
	    arr[i].k = 1;
	    arr[i].parent = arr[i-1];
	}
    }

    void barrier()
    {
	bool thread_local sense = true;
	qnode thread_local node;
	waitAll(&node, sense);
    }
private:
    void waitAll(qnode* noda, bool sense)
    {
	
    }
    std::array<qnode, procCount> arr;
};
