#include <iostream>
#include "page.h"

int _search(Node* node, int key) {
    int i;
    for (i = 0; i < node->size && node->key[i] < key; i++);
    return i;
}
void insert_child(Internal_Node* internal, int i, int key, v_addr child) {
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
    Internal_Node* root;
    BPtree() {
        init_page();
        root = (Internal_Node*)new_base(INDEX);
        Leaf_Node* leaf = (Leaf_Node*)new_base(LEAF);
        root->children[0] = leaf->addr;
        leaf->parent = root->addr;
    }
    ~BPtree() {
        free_page();
    }

    _data search(int key) {
        int i;
        transaction_t trans;
        Node* cursor = (Node*)root;
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            Node* parent = trans.get_previous_lock();
            if (parent)
            {
                parent->latch.unlock_shared();
            }
            
            cursor->latch.lock_shared();
            trans.add_lock(cursor);
            cursor = get_base(((Internal_Node*)cursor)->children[i]);
        }
        Leaf_Node* leaf = (Leaf_Node*)cursor;
        i = _search((Node*)leaf, key);
        if (i == leaf->size || leaf->key[i] != key)
            trans.free_all_locks();
            return 0; // key not found
        trans.free_all_locks();
        return leaf->data[i];
    }
    
    void insert(int key, _data data) {
        int i;
        Node* cursor = (Node*)root;
        transaction_t trans;
        root->latch.lock();

        // Search from root, to get the leaf node where we insert the data
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            cursor = get_base(((Internal_Node*)cursor)->children[i]);
            //Internal_Node* previous = trans.get_previous_lock();
            // Safe and exist node, unlock
            // if (previous && previous->size < K)
            // {
            //     previous->latch.unlock();
            // }
            // Add current  cursor into lock_list
            trans.add_lock(cursor);
            cursor->latch.lock();
        }
        Leaf_Node* leaf = (Leaf_Node*)cursor;
        i = _search((Node*)leaf, key);
        if (i != leaf->size && leaf->key[i] == key)
            return; // key found
        insert_data(leaf, i, key, data);

        if (leaf->size == K + 1) { // split LEAF
            int mid_key = leaf->key[K / 2];
            Leaf_Node* splited_leaf = (Leaf_Node*)new_base(LEAF);
            splited_leaf->prev = leaf->addr;
            splited_leaf->next = leaf->next;
            if (leaf->next != 0) {
                Leaf_Node* next_leaf = (Leaf_Node*)get_base(leaf->next);
                next_leaf->prev = splited_leaf->addr;
            }
            leaf->next = splited_leaf->addr;
            
            for (int j = 0; j < (K + 1) / 2; j++) { // fill new node
                insert_data(splited_leaf, j, leaf->key[K / 2 + 1], leaf->data[K / 2 + 1]);
                remove_data(leaf, K / 2 + 1);
            }
            Node* parent = get_base(leaf->parent);
            splited_leaf->parent = parent->addr; // set the same parent
            i = _search((Node*)parent, mid_key);

            ((Internal_Node*)parent)->children[i] = splited_leaf->addr;
            insert_child((Internal_Node*)parent, i, mid_key, leaf->addr);
            Internal_Node* index_cursor;
            while (parent->size == K + 1) { // split INDEX
                index_cursor = (Internal_Node*)parent;
                if (index_cursor->addr == 0) { // if spliting root, then copy $(root_addr) to a new node
                    Internal_Node* root_copy = (Internal_Node*)new_base(INDEX);
                    root_copy->parent = index_cursor->addr;
                    root_copy->children[0] = index_cursor->children[K + 1];
                    for (int i = 0; i < K + 1; i++) {
                        insert_child(root_copy, root_copy->size, index_cursor->key[0], index_cursor->children[0]);
                        remove_child(index_cursor, 0);
                    }
                    for (int i = 0; i <= K + 1; i++) {
                        Node* child = get_base(root_copy->children[i]);
                        child->parent = root_copy->addr;
                    }
                    index_cursor->children[0] = root_copy->addr;
                    index_cursor = root_copy;
                }
                mid_key = index_cursor->key[K / 2];
                Internal_Node* splited_internal = (Internal_Node*)new_base(INDEX);
                splited_internal->prev = index_cursor->addr;
                splited_internal->next = index_cursor->next;
                if (index_cursor->next != 0) {
                    Internal_Node* next_internal = (Internal_Node*)get_base(index_cursor->next);
                    next_internal->prev = splited_internal->addr;
                }
                index_cursor->next = splited_internal->addr;
                index_cursor->next = splited_internal->addr;
                splited_internal->prev = index_cursor->addr;

                Node* last_child = get_base(index_cursor->children[K + 1]);
                splited_internal->children[0] = last_child->addr;
                last_child->parent = splited_internal->addr;
                for (int j = 0; j < (K + 1) / 2; j++) { // fill new node
                    Node* child = get_base(index_cursor->children[K / 2 + 1]);
                    insert_child(splited_internal, j, index_cursor->key[K / 2 + 1], child->addr);
                    remove_child(index_cursor, K / 2 + 1);
                    child->parent = splited_internal->addr;
                }
                parent = get_base(index_cursor->parent);
                splited_internal->parent = parent->addr;
                i = _search((Node*)parent, mid_key);
                ((Internal_Node*)parent)->children[i] = splited_internal->addr;

                insert_child(((Internal_Node*)parent), i, mid_key, index_cursor->addr);
                index_cursor->size--;
            }
            
        }
        trans.free_all_locks();
        root->latch.unlock();
        std::cout << "Insert " << key << "successfully" << std::endl;
    }

    void remove(int key) {
        bool lock_next_next;
        int i;
        Node* cursor = (Node*)root;
        transaction_t trans;
        while (cursor->type != LEAF) {
            for (i = 0; i < cursor->size && cursor->key[i] < key; i++);
            trans.add_lock(cursor);
            cursor->latch.lock();
            cursor = get_base(((Internal_Node*)cursor)->children[i]);
        }
        // Get the leaf node
        Leaf_Node* leaf = (Leaf_Node*)cursor;
        leaf->latch.lock();
        trans.add_lock((Node*)leaf);

        i = _search((Node*)leaf, key);
        if (i == leaf->size || leaf->key[i] != key)
        {
            trans.free_all_locks();
            return; // key not found
        }
        remove_data(leaf, i); // leaf still exist, be careful for the lock

        // Handle Leaf
        if (leaf->size < (K + 1) / 2) { // leaf unbalanced
            Internal_Node* parent = (Internal_Node*)get_base(leaf->parent); // already locked
            Leaf_Node* next = (Leaf_Node*)get_base(leaf->next);
            next->latch.lock();
            trans.add_lock((Node*)next);
            Leaf_Node* prev = (Leaf_Node*)get_base(leaf->prev);
            prev->latch.lock();
            trans.add_lock((Node*)prev);

            if (leaf->next != ROOT_ADDR && next->parent == leaf->parent && next->size > (K + 1) / 2) { // borrow from next
                int borrow_key = next->key[0]; // new_key for cursor
                insert_data(leaf, leaf->size, borrow_key, next->data[0]);
                remove_data(next, 0);
                int new_key_for_leaf = borrow_key;
                i = _search((Node*)parent, key);
                parent->key[i] = borrow_key;
                trans.free_all_locks();
                return;
            } else if (leaf->prev != ROOT_ADDR && prev->parent == leaf->parent && prev->size > (K + 1) / 2) { // borrow from prev
                int borrow_key = prev->key[prev->size - 1];
                insert_data(leaf, 0, borrow_key, prev->data[prev->size - 1]);
                remove_data(prev, prev->size - 1);
                int new_key_for_prev = prev->key[prev->size - 1];
                i = _search((Node*)parent, new_key_for_prev);
                parent->key[i] = new_key_for_prev;
                trans.free_all_locks();
                return;
            } else if (next->parent == leaf->parent && next->addr != ROOT_ADDR) { // if next exist, then merge next
                
            } else if (prev->parent == leaf->parent && prev->addr != ROOT_ADDR) { // if prev exist, then merge prev
                next = leaf;
                leaf = prev;
            } else {
                trans.free_all_locks();
                return;
            }
            int new_key; // merge
            while (next->size > 0) {
                new_key = next->key[0];
                insert_data(leaf, leaf->size, new_key, next->data[0]);
                remove_data(next, 0);
            }
            Node* next_of_next = get_base(next->next);
            leaf->next = next_of_next->addr;
            next_of_next->prev = leaf->addr;
            next->valid = 0;
            
            i = _search((Node*)parent, new_key);
            parent->key[i - 1] = new_key;
            remove_child(parent, i);
            
            Internal_Node* index_cursor;
            // Handle Internal
            while (parent->size < (K + 1) / 2 && parent->addr != ROOT_ADDR) { // update INDEX
                index_cursor = parent;
                parent = (Internal_Node*)get_base(index_cursor->parent);
                Internal_Node* next = (Internal_Node*)get_base(index_cursor->next);
                Internal_Node* prev = (Internal_Node*)get_base(index_cursor->prev);
                
                next->latch.lock();
                trans.add_lock((Node*)next);
                prev->latch.lock();
                trans.add_lock((Node*)prev);

                if (next->addr != 0 && next->parent == index_cursor->parent && next->size > (K + 1) / 2) { // borrow from next
                    int key_up = next->key[0];
                    i = _search((Node*)parent, key_up) - 1;
                    int key_down = parent->key[i];
                    parent->key[i] = key_up;
                    Node* child = get_base(next->children[0]);
                    index_cursor->key[index_cursor->size] = key_down;
                    index_cursor->children[index_cursor->size + 1] = child->addr;
                    index_cursor->size++;
                    child->parent = index_cursor->addr;
                    remove_child(next, 0);
                    trans.free_all_locks();
                    return;
                } else if (prev->addr != 0 && prev->parent == index_cursor->parent && prev->size > (K + 1) / 2) { // borrow from prev  allow not exist key
                    int key_up = prev->key[prev->size - 1];
                    i = _search((Node*)parent, key_up);
                    int key_down = parent->key[i];
                    parent->key[i] = key_up;
                    Node* child = get_base(prev->children[prev->size]);
                    insert_child(index_cursor, 0, key_down, child->addr);
                    child->parent = index_cursor->addr;
                    remove_child(prev, prev->size);
                    trans.free_all_locks();
                    return;
                } else if (next->parent == index_cursor->parent && next->addr != ROOT_ADDR) { // if next exist, then merge next
                    lock_next_next = true;
                } else if (prev->parent == index_cursor->parent && prev->addr != ROOT_ADDR) { // if prev exist, then merge prev
                    next = index_cursor;
                    index_cursor = prev;
                } else {
                    trans.free_all_locks();
                    return;
                }
                // merge
                int sibling_first_key = next->key[0];
                i = _search((Node*)parent, sibling_first_key) - 1;
                int key_down = parent->key[i];
                index_cursor->key[index_cursor->size] = key_down;
                index_cursor->size++;
                Node* last_child = get_base(next->children[next->size]);
                index_cursor->children[index_cursor->size] = last_child->addr;
                last_child->parent = index_cursor->addr;

                while (next->size > 0) {
                    Node* child = get_base(next->children[0]);
                    insert_child(index_cursor, index_cursor->size, next->key[0], child->addr);
                    remove_child(next, 0);
                    child->parent = index_cursor->addr;
                }
                Node* next_of_next = get_base(next->next);
                if (lock_next_next)
                {
                    next_of_next->latch.lock();
                    trans.add_lock(next_of_next);
                }
                
                index_cursor->next = next_of_next->addr;
                next_of_next->prev = index_cursor->addr;
                next->valid = 0;

                parent->key[i] = parent->key[i + 1];
                remove_child(parent, i + 1);

                if (parent->addr == ROOT_ADDR && parent->size == 0) { 
                    // remove root
                    *parent = *index_cursor;
                    parent->addr = 0;
                    for (int i = 0; i <= K + 1; i++) {
                        Node* child = get_base(parent->children[i]);
                        child->parent = parent->addr;
                    }
                    index_cursor->valid = 0;
                }
            }
        }
        std::cout << "Remove " << key << "successfully" << std::endl;
        trans.free_all_locks();
        root->latch.unlock();
    }

    void display(Node* cursor, int depth) {
        if (depth == 0) {
            std::cout << "-----------------" << std::endl;
        }
        for (int i = 0; i < depth; i++) {
            std::cout << "    ";
        }
        std::cout << cursor->addr << ": ";

        for (int i = 0; i < cursor->size; i++)
            std::cout << cursor->key[i] << " ";
        std::cout << "(" << cursor->parent << "-" << cursor->prev << "-" << cursor->next << ")" << std::endl;
        if (cursor->type != LEAF) {
            for (int i = 0; i <= cursor->size; i++)
                display(get_base(((Internal_Node*)cursor)->children[i]), depth + 1);
        }
    }

};
int main() {
    BPtree t;
    _data tmp = 0;
    #pragma omp parallel for
    for (size_t i = 0; i < 50; i++)
    {
        t.insert(i, tmp++);
    }
    t.display((Node*)(t.root), 0);
    #pragma omp parallel for
    for (size_t i = 0; i < 25; i++)
    {
        t.remove(i);
    }
    // t.insert(9, tmp);
    // t.insert(23, tmp);
    // t.insert(24, tmp);
    // t.insert(25, tmp);
    // t.insert(26, tmp);
    // t.insert(27, tmp);
    // t.insert(28, tmp);
    // t.insert(29, tmp);
    // t.insert(30, tmp);

    // t.insert(33, tmp);
    // t.insert(34, tmp);
    // t.insert(35, tmp);
    // t.insert(45, tmp);
    // t.insert(85, tmp);
    // t.insert(94, tmp);
    // t.insert(899, tmp);
    // t.insert(1024, tmp);
    // t.insert(233, tmp);
    // t.insert(941, tmp);
    // t.insert(148, tmp);
    // t.insert(98, tmp);
    // t.insert(555, tmp);
    // t.insert(928, tmp);
    // t.insert(148, tmp);
    // t.insert(9842, tmp);
    // t.insert(281, tmp);
    // t.insert(199, tmp);
    // t.insert(47, tmp);
    // t.insert(99, tmp);
    // t.insert(67, tmp);
    
    // t.remove(9);
    // t.remove(23);
    // t.remove(24);
    // t.remove(25);
    // t.remove(26);
    // t.remove(27);
    // t.remove(28);
    // t.remove(29);
    // t.remove(30);    
    // t.remove(33);
    // t.remove(34);
    // t.remove(35);
    // t.remove(45);
    // t.remove(85);
    // t.remove(94);
    // t.remove(24);
    // t.remove(98);
    // t.remove(99);
    // t.remove(85);
    // t.remove(47);
    // t.remove(45);
    // t.remove(94);
    // t.remove(9);
    // t.remove(23);
    // t.remove(30);
    // t.remove(35);
    // t.remove(34);
    
    t.display((Node*)(t.root), 0);
    // #pragma omp parallel for
    // for (size_t i = 0; i < 50; i++)
    // {
    //     t.remove(i);
    // }
    // t.display((Node*)(t.root), 0);
    // int op;
    // int key;
    // while(1) {
    //     std::cin >> op >> key;
    //     if (op == 0) {
    //         t.insert(key, tmp);
    //     } else if (op == 1) {
    //         t.remove(key);
    //     }
    //     t.display((Node*)(t.root), 0);
    // }
    return 0;
}
