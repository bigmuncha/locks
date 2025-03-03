#include <atomic>
#include <thread>


class plainBarrier
{
    plainBarrier(int threadCount)
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
	    currentCounter_.store(threadCount_);
	    state.store(falseState);
	}
	else
	{
	    while(state.load() != falseState)
	    {
		
	    }
	}
    }

    std::atomic<bool> state;
    std::atomic<int> currentCounter_;
    const int threadCount_;
};
