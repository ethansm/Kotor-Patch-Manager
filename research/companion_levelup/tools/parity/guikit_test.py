#!/usr/bin/env python3
"""Compile (host g++, stub windows.h) and run guikit_test.cpp, the host test of patch_manager_mods/_shared_src/GuiKit.h."""
import os, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
exe = os.path.join(HERE, "guikit_test_bin")
r = subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Wextra", "-I", os.path.join(HERE, "stub"), "-o", exe, os.path.join(HERE, "guikit_test.cpp")], capture_output=True, text=True)
if r.returncode:
    sys.exit(r.stderr)
r = subprocess.run([exe], capture_output=True, text=True)
sys.stdout.write(r.stdout); sys.stderr.write(r.stderr)
sys.exit(r.returncode)
