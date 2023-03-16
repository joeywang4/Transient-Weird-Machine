#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <x86intrin.h>

#define cache_hit_threshold 80
#define ITERATIONS 1000000
#define mix_i 65 // < 256

unsigned int array1_size = 16;
uint8_t array1[16] = {
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16
};
uint8_t array2[256 * 512];
uint8_t array_tmp[256 * 512];
uint8_t array_in1[256 * 512];
uint8_t array_in2[256 * 512];

volatile uint8_t temp = 0; /* Used so compiler won’t optimize out gate_function() */
void gate_function(size_t x, size_t y) {
    if (x < array1_size) {
        temp &= array2[array_tmp[array1[x] + y] * 512];
    }
}

int run_gate(size_t x_offset, size_t y_offset, int input) {
    int j;
    unsigned int junk = 0;
    size_t x, y;
    register uint64_t time1, time2;
    volatile uint8_t * addr;

    _mm_clflush(&array2[mix_i * 512]);

    /* 6 loops: 5 training runs */
    for (j = 5; j >= 0; j--) {
        _mm_clflush(&array1_size);

        /* Bit twiddling to set x and y if j % 6 != 0 */
        /* Avoid jumps in case those tip off the branch predictor */

        /* Set x = 0xFFFFFFFFFFFF0000 if j % 6 == 0, else x = 0 */
        x = ((j % 6) - 1) & ~0xFFFF; 
        /* Set x = -1 if j & 6 = 0, else x = 0 */
        x = (x | (x >> 16));
        y = x & y_offset;
        x = x & x_offset;
        array_in1[4096 - ((input & 1) << 12)] = 0;
        _mm_clflush(&array_in1[(input & 1) << 12]);
        array_in2[4096 - ((input & 2) << 11)] = mix_i;
        _mm_clflush(&array_in2[(input & 2) << 11]);

        /* Delay (can also mfence) */
        for (volatile int z = 0; z < 64; z++) {}
        gate_function(x, y);
    }

    /* Time reads. Order is lightly mixed up to prevent stride prediction */
    addr = &array2[mix_i * 512];
    time1 = __rdtscp(&junk); /* READ TIMER */
    junk = *addr; /* MEMORY ACCESS TO TIME */
    time2 = __rdtscp(&junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */
    if ((int)time2 <= cache_hit_threshold) {
        return mix_i;
    }
    return 0;
}

int main() {
    for (int i = 0; i < 256; i++) {
        array_in1[i] = i & 0xFF;
        array_in2[i] = mix_i;
        array_tmp[i] = 0;
    }    

    size_t x_offset = (size_t)(array_in1 - array1);
    size_t y_offset = (size_t)(array_in2 - array_tmp);
    int i;
    for (i = 0; i < (int)sizeof(array2); i++) {
        array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
    }

    clock_t end_t, start_t = clock();

    int correct = 0;
    for (int seed = 0; seed < ITERATIONS; seed++) {
        int got = run_gate(x_offset, y_offset, seed);
        if ((got == mix_i) == ((seed & 3) == 3)) correct++;
    }
    end_t = clock();

    printf("=== AND gate (Branch predictor) ===\n");
    printf("Correct rate: %.2f%%\n", 100 * ((double)correct / ITERATIONS));
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / CLOCKS_PER_SEC);
    printf("over %d iterations.\n", ITERATIONS);
    return 0;
}
