#!/usr/bin/env python3
"""
plot_case1.py

Generate a plot for the first workload case of the concurrent
programming lab.  The script reads the summarised timing results from
data/case1_summary.csv and produces a PNG image under
report/graphs/case1_plot.png.  The resulting chart shows average
execution time versus thread count with error bars for standard
deviation.  See utils.plot_results() for implementation details.
"""

import os
from utils import plot_results

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    summary_csv = os.path.join(project_root, 'data', 'case1_summary.csv')
    output_png = os.path.join(project_root, 'report', 'graphs', 'case1_plot.png')
    title = 'Case 1: 99% Member, 0.5% Insert, 0.5% Delete'
    plot_results(summary_csv, output_png, title)
    print(f"Generated plot at {output_png}")

if __name__ == '__main__':
    main()