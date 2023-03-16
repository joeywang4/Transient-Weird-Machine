#include "compose.h"

uint8_t tmp_reg1[4*512];
uint8_t tmp_reg2[4*512];
uint8_t tmp_reg3[4*512];
uint8_t tmp_reg4[4*512];
uint8_t tmp_reg5[4*512];
uint8_t tmp_reg6[4*512];
uint8_t tmp_reg7[4*512];
uint8_t tmp_reg8[4*512];
uint8_t tmp_reg9[4*512];
uint8_t tmp_reg10[4*512];

void nand_gate(uint8_t* in1, uint8_t* in2, uint8_t* out) {
    tmp_reg1[0] = 0;
    tmp_reg2[0] = 0;
    tmp_reg3[0] = 0;
    tmp_reg4[0] = 0;
    _mm_clflush(tmp_reg1);
    _mm_clflush(tmp_reg2);
    _mm_clflush(tmp_reg3);
    _mm_clflush(tmp_reg4);
    for (volatile int z = 0; z < 64; z++) {}

    and_gate(in1, in2, tmp_reg1);
    for (volatile int z = 0; z < 64; z++) {}

    assign_gate(tmp_reg1, tmp_reg2, tmp_reg3);
    for (volatile int z = 0; z < 64; z++) {}

    not_gate(tmp_reg2, tmp_reg3, out, tmp_reg4);
}

void xor_gate(uint8_t* in1, uint8_t* in2, uint8_t* out, unsigned input) {
    tmp_reg5[0] = 0;
    tmp_reg6[0] = 0;
    tmp_reg7[0] = 0;
    tmp_reg8[0] = 0;
    tmp_reg9[0] = 0;
    tmp_reg10[0] = 0;
    assign(tmp_reg5, input & 1);
    assign(tmp_reg6, input & 2);
    _mm_clflush(tmp_reg7);
    _mm_clflush(tmp_reg8);
    _mm_clflush(tmp_reg9);
    _mm_clflush(tmp_reg10);
    for (volatile int z = 0; z < 64; z++) {}

    // reg9 = in1 | in2
    or_gate(in1, in2, tmp_reg9);
    for (volatile int z = 0; z < 64; z++) {}

    // reg10 = !(in1 & in2)
    nand_gate(tmp_reg5, tmp_reg6, tmp_reg10);
    for (volatile int z = 0; z < 64; z++) {}
    
    // out = reg9 & reg10
    and_gate(tmp_reg9, tmp_reg10, out);
}

void mux_gate(uint8_t* in1, uint8_t* in2, uint8_t* in3, uint8_t* out, unsigned input) {
    tmp_reg1[0] = 0;
    tmp_reg2[0] = 0;
    tmp_reg3[0] = 0;
    tmp_reg4[0] = 0;
    tmp_reg5[0] = 0;
    tmp_reg6[0] = 0;
    assign(tmp_reg1, input & 4);
    assign(tmp_reg2, input & 4);
    _mm_clflush(tmp_reg3);
    _mm_clflush(tmp_reg4);
    _mm_clflush(tmp_reg5);
    _mm_clflush(tmp_reg6);
    for (volatile int z = 0; z < 64; z++) {}

    and_gate(in2, in3, tmp_reg3);
    not_gate(tmp_reg1, tmp_reg2, tmp_reg4, tmp_reg5);
    for (volatile int z = 0; z < 64; z++) {}

    and_gate(in1, tmp_reg4, tmp_reg6);
    for (volatile int z = 0; z < 64; z++) {}

    or_gate(tmp_reg3, tmp_reg6, out);
}
