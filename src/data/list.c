#include "data/list.h"
#include "data/type.h"
#include "data/atom.h"
#include "parser/token.h"

extern Token tokens[];

enum { HEAD, ATOM, SUBLIST };

static listnode *make_head()
{
    listnode *q = malloc(sizeof(listnode));
    q->ntype = HEAD;
    q->next = NULL;
    return q;
}

static listnode *make_atom(atom *a)
{
    listnode *q = malloc(sizeof(listnode));
    q->ntype = ATOM;
    q->next = NULL;
    q->item = a;
    return q;
}

static listnode *make_sublist(list p)
{
    listnode *q = malloc(sizeof(listnode));
    q->ntype = SUBLIST;
    q->next = NULL;
    q->sub = p;
    return q;
}

static listnode *insert_after(listnode *tail, listnode *q)
{
    tail->next = q;
    return q;
}

list make_nil()
{
    return make_head();
}

list read_from_tokens()
{
    Token tk;
    tk = pop_token();
    if (tk.type == OPEN_BR) {
        listnode *head = make_head();
        listnode *tail = head;
        while (first_token().type != CLOSE_BR) {
            listnode *q = read_from_tokens();
            if (q->ntype == ATOM) {  /* atom, single node */
                tail = insert_after(tail, q);
            } else {  /* sublist */
                tail = insert_after(tail, make_sublist(q));
            }
        }
        pop_token();  /* pop off ')' */
        return head;
    } else if (tk.type == CLOSE_BR) {
        test(0, "Unexpected ')'");
    } else {  /* single atom */
        return make_atom(tk.ap);
    }
}

static void print_list_rec(list p)
{
    if (p->ntype == ATOM) {  /* single atom */
        printf("%s", atom_repr(p->item));
    } else {  /* list */
        putchar('(');
        listnode *q;
        for (q = p->next; q != NULL; q = q->next) {
            if (q != p->next) {
                putchar(' ');
            }
            if (q->ntype == ATOM) {
                printf("%s", atom_repr(q->item));
            } else if (q->ntype == SUBLIST) {
                print_list_rec(q->sub);
            } else {
                test(0, "node type is neither ATOM nor SUBLIST");
            }
        }
        putchar(')');
    }
}

void print_list(list p)
{
    print_list_rec(p);
    putchar('\n');
}
