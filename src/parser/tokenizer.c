#include "common.h"
#include <stdlib.h>

/* use POSIX regex functions to process regular expressions. */
#include <sys/types.h>
#include <regex.h>

#include "data/atom.h"
#include "parser/token.h"

/* used by other modules */
Token tokens[NR_TK];
int tokens_begin = 0;
int tokens_end = 0;

/* used by other modules */
Token tokens[NR_TK];
int begin = 0;
int end = 0;

static int nr_token;

static struct rule {
    char *regex;
    int token_type;
} rules[] = {
    {"[ \n]+",  NOTYPE},    
    {"\\(", OPEN_BR}, {"\\)", CLOSE_BR},
    {"\\[", OPEN_BR}, {"\\]", CLOSE_BR},

    {"[0-9]+", DIGIT},
    /* an identifier cannot start by +/-/. , except single + and - */
    {"[!$%&*/:<=>?@^~A-Za-z_][-+!$%&*./:<=>?@^~A-Za-z_0-9]*", IDENTIFIER},
    {"[-+]", IDENTIFIER},
};

/* number of rules */
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )
static regex_t re[NR_REGEX];

void init_regex() 
{
    int i;
    char error_msg[128];
    int ret;

    for(i = 0; i < NR_REGEX; i++) {
        /* use ERE (POSIX Extended Regular Expression syntax) */
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if(ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            test(0, "regex compilation failed: %s\n%s\n", error_msg, rules[i].regex);
        }
    }
}

char *type_repr(int type)
{
    switch(type) {
        case OPEN_BR: case CLOSE_BR: return "bracket";
        case DIGIT: return "digit";
        case IDENTIFIER: return "identifier";
        default:
            test(0, "No match type representation.");
    }
}

int tokenize(char *e) 
{
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while(e[position] != '\0') {

        /* Try all rules one by one. */
        for(i = 0; i < NR_REGEX; i++) {
            if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;
                position += substr_len;

                int this_type = rules[i].token_type;
                if (this_type != NOTYPE) { 
                    tokens[nr_token].type = this_type;
                    tokens[nr_token].ap = atom_new(substr_start, this_type, substr_len);
                    atom *ap = tokens[nr_token].ap;
                    Log("token[%d], %s \"%s\", size %d", nr_token, type_repr(ap->type), atom_repr(ap), ap->len);
                    ++nr_token;
                }
                break; /* jump out of for loop */
            }
        }
        if (i == NR_REGEX) {
            test(0, "no match at position %d\n%s\n%*.s^\n", position, e, position, "");
        }
    }
    tokens_begin = 0;
    tokens_end = nr_token;
    return nr_token;
}

Token pop_token()
{
    test(tokens_begin < tokens_end, "Pop token fails");
    return tokens[tokens_begin++];
}

Token first_token()
{
    test(tokens_begin <= tokens_end, "Bad tokens");
    return tokens[tokens_begin];
}
