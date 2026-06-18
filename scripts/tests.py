#!/usr/bin/env python3
import os
import re
import subprocess
import sys

from utils import print_color


def run_c_host_tests(coverage=False):
    print("Running C host tests...")
    build_dir = "tests/build"
    os.makedirs(build_dir, exist_ok=True)

    test_files = []
    for root, _, files in os.walk("tests/c"):
        for file in files:
            if file.endswith("_test.c"):
                test_files.append(os.path.join(root, file))

    total_passed = 0
    total_failed = 0
    total_ignored = 0
    total_errors = 0

    unity_re = re.compile(r'(\d+) Tests (\d+) Failures (\d+) Ignored')

    for test_file in test_files:
        filename = os.path.basename(test_file)
        test_name = filename.replace("_test.c", "")

        output_executable = os.path.join(build_dir, f"{test_name}_test")

        print(f"--- {test_name} ---")

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

        # Compile test
        try:
            subprocess.run(compile_command, check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print_color(f"  Compilation FAILED", "red")
            print(e.stdout)
            print(e.stderr)
            total_errors += 1
            continue

        # Run test
        stdout = ""
        try:
            result = subprocess.run([output_executable], check=True, capture_output=True, text=True)
            stdout = result.stdout
            print(stdout.strip())
        except subprocess.CalledProcessError as e:
            stdout = e.stdout
            print(stdout.strip())
            if e.stderr:
                print(e.stderr.strip())
            print_color(f"  Execution FAILED", "red")

        match = unity_re.search(stdout)
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
