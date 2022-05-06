#ifndef _AVL_H_
#define _AVL_H_
#include <stdint.h>

typedef struct {
    union {
        uint32_t v;
        uint32_t card_id;
        long character_id;
    } key;
    union {
        uint32_t v;
        long character_id;
        uint32_t access;
    } data;
    uint32_t bitfield; // 12 bits leftchild, 12 bits rightchild, 2 bits balance
} avl_access_t;

extern int avl_num_entries[2];
int avl_insert_tree(int node, avl_access_t *entry);
void avl_insert(avl_access_t *entry, int ridx);
avl_access_t *avl_find(uint32_t key, int ridx);
void avl_degrade_access();
void avl_print_status();

#endif // _AVL_H_
