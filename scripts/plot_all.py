#!/usr/bin/env python3
"""
plot_all.py

Create a single chart that overlays results from all three workload
cases.  Each line corresponds to a specific case and implementation
combination (e.g., case1-serial, case2-mutex).  The script reads
summary CSV files from the data/ directory and writes a combined
plot to report/graphs/combined_plot.png.  Error bars denote the
standard deviation of execution time across samples.  This plot
provides a holistic comparison of how each implementation performs
across different thread counts and workloads.
"""

import os
import pandas as pd
import matplotlib.pyplot as plt

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    fig, ax = plt.subplots()
    # Iterate over each case and plot its implementations
    for case in (1, 2, 3):
        summary_path = os.path.join(project_root, 'data', f'case{case}_summary.csv')
        if not os.path.exists(summary_path):
            continue
        df = pd.read_csv(summary_path)
        df['threads'] = df['threads'].astype(int)
        df['average'] = df['average'].astype(float)
        df['stddev'] = df['stddev'].astype(float)
        for impl in df['implementation'].unique():
            sub = df[df['implementation'] == impl]
            label = f'case{case}-{impl}'
            ax.errorbar(sub['threads'], sub['average'], yerr=sub['stddev'], label=label, marker='o')
    ax.set_xlabel('Threads')
    ax.set_ylabel('Time (seconds)')
    ax.set_title('Combined Results Across All Cases')
    ax.legend()
    fig.tight_layout()
    out_path = os.path.join(project_root, 'report', 'graphs', 'combined_plot.png')
    out_dir = os.path.dirname(out_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)
    fig.savefig(out_path)
    plt.close(fig)
    print(f"Generated combined plot at {out_path}")

if __name__ == '__main__':
    main()