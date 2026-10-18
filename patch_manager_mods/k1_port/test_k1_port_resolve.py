#!/usr/bin/env python3
"""Tests for k1_port_resolve.py. Tempdirs only; stdlib only (the pykotor backend is mocked), so plain python3 works.

Run: python3 -u test_k1_port_resolve.py   (cwd may be anything; exit 0 = all PASS)
"""
import contextlib
import hashlib
import io
import json
import os
import shutil
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_resolve as kr  # noqa: E402

TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sha_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def make_k1(root: Path, override_name: str = "override", chitin_name: str = "chitin.key") -> Path:
    k1 = root / "swkotor"
    (k1 / override_name).mkdir(parents=True)
    (k1 / chitin_name).write_bytes(b"KEY V1  ")
    return k1


def snapshot(k1: Path):
    """(relative path, is_dir, size, mtime_ns) for everything under k1."""
    rows = []
    for p in sorted(k1.rglob("*")):
        st = p.lstat()
        rows.append((p.relative_to(k1).as_posix(), p.is_dir(), st.st_size, st.st_mtime_ns))
    return rows


def raises(exc, fn, *a, **kw):
    try:
        fn(*a, **kw)
    except exc as e:
        return e
    raise AssertionError(f"expected {exc.__name__}")


class FakeBackend:
    """Stands in for the pykotor backend: {(group, resref, ext): (bytes, label)}."""

    def __init__(self, table):
        self.table = table
        self.calls = []

    def find(self, resref, ext, group):
        self.calls.append((group, resref, ext))
        return self.table.get((group, resref, ext))


@contextlib.contextmanager
def fake_pykotor(table):
    backend = FakeBackend(table)
    opened = []
    orig = kr._open_backend

    @contextlib.contextmanager
    def fake_open(k1):
        opened.append(k1)
        yield backend

    kr._open_backend = fake_open
    try:
        yield backend, opened
    finally:
        kr._open_backend = orig


@contextlib.contextmanager
def no_pykotor():
    """Any attempt to open pykotor fails the test."""
    orig = kr._open_backend

    def boom(k1):
        raise AssertionError("pykotor opened although Override should have answered")

    kr._open_backend = boom
    try:
        yield
    finally:
        kr._open_backend = orig


# ---------------------------------------------------------------- find_k1_dir

@test
def find_k1_dir_precedence(root):
    cli, env_d, cfg = make_k1(root / "a"), make_k1(root / "b"), make_k1(root / "c")
    env = {"KOTOR1_ROOT": str(env_d)}
    check(kr.find_k1_dir(str(cli), str(cfg), env) == cli, "cli must win")
    check(kr.find_k1_dir(None, str(cfg), env) == env_d, "env must beat config")
    check(kr.find_k1_dir(None, str(cfg), {}) == cfg, "config must beat default")
    check(kr.find_k1_dir("", str(cfg), {"KOTOR1_ROOT": " "}) == cfg, "blank cli/env are unset")


@test
def find_k1_dir_default_and_tilde(root):
    home = root / "home"
    make_k1(home / ".local/share/Steam/steamapps/common", "Override")  # creates .../common/swkotor
    old = os.environ.get("HOME")
    os.environ["HOME"] = str(home)
    try:
        got = kr.find_k1_dir(None, None, {})
        check(got == home / ".local/share/Steam/steamapps/common/swkotor", f"default path: {got}")
        check(kr.find_k1_dir("~/.local/share/Steam/steamapps/common/swkotor", None, {}) == got, "cli ~ expands")
    finally:
        if old is None:
            os.environ.pop("HOME", None)
        else:
            os.environ["HOME"] = old


@test
def find_k1_dir_chitin_case_insensitive(root):
    k1 = make_k1(root, chitin_name="CHITIN.KEY")
    check(kr.find_k1_dir(str(k1), None, {}) == k1, "CHITIN.KEY must count")


@test
def find_k1_dir_missing_chitin_names_source(root):
    bad = root / "empty"
    bad.mkdir()
    good = make_k1(root / "g")
    e = raises(kr.ResolveError, kr.find_k1_dir, str(bad), None, {})
    check("--k1-dir" in str(e) and "chitin.key" in str(e) and "KOTOR1_ROOT" in str(e) and "[k1] dir" in str(e), str(e))
    e = raises(kr.ResolveError, kr.find_k1_dir, None, str(good), {"KOTOR1_ROOT": str(bad)})
    check("env KOTOR1_ROOT" in str(e), f"must name env, not fall through to config: {e}")
    e = raises(kr.ResolveError, kr.find_k1_dir, None, str(bad), {})
    check("config" in str(e), str(e))
    check(issubclass(kr.ResolveError, Exception), "ResolveError is an Exception")


# ---------------------------------------------------------------- Override

@test
def override_hit_lowercase_dir(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_test.mdl").write_bytes(b"MDLDATA")
    with no_pykotor():
        r = kr.resolve(k1, "p_test", "mdl")
    check(r is not None and r.data == b"MDLDATA", f"data: {r}")
    check(r.origin == "override" and r.licence == "unverified_override", f"tags: {r}")
    check(r.sha256 == sha_of(b"MDLDATA"), "sha256")
    check(r.path.endswith("p_test.mdl"), r.path)
    check(kr.is_flagged(r.licence), "override is flagged")


@test
def override_hit_capitalised_dir_and_file(root):
    k1 = make_k1(root, "Override")
    (k1 / "Override" / "P_TEST.MDX").write_bytes(b"MDXDATA")
    with no_pykotor():
        r = kr.resolve(k1, "P_Test", "MDX")  # resref and ext are lowercased; file match is case-insensitive
    check(r is not None and r.data == b"MDXDATA" and r.origin == "override", f"{r}")
    check(r.path == "Override/P_TEST.MDX", r.path)


@test
def bad_resref_and_ext_rejected(root):
    k1 = make_k1(root)
    for bad in ("", "../etc/passwd", "a" * 17, "has space", "dot.dot", "a/b", "ünï"):
        raises(kr.ResolveError, kr.resolve, k1, bad, "mdl")
    for bad in ("", "toolong", "m/l", "m.l"):
        raises(kr.ResolveError, kr.resolve, k1, "p_test", bad)
    with fake_pykotor({}):
        check(kr.resolve(k1, "a" * 16, ".mdl") is None, "16-char resref and a leading dot on ext are accepted")


@test
def symlinked_override_file_refused(root):
    k1 = make_k1(root)
    outside = root / "outside.mdl"
    outside.write_bytes(b"SECRET")
    (k1 / "override" / "p_link.mdl").symlink_to(outside)
    with no_pykotor():
        e = raises(kr.ResolveError, kr.resolve, k1, "p_link", "mdl")
    check("symlink" in str(e), str(e))


@test
def symlinked_override_dir_refused(root):
    k1 = root / "swkotor"
    k1.mkdir()
    (k1 / "chitin.key").write_bytes(b"x")
    elsewhere = root / "elsewhere"
    elsewhere.mkdir()
    (elsewhere / "p_x.mdl").write_bytes(b"X")
    (k1 / "override").symlink_to(elsewhere, target_is_directory=True)
    with no_pykotor():
        e = raises(kr.ResolveError, kr.resolve, k1, "p_x", "mdl")
    check("symlink" in str(e), str(e))


@test
def not_an_install_refused(root):
    plain = root / "plain"
    plain.mkdir()
    e = raises(kr.ResolveError, kr.resolve, plain, "p_x", "mdl")
    check("chitin.key" in str(e), str(e))


# ---------------------------------------------------------------- pykotor fallback (mocked)

@test
def chitin_then_module_fallback_and_tags(root):
    k1 = make_k1(root)
    table = {("chitin", "p_a", "mdl"): (b"CHITIN", "chitin:models.bif"),
             ("module", "p_b", "mdl"): (b"MOD", "module:danm13.rim")}
    with fake_pykotor(table) as (backend, opened):
        a = kr.resolve(k1, "p_a", "mdl")
        b = kr.resolve(k1, "p_b", "mdl")
        c = kr.resolve(k1, "p_none", "mdl")
    check(a.origin == "chitin" and a.licence == "vanilla_k1" and a.path == "chitin:models.bif" and a.data == b"CHITIN", f"{a}")
    check(b.origin == "module" and b.licence == "vanilla_k1" and b.sha256 == sha_of(b"MOD"), f"{b}")
    check(c is None, "missing resource is None")
    check(not kr.is_flagged(a.licence), "vanilla is not flagged")
    p_b_calls = [g for g, r, e in backend.calls if r == "p_b"]
    check(p_b_calls == ["chitin", "module"], f"chitin must be searched before modules: {p_b_calls}")
    check(all(g != "texturepack" for g, r, e in backend.calls), "texture packs are for textures only")


@test
def override_beats_chitin(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_a.mdl").write_bytes(b"OVR")
    with fake_pykotor({("chitin", "p_a", "mdl"): (b"CHITIN", "chitin:models.bif")}) as (backend, opened):
        r = kr.resolve(k1, "p_a", "mdl")
    check(r.origin == "override" and r.data == b"OVR", f"{r}")
    check(backend.calls == [] and opened == [], "pykotor must not be touched on an Override hit")


@test
def texture_pack_before_chitin(root):
    k1 = make_k1(root)
    table = {("texturepack", "n_tex", "tpc"): (b"TP", "texturepack:swpc_tex_tpa.erf"),
             ("chitin", "n_tex", "tpc"): (b"CH", "chitin:textures.bif"),
             ("chitin", "n_tex", "txi"): (b"TXI", "chitin:textures.bif")}
    with fake_pykotor(table) as (backend, opened):
        t = kr.resolve(k1, "n_tex", "tpc")
        x = kr.resolve(k1, "n_tex", "txi")
    check(t.origin == "texturepack" and t.data == b"TP" and t.licence == "vanilla_k1", f"{t}")
    check(x.origin == "chitin", "non-texture types skip texture packs")


@test
def resolve_many_opens_once_and_lowercases(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_o.mdl").write_bytes(b"O")
    table = {("chitin", "p_a", "mdl"): (b"A", "chitin:models.bif"), ("chitin", "p_a", "mdx"): (b"AX", "chitin:models.bif")}
    with fake_pykotor(table) as (backend, opened):
        got = kr.resolve_many(k1, [("P_A", "MDL"), ("p_a", "mdx"), ("p_o", "mdl"), ("p_gone", "mdl"), ("p_a", "mdl")])
    check(len(opened) == 1, f"Installation must open once, opened {len(opened)}")
    check(set(got) == {("p_a", "mdl"), ("p_a", "mdx"), ("p_o", "mdl"), ("p_gone", "mdl")}, f"keys: {set(got)}")
    check(got[("p_gone", "mdl")] is None and got[("p_o", "mdl")].origin == "override", "mixed results")


@test
def pykotor_not_opened_when_all_override(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_o.mdl").write_bytes(b"O")
    with no_pykotor():
        got = kr.resolve_many(k1, [("p_o", "mdl")])
    check(got[("p_o", "mdl")].data == b"O", "override only")


@test
def missing_pykotor_gives_setup_hint(root):
    k1 = make_k1(root)
    orig = kr.kt.import_install_api

    def fail(errors):
        errors.append("cannot import pykotor: boom")
        return None

    kr.kt.import_install_api = fail
    try:
        e = raises(kr.ResolveError, kr.resolve, k1, "p_a", "mdl")
    finally:
        kr.kt.import_install_api = orig
    check("setup_venv.sh" in str(e) and "venvs/k1port/bin/python" in str(e), str(e))


# ---------------------------------------------------------------- licence

@test
def hd_overlay_marker_flags_non_override(root):
    k1 = make_k1(root)
    (k1 / "K1_HD_OVERLAY.json").write_text(json.dumps({"source_id": "characters_revisited_main"}))
    (k1 / "override" / "p_o.mdl").write_bytes(b"O")
    table = {("chitin", "p_a", "mdl"): (b"A", "chitin:models.bif"),
             ("texturepack", "n_t", "tpc"): (b"T", "texturepack:swpc_tex_tpa.erf"),
             ("module", "p_m", "mdl"): (b"M", "module:x.rim")}
    with fake_pykotor(table):
        got = kr.resolve_many(k1, [("p_a", "mdl"), ("n_t", "tpc"), ("p_m", "mdl"), ("p_o", "mdl")])
    check(got[("p_a", "mdl")].licence == "hd_overlay", "chitin under overlay marker")
    check(got[("n_t", "tpc")].licence == "hd_overlay", "texturepack under overlay marker")
    check(got[("p_m", "mdl")].licence == "hd_overlay", "module under overlay marker")
    check(got[("p_o", "mdl")].licence == "unverified_override", "override stays unverified_override")
    check(all(kr.is_flagged(v.licence) for v in got.values()), "all flagged")


@test
def unreadable_marker_still_flags(root):
    k1 = make_k1(root)
    (k1 / "K1_HD_OVERLAY.json").write_text("{not json")
    with fake_pykotor({("chitin", "p_a", "mdl"): (b"A", "chitin:models.bif")}):
        r = kr.resolve(k1, "p_a", "mdl")
    check(r.licence == "hd_overlay", f"{r}")


@test
def flagged_set_and_is_flagged(root):
    check(kr.FLAGGED == {"unverified_override", "hd_overlay"}, str(kr.FLAGGED))
    check(kr.is_flagged("hd_overlay") and kr.is_flagged("unverified_override"), "flagged")
    check(not kr.is_flagged("vanilla_k1") and not kr.is_flagged("anything_else"), "not flagged")


@test
def licence_report_rows(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_o.mdl").write_bytes(b"O")
    with fake_pykotor({("chitin", "p_a", "mdl"): (b"A", "chitin:models.bif")}):
        got = kr.resolve_many(k1, [("p_a", "mdl"), ("p_o", "mdl"), ("p_gone", "mdl")])
    rep = kr.licence_report(got)
    check(len(rep) == 2, f"missing items are skipped: {rep}")
    by = {(r["resref"], r["ext"]): r for r in rep}
    a, o = by[("p_a", "mdl")], by[("p_o", "mdl")]
    check(set(a) == {"resref", "ext", "origin", "path", "sha256", "licence", "flagged"}, f"keys: {set(a)}")
    check(a["origin"] == "chitin" and a["flagged"] is False and a["sha256"] == sha_of(b"A"), f"{a}")
    check(o["origin"] == "override" and o["flagged"] is True and o["licence"] == "unverified_override", f"{o}")
    json.dumps(rep)


# ---------------------------------------------------------------- read only + CLI

@test
def install_unchanged_after_resolve(root):
    k1 = make_k1(root, "Override")
    (k1 / "Override" / "p_o.mdl").write_bytes(b"O")
    (k1 / "modules").mkdir()
    (k1 / "modules" / "x.rim").write_bytes(b"R")
    (k1 / "K1_HD_OVERLAY.json").write_text("{}")
    before = snapshot(k1)
    with fake_pykotor({("chitin", "p_a", "mdl"): (b"A", "chitin:models.bif")}):
        kr.resolve_many(k1, [("p_o", "mdl"), ("p_a", "mdl"), ("p_gone", "tga")])
    check(snapshot(k1) == before, "K1 dir listing/mtimes changed")


@test
def cli_exit_codes_and_output(root):
    k1 = make_k1(root)
    (k1 / "override" / "p_o.mdl").write_bytes(b"O")
    out, err = io.StringIO(), io.StringIO()
    with no_pykotor(), contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = kr.main(["--k1-dir", str(k1), "p_o.mdl"])
    check(rc == 0 and "override" in out.getvalue() and "unverified_override" in out.getvalue(), out.getvalue())
    check(sha_of(b"O")[:16] in out.getvalue(), "sha in output")
    out = io.StringIO()
    with fake_pykotor({}), contextlib.redirect_stdout(out):
        rc = kr.main(["--k1-dir", str(k1), "--json", "p_o.mdl", "p_gone.mdl"])
    data = json.loads(out.getvalue())
    check(rc == 1 and data["missing"] == ["p_gone.mdl"] and data["items"][0]["flagged"] is True, out.getvalue())
    with contextlib.redirect_stderr(io.StringIO()):
        rc = kr.main(["--k1-dir", str(root / "nope"), "p_o.mdl"])
    check(rc == 1, "bad k1 dir exits 1")


@test
def test_mesh_names(root: Path):
    data = b"\x01\x02P_CarthBB01\x00\x00\x00\x3fP_CarthH01\x00junkxp_carthba01\x00"
    got = kr.mesh_names(data, ["p_carthbb01", "p_carthh01", "p_carthba01", "p_carth"])
    # p_carthba01 continues a longer name on the left (x...), p_carth is not NUL-terminated: neither counts
    check(got == ["p_carthbb01", "p_carthh01"], got)
    k1 = root / "k1"
    (k1 / "override").mkdir(parents=True)
    (k1 / "chitin.key").write_bytes(b"KEY")
    (k1 / "override" / "p_m.mdl").write_bytes(b"\x00P_T01\x00")
    out = io.StringIO()
    with fake_pykotor({}), contextlib.redirect_stdout(out):
        rc = kr.main(["--k1-dir", str(k1), "--json", "--scan", "p_t01", "--scan", "p_u01", "p_m.mdl"])
    check(rc == 0 and json.loads(out.getvalue())["items"][0]["contains"] == ["p_t01"], out.getvalue())


def main() -> int:
    passed = 0
    saved_env = os.environ.get("KOTOR1_ROOT")
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_resolve_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    if saved_env is not None:
        os.environ["KOTOR1_ROOT"] = saved_env
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
