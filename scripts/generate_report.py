#!/usr/bin/env python3

import os
import platform
import subprocess
import re
import pandas as pd
import numpy as np

def get_system_specs():
    """Gather system specifications. Prefer data/specs.txt; else probe system."""
    import os, platform, subprocess, re, shutil

    # 1) Use data/specs.txt if present
    try:
        with open("data/specs.txt", "r") as f:
            return f.read()
    except FileNotFoundError:
        pass

    # Safe shell helper
    def sh(cmd):
        try:
            return subprocess.check_output(cmd, shell=True, text=True).strip()
        except Exception:
            return ""

    # Small file reader
    def read_first_match(path, pattern=None):
        try:
            with open(path, "r") as f:
                if pattern is None:
                    return f.read().strip()
                for line in f:
                    if re.search(pattern, line):
                        return line.strip()
        except Exception:
            pass
        return ""

    # ---- CPU & topology ----
    cpu_model = ""
    try:
        with open("/proc/cpuinfo") as f:
            for line in f:
                if "model name" in line:
                    cpu_model = line.split(":", 1)[1].strip()
                    break
    except Exception:
        pass

    lscpu = sh("LC_ALL=C lscpu")
    def lscpu_field(key):
        m = re.search(rf"^{re.escape(key)}\s*:\s*(.+)$", lscpu, re.MULTILINE)
        return (m.group(1).strip() if m else "")

    cores_per_socket = lscpu_field("Core(s) per socket")
    sockets          = lscpu_field("Socket(s)")
    threads_per_core = lscpu_field("Thread(s) per core")
    logical_cpus     = sh("nproc")
    arch             = lscpu_field("Architecture")
    vendor           = lscpu_field("Vendor ID")
    cpu_mhz          = lscpu_field("CPU MHz")

    # Physical cores (approx)
    phys_cores = "N/A"
    try:
        if cores_per_socket and sockets:
            phys_cores = str(int(float(cores_per_socket)) * int(float(sockets)))
    except Exception:
        pass

    # Caches
    l1d = lscpu_field("L1d cache")
    l1i = lscpu_field("L1i cache")
    l2  = lscpu_field("L2 cache")
    l3  = lscpu_field("L3 cache")

    # Governor / Turbo (if available)
    governor = read_first_match("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor")
    no_turbo = read_first_match("/sys/devices/system/cpu/intel_pstate/no_turbo")
    turbo_state = {"0": "on", "1": "off"}.get(no_turbo, "")

    # ---- Memory / NUMA / THP ----
    mem_kb = ""
    try:
        with open("/proc/meminfo") as f:
            for line in f:
                if line.startswith("MemTotal:"):
                    mem_kb = line.split()[1]  # kB
                    break
    except Exception:
        pass

    numa_nodes = lscpu_field("NUMA node(s)")
    thp = read_first_match("/sys/kernel/mm/transparent_hugepage/enabled")
    if thp:
        # Normalize THP: show the active item in []
        m = re.search(r"\[(\w+)\]", thp)
        thp = m.group(1) if m else thp

    swap_total = ""
    try:
        with open("/proc/meminfo") as f:
            for line in f:
                if line.startswith("SwapTotal:"):
                    swap_total = line.split()[1]  # kB
                    break
    except Exception:
        pass

    # ---- Virtualization / Steal time ----
    virt = sh("systemd-detect-virt 2>/dev/null") or ""
    steal = ""
    vmstat_line = sh("vmstat 1 3 | tail -1")
    if vmstat_line:
        parts = vmstat_line.split()
        # vmstat cols: ...  id   wa   st (st = steal)
        if len(parts) >= 17:
            steal = parts[-1] + "%"

    # ---- OS / Kernel ----
    os_name = ""
    try:
        with open("/etc/os-release") as f:
            d = {}
            for line in f:
                if "=" in line:
                    k, v = line.strip().split("=", 1)
                    d[k] = v.strip('"')
            os_name = d.get("PRETTY_NAME", "")
    except Exception:
        pass
    kernel = platform.release()

    # ---- Toolchain / Libraries ----
    gcc_ver   = sh("gcc --version | head -n1")
    make_ver  = sh("make --version | head -n1")
    glibc_ver = sh("getconf GNU_LIBC_VERSION")
    nptl_ver  = sh("getconf GNU_LIBPTHREAD_VERSION")
    python_ver = sh("python3 -c 'import sys; print(sys.version.split()[0])'")
    pandas_ver = sh("python3 -c 'import pandas as p; print(p.__version__)'") if shutil.which("python3") else ""
    mpl_ver    = sh("python3 -c 'import matplotlib as m; print(m.__version__)'") if shutil.which("python3") else ""

    # ---- Compose formatted report ----
    lines = []
    lines.append("System Information (Extended)")
    lines.append("CPU")
    lines.append(f"• Model: {cpu_model or 'N/A'}")
    if vendor or arch:   lines.append(f"• Vendor/Arch: {vendor or 'N/A'} / {arch or 'N/A'}")
    if cpu_mhz:          lines.append(f"• Reported MHz: {cpu_mhz}")
    lines.append(f"• Physical cores: {phys_cores}")
    lines.append(f"• Threads per core: {threads_per_core or 'N/A'}")
    if l1i or l1d or l2 or l3:
        lines.append(f"• Caches: L1i {l1i or 'N/A'}, L1d {l1d or 'N/A'}, L2 {l2 or 'N/A'}, L3 {l3 or 'N/A'}")

    lines.append("")
    lines.append("Memory / NUMA")
    lines.append(f"• Total (kB): {mem_kb or 'N/A'}")
    lines.append(f"• NUMA nodes: {numa_nodes or 'N/A'}")
    lines.append(f"• THP: {thp or 'N/A'}")
    lines.append(f"• Swap total (kB): {swap_total or 'N/A'}")

    lines.append("")
    lines.append("CPU Frequency / Power")
    lines.append(f"• Governor: {governor or 'N/A'}")
    if turbo_state: lines.append(f"• Turbo: {turbo_state}")

    lines.append("")
    lines.append("Virtualization")
    lines.append(f"• Platform: {virt or 'bare metal / unknown'}")
    if steal: lines.append(f"• Steal time (vmstat sample): {steal}")

    lines.append("")
    lines.append("Operating System")
    lines.append(f"• Distro: {os_name or 'N/A'}")
    lines.append(f"• Kernel: {kernel or 'N/A'}")
    lines.append(f"• Logical CPUs: {logical_cpus or 'N/A'}")

    lines.append("")
    lines.append("Toolchain")
    lines.append(f"• Compiler: {gcc_ver or 'N/A'}")
    if make_ver:   lines.append(f"• make: {make_ver}")
    if glibc_ver:  lines.append(f"• glibc: {glibc_ver}")
    if nptl_ver:   lines.append(f"• libpthread (NPTL): {nptl_ver}")
    if python_ver: lines.append(f"• Python: {python_ver}")
    if pandas_ver: lines.append(f"• pandas: {pandas_ver}")
    if mpl_ver:    lines.append(f"• matplotlib: {mpl_ver}")

    return "\n".join(lines)

def load_data():
    data = {'summaries': {}, 'raws': {}}
    for i in range(1, 4):
        try:
            data['summaries'][i] = pd.read_csv(f"data/case{i}_summary.csv")
        except FileNotFoundError:
            data['summaries'][i] = pd.DataFrame()
        try:
            data['raws'][i] = pd.read_csv(f"data/case{i}_results.csv")
        except FileNotFoundError:
            data['raws'][i] = pd.DataFrame()
    return data

def precompute_metrics(data):
    metrics = {}
    for i in range(1, 4):
        summary_df = data['summaries'][i]
        raw_df = data['raws'][i]
        if summary_df.empty:
            metrics[i] = None
            continue
        worst_ci = 0.0
        for _, row in summary_df.iterrows():
            k = len(raw_df[(raw_df['implementation'] == row['implementation']) & (raw_df['threads'] == row['threads'])])
            if k > 0:
                mean, stddev = row['average'], row['stddev']
                if not pd.isna(mean) and not pd.isna(stddev) and mean > 0:
                    ci = (1.96 * stddev / np.sqrt(k)) / mean * 100
                    if ci > worst_ci:
                        worst_ci = ci
        t1 = summary_df[summary_df['threads'] == 1]
        t8 = summary_df[summary_df['threads'] == 8]
        def get_val(df, impl, col):
            val = df[df['implementation'] == impl][col]
            return val.iloc[0] if not val.empty else np.nan
        m1_avg = get_val(t1, 'mutex', 'average')
        m8_avg = get_val(t8, 'mutex', 'average')
        r1_avg = get_val(t1, 'rwlock', 'average')
        r8_avg = get_val(t8, 'rwlock', 'average')
        s1_avg = get_val(t1, 'serial', 'average')
        mutex_scaling = ((m8_avg - m1_avg) / m1_avg * 100) if not pd.isna(m1_avg) and m1_avg > 0 else np.nan
        rwlock_scaling = ((r8_avg - r1_avg) / r1_avg * 100) if not pd.isna(r1_avg) and r1_avg > 0 else np.nan
        speedup_t8 = (m8_avg / r8_avg) if not pd.isna(m8_avg) and not pd.isna(r8_avg) and r8_avg > 0 else np.nan
        metrics[i] = {
            'worst_ci': worst_ci, 's1_avg': s1_avg, 'm1_avg': m1_avg, 'r1_avg': r1_avg,
            'mutex_scaling': mutex_scaling, 'rwlock_scaling': rwlock_scaling, 'speedup_t8': speedup_t8
        }
    return metrics

def fmt(val, p=2):
    return f"{val:.{p}f}" if not pd.isna(val) else "---"

def generate_page1(specs):
    return f"""\\documentclass{{article}}
\\usepackage{{graphicx,booktabs,geometry,amsmath}}
\\geometry{{a4paper,margin=1in}}
\\title{{Pthread Linked List Benchmark (Lab 1 -- CS4532)}}
\\author{{<Your names / index numbers>}}
\\date{{\\today}}
\\begin{{document}}
\\maketitle
\\section*{{System Information}}
\\begin{{verbatim}}
{specs}
\\end{{verbatim}}
\\section*{{Approach}}
Linked list with Member, Insert (unique), Delete.
Three variants: serial; pthreads + single mutex; pthreads + single rwlock.
Initialization: n=1000 unique keys in [0, $2^{{16}}-1$].
Workloads: m=10000 ops with given fractions, distributed across $T \\in \\{{1,2,4,8\\}}$ threads.
Timing measures only the m-ops region, not initialization.
\\newpage
"""

def generate_page2(data, metrics):
    content = ["\\section*{Experiment Report (Overview Tables)}"]
    case_titles = [
        "n=1000, m=10000, m\\_member=0.99, m\\_insert=0.005, m\\_delete=0.005",
        "n=1000, m=10000, m\\_member=0.90, m\\_insert=0.05, m\\_delete=0.05",
        "n=1000, m=10000, m\\_member=0.50, m\\_insert=0.25, m\\_delete=0.25"
    ]
    for i in range(1, 4):
        summary_df = data['summaries'][i]
        content.append(f"\\subsection*{{Case {i}: {case_titles[i-1]}}}")
        if summary_df.empty:
            content.append("Data not available for this case.\\\\")
            continue
        table = ["\\begin{table}[h!]", "\\centering", "\\begin{tabular}{cccc}", "\\toprule",
                 "\\textbf{Threads} & \\textbf{Serial (s)} & \\textbf{Mutex (s)} & \\textbf{RW-lock (s)} \\\\", "\\midrule"]
        for t in sorted(summary_df['threads'].unique()):
            row = [str(t)]
            for impl in ['serial', 'mutex', 'rwlock']:
                d = summary_df[(summary_df['implementation'] == impl) & (summary_df['threads'] == t)]
                if not d.empty:
                    avg, std = d['average'].iloc[0], d['stddev'].iloc[0]
                    row.append(f"{fmt(avg, 4)} $\\pm$ {fmt(std, 4)}")
                else:
                    row.append("---")
            table.append(" & ".join(row) + " \\\\")
        table.extend(["\\bottomrule", "\\end{tabular}", f"\\caption{{Summary of results for Case {i}.}}", f"\\label{{tab:case{i}}}", "\\end{table}"])
        content.append("\n".join(table))
    content.append("\\paragraph{Sampling/Confidence}")
    ci_ok = True
    for i in range(1, 4):
        if metrics[i] and metrics[i]['worst_ci'] > 5.0:
            ci_ok = False
            content.append(f"For Case {i}, the worst relative CI was {fmt(metrics[i]['worst_ci'])}%, which exceeds the 5% target.")
    if ci_ok:
        content.append("The target of a 5% relative CI was met for all configurations.")
    else:
        content.append("The target of a 5% relative CI was not met for one or more configurations, suggesting more samples are needed.")
    content.append("\\newpage")
    return "\n".join(content)

def generate_case_analysis_section(case_num, metrics):
    case_titles = ["Read-Heavy Workload", "Balanced Workload", "Write-Heavy Workload"]
    workload_insights = [
        "This workload is read-heavy (99% member operations), which explains the significant performance advantage of the rw-lock, as it allows for concurrent reads.",
        "With a higher write fraction (10%), the advantage of rw-lock diminishes. The data suggests that for this particular workload and system, the overhead of the rw-lock is greater than its benefit from concurrent reads.",
        "In this write-heavy scenario (50% insert/delete), both locking strategies suffer from contention as writes are serialized. The rw-lock's performance is worse than the mutex, indicating that its more complex logic adds significant overhead that is not offset by parallelism in read operations."
    ]
    content = [f"\\subsection*{{Case {case_num}: {case_titles[case_num-1]}}}"]
    content.append(f"\\begin{{figure}}[h!]\n\\centering\n\\includegraphics[width=0.9\\textwidth]{{report/graphs/case{case_num}_plot.png}}\n\\caption{{Average time vs. threads for Case {case_num}.}}\n\\label{{fig:case{case_num}}}\n\\end{{figure}}")
    m = metrics[case_num]
    if not m:
        content.append("Analysis not available due to missing data.")
        return "\n".join(content)
    analysis = [
        "\\paragraph{Analysis}",
        f"As shown in Table~\\ref{{tab:case{case_num}}} and Figure~\\ref{{fig:case{case_num}}}, at 1 thread, serial is fastest ({fmt(m['s1_avg'], 4)}s) vs mutex ({fmt(m['m1_avg'], 4)}s) and rw-lock ({fmt(m['r1_avg'], 4)}s).",
        f"From 1 to 8 threads, mutex changes by {fmt(m['mutex_scaling'])}% and rw-lock by {fmt(m['rwlock_scaling'])}%.",
        f"At 8 threads, rw-lock is {fmt(m['speedup_t8'])}x faster than mutex.",
        workload_insights[case_num-1]
    ]
    content.append("\n".join(analysis))
    return "\n".join(content)

def generate_page3(metrics):
    return "\\section*{Case Analyses with Plots}\n" + generate_case_analysis_section(1, metrics) + "\n\\newpage"

def generate_page4(metrics):
    return generate_case_analysis_section(2, metrics) + "\n\\newpage"

def generate_page5(metrics):
    return generate_case_analysis_section(3, metrics) + "\n\\newpage"

def generate_page6(metrics):
    content = ["\\begin{figure}[h!]\n\\centering\n\\includegraphics[width=0.8\\textwidth]{{report/graphs/combined_plot.png}}\n\\caption{{Combined view across all cases and implementations.}}\n\\label{fig:combined}\n\\end{figure}"]
    ci_ok = all(m and m['worst_ci'] <= 5.0 for m in metrics.values())
    content.append(f"""\\section*{{Conclusion}}
Results align with expectations: the serial baseline dominates at T=1 (no lock overhead).
Read-heavy workloads: rwlock outperforms mutex via concurrent readers.
Write-heavier workloads: rwlock advantage shrinks; both converge due to writer serialization; parallel versions can underperform serial when contention dominates.
Scaling saturates near core count due to contention and scheduling overhead.
The {'$\\pm$5% @ 95% CI target was satisfied overall.' if ci_ok else '$\\pm$5% @ 95% CI target was not satisfied overall; it is advised to increase samples.'}
\\end{{document}}""")
    return "\n".join(content)

def main():
    specs = get_system_specs()
    data = load_data()
    metrics = precompute_metrics(data)
    report_parts = [
        generate_page1(specs),
        generate_page2(data, metrics),
        generate_page3(metrics),
        generate_page4(metrics),
        generate_page5(metrics),
        generate_page6(metrics)
    ]
    final_report = "\n".join(report_parts)
    with open("report/lab1_report.tex", "w") as f:
        f.write(final_report)
    print("Successfully generated detailed report at report/lab1_report.tex")

if __name__ == "__main__":
    main()
