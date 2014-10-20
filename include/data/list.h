enum { ATOM, SUBLIST };

typedef struct node_ {
    int ntype;
    union {
        int value;
        struct node_ *next;
    };
} listnode;

typedef listnode *list;
