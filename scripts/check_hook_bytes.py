#!/usr/bin/env python3
"""Static check of every mod hook and code patch against the pristine vanilla exe (doc 20 section 3, T0).

  python3 scripts/check_hook_bytes.py                      # all mods + their KPM .kpatch packaging
  python3 scripts/check_hook_bytes.py --mod level-up-override --quiet
  python3 scripts/check_hook_bytes.py --kpatch /path/Staged.kpatch   # also check a staged build (repeatable)

For each hook (patch_manager_mods/**/kotor2-steam-aspyr.hooks.toml) and each code patch (code_patches.toml sidecar):
  FAIL  original_bytes differ from the vanilla exe, or the span is not backed by file data
  FAIL  span not inside .text (kind="code") / .data or .rdata (kind="data"), or straddling two sections
  FAIL  the span does not end on an instruction boundary (disassembled [addr, addr+len+16); data patches exempt)
  FAIL  detour: a relative call/jmp/jcc/loop/jecxz with an immediate target inside the stolen bytes while
        skip_original_bytes is false (KPM copies the bytes into the trampoline unrelocated); INFO when it is true
  FAIL  simple hook: replacement_bytes length != original_bytes length
GOG (every patch_manager_mods/**/kotor2-gog-aspyr.hooks.toml): the GOG exe is not kept locally, so the sites are checked against the
GOG instruction dumps of doc 21b (patch_manager_mods/gog_map_21b/dumps/gog_r*.txt, a sparse image of every hook-host function):
  FAIL  target_versions != the GOG sha / manifest supported_versions lacks kotor2_gog_aspyr = GOG sha
  FAIL  function / type / original_bytes / parameter sources / skip_original_bytes differ from the Steam hooks file (same order)
  FAIL  original_bytes differ from the dump bytes, or the window does not start/end on a dumped instruction boundary
  FAIL  detour: a relative call/jmp/jcc inside the stolen window while skip_original_bytes is false
  SKIP  a site whose address is not covered by any dump (extend the dumps; not a failure)
Packaging (every Patches/<Name>/*.kpatch of our mods, plus --kpatch files):
  FAIL  missing manifest.toml / hooks toml / binaries/windows_x86.dll (DLL only expected if the mod has one)
  WARN  embedded hooks (compared parsed; comments and extra target_versions such as the LAA exe are fine) or manifest
        version differ from the patch_manager_mods source ("stale .kpatch")
Exit 1 on any FAIL. A missing or different exe is SKIP (exit 0) / FAIL. Needs i686-w64-mingw32-objdump.
"""
import argparse
import re
import shutil
import sys
import tomllib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402

CODE_SECTIONS = (".text",)
DATA_SECTIONS = (".data", ".rdata")


def _hx(b: bytes, limit=16) -> str:
    s = b[:limit].hex(" ")
    return s + (" ..." if len(b) > limit else "")


def check_site(rep, pe, label, address, orig, kind="code", type_="code_patch", skip_original=False,
               replacement=None):
    """Run the per-site checks for one hook or code patch; returns True if it produced no FAIL.

    type_: "detour" | "simple" | "code_patch". Branches are only judged for detours (a trampoline copies the stolen
    bytes); code patches and simple hooks overwrite them wholesale.
    """
    ok = True
    where = f"{label} 0x{address:08X} ({len(orig)} bytes)"
    # bytes equal the vanilla exe
    try:
        got = pe.read_va(address, len(orig))
    except ValueError as e:
        rep.fail("bytes", f"{where}: {e}")
        return False
    if got != bytes(orig):
        rep.fail("bytes", f"{where}: original_bytes [{_hx(orig)}] != vanilla [{_hx(got)}]")
        ok = False
    # address inside the right section
    first, last = pe.section_of(address), pe.section_of(address + len(orig) - 1)
    want = DATA_SECTIONS if kind == "data" else CODE_SECTIONS
    if first is None or first is not last or first.name not in want:
        rep.fail("section", f"{where}: in {first.name if first else 'no section'}"
                 f"{'' if first is last else ' (span straddles two sections)'}, expected {' or '.join(want)}")
        ok = False
    # simple hook replacement length
    if type_ == "simple":
        if replacement is None or len(replacement) != len(orig):
            rep.fail("simple-len", f"{where}: replacement_bytes is {len(replacement) if replacement is not None else 'missing'}"
                     f" bytes, original_bytes {len(orig)}")
            ok = False
    # instruction boundary + relative branches (code only)
    if kind == "code" and got == bytes(orig) and first is not None:
        end = address + len(orig)
        avail = min(len(orig) + 16, first.va + first.raw_size - address)
        insns = sc.disasm(pe.read_va(address, avail), address)
        starts = {i.va for i in insns}
        at_end = avail == len(orig) and insns and insns[-1].end == end
        if end not in starts and not at_end:
            ctx = next((i for i in insns if i.va < end < i.end), None)
            rep.fail("boundary", f"{where}: window ends mid-instruction"
                     + (f" ({ctx.text} at 0x{ctx.va:08X}, {ctx.length} bytes)" if ctx else ""))
            ok = False
        if type_ == "detour":
            br = [(i, sc.branch_target(i)) for i in insns if i.va < end]
            br = [(i, t) for i, t in br if t is not None]
            if br:
                desc = ", ".join(f"{i.text}@0x{i.va:X}" for i, _ in br)
                if skip_original:
                    rep.info("rel-branch", f"{where}: relative branch in stolen bytes (skip_original_bytes=true): {desc}")
                else:
                    rep.fail("rel-branch", f"{where}: relative branch in stolen bytes with skip_original_bytes=false "
                             f"(the trampoline copy would be unrelocated): {desc}")
                    ok = False
    if ok:
        rep.passed("site", where)
    return ok


def _check_mod_sites(rep, pe, mod, label=None, hooks=None, code_patches=None):
    label = label or mod.id
    for h in (mod.hooks if hooks is None else hooks):
        check_site(rep, pe, f"{label} {h.type}", h.address, h.orig, "code", h.type, h.skip_original, h.replacement)
    for cp in (mod.code_patches if code_patches is None else code_patches):
        check_site(rep, pe, f"{label} code_patch {cp.name}", cp.address, cp.orig, cp.kind, "code_patch")


GOG_SHA = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
GOG_HOOKS_TOML = "kotor2-gog-aspyr.hooks.toml"
GOG_DUMPS_REL = "patch_manager_mods/gog_map_21b/dumps"
_INS_RE = re.compile(r"^([0-9a-f]{8})  ((?:[0-9a-f]{2} )+) *(.*)$")


def load_gog_dump_insns(dumps_dir) -> dict:
    """{va: (bytes, text)} from every gog_r*.txt ExportRangeDisasm dump in dumps_dir (instruction lines only)."""
    insns = {}
    for f in sorted(Path(dumps_dir).glob("gog_r*.txt")):
        for line in f.read_text(encoding="utf-8", errors="replace").splitlines():
            m = _INS_RE.match(line)
            if m:
                rest = m.group(3)
                k = rest.find("  ->")
                insns[int(m.group(1), 16)] = (bytes.fromhex(m.group(2).replace(" ", "")), (rest if k < 0 else rest[:k]).strip())
    return insns


def _is_rel_branch_text(text) -> bool:
    mn = text.split(" ", 1)[0].upper()
    return (mn == "CALL" or mn.startswith("J") or mn.startswith("LOOP")) and re.search(r"\b0x[0-9a-f]{8}\b", text) is not None


def check_gog_site(rep, insns, label, h):
    """One GOG hook against the dumped instructions; returns False on FAIL (a site with no dump coverage is SKIP)."""
    where = f"{label} 0x{h.address:08X} ({len(h.orig)} bytes)"
    if h.address not in insns:
        mid = next(((v, t) for v, (b, t) in insns.items() if v < h.address < v + len(b)), None)
        if mid:
            rep.fail("gog-boundary", f"{where}: starts mid-instruction ({mid[1]} at 0x{mid[0]:08X}) in the 21b dumps")
            return False
        rep.skip("gog-bytes", f"{where}: address not an instruction in the 21b dumps (no GOG data)")
        return True
    ok, got, va, covered = True, b"", h.address, []
    while len(got) < len(h.orig):
        if va not in insns:
            rep.fail("gog-bytes", f"{where}: dump has no instruction at 0x{va:08X} (window runs off the dumped range)")
            return False
        b, text = insns[va]
        covered.append((va, text))
        got += b
        va += len(b)
    if got[:len(h.orig)] != h.orig:
        rep.fail("gog-bytes", f"{where}: original_bytes [{_hx(h.orig)}] != GOG dump [{_hx(got[:len(h.orig)])}]")
        ok = False
    if len(got) != len(h.orig):
        rep.fail("gog-boundary", f"{where}: window ends mid-instruction (dump instructions cover {len(got)} bytes)")
        ok = False
    if h.type == "detour":
        br = [(v, t) for v, t in covered if _is_rel_branch_text(t)]
        if br and not h.skip_original:
            rep.fail("gog-rel-branch", f"{where}: relative branch in stolen bytes with skip_original_bytes=false: "
                     + ", ".join(f"{t}@0x{v:X}" for v, t in br))
            ok = False
    if ok:
        rep.passed("gog-site", where)
    return ok


def _hook_sig_doc(path):
    """(target_versions, [(function, type, original_bytes, skip_original, parameter sources)]) of a hooks toml, address-free."""
    doc = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    tv = [str(x).upper() for x in doc.get("metadata", {}).get("target_versions", [])]
    sig = [(h.get("function"), h.get("type", "detour"), sc._to_bytes(h["original_bytes"]), bool(h.get("skip_original_bytes", False)),
            tuple((p.get("source"), p.get("type")) for p in h.get("parameters", []))) for h in doc.get("hooks", [])]
    return tv, sig


def check_gog(rep, repo=sc.REPO, mod_ids=None, dumps_dir=None):
    """GOG hooks files vs the Steam ones (same sites) and vs the 21b GOG dumps; manifests list the GOG sha."""
    root = Path(repo) / "patch_manager_mods"
    dumps_dir = Path(dumps_dir) if dumps_dir else Path(repo) / GOG_DUMPS_REL
    found = sorted(d for d in (p.parent for p in root.rglob(GOG_HOOKS_TOML))
                   if not any(x in sc.SKIP_DIRS for x in d.relative_to(root).parts) and (d / "manifest.toml").is_file())
    if not found:
        return
    insns = load_gog_dump_insns(dumps_dir) if dumps_dir.is_dir() else {}
    if not insns:
        rep.skip("gog-bytes", f"no GOG instruction dumps under {dumps_dir}")
    for d in found:
        pt = tomllib.loads((d / "manifest.toml").read_text(encoding="utf-8")).get("patch", {})
        pid = pt.get("id", d.name)
        if mod_ids and pid not in mod_ids:
            continue
        gv = str(pt.get("supported_versions", {}).get("kotor2_gog_aspyr", "")).upper()
        if gv != GOG_SHA:
            rep.fail("gog-manifest", f"{pid}: [patch.supported_versions] kotor2_gog_aspyr is {gv or 'missing'}, expected {GOG_SHA}")
        gtv, gsig = _hook_sig_doc(d / GOG_HOOKS_TOML)
        if gtv != [GOG_SHA]:
            rep.fail("gog-version", f"{pid}: {GOG_HOOKS_TOML} target_versions {gtv} != [{GOG_SHA}]")
        if (d / sc.HOOKS_TOML).is_file():
            _, ssig = _hook_sig_doc(d / sc.HOOKS_TOML)
            if gsig != ssig:
                rep.fail("gog-parity", f"{pid}: {GOG_HOOKS_TOML} differs from {sc.HOOKS_TOML} in function/type/original_bytes/"
                         f"parameters/skip_original_bytes (or hook count/order): {len(gsig)} vs {len(ssig)} hooks")
        _, ghooks = sc.parse_hooks((d / GOG_HOOKS_TOML).read_bytes(), mod=pid, source=str(d / GOG_HOOKS_TOML))
        if insns:
            for h in ghooks:
                check_gog_site(rep, insns, f"{pid} GOG {h.type}", h)
        if len({h.address for h in ghooks}) != len(ghooks):
            rep.fail("gog-parity", f"{pid}: duplicate hook address in {GOG_HOOKS_TOML}")


def _sig(h):
    return (h.address, h.type, h.function, h.orig, h.skip_original, h.replacement)


def check_packaging(rep, mod, km: "sc.KpatchMembers", name):
    """Members present and embedded hooks toml / version equal to the source `mod`'s."""
    need_dll = mod is None or mod.has_def or mod.code_patches or any(h.type == "detour" for h in mod.hooks)
    missing = [m for m, v in (("manifest.toml", km.manifest), ("*.hooks.toml", km.hooks),
                              ("binaries/windows_x86.dll", km.dll if need_dll else b"")) if v is None]
    if missing:
        rep.fail("packaging", f"{name}: missing {', '.join(missing)} (has {km.names})")
    if mod is None:
        return
    if km.hooks is not None:
        tv, hooks = sc.parse_hooks(km.hooks)
        if [_sig(h) for h in hooks] != [_sig(h) for h in mod.hooks]:
            a, b = {_sig(h) for h in hooks}, {_sig(h) for h in mod.hooks}
            rep.warn("stale-kpatch", f"{name}: embedded {km.hooks_name} holds {len(hooks)} hooks, source {len(mod.hooks)} "
                     f"({len(a - b)} only in the .kpatch, {len(b - a)} only in source)")
        elif not set(mod.target_versions) <= set(tv):
            rep.warn("stale-kpatch", f"{name}: embedded target_versions lacks {sorted(set(mod.target_versions) - set(tv))}")
    if km.manifest is not None:
        pt = km.manifest_doc
        if str(pt.get("version", "")) != mod.version:
            rep.warn("stale-kpatch", f"{name}: manifest version {pt.get('version')!r} != source {mod.version!r}")


def run(repo=sc.REPO, exe=sc.VANILLA_EXE, kpm_dir=sc.KPM_DIR, kpatches=(), mod_ids=None, live_config=sc.LIVE_CONFIG,
        expected_sha=None, mods=None, reporter=None):
    """Run every check; returns the list of Findings (also streamed to `reporter` if given).

    repo/kpm_dir locate the mods (or pass `mods` directly); exe is the vanilla exe; kpatches are extra staged
    .kpatch files; mod_ids filters by patch id; expected_sha defaults to the live config's target_version_sha, else
    the first mod's target_versions[0] (pass live_config=None to ignore the live config).
    """
    rep = reporter or sc.Reporter(out=None)
    mark = len(rep.findings)
    all_mods = mods if mods is not None else sc.load_mods(repo, kpm_dir)
    sel = [m for m in all_mods if not mod_ids or m.id in mod_ids]
    exe = Path(exe)
    if not exe.is_file():
        rep.skip("version", f"vanilla exe {exe} not found")
        return rep.findings[mark:]
    sha = expected_sha or (sc.live_target_sha(live_config) if live_config else None) \
        or next((m.target_versions[0] for m in all_mods if m.target_versions), None)
    if sha and not sc.require_version(exe, sha, rep):
        return rep.findings[mark:]
    pe = sc.PeImage(exe)
    exe_sha = sc.sha256_file(exe) if sha else None
    rep.info("scope", f"{len(sel)} mods, {sum(len(m.hooks) for m in sel)} hooks, "
             f"{sum(len(m.code_patches) for m in sel)} code patches vs {exe.name}")
    for m in sel:
        if m.target_versions and exe_sha and exe_sha not in m.target_versions:
            rep.info("scope", f"{m.id}: targets other exe version(s), skipped")
            continue
        _check_mod_sites(rep, pe, m)
    # KPM Patches/<Name>/*.kpatch packaging
    for m in sel:
        if m.kpm_dir is None:
            rep.skip("packaging", f"{m.id}: no KPM Patches dir (kpm_dir {kpm_dir})")
            continue
        packs = sorted(Path(m.kpm_dir).glob("*.kpatch"))
        if not packs:
            rep.skip("packaging", f"{m.id}: no .kpatch in {m.kpm_dir}")
        for kp in packs:
            check_packaging(rep, m, sc.kpatch_members(kp), kp.name)
    # staged builds
    variants = None
    for kp in kpatches:
        kp = Path(kp)
        try:
            km = sc.kpatch_members(kp)
        except Exception as e:  # unreadable / not a zip
            rep.fail("packaging", f"{kp}: {e}")
            continue
        pt = km.manifest_doc
        pid, ver = pt.get("id", ""), str(pt.get("version", ""))
        if mod_ids and pid not in mod_ids:
            continue
        if variants is None:
            variants = mods if mods is not None else sc.load_mods(repo, kpm_dir, variants=True)
        src = next((m for m in variants if m.id == pid and m.version == ver), None) \
            or next((m for m in variants if m.id == pid), None)
        if src is None:
            rep.warn("packaging", f"{kp.name}: id {pid!r} matches no mod under patch_manager_mods")
        check_packaging(rep, src, km, kp.name)
        if km.hooks is not None:
            tv, hooks = sc.parse_hooks(km.hooks, mod=pid, source=f"{kp.name}:{km.hooks_name}")
            if tv and exe_sha and exe_sha not in tv:
                rep.info("scope", f"{kp.name}: targets other exe version(s), hooks skipped")
            else:
                for h in hooks:
                    check_site(rep, pe, f"{pid}@{kp.name} {h.type}", h.address, h.orig, "code", h.type,
                               h.skip_original, h.replacement)
    return rep.findings[mark:]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--kpatch", action="append", default=[], metavar="PATH", help="also check this staged .kpatch (repeatable)")
    ap.add_argument("--mod", action="append", default=[], metavar="ID", help="only this patch id (repeatable)")
    ap.add_argument("--exe", default=str(sc.VANILLA_EXE), help="vanilla swkotor2.exe (default: $KOTOR2_VANILLA_EXE or backups/vanilla_clean)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    if not shutil.which(sc.OBJDUMP):
        rep.skip("tools", f"{sc.OBJDUMP} not found")
        return rep.finish("check_hook_bytes")
    run(exe=a.exe, kpatches=a.kpatch, mod_ids=set(a.mod) or None, reporter=rep)
    check_gog(rep, mod_ids=set(a.mod) or None)
    return rep.finish("check_hook_bytes")


if __name__ == "__main__":
    sys.exit(main())
