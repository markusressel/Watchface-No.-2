#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys
from multiprocessing import Pool
from typing import List

PHONE_IP = "192.168.2.159"
VALID_PLATFORMS = ["phone", "basalt", "chalk", "diorite", "emery", "flint"]


def build(emulator: bool, release: bool):
    target = "Emulator" if emulator else "Hardware"
    mode = "Debug" if debug_build else "Release"
    print(f"🛠️  Building project, target: {target} mode: {mode}...")

    env = os.environ.copy()
    if emulator:
        env["PEBBLE_EMULATOR_BUILD"] = "1"
    else:
        env.pop("PEBBLE_EMULATOR_BUILD", None)

    if release:
        env["PEBBLE_RELEASE"] = "1"
    else:
        env.pop("PEBBLE_RELEASE", None)

    subprocess.run(["./scripts/build.py", "--minify"], env=env, check=True)


def build_emu(debug_build: bool):
    # Explicitly compile the emulator-flavored .pbw
    build(emulator=True, release=not debug_build)


def build_real(debug_build: bool):
    # Explicitly compile the production-flavored .pbw
    build(emulator=False, release=not debug_build)


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


def build_and_deploy(platforms: List[str], follow_logs: bool, debug_build: bool):
    # Split targets since emulator and hardware builds require different CFLAGS
    hardware_targets = [p for p in platforms if p == "phone"]
    emulator_targets = [p for p in platforms if p != "phone"]

    # 1. Handle Emulator Targets
    if emulator_targets:
        # Explicitly compile the emulator-flavored .pbw first
        build_emu(debug_build)

        # Deploy to the requested emulators in parallel
        with Pool(len(emulator_targets)) as p:
            p.starmap(deploy_emu, [(t, follow_logs) for t in emulator_targets])

    # 2. Handle Physical Hardware Targets
    if hardware_targets:
        # Explicitly compile the production-flavored .pbw
        build_real(debug_build)

        for target in hardware_targets:
            print(f"📱 Deploying to {target}...")
            deploy_real(ip_addr=PHONE_IP, follow_logs=follow_logs)


if __name__ == "__main__":
    try:
        parser = argparse.ArgumentParser(description="Build and deploy Pebble app to emulators and/or phone")
        parser.add_argument("platforms", nargs="+", choices=VALID_PLATFORMS)
        parser.add_argument("--logs", action="store_true", help="Follow logs after install (single target only)")

        mode = parser.add_mutually_exclusive_group()
        mode.add_argument("--debug", action="store_true", help="Build debug variant")
        mode.add_argument("--release", action="store_true", help="Build release variant (default)")

        args = parser.parse_args()

        if args.logs and len(args.platforms) > 1:
            print("The --logs flag can only be used with a single target.")
            sys.exit(1)

        debug_build = args.debug

        print(f"🚀 Processing targets: {', '.join(args.platforms)}")
        print(f"🔧 Build mode: {'Debug' if debug_build else 'Release'}")
        build_and_deploy(args.platforms, args.logs, debug_build)
        print("✅ All targets processed!")
    except KeyboardInterrupt:
        pass
