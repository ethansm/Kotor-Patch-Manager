#!/usr/bin/env python3
"""Tests for k1_port_cr_extract.py. Tempdirs only; no rar, no game files, no unrar.

CLI calls run under python -I with K1PORT_CR_* removed from the environment (unless a test sets them).
Run: python3 -u test_k1_port_cr_extract.py   (cwd may be anything; exit 0 = all PASS)
"""
import contextlib
import hashlib
import io
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_cr_extract.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
import k1_port_cr_extract as kx  # noqa: E402

TESTS = []

FIXTURE = (
    "main\t1000\tOverride/pfha03d1.tga\n"
    "main\t1000\tOverride/pfha03d1.txi\n"
    "main\t2000\tOverride/c_hutt01.mdl\n"
    "main\t3000\tOverride/c_drdwar.tga\n"
    "creatures\t4000\tOverride/c_rancor.mdx\n"
    "main\t5000\tOverride/po_pfha01.tga\n"
)


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def expect_raises(exc, fn, *args):
    try:
        fn(*args)
    except exc:
        return
    raise AssertionError(f"{fn.__name__}{args} did not raise {exc.__name__}")


def cli(*args, env=None):
    base = {k: v for k, v in os.environ.items() if not k.startswith("K1PORT_CR_")}
    base.update(env or {})
    return subprocess.run(
        [sys.executable, "-I", str(CLI), *args],
        capture_output=True, text=True, env=base, cwd=str(HERE), check=False,
    )


def write_fixture(root: Path) -> Path:
    fx = root / "fixture.tsv"
    fx.write_text(FIXTURE, encoding="utf-8")
    return fx


@test
def test_family_vectors():
    cases = {
        "pfha03d1": "head_player",
        "n_wookiem02": "species",
        "po_pfha01": "portrait",
        "i_sithmask": "item",
        "pmbb01": "body_player",
        "c_hutt01": "species",
        "c_drdwar": "creature_droid",
        "n_commm03": "commoner_or_head",
        "xyz": "other",
        "PFHA03D1": "head_player",
        "n_smugg01": "faction_npc",
        "p_bastila": "party",
    }
    for stem, want in cases.items():
        got = kx.family(stem)
        check(got == want, f"family({stem!r}) = {got}, want {want}")


@test
def test_shard_of_vectors():
    cases = [
        (("main", "pfha03d1", "tga"), "head_player"),
        (("main", "pfha03d1", "txi"), "head_player"),
        (("main", "pmbb01", "tga"), "body_player"),
        (("main", "c_hutt01", "mdl"), "models"),
        (("main", "c_hutt01", "MDX"), "models"),
        (("main", "c_drdwar", "tga"), "creature"),
        (("main", "n_commm03", "tga"), "commoner"),
        (("main", "po_pfha01", "tga"), "portrait"),
        (("main", "i_sithmask", "tga"), "item"),
        (("main", "p_bastila", "tga"), "party"),
        (("main", "xyz", "tga"), "other"),
        (("creatures", "c_rancor", "mdx"), "creature"),
        (("creatures", "c_rancor", "tga"), "creature"),
        (("updates", "n_commm03", "tga"), "updates"),
        (("updates", "c_hutt01", "mdl"), "updates"),
        (("bogus", "x", "tga"), None),
    ]
    for (pack, stem, ext), want in cases:
        got = kx.shard_of(pack, stem, ext)
        check(got == want, f"shard_of({pack},{stem},{ext}) = {got}, want {want}")
    check("portrait" not in kx.DEFAULT_SHARDS and "item" not in kx.DEFAULT_SHARDS,
          "DEFAULT_SHARDS contains portrait or item")
    check(set(kx.DEFAULT_SHARDS) | {"portrait", "item"} == set(kx.ALL_SHARDS),
          "DEFAULT_SHARDS plus portrait/item != ALL_SHARDS")


@test
def test_parse_tsv_counts_malformed_lines():
    with tempfile.TemporaryDirectory() as td:
        f = Path(td) / "list.tsv"
        f.write_text(
            "main\t10\tOverride/a.tga\n"
            "\n"
            "main\tabc\tOverride/b.tga\n"
            "onlytwo\tfields\n"
            "mystery\t5\tOverride/c.tga\n"
            "updates\t7\tOverride/d.tga\n",
            encoding="utf-8",
        )
        rows, skipped = kx.parse_tsv(f)
    check(rows == [("main", 10, "Override/a.tga"), ("updates", 7, "Override/d.tga")], f"rows {rows}")
    check(skipped == 3, f"skipped {skipped}, want 3")


@test
def test_plan_shards_fixture():
    with tempfile.TemporaryDirectory() as td:
        rows, skipped = kx.parse_tsv(write_fixture(Path(td)))
    check(skipped == 0 and len(rows) == 6, f"fixture rows {len(rows)} skipped {skipped}")
    plan = kx.plan_shards(rows)
    want = {
        "head_player": [("Override/pfha03d1.tga", 1000), ("Override/pfha03d1.txi", 1000)],
        "models": [("Override/c_hutt01.mdl", 2000)],
        "creature": [("Override/c_drdwar.tga", 3000), ("Override/c_rancor.mdx", 4000)],
        "portrait": [("Override/po_pfha01.tga", 5000)],
    }
    check(plan == want, f"plan_shards = {plan}")


@test
def test_plan_shards_drops_directory_entries():
    rows = [("main", 0, "Override"), ("updates", 0, "update"), ("updates", 0, "update/Override"),
            ("main", 1000, "Override/pfha03d1.tga")]
    plan = kx.plan_shards(rows)
    check(plan == {"head_player": [("Override/pfha03d1.tga", 1000)]}, f"plan_shards = {plan}")


@test
def test_check_dest_refuses_inside_repo_accepts_outside():
    with tempfile.TemporaryDirectory() as td:
        repo = Path(td) / "repo"
        (repo / ".git").mkdir(parents=True)
        expect_raises(ValueError, kx.check_dest, repo, repo)
        expect_raises(ValueError, kx.check_dest, repo / "a" / "b", repo)
        kx.check_dest(Path(td) / "outside" / "dest", repo)
    real_root = kx.find_repo_root(HERE)
    check((real_root / ".git").exists(), "find_repo_root did not return a dir containing .git")
    expect_raises(ValueError, kx.check_dest,
                  real_root / "patch_manager_mods" / "k1_port" / "probe", real_root)


@test
def test_check_disk_boundary():
    kx.check_disk(200, 100)
    kx.check_disk(0, 0)
    expect_raises(ValueError, kx.check_disk, 199, 100)


@test
def test_flatten_shard_lowercases_and_removes_subdirs():
    with tempfile.TemporaryDirectory() as td:
        shard_dir = Path(td) / "dest" / "head_player"
        (shard_dir / "Override").mkdir(parents=True)
        (shard_dir / "Override" / "Pfha03D1.tga").write_bytes(b"abc")
        (shard_dir / "Override" / "pfha03d1.txi").write_bytes(b"xy")
        kx.flatten_shard(shard_dir)
        names = sorted(p.name for p in shard_dir.iterdir())
        check(names == ["pfha03d1.tga", "pfha03d1.txi"], f"after flatten: {names}")
        check((shard_dir / "pfha03d1.tga").read_bytes() == b"abc", "content changed by flatten")


@test
def test_manifest_rewrites_only_its_shard():
    with tempfile.TemporaryDirectory() as td:
        dest = Path(td) / "dest"
        kx.write_manifest(dest, "head_player", [("pfha03d1", "tga", 3, "aa")])
        kx.write_manifest(dest, "models", [("c_hutt01", "mdl", 2, "bb")])
        kx.write_manifest(dest, "head_player", [("pfha03d1", "tga", 3, "cc")])
        lines = (dest / "manifest.tsv").read_text(encoding="utf-8").splitlines()
    check(lines[0] == kx.MANIFEST_HEADER, f"header {lines[0]!r}")
    body = [ln.split("\t") for ln in lines[1:]]
    check(sorted((r[4], r[3]) for r in body) == [("head_player", "cc"), ("models", "bb")], f"rows {body}")


@test
def test_cli_list_plan_and_dry_run_mention_each_shard():
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx = write_fixture(root)
        dest = root / "dest"
        dry = ""
        for argv in (
            ["list", "--file-list", str(fx)],
            ["plan", "--file-list", str(fx), "--dest", str(dest)],
            ["run", "--shard", "all", "--dry-run", "--file-list", str(fx), "--dest", str(dest)],
        ):
            r = cli(*argv)
            check(r.returncode == 0, f"{argv[0]} exit {r.returncode}: {r.stderr.strip()[:300]}")
            for shard in kx.DEFAULT_SHARDS:
                check(f"shard {shard}" in r.stdout, f"{argv[0]} output misses shard {shard}")
            if argv[0] == "run":
                dry = r.stdout
        for shard in kx.DEFAULT_SHARDS:
            check(re.search(rf"shard {shard}: \d+ files expected, 0 extracted", dry),
                  f"dry-run final line missing for shard {shard}")
        check(not dest.exists() or not any(dest.iterdir()), "--dry-run touched dest")


@test
def test_cli_run_refuses_dest_inside_repo():
    real_root = kx.find_repo_root(HERE)
    dest = real_root / "patch_manager_mods" / "k1_port" / "cr_probe_should_not_exist"
    with tempfile.TemporaryDirectory() as td:
        fx = write_fixture(Path(td))
        r = cli("run", "--shard", "all", "--dry-run", "--file-list", str(fx), "--dest", str(dest))
    check(r.returncode != 0, "run accepted a dest inside the repo")
    check(not dest.exists(), "refused run still created dest")


@test
def test_cli_env_overrides_dest_and_rar():
    with tempfile.TemporaryDirectory() as td:
        root = Path(td)
        fx = write_fixture(root)
        dest = root / "envdest"
        rar = root / "no_such_main.rar"
        r = cli("plan", "--file-list", str(fx),
                env={"K1PORT_CR_DEST": str(dest), "K1PORT_CR_RAR_MAIN": str(rar)})
    check(r.returncode == 0, f"plan exit {r.returncode}: {r.stderr.strip()[:300]}")
    check(str(dest / "head_player") in r.stdout, "K1PORT_CR_DEST not used")
    check(str(rar) in r.stdout and "MISSING" in r.stdout, "K1PORT_CR_RAR_MAIN not used")


FAKE_UNRAR = '''#!{exe}
import os, sys
NAMES, SIZES, RC, LOG, PARTIAL = {names!r}, {sizes!r}, {rc}, {log!r}, {partial}
lst = next((p for p in (a.lstrip("-@") for a in sys.argv[1:]) if os.path.basename(p) == NAMES), None)
if lst is None:
    sys.exit(9)
wanted = open(lst, encoding="utf-8").read().split()
open(LOG, "a", encoding="utf-8").write(" ".join(wanted) + "\\n")
if RC:
    sys.stderr.write("fake unrar failure\\n")
for n in (wanted[:PARTIAL] if RC else wanted):
    dst = os.path.join(os.path.dirname(lst), *n.split("/"))
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    open(dst, "wb").write(b"x" * SIZES[n])
sys.exit(RC)
'''

EXTRACT_ITEMS = [("main", "Override/Pfha03D1.TGA", 1000), ("main", "Override/c_hutt01.mdl", 2000),
                 ("creatures", "Override/c_rancor.mdx", 4000)]


@contextlib.contextmanager
def _fake_unrar(td, rc=0, partial=0):
    """Fake unrar first on PATH; rar stubs at K1PORT_CR_RAR_<PACK>. rc!=0 still writes the first `partial` names of each pack.
    Yields (dest, call_log, manifest_calls)."""
    bindir, log, exe = Path(td) / "bin", Path(td) / "calls.log", Path(td) / "bin" / "unrar"
    bindir.mkdir()
    sizes = {p: s for _pk, p, s in EXTRACT_ITEMS}
    exe.write_text(FAKE_UNRAR.format(exe=sys.executable, names=os.path.basename(str(kx.NAMES_FILE)),
                                     sizes=sizes, rc=rc, log=str(log), partial=partial), encoding="utf-8")
    exe.chmod(0o755)
    env = {"PATH": f"{bindir}{os.pathsep}{os.environ['PATH']}"}
    for pk in ("main", "creatures"):
        rar = Path(td) / f"{pk}.rar"
        rar.write_bytes(b"")
        env[f"K1PORT_CR_RAR_{pk.upper()}"] = str(rar)
    saved = {k: os.environ.get(k) for k in env}
    real_manifest, manifest = kx.write_manifest, []
    os.environ.update(env)
    kx.write_manifest = lambda dest, shard, rows: manifest.append((dest, shard, rows))
    try:
        yield Path(td) / "dest", log, manifest
    finally:
        kx.write_manifest = real_manifest
        for k, v in saved.items():
            if v is None:
                os.environ.pop(k, None)
            else:
                os.environ[k] = v


@test
def test_run_shard_extracts_flattens_and_builds_manifest_rows():
    with tempfile.TemporaryDirectory() as td, _fake_unrar(td) as (dest, log, manifest):
        ret = kx.run_shard("head_player", EXTRACT_ITEMS, dest)
        shard_dir = dest / "head_player"
        check(ret == (3, 3), f"run_shard returned {ret}, want (3, 3)")
        for name, size in (("pfha03d1.tga", 1000), ("c_hutt01.mdl", 2000), ("c_rancor.mdx", 4000)):
            check((shard_dir / name).stat().st_size == size, f"{name} not flattened to lowercase, size {size}")
        asked = sorted(n for line in log.read_text().splitlines() for n in line.split())
        check(asked == sorted(p for _pk, p, _s in EXTRACT_ITEMS), f"unrar asked for {asked}")
        check(len(manifest) == 1 and manifest[0][:2] == (dest, "head_player"), f"manifest calls {manifest}")
        rows = {r[0]: r for r in manifest[0][2]}
        check(sorted(rows) == ["c_hutt01", "c_rancor", "pfha03d1"], f"manifest rows {manifest[0][2]}")
        check(rows["c_hutt01"][1].endswith("mdl") and rows["c_hutt01"][2] == 2000, f"row {rows['c_hutt01']}")
        check(rows["c_hutt01"][3] == hashlib.sha256(b"x" * 2000).hexdigest(), "manifest sha256 mismatch")


@test
def test_run_shard_resume_requests_only_missing_entries():
    with tempfile.TemporaryDirectory() as td, _fake_unrar(td) as (dest, log, manifest):
        kx.run_shard("head_player", EXTRACT_ITEMS, dest)
        (dest / "head_player" / "pfha03d1.tga").unlink()
        log.unlink()
        ret = kx.run_shard("head_player", EXTRACT_ITEMS, dest)
        asked = log.read_text().split()
        check(ret == (3, 3), f"resume run returned {ret}")
        check(asked == ["Override/Pfha03D1.TGA"], f"resume asked unrar for {asked}")
        check(len(manifest) == 2 and len(manifest[1][2]) == 3, "resume manifest lacks the skipped entries")


@test
def test_run_shard_unrar_failure_leaves_entries_missing_and_warns():
    with tempfile.TemporaryDirectory() as td, _fake_unrar(td, rc=2) as (dest, _log, manifest):
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            ret = kx.run_shard("head_player", EXTRACT_ITEMS, dest)
        warn = err.getvalue()
        check(ret == (3, 0), f"failed unrar returned {ret}, want (3, 0)")
        check("WARN: unrar exit 2 (pack main" in warn and "(pack creatures" in warn, f"no WARN per pack: {warn!r}")
        check(manifest[0][2] == [], f"failed run wrote rows {manifest[0][2]}")


@test
def test_run_shard_partial_unrar_failure_counts_only_extracted_entries():
    with tempfile.TemporaryDirectory() as td, _fake_unrar(td, rc=3, partial=1) as (dest, _log, manifest):
        err = io.StringIO()
        with contextlib.redirect_stderr(err):
            ret = kx.run_shard("head_player", EXTRACT_ITEMS, dest)
        shard_dir = dest / "head_player"
        check(ret == (3, 2), f"partial unrar returned {ret}, want (3, 2)")
        check(err.getvalue().count("WARN: unrar exit 3") == 2, f"want one WARN per pack: {err.getvalue()!r}")
        check(sorted(p.name for p in shard_dir.iterdir() if p.name != kx.NAMES_FILE) == ["c_rancor.mdx", "pfha03d1.tga"],
              "partial extract left unexpected files")
        check(sorted(r[0] for r in manifest[0][2]) == ["c_rancor", "pfha03d1"], f"manifest rows {manifest[0][2]}")


if __name__ == "__main__":
    for key in [k for k in os.environ if k.startswith("K1PORT_CR_")]:
        del os.environ[key]
    failed = 0
    for fn in TESTS:
        try:
            fn()
            print(f"PASS {fn.__name__}")
        except Exception as exc:  # noqa: BLE001
            failed += 1
            print(f"FAIL {fn.__name__}: {exc}")
    print(f"{len(TESTS) - failed}/{len(TESTS)} PASS")
    sys.exit(1 if failed else 0)
