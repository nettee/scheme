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
        list key = list_car(exp);
        assert(is_atom(key));
        if (atom_sameword(key->item, "quote")) {
            printf("quote!\n");
            return list_car(list_cdr(exp));
        } else {   /* a procedure */
            printf("procedure!\n");
            return make_nil();
        }
    }
}
