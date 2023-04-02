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
./run_all.sh
```

On an __AMD__ machine, please instead execute the `run_amd.sh` script.
```bash
./run_amd.sh
```

On an __ARM64__ machine, please instead execute the `run_arm.sh` script.
```bash
./run_arm.sh
```

## Stability Issue

Due to the noises in the cache side channel, the accuracy of the weird gates can sometimes be much lower than the results reported in our paper. To remove the effect of these outliers, we also provide a python script ([median.py](median.py)) to measure the median and average accuracy accross multiple executions. Please make sure python3 is installed before running this script.

Usage: `python3 median.py [-h] [-t TRIALS] arch`
- TRIALS: the number of executions (default to 10)
- arch: the processor architecture (Intel/AMD/ARM)
