#!/usr/bin/env python3
import shutil
import subprocess
import sys


def main():
    # The Pebble toolchain will pass the URL as the first argument
    if len(sys.argv) < 2:
        print("Error: No URL provided by the Pebble toolchain.")
        sys.exit(1)

    url = sys.argv[1]
    profile_dir = "/tmp/pebble-clay-profile"

    # 1. Kill any existing config window.
    # This prevents window clutter and destroys the useless window pointing to the dead port.
    subprocess.run(["pkill", "-f", profile_dir], capture_output=True)

    # 2. Locate the browser binary (Arch usually uses 'chromium')
    browser_bin = shutil.which("google-chrome-beta") or shutil.which("google-chrome-stable")
    if not browser_bin:
        print("Error: Chromium not found in PATH.")
        sys.exit(1)

    # 3. Assemble the isolated, insecure browser command
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


if __name__ == "__main__":
    main()
