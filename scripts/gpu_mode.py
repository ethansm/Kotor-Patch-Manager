#!/usr/bin/env python3
"""gpu_mode.py - Hardware profile manager for RTX 5070 12GB.

Manages VRAM allocation across active AI modes:
  1. kev:     Kev-0.8B System-1 decision router daemon on port 8101 (~1.54 GB VRAM)
  2. lms:     LM Studio local generative LLM (qwen3.5-9b, ~6.7 GB VRAM)
  3. game:    AI models unloaded (>10.2 GB free VRAM for KOTOR II / games)

Usage:
  gpu-mode status
  gpu-mode kev
  gpu-mode clef                (alias of kev)
  gpu-mode lms [model_identifier]
  gpu-mode game [--all]
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import time
import urllib.request

KEV_SERVICE = "kev-08b.service"
KEV_HEALTH_URL = os.environ.get("KEV_HEALTH_URL", "http://127.0.0.1:8101/docs")
DEFAULT_LMS_MODEL = "qwen/qwen3.5-9b"


def run_cmd(cmd: list[str], timeout: float = 10.0) -> tuple[int, str]:
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=timeout,
        )
        return proc.returncode, proc.stdout.strip()
    except Exception as e:
        return 1, str(e)


def get_gpu_memory_mb() -> tuple[int, int]:
    rc, out = run_cmd(["nvidia-smi", "--query-gpu=memory.used,memory.total", "--format=csv,noheader,nounits"])
    if rc == 0 and out:
        parts = [p.strip() for p in out.split(",")]
        if len(parts) >= 2:
            try:
                return int(parts[0]), int(parts[1])
            except ValueError:
                pass
    return 0, 0


def is_kev_active() -> bool:
    rc, _ = run_cmd(["systemctl", "--user", "is-active", KEV_SERVICE])
    return rc == 0


def get_lms_loaded_models() -> list[str]:
    lms_bin = shutil.which("lms") or os.path.expanduser("~/.lmstudio/bin/lms")
    if not os.path.isfile(lms_bin):
        return []
    rc, out = run_cmd([lms_bin, "ps"])
    if rc != 0 or not out or "No models are currently loaded" in out:
        return []
    models = []
    for line in out.splitlines():
        line = line.strip()
        if line and not line.startswith("Loaded Models") and not line.startswith("IDENTIFIER"):
            parts = line.split()
            if parts:
                models.append(parts[0])
    return models


def detect_mode() -> str:
    kev_on = is_kev_active()
    lms_models = get_lms_loaded_models()

    if kev_on and not lms_models:
        return "kev (always-on router)"
    if lms_models and not kev_on:
        return "lms (generative coding)"
    if lms_models and kev_on:
        return "hybrid (kev + lms)"
    return "gaming (clean vram)"


def cmd_status() -> int:
    mode = detect_mode()
    used_mb, total_mb = get_gpu_memory_mb()
    free_mb = max(0, total_mb - used_mb)
    lms_models = get_lms_loaded_models()

    print(f"=== GPU Mode: {mode.upper()} ===")
    print(f"VRAM: {used_mb} MiB used / {total_mb} MiB total ({free_mb} MiB free)")
    print(f"kev-0.8b service:   {'ACTIVE (port 8101)' if is_kev_active() else 'INACTIVE'}")
    print(f"LM Studio models:   {', '.join(lms_models) if lms_models else 'None (unloaded)'}")
    return 0


def cmd_kev() -> int:
    print("[gpu-mode] Ensuring Kev-0.8B System-1 Router is active...")
    rc, out = run_cmd(["systemctl", "--user", "start", KEV_SERVICE])
    if rc != 0:
        print(f"[gpu-mode] Error starting service: {out}", file=sys.stderr)
        return 1

    # Wait up to 5s for health endpoint
    for _ in range(25):
        try:
            req = urllib.request.Request(KEV_HEALTH_URL, headers={"User-Agent": "gpu-mode/1.0"})
            with urllib.request.urlopen(req, timeout=0.2) as resp:
                if resp.status == 200:
                    print("[gpu-mode] OK: Kev-0.8B daemon online and ready on port 8101.")
                    cmd_status()
                    return 0
        except Exception:
            time.sleep(0.2)

    print("[gpu-mode] Warning: Service started but port 8101 not yet responding.", file=sys.stderr)
    cmd_status()
    return 1


def cmd_lms(model: str | None = None) -> int:
    target_model = model or DEFAULT_LMS_MODEL
    print(f"[gpu-mode] Switching profile to LMS (Local LLM: {target_model})...")

    lms_bin = shutil.which("lms") or os.path.expanduser("~/.lmstudio/bin/lms")
    if not os.path.isfile(lms_bin):
        print(f"[gpu-mode] Error: lms binary not found at {lms_bin}", file=sys.stderr)
        return 1

    print(f"[gpu-mode] Loading {target_model} via LM Studio...")
    rc, out = run_cmd([lms_bin, "load", target_model], timeout=30.0)
    if rc != 0:
        print(f"[gpu-mode] Warning during lms load: {out}", file=sys.stderr)
        cmd_status()
        return 1

    cmd_status()
    return 0


def cmd_game(stop_all: bool = False) -> int:
    print("[gpu-mode] Preparing GPU for gaming...")

    lms_bin = shutil.which("lms") or os.path.expanduser("~/.lmstudio/bin/lms")
    unload_ok = True
    if os.path.isfile(lms_bin):
        print("[gpu-mode] Unloading LM Studio models...")
        unload_rc, unload_out = run_cmd([lms_bin, "unload", "-a"])
        if unload_rc != 0:
            print(f"[gpu-mode] Warning during lms unload: {unload_out}", file=sys.stderr)
            unload_ok = False

    if stop_all:
        print(f"[gpu-mode] Stopping {KEV_SERVICE} for 100% clean VRAM...")
        run_cmd(["systemctl", "--user", "stop", KEV_SERVICE])
    else:
        print("[gpu-mode] Note: kev-08b left active (1.54 GB VRAM fits alongside KOTOR II).")

    time.sleep(0.5)
    used_mb, total_mb = get_gpu_memory_mb()
    free_mb = max(0, total_mb - used_mb)
    if not unload_ok or total_mb == 0:
        print("[gpu-mode] Error: VRAM not confirmed ready (unload failed or nvidia-smi unavailable).", file=sys.stderr)
        return 1
    print(f"[gpu-mode] OK: VRAM ready. {free_mb} MiB VRAM free for KOTOR II / games.")
    return 0


def main():
    parser = argparse.ArgumentParser(description="RTX 5070 GPU Profile Manager")
    parser.add_argument("mode", choices=["status", "kev", "clef", "lms", "game"], nargs="?", default="status")
    parser.add_argument("--model", type=str, default=None, help="Model identifier when switching to lms")
    parser.add_argument("--all", action="store_true", help="Stop all AI services including kev-08b when switching to game")

    args = parser.parse_args()
    if args.mode == "status":
        sys.exit(cmd_status())
    elif args.mode in ("kev", "clef"):
        sys.exit(cmd_kev())
    elif args.mode == "lms":
        sys.exit(cmd_lms(args.model))
    elif args.mode == "game":
        sys.exit(cmd_game(stop_all=args.all))


if __name__ == "__main__":
    main()
