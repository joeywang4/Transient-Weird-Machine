#!/usr/bin/env python3
"""Calculate the median accuracy and time usage"""
import argparse
import os
import subprocess
import sys
import statistics
from shutil import rmtree
from collections import defaultdict

LOG = "./log"


def clear_log(path):
    """Clear log folder"""
    rmtree(path, True)
    os.mkdir(LOG)


def run_proc(cmd, cwd, output=""):
    """Run a subprocess"""
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, cwd=cwd, check=False)
    if len(output) > 0:
        with open(output, "wb") as ofile:
            ofile.write(proc.stdout)


def progress(i, total):
    """Show progress bar"""
    print(f"[*] Progress: {i}/{total}", end="\r", file=sys.stdout, flush=True)


def parse_log(filename):
    """Parse execution result and return accuracy and time usage"""
    output = {}  # gate -> (acc, time)
    with open(filename, "r", encoding="utf-8") as infile:
        lines = infile.read().splitlines()

    i = 0
    while i < len(lines) - 2:
        if lines[i].find("===") == -1:
            i += 1
            continue

        name = lines[i][4 : lines[i][4:].find("===") + 4]
        beg = lines[i + 1].find("= ")
        beg = beg + 3 if beg != -1 else lines[i + 1].find(": ") + 2
        acc = float(lines[i + 1][beg : lines[i + 1].find("%")])
        sec = float(lines[i + 2][lines[i + 2].find(": ") + 2 : lines[i + 2].find("s ")])

        output[name] = (acc, sec)
        i += 3

    return output


def report(results):
    """Print median and mean values"""
    for name, acc_sec in results.items():
        accs, secs = list(zip(*acc_sec))
        acc_med, acc_avg = statistics.median(accs), statistics.mean(accs)
        sec_med, sec_avg = statistics.median(secs), statistics.mean(secs)

        print(f"=== {name} ===")
        print(f"Accuracy: median = {acc_med:.4f}%, mean = {acc_avg:.4f}%")
        print(f"Time usage: median = {sec_med:.3f}s, mean = {sec_avg:.3f}s")


def run_intel(trials):
    """Compile and run Intel version"""
    print(f"[*] Testing on Intel ({trials} trials)")
    run_proc(["make", "clean"], "./exceptions")
    run_proc(["make"], "./exceptions")
    run_proc(["make", "clean"], "./spectre")
    run_proc(["make"], "./spectre")
    run_proc(["make", "clean"], "./spectrev2")
    run_proc(["make"], "./spectrev2")
    run_proc(["make", "clean"], "./TSX")
    run_proc(["make"], "./TSX")

    results = defaultdict(list)

    for i in range(trials):
        name = f"{LOG}/intel-exceptions-{i + 1}.log"
        run_proc("./main.elf", "./exceptions", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)

        name = f"{LOG}/intel-spectre-{i + 1}.log"
        run_proc("./spectre.elf", "./spectre", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)

        name = f"{LOG}/intel-spectrev2-{i + 1}.log"
        run_proc("./spectrev2.elf", "./spectrev2", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)

        name = f"{LOG}/intel-TSX-{i + 1}.log"
        run_proc("./tsx.elf", "./TSX", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)

        progress(i + 1, trials)

    report(results)


def run_amd(trials):
    """Compile and run AMD version"""
    print(f"[*] Testing on AMD ({trials} trials)")
    run_proc(["make", "clean"], "./exceptions")
    run_proc(["make", "amd"], "./exceptions")

    results = defaultdict(list)

    for i in range(trials):
        name = f"{LOG}/amd-{i + 1}.log"
        run_proc("./main-amd.elf", "./exceptions", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)
        progress(i + 1, trials)

    report(results)


def run_arm(trials):
    """Compile and run ARM version"""
    print(f"[*] Testing on ARM ({trials} trials)")
    run_proc(["make", "clean"], "./ARM")
    run_proc(["make"], "./ARM")

    results = defaultdict(list)

    for i in range(trials):
        name = f"{LOG}/arm-{i + 1}.log"
        run_proc("./arm.elf", "./ARM", name)
        got = parse_log(name)
        for k, val in got.items():
            results[k].append(val)
        progress(i + 1, trials)

    report(results)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("arch", help="Processor architecture (intel/amd/arm)")
    parser.add_argument("-t", "--trials", type=int, default=10, help="Number of trials")
    args = parser.parse_args()

    clear_log(LOG)

    if args.arch.lower() == "intel":
        run_intel(args.trials)
    elif args.arch.lower() == "amd":
        run_amd(args.trials)
    elif args.arch.lower() == "arm":
        run_arm(args.trials)
    else:
        print("[!] arch should be one of Intel/AMD/ARM")
