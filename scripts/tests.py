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

    all_tests_passed = True
    total_tests = 0
    total_failures = 0
    total_ignored = 0

    unity_re = re.compile(r'(\d+) Tests (\d+) Failures (\d+) Ignored')

    for test_file in test_files:
        filename = os.path.basename(test_file)
        test_name = filename.replace("_test.c", "")

        output_executable = os.path.join(build_dir, f"{test_name}_test")

        print(f"--- Compiling and running {test_name} tests ---")

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
            print(f"Compilation successful for {test_name}.")
        except subprocess.CalledProcessError as e:
            print(f"Compilation FAILED for {test_name}:")
            print(e.stdout)
            print(e.stderr)
            all_tests_passed = False
            print("")
            continue

        # Run test
        stdout = ""
        try:
            result = subprocess.run([output_executable], check=True, capture_output=True, text=True)
            stdout = result.stdout
            print(stdout)
        except subprocess.CalledProcessError as e:
            stdout = e.stdout
            print(stdout)
            print(e.stderr)
            print(f"{test_name} tests: FAILED (Execution Error)")
            all_tests_passed = False

        match = unity_re.search(stdout)
        if match:
            total_tests += int(match.group(1))
            total_failures += int(match.group(2))
            total_ignored += int(match.group(3))

        print("")  # Newline for readability

    if all_tests_passed:
        if total_tests > 0:
            print_color(f"All {total_tests} C host tests PASSED", "green")
        else:
            print_color("No C host tests were found or run", "red")
        return 0
    else:
        print_color(f"Some C host tests FAILED ({total_failures} failed out of {total_tests})", "red")
        return 1


if __name__ == "__main__":
    coverage_requested = "--coverage" in sys.argv
    sys.exit(run_c_host_tests(coverage=coverage_requested))
