#!/usr/bin/env python3
"""Tests for k1_port_textures.py. Tempdirs only; nothing is written next to the repo.

Runs under system python3 and drives the CLI through the patched pykotor venv (K1PORT_PY, default
~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python). If that interpreter is missing the whole file SKIPs (exit 0).
"""
import hashlib
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_textures.py"
PY = os.environ.get("K1PORT_PY") or str(Path("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python").expanduser())
TESTS = []


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def make_tga(path: Path, w: int = 8, h: int = 8, seed: int = 1) -> Path:
    """Uncompressed 32-bit TGA (header 18 bytes + pixels). read_tpc needs >= 128 bytes to detect TGA."""
    header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, w, h, 32, 8)
    pixels = bytes((i * 37 + seed) % 256 for i in range(w * h * 4))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + pixels)
    return path


def make_tpc(root: Path, tga: Path, out: Path) -> None:
    """Real TPC via pykotor (needs the patched venv); written from the synthetic TGA."""
    code = ("import sys; from pykotor.resource.formats.tpc import read_tpc, write_tpc; "
            "write_tpc(read_tpc(sys.argv[1]), sys.argv[2])")
    out.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run([PY, "-B", "-c", code, str(tga), str(out)], cwd=str(root), env=cli_env(root),
                       capture_output=True, text=True, timeout=300)
    check(r.returncode == 0 and out.exists(), f"make_tpc failed: {r.stderr[-500:]}")


def cli_env(root: Path) -> dict:
    env = dict(os.environ)
    env["K1PORT_LOG_DIR"] = str(root / "pylogs")  # keeps pykotor's logs/ out of the sandbox cwd and the repo
    env["K1PORT_LEDGER"] = str(root / "ledger.json")  # never the real ledger
    return env


def run_cli(root: Path, *args, source: str | None = "vanilla_k1") -> subprocess.CompletedProcess:
    """source=None omits --source-id (the CLI then refuses). Every call gets a tempdir ledger."""
    cmd = [PY, "-B", str(CLI)]
    if source is not None:
        cmd += ["--source-id", source]
    cmd += [str(a) for a in args]
    return subprocess.run(cmd, cwd=str(root), env=cli_env(root), capture_output=True, text=True, timeout=300)


def ledger_entries(root: Path) -> list:
    return json.loads((root / "ledger.json").read_text(encoding="utf-8"))["entries"]


class Skip(Exception):
    """A requirement is missing (the real K1 install). Reported as SKIP and counted as a pass."""


INSTALL = Path("~/.local/share/Steam/steamapps/common/swkotor").expanduser()


def write_map(root: Path, mapping: dict) -> Path:
    p = root / "map.json"
    p.write_text(json.dumps(mapping), encoding="utf-8")
    return p


def expect_ok(r):
    check(r.returncode == 0, f"expected exit 0, got {r.returncode}: {r.stderr[-800:]}")


def expect_refused(r, *needles):
    check(r.returncode == 1, f"expected exit 1, got {r.returncode}: stdout={r.stdout[-300:]} stderr={r.stderr[-800:]}")
    for n in needles:
        check(n in r.stderr, f"stderr lacks {n!r}: {r.stderr[-800:]}")


@test
def help_runs_without_pykotor(root):
    r = subprocess.run([sys.executable, "-B", str(CLI), "--help"], cwd=str(root), capture_output=True, text=True,
                       timeout=60, env=cli_env(root))
    check(r.returncode == 0 and "--map-json" in r.stdout and "--source-id" in r.stdout,
          f"--help failed: {r.stderr[-400:]}")


@test
def happy_path_stages_tga_and_writes_manifest(root):
    src = make_tga(root / "pack" / "sub" / "N_CommM04.TGA")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage, "--pack-name", "kotor1_test")
    expect_ok(r)
    staged = stage / "override" / "k1p_commm04.tga"
    check(staged.exists(), "k1p_commm04.tga not staged")
    check(staged.read_bytes() == src.read_bytes(), "staged bytes differ from source")
    check(sorted(p.name for p in stage.iterdir()) == ["override", "stage_manifest.json"],
          f"unexpected stage contents: {sorted(p.name for p in stage.iterdir())}")
    check(not (root / "logs").exists(), "pykotor logs/ leaked into the CLI cwd")
    run = json.loads(r.stdout)
    e = run["files"][0]
    check(e["sha256"] == sha(src), "manifest sha256 != source sha256")
    check((e["width"], e["height"]) == (8, 8), f"bad dims {e['width']}x{e['height']}")
    check(e["bytes"] == src.stat().st_size, "manifest bytes != source size")
    check(e["source_pack"] == "kotor1_test" and e["source_rel_path"] == "sub/N_CommM04.TGA", f"bad source fields: {e}")
    check(e["old"] == "N_CommM04" and e["new"] == "k1p_commm04" and e["format"], f"bad name/format fields: {e}")
    man = json.loads((stage / "stage_manifest.json").read_text(encoding="utf-8"))
    check(man["files"] == run["files"], "stage_manifest.json entries differ from stdout run manifest")


@test
def tpc_source_found_case_insensitively_keeps_tpc_ext(root):
    tga = make_tga(root / "synthetic.tga")
    src = root / "pack" / "N_TEX.tpc"
    make_tpc(root, tga, src)
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"n_tex": "k1p_tex"}),
                "--stage-dir", stage)
    expect_ok(r)
    staged = stage / "override" / "k1p_tex.tpc"
    check(staged.exists() and staged.read_bytes() == src.read_bytes(), "k1p_tex.tpc missing or bytes differ")
    check(json.loads(r.stdout)["files"][0]["source_pack"] == "pack", "default --pack-name should be the pack dir name")


@test
def tpc_beats_tga_fallback(root):
    tga = make_tga(root / "pack" / "N_Pref.tga")
    make_tpc(root, tga, root / "pack" / "N_Pref.tpc")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_Pref": "k1p_pref"}),
                "--stage-dir", stage)
    expect_ok(r)
    e = json.loads(r.stdout)["files"][0]
    check(e["source_rel_path"] == "N_Pref.tpc", f"picked {e['source_rel_path']}, expected the .tpc")
    check((stage / "override" / "k1p_pref.tpc").exists(), "k1p_pref.tpc not staged")


@test
def manifest_merges_on_second_run(root):
    make_tga(root / "pack" / "N_A.tga")
    make_tga(root / "pack" / "N_B.tga", seed=7)
    stage = root / "stage"
    expect_ok(run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_A": "k1p_a"}),
                      "--stage-dir", stage))
    expect_ok(run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_B": "k1p_b"}),
                      "--stage-dir", stage))
    man = json.loads((stage / "stage_manifest.json").read_text(encoding="utf-8"))
    check([f["new"] for f in man["files"]] == ["k1p_a", "k1p_b"], f"manifest not merged: {man}")
    check((stage / "override" / "k1p_a.tga").exists() and (stage / "override" / "k1p_b.tga").exists(),
          "both staged files should exist")


@test
def name_longer_than_31_refused(root):
    make_tga(root / "pack" / "N_L31.tga")
    make_tga(root / "pack" / "N_L32.tga", seed=3)
    stage = root / "stage"
    ok31 = "a" * 31
    expect_ok(run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_L31": ok31}),
                      "--stage-dir", stage))
    check((stage / "override" / f"{ok31}.tga").exists(), "31-char name should be accepted")
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_L32": "b" * 32}),
                "--stage-dir", stage)
    expect_refused(r, "max is 31")
    check(sorted(p.name for p in (stage / "override").iterdir()) == [f"{ok31}.tga"], "32-char run wrote something")


@test
def invalid_chars_refused_and_nothing_written(root):
    make_tga(root / "pack" / "N_X.tga")
    make_tga(root / "pack" / "N_Y.tga", seed=5)
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack",
                "--map-json", write_map(root, {"N_X": "k1p-bad", "N_Y": "Upper"}), "--stage-dir", stage)
    expect_refused(r, "'k1p-bad' must match", "'Upper' must match")
    check(not stage.exists(), "refused run must not create the stage dir")


@test
def duplicate_new_case_insensitive_refused(root):
    make_tga(root / "pack" / "N_Dup1.tga")
    make_tga(root / "pack" / "N_Dup2.tga", seed=9)
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack",
                "--map-json", write_map(root, {"N_Dup1": "k1p_dup", "N_Dup2": "K1P_DUP"}), "--stage-dir", stage)
    expect_refused(r, "duplicate NEW (case-insensitive)")
    check(not stage.exists(), "refused run must not create the stage dir")


@test
def overwrite_refused(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    (stage / "override").mkdir(parents=True)
    existing = stage / "override" / "k1p_commm04.tga"
    existing.write_bytes(b"old")
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage)
    expect_refused(r, "target exists")
    check(existing.read_bytes() == b"old", "existing staged file was overwritten")
    check(not (stage / "stage_manifest.json").exists(), "refused run must not write a manifest")


@test
def new_name_colliding_with_other_extension_refused(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    (stage / "override").mkdir(parents=True)
    (stage / "override" / "k1p_commm04.tpc").write_bytes(b"x")
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage)
    expect_refused(r, "target exists")
    check(not (stage / "override" / "k1p_commm04.tga").exists(), "tga written despite same-stem tpc")


@test
def missing_source_reported_and_batch_rejected(root):
    make_tga(root / "pack" / "N_Good.tga")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack",
                "--map-json", write_map(root, {"N_Good": "k1p_good", "N_Missing": "k1p_missing"}),
                "--stage-dir", stage)
    expect_refused(r, "missing source: N_Missing")
    check(not stage.exists(), "batch with a missing source must write nothing")


@test
def unreadable_source_reported_and_batch_rejected(root):
    make_tga(root / "pack" / "N_Good.tga")
    (root / "pack" / "N_Bad.tga").write_bytes(b"not a texture")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack",
                "--map-json", write_map(root, {"N_Good": "k1p_good", "N_Bad": "k1p_bad"}), "--stage-dir", stage)
    expect_refused(r, "unreadable texture")
    check(not stage.exists(), "batch with an unreadable source must write nothing")


@test
def multiple_matches_pick_shallowest_with_warning(root):
    make_tga(root / "pack" / "a" / "N_Dup.tga")
    make_tga(root / "pack" / "b" / "c" / "N_Dup.tga", w=8, h=8, seed=2)
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_Dup": "k1p_dup"}),
                "--stage-dir", stage)
    expect_ok(r)
    run = json.loads(r.stdout)
    check(run["files"][0]["source_rel_path"] == "a/N_Dup.tga", f"picked {run['files'][0]['source_rel_path']}")
    check(any("2 matches" in w for w in run["warnings"]), f"no multi-match warning: {run['warnings']}")


@test
def stage_inside_repo_refused(root):
    stage = HERE / "k1port_textures_refusal_probe"  # refused before any mkdir; cleanup only if the guard regresses
    try:
        make_tga(root / "pack" / "N_CommM04.tga")
        r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                    "--stage-dir", stage)
        expect_refused(r, "inside the repo")
        check(not stage.exists(), "guard let the CLI create a stage dir inside the repo")
    finally:
        if stage.exists():
            shutil.rmtree(stage, ignore_errors=True)


@test
def stage_under_steamapps_common_refused_incl_symlink(root):
    game = root / "Games" / "steamapps" / "common" / "KOTOR II"
    game.mkdir(parents=True)
    link = root / "link"
    link.symlink_to(game, target_is_directory=True)
    make_tga(root / "pack" / "N_CommM04.tga")
    for stage in (game / "stage", link / "stage"):  # direct, and reached through a symlink (resolved first)
        r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                    "--stage-dir", stage)
        expect_refused(r, "steamapps/common")
        check(not (game / "stage").exists(), "steamapps stage dir was created")


@test
def stage_under_compatdata_or_named_override_refused(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    cases = [(root / "compatdata" / "208580" / "pfx" / "stage", "compatdata"),
             (root / "game" / "override", "final component is 'override'")]
    for stage, needle in cases:
        r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                    "--stage-dir", stage)
        expect_refused(r, needle)
        check(not stage.exists(), f"refused stage {stage} was created")


@test
def unknown_source_id_refused_nothing_written(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage, source="no_such_source")
    expect_refused(r, "unregistered source id")
    check(not stage.exists(), "refused run must not create the stage dir")
    check(not (root / "ledger.json").exists(), "refused run must not create the ledger")


@test
def missing_source_id_rejected(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage, source=None)
    check(r.returncode != 0 and "--source-id" in r.stderr, f"rc={r.returncode}: {r.stderr[-300:]}")
    check(not stage.exists() and not (root / "ledger.json").exists(), "rejected run wrote something")


@test
def pack_and_install_are_mutually_exclusive(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    r = run_cli(root, "--pack-dir", root / "pack", "--from-install", INSTALL,
                "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}), "--stage-dir", root / "stage")
    check(r.returncode != 0 and "not allowed with" in r.stderr, f"rc={r.returncode}: {r.stderr[-300:]}")
    check(not (root / "stage").exists(), "rejected run wrote something")


@test
def permission_source_flagged_in_manifest_and_ledger(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage, source="upscale_plus_v0.52")
    expect_ok(r)
    e = json.loads(r.stdout)["files"][0]
    check(e["source_id"] == "upscale_plus_v0.52" and e["license_class"] == "permission_required", f"manifest: {e}")
    check(e["requires_author_approval"] is True and e["removal_group"] == "pending_author_approval", f"manifest: {e}")
    led = ledger_entries(root)
    check(len(led) == 1, f"ledger entries: {led}")
    check(led[0]["removal_group"] == "pending_author_approval" and led[0]["author_approved"] is False,
          f"ledger entry not flagged: {led[0]}")
    check(led[0]["kind"] == "texture" and led[0]["old_name"] == "N_CommM04" and led[0]["new_name"] == "k1p_commm04",
          f"ledger names: {led[0]}")


@test
def credit_source_not_flagged(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    stage = root / "stage"
    r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage, source="leilukin_k1_twilek_f_2179")
    expect_ok(r)
    e = json.loads(r.stdout)["files"][0]
    check(e["removal_group"] is None and e["requires_author_approval"] is False, f"manifest: {e}")
    check(isinstance(e["credits"], list) and e["credits"], f"credits missing: {e}")
    led = ledger_entries(root)
    check(led[0]["removal_group"] is None and led[0]["author_approved"] is False, f"ledger: {led[0]}")


@test
def ledger_entries_have_sha256_and_absolute_paths(root):
    make_tga(root / "pack" / "N_A.tga")
    make_tga(root / "pack" / "N_B.tga", seed=7)
    stage = root / "stage"
    expect_ok(run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_A": "k1p_a", "N_B": "k1p_b"}),
                      "--stage-dir", stage))
    led = ledger_entries(root)
    check(sorted(Path(e["file"]).name for e in led) == ["k1p_a.tga", "k1p_b.tga"], f"ledger files: {led}")
    for e in led:
        f = Path(e["file"])
        check(f.is_absolute() and f.is_file(), f"ledger path not an absolute staged file: {e['file']}")
        check(e["sha256"] == sha(f), f"ledger sha256 differs from the staged file {f}")
        check(e["kind"] == "texture" and e["source_id"] == "vanilla_k1" and e["removal_group"] is None, f"{e}")
        check((e["width"], e["height"]) == (8, 8), f"bad dims in ledger: {e}")


@test
def ledger_location_guard_refuses_repo_and_steamapps(root):
    make_tga(root / "pack" / "N_CommM04.tga")
    probe = HERE / "k1port_textures_ledger_refusal_probe.json"  # refused before any write; cleanup if the guard regresses
    game_ledger = root / "Games" / "steamapps" / "common" / "KOTOR II" / "ledger.json"
    try:
        stage = root / "stage"
        r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                    "--stage-dir", stage, "--ledger", probe)
        expect_refused(r, "inside the repo")
        check(not probe.exists() and not stage.exists(), "guard let the CLI write a ledger inside the repo")
        r = run_cli(root, "--pack-dir", root / "pack", "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                    "--stage-dir", stage, "--ledger", game_ledger)
        expect_refused(r, "steamapps/common")
        check(not game_ledger.parent.exists() and not stage.exists(), "steamapps ledger dir was created")
    finally:
        if probe.exists():
            probe.unlink()


@test
def from_install_stages_real_k1_texture(root):
    if not INSTALL.is_dir():
        raise Skip(f"K1 install not found: {INSTALL}")
    stage = root / "stage"
    r = run_cli(root, "--from-install", INSTALL, "--map-json", write_map(root, {"N_CommM04": "k1p_commm04"}),
                "--stage-dir", stage)
    expect_ok(r)
    e = json.loads(r.stdout)["files"][0]
    check((e["width"], e["height"]) == (512, 512), f"N_CommM04 is {e['width']}x{e['height']}, expected 512x512")
    check(e["source_id"] == "vanilla_k1" and e["removal_group"] is None, f"manifest: {e}")
    staged = stage / e["staged_rel_path"]
    check(staged.is_file() and sha(staged) == e["sha256"], "staged bytes differ from the manifest sha256")
    led = ledger_entries(root)
    check(len(led) == 1 and led[0]["file"] == str(staged) and led[0]["removal_group"] is None
          and led[0]["author_approved"] is False, f"ledger entry: {led}")


def main() -> int:
    if not Path(PY).exists():
        print(f"SKIP: patched pykotor python not found at {PY} (set K1PORT_PY, see setup_venv.sh)")
        return 0
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_tex_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}")
            passed += 1  # a missing requirement is not a failure
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
