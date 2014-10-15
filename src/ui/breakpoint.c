#include <stdlib.h>
#include "ui/ui.h"
#include "ui/breakpoint.h"

#include "nemu.h"

#define NR_BP 32

uint32_t eval_expr(char *expr, bool *success);

static BP bp_pool[NR_BP];
static BP *head, *free_;

void init_bp_pool() {
	int i;
	for (i = 0; i < NR_BP - 1; i++) {
		bp_pool[i].NO = i;
		bp_pool[i].next = &bp_pool[i + 1];
	}
    bp_pool[i].NO = i;
	bp_pool[i].next = NULL;

	head = NULL;
	free_ = bp_pool;
}

void new_bp(uint32_t addr)
{
    test(free_, "BP pool full, new BP fails.");

    /* remove a BP from free_ list */
    BP *bp = free_;
    free_ = free_->next;

    bp->type = 'b';
    bp->addr = addr;
    bp->instr_byte = swaddr_read(addr, 1);

    /* insert BP to the front of head */
    bp->next = head;
    head = bp;
    
    /* Anyway, write 0xcc to memory */
    swaddr_write(addr, 1, INT3_CODE);

    printf("Breakpoint %d at 0x%x.\n", bp->NO, addr);
}

void new_wp(char *expr, uint32_t init_value)
{
    test(free_, "BP pool full, new BP fails.");

    /* remove a BP from free_ list */
    BP *wp = free_;
    free_ = free_->next;

    wp->type = 'w';
    wp->expr = expr;
    wp->value = init_value;

    /* insert BP to the front of head */
    wp->next = head;
    head = wp;

    printf("Watchpoint %d: %s\n", wp->NO, expr);
    printf("Original value: %u\n", wp->value);
}

void free_bp(int bp_NO)
{
    test(head, "Delete BP[%d] fails, no BP in use.", bp_NO);
    test(bp_NO >= 0 && bp_NO < NR_BP, 
            "Delete BP[%d] fails, BP num out of range.", bp_NO);
    
    BP **pp = &head;
    BP *q;
    while ((q = *pp) != NULL) {
        if (q->NO == bp_NO) {
            /* remove q from free_ list */
            *pp = q->next;
            /* insert q to the front of head */
            q->next = free_;
            free_ = q;

            assert(q->type == 'b' || q->type == 'w');
            if (q->type == 'b') {
                swaddr_write(q->addr, 1, q->instr_byte);
            } else if (q->type == 'w') {
                free(q->expr);
            }
            return;
        }
        pp = &q->next;
    }
    /* no match NO in list */
    test(0, "Delete BP[%d] fails. BP not in use.", bp_NO);
}

void free_all_bp()
{
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        assert(q->type == 'b' || q->type == 'w');
        if (q->type == 'b') {
            swaddr_write(q->addr, 1, q->instr_byte);
        } else if (q->type == 'w') {
            free(q->expr);
        }
    }
    init_bp_pool();   /* this statement is necessary */
}

/* info_bp - print number and address of used BPs
 * called by cmd_info_b() in ui.c
 */
void info_bp()
{
    if (head == NULL) {
        printf("No breakpoints or watchpoints.\n");
        return;
    }
    printf("Num\tType\t\tAddress\t\tWhat\n");
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        if (q->type == 'b') {
            printf("%d\t%s\t0x%x\n", q->NO, "breakpoint", q->addr);
        } else if (q->type == 'w') {
            printf("%d\t%s\t\t\t%s\n", q->NO, "watchpoint", q->expr);
        } else {
            assert(0);
        }
    }
}

/* print_bp - print used and unused BP number
 * Note: ONLY for test
 */
void print_bp()
{
    printf("breakpoints in use: \n");
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        printf("%d ", q->NO);
    }
    printf("\n");
    printf("breakpoints not in use: \n");
    for (q = free_; q != NULL; q = q->next) {
        printf("%d ", q->NO);
    }
    printf("\n");
}

void write_bp()
{
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        if (q->type == 'b') {
            swaddr_write(q->addr, 1, INT3_CODE);
        }
    }
}

int trig_bp(uint32_t eip)
{
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        if (q->addr == eip) {
            swaddr_write(eip, 1, q->instr_byte);
            return q->NO;
        }
    }
    test(0, "Cannot find breakpoint at address 0x%x", eip);
}

void watch_around()
{
    bool success;
    uint32_t value;
    BP *q;
    for (q = head; q != NULL; q = q->next) {
        if (q->type != 'w') {
            continue;
        }
        success = true;
        value = eval_expr(q->expr, &success);
        assert(success);
        if (q->value != value) {  /* value changed */
            printf("Watchpoint %d: %s\n", q->NO, q->expr);
            printf("Old value = %d (0x%x)\n", q->value, q->value);
            printf("New value = %d (0x%x)\n", value, value);
            q->value = value;
            nemu_state = STOP;
            return;
        }
    }
}
