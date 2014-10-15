#include "common.h"
#include "memory.h"
#include "cpu/reg.h"
#include <stdlib.h>

/* use POSIX regex functions to process regular expressions. */
#include <sys/types.h>
#include <regex.h>

extern CPU_state cpu;

#define NR_TK 32

#define BAD_VAL 66666   /* BAD_VAL can be any value */

enum {
    NOTYPE = 256,
    DIGIT, HEX, REG,
    LT, LE, GT, GE, EQ, NE,
    AND, OR, NOT,
    SL, SR,
    INDIR,  /* unary *, indirection */
    POS, NEG,  /* unary +/- */
};

static char *type_repr(int type)
{
    char *repr;
    if (type < 256) { /* can be represented by a single character */
        repr = (char *)malloc(2);  /* one char, one '\0' */
        repr[0] = type;
        repr[1] = '\0';
        return repr;
    } else {
        switch(type) {
        case NOTYPE: return "NOTYPE";
        case DIGIT: return "DIGIT";
        case HEX: return "HEX";
        case REG: return "REG";
        case LT: return "<";
        case LE: return "<=";
        case GT: return ">";
        case GE: return ">=";
        case EQ: return "==";
        case NE: return "!=";
        case AND: return "&&";
        case OR: return "||";
        case NOT: return "!";
        case SL: return "<<";
        case SR: return ">>";
        case INDIR: return "*(indirection)";
        case POS: return "+(positive)";
        case NEG: return "-(negative)";
        default:
            test(0, "No match type representation.");
        }
    }
}

/* operator precedence */
static int opprec(int type)
{
    switch (type) {
        case NOT: case '~':
        case INDIR: case POS: case NEG:
            return 13;
        case '*': case '/': case '%':
            return 12;
        case '+': case '-':
            return 11;
        case SL: case SR:
            return 10;
        case LT: case LE: case GT: case GE:
            return 9;
        case EQ: case NE:
            return 8;
        case '&': return 7;
        case '^': return 6;
        case '|': return 5;
        case AND: return 4;
        case OR: return 3;
        default:
            test(0, "No match precedence for %s", type_repr(type));
    }
}

/* tell unary operator from binary operator */
static int opoperand(int type)
{
    /* unary operators all have precedence 13 */
    if (opprec(type) == 13) {
        return 1;
    } else if (opprec(type) == 2) {  /* ? : operator */
        return 3;
    } else {
        return 2;
    }
}

/* associativity of operator */
static int opassoc(int type)
{
    int prec = opprec(type);
    if (prec == 13 || prec == 1) {
        return 'r';
    } else {
        return 'l';
    }
}

static bool is_operand(type)
{
    return type == DIGIT || type == HEX || type == REG;
}

static bool is_operator(type)
{
    return !is_operand(type) && type != '(' && type != ')';
}

static struct rule {
    char *regex;
    int token_type;
} rules[] = {
     /* Pay attention to match precedence of different tokens. */
    {" +",  NOTYPE},    
    {"\\(", '('}, {"\\)", ')'},

    {"\\+", '+'}, {"-", '-'}, {"\\*", '*'}, {"/", '/'}, {"%", '%'},

    {"<<", SL}, {">>", SR},
    {"<=", LE}, {"<", LT}, {">=", GE}, {">", GT},
    {"==", EQ}, {"!=", NE},
    {"&&", AND}, {"\\|\\|", OR}, {"!", NOT},

    {"&", '&'}, {"\\|", '|'}, {"~", '~'}, {"\\^", '^'},

    {"0(x|X)[0-9a-fA-F]+", HEX},
    {"[0-9]+", DIGIT},
    {"\\$[a-z]+", REG},
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
            test(0, "regex compilation failed: %s\n%s\n",
                    error_msg, rules[i].regex);
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

            if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 
                    && pmatch.rm_so == 0) {
                char *substr_start = e + position;
                /* rm_eo is exact the length of sub-string */
                int substr_len = pmatch.rm_eo;

                position += substr_len;

                int this_type = rules[i].token_type;
                if (this_type == NOTYPE) { /* match spaces */
                    //Log("blank token, omitted.");
                    ;
                } else if (is_operand(this_type)) {
                    tokens[nr_token].type = this_type;
                    tokens[nr_token].str = strndup(substr_start, substr_len);
                    Log("token[%d], type: %s, value: \"%s\"", 
                            nr_token, type_repr(this_type),
                            tokens[nr_token].str);
                    ++nr_token;
                } else if (this_type == '(' || this_type == ')') {
                    tokens[nr_token].type = this_type;
                    Log("token[%d], parenthesis %s",
                            nr_token, type_repr(this_type));
                    ++nr_token;
                } else { /* match operator */
                    tokens[nr_token].type = this_type;
                    Log("token[%d], operator %s, precedence %d",
                            nr_token, type_repr(this_type), opprec(this_type));
                    ++nr_token;
                }
                break; /* jump out of for loop */
            }
        }
        if (i == NR_REGEX) {
            printf("no match at position %d\n%s\n%*.s^\n",
                    position, e, position, "");
            return false;
        }
    }
    return true; 
}

/* check if a range of tokens is contained in a pair of matched "()".
 * NOTE: this function cannot detect whether the expression is valid.
 * e.g. "(1+2)*(3+4)" and "(1+2))" both return false, 
 *     and these two case cannot be distinguished.
 */
bool check_parentheses(int first, int last)
{
    if (tokens[first].type != '(' || tokens[last].type != ')') {
        return false;
    }
    ++first;
    --last;
    int top = 0;
    int i;
    for (i = first; i <= last; i++) {
        if (tokens[i].type == '(') {
            ++top;  /* simulate push */
        } else if (tokens[i].type == ')') {
            /* simulate pop */
            if (top == 0) { 
                return false;  /* parentheses not matched */
            }
            --top;
        }
    }
    return top == 0;
}

int eval_atom(Token tk, bool *well)
{
    int result;
    int i;
    switch (tk.type) {
    case DIGIT:
        return atoi(tk.str);
    case HEX:
        sscanf(tk.str, "%x", &result);
        return result;
    case REG:
        if (strcmp(tk.str, "$eip") == 0) {
            return cpu.eip;
        }
        for (i = 0; i < 8; i++) {
            if (strcmp(tk.str + 1, regsl[i]) == 0)
                return reg_l(i);
            if (strcmp(tk.str + 1, regsw[i]) == 0)
                return reg_w(i);
            if (strcmp(tk.str + 1, regsb[i]) == 0)
                return reg_b(i);
        }
        printf("Invalid register %s for evaluation.\n", tk.str);
        *well = false; 
        return BAD_VAL;
    default:
        printf("Bad atom of type %s for evaluation.\n", type_repr(tk.type));
        *well = false; 
        return BAD_VAL;
    }
}

/* return -1 to indicate an error */
int find_dominant_index(int first, int last, bool *well)
{
    int ido = -1;
    int prec = 15;  /* current prec, init larger than any possible prec */
    int top = 0;  /* simulate a stack */
    int i;
    for (i = first; i <= last; i++) {
        int this_type = tokens[i].type;
        if (is_operand(this_type)) {
            continue;
        } else if (this_type == '(') {
            ++top;
        } else if (this_type == ')') {
            if (top == 0) {
                printf("Parentheses not matched.\n");
                *well = false; 
                return BAD_VAL;
            }
            --top;
        } else {
            if (top != 0) {  /* operator in parentheses */
                continue;
            }
            if ((opprec(this_type) == prec && opassoc(this_type) == 'l')
                    || opprec(this_type) < prec) {
                ido = i;
                prec = opprec(this_type);
            }
        }
    }
    if (ido == -1) {
        printf("Cannot split expression.\n");
        *well = false; 
        return BAD_VAL;
    }
    return ido;
}

static int unary_operate(int op, int val)
{
    switch (op) {
        case '~': return ~val;
        case NOT: return !val;
        case POS: return val;
        case NEG: return -val;
        case INDIR: return swaddr_read(val, 1);
        default:
            test(0, "operator %s not defined.", type_repr(op));
    }
}

static int binary_operate(int op, int val1, int val2)
{
    switch (op) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        case '%': return val1 % val2;
        case '&': return val1 & val2;
        case '|': return val1 | val2;
        case '^': return val1 ^ val2;
        case SL: return val1 << val2;
        case SR: return val1 >> val2;
        case LT: return val1 < val2;
        case LE: return val1 <= val2;
        case GT: return val1 > val2;
        case GE: return val1 >= val2;
        case EQ: return val1 == val2;
        case NE: return val1 != val2;
        case AND: return val1 && val2;
        case OR: return val1 || val2;
        default:
            test(0, "operator %s not defined.", type_repr(op));
    }
}

int eval_tokens(int first, int last, bool *well)
{
    if (first > last) {
        printf("Error when evaluate tokens, bad expression.\n");
        *well = false; 
        return BAD_VAL;
    } else if (first == last) {  /* single token */
        return eval_atom(tokens[first], well);
    } else if (check_parentheses(first, last)) {
        /* the whole expression is surrounded by parentheses */
        printf("Step into parentheses.\n");
        return eval_tokens(first+1, last-1, well);
    } else {
        int ido = find_dominant_index(first, last, well);
        if (!(*well)) { 
            return BAD_VAL;
        }
        int this_type = tokens[ido].type;
        printf("Split at tokens[%d]: %s\n", ido, type_repr(this_type));

        if (opoperand(this_type) == 1) {
            int val = eval_tokens(ido + 1, last, well);
            if (!(*well)) { 
                return BAD_VAL;
            }
            return unary_operate(this_type, val);
        } else { /* binary operator */
            int val1 = eval_tokens(first, ido - 1, well);
            int val2 = eval_tokens(ido + 1, last, well);
            if (!(*well)) { 
                return BAD_VAL;
            }
            return binary_operate(this_type, val1, val2);
        }
    }
}

int eval_expr(char *e, bool *success)
{
    if (!make_token(e)) {
        *success = false;
        return BAD_VAL;
    }
    /* recognize unary +, -, and *(indirection) */
    int i;
    for (i = 0; i < nr_token; i++) {
        int *typep = &tokens[i].type;
        switch (*typep) { case '+': case '-': case '*':
            if (i == 0 || is_operator(tokens[i-1].type) 
                    || tokens[i-1].type == '(') {
                switch (*typep) {
                    case '+': *typep = POS; break;
                    case '-': *typep = NEG; break;
                    case '*': *typep = INDIR; break;
                }
                Log("token[%d], operator %s, precedence %d",
                        i, type_repr(*typep), opprec(*typep));
            }
        }
    }
    return eval_tokens(0, nr_token-1, success);
}
