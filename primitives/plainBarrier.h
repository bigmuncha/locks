#include <atomic>
#include <thread>
#include <iostream>
#define PAUSE() __asm__ __volatile__("pause\n" : : : "memory")
class plainBarrier
{
public:
    explicit plainBarrier(int threadCount)
	:threadCount_(threadCount)
    {
	currentCounter_ = threadCount_;
	state = true;
    }

    void WaitAll()
    {
	auto falseState = !state.load();
	if(currentCounter_.fetch_sub(1) == 1)
	{
	    std::cout << "barrier passed\n";
	    currentCounter_.store(threadCount_);
	    state.store(falseState);
	}
	else
	{
	    std::cout << "waiting\n";
	    while(state.load() != falseState)
	    {
		PAUSE();
	    }
	}
    }
private:
    std::atomic<bool> state;
    std::atomic<int> currentCounter_;
    const int threadCount_;
};
