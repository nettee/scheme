#include "ui/ui.h"
#include "data/list.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

list parse(char *);
void eval(list exp);

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

void main_loop() 
{
    char *line;

    while(1) {
        line = rl_gets();  /* get a line from stdin */
        if (line == NULL) {
            printf("\n");
            return;
        } else if (strlen(line) == 0) {
            continue;
        } else if (strcmp(line, "(exit)") == 0) {
            return;
        } else {
            eval(parse(line));
        }
    }
}
