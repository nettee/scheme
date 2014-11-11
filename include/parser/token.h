#include "data/atom.h"

#define NR_TK 32

typedef struct token {
    int type;
    atom *ap;
} Token;

Token pop_token();
Token first_token();
