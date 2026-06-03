#!/usr/bin/env python3
import os
import subprocess
import sys
from multiprocessing import Pool
from typing import List

PHONE_IP = "192.168.2.159"


def build_emu():
    emu_env = os.environ.copy()
    emu_env["PEBBLE_EMULATOR_BUILD"] = "1"
    subprocess.run(["pebble", "build"], env=emu_env, check=True)


def build_real():
    real_env = os.environ.copy()
    real_env.pop("PEBBLE_EMULATOR_BUILD", None)
    subprocess.run(["pebble", "build"], env=real_env, check=True)


def deploy_real(ip_addr: str, follow_logs: bool):
    args = ["pebble", "install", "--phone", ip_addr]
    if follow_logs:
        args.append("--logs")
    subprocess.run(args, check=True)


# Lifted to top-level scope so multiprocessing can pickle it safely
def deploy_emu(platform: str, follow_logs: bool):
    print(f"⌚ Launching emulator for {platform}...")
    args = ["pebble", "install", "--emulator", platform]
    if follow_logs:
        args.append("--logs")
    subprocess.run(args, check=True)


def build_and_deploy(platforms: List[str], follow_logs: bool):
    # Split targets since emulator and hardware builds require different CFLAGS
    hardware_targets = [p for p in platforms if p == "phone"]
    emulator_targets = [p for p in platforms if p != "phone"]

    # 1. Handle Emulator Targets
    if emulator_targets:
        print("🛠️  Building project for Emulator (PEBBLE_EMULATOR_BUILD=1)...")
        emu_env = os.environ.copy()
        emu_env["PEBBLE_EMULATOR_BUILD"] = "1"

        # Explicitly compile the emulator-flavored .pbw first
        build_emu()

        # Deploy to the requested emulators in parallel
        with Pool(len(emulator_targets)) as p:
            p.starmap(deploy_emu, [(t, follow_logs) for t in emulator_targets])

    # 2. Handle Physical Hardware Targets
    if hardware_targets:
        print("🛠️  Building project for Hardware (Production)...")

        # Explicitly compile the production-flavored .pbw
        build_real()

        for target in hardware_targets:
            print(f"📱 Deploying to {target}...")
            deploy_real(ip_addr=PHONE_IP, follow_logs=follow_logs)


if __name__ == "__main__":
    platforms = sys.argv[1:]
    if not platforms:
        print("Error: No platforms specified.")
        sys.exit(1)

    # "--logs" flag
    follow_logs = False
    if "--logs" in platforms:
        platforms.remove("--logs")
        if len(platforms) > 1:
            print("The --logs flag can only be used with a single target.")
            sys.exit(1)
        follow_logs = True

    print(f"🚀 Processing targets: {', '.join(platforms)}")
    build_and_deploy(platforms, follow_logs)
    print("✅ All targets processed!")
