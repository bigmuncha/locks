#include <memory>
#include <atomic>

template <typename T>
class noReclaimstackLF
{
    struct node
    {
	T value;
	node* next;
    };

    void push(T val)
	{
	    node* newHead = new node;
	    newHead->value = val;

	    auto oldHead = head.load();
	    newHead->next = oldHead;
	    while(!head.compare_exchange_strong(newHead->next, newHead))
	    {
	    }
	}

    T pop()
	{
	    node *oldHead = head.load();
	    if(oldHead == nullptr)
		return;

	    while(!head.compare_exchange_strong(oldHead, oldHead->next))
	    {
	    }
	    return oldHead->value;
	    //reclaim
	}
    std::atomic<node*> head;
};

template <typename T>
class reclaimstackLF
{
    struct node
    {
	T value;
	node* next;
    };

    void push(T val)
	{
	    node* newHead = new node;
	    newHead->value = val;

	    auto oldHead = head.load();
	    newHead->next = oldHead;
	    while(!head.compare_exchange_strong(newHead->next, newHead))
	    {
	    }
	}

    T pop()
	{
	    thread_in_pop.fetch_add(1);
	    node *oldHead = head.load();
	    if(oldHead == nullptr)
		return;

	    while(!head.compare_exchange_strong(oldHead, oldHead->next))
	    {
	    }
	    auto val = oldHead->value;
	    try_reclaime(oldHead);
	    return val;
	    //reclaim
	}
    void deleteNodex(node* header)
	{
	    while(header)
	    {
		auto tmp  = header;
		header = header->next;
		delete tmp;
	    }
	}
    void try_reclaime(node* reclaim_node)
	{
	    if(thread_in_pop.load() != 1)
	    {
		thread_in_pop--;
	    }
	    else
	    {
		auto to_delete = reclamator.exchange(nullptr);
		if(--thread_in_pop == 0)
		{
		    deleteNodex(to_delete);
		}
		else
		{
		    
		}
		delete reclaim_node;
	    }
	}
    void chain_pending_nodes(node* nodes)
	{
	    
	}
    std::atomic<node*> head;
    std::atomic<node*> reclamator;
    std::atomic<int> thread_in_pop;
};
