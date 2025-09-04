"""
utils.py

Utility functions for loading summary data and generating plots for
the linked list experiments.  These helpers are used by the
plot_case*.py scripts to avoid duplicating common logic.
"""

import os
import pandas as pd
import matplotlib.pyplot as plt

def plot_results(summary_csv: str, output_png: str, case_title: str) -> None:
    """
    Generate a line chart with error bars from a summary CSV file.

    Parameters:
        summary_csv: Path to the CSV file containing columns
            implementation, threads, average, stddev.
        output_png: Path where the generated PNG should be saved.
        case_title: Title to display at the top of the plot.

    The function groups data by implementation and plots average
    execution time against the number of threads with error bars
    representing the standard deviation.  The x‑axis is threads and
    the y‑axis is time in seconds.  No specific colours are set; the
    default matplotlib cycle is used.  A legend distinguishes
    implementations.
    """
    # Read the data
    df = pd.read_csv(summary_csv)
    # Ensure numeric types and convert to microseconds
    df['threads'] = df['threads'].astype(int)
    df['average'] = df['average'].astype(float) * 1_000_000
    df['stddev'] = df['stddev'].astype(float) * 1_000_000
    # Create the figure and axis
    fig, ax = plt.subplots()
    for impl in df['implementation'].unique():
        sub = df[df['implementation'] == impl]
        x = sub['threads']
        y = sub['average']
        yerr = sub['stddev']
        ax.errorbar(x, y, yerr=yerr, label=impl, marker='o')
    ax.set_xlabel('Threads')
    ax.set_ylabel('Time (microseconds)')
    ax.set_title(case_title)
    ax.legend()
    fig.tight_layout()
    # Ensure output directory exists
    out_dir = os.path.dirname(output_png)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)
    fig.savefig(output_png)
    plt.close(fig)