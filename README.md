# Transient Weird Machine

This repository contains the transient weird machines implemented for ARM64 and x86-64 machines using the exception, TSX, branch predictor, and branch target buffer as the transient execution primitives.

## Requirement

We recommend using the Ubuntu operating system to execute our code, while most Unix-based operating systems should be able to execute our weird machine.

To compile the code, the following software packages should be installed:
1. gcc/g++ (version >= 7.3.1)
2. GNU Make (version >= 3.82)

These packages can be installed using the command below on Ubuntu:
```bash
sudo apt install build-essential
```

## Tested Processors

Since different processors have different microarchitectures, the accuracy and performance of our transient weird machine may change accross different machines. We tested our code using the following machines, which are provided by the AWS EC2 cloud platform.

- Intel Xeon CPU E7-8880 v3 @ 2.30GHz (AWS EC2 x1e.xlarge)
- AMD EPYC 7R13 Processor (AWS EC2 c6a.large)
- ARM-based AWS Graviton Processors (AWS EC2 a1.large)

## Execution

On an __Intel__ machine, please execute the `run_all.sh` script to test the accuracy and performance of our weird machine.
```bash
chmod +x ./run_all.sh
./run_all.sh
```

On an __AMD__ machine, please instead execute the `run_amd.sh` script.
```bash
chmod +x ./run_amd.sh
./run_amd.sh
```

On an __ARM64__ machine, please instead execute the `run_arm.sh` script.
```bash
chmod +x ./run_arm.sh
./run_arm.sh
```

## Example output

On a successful execution, our weird machine generates the following output:

__Intel__
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

__AMD__
```
** Testing exception-based gates...
rm -rf *.o build/ *.elf *~
mkdir build
g++ -O2 -c -o build/compose-amd.o gates/compose.cpp
g++ -O2 -o main-amd.elf main.cpp build/compose-amd.o -lm
=== AND gate (Exception) ===
Correct rate: (avg, std) = (99.9655%, 0.0014)
Time usage: 1.811s over 1000000 iterations.
=== OR gate (Exception) ===
Correct rate: (avg, std) = (99.9911%, 0.0003)
Time usage: 1.755s over 1000000 iterations.
=== ASSIGN gate (Exception) ===
Correct rate: (avg, std) = (99.9521%, 0.0022)
Time usage: 1.741s over 1000000 iterations.
=== NOT gate (Exception) ===
Correct rate: (avg, std) = (99.8425%, 0.0009)
Time usage: 1.780s over 1000000 iterations.
=== NAND gate (Exception) ===
Correct rate: (avg, std) = (96.8500%, 0.0418)
Time usage: 4.036s over 1000000 iterations.
=== XOR gate (Exception) ===
Correct rate: (avg, std) = (96.9495%, 0.0648)
Time usage: 7.351s over 1000000 iterations.
=== MUX gate (Exception) ===
Correct rate: (avg, std) = (97.0653%, 0.0443)
Time usage: 5.831s over 1000000 iterations.

** Done!
```

__ARM64__
```
** Testing ARM gates...
** This can take several minutes to complete...
rm arm.elf
gcc -o arm.elf signal.c gates.s -lpthread
=== AND gate (ARM) ===
Correct rate: 99.651%
Time usage: 32.739s over 1000000 iterations.
=== OR gate (ARM) ===
Correct rate: 99.337%
Time usage: 32.654s over 1000000 iterations.
=== ASSIGN gate (ARM) ===
Correct rate: 99.258%
Time usage: 32.454s over 1000000 iterations.
=== NOT gate (ARM) ===
Correct rate: 98.821%
Time usage: 32.567s over 1000000 iterations.
=== XOR gate (ARM) ===
Correct rate: 97.864%
Time usage: 131.349s over 1000000 iterations.
=== MUX gate (ARM) ===
Correct rate: 94.585%
Time usage: 131.303s over 1000000 iterations.

** Done!
```