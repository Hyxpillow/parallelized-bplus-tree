#ifndef _PAGER_H_
#define _PAGER_H_
#include <iostream>
#include <cstring>

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
    int size;
    int key[K + 1];
} Node;

typedef struct internal_node {
    Node_Type type;
    struct node* prev;
    struct node* next;
    struct node* parent;
    int size;
    int key[K + 1];
    struct node* children[K + 2];
} Internal_Node;

typedef struct leaf_node {
    Node_Type type;
    struct node* prev;
    struct node* next;
    struct node* parent;
    int size;
    int key[K + 1];
    _data data[K + 1];
} Leaf_Node;

#endif