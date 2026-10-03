#!/usr/bin/env python3
"""Static check that every detour hook's `function` is exported (doc 20 section 3, T0).

  python3 scripts/check_exports.py                         # all mods vs their KPM Patches/<Name>/ .kpatch
  python3 scripts/check_exports.py --mod level-up-override --quiet
  python3 scripts/check_exports.py --kpatch /path/Staged.kpatch   # also check a staged build (repeatable)
  python3 scripts/check_exports.py --repo /tmp/x/repo --kpm-dir /tmp/x/kpm   # alternate trees (tests, break checks)

KPM only logs "Function not found" at runtime and silently skips a hook whose function is not exported, so for
every mod with at least one `detour` hook (type "simple" hooks have no function and are ignored):
  FAIL  a detour `function` is missing from the mod's exports.def (names compared as sets; functions repeat)
  FAIL  a detour `function` is missing from the PE export table of binaries/windows_x86.dll in the .kpatch
        (also applied to the functions of the .kpatch's embedded hooks toml)
  FAIL  the .kpatch has no readable DLL, or the DLL is not a PE32 image
  WARN  an exports.def name is not in the DLL's export table (stale build; the .def also lists test seams)
  SKIP  no .kpatch for the mod in KPM Patches/<Name>/ (and none passed with --kpatch)
Mods without detours are skipped (INFO).

Which KPM .kpatch: every Patches/<Name>/*.kpatch whose manifest id and version match the source mod (so the stale
CompanionAI-0.1.0-live.kpatch is ignored while CompanionAI.kpatch is 0.2.0-probe); if none matches, the canonical
<Name>.kpatch is used and the version mismatch is a WARN. --kpatch files are matched to their source mod by manifest
id (+ version, so a staged 1.0.1-probe build uses its own source dir). Exit 1 on any FAIL.
"""
import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402


def detour_functions(hooks) -> set:
    """Names of the `function` fields of detour hooks (a set: functions repeat). Hooks lacking one yield ''."""
    return {(h.function or "") for h in hooks if h.type == "detour"}


def check_def(rep, mod) -> bool:
    """detour functions of `mod` are a subset of its exports.def. Returns True if no FAIL."""
    funcs = detour_functions(mod.hooks)
    if not mod.has_def:
        rep.fail("def", f"{mod.id}: has {len(funcs)} detour function(s) but no exports.def in {mod.dir}")
        return False
    ok = True
    if "" in funcs:
        rep.fail("def", f"{mod.id}: a detour hook has no `function`")
        funcs.discard("")
        ok = False
    missing = sorted(funcs - set(mod.exports))
    if missing:
        rep.fail("def", f"{mod.id}: detour function(s) not in exports.def: {', '.join(missing)}")
        ok = False
    return ok


def check_dll(rep, mod, label, dll, extra_funcs=()) -> bool:
    """detour functions (plus `extra_funcs`, e.g. an embedded hooks toml's) are in the DLL's export table; the
    .def names missing from it are a WARN. `dll` is the DLL's bytes (None = absent). Returns True if no FAIL."""
    if not dll:
        rep.fail("dll", f"{mod.id} [{label}]: no binaries/windows_x86.dll")
        return False
    try:
        exported = set(sc.PeImage(data=dll).exports())
    except (ValueError, OSError) as e:
        rep.fail("dll", f"{mod.id} [{label}]: unreadable DLL: {e}")
        return False
    funcs = (detour_functions(mod.hooks) | set(extra_funcs)) - {""}
    ok = True
    missing = sorted(funcs - exported)
    if missing:
        rep.fail("dll", f"{mod.id} [{label}]: detour function(s) not exported by the DLL (KPM would skip the hook "
                 f"at runtime): {', '.join(missing)}")
        ok = False
    stale = sorted(set(mod.exports) - exported)
    if stale:
        rep.warn("stale-dll", f"{mod.id} [{label}]: exports.def name(s) absent from the DLL (stale build?): "
                 f"{', '.join(stale)}")
    return ok


def _embedded_functions(km) -> set:
    """Detour functions of the hooks toml embedded in a .kpatch (empty if absent or unparsable)."""
    if km.hooks is None:
        return set()
    try:
        return detour_functions(sc.parse_hooks(km.hooks)[1])
    except Exception:  # broken embedded toml is check_hook_bytes' business
        return set()


def kpm_kpatches(mod) -> list:
    """[(label, KpatchMembers)] of the KPM .kpatch to check for `mod` (see module docstring); [] if none."""
    if mod.kpm_dir is None:
        return []
    packs = sorted(Path(mod.kpm_dir).glob("*.kpatch"))
    loaded = []
    for p in packs:
        try:
            loaded.append((p, sc.kpatch_members(p)))
        except Exception:
            loaded.append((p, None))

    def ident(km):
        d = km.manifest_doc if km is not None and km.manifest else {}
        return d.get("id"), str(d.get("version", ""))
    match = [(p, km) for p, km in loaded if ident(km) == (mod.id, mod.version)]
    if match:
        return [(p.name, km) for p, km in match]
    canon = Path(mod.kpm_dir) / f"{Path(mod.kpm_dir).name}.kpatch"
    pick = [(p, km) for p, km in loaded if p == canon] or loaded[:1]
    return [(p.name, km) for p, km in pick]


def run(repo=sc.REPO, kpm_dir=sc.KPM_DIR, kpatches=(), mod_ids=None, mods=None, dlls=None, reporter=None):
    """Run the checks; returns the list of Findings (also streamed to `reporter` if given).

    repo/kpm_dir locate the mods and Patches/ (or pass `mods`, a list of _static_common.Mod, directly).
    dlls: optional {mod id: DLL bytes | [(label, DLL bytes)]} used instead of the KPM .kpatch lookup (tests).
    kpatches: staged .kpatch paths (or open file objects), matched to a source mod by manifest id (+ version).
    mod_ids filters by patch id. Mods without detour hooks are skipped.
    """
    rep = reporter or sc.Reporter(out=None)
    mark = len(rep.findings)
    all_mods = mods if mods is not None else sc.load_mods(repo, kpm_dir)
    sel = [m for m in all_mods if not mod_ids or m.id in mod_ids]
    n_checked = 0
    for m in sel:
        funcs = detour_functions(m.hooks)
        if not funcs:
            rep.info("scope", f"{m.id}: no detour hooks, skipped")
            continue
        n_checked += 1
        ok = check_def(rep, m)
        if dlls is not None:
            given = dlls.get(m.id)
            targets = [("dll", given)] if isinstance(given, (bytes, bytearray)) else list(given or [])
            if not targets:
                rep.skip("dll", f"{m.id}: no DLL supplied")
            for label, dll in targets:
                ok = check_dll(rep, m, label, dll) and ok
        else:
            packs = kpm_kpatches(m)
            if not packs:
                rep.skip("dll", f"{m.id}: no .kpatch in KPM Patches/ ({m.kpm_dir or kpm_dir})")
            for name, km in packs:
                if km is None:
                    rep.fail("dll", f"{m.id}: {name} is not a readable zip")
                    ok = False
                    continue
                if km.manifest and str(km.manifest_doc.get("version", "")) != m.version:
                    rep.warn("stale-kpatch", f"{m.id}: {name} is version {km.manifest_doc.get('version')!r}, "
                             f"source {m.version!r}")
                ok = check_dll(rep, m, name, km.dll, _embedded_functions(km)) and ok
        if ok:
            rep.passed("exports", f"{m.id}: {len(funcs)} detour function(s) exported")
    # staged builds
    variants = None
    for kp in kpatches:
        label = Path(kp).name if isinstance(kp, (str, Path)) else "<kpatch>"
        try:
            km = sc.kpatch_members(kp)
        except Exception as e:  # unreadable / not a zip
            rep.fail("dll", f"{label}: {e}")
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
            rep.warn("staged", f"{label}: id {pid!r} matches no mod under patch_manager_mods")
            continue
        if src.version != ver:
            rep.warn("staged", f"{label}: version {ver!r} has no source dir of its own, checked against {src.version!r}")
        if not detour_functions(src.hooks):
            rep.info("scope", f"{label}: {pid} has no detour hooks, skipped")
            continue
        ok = check_def(rep, src)
        ok = check_dll(rep, src, label, km.dll, _embedded_functions(km)) and ok
        if ok:
            rep.passed("exports", f"{label} ({pid} {ver}): detour functions exported")
    if not n_checked and not kpatches:
        rep.info("scope", "no mods with detour hooks selected")
    return rep.findings[mark:]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--kpatch", action="append", default=[], metavar="PATH", help="also check this staged .kpatch (repeatable)")
    ap.add_argument("--mod", action="append", default=[], metavar="ID", help="only this patch id (repeatable)")
    ap.add_argument("--repo", default=str(sc.REPO), help="repo root holding patch_manager_mods/ (default: this checkout)")
    ap.add_argument("--kpm-dir", default=str(sc.KPM_DIR), help="Kotor Patch Manager checkout holding Patches/ (default: $KPM_DIR)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    run(repo=a.repo, kpm_dir=a.kpm_dir, kpatches=a.kpatch, mod_ids=set(a.mod) or None, reporter=rep)
    return rep.finish("check_exports")


if __name__ == "__main__":
    sys.exit(main())
