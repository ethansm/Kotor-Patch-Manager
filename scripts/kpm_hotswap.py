#!/usr/bin/env python3
"""Batched KPM DLL hot-swap: one verdict check, one backup, one atomic swap, one rollback.

Usage:
  kpm_hotswap.py --mod <id|src dir|KPM folder>...              dry run: kpm_status verdict + the action each mod needs
  kpm_hotswap.py --mod ... --prepare                          mirror production files to KPM and rebuild with create-patch.py
  kpm_hotswap.py --mod ... [--prepare] --apply                swap the DLLs inside the kpatches into the live game (game closed)
  kpm_hotswap.py --rollback latest|<backup dir>               restore the DLLs of a swap (config only if it is unchanged)
  kpm_hotswap.py --list-backups
Options: --game-dir D (default $KOTOR2_ROOT or kpm_status.GAME_DEFAULT), --kpm-dir K (default $KPM_DIR or kpm_status.KPM_DEFAULT).
Exit codes: 0 ok, 1 refused or failed, 2 usage.

kpm_status.py is the single gate. Only SWAP mods are swapped; LIVE is skipped; NOT-LIVE, DIRTY, REINSTALL and LIVE-OTHER
refuse the whole batch before anything is written. MIRROR and REBUILD need --prepare. This script never commits: it prints the
exact commit commands. It never repacks a kpatch (create-patch.py packs all targets); the swap source is the DLL inside it.
"""
import argparse
import datetime
import difflib
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import tomllib
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import kpm_status as ks  # noqa: E402
import _static_common as sc  # noqa: E402
import _load_limit as ll  # noqa: E402
try:
    import master_ledger as ml
except ImportError:
    ml = None

REPO = ks.REPO
MODS = ks.MODS
GAME = Path(os.environ.get("KOTOR2_ROOT") or ks.GAME_DEFAULT)
KPM = Path(os.environ.get("KPM_DIR") or ks.KPM_DEFAULT)
DLL_IN_KPATCH = "binaries/windows_x86.dll"
CONFIG_FILES = ("patch_config.toml", "kpm_install_state.json")
MANIFEST_NAME = "swap_manifest.json"
HARD_REFUSE = ("NOT-LIVE", "DIRTY", "REINSTALL", "LIVE-OTHER")
REINSTALL_DOC = "patch_manager_mods/BUILD_AND_INSTALL.md (kotor2-kpm-install skill)"
GATE_RECORDS = ll.TMP / "gate-records"   # pre_install_gate.py writes <mod_id>.json here on GATE PASS


# ---------------------------------------------------------------- small helpers

def full_md5(data):
    return hashlib.md5(data).hexdigest()


def file_md5(path):
    return full_md5(Path(path).read_bytes()) if Path(path).is_file() else ""


def is_game_running():
    return subprocess.run(["pgrep", "-f", r"^S:.*swkotor2\.exe"], stdout=subprocess.DEVNULL).returncode == 0


def git_head(directory):
    try:
        p = subprocess.run(["git", "-C", str(directory), "rev-parse", "--short", "HEAD"], capture_output=True, text=True)
        return p.stdout.strip() or "unknown"
    except OSError:
        return "unknown"


def git_pending(directory, pathspec):
    """(untracked, modified) paths relative to `directory` under `pathspec`, ignoring *.bak and *.kpatch."""
    r = subprocess.run(["git", "-C", str(directory), "status", "--porcelain", "-uall", "--", pathspec], capture_output=True, text=True)
    untracked, modified = [], []
    for line in r.stdout.splitlines():
        path = line[3:].strip().strip('"')
        if path.endswith((".bak", ".kpatch")):
            continue
        (untracked if line.startswith("??") else modified).append(path)
    return untracked, modified


def pe_exports(data):
    """Export names of a PE32 DLL (raises ValueError if it is not one)."""
    return sorted(sc.PeImage(data=data).exports())


def pe_imports(data):
    """Lower-case imported DLL names of a PE32 DLL (raises ValueError if it is not one)."""
    pe = sc.PeImage(data=data)
    if len(pe._dirs) < 2 or pe._dirs[1][0] == 0:
        return []
    off = pe.va_to_offset(pe.image_base + pe._dirs[1][0])
    if off is None:
        raise ValueError("import directory outside the image")
    names = set()
    while off + 20 <= len(pe.data):
        name_rva, = struct.unpack_from("<I", pe.data, off + 12)
        if name_rva == 0:
            break
        no = pe.va_to_offset(pe.image_base + name_rva)
        if no is None:
            raise ValueError("import name outside the image")
        names.add(pe.data[no:pe.data.index(b"\0", no)].decode("ascii", "replace").lower())
        off += 20
    return sorted(names)


def write_dll(dst, data):
    """Write next to the target, then rename over it, so a crash never leaves a half-written DLL."""
    tmp = dst.with_name(dst.name + ".kptmp")
    tmp.write_bytes(data)
    os.replace(tmp, dst)


def kpatch_dll(kpatch):
    with zipfile.ZipFile(kpatch) as z:
        return z.read(DLL_IN_KPATCH)


# ---------------------------------------------------------------- resolution and status

def canon(s):
    s = re.sub(r"_src$", "", Path(str(s).rstrip("/")).name.lower())
    return re.sub(r"[^a-z0-9]", "", s)


def pascal_dir(pid):
    return pid.replace("_", "-").title().replace("-", "")


def canonical_kpm_dir(pid, kpm):
    """Existing Patches/<X> whose canon() equals canon(pid) (any casing), else Patches/<PascalCase>. ValueError if the
    chosen dir already holds a manifest for a different patch id."""
    patches = kpm / "Patches"
    hits = sorted(d for d in patches.iterdir() if d.is_dir() and canon(d.name) == canon(pid)) if patches.is_dir() else []
    kd = hits[0] if hits else patches / pascal_dir(pid)
    mf = kd / "manifest.toml"
    if mf.is_file():
        try:
            with open(mf, "rb") as f:
                other = tomllib.load(f)["patch"]["id"]
        except (KeyError, tomllib.TOMLDecodeError, OSError):
            other = None
        if other not in (None, pid):
            raise ValueError(f"{kd} holds patch id '{other}', not '{pid}'")
    return kd


def load_kmap(kpm):
    """ks.kpm_dirs, or None after printing the duplicate-id FATAL to stderr."""
    try:
        return ks.kpm_dirs(kpm)
    except ks.DuplicateKpmId as e:
        print(str(e), file=sys.stderr)
        return None


def load_live(game):
    cfg = game / "patch_config.toml"
    return {p["id"]: p for p in tomllib.load(open(cfg, "rb")).get("patches", [])}


def _unknown_mod(q, entries):
    by_key = {k: e[0] for e in entries for k in e[2]}
    msg = f"unknown mod '{q}'"
    close = difflib.get_close_matches(canon(q), list(by_key), n=1, cutoff=0.6)
    if close:
        msg += f"\nDid you mean: '{by_key[close[0]]}'?"
    return msg + f"\nAvailable mod IDs: {', '.join(sorted({e[0] for e in entries}))}"


def resolve(queries, kmap):
    """query -> manifest path of the repo mod, by patch id, src dir name, relative dir (x_src/sub), parent dir name or KPM folder
    name. Raises ValueError if unknown, with a closest-id suggestion."""
    entries = []
    for m in ks.pick_sources(ks.repo_manifests(), kmap):
        pid = tomllib.load(open(m, "rb"))["patch"]["id"]
        keys = {canon(pid), canon(m.parent.name)}
        if pid in kmap:
            keys.add(canon(kmap[pid].name))
        rel = str(m.parent.relative_to(ks.MODS)).replace("\\", "/").lower()
        entries.append((pid, m, keys, rel))
    out = {}
    for q in queries:
        qrel = q.replace("\\", "/").rstrip("/").lower()
        hit = [e for e in entries if canon(q) in e[2] or qrel == e[3]]
        if not hit:
            raise ValueError(_unknown_mod(q, entries))
        exact = [e for e in hit if e[3] == qrel]  # several pids can share the parent alias (every nested 'probe'): exact dir wins
        pick = (exact or hit)[0]
        out[pick[0]] = pick[1]
    return out


def statuses(manifests, kmap, live, game):
    return {pid: ks.status(m, kmap, live, game) for pid, m in manifests.items()}


def live_dll_path(game, live, pid):
    rel = live[pid].get("dll")
    return game / rel if rel else None


# ---------------------------------------------------------------- prepare: mirror + build

MIRROR_SKIP_DIRS = {"__pycache__", "test", "tests", "testdata", "stubs"}   # stubs = host-test shims (e.g. enemy_effects_strip_src/stubs/windows.h), never part of the DLL


def mirror(src, kd):
    """Copy the production files that differ, recursing into nested asset trees (override/ etc.).
    Skips hidden dirs, test-harness dirs and nested mods (a subdir with its own manifest.toml). No --sync, never Patches/Common."""
    copied = []
    for root, dirs, files in os.walk(src):
        dirs[:] = sorted(d for d in dirs if not d.startswith(".") and d not in MIRROR_SKIP_DIRS
                         and not (Path(root) / d / "manifest.toml").exists())
        for name in sorted(files):
            if not ks.is_production(name):
                continue
            f = Path(root) / name
            rel = f.relative_to(src)
            dst = kd / rel
            if not dst.exists() or dst.read_bytes() != f.read_bytes():
                dst.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(f, dst)
                copied.append(rel.as_posix())
    return copied


def address_db_dir(kpm, scratch):
    """A dir of symlinks to the non-empty address DBs (create-patch.py chokes on the 0-byte placeholders)."""
    scratch.mkdir(parents=True, exist_ok=True)
    for db in sorted((kpm / "AddressDatabases").glob("*.db")):
        if db.stat().st_size > 0:
            (scratch / db.name).symlink_to(db)
    return scratch


def run_create_patch(kd, kpm):
    """create-patch.py is the only builder. Returns (rc, tail of output)."""
    with tempfile.TemporaryDirectory(prefix="kpm_dbs_") as t:
        env = dict(os.environ, KPATCH_ADDRESS_DATABASES=str(address_db_dir(kpm, Path(t) / "dbs")))
        # create-patch.py: its common-library cache is not safe against a cold cache run concurrently, and it starts a
        # default-sized thread pool, so one run at a time machine-wide.
        with ll.exclusive("create-patch", "KPM create-patch.py"):
            p = subprocess.run([sys.executable, str(kpm / "Patches/create-patch.py")], cwd=kd, env=env,
                               capture_output=True, text=True)
    return p.returncode, "\n".join((p.stdout + p.stderr).splitlines()[-15:])


def prepare(pid, row, src, kpm, tag):
    """Mirror (if MIRROR) and rebuild. Returns (ok, message, copied files)."""
    kd = Path(row["kpm"]) if row.get("kpm") else None
    if kd is None:
        try:
            kd = canonical_kpm_dir(pid, kpm)
        except ValueError as e:
            return False, f"{pid}: {e}", []
        if not kd.exists():
            kd.mkdir(parents=True)
            print(f"created {kd}")
    copied = mirror(src, kd) if row["verdict"] == "MIRROR" else []
    old = sorted(kd.glob("*.kpatch"))
    if old:
        shutil.copy2(old[0], kd / f"{old[0].name}.{tag}_pre.bak")
    rc, tail = run_create_patch(kd, kpm)
    if rc != 0:
        return False, f"{pid}: create-patch.py failed (rc={rc}); old kpatch kept as .{tag}_pre.bak\n{tail}", copied
    return True, "", copied


# ---------------------------------------------------------------- gate cross-check (report only, never fatal)

def _pe_layout(b):
    """(e_lfanew, optional-header offset, section-table offset, section count) of a PE32 image; ValueError if not one."""
    if len(b) < 0x40 or b[:2] != b"MZ":
        raise ValueError("not an MZ image")
    e, = struct.unpack_from("<I", b, 0x3C)
    if e + 24 + 96 > len(b) or b[e:e + 4] != b"PE\0\0":
        raise ValueError("no PE header")
    nsec, = struct.unpack_from("<H", b, e + 6)
    opt_size, = struct.unpack_from("<H", b, e + 20)
    o = e + 24
    if struct.unpack_from("<H", b, o)[0] != 0x10B:
        raise ValueError("not PE32")
    return e, o, o + opt_size, nsec


def _rva_to_off(b, so, nsec, rva):
    for i in range(nsec):
        _n, vs, va, rs, rp = struct.unpack_from("<8sIIII", b, so + i * 40)
        if va <= rva < va + max(vs, rs):
            return rp + rva - va
    return None


def normalize_pe(data):
    """(normalised bytes, ImageBase) of a PE32 DLL, so two builds of the same sources compare equal.

    Zeroed: COFF TimeDateStamp (e_lfanew+8), optional-header CheckSum (e_lfanew+24+64), and, because MinGW's ld picks a
    random ImageBase per link (dynamicbase), the ImageBase itself, the export directory TimeDateStamp, and the rebase
    of every HIGHLOW entry of the .reloc table (so absolute pointers read as base-relative). Not a PE: unchanged, base 0.
    """
    b = bytearray(data)
    try:
        e, o, so, nsec = _pe_layout(b)
        base, = struct.unpack_from("<I", b, o + 28)
        b[e + 8:e + 12] = bytes(4)
        b[o + 64:o + 68] = bytes(4)
        b[o + 28:o + 32] = bytes(4)
        ndirs, = struct.unpack_from("<I", b, o + 92)
        if ndirs > 0:
            rva, = struct.unpack_from("<I", b, o + 96)
            off = _rva_to_off(b, so, nsec, rva) if rva else None
            if off is not None:
                b[off + 4:off + 8] = bytes(4)
        if ndirs > 5:
            rva, size = struct.unpack_from("<II", b, o + 96 + 5 * 8)
            off = _rva_to_off(b, so, nsec, rva) if rva else None
            end = off + size if off is not None else 0
            while off is not None and off + 8 <= end:
                page, blk = struct.unpack_from("<II", b, off)
                if blk < 8:
                    break
                for k in range(off + 8, min(off + blk, end), 2):
                    w, = struct.unpack_from("<H", b, k)
                    if w >> 12 == 3:   # IMAGE_REL_BASED_HIGHLOW
                        t = _rva_to_off(b, so, nsec, page + (w & 0xFFF))
                        if t is not None and t + 4 <= len(b):
                            struct.pack_into("<I", b, t, (struct.unpack_from("<I", b, t)[0] - base) & 0xFFFFFFFF)
                off += blk
        return bytes(b), base
    except (ValueError, struct.error):
        return bytes(data), 0


def normalized_md5(data):
    """md5 of normalize_pe(data). Equal for builds that differ only in the PE header stamps / rebased pointers."""
    return full_md5(normalize_pe(data)[0])


def dlls_equivalent(a, b):
    """True if two builds of the same sources are the same code. After normalize_pe, a rel32 call/jmp to a fixed address
    outside the DLL (the game exe) still differs by exactly the difference of the two ImageBases; those 4-byte windows
    are accepted, any other difference is not."""
    (na, ba), (nb, bb) = normalize_pe(a), normalize_pe(b)
    if na == nb:
        return True
    if len(na) != len(nb) or ba == bb:
        return False
    delta = (bb - ba) & 0xFFFFFFFF
    i, n = 0, len(na)
    while i < n:
        if na[i] == nb[i]:
            i += 1
            continue
        for s in range(max(i - 3, 0), i + 1):   # the window must cover i
            if s + 4 <= n and (struct.unpack_from("<I", na, s)[0] - struct.unpack_from("<I", nb, s)[0]) & 0xFFFFFFFF == delta:
                i = s + 4
                break
        else:
            return False
    return True


def _record_time(v):
    """Epoch seconds from a gate record "time" (epoch number or ISO-8601 string); None if unreadable."""
    if isinstance(v, (int, float)):
        return float(v)
    try:
        return float(v)
    except (TypeError, ValueError):
        pass
    try:
        return datetime.datetime.fromisoformat(str(v)).timestamp()
    except ValueError:
        return None


def newest_source(src):
    """(mtime, relative path) of the newest production file in a mod source dir (same walk as mirror()), or (0, '')."""
    best = (0.0, "")
    for root, dirs, files in os.walk(src):
        dirs[:] = sorted(d for d in dirs if not d.startswith(".") and d not in MIRROR_SKIP_DIRS
                         and not (Path(root) / d / "manifest.toml").exists())
        for name in files:
            if ks.is_production(name):
                f = Path(root) / name
                try:
                    t = f.stat().st_mtime
                except OSError:
                    continue
                if t > best[0]:
                    best = (t, f.relative_to(src).as_posix())
    return best


def gate_cross_check(pid, kd, src, records=None):
    """Compare the DLL in the freshly rebuilt kpatch with the one pre_install_gate.py built and passed. Returns the
    report lines (GATE / WARN / NOTE); never raises, never changes the exit code."""
    try:
        rec_path = Path(records if records is not None else GATE_RECORDS) / f"{pid}.json"
        if not rec_path.is_file():
            return [f"NOTE  {pid}: no gate record"]
        rec = json.loads(rec_path.read_text())
        kps = sorted(Path(kd).glob("*.kpatch")) if kd else []
        if not kps:
            return [f"NOTE  {pid}: no kpatch to compare with the gate record"]
        new = kpatch_dll(kps[0])
        want, head = str(rec.get("md5", "")).lower(), rec.get("head", "unknown")
        lines, same, gone = [], False, False
        if want in (full_md5(new), normalized_md5(new)):
            same = True
        elif rec.get("dll") and Path(rec["dll"]).is_file():
            same = dlls_equivalent(Path(rec["dll"]).read_bytes(), new)
        else:
            gone = True
        if same:
            lines.append(f"GATE  {pid}: fresh DLL matches the gated build (gate head {head})")
        elif gone:
            lines.append(f"NOTE  {pid}: gate DLL {rec.get('dll') or '(none recorded)'} is gone; only the raw md5 could be "
                         f"compared and it differs (gate {want[:12]} at {head}; ImageBase is randomised per link)")
        else:
            lines.append(f"WARN  {pid}: rebuilt DLL differs from the gated build (gate {want} at {head}); re-run pre_install_gate")
        rt = _record_time(rec.get("time"))
        newest, name = newest_source(src)
        if rt is not None and newest > rt:
            lines.append(f"WARN  {pid}: gate record is older than the source ({name}); stale gate, re-run pre_install_gate")
        return lines
    except Exception as e:   # report-only: a broken record must never fail a prepare
        return [f"NOTE  {pid}: gate cross-check skipped ({type(e).__name__}: {e})"]


# ---------------------------------------------------------------- swap

def check_new_dll(pid, new, live_bytes):
    """Fail-closed PE checks. Returns an error string or ''."""
    try:
        new_imp, live_imp = set(pe_imports(new)), set(pe_imports(live_bytes))
        new_exp, live_exp = set(pe_exports(new)), set(pe_exports(live_bytes))
    except (ValueError, struct.error) as e:
        return f"{pid}: cannot parse a DLL as PE32 ({e})"
    if new_imp - live_imp:
        return f"{pid}: new DLL dependency {', '.join(sorted(new_imp - live_imp))} (needs a full reinstall, not a swap)"
    if new_exp != live_exp:
        return (f"{pid}: exports differ from the live DLL (+{','.join(sorted(new_exp - live_exp)) or '-'} "
                f"-{','.join(sorted(live_exp - new_exp)) or '-'}); needs a full reinstall")
    return ""


def unique_dir(parent, name):
    d, n = parent / name, 1
    while d.exists():
        n += 1
        d = parent / f"{name}_{n}"
    d.mkdir(parents=True)
    return d


def swap_batch(rows, manifests, kmap, game, kpm, ts):
    """rows: {pid: status row} that are all SWAP. Atomic: any failure restores the already swapped DLLs. Returns (rc, record)."""
    live = load_live(game)
    plan = {}
    for pid, row in rows.items():                      # 1. pre-extract and verify everything before touching the game dir
        dll = live_dll_path(game, live, pid)
        if dll is None or not dll.is_file():
            print(f"REFUSED {pid}: live DLL missing or patch_config.toml has no dll path (needs a full install)")
            return 1, None
        kps = sorted(Path(row["kpm"]).glob("*.kpatch"))
        new = kpatch_dll(kps[0])
        err = check_new_dll(pid, new, dll.read_bytes())
        if err:
            print("REFUSED " + err)
            return 1, None
        plan[pid] = {"dll": dll, "rel": str(dll.relative_to(game)), "new": new, "old_md5": file_md5(dll), "new_md5": full_md5(new)}

    label = next(iter(plan)) if len(plan) == 1 else "batch"
    backup = unique_dir(game / "backups", f"{label}_swap_{ts}")
    cfg_md5 = {n: file_md5(game / n) for n in CONFIG_FILES}
    for n in CONFIG_FILES:
        if (game / n).is_file():
            shutil.copy2(game / n, backup / n)
    meta = {"timestamp": ts, "repo_head": git_head(REPO), "kpm_head": git_head(kpm), "config_md5": cfg_md5, "mods": {}}
    for pid, p in plan.items():
        shutil.copy2(p["dll"], backup / f"{pid}.dll")
        meta["mods"][pid] = {"dll": p["rel"], "backup_file": f"{pid}.dll", "old_md5": p["old_md5"], "new_md5": p["new_md5"]}
    (backup / MANIFEST_NAME).write_text(json.dumps(meta, indent=2))

    done = []

    def undo(why):
        for pid in done:
            write_dll(plan[pid]["dll"], (backup / f"{pid}.dll").read_bytes())
        print(f"FAILED: {why}; restored {len(done)} swapped DLL(s) from {backup}")
        return 1, None

    try:
        for pid, p in plan.items():                    # 2. swap
            write_dll(p["dll"], p["new"])
            done.append(pid)
            if file_md5(p["dll"]) != p["new_md5"]:
                return undo(f"{pid}: written DLL md5 != kpatch DLL md5")
    except OSError as e:
        return undo(f"write error: {e}")
    for n in CONFIG_FILES:                             # 3. config must not have moved
        if file_md5(game / n) != cfg_md5[n]:
            return undo(f"{n} changed during the swap")
    after = statuses({pid: manifests[pid] for pid in plan}, kmap, load_live(game), game)
    bad = {pid: r["verdict"] for pid, r in after.items() if r["verdict"] != "LIVE"}
    if bad:
        return undo(f"post-swap status is not LIVE: {bad}")
    return 0, {"backup": backup, "meta": meta, "plan": plan}


# ---------------------------------------------------------------- rollback

def swap_dirs(game):
    return sorted((game / "backups").glob("*_swap_*"), key=lambda d: d.stat().st_mtime, reverse=True)


def rollback(target, game):
    if is_game_running():
        print("REFUSED: KOTOR II is running; close swkotor2.exe first")
        return 1
    if target == "latest":
        dirs = swap_dirs(game)
        if not dirs:
            print(f"REFUSED: no *_swap_* backups in {game / 'backups'}")
            return 1
        snap = dirs[0]
    else:
        snap = Path(target)
        if not snap.is_dir():
            snap = game / "backups" / target
        if not snap.is_dir():
            print(f"REFUSED: backup dir '{target}' does not exist")
            return 1
    mf = snap / MANIFEST_NAME
    if not mf.is_file():
        print(f"REFUSED: {snap} has no {MANIFEST_NAME} (old-style backup): copy its DLLs by hand")
        return 1
    meta = json.loads(mf.read_text())
    todo = []
    for pid, m in meta["mods"].items():
        src = snap / m["backup_file"]
        if not src.is_file():
            print(f"REFUSED: backup file {src} is missing; nothing restored")
            return 1
        todo.append((pid, src, game / m["dll"], m["old_md5"]))
    print(f"Rolling back {snap}")
    for pid, src, dst, old in todo:
        write_dll(dst, src.read_bytes())
        got = file_md5(dst)
        print(f"  {pid}: {dst.name} -> {got[:8]}{'' if got == old else '  (MD5 MISMATCH vs snapshot)'}")
    cfg = meta.get("config_md5", {})
    if all(file_md5(game / n) == cfg.get(n, "") for n in CONFIG_FILES):
        for n in CONFIG_FILES:
            if (snap / n).is_file():
                shutil.copy2(snap / n, game / n)
        print("  config files restored (unchanged since the snapshot)")
    else:
        print("  WARNING: patch_config.toml / kpm_install_state.json changed since the snapshot (a KPatchLauncher run?); "
              "config NOT restored, only the DLLs were. Check `python3 scripts/kpm_status.py`.")
    if ml:
        ml.log_event("HOTSWAP_ROLLBACK", {
            "backup": snap.name,
            "mods": [p[0] for p in todo],
        })
    return 0


def list_backups(game):
    dirs = swap_dirs(game)
    if not dirs:
        print(f"no *_swap_* backups in {game / 'backups'}")
    for d in dirs[:10]:
        mf = d / MANIFEST_NAME
        ids = ",".join(json.loads(mf.read_text())["mods"]) if mf.is_file() else "(no manifest: old-style)"
        print(f"{d.name}  {ids}")
    return 0


# ---------------------------------------------------------------- reporting

def action_for(row):
    v = row["verdict"]
    return {
        "LIVE": "nothing to do",
        "SWAP": "ready: --apply",
        "MIRROR": "needs --prepare (mirror production files, rebuild)",
        "REBUILD": "needs --prepare (rebuild with create-patch.py)",
        "NOT-LIVE": "refused: id is not in the live patch_config.toml (full install)",
        "DIRTY": f"refused: commit first: git -C {REPO} commit -- {row['src']}",
        "REINSTALL": f"refused: hooks changed, no hot-swap; full reinstall, see {REINSTALL_DOC}",
        "LIVE-OTHER": "refused: live DLL is a newer build of the same source; rebuild/stage on purpose before swapping",
    }[v]


def print_commit_commands(rows, kpm):
    print("\nCommit commands (run them yourself; --commit-kpm commits the KPM side for you):")
    for pid, row in rows.items():
        src = REPO / row["src"] if not Path(row["src"]).is_absolute() else Path(row["src"])
        try:
            rel_src = str(src.relative_to(REPO))
        except ValueError:
            rel_src = str(src)
        untr, mod = git_pending(REPO, rel_src)
        if mod or untr:
            print(f"  repo {pid}: git -C {REPO} commit -m '<msg>' -- {rel_src}   (pending: {', '.join((mod + untr)[:4])})")
        else:
            print(f"  repo {pid}: clean at {git_head(REPO)}, nothing to commit")
        if row.get("kpm"):
            kd = Path(row["kpm"])
            try:
                pathspec = str(kd.relative_to(kpm))
            except ValueError:
                continue
            untr, mod = git_pending(kpm, pathspec)
            paths = " ".join(f"'{p}'" for p in untr + mod)
            if paths:
                if untr:
                    print(f"  kpm  {pid}: git -C {kpm} add -- {' '.join(repr(p) for p in untr)}")
                print(f"  kpm  {pid}: git -C {kpm} commit -m 'Patches/{kd.name}: {pid} {row['version']} (repo {git_head(REPO)})' -- {paths}")
            else:
                print(f"  kpm  {pid}: clean at {git_head(kpm)}, nothing to commit")


def commit_kpm(pids, rows, kpm):
    """--commit-kpm: commit the mirrored files of each prepared mod in $KPM (explicit paths, never -a). Returns False on any git failure."""
    for pid in pids:
        row = rows[pid]
        if not row.get("kpm"):
            continue
        kd = Path(row["kpm"])
        try:
            pathspec = str(kd.relative_to(kpm))
        except ValueError:
            continue
        untr, mod = git_pending(kpm, pathspec)
        paths = untr + mod
        if not paths:
            print(f"  kpm  {pid}: clean at {git_head(kpm)}, nothing to commit")
            continue
        msg = f"Patches/{kd.name}: {pid} {row['version']} (repo {git_head(REPO)})"
        steps = ([["git", "-C", str(kpm), "add", "--"] + untr] if untr else []) + [["git", "-C", str(kpm), "commit", "-m", msg, "--"] + paths]
        for cmd in steps:
            p = subprocess.run(cmd, capture_output=True, text=True)
            if p.returncode != 0:
                print(f"FAILED kpm commit for {pid} (rc={p.returncode}): {(p.stdout + p.stderr).strip()[-400:]}")
                return False
        print(f"  kpm  {pid}: committed {git_head(kpm)}")
    return True


# ---------------------------------------------------------------- main

def main(argv=None):
    ll.lower_priority()
    ap = argparse.ArgumentParser(description="Batched KPM DLL hot-swap with backup and rollback (kpm_status.py is the gate).")
    ap.add_argument("--mod", nargs="+", action="extend", default=[], metavar="MOD", help="patch id, src dir name or KPM folder name")
    ap.add_argument("--prepare", action="store_true", help="mirror production files to KPM and rebuild MIRROR/REBUILD mods")
    ap.add_argument("--apply", action="store_true", help="swap the DLLs from the kpatches into the live game (game must be closed)")
    ap.add_argument("--commit-kpm", action="store_true", help="with --prepare: commit the mirrored files in the KPM checkout (explicit paths); abort before --apply if the commit fails")
    ap.add_argument("--rollback", metavar="latest|DIR", help="restore the DLLs of a swap backup")
    ap.add_argument("--list-backups", action="store_true", help="list *_swap_* backups, newest first")
    ap.add_argument("--game-dir", type=Path, default=GAME, help=f"default {GAME}")
    ap.add_argument("--kpm-dir", type=Path, default=KPM, help=f"default {KPM}")
    try:
        a = ap.parse_args(argv)
    except SystemExit as e:
        return int(e.code or 0)
    game, kpm = a.game_dir.expanduser(), a.kpm_dir.expanduser()
    if a.list_backups:
        return list_backups(game)
    if a.rollback:
        return rollback(a.rollback, game)
    if not a.mod:
        print("usage error: --mod, --rollback or --list-backups is required", file=sys.stderr)
        return 2
    if a.commit_kpm and not a.prepare:
        print("usage error: --commit-kpm needs --prepare", file=sys.stderr)
        return 2
    if not (game / "patch_config.toml").is_file():
        print(f"no {game / 'patch_config.toml'}", file=sys.stderr)
        return 2
    live, kmap = load_live(game), load_kmap(kpm)
    if kmap is None:
        return 2
    try:
        manifests = resolve(a.mod, kmap)
    except ValueError as e:
        print(f"usage error: {e}", file=sys.stderr)
        return 2
    rows = statuses(manifests, kmap, live, game)
    for pid, r in rows.items():
        print(f"{r['verdict']:<10} {pid:<24} {r['version']:<12} {(r['live'] or '-')}/{(r['kpatch'] or '-')}  {action_for(r)}"
              + (f"  [{r['detail']}]" if r["detail"] and r["verdict"] not in ("LIVE",) else ""))
    # --prepare alone may pass a REINSTALL mod: it only mirrors and rebuilds, and --apply still refuses REINSTALL below
    prep_ok = ("REINSTALL",) if (a.prepare and not a.apply) else ()
    refused = [pid for pid, r in rows.items() if r["verdict"] in HARD_REFUSE and r["verdict"] not in prep_ok]
    if refused:
        print(f"REFUSED whole batch: {' '.join(refused)}")
        return 1
    if not (a.prepare or a.apply):
        return 0
    todo = {pid: r for pid, r in rows.items() if r["verdict"] != "LIVE"}
    if not todo:
        print("nothing to do")
        return 0
    needs_prep = [pid for pid, r in todo.items() if r["verdict"] in ("MIRROR", "REBUILD")]
    if needs_prep and not a.prepare:
        print(f"REFUSED: {' '.join(needs_prep)} need --prepare first")
        return 1
    if a.apply and is_game_running():
        print("REFUSED: KOTOR II is running; close swkotor2.exe first")
        return 1

    ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    if needs_prep:
        for pid in needs_prep:
            ok, msg, copied = prepare(pid, rows[pid], manifests[pid].parent, kpm, ts)
            if not ok:
                print("FAILED " + msg)
                return 1
            print(f"prepared {pid}: mirrored {', '.join(copied) or 'nothing'}; rebuilt")
        kmap = load_kmap(kpm)   # --prepare may have created Patches/<X> dirs
        if kmap is None:
            return 2
        rows = statuses(manifests, kmap, live, game)
        todo = {pid: r for pid, r in rows.items() if r["verdict"] != "LIVE"}
        swappable = [pid for pid, r in todo.items() if r["verdict"] == "SWAP"]
        reinstall = [pid for pid, r in todo.items() if r["verdict"] == "REINSTALL"]
        bad = {pid: f"{r['verdict']} {r['detail']}" for pid, r in todo.items() if r["verdict"] not in ("SWAP", "REINSTALL")}
        if bad or (reinstall and a.apply):   # other verdicts always fail; REINSTALL fails only when --apply was asked for
            bad.update({pid: f"{todo[pid]['verdict']} {todo[pid]['detail']}" for pid in reinstall})
            print(f"FAILED: after --prepare these are not SWAP: {bad}"
                  + ("  (REINSTALL = hooks changed: " + REINSTALL_DOC + ")" if reinstall else ""))
            return 1
        print("after prepare: " + " ".join(f"{pid}={r['verdict']}" for pid, r in rows.items()))
        print(f"PREPARE SUCCESS: {len(needs_prep)} mod(s) mirrored and rebuilt.")
        for pid in needs_prep:   # report only: is the rebuilt DLL the one pre_install_gate passed?
            for line in gate_cross_check(pid, rows[pid].get("kpm"), manifests[pid].parent):
                print(line)
        if swappable:
            print(f"  Ready for --apply (hot-swap): {', '.join(swappable)}")
        if reinstall:
            print(f"  Requires KPatchLauncher reinstall (hooks changed): {', '.join(reinstall)}  ({REINSTALL_DOC})")
        if a.commit_kpm and not commit_kpm(needs_prep, rows, kpm):
            return 1
    if not a.apply:
        print_commit_commands(rows, kpm)
        if any(r["verdict"] == "SWAP" for r in todo.values()):
            print("\nNext: re-run with --apply (game closed)")
        return 0
    if not todo:
        print("nothing to swap after prepare")
        print_commit_commands(rows, kpm)
        return 0

    rc, rec = swap_batch(todo, manifests, kmap, game, kpm, ts)
    if rc:
        return rc
    if ml:
        ml.log_event("HOTSWAP_APPLY", {
            "mods": list(rec["plan"].keys()),
            "backup": rec["backup"].name,
            "repo_head": rec["meta"].get("repo_head", ""),
            "kpm_head": rec["meta"].get("kpm_head", ""),
        })
    print(f"\nSwapped {len(rec['plan'])} DLL(s); all LIVE; patch_config.toml and kpm_install_state.json unchanged")
    print(f"{'id':<24} {'old md5 -> new md5':<22} DLL")
    for pid, p in rec["plan"].items():
        print(f"{pid:<24} {p['old_md5'][:8]} -> {p['new_md5'][:8]}    {p['rel']}")
    print(f"backup: {rec['backup']}\nrepo HEAD {rec['meta']['repo_head']} | KPM HEAD {rec['meta']['kpm_head']}")
    print_commit_commands(todo, kpm)
    print(f"\nRollback: python3 scripts/kpm_hotswap.py --rollback {rec['backup'].name}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
