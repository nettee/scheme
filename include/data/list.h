#ifndef __LIST_H__
#define __LIST_H__

#include "data/atom.h"

typedef struct listnode_ {
    int ntype;
    struct listnode_ *next;
    union {
        atom *item;
        struct listnode_ *sub;
    };
} listnode;

typedef listnode *list;

list make_nil();

list read_from_tokens();

void print_list(list p);

#endif 
