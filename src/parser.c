#include "common.h"
#include <stdlib.h>

/* use POSIX regex functions to process regular expressions. */
#include <sys/types.h>
#include <regex.h>

#define NR_TK 32

enum {
    NOTYPE, BRACKET, DIGIT, SYMBOL,
};

static char *type_repr(int type)
{
    switch(type) {
        case BRACKET: return "bracket";
        case DIGIT: return "digit";
        case SYMBOL: return "symbol";
        default:
            test(0, "No match type representation.");
    }
}

static struct rule {
    char *regex;
    int token_type;
} rules[] = {
    {" +",  NOTYPE},    
    {"\\(", BRACKET}, {"\\)", BRACKET},

    {"[0-9]+", DIGIT},
    {"[-+!&*/?a-z_][-+!&*/?a-z_0-9]*", SYMBOL},
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

typedef struct token {
    int type;
    char *str;
} Token;

Token tokens[NR_TK];
int nr_token;

static bool make_token(char *e) 
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
                /* rm_eo is exact the length of sub-string */
                int substr_len = pmatch.rm_eo;

                position += substr_len;

                int this_type = rules[i].token_type;
                if (this_type != NOTYPE) { 
                    tokens[nr_token].type = this_type;
                    tokens[nr_token].str = strndup(substr_start, substr_len);
                    Log("token[%d], %s \"%s\"", nr_token, type_repr(this_type), tokens[nr_token].str);
                    ++nr_token;
                }
                break; /* jump out of for loop */
            }
        }
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
            return false;
        }
    }
    return true; 
}

void parse(char *expr) {
    bool res = make_token(expr);
    printf("Parse %s.\n", res ? "succeeded" : "failed");
}
