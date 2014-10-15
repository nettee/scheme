#include "ui/ui.h"
#include "ui/breakpoint.h"

#include "nemu.h"

#include <setjmp.h>

#define LOADER_START 0x100000

int exec(swaddr_t);
void load_prog();
void init_dram();

char assembly[40];
jmp_buf jbuf;	/* Make it easy to perform exception handling */

extern uint8_t loader [];
extern uint32_t loader_len;

extern int quiet;

void restart() {
	/* Perform some initialization to restart a program */
	load_prog();
	memcpy(hwa_to_va(LOADER_START), loader, loader_len);

	cpu.eip = LOADER_START;

	init_dram();

    write_bp();
    cpu.esp = 0x8000000;
}

static void print_bin_instr(swaddr_t eip, int len) {
	int i;
	printf("%8x:   ", eip);
	for(i = 0; i < len; i ++) {
		printf("%02x ", swaddr_read(eip + i, 1));
	}
	printf("%*.s", 50 - (12 + 3 * len), "");
}

void cpu_exec(volatile uint32_t n) {
    volatile uint32_t n_temp = n;

    setjmp(jbuf);

    /* cmd_c() passes -1 to cpu_exec, since it's unsigned,
     * it will be converted to the biggest number */
    for( ; n > 0; n--) { /* execute n times */
        swaddr_t eip_temp = cpu.eip;
        int instr_len = exec(cpu.eip);

        cpu.eip += instr_len;

        if ((n_temp != -1 || (enable_debug && !quiet))
                && !(nemu_state == STOP && breaked)) {
            print_bin_instr(eip_temp, instr_len);
            puts(assembly);
        }

        watch_around();

        if (nemu_state == INT) {
            printf("\n\nUser interrupt\n");
            return;
        } else if (nemu_state == STOP) {
            return;
        } else if (nemu_state == RUNNING) {
            if (breaked) {
                swaddr_write(eip_temp, 1, INT3_CODE);
                breaked = false;
            }
        } else if(nemu_state == END) {
            return; 
        }
    }
}
