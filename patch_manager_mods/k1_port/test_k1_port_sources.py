#!/usr/bin/env python3
"""Tests for k1_port_sources.py. Tempdirs only; the real ledger and stage dirs are never touched.

Every test runs with K1PORT_LEDGER pointed into its tempdir and K1PORT_SOURCES unset (the real sources.json).
Run: python3 -u test_k1_port_sources.py   (cwd may be anything; exit 0 = all PASS)
"""
import contextlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import warnings
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_sources.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_sources as ks  # noqa: E402
from k1_port_sources import Ledger, SourceError  # noqa: E402

TESTS = []
CLASSES = {"vanilla": 0, "open": 1, "credit_required": 2, "unknown": 8,
           "personal_use_only": 9, "permission_required": 10}
UPSCALE = "upscale_plus_v0.52"
LEILUKIN = "leilukin_k1_twilek_f_2179"
VANILLA = "vanilla_k1"


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def expect_raises(exc, fn, *a, **k):
    try:
        fn(*a, **k)
    except exc as e:
        return e
    raise AssertionError(f"{getattr(fn, '__name__', fn)} did not raise {exc.__name__}")


@contextlib.contextmanager
def env(**kv):
    """Set env vars (None unsets); restores the previous values on exit."""
    old = {k: os.environ.get(k) for k in kv}
    try:
        for k, v in kv.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = str(v)
        yield
    finally:
        for k, v in old.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v


def src(cls, approval=None, **kw):
    d = {"name": kw.pop("name", cls), "author": "t", "url": None, "license_class": cls,
         "credits": [], "evidence": "test", "local_dir": None, "notes": ""}
    if approval is not None:
        d["requires_author_approval"] = approval
    d.update(kw)
    return d


def write_registry(root: Path, sources: dict, name: str = "sources.json") -> Path:
    p = root / name
    p.write_text(json.dumps({"schema": 1, "classes": CLASSES, "sources": sources}), encoding="utf-8")
    return p


def run_cli(root: Path, *args, ledger: Path = None):
    cli_env = dict(os.environ)
    cli_env.pop("K1PORT_SOURCES", None)
    cli_env["K1PORT_LEDGER"] = str(ledger or root / "cli_default_ledger.json")
    return subprocess.run([sys.executable, "-u", str(CLI), *args], cwd=str(root), env=cli_env,
                          capture_output=True, text=True, timeout=120)


@test
def test_registry_loads(root):
    reg = ks.load_registry()
    check(reg["schema"] == 1, "schema must be 1")
    check({VANILLA, LEILUKIN, UPSCALE} <= set(reg["sources"]), f"sources: {sorted(reg['sources'])}")
    check(reg["classes"] == CLASSES, "classes must match the registry block")
    check(ks.default_registry_path() == HERE / "sources.json", "default registry must be sources.json next to module")
    check(ks.requires_approval(VANILLA) is False, "vanilla must not need approval")
    check(ks.requires_approval(LEILUKIN) is False, "leilukin (credit_required) must not need approval")
    check(ks.requires_approval(UPSCALE) is True, "upscale (permission_required) must need approval")
    check(ks.removal_group(UPSCALE) == "pending_author_approval", "upscale removal group")
    check(ks.removal_group(VANILLA) is None, "vanilla removal group must be None")
    check(ks.get_source(LEILUKIN)["credits"][:2] == ["Leilukin", "SpaceAlex"], "leilukin credits")


@test
def test_approval_derived_from_class(root):
    p = write_registry(root, {
        "v_claims_approval": src("vanilla", approval=True),          # conflicting stored value, ignored
        "u_claims_none": src("unknown", approval=False),             # conflicting stored value, ignored
        "pu_no_field": src("personal_use_only"),                     # derived True
        "perm_true": src("permission_required", approval=True),      # agrees
        "open_false": src("open", approval=False),                   # agrees
    })
    with env(K1PORT_SOURCES=str(p)):
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            reg = ks.load_registry()
        s = reg["sources"]
        check(s["v_claims_approval"]["requires_author_approval"] is False, "vanilla must derive False")
        check(s["u_claims_none"]["requires_author_approval"] is True, "unknown must derive True")
        check(s["pu_no_field"]["requires_author_approval"] is True, "personal_use_only must derive True")
        check(s["perm_true"]["requires_author_approval"] is True, "permission_required must derive True")
        check(s["open_false"]["requires_author_approval"] is False, "open must derive False")
        conflicts = [str(w.message) for w in caught if "requires_author_approval" in str(w.message)]
        check(len(conflicts) == 2, f"expected 2 conflict warnings, got {len(conflicts)}: {conflicts}")
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            check(ks.requires_approval("u_claims_none") is True, "requires_approval must follow the class")


@test
def test_unknown_source_rejected(root):
    p = write_registry(root, {"open_one": src("open")})
    with env(K1PORT_SOURCES=str(p)):
        e = expect_raises(SourceError, ks.get_source, "not_registered")
        check(isinstance(e, KeyError), "SourceError must also be a KeyError")
        check(str(e).startswith("unregistered source id") and "not_registered" in str(e), f"message: {e}")
        expect_raises(SourceError, ks.requires_approval, "not_registered")
        expect_raises(SourceError, ks.removal_group, "not_registered")
        expect_raises(SourceError, ks.pick, [{"source_id": "not_registered"}])
        led = Ledger(root / "unk.json")
        expect_raises(SourceError, led.add, "a.tpc", "texture", "not_registered")
        expect_raises(SourceError, led.mark_approved, "not_registered", "note")
        check(led.entries() == [], "rejected add must not leave an entry")
        check(ks.get_source("open_one")["id"] == "open_one", "registered id must still resolve")


@test
def test_env_registry_override(root):
    p = write_registry(root, {"only_here": src("open")}, name="alt_sources.json")
    with env(K1PORT_SOURCES=str(p)):
        check(ks.default_registry_path() == p, "env must override the default path")
        check(ks.get_source("only_here")["license_class"] == "open", "override registry not used")
        expect_raises(SourceError, ks.get_source, VANILLA)


@test
def test_pick_prefers_approval_free(root):
    res = ks.pick([{"source_id": UPSCALE, "quality": 100, "path": "upscale.tpc"},
                   {"source_id": VANILLA, "quality": 1, "path": "vanilla.tpc"}])
    check(res["chosen"]["source_id"] == VANILLA, f"chosen {res['chosen']}")
    check(res["warning"] is None, f"warning must be None: {res['warning']}")
    check(len(res["rejected"]) == 1 and res["rejected"][0]["candidate"]["source_id"] == UPSCALE,
          f"rejected: {res['rejected']}")
    check("approval" in res["rejected"][0]["reason"], f"reason: {res['rejected'][0]['reason']}")
    check(res["chosen"]["path"] == "vanilla.tpc", "extra keys must be carried through")


@test
def test_pick_all_flagged_warns(root):
    p = write_registry(root, {
        "perm_src": src("permission_required"),
        "pd_src": src("personal_use_only"),
        "open_src": src("open"),
    })
    with env(K1PORT_SOURCES=str(p)):
        res = ks.pick([{"source_id": "perm_src", "quality": 5}, {"source_id": "pd_src", "quality": 2}])
        check(res["warning"] == "all candidates require author approval", f"warning: {res['warning']}")
        check(res["chosen"]["source_id"] == "pd_src", "lowest class rank (9 < 10) wins even at lower quality")
        check(len(res["rejected"]) == 1 and res["rejected"][0]["candidate"]["source_id"] == "perm_src",
              f"rejected: {res['rejected']}")
        res2 = ks.pick([{"source_id": "perm_src", "quality": 5}, {"source_id": "open_src", "quality": 0}])
        check(res2["warning"] is None and res2["chosen"]["source_id"] == "open_src", "open beats flagged")


@test
def test_pick_class_then_quality(root):
    p = write_registry(root, {
        "v": src("vanilla"), "c": src("credit_required"),
        "o1": src("open"), "o2": src("open"),
    })
    with env(K1PORT_SOURCES=str(p)):
        res = ks.pick([{"source_id": "c", "quality": 100}, {"source_id": "o1", "quality": 1},
                       {"source_id": "v", "quality": 0}])
        check(res["chosen"]["source_id"] == "v", "vanilla (rank 0) beats credit/open despite low quality")
        res = ks.pick([{"source_id": "o1", "quality": 1}, {"source_id": "o2", "quality": 5}])
        check(res["chosen"]["source_id"] == "o2", "higher quality wins within a class")
        check("quality" in res["rejected"][0]["reason"], f"reason: {res['rejected'][0]['reason']}")
        res = ks.pick([{"source_id": "c", "quality": 100}, {"source_id": "o1", "quality": 1}])
        check(res["chosen"]["source_id"] == "o1", "open (rank 1) beats credit_required (rank 2)")
        res = ks.pick([{"source_id": "o1"}, {"source_id": "o2", "quality": 0}])
        check(res["chosen"]["source_id"] == "o2", "missing quality ranks below any number")


@test
def test_pick_deterministic_ties(root):
    p = write_registry(root, {"z_open": src("open"), "a_open": src("open")})
    with env(K1PORT_SOURCES=str(p)):
        fwd = ks.pick([{"source_id": "z_open", "quality": 3}, {"source_id": "a_open", "quality": 3}])
        rev = ks.pick([{"source_id": "a_open", "quality": 3}, {"source_id": "z_open", "quality": 3}])
        check(fwd["chosen"]["source_id"] == "a_open" and rev["chosen"]["source_id"] == "a_open",
              f"tie must go to source_id sort order: {fwd['chosen']} / {rev['chosen']}")
        check("tie" in fwd["rejected"][0]["reason"], f"reason: {fwd['rejected'][0]['reason']}")
        check(fwd == ks.pick([{"source_id": "z_open", "quality": 3}, {"source_id": "a_open", "quality": 3}]),
              "pick must be repeatable")


@test
def test_pick_rejects_bad_input(root):
    expect_raises(SourceError, ks.pick, [])
    expect_raises(SourceError, ks.pick, "vanilla_k1")
    expect_raises(SourceError, ks.pick, [{"quality": 1}])
    expect_raises(SourceError, ks.pick, [{"source_id": VANILLA, "quality": "high"}])
    expect_raises(SourceError, ks.pick, [{"source_id": VANILLA, "quality": True}])


@test
def test_ledger_add_and_replace(root):
    led = Ledger(root / "stage" / "ledger.json")
    check(led.entries() == [], "missing ledger file must load empty")
    e = led.add("override/u.tpc", "texture", UPSCALE, sha256="ab" * 32, note_extra="kept")
    check(e["removal_group"] == "pending_author_approval", "upscale entry must be pending")
    check(e["author_approved"] is False and e["requires_author_approval"] is True, "approval fields")
    check(e["license_class"] == "permission_required" and e["source_id"] == UPSCALE, "class/source fields")
    check(e["file"] == "override/u.tpc" and e["kind"] == "texture" and e["credits"] == ks.get_source(UPSCALE)["credits"], "basic fields")
    check(e["sha256"] == "ab" * 32 and e["note_extra"] == "kept", "sha and extra fields stored")
    check(e["added"].endswith("+00:00"), f"added must be ISO UTC: {e['added']}")
    led.add("override/u.tpc", "model", VANILLA)
    ents = led.entries()
    check(len(ents) == 1, f"same file must replace, got {len(ents)} entries")
    check(ents[0]["source_id"] == VANILLA and ents[0]["removal_group"] is None and ents[0]["kind"] == "model",
          f"replaced entry: {ents[0]}")
    led.add("override/l.tpc", "texture", LEILUKIN)
    l_entry = [x for x in led.entries() if x["file"] == "override/l.tpc"][0]
    check(l_entry["credits"][:2] == ["Leilukin", "SpaceAlex"] and l_entry["removal_group"] is None,
          f"leilukin entry: {l_entry}")
    expect_raises(ValueError, led.add, "bad.tpc", "sound", VANILLA)


@test
def test_ledger_flagged_and_mark_approved(root):
    led = Ledger(root / "ledger.json")
    led.add("u1.tpc", "texture", UPSCALE)
    led.add("u2.tpc", "model", UPSCALE)
    led.add("l.tpc", "texture", LEILUKIN)
    led.add("v.tpc", "other", VANILLA)
    check(sorted(f["file"] for f in led.flagged()) == ["u1.tpc", "u2.tpc"], f"flagged: {led.flagged()}")
    copied = led.entries()
    copied[0]["file"] = "tampered"
    check(led.entries()[0]["file"] == "u1.tpc", "entries() must return copies")
    n = led.mark_approved(UPSCALE, "author replied 2026-10-11")
    check(n == 2, f"mark_approved updated {n}, expected 2")
    check(led.flagged() == [], "approval must clear the flag")
    for e in led.entries():
        if e["source_id"] == UPSCALE:
            check(e["author_approved"] is True and e["removal_group"] is None, f"approved entry: {e}")
            check(e["approval_note"] == "author replied 2026-10-11", "approval note must be recorded")
    led.add("u3.tpc", "texture", UPSCALE)
    u3 = [x for x in led.entries() if x["file"] == "u3.tpc"][0]
    check(u3["removal_group"] is None and u3["author_approved"] is True,
          "a new file from an approved source inherits the approval")
    check(led.flagged() == [], "inherited approval must not be flagged")


@test
def test_ledger_remove_and_find_by_sha(root):
    led = Ledger(root / "ledger.json")
    led.add("a.tpc", "texture", UPSCALE, sha256="ab" * 32)
    led.add("b.tpc", "texture", VANILLA, sha256="cd" * 32)
    found = led.find_by_sha(("AB" * 32))
    check([f["file"] for f in found] == ["a.tpc"], f"find_by_sha: {found}")
    check(led.find_by_sha("ee" * 32) == [], "unknown sha must return []")
    check(led.remove("a.tpc") == 1, "remove must report 1")
    check(led.remove("a.tpc") == 0, "second remove must report 0")
    check([e["file"] for e in led.entries()] == ["b.tpc"], "only b.tpc must remain")


@test
def test_ledger_add_derived_inherits_license(root):
    led = Ledger(root / "ledger.json")
    led.add("up/u.tpc", "texture", UPSCALE, sha256="ab" * 32)
    ai = {"tool": "comfyui", "model_id": "Qwen/Qwen-Image-Edit", "model_licence": "Apache-2.0",
          "seed": 7, "mask_sha256": "cd" * 32, "ai_pixel_fraction": 0.2}
    d = led.add_derived("out/e.tpc", "texture", "AB" * 32, "ef" * 32, ai=ai)
    check(d["source_id"] == UPSCALE and d["license_class"] == "permission_required", f"class must carry: {d}")
    check(d["removal_group"] == "pending_author_approval" and d["requires_author_approval"], "flag must carry")
    check(d["credits"] == ks.get_source(UPSCALE)["credits"], "credits must carry")
    check(d["derived"] and d["ai_derived"] and d["local_only"], f"derived flags: {d}")
    check(d["parent_sha256"] == "ab" * 32 and d["parent_file"] == "up/u.tpc", "parent link")
    check(d["ai"]["model_licence"] == "Apache-2.0" and d["ai"]["seed"] == 7, "ai fields")
    check(len(led.flagged()) == 2, "parent and copy are both flagged")
    check(led.mark_approved(UPSCALE, "ok") == 2, "approving the source must clear parent and copy")
    check(led.flagged() == [], "nothing flagged after approval")
    d2 = led.add_derived("out/e2.tpc", "texture", "ef" * 32, "12" * 32)  # copy of a copy, no AI
    check(d2["author_approved"] is True and d2["removal_group"] is None and d2["ai_derived"] is False,
          f"approval must carry down the chain: {d2}")
    check(d2["parent_file"] == "out/e.tpc", "chain parent link")
    led.add("v/n.tpc", "texture", VANILLA, sha256="34" * 32)
    v = led.add_derived("out/v.tpc", "texture", "34" * 32, "56" * 32)
    check(v["removal_group"] is None and v["source_id"] == VANILLA, "vanilla parent stays unflagged")
    e = expect_raises(SourceError, led.add_derived, "out/x.tpc", "texture", "99" * 32, "78" * 32)
    check("not in the ledger" in str(e), f"unknown parent must fail closed: {e}")
    expect_raises(SourceError, led.add_derived, "out/x.tpc", "texture", "", "78" * 32)


@test
def test_ledger_save_load_roundtrip(root):
    p = root / "deep" / "nested" / "ledger.json"
    led = Ledger(p)
    led.add("u.tpc", "texture", UPSCALE)
    led.add("v.tpc", "model", VANILLA)
    led.mark_approved(UPSCALE, "ok")
    led.add("w.tpc", "texture", UPSCALE)
    written = led.save()
    check(written == p and p.exists(), "save must create parent dirs and the file")
    again = Ledger(p)
    check(again.entries() == led.entries(), "reload must return the same entries")
    check(again.flagged() == [] and len(again.entries()) == 3, "reload must keep approval state")
    check(json.loads(p.read_text(encoding="utf-8"))["approvals"][UPSCALE]["note"] == "ok", "approval record saved")


@test
def test_atomic_save_keeps_old_file(root):
    p = root / "ledger.json"
    led = Ledger(p)
    led.add("a.tpc", "texture", VANILLA)
    led.save()
    before = p.read_text(encoding="utf-8")
    led.add("b.tpc", "texture", VANILLA)
    real_replace = os.replace

    def boom(*a, **k):
        raise OSError("simulated failure before replace")

    os.replace = boom
    try:
        expect_raises(OSError, led.save)
    finally:
        os.replace = real_replace
    check(p.read_text(encoding="utf-8") == before, "failed save must leave the old file untouched")
    json.loads(p.read_text(encoding="utf-8"))
    leftovers = list(root.glob(".ledger.json.*"))
    check(leftovers == [], f"temp files left behind: {leftovers}")
    led.save()
    check(len(json.loads(p.read_text(encoding="utf-8"))["entries"]) == 2, "a later save must succeed")


@test
def test_ledger_env_default_path(root):
    with env(K1PORT_LEDGER=str(root / "from_env.json")):
        check(Ledger().path == root / "from_env.json", "env K1PORT_LEDGER must set the default path")
        check(Ledger(root / "explicit.json").path == root / "explicit.json", "explicit path must win over env")
    check(ks.DEFAULT_LEDGER == "~/Games/KOTOR2_Mod_Workspace/k1_port_stage/ledger.json", "default ledger constant")


@test
def test_cli_smoke(root):
    r = run_cli(root, "show", "--json")
    check(r.returncode == 0, f"show --json rc {r.returncode}: {r.stderr}")
    rows = {x["id"]: x for x in json.loads(r.stdout)}
    check({VANILLA, LEILUKIN, UPSCALE} <= set(rows), f"show ids: {sorted(rows)}")
    check(rows[UPSCALE]["requires_author_approval"] is True and rows[VANILLA]["requires_author_approval"] is False,
          "show approval values")
    r = run_cli(root, "show")
    check(r.returncode == 0 and UPSCALE in r.stdout and "required" in r.stdout, f"show table: {r.stdout!r}")

    cands = json.dumps([{"source_id": UPSCALE, "quality": 9}, {"source_id": VANILLA, "quality": 1}])
    r = run_cli(root, "pick", "--json", cands)
    check(r.returncode == 0, f"pick rc {r.returncode}: {r.stderr}")
    out = json.loads(r.stdout)
    check(out["chosen"]["source_id"] == VANILLA and out["warning"] is None, f"pick output: {out}")

    r = run_cli(root, "pick", "--json", json.dumps([{"source_id": "ghost"}]))
    check(r.returncode == 1 and "unregistered" in r.stderr, f"unknown id: rc {r.returncode} err {r.stderr!r}")
    r = run_cli(root, "pick", "--json", "not json")
    check(r.returncode == 1, f"bad json must exit 1, got {r.returncode}")
    r = run_cli(root, "pick")
    check(r.returncode == 2, f"missing --json must exit 2, got {r.returncode}")

    led_path = root / "cli_ledger.json"
    led = Ledger(led_path)
    led.add("u.tpc", "texture", UPSCALE)
    led.add("v.tpc", "texture", VANILLA)
    led.save()
    r = run_cli(root, "ledger", "--json", ledger=led_path)
    check(r.returncode == 0, f"ledger --json rc {r.returncode}: {r.stderr}")
    data = json.loads(r.stdout)
    check(data["flagged"] == 1 and len(data["entries"]) == 2, f"ledger json: {data}")
    r = run_cli(root, "ledger", ledger=led_path)
    check(r.returncode == 0 and "flagged: 1" in r.stdout, f"ledger text: {r.stdout!r}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_src_test_"))
        try:
            with env(K1PORT_LEDGER=str(root / "unused_default_ledger.json"), K1PORT_SOURCES=None):
                fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
