#!/bin/bash
# run_tests.sh
#
# Compile the linked list applications and execute a series of
# experiments for three predefined workloads.  For each workload and
# implementation, the script runs the program for thread counts of
# 1, 2, 4 and 8 and records the elapsed time.  Multiple samples are
# taken for each combination to allow statistical analysis.  Results
# are written to CSV files in the data/ directory.

# Exit immediately if a command exits with a non‑zero status.
set -e

# Determine the directory of this script.  All paths are relative to
# the project root.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Build the applications
make -C "$PROJECT_ROOT"

# Ensure the data directory exists
mkdir -p "$PROJECT_ROOT/data"

# Number of samples per configuration.  Defaults to 5 but can be
# overridden by passing an argument to the script.
SAMPLES=${1:-5}

echo "Running tests with $SAMPLES samples per configuration..."

for CASE in 1 2 3; do
    # Set workload parameters for each case
    if [ "$CASE" -eq 1 ]; then
        N=1000
        M=100000
        M_MEMBER=0.99
        M_INSERT=0.005
        M_DELETE=0.005
    elif [ "$CASE" -eq 2 ]; then
        N=1000
        M=100000
        M_MEMBER=0.90
        M_INSERT=0.05
        M_DELETE=0.05
    else
        N=1000
        M=100000
        M_MEMBER=0.50
        M_INSERT=0.25
        M_DELETE=0.25
    fi
    OUTFILE="$PROJECT_ROOT/data/case${CASE}_results.csv"
    # Remove any existing file before appending
    rm -f "$OUTFILE"
    touch "$OUTFILE"
    # Header row
    echo "implementation,threads,time" > "$OUTFILE"
    for IMPL in serial mutex rwlock; do
        for THREADS in 1 2 4 8; do
            for SAMPLE in $(seq 1 $SAMPLES); do
                # Execute the program.  Serial ignores the thread count but
                # still expects the argument.  Capture the elapsed time
                # printed by the program.
                RESULT=$("$PROJECT_ROOT/bin/linkedlist_${IMPL}" "$THREADS" "$N" "$M" "$M_MEMBER" "$M_INSERT" "$M_DELETE")
                # Append to CSV: implementation,threads,time
                echo "${IMPL},${THREADS},${RESULT}" >> "$OUTFILE"
            done
        done
    done
    echo "Generated $OUTFILE"
done