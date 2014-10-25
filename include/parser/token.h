#include "data/atom.h"

#define NR_TK 32

typedef struct token {
    int type;
    atom *element;
} Token;

Token pop_token();
Token first_token();
