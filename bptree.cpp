#include <iostream>
#include "bptree.h"

Node* new_node(Node_Type node_type) {
    Node* node;
    if (node_type == LEAF)
        node = (Node*)malloc(sizeof(Leaf_Node));
    else
        node = (Node*)malloc(sizeof(Internal_Node));
    node->type = node_type;
    node->size = 0;
    for (int i = 0; i <= K; i++)
        node->key[i] = 0;
    node->next = NULL;
    node->parent = NULL;
    node->prev = NULL;
    return node;
}

int _search(Node* node, int key) {
    int i;
    for (i = 0; i < node->size && node->key[i] < key; i++);
    return i;
}
void insert_child(Internal_Node* internal, int i, int key, Node* child) {
    internal->children[internal->size + 1] = internal->children[internal->size];
    for (int j = internal->size; j > i; j--) {
        internal->key[j] = internal->key[j - 1];
        internal->children[j] = internal->children[j - 1];
    }
    internal->key[i] = key;
    internal->children[i] = child;
    internal->size++;
}

void remove_child(Internal_Node* internal, int i) {
    for (int j = i; j < internal->size - 1; j++) {
        internal->key[j] = internal->key[j + 1];
        internal->children[j] = internal->children[j + 1];
    }
    if (i < internal->size)
        internal->children[internal->size - 1] = internal->children[internal->size];
    internal->size--;
}

void insert_data(Leaf_Node* leaf, int i, int key, _data data) {
    for (int j = leaf->size; j > i; j--) {
        leaf->key[j] = leaf->key[j - 1];
        leaf->data[j] = leaf->data[j - 1];
    }
    leaf->key[i] = key;
    leaf->data[i] = data;
    leaf->size++;
}

void remove_data(Leaf_Node* leaf, int i) {
    for (int j = i; j < leaf->size - 1; j++) {
        leaf->key[j] = leaf->key[j + 1];
        leaf->data[j] = leaf->data[j + 1];
    }
    leaf->size--;
}

class BPtree {
public:
    Node* root;
    BPtree() {
        root = new_node(INDEX);
        Node* leaf = new_node(LEAF);
        ((Internal_Node*)root)->children[0] = leaf;
        leaf->parent = root;
    }

    _data search(int key) {
        transaction_t trans;
        int i;
        Node* cursor = root;
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            Node* parent = cursor;
            parent->latch.lock_shared();
            trans.add_lock(parent);
            cursor = ((Internal_Node*)cursor)->children[i];
            trans.free_all_shared_locks();
            trans.add_lock(cursor);
        }
        Node* leaf = cursor;
        i = _search(leaf, key);
        if (i == leaf->size || leaf->key[i] != key)
            return 0; // key not found
        return ((Leaf_Node*)leaf)->data[i];
    }
    
    void insert(int key, _data data) {
        transaction_t trans;
        int i;
        Node* cursor = root;
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            Node* parent = cursor;
            parent->latch.lock();
            trans.add_lock(parent);
            cursor = ((Internal_Node*)cursor)->children[i];
            //cursor->latch.lock();
            if (cursor->size < K)
            {
                trans.free_all_locks();
            }
            trans.add_lock(cursor);
        }
        Node* leaf = cursor;
        i = _search(leaf, key);
        if (i != leaf->size && leaf->key[i] == key)
        {
            trans.free_all_locks();
            return; // key found
        }
        insert_data((Leaf_Node*)leaf, i, key, data);

        if (leaf->size == K + 1) { // split LEAF
            int mid_key = leaf->key[K / 2];
            Node* splited_leaf = new_node(LEAF);
            splited_leaf->prev = leaf;
            splited_leaf->next = leaf->next;
            if (leaf->next != 0) {
                Node* next_leaf = leaf->next;
                next_leaf->prev = splited_leaf;
            }
            leaf->next = splited_leaf;
            
            for (int j = 0; j < (K + 1) / 2; j++) { // fill new node
                insert_data(((Leaf_Node*)splited_leaf), j, leaf->key[K / 2 + 1], ((Leaf_Node*)leaf)->data[K / 2 + 1]);
                remove_data(((Leaf_Node*)leaf), K / 2 + 1);
            }
            Node* parent = leaf->parent;
            splited_leaf->parent = parent; // set the same parent
            i = _search(parent, mid_key);

            ((Internal_Node*)parent)->children[i] = splited_leaf;
            insert_child(((Internal_Node*)parent), i, mid_key, leaf);

            while (parent->size == K + 1) { // split INDEX
                cursor = parent;
                if (cursor == root) { // if spliting root, then copy to a new node
                    Node* root_copy = new_node(INDEX);
                    root_copy->parent = cursor;
                    ((Internal_Node*)root_copy)->children[0] = ((Internal_Node*)cursor)->children[K + 1];
                    for (int i = 0; i < K + 1; i++) {
                        insert_child(((Internal_Node*)root_copy), root_copy->size, cursor->key[0], ((Internal_Node*)cursor)->children[0]);
                        remove_child(((Internal_Node*)cursor), 0);
                    }
                    for (int i = 0; i <= K + 1; i++) {
                        Node* child = ((Internal_Node*)root_copy)->children[i];
                        child->parent = root_copy;
                    }
                    ((Internal_Node*)cursor)->children[0] = root_copy;
                    cursor = root_copy;
                }
                mid_key = cursor->key[K / 2];
                Node* splited_internal = new_node(INDEX);
                splited_internal->prev = cursor;
                splited_internal->next = cursor->next;
                if (cursor->next != 0) {
                    Node* next_internal = cursor->next;
                    next_internal->prev = splited_internal;
                }
                cursor->next = splited_internal;
                cursor->next = splited_internal;
                splited_internal->prev = cursor;

                Node* last_child = ((Internal_Node*)cursor)->children[K + 1];
                ((Internal_Node*)splited_internal)->children[0] = last_child;
                last_child->parent = splited_internal;
                for (int j = 0; j < (K + 1) / 2; j++) { // fill new node
                    Node* child = ((Internal_Node*)cursor)->children[K / 2 + 1];
                    insert_child(((Internal_Node*)splited_internal), j, cursor->key[K / 2 + 1], child);
                    remove_child(((Internal_Node*)cursor), K / 2 + 1);
                    child->parent = splited_internal;
                }
                parent = cursor->parent;
                splited_internal->parent = parent;
                i = _search(parent, mid_key);
                ((Internal_Node*)parent)->children[i] = splited_internal;
                insert_child(((Internal_Node*)parent), i, mid_key, cursor);
                cursor->size--;
            }
        }
        trans.free_all_locks();
    }

    void remove(int key) {
        transaction_t trans;
        int i;
        Node* cursor = root;
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            Node* parent = cursor;
            parent->latch.lock();
            trans.add_lock(parent);
            cursor = ((Internal_Node*)cursor)->children[i];
            if (cursor->size > (K+1)/2)
            {
                trans.free_all_locks();
            }
            trans.add_lock(cursor);
        }
        Node* leaf = cursor;
        i = _search(leaf, key);
        if (i == leaf->size || leaf->key[i] != key)
        {
            trans.free_all_locks();
            return; // key not found
        }
        remove_data((Leaf_Node*)leaf, i);

        if (leaf->size < (K + 1) / 2) { // leaf unbalanced
            Node* parent = leaf->parent;
            Node* next = leaf->next;
            Node* prev = leaf->prev;
            if (leaf->next != NULL && next->parent == leaf->parent && next->size > (K + 1) / 2) { // borrow from next
                int borrow_key = next->key[0]; // new_key for cursor
                insert_data(((Leaf_Node*)leaf), leaf->size, borrow_key, ((Leaf_Node*)next)->data[0]);
                remove_data(((Leaf_Node*)next), 0);
                int new_key_for_leaf = borrow_key;
                i = _search(parent, key);
                parent->key[i] = borrow_key;
                trans.free_all_locks();
                return;
            } else if (leaf->prev != NULL && prev->parent == leaf->parent && prev->size > (K + 1) / 2) { // borrow from prev
                int borrow_key = prev->key[prev->size - 1];
                insert_data(((Leaf_Node*)leaf), 0, borrow_key, ((Leaf_Node*)prev)->data[prev->size - 1]);
                remove_data(((Leaf_Node*)prev), prev->size - 1);
                int new_key_for_prev = prev->key[prev->size - 1];
                i = _search(parent, new_key_for_prev);
                parent->key[i] = new_key_for_prev;
                trans.free_all_locks();
                return;
            } else if (next != NULL && next->parent == leaf->parent) { // if next exist, then merge next
                
            } else if (prev != NULL && prev->parent == leaf->parent) { // if prev exist, then merge prev
                next = leaf;
                leaf = prev;
            } else {
                trans.free_all_locks();
                return;
            }
            int new_key; // merge
            while (next->size > 0) {
                new_key = next->key[0];
                insert_data((Leaf_Node*)leaf, leaf->size, new_key, ((Leaf_Node*)next)->data[0]);
                remove_data((Leaf_Node*)next, 0);
            }
            Node* next_of_next = next->next;
            leaf->next = next_of_next;
            if (next_of_next != NULL)
                next_of_next->prev = leaf;
            
            i = _search(parent, new_key);
            parent->key[i - 1] = new_key;
            remove_child((Internal_Node*)parent, i);
            while (parent->size < (K + 1) / 2 && parent != NULL) { // update INDEX
                cursor = parent;
                parent = cursor->parent;
                Node* next = cursor->next;
                Node* prev = cursor->prev;
                // next->latch.lock();
                // prev->latch.lock();
                // trans.add_lock(next);
                // trans.add_lock(prev);

                if (next != 0 && next->parent == cursor->parent && next->size > (K + 1) / 2) { // borrow from next
                    int key_up = next->key[0];
                    i = _search(parent, key_up) - 1;
                    int key_down = parent->key[i];
                    parent->key[i] = key_up;
                    Node* child = ((Internal_Node*)next)->children[0];
                    cursor->key[cursor->size] = key_down;
                    ((Internal_Node*)cursor)->children[cursor->size + 1] = child;
                    cursor->size++;
                    child->parent = cursor;
                    remove_child((Internal_Node*)next, 0);
                    trans.free_all_locks();
                    return;
                } else if (prev != 0 && prev->parent == cursor->parent && prev->size > (K + 1) / 2) { // borrow from prev  allow not exist key
                    int key_up = prev->key[prev->size - 1];
                    i = _search(parent, key_up);
                    int key_down = parent->key[i];
                    parent->key[i] = key_up;
                    Node* child = ((Internal_Node*)prev)->children[prev->size];
                    insert_child(((Internal_Node*)cursor), 0, key_down, child);
                    child->parent = cursor;
                    remove_child(((Internal_Node*)prev), prev->size);
                    trans.free_all_locks();
                    //std::cout << "Remove " << key << " I am good at 1" <<std::endl;
                    return;
                } else if (next != NULL && next->parent == cursor->parent) { // if next exist, then merge next

                } else if (prev != NULL && prev->parent == cursor->parent) { // if prev exist, then merge prev
                    next = cursor;
                    cursor = prev;
                } else {
                    trans.free_all_locks();
                    return;
                }
                // merge
                //std::cout << "Remove " << key << " I am good at 2" <<std::endl;
                int sibling_first_key = next->key[0];
                i = _search(parent, sibling_first_key) - 1;
                int key_down = parent->key[i];
                cursor->key[cursor->size] = key_down;
                cursor->size++;
                Node* last_child = ((Internal_Node*)next)->children[next->size];
                ((Internal_Node*)cursor)->children[cursor->size] = last_child;
                last_child->parent = cursor;

                while (next->size > 0) {
                    Node* child = ((Internal_Node*)next)->children[0];
                    insert_child((Internal_Node*)cursor, cursor->size, next->key[0], child);
                    remove_child((Internal_Node*)next, 0);
                    child->parent = cursor;
                }
                Node* next_of_next = next->next;
                cursor->next = next_of_next;
                if (next_of_next != NULL)
                    next_of_next->prev = cursor;

                parent->key[i] = parent->key[i + 1];
                remove_child((Internal_Node*)parent, i + 1);

                if (parent == root && parent->size == 0) { 
                    // free(root);
                    root = cursor;
                    root->parent = NULL;
                }
                //std::cout << "Remove " << key << " I am good at 3" <<std::endl;
            }
        }
        trans.free_all_locks();
    }

    void display(Node* cursor, int depth) {
        if (depth == 0) {
            std::cout << "-----------------" << std::endl;
        }
        for (int i = 0; i < depth; i++) {
            std::cout << "    ";
        }
        std::cout << cursor << ": ";

        for (int i = 0; i < cursor->size; i++)
            std::cout << cursor->key[i] << " ";
        std::cout << "(" << cursor->parent << "-" << cursor->prev << "-" << cursor->next << ")" << std::endl;
        if (cursor->type != LEAF) {
            for (int i = 0; i <= cursor->size; i++)
                display(((Internal_Node*)cursor)->children[i], depth + 1);
        }
    }

};

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " n_insert n_delete n_search n_threads" << std::endl;
        return 0;
    }
    int n_insert = atoi(argv[1]);
    int n_delete = atoi(argv[2]);
    int n_search = atoi(argv[3]);
    int n_threads = atoi(argv[4]);

    BPtree t;
    double start_time, end_time;
    srand(time(NULL)); 

    start_time = omp_get_wtime();
    #pragma omp parallel for num_threads(n_threads)
    for (int i = 0; i < n_insert; i++) {
        t.insert(rand() % n_insert, 0);
    }
    #pragma omp parallel for num_threads(n_threads)
    for (int i = 0; i < n_delete; i++) {
        t.remove(rand() % n_delete);
    }
    #pragma omp parallel for num_threads(n_threads)
    for (int i = 0; i < n_search; i++) {
        t.search(rand() % n_search);
    }
    end_time = omp_get_wtime();
    t.display((t.root), 0);
    std::cout << "Execute time: " << end_time-start_time << std::endl;
    return 0;
}
