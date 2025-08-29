#!/usr/bin/env python3
"""
plot_case2.py

Generate a plot for the second workload case of the concurrent
programming lab.  The script reads summarised timing results from
data/case2_summary.csv and writes a PNG image to
report/graphs/case2_plot.png.  Average execution time is plotted
against the number of threads with error bars representing standard
deviation.  The chart title describes the operation mix for this
case.
"""

import os
from utils import plot_results

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..'))
    summary_csv = os.path.join(project_root, 'data', 'case2_summary.csv')
    output_png = os.path.join(project_root, 'report', 'graphs', 'case2_plot.png')
    title = 'Case 2: 90% Member, 5% Insert, 5% Delete'
    plot_results(summary_csv, output_png, title)
    print(f"Generated plot at {output_png}")

if __name__ == '__main__':
    main()