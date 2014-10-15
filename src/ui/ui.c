#include "nemu.h"

#include "ui/ui.h"
#include "ui/breakpoint.h"

#include <signal.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

/* for test use */
#define DATA_BYTE 4
#include "exec/template-start.h"

int nemu_state = END;
bool breaked = false;

void cpu_exec(uint32_t);
void restart();
int eval_expr(char *, bool *);

/* Use the readline library to provide more flexibility.
 * Return a line read from standard input. */
char *rl_gets() 
{
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read) {
        add_history(line_read);
    }
    return line_read;
}

/* This function will be called when you press <C-c>. And it will return to 
 * where you press <C-c>. If you are interesting in how it works, please
 * search for "Unix signal" in the Internet.
 */
static void control_C(int signum) 
{
    if (nemu_state == RUNNING) {
        nemu_state = INT;
    }
}

void init_signal() 
{
    /* Register a signal handler. */
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    s.sa_handler = control_C;
    int ret = sigaction(SIGINT, &s, NULL);
    assert(ret == 0);
}

static void cmd_c() 
{
    if (nemu_state == END) {
        printf("The program is not running.\n");
        return;
    } else if (nemu_state == STOP) {
        nemu_state = RUNNING;
    }
    cpu_exec(-1);
    if (nemu_state == INT || nemu_state == RUNNING)
        nemu_state = STOP;
}

static void cmd_si(int N)
{
    if (N <= 0) { /* invalid N */
        return;
    }
    if (nemu_state == END) {
        restart();
    }
    nemu_state = RUNNING;
    cpu_exec(N);
    if (nemu_state == INT || nemu_state == RUNNING)
        nemu_state = STOP;
}

static void cmd_r() 
{
    if (nemu_state != END) { 
        char c;
        while (1) {
            printf("The program has been started already.\n");
            printf("Start it from the beginning? (y or n) ");
            fflush(stdout);
            scanf(" %c", &c);
            if (c == 'y') {
                break;
            } else if (c == 'n') {
                return;
            } else {
                puts("Please answer y or n.");
            }
        }
    }

    restart();
    nemu_state = STOP;
    cmd_c();
}

static void cmd_info_r()
{
    if (nemu_state == END) {
        printf("The program has no registers now.\n");
        return;
    }
    printf("eax\t%#.8x\t%d\n", cpu.eax, cpu.eax);
    printf("ecx\t%#.8x\t%d\n", cpu.ecx, cpu.ecx);
    printf("edx\t%#.8x\t%d\n", cpu.edx, cpu.edx);
    printf("ebx\t%#.8x\t%d\n", cpu.ebx, cpu.ebx);
    printf("esp\t%#.8x\n", cpu.esp);
    printf("ebp\t%#.8x\n", cpu.ebp);
    printf("esi\t%#.8x\n", cpu.esi);
    printf("edi\t%#.8x\n", cpu.edi);
    printf("eip\t%#.8x\n", cpu.eip);
}

static void cmd_info_b()
{
    info_bp();
}

static void cmd_x(int N, char *expr)
{
    if (N <= 0) { /* invalid N */
        return;
    }
    bool success = true;
    uint32_t addr = eval_expr(expr, &success);
    test(success, "Invalid expression for memory examine.");

    int addr_32base;
    int i, j;
    for (j = 0; j < N; j++) {
        addr_32base = addr + 4 * j;
        /* output 4 bytes each time */
        printf("0x%08x: ", addr_32base);
        for (i = 0; i < 4; i++) {
            printf("%02x ", swaddr_read(addr_32base + i, 1));
        }
        printf("\n");
    }
}

static void cmd_p(char *expr)
{
    bool success = true;
    int result = eval_expr(expr, &success);
    if (success) {
        printf("%s = %d\n", expr, result);
    } else {
        printf("Bad expression.\n");
    }
}

static void cmd_b(char *expr)
{
    bool success = true;
    uint32_t addr = eval_expr(expr, &success);
    test(success, "Invalid expression for breakpoint.");
    if (nemu_state == END) {
        restart();
    }
    new_bp(addr);
}

static void cmd_w(char *expr)
{
    bool success = true;
    uint32_t value = eval_expr(expr, &success);
    test(success, "Invalid expression for watchpoint.");
    
    new_wp(expr, value);
}

static void cmd_d(int N)
{
    assert(N >= -1);
    if (N == -1) {
        free_all_bp();
    } else {
        free_bp(N);
    }
}

static void cmd_bt()
{
    /* to be implemented */
    printf("in func cmd_bt\n");
}

char *lstrip(char *s, char c)
{
    assert(c != '\0');
    char *t = s;
    while (*t == c) {
        t++;
    }
    return t;
}

void main_loop() 
{
    char *cmd, *cmd_dup;

    while(1) {
        cmd = rl_gets();  /* get a line from stdin */
        cmd_dup = strdup(cmd);
        char *p = strtok(cmd, " "); /* get the first word in cmd */
        if (p == NULL) {
            continue; 
        } else if (strcmp(p, "q") == 0) {
            return; 
        } else if (strcmp(p, "r") == 0) {
            cmd_r(); 
        } else if (strcmp(p, "c") == 0) { 
            cmd_c(); 
        } else if (strcmp(p, "si") == 0) {
            char *q = strtok(NULL, " ");
            int N = (q == NULL) ? 1 : atoi(q);
            cmd_si(N);
        } else if (strcmp(p, "info") == 0) {
            char *q = strtok(NULL, " ");
            if (q == NULL) {
                printf("'info' must be followed by an info command.\n");
                continue;
            } else if (strcmp(q, "r") == 0) {
                cmd_info_r();
            } else if (strcmp(q, "b") == 0) {
                cmd_info_b();
            } else {
                printf("Unknown command 'info %s'\n", q);
            }
        } else if (strcmp(p, "p") == 0) { 
            char *q = strtok(NULL, " ");
            if (q == NULL) { /* no argument */
                printf("p: argument required.\n");
                continue;
            }
            char *expr = lstrip(cmd_dup + 1, ' ');
            cmd_p(expr);
        } else if (strcmp(p, "x") == 0) {
            char *q = strtok(NULL, " ");
            if (q == NULL) {
                printf("x: argument required.\n");
                continue;
            }
            int N = atoi(q);
            char *r = strtok(NULL, " ");
            if (r == NULL) { 
                printf("x: more argument required.\n");
                continue;
            }
            char *tmpp = lstrip(cmd_dup + 1, ' ');
            char *expr = lstrip(tmpp + strlen(q), ' ');
            cmd_x(N, expr);
        } else if (strcmp(p, "b") == 0) {
            char *q = strtok(NULL, " ");
            if (q == NULL) { 
                printf("b: argument required.\n");
                continue;
            }
            if (q[0] == '*') { /* break at an address */
                char *expr = lstrip(cmd_dup + 1, ' ') + 1;
                cmd_b(expr);
            } else {
                printf("b: invalid argument.\n");
                continue;
            }
        } else if (strcmp(p, "w") == 0) {
            char *q = strtok(NULL, " ");
            if (q == NULL) {
                printf("w: argument required.\n");
                continue;
            }
            char *expr = strdup(lstrip(cmd_dup + 1, ' '));
            cmd_w(expr);
        } else if (strcmp(p, "d") == 0) {
            char *q = strtok(NULL, " ");
            int N = (q == NULL) ? -1 : atoi(q);
            cmd_d(N);
        } else if (strcmp(p, "bt") == 0) {
            cmd_bt();
        } else if (strcmp(p, "t") == 0) { /* for test */
            if (!enable_debug) {
                printf("Invalid command \"t\"\n");
                continue;
            }
            char *q = strtok(NULL, " ");
            assert(q != NULL);
            char *r = strtok(NULL, " ");
            assert(r != NULL);
            if (strcmp(q, "msb") == 0) {
                int n;
                sscanf(r, "%x", &n);
                printf("%u\n", MSB(n));
            } else if (strcmp(q, "parity") == 0) {
                int n;
                sscanf(r, "%x", &n);
                printf("%u\n", PARITY(n));
            }

        } else {
            printf("Invalid command \"%s\".\n", p);
        }
        free(cmd_dup);
    }
}
