#!/usr/bin/env python3
"""Thin wrapper (since the party AI scripts build): the Mod 8-only k_ai_master.ncs. Never writes the game dir.

  build_kite_script.py [--work DIR]      (default DIR = ~/Games/KOTOR2_Mod_Workspace/mod8_kite_script)

Same result and same paths as before (DIR/src/k_inc_generic.nss, DIR/out/k_ai_master.ncs, DIR/checkroot), now through
build_party_ai_scripts.py --baseline-only: the vanilla toolchain proof, the two Mod 8 insertions (party_ai_patches.MOD8_ONLY)
with the v1 kite routine (git dad84a5, kept in DIR/baseline/), a required md5 of 94db8304 (the live Mod 8 0.2.0 build),
and the arg-order check. apply_mods68_data.py installs DIR/out/k_ai_master.ncs.
The full party AI set (17 scripts, kite routine v2 and the vanilla fixes) is built by build_party_ai_scripts.py.
"""
import os, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.exit(subprocess.call([sys.executable, os.path.join(HERE, "build_party_ai_scripts.py"), "--baseline-only"] + sys.argv[1:]))
