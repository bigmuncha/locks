#include <cstdio>
#include <bit>
#include <atomic>
#include <array>
#include <cmath>
#include <assert.h>

//this must be like this, howewer 0x00010000 hibit is realy 5, wee need return 4.
static constexpr int highestBit(unsigned int val)
{
    return 32 - std::countl_zero(val) - 1;
}

static_assert(highestBit(8) == 3);


template <typename T, int powFactor = 3>
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
	    auto descriptor_next = new Descriptor{};
	     auto wd = new WriteDescriptor();
	    do{
		completeWrite(descriptor_current->writeop);
		int bucket = highestBit(descriptor_current->size + first_bucket_size) - highestBit(first_bucket_size);
		if(memory[bucket].load() == NULL)
		{
		    allocBucket(bucket);
		}
		*(wd) = WriteDescriptor{at(descriptor_current->size).load(), elem, descriptor_current->size, false};
		descriptor_next->size = descriptor_current->size + 1;
		descriptor_next->writeop.store(wd);
	    }while(desc.compare_exchange_strong(descriptor_current, descriptor_next));
	    completeWrite(descriptor_next->writeop);
	}
    T popBack()
	{
	    auto current_desc = desc;
	    auto last_elem = at(current_desc->size);
	    decltype(current_desc) descriptor_next = new Descriptor;
	    do{
		completeWrite(current_desc);
		last_elem = at(current_desc->size -1);
		WriteDescriptor* newWD = nullptr;
		descriptor_next = {current_desc->size - 1, newWD};
	    }while(desc.compare_exchange_strong(current_desc, descriptor_next));
	    return last_elem;
	}
    size_t size()
	{
	    Descriptor *descr = desc.load();
	    int size = descr->size;
	    if(!descr->writeop.load()->completed)
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

    void completeWrite(std::atomic<WriteDescriptor*>& wd)
	{
	    if(wd.load() != nullptr && !wd.load()->completed)
	    {
		at(wd.load()->pos).compare_exchange_weak(wd.load()->oldValue, wd.load()->newValue);
		wd.load()->completed = false;
	    }
	}

private:
    static constexpr int first_bucket_size = 1<< powFactor;
    std::atomic<Descriptor*> desc;
    static_assert(std::atomic<Descriptor*>{}.is_always_lock_free);
    std::atomic<T*> memory[first_bucket_size];
};
