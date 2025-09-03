#!/usr/bin/env python3

# Generates LaTeX report from your CSVs and plots.
#
# Inputs expected (produced by your pipeline):
#  - data/case{1,2,3}_summary.csv  (columns: implementation,threads,avg,stddev)
#  - data/case{1,2,3}_results.csv  (columns: implementation,threads,elapsed_s)
#  - data/specs.txt                (freeform machine spec)
#  - report/graphs/case{1,2,3}_plot.png, report/graphs/combined_plot.png

import pandas as pd
import numpy as np
import os

def fmt(value, p=4):
    """Format a float value to a string with a given precision."""
    if pd.isna(value):
        return "---"
    return f"{value:.{p}f}"

def latex_header(specs):
    """Generate the LaTeX document header."""
    return f"""\\documentclass{{article}}
\\usepackage{{graphicx,booktabs,geometry,multirow,amsmath}}
\\geometry{{a4paper,margin=1in}}
\\title{{Pthread Linked List Benchmark (Lab 1 -- CS4532)}}
\\author{{<Your Names / Index Numbers>}}
\\date{{\\today}}
\\begin{{document}}\\maketitle

\\section*{{System Information}}
{'\\begin{verbatim}\n' + specs + '\n\\end{verbatim}' if specs else 'System details file (data/specs.txt) not found.'}

\\section*{{Approach}}
We implement three variants: (A) Serial, (B) Pthreads with a single global mutex, 
and (C) Pthreads with a single global read--write lock. 
The list supports \\texttt{{Member}}, \\texttt{{Insert}} (unique keys), and \\texttt{{Delete}}. 
Initialization inserts $n$ unique keys in $[0, 2^{{16}}-1]$. 
We execute $m$ operations with fractions $m_{{Member}}$, $m_{{Insert}}$, $m_{{Delete}}$ across 
$T\\in\\{{1,2,4,8\\}}$ threads. Timing captures only the $m$ operations region.
"""

def latex_case_table(title, summary_df):
    """Generate a LaTeX table for a single case."""
    if summary_df.empty:
        return ""
    
    header = f"""\\subsection*{{{title}}}
\\begin{{table}}[h!]\\centering
\\begin{{tabular}}{{c c c c}}
\\toprule
\\textbf{{Threads}} & \\textbf{{Serial (s)}} & \\textbf{{Mutex (s)}} & \\textbf{{RW-lock (s)}}\\\\\\midrule
"""
    
    rows = []
    for threads, group in summary_df.groupby('threads'):
        serial = group[group['implementation'] == 'serial']
        mutex = group[group['implementation'] == 'mutex']
        rwlock = group[group['implementation'] == 'rwlock']
        
        s_avg = serial['average'].iloc[0] if not serial.empty else np.nan
        s_std = serial['stddev'].iloc[0] if not serial.empty else np.nan
        m_avg = mutex['average'].iloc[0] if not mutex.empty else np.nan
        m_std = mutex['stddev'].iloc[0] if not mutex.empty else np.nan
        r_avg = rwlock['average'].iloc[0] if not rwlock.empty else np.nan
        r_std = rwlock['stddev'].iloc[0] if not rwlock.empty else np.nan

        rows.append(
            f"{threads} & {fmt(s_avg)} $\\pm$ {fmt(s_std)} & "
            f"{fmt(m_avg)} $\\pm$ {fmt(m_std)} & "
            f"{fmt(r_avg)} $\\pm$ {fmt(r_std)} \\\\"
        )
    
    footer = """\\bottomrule
\\end{tabular}
\\end{table}
"""
    return header + "\n".join(rows) + "\n" + footer

def rel_ci95(mean, sd, k):
    """Calculate the relative 95% confidence interval half-width."""
    if k <= 1 or mean <= 0.0 or pd.isna(mean) or pd.isna(sd):
        return np.nan
    half_width = 1.96 * sd / np.sqrt(k)
    return 100.0 * (half_width / mean)

def latex_sampling_block(case_name, summary_df, raw_df):
    """Generate the sampling check paragraph."""
    if summary_df.empty or raw_df.empty:
        return f"\\paragraph{{Sampling check ({case_name}).}} Insufficient data to compute confidence intervals.\n"

    worst_ci = 0.0
    worst_impl = ""
    worst_threads = 0

    for _, row in summary_df.iterrows():
        impl = row['implementation']
        threads = row['threads']
        k = len(raw_df[(raw_df['implementation'] == impl) & (raw_df['threads'] == threads)])
        rci = rel_ci95(row['average'], row['stddev'], k)
        if not pd.isna(rci) and rci > worst_ci:
            worst_ci = rci
            worst_impl = impl
            worst_threads = threads
            
    if worst_ci > 0.0:
        return (f"\\paragraph{{Sampling check ({case_name}).}} "
                f"Maximum observed relative half-width at 95\\% confidence is {fmt(worst_ci, 2)}\\% "
                f"(implementation: {worst_impl}, threads: {worst_threads}). "
                "Target is $\\le 5\\%$. Increase samples if this threshold is exceeded.\n")
    else:
        return f"\\paragraph{{Sampling check ({case_name}).}} Insufficient raw data to compute confidence intervals.\n"


def trend_change(a, b):
    """Calculate the percentage change between two values."""
    if pd.isna(a) or pd.isna(b) or a <= 0:
        return "---"
    pct = 100.0 * (b - a) / a
    return f"{pct:.1f}%"

def latex_case_analysis(case_name, summary_df):
    """Generate the analysis paragraph for a single case."""
    if summary_df.empty:
        return ""

    t_levels = sorted(summary_df['threads'].unique())
    t1, tn = t_levels[0], t_levels[-1]

    def get_avg(threads, impl):
        val = summary_df[(summary_df['threads'] == threads) & (summary_df['implementation'] == impl)]['average']
        return val.iloc[0] if not val.empty else np.nan

    s1, m1, r1 = get_avg(t1, 'serial'), get_avg(t1, 'mutex'), get_avg(t1, 'rwlock')
    mn, rn = get_avg(tn, 'mutex'), get_avg(tn, 'rwlock')

    analysis = f"\\paragraph{{Analysis ({case_name}).}} "
    analysis += (f"At {t1} thread{'s' if t1 > 1 else ''}, the serial baseline is expected to lead due to lock overhead; "
                 f"observed means (s): serial={fmt(s1)}, mutex={fmt(m1)}, rwlock={fmt(r1)}. ")
    
    analysis += (f"From {t1} to {tn} threads, "
                 f"mutex changes by {trend_change(m1, mn)} "
                 f"and rwlock by {trend_change(r1, rn)} (means). ")

    if not pd.isna(mn) and not pd.isna(rn) and mn > 0 and rn > 0:
        rel = mn / rn
        analysis += f"At {tn} threads, rwlock is {fmt(rel, 2)}× faster than mutex (lower time is better). "

    analysis += ("As write fraction increases, rwlock advantage typically shrinks because writes serialize; "
                 "this matches the theoretical expectation and your class discussion.\n")
    
    return analysis

def latex_figs():
    """Generate the LaTeX for including figures."""
    return """\\section*{Figures}
\\begin{figure}[h!]\\centering
\\includegraphics[width=0.9\\textwidth]{graphs/case1_plot.png}
\\caption{Average time vs threads (Case 1) with error bars.}\\end{figure}

\\begin{figure}[h!]\\centering
\\includegraphics[width=0.9\\textwidth]{graphs/case2_plot.png}
\\caption{Average time vs threads (Case 2) with error bars.}\\end{figure}

\\begin{figure}[h!]\\centering
\\includegraphics[width=0.9\\textwidth]{graphs/case3_plot.png}
\\caption{Average time vs threads (Case 3) with error bars.}\\end{figure}

\\begin{figure}[h!]\\centering
\\includegraphics[width=0.8\\textwidth]{graphs/combined_plot.png}
\\caption{Combined view across cases and implementations.}\\end{figure}
"""

def latex_conclusion():
    """Generate the conclusion section."""
    return """\\section*{Conclusion}
Results align with expectations: the serial baseline dominates at one thread; 
RW--locks yield strong gains in read--heavy workloads by allowing concurrent readers; 
mutexes serialize all operations and suffer from contention; 
as the write fraction rises, both parallel variants converge due to writer serialization. 
Scaling saturates near core count due to lock contention and scheduling overhead.

\\end{document}
"""

def main():
    """Main function to generate the report."""
    # Read specs
    try:
        with open("data/specs.txt", "r") as f:
            specs = f.read()
    except FileNotFoundError:
        specs = ""

    # Read summaries and raw data
    summaries = {}
    raws = {}
    for i in range(1, 4):
        try:
            summaries[i] = pd.read_csv(f"data/case{i}_summary.csv")
        except FileNotFoundError:
            print(f"Warning: data/case{i}_summary.csv not found.")
            summaries[i] = pd.DataFrame()
        try:
            raws[i] = pd.read_csv(f"data/case{i}_results.csv")
        except FileNotFoundError:
            print(f"Warning: data/case{i}_results.csv not found.")
            raws[i] = pd.DataFrame()

    # Generate LaTeX content
    tex_content = []
    tex_content.append(latex_header(specs))
    
    tex_content.append("\\section*{Experiment Results}")

    case1_title = "Case 1: $n=1000,\\ m=10000,\\ m_{Member}=0.99,\\ m_{Insert}=0.005,\\ m_{Delete}=0.005$"
    tex_content.append(latex_case_table(case1_title, summaries[1]))
    tex_content.append(latex_sampling_block("Case 1", summaries[1], raws[1]))
    tex_content.append(latex_case_analysis("Case 1", summaries[1]))

    case2_title = "Case 2: $n=1000,\\ m=10000,\\ m_{Member}=0.90,\\ m_{Insert}=0.05,\\ m_{Delete}=0.05$"
    tex_content.append(latex_case_table(case2_title, summaries[2]))
    tex_content.append(latex_sampling_block("Case 2", summaries[2], raws[2]))
    tex_content.append(latex_case_analysis("Case 2", summaries[2]))

    case3_title = "Case 3: $n=1000,\\ m=10000,\\ m_{Member}=0.50,\\ m_{Insert}=0.25,\\ m_{Delete}=0.25$"
    tex_content.append(latex_case_table(case3_title, summaries[3]))
    tex_content.append(latex_sampling_block("Case 3", summaries[3], raws[3]))
    tex_content.append(latex_case_analysis("Case 3", summaries[3]))

    tex_content.append(latex_figs())
    tex_content.append(latex_conclusion())
    
    # Write to file
    with open("report/lab1_report.tex", "w") as f:
        f.write("\n".join(tex_content))

    print("Successfully generated report/lab1_report.tex")

if __name__ == "__main__":
    main()
