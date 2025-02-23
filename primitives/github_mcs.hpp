/*
 * atomic.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_
#include <stdint.h>

#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")
#define MEM_BARRIER() __sync_synchronize()

#define CAS(address,oldValue,newValue) __sync_bool_compare_and_swap(address,oldValue,newValue)
#define CAS_RET_VAL(address,oldValue,newValue) __sync_val_compare_and_swap(address,oldValue,newValue)
#define ADD_AND_FETCH(address,offset) __sync_add_and_fetch(address,offset)
#define FETCH_AND_ADD(address,offset) __sync_fetch_and_add(address,offset)

#define ATOMIC_LOAD(x) ({COMPILER_BARRIER(); *(x);})
#define ATOMIC_STORE(x, v) ({COMPILER_BARRIER(); *(x) = v; MEM_BARRIER(); })
#define ATOMIC_EXCHANGE(address, val) __atomic_exchange_n(address, val, __ATOMIC_SEQ_CST)
#define PAUSE() __asm__ __volatile__("pause\n" : : : "memory")
#define CACHE_ALIGN_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_ALIGN_SIZE)))


/*
 * MCSSpinLock.h
 *
 *  Created on: 2017年6月12日
 *      Author: zxy
 */

#ifndef MCSSPINLOCK_H_
#define MCSSPINLOCK_H_
#include <atomic>
#include <thread>
#include <iostream>
class WorkingGithubImplementation {
    struct MCSNode {
    bool waiting_;
    MCSNode *next_;
};
public:
    [[gnu::noinline]] void lock() {
        MCSNode *tls_node = get_tls_node();
        tls_node->next_ = nullptr;
        MCSNode *prev_gnode = ATOMIC_EXCHANGE(&gnode_, tls_node);
        if (prev_gnode==nullptr) {
            return;
        }
        tls_node->waiting_ = true;
        COMPILER_BARRIER();
        prev_gnode->next_ = tls_node;
        while (ATOMIC_LOAD(&tls_node->waiting_)) {
            PAUSE();
        }
        MEM_BARRIER();
    }
    [[gnu::noinline]] void unlock() {
        MEM_BARRIER();
        MCSNode *const tls_node = get_tls_node();
        if (tls_node->next_==nullptr) {
            if (CAS(&gnode_, tls_node, nullptr)) {
                return;
            }
	    std::cout << "cas error no deadlock"<< std::endl;
            while (!ATOMIC_LOAD(&tls_node->next_)) {
                PAUSE();
            }
        }
        tls_node->next_->waiting_ = false;
    }
private:
    MCSNode* gnode_ = nullptr;
    std::atomic<int> flager{};
    MCSNode *get_tls_node() {
        static __thread MCSNode tls_node;
	if(flager == 0)
	{
	    ++flager;
	    //std::cout << " node 1 address: " << &tls_node << " thread is "<< std::this_thread::get_id() <<std::endl;
	}
	else if(flager == 1)
	{
	    ++flager;
	    //    std::cout << " node 2 address: " << &tls_node <<  " thread is "<< std::this_thread::get_id() <<std::endl;
	}
        return &tls_node;
    }
};
#endif /* MCSSPINLOCK_H_ */

#endif /* ATOMIC_H_ */
