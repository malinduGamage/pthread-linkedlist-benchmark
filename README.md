# Pthread Linkedlist Benchmark (Lab 1 – Concurrent Programming, CS4532)

This project benchmarks three singly linked list implementations under mixed workloads:

1. **Serial** (baseline, no locks)
2. **Pthreads + single mutex** (one global `pthread_mutex_t`)
3. **Pthreads + single read–write lock** (one global `pthread_rwlock_t`)

We compare performance across thread counts and operation mixes to study contention and read-parallelism.

## Problem Statement (from lab brief)

- Implement a linked list that supports `Member`, `Insert` (unique keys), and `Delete`.
- Initialize the list with `n` unique random integers in the range `[0, 2^16 - 1]`.
- Execute `m` operations with the fractions `mMember`, `mInsert`, and `mDelete` distributed over `T` threads.
- Measure **only** the **m-ops** section.
- Report averages and standard deviations with **±5% @ 95% CI**.
- Plot **time vs threads** for three cases: read-heavy, medium-write, heavy-write.

## Techniques & Rationale

- **Single Mutex**: Serializes all operations—simple but contention-heavy.
- **RW-Lock**: Enables concurrent readers; benefits diminish as the write fraction rises.
- **Sorted Insert + Existence Checks**: Ensures uniqueness constraints without per-node locks.

## Directory Layout

```
lab1_linkedlist/
├─ src/
│  ├─ include/           # headers: linkedlist.h, workload.h, timing.h
│  ├─ linkedlist/        # linkedlist.c
│  ├─ workload/          # workload.c
│  ├─ timing/            # timing.c
│  └─ apps/              # main programs: serial/mutex/rwlock
├─ bin/                  # built executables (created by make)
├─ scripts/              # run, summarize, plot
├─ data/                 # raw/summary CSV
├─ report/graphs/        # PNG plots
└─ README.md
```

## Prerequisites

- **gcc/clang** with `-pthread`
- **Python 3** with `pandas` and `matplotlib`:

  ```bash
  pip install pandas matplotlib
  ```

## Build, Run, and Analyze

### 1) Build all binaries into `./bin`

```bash
make
```

### 2) Run the full experiment suite (Cases 1–3, T∈{1,2,4,8}, default `SAMPLES=5`)

```bash
make run            # or: make run SAMPLES=10
```

### 3) Summarize raw CSV → summary CSV (average, standard deviation)

```bash
make results
```

### 4) Generate plots (per case + combined)

```bash
make plot
```

### Clean all generated files

```bash
make clean
```

### Case Definitions

- **Case 1**: `n=1000`, `m=10000`, `mMember=0.99`, `mInsert=0.005`, `mDelete=0.005`
- **Case 2**: `n=1000`, `m=10000`, `mMember=0.90`, `mInsert=0.05`, `mDelete=0.05`
- **Case 3**: `n=1000`, `m=10000`, `mMember=0.50`, `mInsert=0.25`, `mDelete=0.25`

## Machine Specification (Include in Report)

- **CPU Model**: Intel Xeon
- **Physical Cores**: 4
- **Threads**: 8 (if applicable)
- **RAM**: Size & speed (optional)
- **OS**: Ubuntu 18.04 LTS
- **Kernel Version**: \[Kernel version]
- **Compiler**: `gcc --version` (or clang)
- **Other System Details**: Any other information that might affect performance (VM/cloud details)

## Notes on Timing & Statistics

- Use `clock_gettime(CLOCK_MONOTONIC, …)` to time **only** the **m-ops** region.
- For 95% CI of the mean, the half-width ≈ `1.96 * s / sqrt(k)`; increase `k` (samples) until the result is ≤ 5% of the mean.

## Expected Behavior (To Discuss)

- **T=1**: `serial < mutex < rwlock` (lock overhead).
- **Read-heavy**: RW-lock > mutex (concurrent reads).
- **Write-heavy**: RW-lock ≈ mutex (writes serialize both).
- **Diminishing Returns**: Beyond core count, contention dominates.

## License

This work is for educational use in the **CS4532 Concurrent Programming** lab.

---
