

#include <cstdint>
#include <memory>
#include <cstring>
#include <bit>
#include <iostream>
#include <sstream>

class lcTree
{
    static constexpr unsigned long long expand_mask = (1ll << 63);
    static constexpr uint16_t block_size = (1 << 8);
    static constexpr uint8_t index_size = sizeof(uint64_t);
    static constexpr uint8_t bits_in_ip = 8;
    static constexpr uint64_t maxUInt = -1;
    static constexpr uint64_t flushMask = maxUInt >> 1;
public:
    lcTree(int size)
	{
	    free_pool_index =0;
	    std::memset(first_stride, 0, block_size);
	    data_pool = allocate_pool(size * index_size * block_size);
	}
    ~lcTree()
	{
	    delete[] data_pool;
	}
    bool isPowOf2(uint8_t num)
	{
	    return std::popcount(num) == 1;
	}
    uint8_t getRightEdge(uint8_t num, int remainMask)
	{
	    int currentIndex = remainMask;
	    int i = 0;
	    while(i != remainMask)
	    {
		num|= (1<<i);
		i++;
	    }
	    return num;
	}
    std::string ipToStr(uint8_t ip[16])
	{
	    std::stringstream ip_str;
	    for(int i = 0; i < 16; i++)
	    {
		ip_str << (int)ip[i] <<':';
	    }
	    return ip_str.str();
	}

    void applyMask(uint8_t ip[16], uint8_t maskLen)
	{
	    uint8_t max_depth = maskLen / bits_in_ip;
	    int firstIndex = max_depth;
	    if(isPowOf2(maskLen))
	    {
		memset(ip + firstIndex, 0, 16 - firstIndex); //fill remain 
	    }
	    else
	    {
		auto filled_mask =maskLen -  (max_depth * bits_in_ip);
		uint8_t number = 0b10000000;
		while(--filled_mask)
		{
		    number |= (number >> 1);
		}
		ip[firstIndex] = ip[firstIndex] & number;
		memset(ip + firstIndex + 1, 0, 16 - firstIndex- 1); //fill remain 
	    }
	}

    void fillNh(uint64_t stridePtr[256], uint8_t leftEdge, uint8_t rightEdge, uint32_t nh)
	{
	    std::cout << "expand from " <<
		(int) leftEdge << " to " <<
		(int)rightEdge << " in stride " << stridePtr << " nh " << nh << '\n';
	    for(int i = leftEdge; i < rightEdge; i++)
	    {
		if(stridePtr[i] & expand_mask)
		{
		    continue;
		}
		else
		{
		    stridePtr[i] = nh;
		}
	    }
	}

    void add(unsigned char ip[16], const uint8_t maskLen, u_int32_t nh)
	{
	    uint8_t max_depth = maskLen / bits_in_ip;
	    int i = 0;
	    auto stridePtr = first_stride;
	    std::cout <<" original ip " <<  ipToStr(ip) <<"\n";
	    std::cout <<" masklen " << (int)maskLen << " depth " << (int)max_depth << '\n';
	    applyMask(ip, maskLen);
	    std::cout <<" masked ip " <<  ipToStr(ip) <<"\n";
	    bool prev_nh = false;
	    int fill_nh = 0;
	    while(i != max_depth) // expand to next block
	    {
		std::cout << " try install byte " << (int)ip[i] << "in stride " << stridePtr;
		auto ptrToIndex = &stridePtr[ip[i]];
		i++;
		if(*ptrToIndex & expand_mask)
		{
		    std::cout << " there is some ";
		    auto existing_pool_index = ((*ptrToIndex & flushMask));  //shift expand mask;
		    auto existing_pool = getDataPool(existing_pool_index);
		    stridePtr = existing_pool;
		}
		else
		{
		    if(prev_nh == false)
		    {
			fill_nh = *ptrToIndex;
			prev_nh = true;
		    }
		    fillNh(stridePtr, 0, 0xff, fill_nh);
		    auto free_pool = (free_pool_index * block_size);
		    ++free_pool_index;

		    auto newPtr = getDataPool(free_pool);
		    std::cout << " and allocate " << newPtr ;
		    *ptrToIndex = (expand_mask | free_pool);
		    std::cout << " expand "<< ptrToIndex;
		    stridePtr = newPtr;
		}
		std::cout <<'\n';
	    }
	    // new stride is find, now we need get real reange
	    auto leftBarrier = ip[i];
	    auto remainMask = maskLen - (8 * max_depth);
	    auto rightBarrier = getRightEdge(leftBarrier, remainMask);
	    ///fill edges
	    if(isPowOf2(maskLen))
	    {
		rightBarrier = block_size - 1;
	    }
	    fillNh(stridePtr, 0,0xff, fill_nh);
	    fillNh(stridePtr, leftBarrier, rightBarrier, nh);
	}
    void add(std::array<uint8_t,16> ip, const uint8_t maskLen, u_int32_t nh)
	{
	    std::cout << " add value\n \n";
	    add(ip.data(), maskLen, nh);
	}
    uint32_t lookup(uint8_t ip[16])
	{
	    auto stridePtr = first_stride;
	    int index = 0;
	    do
	    {
		auto ptrToStride = &stridePtr[ip[index]];

		if(!(*ptrToStride & expand_mask))
		{
		    std::cout << " no expand for stride " << stridePtr << '\n'; 
		    return *ptrToStride;
		}
		std::cout << "current ip byte " << (int) ip[index] << " stride " << stridePtr<< '\n';
		auto nextStride = (*ptrToStride & flushMask); //shift
		std::cout << " find next ptr stride " << nextStride <<"\n";
		stridePtr = getDataPool(nextStride);
		index++;
	    }while(true);
	}
    uint32_t lookup(std::array<uint8_t,16> ip)
	{
	    std::cout << " make lookup\n \n";
	    return lookup(ip.data());
	}

private:

    uint64_t* getDataPool(unsigned int index)
	{
	    auto new_data = data_pool + index;
	    std::cout <<" get data from index " << index << '\n';
	    return new_data;
	}

    uint64_t * allocate_pool(unsigned int size)
	{
	    uint64_t* newMemory = static_cast<uint64_t*>(std::aligned_alloc(std::hardware_destructive_interference_size, size));
	    std::memset(newMemory, 0, size);
	    return newMemory;
	}
    uint64_t first_stride[block_size];
    uint64_t *data_pool;
    uint16_t free_pool_index;
};
