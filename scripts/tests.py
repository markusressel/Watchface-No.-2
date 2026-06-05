#!/usr/bin/env python3
import os
import subprocess
import sys


def run_c_host_tests():
    print("Running C host tests...")
    build_dir = "tests/build"
    os.makedirs(build_dir, exist_ok=True)

    test_files = []
    for root, _, files in os.walk("tests/c"):
        for file in files:
            if file.endswith("_test.c"):
                test_files.append(os.path.join(root, file))

    all_tests_passed = True

    for test_file in test_files:
        filename = os.path.basename(test_file)
        test_name = filename.replace("_test.c", "")

        # Assuming the production code file is in src/c/ and has the same base name
        # This might need to be more sophisticated if test_name doesn't directly map to a production .c file
        production_c_file = os.path.join("src/c", f"{test_name}.c")
        # For now, we only have util.c, so we'll hardcode it for this example
        # In a real scenario, you might need a mapping or convention
        if test_name == "util":
            production_c_file = "src/c/util.c"
        else:
            # If no corresponding production .c file, just compile the test file itself
            production_c_file = None


        output_executable = os.path.join(build_dir, f"{test_name}_test")

        print(f"--- Compiling and running {test_name} tests ---")

        compile_command = ["gcc", "-Itests/c", test_file]
        if production_c_file and os.path.exists(production_c_file):
            compile_command.append(production_c_file)
        compile_command.extend(["-o", output_executable])

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
        try:
            result = subprocess.run([output_executable], check=True, capture_output=True, text=True)
            print(result.stdout)
            print(f"{test_name} tests: PASSED")
        except subprocess.CalledProcessError as e:
            print(e.stdout)
            print(e.stderr)
            print(f"{test_name} tests: FAILED")
            all_tests_passed = False
        print("")  # Newline for readability

    if all_tests_passed:
        print("All C host tests PASSED")
        return 0
    else:
        print("Some C host tests FAILED")
        return 1


if __name__ == "__main__":
    sys.exit(run_c_host_tests())
