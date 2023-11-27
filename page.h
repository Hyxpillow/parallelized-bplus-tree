#ifndef _PAGER_H_
#define _PAGER_H_
#include <iostream>
#include <deque>
#include <cstring>
#include "parallel_rwlatch.hpp"

#define K 4
#define DATA_INDEX_BITS 8
#define DATA_INDEX_MASK ((1 << DATA_INDEX_BITS) - 1)
#define PAGE_SIZE 2048

#define ROOT_ADDR 0

typedef unsigned int v_addr;
typedef unsigned int _data;
typedef unsigned char byte;

typedef enum {
    INDEX, LEAF
} Node_Type;

typedef struct node {
    int valid;
    Node_Type type;
    v_addr addr;
    v_addr prev;
    v_addr next;
    v_addr parent;
    rwlatch latch;
    int size;
    int key[K + 1];
} Node;

typedef struct leaf_node {
    int valid;
    Node_Type type;
    v_addr addr;
    v_addr prev;
    v_addr next;
    v_addr parent;
    rwlatch latch;
    int size;
    int key[K + 1];
    _data data[K + 1];
} Leaf_Node;

typedef struct internal_node {
    int valid;
    Node_Type type;
    v_addr addr;
    v_addr prev;
    v_addr next;
    v_addr parent;
    rwlatch latch;
    int size;
    int key[K + 1];
    v_addr children[K + 2];
} Internal_Node;

typedef struct page_header {
    Node_Type type;
    unsigned int node_size;
    unsigned int count;
} Page_Header;

class transaction_t
{
private:
    std::deque<Internal_Node*> lock_list;
public:
    void add_lock(Internal_Node* node)
    {
        lock_list.push_back(node);
    }
    
    void free_all_locks()
    {
        while (!lock_list.empty())
        {
            free_last_lock();
        }
    }

    void free_last_lock()
    {
        if (!lock_list.empty())
        {
            Internal_Node* last_lock = lock_list.back();
            last_lock->latch.unlock();
            lock_list.pop_back();
        }
    }

    Internal_Node* get_previous_lock()
    {
        return !lock_list.empty()?lock_list.back():nullptr;
    }
};

void init_page();
void free_page();
Node* get_base(v_addr addr);
Node* new_base(Node_Type Node_Type);
int new_page(Node_Type Node_Type);

#endif