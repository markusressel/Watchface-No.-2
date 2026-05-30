#!/usr/bin/env python3
import sys
import subprocess
from multiprocessing import Pool

PHONE_IP = "192.168.2.159"

def deploy(platform):
    if platform == "phone":
        print(f"📱 Deploying to phone...")
        cmd = ["pebble", "install", "--phone", PHONE_IP]
    else:
        print(f"⌚ Launching emulator for {platform}...")
        cmd = ["pebble", "install", "--emulator", platform]

    subprocess.run(cmd)

if __name__ == "__main__":
    platforms = sys.argv[1:]
    if not platforms:
        print("Error: No platforms specified.")
        sys.exit(1)

    print(f"🚀 Deploying to: {', '.join(platforms)}")

    # Pool automatically runs the deploy function in parallel for each platform
    with Pool(len(platforms)) as p:
        p.map(deploy, platforms)

    print("✅ All platforms deployed!")
