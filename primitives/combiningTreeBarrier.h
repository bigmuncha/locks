#include <atomic>
#include <array>
#include <vector>
// less contantion on barrier

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
	//create tree structure(heap)
	for(int i=0; i < procCount; i++)
	{
	    array.push_back({ new qnode});
	}
	for(int i = 1; i < procCount; i+=2)
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
    std::vector<qnode*> array;
};
