#include "common.h"

#include <unistd.h>

void main_loop();
void init_signal();

int enable_debug = false;
int quiet = false;

static void process_args(int argc, char *argv[]) {
	int opt;
	while( (opt = getopt(argc, argv, "dq")) != -1) {
		switch(opt) {
			case 'd':
				enable_debug = true;
				break;
			case 'q':
				quiet = true;
				break;
			default:
				test(0, "bad option = %s\n", optarg);
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	process_args(argc, argv);

	init_signal();

	main_loop();

	return 0;
}
