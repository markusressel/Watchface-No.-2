#!/usr/bin/env python3
import os
import re
import shutil
import sys

from utils import print_color, run_command


def clean_coverage():
    print("Cleaning old coverage data...")
    extensions = [".gcda", ".gcno", ".gcov"]
    for root, _, files in os.walk("."):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                os.remove(os.path.join(root, file))

    if os.path.exists("build/coverage"):
        shutil.rmtree("build/coverage")
    os.makedirs("build/coverage", exist_ok=True)


def parse_gcov_output(output):
    # Example gcov output snippet:
    # File 'src/c/ui/theme.c'
    # Lines executed:98.95% of 95
    # Creating 'theme.c.gcov'

    results = []
    file_re = re.compile(r"File '(.*?)'")
    lines_re = re.compile(r"Lines executed:(.*?)% of (\d+)")

    current_file = None

    for line in output.splitlines():
        file_match = file_re.search(line)
        if file_match:
            current_file = file_match.group(1)
            continue

        lines_match = lines_re.search(line)
        if lines_match and current_file:
            if current_file.startswith("src/c/"):
                coverage_pct = float(lines_match.group(1))
                total_lines = int(lines_match.group(2))
                executed_lines = int(round(total_lines * coverage_pct / 100))

                results.append({
                    "file": current_file,
                    "coverage": coverage_pct,
                    "executed": executed_lines,
                    "total": total_lines
                })
            current_file = None

    return results


def print_table(results):
    if not results:
        print_color("No coverage data found for src/c/ files.", "yellow")
        return

    # Sort by coverage descending
    results.sort(key=lambda x: x["coverage"], reverse=True)

    header = f"{'File':<40} | {'Executed':>10} | {'Total':>10} | {'Coverage':>10}"
    separator = "-" * len(header)

    print_color("\n--- C Coverage Summary ---", "bold")
    print(header)
    print(separator)

    total_exec = 0
    total_lines = 0

    for res in results:
        total_exec += res["executed"]
        total_lines += res["total"]

        cov_str = f"{res['coverage']:>9.2f}%"
        if res["coverage"] >= 80:
            cov_str = f"\033[92m{cov_str}\033[0m"
        elif res["coverage"] >= 50:
            cov_str = f"\033[93m{cov_str}\033[0m"
        else:
            cov_str = f"\033[91m{cov_str}\033[0m"

        print(f"{res['file']:<40} | {res['executed']:>10} | {res['total']:>10} | {cov_str}")

    print(separator)
    overall_cov = (total_exec / total_lines * 100) if total_lines > 0 else 0
    overall_str = f"{overall_cov:>9.2f}%"
    if overall_cov >= 80:
        overall_str = f"\033[92m{overall_str}\033[0m"
    elif overall_cov >= 50:
        overall_str = f"\033[93m{overall_str}\033[0m"
    else:
        overall_str = f"\033[91m{overall_str}\033[0m"

    print(f"{'OVERALL':<40} | {total_exec:>10} | {total_lines:>10} | {overall_str}\n")


def main():
    clean_coverage()

    print("Running tests with coverage...")
    run_command(["./scripts/tests.py", "--coverage"], check=True, capture_output=False)

    print("\nProcessing coverage data...")
    # Find all gcda files
    gcda_files = []
    for root, _, files in os.walk("tests/build"):
        for file in files:
            if file.endswith(".gcda"):
                gcda_files.append(os.path.join(root, file))

    if not gcda_files:
        print_color("No .gcda files found in tests/build. Did tests run correctly?", "red")
        sys.exit(1)

    # Run gcov on all data files
    gcov_command = ["gcov", "-o", "tests/build/"] + gcda_files
    result = run_command(gcov_command, capture_output=True)

    results = parse_gcov_output(result.stdout)
    print_table(results)

    # Move gcov files to build/coverage
    for f in os.listdir("."):
        if f.endswith(".gcov"):
            shutil.move(f, os.path.join("build/coverage", f))

    # Try gcovr if available
    if shutil.which("gcovr"):
        print("Generating HTML report with gcovr...")
        gcovr_command = [
            "gcovr", "-r", ".",
            "--filter", "src/c",
            "--html", "--html-details",
            "-o", "build/coverage/index.html"
        ]
        run_command(gcovr_command)
        print_color(f"HTML report generated at build/coverage/index.html", "cyan")
    else:
        print_color("Tip: Install gcovr ('pip install gcovr') for a better summary and HTML reports.", "yellow")


if __name__ == "__main__":
    main()
