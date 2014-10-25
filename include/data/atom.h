#ifndef __ATOM_H__
#define __ATOM_H__

typedef struct atom_ {
    struct atom_ *next;
    int len;
    byte *str;
} atom;

byte *atom_new(const char *str, int len);

#endif
