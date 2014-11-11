#ifndef __ATOM_H__
#define __ATOM_H__

#include "common.h"

typedef struct atom_ {
    struct atom_ *next;
    int type;
    int len;
    byte *str;
} atom;

atom *atom_new(const char *str, int type, int len);
char *atom_repr(atom *);
bool atom_sameword(atom *, const char *);

#endif
