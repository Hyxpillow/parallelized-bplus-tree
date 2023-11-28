#ifndef _PAGER_H_
#define _PAGER_H_
#include <iostream>
#include <cstring>
#include <deque>
#include "parallel_rwlatch.hpp"

#define K 4
typedef unsigned int _data;

typedef enum {
    INDEX, LEAF
} Node_Type;

typedef struct node {
    Node_Type type;
    struct node* prev;
    struct node* next;
    struct node* parent;
    rwlatch latch;
    int size;
    int key[K + 1];
} Node;

typedef struct internal_node {
    Node_Type type;
    struct node* prev;
    struct node* next;
    struct node* parent;
    rwlatch latch;
    int size;
    int key[K + 1];
    struct node* children[K + 2];
} Internal_Node;

typedef struct leaf_node {
    Node_Type type;
    struct node* prev;
    struct node* next;
    struct node* parent;
    rwlatch latch;
    int size;
    int key[K + 1];
    _data data[K + 1];
} Leaf_Node;

class transaction_t
{
private:
    std::deque<Node*> lock_list;
public:
    void add_lock(Node* node)
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
            Node* last_lock = lock_list.back();
            last_lock->latch.unlock();
            lock_list.pop_back();
        }
    }

    Node* get_previous_lock()
    {
        return !lock_list.empty()?lock_list.back():nullptr;
    }
};
#endif