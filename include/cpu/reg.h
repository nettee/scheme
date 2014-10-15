#ifndef __CPU_REG_H__
#define __CPU_REG_H__

#include "common.h"

typedef struct {
	/* Mind the order of the GPRs' definitions.
	 * They match the register encoding scheme.
	 */
    union{
        union {
            uint32_t _32;   /*  [31:0] */
            uint16_t _16;   /*  [15:0] */
            uint8_t _8[2];  /*  [7:0], [15:8]*/
        } gpr[8];  /* GPR - General Purpose Registers */
        struct {
            uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
        };
    };
    swaddr_t eip;
    struct {
        uint32_t CF : 1;
        uint32_t PF : 1;
        uint32_t ZF : 1;
        uint32_t SF : 1;
        uint32_t IF : 1;
        uint32_t DF : 1;
        uint32_t OF : 1;
    } eflags;
} CPU_state;

extern CPU_state cpu;

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

#define reg_l(index) (cpu.gpr[index]._32)
#define reg_w(index) (cpu.gpr[index]._16)
#define reg_b(index) (cpu.gpr[index & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

#endif
