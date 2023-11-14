#ifndef _PAGER_H_
#define _PAGER_H_
#include <iostream>

#define K 4
#define DATA_INDEX_BITS 8
#define DATA_INDEX_MASK ((1 << DATA_INDEX_BITS) - 1)
#define PAGE_SIZE 2048

#define DATA_SIZE 32
#define ROOT_ADDR 0
#define ADDRESS_SIZE sizeof(v_addr)

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
    int size;
    int key[K + 1];
    v_addr children[K + 2];
} Internal_Node;

typedef struct page_header {
    Node_Type type;
    unsigned int node_size;
    unsigned int count;
    unsigned int data_size;
} Page_Header;

void init_page();
void free_page();
Node* get_base(v_addr addr);
Node* new_base(Node_Type Node_Type);
int new_page(Node_Type Node_Type);

#endif