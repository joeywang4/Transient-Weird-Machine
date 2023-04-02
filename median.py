#!/usr/bin/env python3
import argparse, os, subprocess, sys, statistics
from shutil import rmtree
from collections import defaultdict

log = "./log"

def clear_log(dir):
    rmtree(dir, True)
    os.mkdir(log)

def run_proc(cmd, cwd, output=""):
    p = subprocess.run(cmd, stdout=subprocess.PIPE, cwd=cwd)
    if len(output) > 0:
        with open(output, 'wb') as ofile:
            ofile.write(p.stdout)

def progress(i, total):
    print("[*] Progress: {}/{}".format(i, total), 
            end='\r', file=sys.stdout, flush=True)

def parse_log(filename):
    output = {} # gate -> (acc, time)
    with open(filename, 'r') as infile:
        lines = infile.read().splitlines()
    
    i = 0
    while i < len(lines) - 2:
        if lines[i].find("===") == -1:
            i += 1
            continue
        
        name = lines[i][4 : lines[i][4:].find("===") + 4]
        beg = lines[i+1].find("= ")
        beg = beg + 3 if beg != -1 else lines[i+1].find(": ") + 2
        acc = float(lines[i+1][
            beg :
            lines[i+1].find("%")
        ])
        sec = float(lines[i+2][
            lines[i+2].find(": ") + 2 :
            lines[i+2].find("s ")
        ])

        output[name] = (acc, sec)
        i += 3
    
    return output

def report(results):
    for k, l in results.items():
        accs, secs = list(zip(*l))
        acc_med, acc_avg = statistics.median(accs), statistics.mean(accs)
        sec_med, sec_avg = statistics.median(secs), statistics.mean(secs)

        print(f"=== {k} ===")
        print(f"Accuracy: median = {acc_med:.4f}%, mean = {acc_avg:.4f}%")
        print(f"Time usage: median = {sec_med:.3f}s, mean = {sec_avg:.3f}s")

def run_intel(trials):
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
        name = f"{log}/intel-exceptions-{i + 1}.log"
        run_proc("./main.elf", "./exceptions", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)

        name = f"{log}/intel-spectre-{i + 1}.log"
        run_proc("./spectre.elf", "./spectre", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)

        name = f"{log}/intel-spectrev2-{i + 1}.log"
        run_proc("./spectrev2.elf", "./spectrev2", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)

        name = f"{log}/intel-TSX-{i + 1}.log"
        run_proc("./tsx.elf", "./TSX", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)

        progress(i + 1, trials)
    
    report(results)

def run_amd(trials):
    print(f"[*] Testing on AMD ({trials} trials)")
    run_proc(["make", "clean"], "./exceptions")
    run_proc(["make", "amd"], "./exceptions")
    
    results = defaultdict(list)

    for i in range(trials):
        name = f"{log}/amd-{i + 1}.log"
        run_proc("./main-amd.elf", "./exceptions", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)
        progress(i + 1, trials)
    
    report(results)

def run_arm(trials):
    print(f"[*] Testing on ARM ({trials} trials)")
    run_proc(["make", "clean"], "./ARM")
    run_proc(["make"], "./ARM")
    
    results = defaultdict(list)

    for i in range(trials):
        name = f"{log}/arm-{i + 1}.log"
        run_proc("./arm.elf", "./ARM", name)
        got = parse_log(name)
        for k, v in got.items():
            results[k].append(v)
        progress(i + 1, trials)
    
    report(results)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("arch", help="Processor architecture (intel/amd/arm)")
    parser.add_argument("-t", "--trials", type=int, default=10, help="Number of trials")
    args = parser.parse_args()

    clear_log(log)

    if args.arch.lower() == 'intel':
        run_intel(args.trials)
    elif args.arch.lower() == 'amd':
        run_amd(args.trials)
    elif args.arch.lower() == 'arm':
        run_arm(args.trials)
    else:
        print("[!] arch should be one of Intel/AMD/ARM")
