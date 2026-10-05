#!/usr/bin/env python3
"""Static check that no two patches touch the same exe bytes (doc 20 section 3, T0; promotes 20a_audit_tools/overlap.py).

  python3 scripts/check_hook_spans.py                       # ours (source) + installed (live patch_config.toml)
  python3 scripts/check_hook_spans.py --mod perf-lab --quiet
  python3 scripts/check_hook_spans.py --live-config /path/patch_config.toml --exceptions tests/static/span_exceptions.toml

A span is [address, address + len(original_bytes)) of a hook (kotor2-steam-aspyr.hooks.toml, or a live-config hook) or of a
code patch (the per-mod code_patches.toml sidecar: bytes a DLL writes itself). Two scopes are checked:
  source     every mod under patch_manager_mods whose target_versions contain the live exe's target_version_sha (absent = all)
  installed  the live patch_config.toml hooks (covers third-party patches) + the code_patches of the installed ids
For each scope:
  FAIL  two different patches overlap in at least one byte
  FAIL  two spans of one patch overlap
  WARN  two different patches are <= 8 bytes apart (adjacent or nearly so)
Once:
  WARN  an installed id's source hook addresses differ from its live-config hooks (a staged build is normal)
Exceptions (tests/static/span_exceptions.toml, schema in that file): an overlap between exactly the listed patches at the
listed address is accepted (INFO) only if both overlapping spans are code patches with verifies_original = true (the second
writer sees the first one's bytes and refuses); otherwise FAIL. An exception used in no scope is FAIL (stale), as is a
`tests` entry `wine:<smoke>[<mode>]` that smokes.toml does not define.
Exit 1 on any FAIL. Read-only: the live config is only parsed.
"""
import argparse
import sys
import tomllib
from dataclasses import dataclass
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import _static_common as sc  # noqa: E402

DEFAULT_EXCEPTIONS = sc.REPO / "tests" / "static" / "span_exceptions.toml"
GAP_WARN = 8


@dataclass
class Span:
    patch: str          # owning patch id
    start: int
    end: int            # exclusive; at least start + 1
    label: str          # "detour fnName" / "simple" / "code_patch tx"
    cp: object = None   # the CodePatch, for code-patch spans

    def __str__(self):
        return f"{self.patch} {self.label} 0x{self.start:08X}-0x{self.end:08X}"


def _hook_span(h, patch):
    return Span(patch, h.address, h.address + max(len(h.orig), 1), f"{h.type}{' ' + h.function if h.function else ''}")


def _cp_span(cp):
    return Span(cp.mod, cp.address, cp.address + max(len(cp.orig), 1), f"code_patch {cp.name}", cp)


def load_exceptions(src):
    """[{address, patches, reason, tests}] from a span_exceptions.toml path, or pass a list of dicts through."""
    if src is None:
        return []
    if not isinstance(src, (str, Path)):
        return [dict(e) for e in src]
    p = Path(src)
    if not p.is_file():
        return []
    out = []
    for i, e in enumerate(tomllib.loads(p.read_text(encoding="utf-8")).get("exception", [])):
        for key in ("address", "patches"):
            if key not in e:
                raise ValueError(f"{p}: exception[{i}] lacks '{key}'")
        out.append({"address": e["address"], "patches": list(e["patches"]), "reason": e.get("reason", ""),
                    "tests": list(e.get("tests", []))})
    return out


def _overlap(a, b):
    return a.start < b.end and b.start < a.end


def _match_exception(exc, a, b):
    """None if `exc` does not cover the overlap of a and b, else '' (valid) or a reason it is invalid."""
    lo, hi = max(a.start, b.start), min(a.end, b.end)
    if not (lo <= exc["address"] < hi) or not {a.patch, b.patch} <= set(exc["patches"]) or a.patch == b.patch:
        return None
    for s in (a, b):
        if s.cp is None:
            return f"{s} is a hook, not a code patch (exceptions only cover mutually refusing code patches)"
        if not s.cp.verifies_original:
            return f"{s} has verifies_original = false, so nothing stops the second writer from clobbering the first"
    return ""


def check_scope(spans, exceptions, only=None, declared=frozenset()):
    """Pairwise checks for one scope -> ([(key, sev, check, msg)], used exception indexes).

    only: set of patch ids; when given, pairs not involving one of them are skipped.
    declared: set of frozenset({id_a, id_b}) pairs a manifest lists in `conflicts` (KPM refuses to install both), so their
              overlaps are INFO, not FAIL (e.g. a library that takes over another mod's hooks before that mod migrates).
    """
    recs, used = [], set()
    ordered = sorted(spans, key=lambda s: (s.start, s.end, s.patch))
    for i, a in enumerate(ordered):
        for b in ordered[i + 1:]:
            if b.start >= a.end + GAP_WARN + 1:
                break   # sorted by start: every later span is further away
            if only and a.patch not in only and b.patch not in only:
                continue
            if _overlap(a, b):
                if a.patch == b.patch:
                    recs.append((("self", str(a), str(b)), "FAIL", "overlap-self", f"{a} <-> {b}"))
                    continue
                if frozenset((a.patch, b.patch)) in declared:
                    recs.append((("decl", str(a), str(b)), "INFO", "declared-conflict",
                                 f"{a} <-> {b}: accepted, the manifests declare `conflicts` (KPM never installs both)"))
                    continue
                hit = [(k, v) for k, e in enumerate(exceptions) if (v := _match_exception(e, a, b)) is not None]
                good = [k for k, v in hit if v == ""]
                if good:
                    used.add(good[0])
                    recs.append((("exc", str(a), str(b)), "INFO", "exception",
                                 f"{a} <-> {b}: accepted, 0x{exceptions[good[0]]['address']:08X} is a documented mutual-refusal site"))
                elif hit:
                    used.add(hit[0][0])   # matched, but invalid: FAIL below, not also "stale"
                    recs.append((("exc-bad", str(a), str(b)), "FAIL", "exception",
                                 f"{a} <-> {b}: exception at 0x{exceptions[hit[0][0]]['address']:08X} does not apply: {hit[0][1]}"))
                else:
                    recs.append((("ov", str(a), str(b)), "FAIL", "overlap", f"{a} <-> {b}"))
            elif a.patch != b.patch:
                gap = max(a.start - b.end, b.start - a.end)
                if gap <= GAP_WARN:
                    recs.append((("gap", str(a), str(b)), "WARN", "gap", f"{gap} byte gap: {a} <-> {b}"))
    return recs, used


def _source_spans(mods, sha):
    """(spans, {id: set of hook addresses}) for the mods whose target_versions include `sha` (empty = all)."""
    spans, hooks = [], {}
    for m in mods:
        if m.target_versions and sha and sha.upper() not in {t.upper() for t in m.target_versions}:
            continue
        spans += [_hook_span(h, m.id) for h in m.hooks] + [_cp_span(cp) for cp in m.code_patches]
        hooks[m.id] = {h.address for h in m.hooks}
    return spans, hooks


def _check_test_ids(rep, exceptions, smokes):
    """FAIL for a `wine:<name>[<mode>]` test id that smokes.toml does not define."""
    if not smokes or not Path(smokes).is_file():
        return
    defined = {}
    for s in tomllib.loads(Path(smokes).read_text(encoding="utf-8")).get("smoke", []):
        defined[s["name"]] = list(s.get("modes", []))
    for e in exceptions:
        for t in e["tests"]:
            if not t.startswith("wine:"):
                continue
            name, _, rest = t[5:].partition("[")
            mode = rest.rstrip("]") if rest else ""
            if name not in defined or (mode and mode not in defined[name]) or (not mode and defined[name]):
                rep.fail("exception-test", f"0x{e['address']:08X}: test id {t!r} is not in smokes.toml")


def run(mods, live_config=None, exceptions=None, live_sha=None, mod_ids=None, smokes=None, reporter=None):
    """Run every check; returns the list of Findings (also streamed to `reporter` if given).

    mods           [Mod] (sc.load_mods) - their hooks and code_patches are the source scope
    live_config    a patch_config.toml path or {id: [Hook]} (sc.load_live_config); None = source scope only
    exceptions     a span_exceptions.toml path or a list of {address, patches, reason, tests} dicts
    live_sha       target_version_sha filter for the mods' hooks (default: the live config's, when it is a path)
    mod_ids        only report pairs involving one of these patch ids (skips the stale-exception check)
    smokes         optional smokes.toml path: `wine:<name>[<mode>]` test ids in exceptions must exist there
    """
    rep = reporter or sc.Reporter(out=None)
    mark = len(rep.findings)
    only = set(mod_ids) if mod_ids else None
    excs = load_exceptions(exceptions)
    live = None
    if isinstance(live_config, (str, Path)):
        if Path(live_config).is_file():
            live_sha = live_sha or sc.live_target_sha(live_config)
            live = sc.load_live_config(live_config)
        else:
            rep.skip("live-config", f"{live_config} not found: installed scope skipped")
    elif live_config is not None:
        live = live_config
    mods = list(mods)
    merged = {}   # key -> [sev, check, msg, [scopes]]
    used_any = set()

    declared = {frozenset((m.id, c)) for m in mods for c in getattr(m, "conflicts", ()) if c != m.id}

    def add_scope(name, spans):
        recs, used = check_scope(spans, excs, only, declared if name == "source" else frozenset())
        used_any.update(used)
        for key, sev, chk, msg in recs:
            if key in merged:
                merged[key][3].append(name)
            else:
                merged[key] = [sev, chk, msg, [name]]

    src_spans, src_hooks = _source_spans(mods, live_sha)
    rep.info("scope", f"source: {len(mods)} mods, {len(src_spans)} spans" + (f" (target {live_sha[:8]}...)" if live_sha else ""))
    add_scope("source", src_spans)
    if live is not None:
        inst = [_hook_span(h, pid) for pid, hs in live.items() for h in hs]
        by_id = {m.id: m for m in mods}
        for pid in live:
            if pid in by_id:
                inst += [_cp_span(cp) for cp in by_id[pid].code_patches]
        rep.info("scope", f"installed: {len(live)} patches, {len(inst)} spans")
        add_scope("installed", inst)
        # source vs live drift, once per installed id
        for pid in sorted(live):
            if pid not in src_hooks or (only and pid not in only):
                continue
            lv = {h.address for h in live[pid]}
            sv = src_hooks[pid]
            if lv != sv:
                rep.warn("drift", f"{pid}: source hooks {len(sv)}, live {len(lv)}; source-only "
                         f"{[hex(a) for a in sorted(sv - lv)][:6]} live-only {[hex(a) for a in sorted(lv - sv)][:6]}")
    for key, (sev, chk, msg, scopes) in merged.items():
        rep.add(sev, chk, f"[{'+'.join(scopes)}] {msg}")
    if not only:
        for k, e in enumerate(excs):
            if k not in used_any:
                rep.fail("exception-stale", f"exception at 0x{e['address']:08X} ({', '.join(e['patches'])}) matched no overlap in "
                         f"{'source or installed scope' if live is not None else 'the source scope'}")
            if not e["tests"]:
                rep.fail("exception", f"exception at 0x{e['address']:08X} lists no `tests` proving the refusal")
    _check_test_ids(rep, excs, smokes)
    if not any(f.sev == "FAIL" for f in rep.findings[mark:]):
        rep.passed("spans", "no overlapping patches" + (f"; {len(used_any)} documented exception(s) used" if used_any else ""))
    return rep.findings[mark:]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--repo", default=str(sc.REPO), help="repo root holding patch_manager_mods/")
    ap.add_argument("--live-config", default=str(sc.LIVE_CONFIG), help="live patch_config.toml (read-only; missing = source scope only)")
    ap.add_argument("--exceptions", default=str(DEFAULT_EXCEPTIONS), help="span_exceptions.toml")
    ap.add_argument("--mod", action="append", default=[], metavar="ID", help="only pairs involving this patch id (repeatable)")
    ap.add_argument("--quiet", action="store_true", help="hide PASS and INFO lines")
    a = ap.parse_args(argv)
    rep = sc.Reporter(quiet=a.quiet)
    mods = sc.load_mods(a.repo, sc.KPM_DIR)
    run(mods, live_config=a.live_config, exceptions=a.exceptions, mod_ids=a.mod,
        smokes=Path(a.repo) / "patch_manager_mods" / "perf_lab_src" / "harness" / "smokes.toml", reporter=rep)
    return rep.finish("check_hook_spans")


if __name__ == "__main__":
    sys.exit(main())
