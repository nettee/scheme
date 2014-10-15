#include "ui/ui.h"

#include <signal.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

int nemu_state = END;

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
    char *cmd;

    while(1) {
        cmd = rl_gets();  /* get a line from stdin */
        if (cmd == NULL) {
            printf("\n");
            return;
        } else if (strlen(cmd) == 0) {
            continue;
        } else if (strcmp(cmd, "exit") == 0) {
            return;
        } else {
            printf("command: '%s'\n", cmd);
        }
    }
}
