#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "data/atom.h"
#include "data/type.h"

#define BUCKET_SIZE 64

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

atom *atom_new(const char *str, int type, int len)
{
    assert(str != NULL);
    assert(len > 0);

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
                return q;
            }
        }
    }
    q = malloc(sizeof(atom));
    q->type = type;
    if (type == DIGIT) {
        q->len = 4;
        q->str = (byte *)malloc(4 + 1);
        int32_t d = atoi(str);
        memcpy(q->str, (void *)&d, 4);
        q->str[4] = '\0';
    } else {
        q->len = len;
        q->str = (byte *)malloc(len + 1);
        memcpy(q->str, str, len);
        q->str[len] = '\0';  /* only for str convenient */
    }

    q->next = buckets[h];
    buckets[h] = q;
    return q;
}

char *atom_repr(atom *a)
{
    test(a, "reprensent no atom");
    int type = a->type;
    if (type == DIGIT) {
        char s[20];
        uint32_t d;
        memcpy((void *)&d, a->str, 4);
        sprintf(s, "%d", d);
        return strdup(s);
    } else {
        return (char *)a->str;
    }
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

void print_bytes(atom *it)
{
    return;
}
        
