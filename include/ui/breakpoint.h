#ifndef __UI_BREAKPOINT_H__
#define __UI_BREAKPOINT_H__

#include "common.h"

#define INT3_CODE 0xcc

typedef struct breakpoint {
    int NO;
    char type;
    struct breakpoint *next;
    union {
        struct {  /* for breakpoint use */
            uint32_t addr;
            uint8_t instr_byte;
        };
        struct {  /* for watchpoint use */
            char *expr;
            uint32_t value;
        };
    };
} BP;

void new_bp(uint32_t addr);
void new_wp(char *expr, uint32_t init_value);
void free_bp(int bp_NO);
void free_all_bp();

void info_bp();
void print_bp();

void write_bp();
int trig_bp(uint32_t eip);
void watch_around();

#endif
