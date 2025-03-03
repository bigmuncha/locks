#include <atomic>
#include <thread>
#include <vector>

class McsMyLockImpelemet
{

    McsMyLockImpelemet(int threadCount)
    {
	nodes.resize(threadCount);
	for(int i = 0; i < threadCount; i++)
	{
	    nodes[i] = new qnode;
	}
	Main.store(nullptr);
	threads  = 0;
    }

    ~McsMyLockImpelemet()
    {
	for(int i = 0; i < nodes.size(); i++)
	{
	    delete nodes[i];
	}
    }

    void * registerThread()
    {
	if(threads > nodes.size())
	{
	    throw std::exception();
	}

	auto node = nodes[threads];
	++threads;
	return node;
    }

    void lock(void* noda)
    {
	qnode * threadNode  = static_cast<qnode*>(noda);

	auto predecessor = Main.exchange(threadNode);
	if(predecessor != nullptr)
	{
	    threadNode->locked.store(true, std::memory_order_relaxed);
	    predecessor->next.store(threadNode, std::memory_order_release);

	    while(threadNode->locked.load(std::memory_order_relaxed) == true)
	    {
		//PAUSE
	    }
	}
    }

    void unlock(void* noda)
    {
	qnode * threadNode  = static_cast<qnode*>(noda);

	if(threadNode->next.load(std::memory_order_acquire) == nullptr)
	{
	    auto temporary = threadNode;
	    if(Main.compare_exchange_weak(temporary, nullptr) == true)
	    {
		return;
	    }
	    while(threadNode->next.load(std::memory_order_acquire) == nullptr)
	    {
		//PAUSE
	    }
	}
	threadNode->next.load(std::memory_order_acquire)->locked.store(false, std::memory_order_relaxed);
    }

    void unlock_no_cas(void* noda)
    {
	qnode * threadNode  = static_cast<qnode*>(noda);

	if(threadNode->next.load(std::memory_order_acquire) == nullptr)
	{
	    auto content = Main.exchange(nullptr);
	    if(content == threadNode)
	    {
		return;
	    }
	    auto current = Main.exchange(content);
	    if(current == nullptr)
	    {
		while(threadNode->next.load(std::memory_order_acquire) == nullptr)
		{
		    //PAUSE
		}
	    }
	}
	threadNode->next.load(std::memory_order_acquire)->locked.store(false, std::memory_order_relaxed);
    }

private:
    struct qnode
    {
	std::atomic<qnode*> next;
	std::atomic<bool> locked;
    };
    std::vector<qnode*> nodes;
    int threads;
    std::atomic<qnode*> Main;
};

