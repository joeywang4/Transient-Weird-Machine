# Transient Weird Machine

This repository contains the transient weird machines implemented for ARM64 and x86-64 machines using the exception, TSX, branch predictor, and branch target buffer as the transient execution primitives.

These weird machines can perform logic operations (e.g., `OR`, `AND`, `NOT`, ...) in the cache states using transient execution, and their computations are purely microarchitectural. This means that no existing deubgger or analysis tool can capture their computation states during execution.

For more details, please refer to our paper:
> Ping-Lun Wang, Fraser Brown, and Riad S. Wahby.
> "The ghost is the machine: Weird machines in transient execution."
> WOOT, 2023.

## Requirement

We recommend using the Ubuntu operating system to execute our code, while most Unix-based operating systems should be able to execute our weird machine.

To compile the code, the following software packages should be installed:
1. gcc/g++ (version >= 7.3.1)
2. GNU Make (version >= 3.82)

These packages can be installed using the command below on Ubuntu:
```bash
sudo apt install build-essential
```

## Tested processors

Since different processors have different microarchitectures, the accuracy and performance of our transient weird machine may change accross different machines. We tested our code using the following machines, which are provided by the AWS EC2 cloud platform.

- Intel Xeon CPU E7-8880 v3 @ 2.30GHz (AWS EC2 x1e.xlarge)
- AMD EPYC 7R13 Processor (AWS EC2 c6a.large)
- ARM-based AWS Graviton Processors (AWS EC2 a1.large)

## Reproduce our results

We provide scripts to execute a [single run](#single-run) (1M operations) of our weird gates or [multiple runs](#multiple-runs) to measure their median accuracy and runtime.

### Single run

The scripts below will compile the code and execute all the weird gates for 1M operations.

On an __Intel__ machine, please execute the [`run_all.sh`](run_all.sh) script to test the accuracy and performance of our weird machine.
```bash
./run_all.sh
```

On an __AMD__ machine, please instead execute the [`run_amd.sh`](run_amd.sh) script.
```bash
./run_amd.sh
```

On an __ARM64__ machine, please execute the [`run_arm.sh`](run_arm.sh) script.
```bash
./run_arm.sh
```

### Multiple runs

Due to the noises in the cache side channel, the accuracy of the weird gates can sometimes be much lower than the results reported in our paper. To remove the effect of these outliers, we also provide a python script ([median.py](median.py)) to measure the median and average accuracy accross multiple executions. Please make sure python3 is installed before running this script.

Usage: `python3 median.py [-h] [-t TRIALS] arch`
- `TRIALS`: the number of executions (default to 10)
- `arch`: the processor architecture (Intel/AMD/ARM)

## Development

To construct a customized circuit of our weird gates, simply include the definitions of the basic gates (e.g., `OR`, `AND`, and `NOT`) and then chain their inputs and outputs together.
Our `NAND`, `XOR`, and `MUX` gates are also circuits of these basic gates, and they are great starting points to learn how to construct other circuits.
The definitions of these existing circuits can be found in the following files:
- x86-64: [exceptions/gates/compose.cpp](exceptions/gates/compose.cpp)
- ARM64: [ARM/signal.c](ARM/signal.c) (in functions [`xor_gate`](ARM/signal.c#L197) and [`mux_gate`](ARM/signal.c#L239))

It can be challenging to construct a large circuit while maintaining its accuracy. We plan to design a compiler to build large circuits automatically in the future.
