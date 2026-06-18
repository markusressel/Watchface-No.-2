#!/usr/bin/env python3
import argparse
import os

from utils import run_command


def main():
    """Main function to build and optionally minify the project."""
    parser = argparse.ArgumentParser(description="Build the project and optionally minify JavaScript files.")
    parser.add_argument('--minify', action='store_true', help='Enable JavaScript minification.')
    parser.add_argument('--emu', action='store_true', help='Build for emulator.')
    args = parser.parse_args()

    env = os.environ.copy()
    if "PEBBLE_RELEASE" in env:
        print(f"PEBBLE_RELEASE is set to: {env['PEBBLE_RELEASE']}")

    if args.emu or "PEBBLE_EMULATOR_BUILD" in env:
        env["PEBBLE_EMULATOR_BUILD"] = "1"
    else:
        env.pop("PEBBLE_EMULATOR_BUILD", None)

    # The pebble build command creates the 'build' directory and transpiles the code.
    print("Running pebble build...")
    if run_command("pebble build", env=env, check=False, stream_output=True) != 0:
        print("Pebble build command failed.")
        exit(1)

    if args.minify:
        print("Minifying JS files...")
        pkjs_dir = "build/generated/pkjs"
        if not os.path.isdir(pkjs_dir):
            print(f"Directory not found: {pkjs_dir}. Skipping minification.")
            exit(1)

        for root, _, files in os.walk(pkjs_dir):
            for filename in files:
                if filename.endswith(".js"):
                    filepath = os.path.join(root, filename)
                    print(f"Minifying {filepath}...")
                    # Use npx to ensure terser is found
                    if run_command(f"npx terser {filepath} -o {filepath} -c -m", env=env, check=False, stream_output=True) != 0:
                        print(f"Minification failed for {filepath}")
                        exit(1)
                    else:
                        print(f"Successfully minified {filepath}")

        print("Minification complete.")
    else:
        print("Skipping minification.")

    print("Build process finished.")

if __name__ == "__main__":
    main()
