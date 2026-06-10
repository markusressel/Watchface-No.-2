#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys


def kill_existing_window(profile_dir: str):
    """1. Kill any existing config window."""
    # This prevents window clutter and destroys the useless window pointing to the dead port.
    subprocess.run(["pkill", "-f", profile_dir], capture_output=True)


def locate_browser() -> str:
    """2. Locate the browser binary."""
    browser_bin = shutil.which("google-chrome-beta") or shutil.which("google-chrome-stable")
    if not browser_bin:
        print("Error: Chromium not found in PATH.")
        sys.exit(1)
    return browser_bin


def launch_browser(browser_bin: str, profile_dir: str, url: str):
    """3. Assemble the isolated, insecure browser command and launch it."""
    cmd = [
        browser_bin,
        f"--user-data-dir={profile_dir}",  # Reusable profile to skip first-run dialogs
        "--disable-web-security",  # Drop CORS
        "--disable-features=BlockInsecurePrivateNetworkRequests",  # Drop PNA
        "--no-first-run",  # Skip welcome screens
        "--no-default-browser-check",  # Skip default browser prompts
        "--disable-sync",  # Don't ask to sign in
        f"--app={url}"  # Open as a clean, chromeless window
    ]

    # 4. Launch the browser detached from the python script's lifecycle
    subprocess.Popen(cmd, start_new_session=True)


def launch_pebble_command(emulator: str):
    """Launch the original pebble emu-app-config command."""
    env = os.environ.copy()
    env["BROWSER"] = os.path.abspath(__file__)
    subprocess.run(["pebble", "emu-app-config", "--emulator", emulator], env=env)


def main():
    parser = argparse.ArgumentParser(description="Pebble App Config Browser Wrapper")
    parser.add_argument("url", nargs="?", help="The URL to open (passed by pebble toolchain)")
    parser.add_argument("--emulator", help="Launch pebble emu-app-config with the specified emulator")

    args = parser.parse_args()

    if args.emulator:
        launch_pebble_command(args.emulator)
        return

    if not args.url:
        print("Error: No URL or emulator provided.")
        parser.print_help()
        sys.exit(1)

    profile_dir = "/tmp/pebble-clay-profile"

    kill_existing_window(profile_dir)
    browser_bin = locate_browser()
    launch_browser(browser_bin, profile_dir, args.url)


if __name__ == "__main__":
    main()
