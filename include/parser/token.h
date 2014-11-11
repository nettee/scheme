#ifndef __TOKEN_H__
#define __TOKEN_H__

#include "data/atom.h"

#define NR_TK 32

typedef struct token {
    int type;
    atom *ap;
} Token;

Token pop_token();
Token first_token();

enum {
    NOTYPE, DIGIT, 
    IDENTIFIER,
    OPEN_BR, CLOSE_BR,
};

char *type_repr(int type);

#endif
