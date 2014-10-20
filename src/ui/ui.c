#include "ui/ui.h"

#include <signal.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int nemu_state = END;
void parse(char *);

/* Use the readline library to provide more flexibility.
 * Return a line read from standard input. */
char *rl_gets() 
{
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline(">>> ");

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
    printf("boom...\n");
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


void main_loop() 
{
    char *sexp;

    while(1) {
        sexp = rl_gets();  /* get a line from stdin */
        if (sexp == NULL) {
            printf("\n");
            return;
        } else if (strlen(sexp) == 0) {
            continue;
        } else if (strcmp(sexp, "exit") == 0) {
            return;
        } else {
            parse(sexp);
        }
    }
}
