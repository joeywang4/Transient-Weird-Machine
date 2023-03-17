#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ITER 1000000

volatile uint64_t counter = 0;
volatile int read_bit;
uint8_t *probe1;
uint8_t *probe2;
uint8_t *probe3;
uint8_t *probe4;
uint8_t *probe5;
uintptr_t *zrbf;
uint32_t *codebuf;
uint32_t *not_buf;
uint32_t *or_buf;
uint32_t *assign_buf;
uint32_t *and_buf;
uint64_t miss_min;

#define probe_stride (4 * 1024)
const size_t probe_size = 1 * probe_stride;
#define zrbf_stride (4 * 1024/sizeof(uintptr_t))
const size_t zrbf_size = 10 * zrbf_stride * sizeof(uintptr_t);

typedef void (*gate_fn)(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
gate_fn do_gate;
gate_fn do_not;
gate_fn do_or;
gate_fn do_assign;
gate_fn do_and;
extern void or_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
extern void assign_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
extern void and_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
extern void not_gate(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
extern void not_gate2(void* miss_buf, uint8_t* reg1, uint8_t* reg2, uint8_t* reg3);
#define CODE_SIZE 100
#define JUMP_SIZE 40 // 4 * (LoC + 1)

uint64_t timed_read(uint8_t *addr) {
    uint64_t ns = counter;

    asm volatile (
        "DSB SY\n"
        "LDR X5, [%[ad]]\n"
        "DSB SY\n"
        : : [ad] "r" (addr) : "x5"
    );

    return counter - ns;
}

void flush(void *addr, size_t n, size_t stride) {
    for (int i = 0; i < n; i += 1) {
        asm volatile ("DC CIVAC, %[ad]" : : [ad] "r" (addr));
        addr += stride;
    }
    asm volatile("DSB SY");
}

uint64_t measure_latency() {
  uint64_t ns;
  uint64_t min = 0xFFFFF;

  for (int r = 0; r < 300; r++) {
    flush(probe1, 1, probe_stride);
    ns = timed_read(&probe1[0]);
    if (ns < min) min = ns;
  }

  return min;
}

void* inc_counter(void* a) {
    while(1) {
        counter++;
        asm volatile ("DMB SY");
    }
}

void get_value(int i, siginfo_t *info, void *ctx) {
    read_bit = 0;
    uint64_t ns = timed_read(&probe3[0]);
    if (ns < miss_min && ns > 0) {
        read_bit = 1;
    }

    ucontext_t *c = (ucontext_t *)ctx;
    c->uc_mcontext.pc += JUMP_SIZE;
}

static inline void assign(uintptr_t* addr, bool val) {
    if (val) {
        addr[0] = 0;
    }
    else {
        flush(addr, 1, probe_stride);
    }
}

void test_assign_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        assign((uintptr_t*)probe1, i & 1);
        flush(probe3, 1, probe_stride);
        for (volatile int z = 0; z < 50; z++) {}

        do_assign((void*) 0, probe1, probe2, probe3);

        if (read_bit == ((i & 1) > 0)) correct++;
    }
    end_t = clock();

    printf("=== ASSIGN gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

void test_or_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        assign((uintptr_t*)probe1, i & 1);
        assign((uintptr_t*)probe2, i & 2);
        flush(probe3, 1, probe_stride);
        for (volatile int z = 0; z < 50; z++) {}

        do_or((void*) 0, probe1, probe2, probe3);

        if (read_bit == ((i & 3) > 0)) correct++;
    }
    end_t = clock();

    printf("=== OR gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

void test_and_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        assign((uintptr_t*)probe1, i & 1);
        assign((uintptr_t*)probe2, i & 2);
        flush(probe3, 1, probe_stride);
        for (volatile int z = 0; z < 50; z++) {}

        do_and((void*) 0, probe1, probe2, probe3);

        if (read_bit == ((i & 3) == 3)) correct++;
    }
    end_t = clock();

    printf("=== AND gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

void test_not_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        assign(zrbf, i & 1);
        flush(probe1, 1, probe_stride);
        flush(probe3, 1, probe_stride);
        for (volatile int z = 0; z < 50; z++) {}

        do_not(&zrbf[zrbf_stride*0], probe1, probe2, probe3);

        if (read_bit == ((i & 1) == 0)) correct++;
    }
    end_t = clock();

    printf("=== NOT gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

void xor_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        __clear_cache(or_buf, or_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, i & 1);
        assign((uintptr_t*)probe2, i & 2);
        flush(probe3, 1, probe_stride);

        do_or((void*) 0, probe1, probe2, probe3);
        int l_val = read_bit;
        
        __clear_cache(and_buf, and_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, i & 1);
        assign((uintptr_t*)probe2, i & 2);
        flush(probe3, 1, probe_stride);

        do_and((void*) 0, probe1, probe2, probe3);

        __clear_cache(not_buf, not_buf + CODE_SIZE + 1);
        flush(probe1, 1, probe_stride);
        assign(zrbf, read_bit);
        flush(probe3, 1, probe_stride);
        do_not(&zrbf[zrbf_stride*0], probe1, probe2, probe3);
        
        __clear_cache(and_buf, and_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, l_val);
        assign((uintptr_t*)probe2, read_bit);
        flush(probe3, 1, probe_stride);

        do_and((void*) 0, probe1, probe2, probe3);
        if (read_bit == (((i & 3) == 1) || ((i & 3) == 2))) correct++;
    }
    end_t = clock();

    printf("=== XOR gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

void mux_gate() {
    unsigned correct = 0;
    clock_t end_t, start_t = clock();

    for (int i = 0; i < ITER; i++) {
        __clear_cache(not_buf, not_buf + CODE_SIZE + 1);
        flush(probe1, 1, probe_stride);
        assign(zrbf, i & 4);
        flush(probe3, 1, probe_stride);
        do_not(&zrbf[zrbf_stride*0], probe1, probe2, probe3);

        __clear_cache(and_buf, and_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, read_bit);
        assign((uintptr_t*)probe2, i & 1);
        flush(probe3, 1, probe_stride);

        do_and((void*) 0, probe1, probe2, probe3);
        int l_val = read_bit;
        
        __clear_cache(and_buf, and_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, i & 2);
        assign((uintptr_t*)probe2, i & 4);
        flush(probe3, 1, probe_stride);

        do_and((void*) 0, probe1, probe2, probe3);
        
        __clear_cache(or_buf, or_buf + CODE_SIZE + 1);
        assign((uintptr_t*)probe1, l_val);
        assign((uintptr_t*)probe2, read_bit);
        flush(probe3, 1, probe_stride);

        do_or((void*) 0, probe1, probe2, probe3);

        if (read_bit == ((i & 4) ? ((i & 2) > 0) : (i & 1))) correct++;
    }
    end_t = clock();

    printf("=== MUX gate (ARM) ===\n");
    printf("Correct rate: %.3f%%\n", ((double)correct * 100) / ITER);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / (CLOCKS_PER_SEC << 1));
    printf("over %d iterations.\n", ITER);
}

int main(int argc, char** argv){
    struct sigaction act;
    act.sa_sigaction = get_value;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, NULL);

    codebuf = mmap(NULL, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    not_buf = mmap(NULL, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assign_buf = mmap(NULL, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    or_buf = mmap(NULL, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    and_buf = mmap(NULL, 4096, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(codebuf != MAP_FAILED);
    assert(not_buf != MAP_FAILED);
    assert(assign_buf != MAP_FAILED);
    assert(or_buf != MAP_FAILED);
    assert(and_buf != MAP_FAILED);
    memcpy(not_buf, not_gate2, CODE_SIZE);
    memcpy(assign_buf, assign_gate, CODE_SIZE);
    memcpy(or_buf, or_gate, CODE_SIZE);
    memcpy(and_buf, and_gate, CODE_SIZE);
    do_gate = (gate_fn)codebuf;
    do_not = (gate_fn)not_buf;
    do_assign = (gate_fn)assign_buf;
    do_or = (gate_fn)or_buf;
    do_and = (gate_fn)and_buf;

    probe1 = mmap(NULL, probe_size, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(probe1 != MAP_FAILED);
    probe2 = mmap(NULL, probe_size, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(probe2 != MAP_FAILED);
    probe3 = mmap(NULL, probe_size, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(probe3 != MAP_FAILED);

    zrbf = mmap(NULL, zrbf_size, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    assert(zrbf != MAP_FAILED);
    zrbf[0] = 0;

    // set up the dereference chain used to stall execution
    for (int i = 1; i < 10; i++) {
        zrbf[i*zrbf_stride] = (uintptr_t)&zrbf[(i-1)*zrbf_stride];
    }

    // trigger CoW on the probe pages
    for (int i = 0; i < probe_size; i += 1) {
        probe1[i] = 0;
        probe2[i] = 0;
        probe3[i] = 0;
    }

    pthread_t inc_counter_thread;
    if(pthread_create(&inc_counter_thread, NULL, inc_counter, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    while(counter == 0);
    asm volatile ("DSB SY");

    miss_min = measure_latency();
    if (miss_min == 0) {
        fprintf(stderr, "Unreliable access timing\n");
        exit(1);
    }
    miss_min -= 1;

    test_and_gate();
    test_or_gate();
    test_assign_gate();
    test_not_gate();
    xor_gate();
    mux_gate();

    return 0;
}
