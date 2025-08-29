#!/usr/bin/env python3
"""
plot_case3.py

Generate a plot for the third workload case of the concurrent
programming lab.  Reads summarised timing results from
data/case3_summary.csv and writes a PNG image to
report/graphs/case3_plot.png.  The chart plots average execution
time versus thread count with error bars showing standard deviation.
The title reflects the balanced mix of operations for this case.
"""

import os
from utils import plot_results

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    summary_csv = os.path.join(project_root, 'data', 'case3_summary.csv')
    output_png = os.path.join(project_root, 'report', 'graphs', 'case3_plot.png')
    title = 'Case 3: 50% Member, 25% Insert, 25% Delete'
    plot_results(summary_csv, output_png, title)
    print(f"Generated plot at {output_png}")

if __name__ == '__main__':
    main()