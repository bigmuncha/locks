#include <iostream>
#include <atomic>

struct node_t
{
    void *value;
    std::atomic<node_t*> next;
};

struct queue
{
    std::atomic<node_t*> Head;
    std::atomic<node_t*> Tail;
};

void initialize(queue* q)
{
    auto node = new struct node_t;
    node->next = nullptr;
    q->Head.store(node);
    q->Tail.store(node);
}


// ALGO:
// 1) create new node
// 2) take tail
// cond if( tail == real_tail)
//          3)take next = tail->next
//          cond:
///              if(next == nullptr) //expected(what we need)
//                   update tail->next(add node after)
//               else(somebody use my tail, but dont update it )
//                   update tail on next value
// real_tail = new_node
void enqueue(queue* q, void* value)
{
    auto node = new struct node_t();
    node->value = value;
    node->next = NULL;
    struct node_t* tail = nullptr;
    while(true)
    {
	tail = q->Tail.load();
	if(tail == q->Tail)
	{
	    auto next = tail->next.load();
	    if(next == nullptr)
	    {
		if(tail->next.compare_exchange_weak(next, node))
		    break;
            }
	    else
	    {
		q->Tail.compare_exchange_weak(tail, next);
	    }
	}
    }
    q->Tail.compare_exchange_weak(tail, node); // we dont care if it proper work
}

bool dequeue(queue* q, void** value)
{
    node_t* head =nullptr;
    while (true)
    {
	head =  q->Head.load();
	auto tail = q->Tail.load();
	auto real = head->next.load();
	if(head == q->Head.load())
	{
	    if(head == tail)
	    {
		if(real == nullptr)
		    return false;
		else
		    q->Tail.compare_exchange_weak(tail, real);
	    }
	    else
	    {
		if(q->Head.compare_exchange_weak(head, real))
		    break;
	    }
	}
    }
    //now  i can use head;
    *value = head;
    free(head);
}

int main()
{
    
}
