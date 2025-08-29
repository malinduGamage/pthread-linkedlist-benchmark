"""
collect_results.py

Parse raw timing data produced by run_tests.sh and compute average
execution time and standard deviation for each workload, implementation
and thread count.  The script reads files named caseX_results.csv
in the data/ directory and writes corresponding summary files
named caseX_summary.csv.  Each summary CSV contains columns:
implementation, threads, average, stddev.

Usage:
    python3 collect_results.py

This script expects to be executed from within the scripts/ directory
or with the project root on the PYTHONPATH.  It uses only the
standard library.
"""

import csv
import os
import statistics

def process_case(case_number: int, data_dir: str) -> None:
    input_path = os.path.join(data_dir, f"case{case_number}_results.csv")
    output_path = os.path.join(data_dir, f"case{case_number}_summary.csv")
    if not os.path.exists(input_path):
        print(f"Warning: {input_path} does not exist. Skipping.")
        return
    # Map (implementation, threads) -> list of times
    results = {}
    with open(input_path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            impl = row['implementation']
            threads = int(row['threads'])
            time_val = float(row['time'])
            results.setdefault((impl, threads), []).append(time_val)
    # Write summary
    with open(output_path, 'w', newline='') as out:
        writer = csv.writer(out)
        writer.writerow(['implementation', 'threads', 'average', 'stddev'])
        for (impl, threads), times in sorted(results.items(), key=lambda x: (x[0][0], x[0][1])):
            avg = sum(times) / len(times)
            stddev = statistics.stdev(times) if len(times) > 1 else 0.0
            writer.writerow([impl, threads, f"{avg:.6f}", f"{stddev:.6f}"])
    print(f"Wrote summary to {output_path}")

def main():
    # Determine the project root based on this script's location
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    data_dir = os.path.join(project_root, 'data')
    for case_number in (1, 2, 3):
        process_case(case_number, data_dir)

if __name__ == '__main__':
    main()