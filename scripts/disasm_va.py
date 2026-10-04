#!/usr/bin/env python3
"""Disassemble a VA range of the Steam or GOG K2 exe offline (read-only).

    python3 scripts/disasm_va.py steam 0x40D901 [--before 32] [--len 64]
    python3 scripts/disasm_va.py gog 0x613863 --bytes 6     # just hex bytes

steam = the vanilla exe (PE sections mapped by objdump); gog = the raw memory blocks exported
from the OpenKotOR Ghidra server by scripts/ghidra_scripts/ExportMemoryBlocks.java.
--before starts the listing earlier; objdump may desync at the start, so read from the
first instruction that lands on the target VA.
"""
import argparse, csv, os, subprocess, sys
from pathlib import Path

STEAM_EXE = Path(os.environ.get("K2_STEAM_EXE", os.path.expanduser(
    "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe.unpatched_vanilla")))
GOG_BLOCKS = Path(os.path.expanduser(
    "~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/k2_win_gog_aspyr_swkotor2.exe/memblocks"))


def gog_block(va):
    with open(GOG_BLOCKS / "blocks.tsv") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            start, size = int(r["start"], 16), int(r["size"])
            if start <= va < start + size:
                return GOG_BLOCKS / r["file"], start
    sys.exit(f"0x{va:X} not in any GOG block")


def read_bytes(which, va, n):
    if which == "gog":
        path, start = gog_block(va)
        with open(path, "rb") as f:
            f.seek(va - start)
            return f.read(n)
    import struct
    data = STEAM_EXE.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
    for i in range(nsec):
        o = pe + 24 + opt + 40 * i
        vsz, rva, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
        if base + rva <= va < base + rva + max(vsz, rsz):
            off = raw + va - base - rva
            return data[off:off + n]
    sys.exit(f"0x{va:X} not in any Steam section")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("exe", choices=["steam", "gog"])
    ap.add_argument("va", type=lambda s: int(s, 16))
    ap.add_argument("--before", type=int, default=0)
    ap.add_argument("--len", type=int, default=48)
    ap.add_argument("--bytes", type=int, help="print N hex bytes at VA and exit")
    a = ap.parse_args()
    if a.bytes:
        print(" ".join(f"{b:02X}" for b in read_bytes(a.exe, a.va, a.bytes)))
        return
    start = a.va - a.before
    blob = read_bytes(a.exe, start, a.before + a.len)
    tmp = Path(os.environ.get("TMPDIR", "/tmp")) / f"disasm_va_{os.getpid()}.bin"
    tmp.write_bytes(blob)
    try:
        out = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386", "-M", "intel",
                              f"--adjust-vma=0x{start:X}", str(tmp)], capture_output=True, text=True).stdout
    finally:
        tmp.unlink()
    print("\n".join(l for l in out.splitlines() if l[:1] == " " and ":" in l))


if __name__ == "__main__":
    main()
