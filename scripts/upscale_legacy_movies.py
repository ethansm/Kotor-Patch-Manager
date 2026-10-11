#!/usr/bin/env python3
"""
Upscale Legacy 2003 Movies (Echoes of the Republic) to 3440x1440 Bink 1.

Processes all 14 legacy .bik files:
  - 8 Taris / swoop race cutscenes (05_3a..05_8c)
  - 4 Turret minigame cutscenes (22a, 22b, 23a, 23b)
  - 2 Manaan / Sleheyron swoop cutscenes (LIVE_1a, Live_1c)

Pipeline per movie:
  1. Check if already upscaled (skips if already 3440x1440)
  2. Extract PNG frames & 16-bit PCM WAV audio via FFmpeg
  3. AI Upscale 4x (2560x1088) with realesrgan-ncnn-vulkan (realesr-animevideov3) on RTX 5070
  4. Resample & pad to 3440x1440 via FFmpeg Lanczos (3388x1440 with 26px padding to maintain 2.35:1)
  5. Encode Bink 1 (BIKi) video via RAD Video Tools (radvideo64.exe binkc) under an isolated Proton prefix
  6. Mux original audio via RAD Video Tools (radvideo64.exe binkmix) under an isolated Proton prefix
  7. Verify output integrity & deploy to staging and game Movies/
  8. Purge intermediate frames per cutscene to maintain minimal disk footprint (<1 GB overhead)
"""

import os
import sys
import glob
import shutil
import subprocess
import time
from pathlib import Path

# Tool Paths
FFMPEG_BIN = Path("/home/ethansm/.local/bin/ffmpeg")
REALESRGAN_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/realesrgan")
REALESRGAN_BIN = REALESRGAN_DIR / "realesrgan-ncnn-vulkan"
RADVIDEO_BIN = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/radvideo/radvideo64.exe")

PROTON_BIN = Path("/home/ethansm/.local/share/Steam/steamapps/common/Proton - Experimental/proton")
STEAM_CLIENT_PATH = "/home/ethansm/.local/share/Steam"

# Isolated Wine/Proton Prefix specifically for RAD Video Tools (prevents collisions with running games)
RADVIDEO_PFX = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/radvideo/pfx")

SOURCE_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/staging_echoes/Movies")
LIVE_DIR = Path("/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/Movies")
BACKUP_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/legacy_movies_2003")
WORK_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/temp_upscale_movies")

TARGET_FILES = [
    "05_3a.bik", "05_3c.bik", "05_4a.bik", "05_4c.bik",
    "05_5a.bik", "05_5c.bik", "05_8a.bik", "05_8c.bik",
    "22a.bik", "22b.bik", "23a.bik", "23b.bik",
    "LIVE_1a.bik", "Live_1c.bik"
]

def run_cmd(cmd, env=None, cwd=None):
    res = subprocess.run(cmd, env=env, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print(f"\n[ERROR] Command failed (exit {res.returncode}): {' '.join(str(c) for c in cmd)}")
        print("STDOUT:\n", res.stdout[-1000:])
        print("STDERR:\n", res.stderr[-1000:])
        raise RuntimeError(f"Command failed: {cmd[0]}")
    return res

def proton_run(args):
    RADVIDEO_PFX.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env["STEAM_COMPAT_CLIENT_INSTALL_PATH"] = STEAM_CLIENT_PATH
    env["STEAM_COMPAT_DATA_PATH"] = str(RADVIDEO_PFX)
    cmd = [str(PROTON_BIN), "run"] + [str(a) for a in args]
    return run_cmd(cmd, env=env)

def to_wine_path(path: Path) -> str:
    return "Z:" + str(path.resolve()).replace("/", "\\")

def is_already_upscaled(bik_path: Path) -> bool:
    if not bik_path.exists():
        return False
    cmd = [str(FFMPEG_BIN), "-i", str(bik_path)]
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    out = res.stdout + res.stderr
    return "3440x1440" in out and "Video: binkvideo" in out

def verify_bik(bik_path: Path):
    cmd = [str(FFMPEG_BIN), "-i", str(bik_path)]
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    out = res.stdout + res.stderr
    if "Video: binkvideo" not in out or "3440x1440" not in out:
        raise ValueError(f"Verification failed for video stream in {bik_path.name}")
    if "Audio: binkaudio" not in out:
        raise ValueError(f"Verification failed for audio stream in {bik_path.name}")
    print(f"  [OK] Verified {bik_path.name}: BIKi 3440x1440 with Bink DCT audio")

def process_movie(filename: str, idx: int, total: int):
    print(f"\n{'='*70}")
    print(f"[{idx}/{total}] Checking: {filename}")
    print(f"{'='*70}")
    
    t0 = time.time()
    
    # Locate live target file
    live_target = LIVE_DIR / filename
    if not live_target.exists():
        for f in LIVE_DIR.iterdir():
            if f.name.lower() == filename.lower():
                live_target = f
                break
                
    if live_target.exists() and is_already_upscaled(live_target):
        print(f"  [SKIP] {live_target.name} is already upscaled to 3440x1440.")
        return

    # Locate source file (prefer original backup if available, else staging or live)
    src_file = None
    backup_file = BACKUP_DIR / filename
    if not backup_file.exists():
        for f in BACKUP_DIR.iterdir():
            if f.name.lower() == filename.lower():
                backup_file = f
                break
                
    if backup_file.exists():
        src_file = backup_file
    else:
        for candidate in [SOURCE_DIR / filename, live_target]:
            if candidate.exists():
                src_file = candidate
                break

    if not src_file or not src_file.exists():
        print(f"  [SKIP] Could not find source file for {filename}")
        return

    # Backup original before modifying
    BACKUP_DIR.mkdir(parents=True, exist_ok=True)
    if not (BACKUP_DIR / src_file.name).exists():
        shutil.copy2(src_file, BACKUP_DIR / src_file.name)
        print(f"  Backed up original {src_file.name} -> {BACKUP_DIR / src_file.name}")

    # Set up clean work directories
    raw_dir = WORK_DIR / "raw_frames"
    upscaled_dir = WORK_DIR / "upscaled_frames"
    final_dir = WORK_DIR / "final_frames"
    
    for d in [raw_dir, upscaled_dir, final_dir]:
        if d.exists():
            shutil.rmtree(d)
        d.mkdir(parents=True, exist_ok=True)

    audio_wav = WORK_DIR / "audio.wav"
    if audio_wav.exists():
        audio_wav.unlink()

    # Step 1: Extraction
    print(f"  [1/5] Extracting video frames and audio from {src_file.name}...")
    extract_video_cmd = [
        str(FFMPEG_BIN), "-y", "-i", str(src_file),
        "-an", str(raw_dir / "%05d.png")
    ]
    run_cmd(extract_video_cmd)
    
    extract_audio_cmd = [
        str(FFMPEG_BIN), "-y", "-i", str(src_file),
        "-vn", "-c:a", "pcm_s16le", str(audio_wav)
    ]
    run_cmd(extract_audio_cmd)
    
    frame_count = len(list(raw_dir.glob("*.png")))
    print(f"  Extracted {frame_count} frames and audio.wav")

    # Step 2: AI Upscale
    print("  [2/5] AI Upscaling (realesr-animevideov3 on RTX 5070)...")
    t_up = time.time()
    upscale_cmd = [
        str(REALESRGAN_BIN),
        "-i", str(raw_dir),
        "-o", str(upscaled_dir),
        "-s", "4",
        "-n", "realesr-animevideov3"
    ]
    run_cmd(upscale_cmd, cwd=str(REALESRGAN_DIR))
    print(f"  Upscaling completed in {time.time() - t_up:.1f}s")

    # Step 3: Resize and Pad to 3440x1440
    print("  [3/5] Scaling to 3440x1440 (Lanczos, 2.35:1 padded)...")
    rescale_cmd = [
        str(FFMPEG_BIN), "-y",
        "-framerate", "29.97",
        "-i", str(upscaled_dir / "%05d.png"),
        "-vf", "scale=3388:1440:flags=lanczos,pad=3440:1440:(ow-iw)/2:(oh-ih)/2:black",
        str(final_dir / "%05d.png")
    ]
    run_cmd(rescale_cmd)

    # Step 4: Write .lst file for RAD Video Tools
    final_frames = sorted(list(final_dir.glob("*.png")))
    lst_file = WORK_DIR / "frames.lst"
    with open(lst_file, "w") as f:
        for p in final_frames:
            f.write(to_wine_path(p) + "\n")

    # Step 5: Bink 1 Compression & Audio Muxing
    print("  [4/5] Compressing to Bink 1 (BIKi) & muxing audio...")
    temp_bik = WORK_DIR / "temp_video.bik"
    out_bik = WORK_DIR / filename
    
    if temp_bik.exists():
        temp_bik.unlink()
    if out_bik.exists():
        out_bik.unlink()

    # Bink Video
    proton_run([
        to_wine_path(RADVIDEO_BIN),
        "binkc",
        to_wine_path(lst_file),
        to_wine_path(temp_bik),
        "/f29.97",
        "/#"
    ])

    # Bink Audio Mux
    proton_run([
        to_wine_path(RADVIDEO_BIN),
        "binkmix",
        to_wine_path(temp_bik),
        to_wine_path(audio_wav),
        to_wine_path(out_bik),
        "/#"
    ])

    # Step 6: Verification & Deployment
    print("  [5/5] Verifying & Deploying...")
    verify_bik(out_bik)

    # Deploy to staging and live
    for dest_dir in [SOURCE_DIR, LIVE_DIR]:
        if dest_dir.exists():
            target = dest_dir / filename
            shutil.copy2(out_bik, target)
            print(f"  Deployed to {dest_dir.name}/{filename}")

    # Step 7: Cleanup intermediate frames
    for d in [raw_dir, upscaled_dir, final_dir]:
        shutil.rmtree(d, ignore_errors=True)
    for f in [audio_wav, lst_file, temp_bik, out_bik]:
        if f.exists():
            f.unlink()

    total_time = time.time() - t0
    print(f"  Finished {filename} in {total_time:.1f}s ({total_time/frame_count*1000:.1f} ms/frame)")

def main():
    print("=================================================================")
    print(" KOTOR II Legacy Movies Upscaler (3440x1440 Bink 1)")
    print("=================================================================")
    print(f"Found {len(TARGET_FILES)} target files to check/upscale.")
    
    WORK_DIR.mkdir(parents=True, exist_ok=True)
    t_start = time.time()
    
    for idx, fname in enumerate(TARGET_FILES, 1):
        process_movie(fname, idx, len(TARGET_FILES))
        
    shutil.rmtree(WORK_DIR, ignore_errors=True)
    
    print("\n" + "="*70)
    print("BATCH UPSCALING RUN FINISHED.")
    print(f"Total time elapsed: {(time.time() - t_start)/60:.1f} minutes")
    print("="*70)

if __name__ == "__main__":
    main()
