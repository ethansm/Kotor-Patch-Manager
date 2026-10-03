#!/usr/bin/env python3
"""Static checks for CharEffectsPanel.kpatch (Mod 14 character-effects-panel; copied from check_equip3d_kpatch.py).
(a) hooks' original_bytes == exe bytes at file offset VA-0x400C00; (b) no hook within +-8 bytes of another live hook
(patch_config.toml addresses + the hooks of every other .kpatch in Patches/*/ and in the optional staging dir);
(c) zip has manifest, hooks toml, DLL; (d) DLL exports exactly the 3 hooks, imports USER32 (input).
usage: check_chareffects_kpatch.py [kpatch] [staging dir]
"""
import re, subprocess, sys, zipfile, tomllib
from pathlib import Path

TOOL = Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"
GAME = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
KP = Path(sys.argv[1]) if len(sys.argv) > 1 else TOOL / "Patches/CharEffectsPanel/CharEffectsPanel.kpatch"
STAGE = Path(sys.argv[2]) if len(sys.argv) > 2 else None
fails = []
def ck(name, ok, detail=""):
    print(("PASS " if ok else "FAIL ") + name + (" - " + detail if detail else ""))
    if not ok: fails.append(name)

z = zipfile.ZipFile(KP)
names = z.namelist()
hooks_name = next((n for n in names if n.endswith(".hooks.toml")), None)
ck("(c) zip contents", "manifest.toml" in names and hooks_name and "binaries/windows_x86.dll" in names, str(names))
hooks = tomllib.loads(z.read(hooks_name).decode())["hooks"]
exe = (GAME / "swkotor2.exe").read_bytes()
mine = []
for h in hooks:
    va = h["address"] if isinstance(h["address"], int) else int(str(h["address"]), 16)
    ob = bytes.fromhex(h["original_bytes"].replace(" ", "")) if isinstance(h["original_bytes"], str) else bytes(h["original_bytes"])
    ck(f"(a) hook 0x{va:x} original_bytes", exe[va - 0x400C00:va - 0x400C00 + len(ob)] == ob, ob.hex(" "))
    mine.append((va, len(ob)))
# own hooks must not overlap each other (+-8)
self_clash = [(hex(a), hex(b)) for i, (a, la) in enumerate(mine) for j, (b, lb) in enumerate(mine) if i < j and a - 8 - la <= b <= a + la + 8 or False]
ck("(b0) own hooks do not overlap", not self_clash, str(self_clash))
other = []
cfg = tomllib.loads((GAME / "patch_config.toml").read_text())
own_id = tomllib.loads(z.read("manifest.toml").decode())["patch"]["id"]
for pt in cfg.get("patches", []):
    if pt.get("id") == own_id:  # the installed copy of THIS patch is replaced on reinstall
        continue
    for h in pt.get("hooks", []): other.append((h["address"], pt.get("id", "?")))
kps = list((TOOL / "Patches").glob("*/*.kpatch")) + (list(STAGE.glob("*.kpatch")) if STAGE else [])
for kp in kps:
    if kp.resolve() == KP.resolve(): continue
    try:
        if tomllib.loads(zipfile.ZipFile(kp).read("manifest.toml").decode())["patch"]["id"] == own_id: continue
    except Exception:
        pass
    zz = zipfile.ZipFile(kp)
    for n in zz.namelist():
        if n.endswith(".hooks.toml"):
            for h in tomllib.loads(zz.read(n).decode()).get("hooks", []):
                a = h["address"]; other.append((a if isinstance(a, int) else int(str(a), 16), kp.name))
clash = [(hex(va), hex(a), src) for va, ln in mine for a, src in other if va - 8 - 6 <= a <= va + ln + 8]
ck("(b) no hook within +-8 bytes of another live hook", not clash, f"{len(other)} other hook addrs checked; clashes={clash} (own installed copy excluded)")
import tempfile
dll = Path(tempfile.gettempdir()) / "_chareffects_chk.dll"
dll.write_bytes(z.read("binaries/windows_x86.dll"))
out = subprocess.run(["i686-w64-mingw32-objdump", "-p", str(dll)], capture_output=True, text=True).stdout
dll.unlink()
exports = re.findall(r"^\s+\[\s*\d+\]\s+(\w+)\s*$", out.split("[Ordinal/Name Pointer] Table")[-1].split("PE File")[0], re.M)
imports = sorted(set(re.findall(r"DLL Name: (\S+)", out)))
ck("(d) exports exactly the 3 hooks", sorted(exports) == ["BindCharEffects", "CharEffectsDestroy", "CharEffectsTick"], str(exports))
ck("(d) imports USER32.dll", any(i.lower() == "user32.dll" for i in imports), str(imports))
# import names of interest
for fn in ("SetWindowLongA", "GetAsyncKeyState", "GetCursorPos", "ScreenToClient", "EnumWindows"):
    ck(f"(d) imports {fn}", fn in out)
print("ALL PASS" if not fails else f"FAILED: {fails}")
sys.exit(1 if fails else 0)
