#ifndef GATES_H
#define GATES_H

#include <stdint.h>
#include <x86intrin.h>

#define NOP "0x90"
#define NOP4 "0x90,0x90,0x90,0x90"
#define NOP16 NOP4 "," NOP4 "," NOP4 "," NOP4
#define NOP64 NOP16 "," NOP16 "," NOP16 "," NOP16
#define NOP256 NOP64 "," NOP64 "," NOP64 "," NOP64
#define NOPs(data) asm volatile(".byte " data ::: "memory");

void inline assign(uint8_t* ptr, int input) {
    if (input) { ptr[0] = 0; }
    else { _mm_clflush(ptr); }
}

#endif
