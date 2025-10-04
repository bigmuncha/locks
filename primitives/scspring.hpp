#include <new>
#include <cmath>
#include <iostream>
#include <bit>
#include <assert.h>
#include <atomic>

// no atomic realisation(like dpdk)

template <typename T>
class RingInterface
{
    bool enqueue(T);
    bool dequeue(T&);
    bool enqueue_bulk(T*, int size);
    bool dequeue_bulk(T*, int size);

    int size();
};

static inline int fastModForPow2(int a, int capacity)
{
    return a & (capacity - 1);
}

template <typename T>
class scspRing {
public:
    scspRing(unsigned int cap)
	:capacity(cap)
	{
	    if(std::popcount(capacity) != 1)
		throw std::exception {};
	    data = new T[cap];
	    head = 0;
	    tail = 0;
	}
    ~scspRing()
	{
	    delete [] data;
	}
    bool enqueue(T val)
	{
	    if(std::abs(tail - head) == capacity)
		return false;
	    auto oldTail = tail;
	    tail = oldTail + 1;
	    auto tailTake = fastModForPow2(oldTail, capacity);
	    std::cout << "enqueue OLD tail "<< oldTail <<'\n';
	    data[tailTake] = val;
	    return true;
	}

    bool dequeue(T* val)
	{
	    if(tail == head)
		return false;
	    auto oldHead = head;
	    head = oldHead + 1;
	    auto headTake = fastModForPow2(oldHead, capacity);
	    std::cout << "dequeue OLD head "<< oldHead <<'\n';
	    *val = data[headTake];
	    return true;
	}
    bool empty()
	{
	    return tail == head;
	}
    int size()
	{
	    return (std::abs(tail - head));
	}
private:
    int head;
    int tail;
    unsigned int capacity;
    T* data;
};

template <typename T>
class scspRingAtomic {
public:
    scspRingAtomic(unsigned int cap)
	:capacity(cap)
	{
	    if(std::popcount(capacity) != 1)
		throw std::exception {};
	    data = new (std::align_val_t(64)) T[cap];
	    head = 0;
	    tail = 0;
	}
    ~scspRingAtomic()
	{
	    delete [] data;
	}

    bool enqueue(T val)
	{
	    if(size() == capacity)
		return false;
	    auto oldTail = tail.load();
	    auto tailTake = fastModForPow2(oldTail, capacity);
	    (data[tailTake]) = val;
	    tail = oldTail + 1;
	    return true;
	}

    bool dequeue(T* val)
	{
	    if(empty())
		return false;
	    auto oldHead = head.load();
	    auto headTake = fastModForPow2(oldHead, capacity);
	    *val = (data[headTake]);
	    head = oldHead + 1;
	    return true;
	}
    bool empty()
	{
	    return tail == head;
	}
    int size()
	{
	    return (int)tail - (int)head;
	}
private:
    std::atomic<unsigned int> head;
    std::atomic<unsigned int> tail;
    const unsigned int capacity;
    T* data;
};

template <typename T>
class scspRingAtomicAligned {
public:
    scspRingAtomicAligned(unsigned int cap)
	:capacity(cap)
	{
	    if(std::popcount(capacity) != 1)
		throw std::exception {};
	    data = new (std::align_val_t(64)) T[cap];
	    head = 0;
	    tail = 0;
	}
    ~scspRingAtomicAligned()
	{
	    delete [] data;
	}

    bool enqueue(T val)
	{
	    if(size() == capacity)
		return false;
	    auto oldTail = tail.load();
	    auto tailTake = fastModForPow2(oldTail, capacity);
	    (data[tailTake]) = val;
	    tail = oldTail + 1;
	    return true;
	}

    bool dequeue(T* val)
	{
	    if(empty())
		return false;
	    auto oldHead = head.load();
	    auto headTake = fastModForPow2(oldHead, capacity);
	    *val = (data[headTake]);
	    head = oldHead + 1;
	    return true;
	}
    bool empty()
	{
	    return tail == head;
	}
    int size()
	{
	    return (int)tail - (int)head;
	}
private:
    alignas(64) std::atomic<unsigned int> head;
    alignas(64) std::atomic<unsigned int> tail;
    alignas(64) const unsigned int capacity;
    alignas(64) T* data;
};


template <typename T>
class scspRingAtomicAlignedCache {
public:
    scspRingAtomicAlignedCache(unsigned int cap)
	:capacity(cap)
	{
	    if(std::popcount(capacity) != 1)
		throw std::exception {};
	    data = new (std::align_val_t(64)) T[cap];
	    head = 0;
	    tail = 0;
	    cons = {};
	    prod = {};
	}
    ~scspRingAtomicAlignedCache()
	{
	    delete [] data;
	}

    bool enqueue(T val)
	{
	    auto oldTail = tail.load();
	    auto curhead = head.load();
	    if(oldTail - curhead == capacity)
		return false;

	    auto tailTake = fastModForPow2(oldTail, capacity);
	    (data[tailTake]) = val;
	    tail = oldTail + 1;
	    return true;
	}
    
    bool enqueue_bulk(T val[], int count)
	{
	    auto isPlacepble = (tail - head + count) <capacity;
	    if(!isPlacepble)
	    {
		return false;
	    }
	    auto oldTail = tail.load();
	    auto newTail = tail+ count;
	    auto tailPlace = fastModForPow2(oldTail, capacity);
	    for(int i = 0 ; i < count; i++)
	    {
		data[tailPlace + i] = val[i];
	    }
	    tail = newTail;
	    return true;
	}

    bool dequeue(T* val)
	{
	    auto curTail = tail.load();
	    auto oldHead = cons.head.load();
	    if(curTail == oldHead)
		return false;

	    auto headTake = fastModForPow2(oldHead, capacity);
	    *val = (data[headTake]);
	    head = oldHead + 1;
	    return true;
	}
    bool dequeue_bulk(T* val,int size)
	{
	    auto curTail = tail.load();
	    auto oldHead = cons.head.load();
	    if(curTail == oldHead)
		return false;

	    auto headTake = fastModForPow2(oldHead, capacity);
	    *val = (data[headTake]);
	    head = oldHead + 1;
	    return true;
	}
    bool emptyDequeue()
	{
	    cons.tail == prod.head;
	}
    bool empty()
	{
	    return tail == head;
	}
    int size()
	{
	    return (int)tail - (int)head;
	}
private:
    alignas(64) struct 
    {
	std::atomic<unsigned int> head;
	std::atomic<unsigned int> tail;
    }prod;

    alignas(64) struct 
    {
	std::atomic<unsigned int> head;
	std::atomic<unsigned int> tail;
    }cons;

    alignas(64) std::atomic<unsigned int> head;
    alignas(64) std::atomic<unsigned int> tail;
    alignas(64) const unsigned int capacity;
    alignas(64) T* data;
};

template <typename T>
class scspRingNoAtomic : RingInterface<T>
{
    scspRingNoAtomic(int cap)
	{
	    data = new (std::align_val_t(std::hardware_destructive_interference_size)) T[capacity];
	    cons.head = 0;
	    cons.tail = 0;
	    prod.tail =0;
	    prod.head = 0;
	    capacity = cap;
	}

    ~scspRingNoAtomic()
	{
	    delete[] data;
	}

    struct alignas(std::hardware_destructive_interference_size)
    {
	volatile int  head;
	volatile int tail;
    }prod;

    struct alignas(std::hardware_destructive_interference_size)
    {
	volatile int head;
	volatile int tail;
    }cons;

    //change only producer, read consumer
    int capacity;
    T* data;
};
