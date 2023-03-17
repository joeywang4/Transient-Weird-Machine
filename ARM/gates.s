// void assign_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
.global assign_gate
.func
assign_gate:
    LDR X0, [X0]

    LDR X4, [X1]
    ADD X4, X3, X4
    LDR X4, [X4]
    NOP
    NOP

    NOP
    NOP
    NOP
    NOP

    RET
.endfunc

// void or_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
.global or_gate
.func
or_gate:
    LDR X0, [X0]

    LDR X4, [X1]
    ADD X4, X3, X4
    LDR X4, [X4]
    LDR X4, [X2]
    ADD X4, X3, X4

    LDR X4, [X4]
    NOP
    NOP
    NOP

    RET
.endfunc

// void and_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
.global and_gate
.func
and_gate:
    LDR X0, [X0]

    LDR X4, [X1]
    ADD X3, X3, X4
    LDR X4, [X2]
    ADD X3, X3, X4
    LDR X3, [X3]

    NOP
    NOP
    NOP
    NOP

    RET
.endfunc

// void not_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
.global not_gate
.func
not_gate:
    LDR X0, [X0]
    LDR X0, [X0]

    LDR X4, [X1]
    ADD X4, X1, X4
    LDR X4, [X4]
    ADD X4, X1, X4
    LDR X4, [X4]
    ADD X3, X3, X4
    LDR X3, [X3]
    NOP

    RET
.endfunc

// void not_gate2(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
.global not_gate2
.func
not_gate2:
    LDR X0, [X0]
    LDR X0, [X0]

    LDR X4, [X1]
    ADD X4, X1, X4
    LDR X4, [X4]
    ADD X4, X1, X4
    LDR X4, [X4]
    
    ADD X4, X1, X4
    LDR X4, [X4]
    ADD X3, X3, X4
    LDR X3, [X3]

    RET
.endfunc
