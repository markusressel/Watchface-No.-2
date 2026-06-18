#!/usr/bin/env python3
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

from utils import print_color


def run_single_test(test_file, build_dir, coverage=False):
    filename = os.path.basename(test_file)
    test_name = filename.replace("_test.c", "")
    output_executable = os.path.join(build_dir, f"{test_name}_test")

    # Compile the test file, the Unity runner, and include paths for mocks and Unity headers
    compile_command = [
        "gcc",
        "-Itests/c/pebble",  # For mock pebble.h
        "-Itests/c/unity",  # For official unity.h and unity_internals.h
        "tests/c/unity/unity.c",  # The Unity framework implementation
        test_file,  # Your test file (which includes the production .c file)
        "-o", output_executable
    ]

    if coverage:
        compile_command.append("--coverage")

    result = {
        "test_name": test_name,
        "compile_success": False,
        "execution_success": False,
        "stdout": "",
        "stderr": "",
        "error_msg": ""
    }

    # Compile test
    try:
        subprocess.run(compile_command, check=True, capture_output=True, text=True)
        result["compile_success"] = True
    except subprocess.CalledProcessError as e:
        result["stdout"] = e.stdout
        result["stderr"] = e.stderr
        return result

    # Run test
    try:
        proc_result = subprocess.run([output_executable], check=True, capture_output=True, text=True)
        result["stdout"] = proc_result.stdout
        result["execution_success"] = True
    except subprocess.CalledProcessError as e:
        result["stdout"] = e.stdout
        result["stderr"] = e.stderr

    return result


def run_c_host_tests(coverage=False):
    print("Running C host tests (in parallel)...")
    build_dir = "tests/build"
    os.makedirs(build_dir, exist_ok=True)

    test_files = []
    for root, _, files in os.walk("tests/c"):
        for file in files:
            if file.endswith("_test.c"):
                test_files.append(os.path.join(root, file))

    # Sort test files to ensure deterministic output order
    test_files.sort()

    total_passed = 0
    total_failed = 0
    total_ignored = 0
    total_errors = 0

    unity_re = re.compile(r'(\d+) Tests (\d+) Failures (\d+) Ignored')

    # Run tests in parallel
    with ThreadPoolExecutor() as executor:
        # map ensures results are returned in the same order as test_files
        results = list(executor.map(lambda f: run_single_test(f, build_dir, coverage), test_files))

    # Process results in order
    for result in results:
        print(f"--- {result['test_name']} ---")

        if not result["compile_success"]:
            print_color(f"  Compilation FAILED", "red")
            print(result["stdout"])
            print(result["stderr"])
            total_errors += 1
            continue

        print(result["stdout"].strip())

        if not result["execution_success"]:
            if result["stderr"]:
                print(result["stderr"].strip())
            print_color(f"  Execution FAILED", "red")

        match = unity_re.search(result["stdout"])
        if match:
            num_tests = int(match.group(1))
            num_fail = int(match.group(2))
            num_ignored = int(match.group(3))

            total_failed += num_fail
            total_ignored += num_ignored
            total_passed += (num_tests - num_fail - num_ignored)
        else:
            # If no Unity summary found but it failed/crashed
            total_errors += 1

    print_color("\n--- C Test Summary ---", "bold")
    total_tests = total_passed + total_failed + total_ignored

    print("-----------------------")
    print(f"{total_tests} Tests {total_failed} Failures {total_ignored} Ignored")

    if total_failed == 0 and total_errors == 0 and total_passed > 0:
        print_color("OK", "green")
        return 0
    else:
        if total_errors > 0:
            print_color(f"FAIL ({total_errors} crashes or compilation failures)", "red")
        else:
            print_color("FAIL", "red")
        return 1


if __name__ == "__main__":
    coverage_requested = "--coverage" in sys.argv
    sys.exit(run_c_host_tests(coverage=coverage_requested))
