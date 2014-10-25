#include "common.h"
#include "parser/token.h"
#include "data/list.h"

extern Token tokens[];
int tokenize(char *);

void parse(char *s)
{
    int n_tokens = tokenize(s);
    Log("Tokenize into %d tokens.", n_tokens);
    list exp = read_from_tokens();
    print_list(exp);
}
