#include "page.h"

static byte page_list[1024][PAGE_SIZE];
static int page_count;

void init_page() {
    int root_page = new_page(INDEX);
    int leaf_page = new_page(LEAF);
}

void free_page() {
    // for (int i = 0; i < page_count; i++)
    //     free(page_list[i]);
    // free(page_list);
}

Node* get_base(v_addr addr) {
    int page_index = addr >> DATA_INDEX_BITS;
    int data_index = addr & DATA_INDEX_MASK;
    byte* page = page_list[page_index];
    Page_Header* header = (Page_Header*)page;
    byte* base = page + sizeof(Page_Header) + data_index * header->node_size;
    return (Node*)base;
}

int new_page(Node_Type node_type) { // return its addr
    byte* page = (byte*)&page_list[page_count];
    Page_Header* header = (Page_Header*)page;
    header->type = node_type;
    if (node_type == LEAF) {
        header->node_size = sizeof(Leaf_Node);
    } else {
        header->node_size = sizeof(Internal_Node);
    }
    page_count++;
    return page_count - 1;
}


Node* new_base(Node_Type node_type) {
    Node* cursor = NULL;
    #pragma omp critical
    {
        int target_page = -1;
        Page_Header* header;
        for (int i = 0; i < page_count; i++) {
            header = (Page_Header*)page_list[i];
            if (header->type == node_type && sizeof(Page_Header) + ((header->count + 1) * header->node_size) < PAGE_SIZE) {
                target_page = i;
                break;
            }     
        }
        if (target_page == -1) {
            target_page = new_page(node_type);
            header = (Page_Header*)page_list[target_page];
        }
        int node_idx = 0;
        
        while (1) {
            int offset = sizeof(Page_Header) + node_idx * header->node_size;
            cursor = (Node*)((byte*)header + offset);
            if (cursor->valid == 0)
                break;
            node_idx++;
        }
        memset(cursor, 0, header->node_size);
        cursor->valid = 1;
        cursor->type = node_type;
        cursor->addr = (target_page << DATA_INDEX_BITS) | node_idx;
        
        header->count++;
    }
    return cursor;
}
