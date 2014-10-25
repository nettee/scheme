#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "data/atom.h"

#define BUCKET_SIZE 64

typedef struct atom_ {
    struct atom_ *next;
    int len;
    byte *str;
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

byte *atom_new(const char *str, int len)
{
    assert(str != NULL);
    assert(len >= 0);

    unsigned h = hash(str);

    atom *q;
    for (q = buckets[h]; q != NULL; q = q->next) {
        /* if atom already in table, simply return the atom */
        if (q->len == len) {
            int i;
            for (i = 0; i < len; i++) {
                if (q->str[i] != str[i]) {
                    break;
                }
            }
            if (i == len) {
                return q->str;
            }
        }
    }
    q = malloc(sizeof(atom));
    q->len = len;
    q->str = (byte *)malloc(len + 1);
    if (len > 0) {
        memcpy(q->str, str, len);
    }
    q->str[len] = '\0';
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
        
