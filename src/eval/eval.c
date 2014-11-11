#include <stdio.h>
#include "data/list.h"

list eval_atom(list exp)
{
    return exp;
}

list eval(list exp)
{
    if (is_nil(exp)) {
        printf("nil!\n");
        return make_nil();
    } else if (is_atom(exp)) {
        printf("single atom!\n");
        return eval_atom(exp);
    } else {
        list key = car(exp);
        assert(is_atom(key));
        if (atom_sameword(key->item, "quote")) {
            printf("quote!\n");
        } else {
            printf("eval, eval!\n");
        }
        return make_nil();
    }
}
