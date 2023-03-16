#ifndef COMPOSE_H
#define COMPOSE_H

#include"gates.h"
#include "single.h"

void nand_gate(uint8_t* in1, uint8_t* in2, uint8_t* out);
void xor_gate(uint8_t* in1, uint8_t* in2, uint8_t* out, unsigned input);
void mux_gate(uint8_t* in1, uint8_t* in2, uint8_t* in3, uint8_t* out, unsigned input);

#endif
