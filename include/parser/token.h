#include "data/atom.h"

#define NR_TK 32

typedef struct token {
    int type;
    atom *element;
} Token;

Token tokens[NR_TK];
