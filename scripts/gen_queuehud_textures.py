#!/usr/bin/env python3
"""Queue HUD texture generator: forwarder. Queue HUD has no art of its own since 0.4.0; its frame / cancel textures are the shared
HUD kit (kit_frame, kit_frame_hi, kit_cancel, kit_cancel_hi, kit_disc) emitted by scripts/gen_hud_kit_textures.py into
patch_manager_mods/_shared_src/HudKitTextures.inc. This script only runs that generator so old muscle memory still works.
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_hud_kit_textures  # noqa: E402

if __name__ == "__main__":
    sys.exit(gen_hud_kit_textures.main())
