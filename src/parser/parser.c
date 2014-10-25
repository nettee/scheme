#include "common.h"
#include "parser/token.h"

int tokenize(char *);

void read_from_tokens(int len)
{
    test(len > 0, "Read from no tokens.");
    printf("In func read_from\n");
}

void parse(char *expr)
{
    int n_tokens = tokenize(expr);
    Log("Tokenize into %d tokens.", n_tokens);
    read_from_tokens(n_tokens);
}


