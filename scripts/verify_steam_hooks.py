#!/usr/bin/env python3
"""Verify the Steam (Aspyr) hook tomls of a Kotor-Patch-Manager worktree against the vanilla exe.

    python3 scripts/verify_steam_hooks.py <kpm_worktree> [--exe <exe>] [--tsv out.tsv] [--patch NAME ...]

Reads <worktree>/Patches/*/kotor2-steam-aspyr.hooks.toml (read-only; the exe is opened "rb" only).
Default exe: $K2_STEAM_EXE, else ~/.local/share/Steam/.../swkotor2.exe.unpatched_vanilla.
Output: TSV (stdout or --tsv) with columns patch, hook_index, address, function, check, result, detail.
Summary line on stderr. Exit 1 on any FAIL, 2 on usage errors, else 0.

Checks (per [[hooks]] entry; hook_index is 0-based, "-" for file-level rows):
  SHA       file-level: exe sha256 must match one [metadata] target_versions entry (FAIL if none; WARN if
            the toml has no target_versions).
  BYTES     original_bytes == exe bytes at `address` (VA -> file offset through the PE section headers).
            Missing original_bytes = WARN; address outside any section / past raw data = FAIL.
  BOUNDARY  `address` is on an x86 instruction boundary. Only for hooks inside .text (else SKIP).
            Heuristic (objdump linear sweep desyncs when started mid-instruction, but x86 self-
            synchronises within a few instructions): decode the VA neighbourhood from several different
            start points (VA-16 .. VA-64 step 8, the .text start if it is within 512 bytes, and the
            nearest 16-byte aligned "function start" preceded by 0xCC padding and beginning with a
            common prologue byte; the last two count double). Each decode votes on whether VA is an
            instruction start. weighted yes-ratio >= 0.7 -> boundary; <= 0.3 -> mid-instruction;
            in between -> ambiguous (WARN). Mid-instruction = FAIL for `detour` hooks (a jmp cannot be
            planted there); for other types (simple/replace/static) it is a WARN, because in-place byte
            patches legitimately edit immediates/displacements. For detours the decode from VA must also
            end an instruction exactly at len(original_bytes) (whole instructions are overwritten) and
            contain no "(bad)"; otherwise FAIL.
  UNIQUE    original_bytes + the 8 following exe bytes occur exactly once in .text (exact bytes.find loop).
            Non-.text or no original_bytes = SKIP.
  DRIFT     vs. the sibling GOG hooks file (kotor2-gog-aspyr.hooks.toml, else the *hooks.toml
            targeting the GOG sha; absent = SKIP): hook counts must match; hooks are paired
            by order and every field except address, original_bytes and consumed_exit_address (an address;
            only its presence is compared) must be equal; each differing field is reported by name.
            Two benign, build-dependent differences are WARN, not FAIL: (a) replacement_bytes of equal
            length differing only inside <= max(4, len/8) four-byte windows (absolute addresses / rel32 operands that
            legitimately differ between the two exes); (b) `parameters` differing only in the `source`
            of ebp-/esp- relative stack references (stack frame layout differs per build).
            Anything else (type, function, lengths, other params, flags ...) is FAIL.

Allowlist (--allow FILE, repeatable; default <repo>/patch_manager_mods/upstream_prs/verify_steam_hooks/
allowlist.tsv if present): TSV lines `patch<TAB>hook_index|*<TAB>check<TAB>reason`; a matching FAIL row is
demoted to WARN with the reason appended. Use it for hooks that are legitimately address-pinned duplicates
(UNIQUE) or otherwise deliberate. Lines starting with # are ignored.
"""
import argparse
import concurrent.futures
import hashlib
import os
import re
import struct
import subprocess
import sys
import tempfile
import tomllib
from pathlib import Path

DEFAULT_EXE = Path(os.environ.get("K2_STEAM_EXE", os.path.expanduser(
    "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe.unpatched_vanilla")))
STEAM_TOML = "kotor2-steam-aspyr.hooks.toml"
DEFAULT_ALLOW = Path(__file__).resolve().parent.parent / "patch_manager_mods/upstream_prs/verify_steam_hooks/allowlist.tsv"
GOG_TOML = "kotor2-gog-aspyr.hooks.toml"
GOG_SHA = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"


def find_gog_toml(d):
    """The patch's GOG hooks file: KPM picks hook files by [metadata] target_versions, not by name,
    so older patches use hooks.toml / kotor2_hooks.toml. Prefer the canonical name, else the
    *hooks.toml whose target_versions holds the GOG sha. None if the patch has no GOG file."""
    if (d / GOG_TOML).is_file():
        return d / GOG_TOML
    for f in sorted(d.glob("*hooks.toml")):
        if f.name == STEAM_TOML:
            continue
        try:
            tv = tomllib.loads(f.read_text(encoding="utf-8")).get("metadata", {}).get("target_versions", [])
        except (tomllib.TOMLDecodeError, OSError):
            continue
        if GOG_SHA in (v.upper() for v in tv):
            return f
    return None
COLUMNS = ["patch", "hook_index", "address", "function", "check", "result", "detail"]
DRIFT_EXEMPT = {"address", "original_bytes", "consumed_exit_address"}
TAIL = 8
YES_RATIO, NO_RATIO = 0.7, 0.3
PROLOGUE_FIRST = {0x55, 0x56, 0x53, 0x51, 0x57, 0x83, 0x6A, 0x81, 0x8B, 0x64}


# --------------------------------------------------------------------------- PE

class PE:
    """Minimal PE32 reader: section table + VA <-> file offset (same header math as disasm_va.py)."""

    def __init__(self, data):
        self.data = data
        if data[:2] != b"MZ":
            raise ValueError("not a PE file (no MZ)")
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        if data[pe:pe + 4] != b"PE\0\0":
            raise ValueError("not a PE file (no PE signature)")
        nsec = struct.unpack_from("<H", data, pe + 6)[0]
        opt = struct.unpack_from("<H", data, pe + 20)[0]
        self.base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
        hdr = struct.unpack_from("<I", data, pe + 24 + 60)[0]  # SizeOfHeaders
        # headers are mapped at ImageBase (4GBPatch edits the PE header's flags there)
        self.sections = [dict(name="<headers>", va=self.base, vsize=hdr, raw=0, rawsize=hdr)]
        for i in range(nsec):
            o = pe + 24 + opt + 40 * i
            name = data[o:o + 8].rstrip(b"\0").decode("latin-1")
            vsz, rva, rsz, raw = struct.unpack_from("<IIII", data, o + 8)
            self.sections.append(dict(name=name, va=self.base + rva, vsize=vsz, raw=raw, rawsize=rsz))

    def section_at(self, va):
        for s in self.sections:
            if s["va"] <= va < s["va"] + max(s["vsize"], s["rawsize"]):
                return s
        return None

    def read(self, va, n):
        """Return n bytes at va, or None if not fully backed by file data of a single section."""
        s = self.section_at(va)
        if s is None or va + n > s["va"] + s["rawsize"]:
            return None
        off = s["raw"] + va - s["va"]
        return self.data[off:off + n]

    def text(self):
        for s in self.sections:
            if s["name"] == ".text":
                return s
        return None

    def text_bytes(self, s):
        return self.data[s["raw"]:s["raw"] + s["rawsize"]]


# ---------------------------------------------------------------------- objdump

_INSN = re.compile(r"^\s*([0-9a-f]+):\t")


def objdump_available():
    try:
        subprocess.run(["objdump", "--version"], capture_output=True, check=True)
        return True
    except (OSError, subprocess.CalledProcessError):
        return False


def decode(path, vma, start, stop):
    """Run objdump on a raw binary slice; return list of (address, mnemonic_text) instruction starts."""
    out = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386", "-M", "intel",
                          f"--adjust-vma=0x{vma:X}", f"--start-address=0x{start:X}",
                          f"--stop-address=0x{stop:X}", str(path)],
                         capture_output=True, text=True).stdout
    insns = []
    for line in out.splitlines():
        m = _INSN.match(line)
        if not m:
            continue
        fields = line.split("\t")
        if len(fields) >= 3:  # continuation lines of long instructions carry only addr + bytes
            insns.append((int(m.group(1), 16), fields[2].strip()))
    return insns


def find_function_start(blob, blob_va, va):
    """Nearest 16-aligned p <= va with 0xCC 0xCC before it and a common prologue byte at it, else None."""
    p = va & ~0xF
    while p >= blob_va + 2:
        i = p - blob_va
        if blob[i - 1] == 0xCC and blob[i - 2] == 0xCC and blob[i] in PROLOGUE_FIRST:
            return p
        p -= 16
    return None


def analyse_boundary(pe, text, va, orig_len):
    """Return dict(ratio, votes, total, end_ok, bad) for the VA via multi-start decoding (see docstring)."""
    lo = max(text["va"], va - 512)
    hi = min(text["va"] + text["rawsize"], va + max(orig_len, 1) + 32)
    blob = pe.read(lo, hi - lo)
    starts = {}  # start -> weight
    for k in range(16, 65, 8):
        if va - k >= text["va"]:
            starts[va - k] = starts.get(va - k, 0) + 1
    if va - text["va"] <= 512:
        starts[text["va"]] = 2
    fs = find_function_start(blob, lo, va)
    if fs is not None:
        starts[fs] = max(starts.get(fs, 0), 2)
    if not starts:
        starts[va] = 1
    with tempfile.TemporaryDirectory() as td:
        path = Path(td) / "w.bin"
        path.write_bytes(blob)
        yes = total = 0
        for s, w in starts.items():
            insns = decode(path, lo, s, hi)
            total += w
            if any(a == va for a, _ in insns):
                yes += w
        # decode from the VA itself for end-of-original coverage
        own = decode(path, lo, va, hi)
    addrs = {a for a, _ in own}
    end_ok = (va + orig_len) in addrs if orig_len else True
    bad = any("(bad)" in t for a, t in own if a < va + orig_len)
    return dict(ratio=yes / total if total else 1.0, votes=yes, total=total, end_ok=end_ok, bad=bad)


# ------------------------------------------------------------------------ rows

def fmt_bytes(b, limit=12):
    b = list(b)
    s = " ".join(f"{x:02X}" for x in b[:limit])
    return s + (f" ..(+{len(b) - limit})" if len(b) > limit else "")


def fmt_val(v):
    if isinstance(v, list) and v and all(isinstance(x, int) for x in v):
        return "[" + fmt_bytes(v) + "]"
    s = repr(v)
    return s if len(s) <= 80 else s[:77] + "..."


def operand_like(a, b):
    """True if byte lists a, b have equal length and differ only inside <= max(4, len//8) 4-byte windows."""
    if len(a) != len(b):
        return False
    idx = [i for i, (x, y) in enumerate(zip(a, b)) if x != y]
    groups, end = 0, -1
    for i in idx:
        if i > end:
            groups += 1
            end = i + 3
    return groups <= max(4, len(a) // 8)


_STACK = re.compile(r"^\s*(ebp|esp)\s*[-+]", re.I)


def params_stack_only(sp, gp):
    """True if parameter lists differ only in `source` of stack-relative refs on both sides."""
    if not (isinstance(sp, list) and isinstance(gp, list)) or len(sp) != len(gp):
        return False
    for x, y in zip(sp, gp):
        if not (isinstance(x, dict) and isinstance(y, dict)):
            return False
        if set(x) != set(y):
            return False
        for k in x:
            if x[k] == y[k]:
                continue
            if not (k == "source" and _STACK.match(str(x[k])) and _STACK.match(str(y[k]))):
                return False
    return True


def drift_fields(sh, gh):
    """Differing fields between a Steam and a GOG hook as [(severity, text)], severity FAIL or WARN."""
    diffs = []
    for k in sorted((set(sh) | set(gh))):
        if k in DRIFT_EXEMPT:
            if k == "consumed_exit_address" and ((k in sh) != (k in gh)):
                diffs.append(("FAIL", f"{k}: present only in {'steam' if k in sh else 'gog'}"))
            continue
        if k not in sh:
            diffs.append(("FAIL", f"{k}: missing in steam (gog={fmt_val(gh[k])})"))
        elif k not in gh:
            diffs.append(("FAIL", f"{k}: missing in gog (steam={fmt_val(sh[k])})"))
        elif sh[k] != gh[k]:
            txt = f"{k}: steam={fmt_val(sh[k])} gog={fmt_val(gh[k])}"
            if k == "replacement_bytes" and operand_like(sh[k], gh[k]):
                nd = sum(1 for x, y in zip(sh[k], gh[k]) if x != y)
                diffs.append(("WARN", f"{k}: {nd} bytes differ, only in address-operand-sized windows"))
            elif k == "parameters" and params_stack_only(sh[k], gh[k]):
                diffs.append(("WARN", f"{k}: only stack-relative sources differ (build-dependent frame layout)"))
            else:
                diffs.append(("FAIL", txt))
    return diffs


def find_all(hay, needle):
    out, i = [], hay.find(needle)
    while i != -1:
        out.append(i)
        i = hay.find(needle, i + 1)
    return out


def verify_patch(pe, exe_sha, name, steam_path, gog_path, have_objdump, pool):
    rows = []

    def add(idx, hook, check, result, detail=""):
        addr = hook.get("address") if hook else None
        rows.append((name, idx, f"0x{addr:08X}" if isinstance(addr, int) else "-",
                     (hook or {}).get("function", "-") if hook else "-", check, result, detail))

    try:
        sdoc = tomllib.loads(steam_path.read_text(encoding="utf-8"))
    except Exception as e:  # noqa: BLE001
        add("-", None, "PARSE", "FAIL", f"steam toml: {e}")
        return rows
    versions = [str(v).upper() for v in sdoc.get("metadata", {}).get("target_versions", [])]
    if not versions:
        add("-", None, "SHA", "WARN", "no [metadata] target_versions in toml")
    elif exe_sha.upper() in versions:
        add("-", None, "SHA", "PASS", f"exe sha256 {exe_sha[:8]}.. in target_versions")
    else:
        add("-", None, "SHA", "FAIL", f"exe sha256 {exe_sha[:8]}.. matches none of target_versions {[v[:8] + '..' for v in versions]}")

    hooks = sdoc.get("hooks", [])
    text = pe.text()
    text_bytes = pe.text_bytes(text) if text else b""

    gdoc = None
    if gog_path is not None and gog_path.exists():
        try:
            gdoc = tomllib.loads(gog_path.read_text(encoding="utf-8"))
        except Exception as e:  # noqa: BLE001
            add("-", None, "PARSE", "FAIL", f"gog toml: {e}")
    ghooks = gdoc.get("hooks", []) if gdoc else []
    if gdoc is not None and len(ghooks) != len(hooks):
        add("-", None, "DRIFT", "FAIL", f"hook count steam={len(hooks)} gog={len(ghooks)}")

    # boundary analyses (objdump) run in the pool
    futures = {}
    for i, h in enumerate(hooks):
        va, orig = h.get("address"), h.get("original_bytes")
        if (have_objdump and isinstance(va, int) and text and text["va"] <= va < text["va"] + text["rawsize"]):
            futures[i] = pool.submit(analyse_boundary, pe, text, va, len(orig) if orig else 0)

    for i, h in enumerate(hooks):
        va, orig, htype = h.get("address"), h.get("original_bytes"), h.get("type", "?")
        if not isinstance(va, int):
            add(i, h, "BYTES", "FAIL", "hook has no integer address")
            continue
        in_text = bool(text and text["va"] <= va < text["va"] + text["rawsize"])
        # BYTES
        if not orig:
            add(i, h, "BYTES", "WARN", "no original_bytes")
        else:
            got = pe.read(va, len(orig))
            if got is None:
                add(i, h, "BYTES", "FAIL", "address range not backed by exe section data")
            elif list(got) == list(orig):
                add(i, h, "BYTES", "PASS", f"{len(orig)} bytes match")
            else:
                add(i, h, "BYTES", "FAIL", f"toml {fmt_bytes(orig)} != exe {fmt_bytes(got)}")
        # BOUNDARY
        if not in_text:
            add(i, h, "BOUNDARY", "SKIP", "outside .text")
        elif not have_objdump:
            add(i, h, "BOUNDARY", "SKIP", "objdump not available")
        else:
            r = futures[i].result()
            strict = htype == "detour"
            if r["ratio"] >= YES_RATIO:
                if strict and r["bad"]:
                    add(i, h, "BOUNDARY", "FAIL", "undecodable (bad) instruction inside original_bytes")
                elif strict and not r["end_ok"]:
                    add(i, h, "BOUNDARY", "FAIL",
                        f"on boundary but original_bytes ({len(orig)}) do not end on an instruction boundary")
                else:
                    add(i, h, "BOUNDARY", "PASS", f"votes {r['votes']}/{r['total']}")
            elif r["ratio"] <= NO_RATIO:
                add(i, h, "BOUNDARY", "FAIL" if strict else "WARN",
                    f"mid-instruction (votes {r['votes']}/{r['total']})"
                    + ("" if strict else "; allowed for in-place byte patches"))
            else:
                add(i, h, "BOUNDARY", "WARN", f"ambiguous decode (votes {r['votes']}/{r['total']})")
        # UNIQUE
        if not in_text or not orig:
            add(i, h, "UNIQUE", "SKIP", "outside .text" if not in_text else "no original_bytes")
        else:
            off = va - text["va"]
            tail = text_bytes[off + len(orig):off + len(orig) + TAIL]
            pat = bytes(orig) + tail
            hits = find_all(text_bytes, pat)
            n = len(hits)
            others = [f"0x{text['va'] + x:08X}" for x in hits if x != off][:4]
            add(i, h, "UNIQUE", "PASS" if n == 1 else "FAIL",
                f"{len(pat)}-byte pattern occurs {n}x in .text"
                + (f"; also at {', '.join(others)}" if others else ""))
        # DRIFT
        if gdoc is None:
            add(i, h, "DRIFT", "SKIP", "no sibling gog toml")
        elif i >= len(ghooks):
            add(i, h, "DRIFT", "FAIL", "no paired gog hook (count mismatch)")
        else:
            diffs = drift_fields(h, ghooks[i])
            res = "FAIL" if any(r == "FAIL" for r, _ in diffs) else ("WARN" if diffs else "PASS")
            add(i, h, "DRIFT", res, "; ".join(t for _, t in diffs) if diffs else "fields equal")
    return rows


def load_allowlist(paths):
    allow = []
    for p in paths:
        for line in Path(p).read_text(encoding="utf-8").splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            f = line.split("\t")
            if len(f) >= 3:
                allow.append((f[0], f[1], f[2].upper(), f[3] if len(f) > 3 else ""))
    return allow


def apply_allowlist(rows, allow):
    out = []
    for r in rows:
        if r[5] == "FAIL":
            for patch, idx, check, reason in allow:
                if patch == r[0] and check == r[4] and idx in ("*", str(r[1])):
                    r = r[:5] + ("WARN", f"{r[6]} [allowlisted: {reason}]")
                    break
        out.append(r)
    return out


def write_tsv(rows, out):
    out.write("\t".join(COLUMNS) + "\n")
    for r in rows:
        out.write("\t".join(re.sub(r"[\t\r\n]+", " ", str(c)) for c in r) + "\n")


def run(worktree, exe, patches=None, allow=()):
    """Verify a worktree; returns list of row tuples. Raises FileNotFoundError/ValueError on bad inputs."""
    worktree = Path(worktree)
    pdir = worktree / "Patches"
    if not pdir.is_dir():
        raise FileNotFoundError(f"{pdir} not found")
    data = Path(exe).read_bytes()
    pe = PE(data)
    sha = hashlib.sha256(data).hexdigest()
    have_objdump = objdump_available()
    rows = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=min(8, (os.cpu_count() or 2))) as pool:
        for d in sorted(pdir.iterdir()):
            sp = d / STEAM_TOML
            if not sp.is_file() or (patches and d.name not in patches):
                continue
            rows += verify_patch(pe, sha, d.name, sp, find_gog_toml(d), have_objdump, pool)
    if not have_objdump:
        print("warning: objdump not found; BOUNDARY checks skipped", file=sys.stderr)
    return apply_allowlist(rows, load_allowlist(allow)) if allow else rows


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("worktree")
    ap.add_argument("--exe", default=str(DEFAULT_EXE))
    ap.add_argument("--tsv")
    ap.add_argument("--patch", nargs="+")
    ap.add_argument("--allow", action="append", default=None, help="allowlist TSV (repeatable)")
    a = ap.parse_args(argv)
    try:
        allow = a.allow
        if allow is None:
            default = DEFAULT_ALLOW
            allow = [str(default)] if default.is_file() else []
        rows = run(a.worktree, a.exe, set(a.patch) if a.patch else None, allow)
    except (OSError, ValueError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 2
    if a.tsv:
        Path(a.tsv).parent.mkdir(parents=True, exist_ok=True)
        with open(a.tsv, "w", encoding="utf-8") as f:
            write_tsv(rows, f)
    else:
        write_tsv(rows, sys.stdout)
    counts = {k: sum(1 for r in rows if r[5] == k) for k in ("PASS", "FAIL", "WARN", "SKIP")}
    npatch = len({r[0] for r in rows})
    print(f"verify_steam_hooks: {npatch} patches, {len(rows)} rows: "
          + " ".join(f"{k}={v}" for k, v in counts.items()), file=sys.stderr)
    return 1 if counts["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
