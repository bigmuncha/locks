#include <cstdio>
#include <bit>
#include <atomic>
#include <array>
#include <cmath>
#include <assert.h>
#include <vector>

// this is refferrence implementation of
// Lock-free Dynamically Resizable Arrays
// https://stroustrup.com/lock-free-vector.pdf
// by Damian Dechev, Peter Pirkelbauer, and Bjarne Stroustrup

//this must be like this, howewer 0x00010000 hibit is realy 5, wee need return 4.
static constexpr int highestBit(unsigned int val)
{
    return 32 - std::countl_zero(val) - 1;
}

static_assert(highestBit(8) == 3);

template <typename T, int powFactor = 3, int arenaSize = 256>
requires ((sizeof(T)) <= 8) // this vector works only with atomic always lockfree types
class vectorLF
{
    struct WriteDescriptor
    {
	T oldValue;
	T newValue;
	int pos;
        bool completed;
    };

    struct Descriptor
    {
	int size;
	std::atomic<WriteDescriptor*> writeop;
    };

public:
    vectorLF()
	{
	    desc = &dummy;
	    desc.load()->size = 0;
	    desc.load()->writeop = nullptr;
	    wdVect = std::vector<WriteDescriptor>(arenaSize);
	    descriptorVect = std::vector<Descriptor>(arenaSize);
	}
    ~vectorLF()
	{
	    for(int i =0; i < first_bucket_size; i++)
	    {
		if(memory[i] != nullptr)
		    delete [] memory[i];
	    }
	}



    void reserve(int size) // if size == 200
	{
	    int i = highestBit(desc.load()->size + first_bucket_size - 1) - highestBit(first_bucket_size); //-1
	    if(i < 0)
		i = 0; // 0
	    while ( i < highestBit(size + first_bucket_size - 1) - highestBit(first_bucket_size)) // from [0 to 4),0,1,2,3
	    {
		i = i + 1;
		allocBucket(i);
	    }
	}

    void pushBack(T elem)
	{
	    auto descriptor_current = desc.load();
	    auto descriptor_next = getThreadLocalDescriptor();
	    auto wd = getThreadLocalWriteDescriptor();
	    do{
		completeWrite(descriptor_current->writeop);
		int bucket = highestBit(descriptor_current->size + first_bucket_size) - highestBit(first_bucket_size);
		if(memory[bucket].load() == NULL)
		{
		    allocBucket(bucket);
		}
		auto oldval =  at(descriptor_current->size).load();
		wd->oldValue = oldval;
		wd->newValue = elem;
		wd->pos = descriptor_current->size;
		auto complete = std::atomic_ref(wd->completed);
		complete.store(true, std::memory_order_release);
		descriptor_next->size = descriptor_current->size + 1;
		descriptor_next->writeop.store(wd);
	    }while(desc.compare_exchange_strong(descriptor_current, descriptor_next));
	    completeWrite(descriptor_next->writeop);
	}

    T popBack()
	{
	    auto current_desc = desc.load();
	    T last_elem;
	    auto descriptor_next = getThreadLocalDescriptor();
	    do{
		completeWrite(current_desc->writeop);
		last_elem = at(current_desc->size -1);
		descriptor_next->size  = current_desc->size - 1;
		descriptor_next->writeop.store(nullptr);
	    }while(desc.compare_exchange_strong(current_desc, descriptor_next));
	    return last_elem;
	}

    size_t size()
	{
	    Descriptor *descr = desc.load();
	    int size = descr->size;
	    if(descr->writeop && descr->writeop.load()->completed)
	    {
		size = size - 1 ;
	    }
	    return size;
	}

    std::atomic_ref<T> at(int i )
	{
	    auto pos = i + first_bucket_size;
	    auto hibit = highestBit(pos);
	    auto idx = (pos xor (int)std::pow(2, hibit));
	    return std::atomic_ref<T>(memory[hibit - highestBit(first_bucket_size)][idx]);
	}

    T read(int index)
	{
	    *at(index).load();
	}

    void  write(int index, T val)
	{
	    at(index).store(val);
	}

    void allocBucket(int bucket)
	{
	    int bucket_size = std::pow(first_bucket_size, bucket + 1);
	    T* mem = new T[bucket_size];
	    auto oldMem = memory[bucket].load();
	    if(!memory[bucket].compare_exchange_weak(oldMem, mem))
	    {
		delete [] mem;
	    }
	}

    WriteDescriptor* getThreadLocalWriteDescriptor()
	{
	    thread_local WriteDescriptor odd_descriptor;
	    thread_local WriteDescriptor even_descriptor;
	    thread_local unsigned int flag = 0;
	    if((flag++) & 0x1)
		return &odd_descriptor;
	    else
		return &even_descriptor;
	}
    Descriptor* getThreadLocalDescriptor()
	{
	    thread_local Descriptor odd_descriptor;
	    thread_local Descriptor even_descriptor;
	    thread_local unsigned int flag;
	    if((flag++) & 0x1)
		return &odd_descriptor;
	    else
		return &even_descriptor;
	}

    void completeWrite(std::atomic<WriteDescriptor*>& wd)
	{
	    WriteDescriptor* wd_ptr = wd.load();
	    if(wd_ptr != nullptr)
	    {
		auto completed = std::atomic_ref(wd_ptr->completed);
		if(completed.load(std::memory_order_acquire) == true)
		{
		    std::atomic_ref<T> position = at(wd_ptr->pos);
		    position.compare_exchange_weak(wd_ptr->oldValue, wd_ptr->newValue);
		    completed.store(false, std::memory_order_relaxed);
		}
	    }
	}

    static constexpr unsigned int maxSize()
	{
	    int currentPower = powFactor;
	    int currentBase = 1;
	    int size = currentBase;
	    for (int i =0; i< powFactor; i++)
	    {
		currentBase = currentBase << currentPower;
		size += currentBase;
	    }
	    return size - 1;
	}
private:
    static constexpr int first_bucket_size = 1<< powFactor;
    std::atomic<Descriptor*> desc;
    Descriptor dummy;
    static_assert(std::atomic<Descriptor*>{}.is_always_lock_free);
    std::atomic<T*> memory[first_bucket_size];
    std::vector<Descriptor> descriptorVect;
    std::vector<WriteDescriptor> wdVect;
};

static_assert(vectorLF<int,4>::maxSize() == 69904);
static_assert(vectorLF<int,3>::maxSize() == 584);
static_assert(vectorLF<int,2>::maxSize() == 20);
