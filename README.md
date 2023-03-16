# Transient Weird Machine

This repository contains the transient weird machines implemented for ARM64 and x86-64 machines using the exception, TSX, branch predictor, and branch target buffer as the transient execution primitives.

## Requirement

We recommend using the Ubuntu operating system to execute our code, while most Unix-based operating systems should be able to execute our weird machine.

To compile the code, the following software packages should be installed:
1. gcc/g++ (version >= 7.3.1)
2. GNU Make (version >= 4.1)

## Tested Processors

Since different processors have different microarchitectures, the accuracy and performance of our transient weird machine may change accross different machines. We tested our code using the following machines, which are provided by the AWS EC2 cloud platform.

- Intel(R) Xeon(R) CPU E7-8880 v3 @ 2.30GHz (AWS EC2 x1e.xlarge)
- AMD **todo**
- ARM **todo**

## Execution

On an x86-64 machine, please execute the `run_all.sh` script to test the accuracy and performance of our weird machine.
```bash
chmod +x ./run_all.sh
./run_all.sh
```

On an ARM64 machine, please instead execute the `run_arm.sh` script.
```bash
chmod +x ./run_arm.sh
./run_arm.sh
```

## Example output

On a successful execution, our weird machine generates the following output:

__x86-64__
```
** Testing exception-based gates...
rm -rf *.o build/ main.elf *~
mkdir build
g++ -O2 -c -o build/compose.o gates/compose.cpp
g++ -O2 -o main.elf main.cpp build/compose.o -lm
=== ASSIGN gate (Exception) ===
Correct rate: (avg, std) = (99.6563%, 0.0042)
Time usage: 2.709s over 1000000 iterations.
=== OR gate (Exception) ===
Correct rate: (avg, std) = (99.9365%, 0.0019)
Time usage: 2.545s over 1000000 iterations.
=== AND gate (Exception) ===
Correct rate: (avg, std) = (99.8955%, 0.0015)
Time usage: 2.650s over 1000000 iterations.
=== NOT gate (Exception) ===
Correct rate: (avg, std) = (99.9430%, 0.0012)
Time usage: 2.770s over 1000000 iterations.
=== NAND gate (Exception) ===
Correct rate: (avg, std) = (99.0637%, 0.0079)
Time usage: 7.829s over 1000000 iterations.
=== XOR gate (Exception) ===
Correct rate: (avg, std) = (96.2333%, 0.0839)
Time usage: 17.665s over 1000000 iterations.
=== MUX gate (Exception) ===
Correct rate: (avg, std) = (97.1245%, 0.0529)
Time usage: 14.798s over 1000000 iterations.

** Testing branch predictor-based gates...
rm -f spectre.elf
cc -std=c99 -O0 -o spectre.elf spectre.c
=== AND gate (Branch predictor) ===
Correct rate: 94.80%
Time usage: 3.318s over 1000000 iterations.

** Testing branch target buffer-based gates...
rm -f spectrev2.elf
cc -std=c99 -O0 -o spectrev2.elf spectrev2.c
=== AND gate (Branch target buffer) ===
Correct rate: 95.77%
Time usage: 4.570s over 1000000 iterations.

** Testing TSX-based gates...
rm -f *.o tsx.elf *~
gcc -O2 -o tsx.elf tsx.c -lm
=== AND gate (TSX) ===
Correct rate: (avg, std) = (99.9033%, 0.0039)
Time usage: 0.561s over 1000000 iterations.

** Done!
```

__ARM64__
```
** Testing ARM gates...
** This can take several minutes to complete...
rm arm.elf
gcc -o arm.elf signal.c gates.s -lpthread
=== OR gate (ARM) ===
Correct rate: 99.586%
Time usage: 134.007s over 1000000 iterations.
=== AND gate (ARM) ===
Correct rate: 98.141%
Time usage: 134.079s over 1000000 iterations.
=== NOT gate (ARM) ===
Correct rate: 99.061%
Time usage: 134.112s over 1000000 iterations.
=== XOR gate (ARM) ===
Correct rate: 97.434%
Time usage: 536.687s over 1000000 iterations.
=== MUX gate (ARM) ===
Correct rate: 97.293%
Time usage: 536.899s over 1000000 iterations.

** Done!
```
