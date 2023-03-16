/* Single stage gates */

#ifndef SINGLE_H
#define SINGLE_H

#include "gates.h"

inline void assign_gate(uint8_t* in, uint8_t* out1, uint8_t* out2) {
    asm volatile (
        "xor %%rdx, %%rdx\n\t"          // ax /= 0
        "div %%dl\n\t"
        "movzxb (%[in]), %%rcx\n\t"     // dl = out1[in[0]]
        "mov %%rcx, %%rdx\n\t"
        "add %[out1], %%rdx\n\t"
        "mov (%%rdx), %%dl\n\t"
        "add %[out2], %%rcx\n\t"        // cl = out2[in[0]]
        "mov (%%rcx), %%cl\n\t"
        : : [in] "r"(in), [out1] "r"(out1), [out2] "r"(out2) : "rax", "rcx", "rdx"
    );
    NOPs(NOP256);
}

inline void or_gate(uint8_t* in1, uint8_t* in2, uint8_t* out) {
    asm volatile (
        "xor %%rdx, %%rdx\n\t"          // ax /= 0
        "div %%dl\n\t"
        "movzxb (%[in1]), %%rcx\n\t"    // dl = out[in1[0]]
        "add %[out], %%rcx\n\t"
        "mov (%%rcx), %%al\n\t"
        "movzxb (%[in2]), %%rcx\n\t"    // dl = out[in2[0]]
        "add %[out], %%rcx\n\t"
        "mov (%%rcx), %%dl\n\t"
        : : [in1] "r"(in1), [in2] "r"(in2), [out] "r"(out) : "rax", "rdx", "rcx"
    );
    NOPs(NOP256);
}

inline void and_gate(uint8_t* in1, uint8_t* in2, uint8_t* out) {
    asm volatile (
        "xor %%rdx, %%rdx\n\t"          // ax /= 0
        "div %%dl\n\t"
        "movzxb (%[in1]), %%rcx\n\t"    // dl = in2[in1[0]]
        "add %[in2], %%rcx\n\t"
        "movzxb (%%rcx), %%rdx\n\t"
        "add %[out], %%rdx\n\t"         // dl = out[rdx]
        "mov (%%rdx), %%dl\n\t"
        : : [in1] "r"(in1), [in2] "r"(in2), [out] "r"(out) : "rax", "rcx", "rdx"
    );
    NOPs(NOP256);
}

inline void not_gate(uint8_t* in1, uint8_t* in2, uint8_t* out, uint8_t* delay) {
    asm volatile (
        "movzxb (%[in1]), %%rdx\n\t"     // ax /= byte in2[in1[0]]
        "add %[in2], %%rdx\n\t"
        "movzxb (%%rdx), %%rdx\n\t"
        "div %%dl\n\t"
        "movzxb (%[delay]), %%rcx\n\t"  // dl = out[delay[0]]
        "add %[out], %%rcx\n\t"
        "mov (%%rcx), %%dl\n\t"
        : : [in1] "r"(in1), [out] "r"(out), [delay] "r"(delay), [in2] "r"(in2) : "rax", "rcx", "rdx"
    );
    NOPs(NOP256);
}

inline uint64_t timer(uint8_t* ptr) {
    uint64_t clk;
    asm volatile (
        "rdtscp\n\t"
        "shl $32, %%rdx\n\t"
        "mov %%rdx, %%rsi\n\t"
        "or %%eax, %%esi\n\t"
        "mov %1, %%al\n\t"
        "rdtscp\n\t"
        "shl $32, %%rdx\n\t"
        "or %%eax, %%edx\n\t"
        "sub %%rsi, %%rdx\n\t"
        "mov %%rdx, %0\n\t"
        : "=r" (clk)
        : "m" (ptr[0])
        : "rcx", "rdx", "rsi", "eax"
    );
    return clk;
}

#endif
