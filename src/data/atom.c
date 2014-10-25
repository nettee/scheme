#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data/atom.h"

#define BUCKET_SIZE 64

typedef struct atom_ {
    struct atom_ *next;
    int len;
    char *str;
} atom;
static atom *buckets[BUCKET_SIZE];

/* copy from K&R C, page 144 */
unsigned hash(const char *s)
{
    unsigned h = 0;
    for ( ; *s != '\0'; s++) {
        h = *s + 31 * h;
    }
    return h % BUCKET_SIZE;
}

char *atom_new(const char *str)
{
    assert(str != NULL);

    unsigned h = hash(str);

    atom *q;
    for (q = buckets[h]; q != NULL; q = q->next) {
        /* if atom already in table, simply return the atom */
        if (strcmp(q->str, str) == 0) {
            return q->str;
        }
    }
    q = malloc(sizeof(atom));
    q->str = strdup(str);
    q->next = buckets[h];
    buckets[h] = q;
    return q->str;
}

/* for test use */
void print_bucket()
{
    int i;
    atom *q;
    for (i = 0; i < BUCKET_SIZE; i++) {
        printf("%d: ", i);
        for (q = buckets[i]; q != NULL; q = q->next) {
            printf("%s ", q->str);
        }
        printf("\n");
    }
}
        
