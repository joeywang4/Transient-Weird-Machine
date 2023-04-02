#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <argp.h>
#include <time.h>
#include <csignal>
#include <vector>
#include "gates/single.h"
#include "gates/compose.h"

/* Config */
#define THRESHOLD 200
bool verbose = false;
unsigned tot_trials = 100;
unsigned single_trial = 10000;
#ifdef INTEL
#define DELAY 64
#else
#define DELAY 512
#endif

/* Side channel registers */
uint8_t reg1[4*512];
uint8_t reg2[4*512];
uint8_t reg3[4*512];
uint8_t reg4[4*512];
uint8_t reg5[4*512];

/* Gate init, exec, measure */
bool do_assign_gate(unsigned input) {
    reg1[0] = 0;
    assign(reg1, input & 1);
    _mm_clflush(reg2);
    _mm_clflush(reg3);
    for (volatile int z = 0; z < DELAY; z++) {}

    assign_gate(reg1, reg2, reg3);

    #ifndef INTEL
    for (volatile int z = 0; z < DELAY; z++) {}
    #endif

    uint64_t clk = timer(reg2);
    uint64_t clk2 = timer(reg3);
    return (
        ((clk <= THRESHOLD) == (input & 1)) && 
        ((clk2 <= THRESHOLD) == (input & 1))
    );
}

bool do_or_gate(unsigned input) {
    reg1[0] = 0;
    reg2[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    _mm_clflush(reg3);
    for (volatile int z = 0; z < DELAY; z++) {}

    or_gate(reg1, reg2, reg3);

    #ifndef INTEL
    for (volatile int z = 0; z < DELAY; z++) {}
    #endif

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == ((input & 3) > 0);
}

bool do_and_gate(unsigned input) {    
    reg1[0] = 0;
    reg2[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    _mm_clflush(reg3);
    for (volatile int z = 0; z < DELAY; z++) {}

    and_gate(reg1, reg2, reg3);

    #ifndef INTEL
    for (volatile int z = 0; z < DELAY; z++) {}
    #endif

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == ((input & 3) == 3);
}

bool do_not_gate(unsigned input) {
    reg1[0] = 0;
    reg2[0] = 0;
    reg4[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 1);
    _mm_clflush(reg3);
    _mm_clflush(reg4);
    for (volatile int z = 0; z < DELAY; z++) {}

    not_gate(reg1, reg2, reg3, reg4);

    #ifndef INTEL
    for (volatile int z = 0; z < DELAY; z++) {}
    #endif

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == ((input & 1) == 0);
}

bool do_nand_gate(unsigned input) {    
    reg1[0] = 0;
    reg2[0] = 0;
    reg3[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    _mm_clflush(reg3);
    for (volatile int z = 0; z < 64; z++) {}

    nand_gate(reg1, reg2, reg3);
    for (volatile int z = 0; z < 64; z++) {}

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == ((input & 3) != 3);
}

bool do_xor_gate(unsigned input) {    
    reg1[0] = 0;
    reg2[0] = 0;
    reg3[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    _mm_clflush(reg3);

    xor_gate(reg1, reg2, reg3, input);
    for (volatile int z = 0; z < 512; z++) {}

    uint64_t clk = timer(reg3);
    return (clk <= THRESHOLD) == (
        ((input & 3) > 0) && ((input & 3) < 3)
    );
}

bool do_mux_gate(unsigned input) {    
    reg1[0] = 0;
    reg2[0] = 0;
    reg3[0] = 0;
    reg4[0] = 0;
    assign(reg1, input & 1);
    assign(reg2, input & 2);
    assign(reg3, input & 4);
    _mm_clflush(reg4);

    mux_gate(reg1, reg2, reg3, reg4, input);
    for (volatile int z = 0; z < 512; z++) {}

    uint64_t clk = timer(reg4);
    return (clk <= THRESHOLD) == (
        (input & 4) ? ((input & 2) == 2) : (input & 1)
    );
}

/* Jump over a WG after an exception */
void signal_handler(int signal, siginfo_t *si, void *context)
{
    const int return_delta = 256;
    ((ucontext_t*)context)->uc_mcontext.gregs[REG_RIP] += return_delta;
}

/* Report accuracy */
void calc_avg_std(
    std::vector<unsigned>& tot_counts,
    std::vector<unsigned>& tot_error_counts,
    unsigned input_size
) {
    const unsigned in_space = 1 << input_size;
    double sum = 0;
    std::vector<double> sum_error(in_space, 0);
    std::vector<double> avg_error(in_space, 0);

    for (int i = 0; i < tot_trials; i++) {
        sum += tot_counts[i];
        for (int j = 0; j < in_space; j++) {
            sum_error[j] += tot_error_counts[(i * in_space) + j];
        }
    }

    double avg = sum / tot_trials;
    sum = 0;
    for (int i = 0; i < in_space; i++) {
        avg_error[i] = sum_error[i]/tot_trials;
        sum_error[i] = 0;
    }

    for (int i = 0; i < tot_trials; i++) {
        sum += (tot_counts[i] - avg) * (tot_counts[i] - avg);
        for (int j = 0; j < in_space; j++) {
            double tmp = tot_error_counts[(i * in_space) + j] - avg_error[j];
            sum_error[j] +=  tmp * tmp;
        }
    }
    
    printf(
        "Correct rate: (avg, std) = (%.4lf%%, %.4lf)\n", 
        (avg * 100) / single_trial, 
        sqrt(sum / tot_trials) / single_trial
    );
    if (!verbose) return;
    for (int i = 0; i < in_space; i++) {
        printf("Input (%d", i&1);
        for (int j = 1; j < input_size; j++)
            printf(", %d", (i & (1 << j)) >> j);
        printf(") Error rate: (avg, std) = ");
        printf("(%.4lf%%, %.4lf)\n", (avg_error[i] * 100) / single_trial, sqrt(sum_error[i] / tot_trials) / single_trial);
    }
}

/* Test the accuracy and time usage of a WG */
void test_gate(
    const char* name,
    bool (*gate_fn)(unsigned), 
    unsigned input_size
) {
    const unsigned in_space = 1 << input_size;
    std::vector<unsigned> tot_counts(tot_trials, 0);
    std::vector<unsigned> tot_error_counts(tot_trials * in_space, 0);    
    clock_t end_t, start_t = clock();

    for (unsigned trial = 0; trial < tot_trials; trial++) {
        unsigned seed = 0;

        for (int seed = 0; seed < single_trial; seed++) {
            bool correct = gate_fn(seed);

            if (correct) {
                tot_counts[trial]++;
            }
            else {
                tot_error_counts[(trial * in_space) + (seed % in_space)]++;
            }
        }
    }
    
    end_t = clock();

    printf("=== %s gate (Exception) ===\n", name);
    calc_avg_std(tot_counts, tot_error_counts, input_size);
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / CLOCKS_PER_SEC);
    printf("over %d iterations.\n", tot_trials * single_trial);
}

/* Arguments */
static char doc[] = "Test the accuracy and run time of exception-based weird gates.";
static char args_doc[] = "";
static struct argp_option options[] = { 
    { "verbose", 'v', 0, 0, "Produce verbose output"},
    { "iter", 'i', "ITER", 0, "Number of iterations in each trial (default: 10000)."},
    { "trial", 't', "TRIAL", 0, "Number of trials (default: 100)."},
    { 0 } 
};
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'v': verbose = true; break;
        case 'i': single_trial = atoi(arg); break;
        case 't': tot_trials = atoi(arg); break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char* argv[]) {
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;

    // Install signal handler
    sigaction(SIGFPE, &sa, NULL);

    argp_parse(&argp, argc, argv, 0, 0, 0);

    test_gate("AND",    do_and_gate,    2);
    test_gate("OR",     do_or_gate,     2);
    test_gate("ASSIGN", do_assign_gate, 1);
    test_gate("NOT",    do_not_gate,    1);
    test_gate("NAND",   do_nand_gate,   2);
    test_gate("XOR",    do_xor_gate,    2);
    test_gate("MUX",    do_mux_gate,    3);
}
