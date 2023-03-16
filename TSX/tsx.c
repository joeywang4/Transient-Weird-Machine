#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <x86intrin.h>

#define _XBEGIN_STARTED (~0u)
#define __rtm_force_inline __attribute__((__always_inline__)) inline

static __rtm_force_inline int _xbegin2(void)
{
    int ret = _XBEGIN_STARTED;
    asm volatile(".byte 0xc7,0xf8; .long 0" : "+a" (ret) :: "memory");
    return ret;
}

static __rtm_force_inline void _xend2(void)
{
    asm volatile(".byte 0x0f,0x01,0xd5" ::: "memory");
}

/* Config */
#define THRESHOLD 200
bool verbose = false;
const unsigned tot_trials = 100;
const unsigned single_trial = 10000;

uint8_t reg1[4*512];
uint8_t reg2[4*512];
uint8_t reg3[4*512];

uint64_t timer(uint8_t* ptr) {
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
        : "rdx", "rsi", "eax"
    );
    return clk;
}

void inline assign(uint8_t* ptr, int input) {
    if (input) { ptr[0] = 0; }
    else { _mm_clflush(ptr); }
}

static inline void and_gate(uint8_t* in1, uint8_t* in2, uint8_t* out) {
    if(_xbegin2() == _XBEGIN_STARTED) {
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
        _xend2();
    }
}

bool do_and_gate(unsigned input) {    
    reg1[0] = 0;
    reg2[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    _mm_clflush(reg3);    
    for (volatile int z = 0; z < 64; z++) {}

    and_gate(reg1, reg2, reg3);
    for (volatile int z = 0; z < 64; z++) {}

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == ((input & 3) == 3);
}

/* Report accuracy */
void calc_avg_std(
    unsigned tot_counts[],
    unsigned tot_error_counts[]
) {
    double sum = 0;
    double sum_error[4] = {0, 0, 0, 0};
    double avg_error[4] = {0, 0, 0, 0};

    for (int i = 0; i < tot_trials; i++) {
        sum += tot_counts[i];
        for (int j = 0; j < 4; j++) {
            sum_error[j] += tot_error_counts[(i * 4) + j];
        }
    }

    double avg = sum / tot_trials;
    sum = 0;
    for (int i = 0; i < 4; i++) {
        avg_error[i] = sum_error[i]/tot_trials;
        sum_error[i] = 0;
    }

    for (int i = 0; i < tot_trials; i++) {
        sum += (tot_counts[i] - avg) * (tot_counts[i] - avg);
        for (int j = 0; j < 4; j++) {
            double tmp = tot_error_counts[(i * 4) + j] - avg_error[j];
            sum_error[j] +=  tmp * tmp;
        }
    }
    
    printf(
        "Correct rate: (avg, std) = (%.4lf%%, %.4lf)\n", 
        (avg * 100) / single_trial, 
        sqrt(sum / tot_trials) / single_trial
    );
    if (!verbose) return;
    for (int i = 0; i < 4; i++) {
        printf("Input (%d", i&1);
        for (int j = 1; j < 4; j++)
            printf(", %d", (i & (1 << j)) >> j);
        printf(") Error rate: (avg, std) = ");
        printf("(%.4lf%%, %.4lf)\n", (avg_error[i] * 100) / single_trial, sqrt(sum_error[i] / tot_trials) / single_trial);
    }
}

void test_gate(
    const char* name,
    bool (*gate_fn)(unsigned)
) {
    unsigned tot_counts[tot_trials];
    unsigned tot_error_counts[4 * tot_trials];
    clock_t end_t, start_t = clock();


    for (unsigned trial = 0; trial < tot_trials; trial++) {
        unsigned seed = 0;
        tot_counts[trial] = 0;
        for (int i = 0; i < 4; i++)
            tot_error_counts[(trial * 4) + i] = 0;

        for (int seed = 0; seed < single_trial; seed++) {
            bool correct = gate_fn(seed);

            if (correct) {
                tot_counts[trial]++;
            }
            else {
                tot_error_counts[(trial * 4) + (seed % 4)]++;
            }
        }
    }
    
    end_t = clock();

    printf("=== %s gate (TSX) ===\n", name);
    calc_avg_std(tot_counts, tot_error_counts);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / CLOCKS_PER_SEC);
    printf("over %d iterations.\n", tot_trials * single_trial);
}

int main() {
    test_gate("AND", do_and_gate);
}
