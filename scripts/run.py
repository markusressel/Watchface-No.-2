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


def deploy_real(ip_addr: str):
    subprocess.run(["pebble", "install", "--phone", ip_addr])


# Lifted to top-level scope so multiprocessing can pickle it safely
def deploy_emu(platform: str):
    print(f"⌚ Launching emulator for {platform}...")
    subprocess.run(["pebble", "install", "--emulator", platform])


def build_and_deploy(platforms: List[str]):
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
            p.map(deploy_emu, emulator_targets)

    # 2. Handle Physical Hardware Targets
    if hardware_targets:
        print("🛠️  Building project for Hardware (Production)...")

        # Explicitly compile the production-flavored .pbw
        build_real()

        for target in hardware_targets:
            print(f"📱 Deploying to {target}...")
            deploy_real(ip_addr=PHONE_IP)


if __name__ == "__main__":
    platforms = sys.argv[1:]
    if not platforms:
        print("Error: No platforms specified.")
        sys.exit(1)

    print(f"🚀 Processing targets: {', '.join(platforms)}")
    build_and_deploy(platforms)
    print("✅ All targets processed!")
