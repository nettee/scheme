#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void init_regex();
void main_loop();
void parse(char *);

int enable_debug = false;
int quiet = false;
int interactive = true;

void interpret(FILE *fp)
{
    char line[200];
    fgets(line, 200, fp);
    parse(line);
}

int main(int argc, char *argv[]) {
    char *filename = NULL;

    init_regex();

    while (1) {
        int opt = getopt(argc, argv, "dqs:");
        if (opt == -1) {
            break;
        }
        switch(opt) {
        case 'd':
            enable_debug = true;
            break;
        case 'q':
            quiet = true;
            break;
        case 's':
            interactive = false;
            filename = strdup(optarg);
            break;
        default:
            test(0, "bad option = %s\n", optarg);
            break;
        }
    }

    if (interactive) {
        main_loop();
    } else {
        assert(filename != NULL);
        FILE *fp = fopen(filename, "r");
        if (fp == NULL) {
            fprintf(stderr, "%s: %s: %s\n",
                    argv[0], optarg, strerror(errno));
            return 1;
        }
        interpret(fp);
        fclose(fp);
    }

    return 0;
}
