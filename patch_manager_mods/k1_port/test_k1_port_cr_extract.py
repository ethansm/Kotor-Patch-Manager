#!/usr/bin/env python3
"""Tests for k1_port_cr_extract.py. Tempdirs only; no rar, no game files, no unrar.

CLI calls run under python -I with K1PORT_CR_* removed from the environment (unless a test sets them).
Run: python3 -u test_k1_port_cr_extract.py   (cwd may be anything; exit 0 = all PASS)
"""
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
