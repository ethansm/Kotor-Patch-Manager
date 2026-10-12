#!/usr/bin/env python3
"""Tests for k1_port_probe.py. Tempdirs only: the real game dir and real stage are never touched.

Every CLI call gets --stage, --game-dir and --proc-root pointing into the test's tempdir, so the
running-game check reads a fake /proc tree. The CLI runs under python -I.
Run: python3 -u test_k1_port_probe.py   (cwd may be anything; exit 0 = all PASS)
"""
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
CLI = HERE / "k1_port_probe.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
os.environ.pop("K1PORT_SOURCES", None)
os.environ.pop("K1PORT_LEDGER", None)
import k1_port_prune as kp  # noqa: E402
import test_k1_port_validate as tv  # noqa: E402  (synthetic MDL/MDX builder)
from k1_port_sources import Ledger  # noqa: E402

TESTS = []
VANILLA = "vanilla_k1"
UPSCALE = "upscale_plus_v0.52"
BASE_2DA = b"2DA V2.b\n\nlabel\n17 N_CommM\n"
PROBE_2DA = b"2DA V2.b\n\nlabel\n17 N_CommM\n812 K1P_CommM_Probe\n"
BASE_SHA = hashlib.sha256(BASE_2DA).hexdigest()
CREATED = ["k1p_commm.mdl", "k1p_commm.mdx", "k1p_commm04.tpc", "c_ithorian003.utc"]


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sha_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def make_world(root: Path, *, tex="k1p_commm04", flagged=False, base=True) -> dict:
    stage, game, proc = root / "stage", root / "game", root / "proc"
    for d in (stage / "models", stage / "override", stage / "probe_T12h" / "override",
              game / "override", proc):
        d.mkdir(parents=True, exist_ok=True)
    (game / "chitin.key").write_bytes(b"KEY")
    (game / "override" / "bystander.utc").write_bytes(b"UTC bystander, not ours")
    if base:
        (game / "override" / "appearance.2da").write_bytes(BASE_2DA)
    mdl, mdx, _ = tv.build(model="k1p_commm", tex=tex)
    (stage / "models" / "k1p_commm.mdl").write_bytes(mdl)
    (stage / "models" / "k1p_commm.mdx").write_bytes(mdx)
    (stage / "override" / "k1p_commm04.tpc").write_bytes(b"TPC probe texture bytes")
    (stage / "probe_T12h" / "override" / "appearance.2da").write_bytes(PROBE_2DA)
    (stage / "probe_T12h" / "override" / "c_ithorian003.utc").write_bytes(b"UTC probe npc")
    led = Ledger(stage / "ledger.json")
    for rel, kind in (("models/k1p_commm.mdl", "model"), ("models/k1p_commm.mdx", "model"),
                      ("override/k1p_commm04.tpc", "texture")):
        p = (stage / rel).resolve()
        source = UPSCALE if (flagged and rel.endswith(".mdl")) else VANILLA
        led.add(str(p), kind, source, sha256=kp.sha256_of(p))
    led.save()
    return {"root": root, "stage": stage, "game": game, "proc": proc}


def snapshot(root: Path) -> dict:
    out = {}
    for p in sorted(root.rglob("*")):
        rel = str(p.relative_to(root))
        out[rel] = p.read_bytes() if p.is_file() else None
    return out


def run(root: Path, *args):
    env = dict(os.environ)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    env.pop("K1PORT_LEDGER", None)
    env.pop("K1PORT_SOURCES", None)
    return subprocess.run([sys.executable, "-I", "-B", "-u", str(CLI), *args],
                          cwd=str(root), env=env, capture_output=True, text=True, timeout=120)


def common(w: dict) -> list:
    return ["--stage", str(w["stage"]), "--game-dir", str(w["game"]), "--proc-root", str(w["proc"])]


def game_args(w: dict) -> list:
    return ["--game-dir", str(w["game"]), "--proc-root", str(w["proc"])]


def manifest_of(w: dict) -> Path:
    ptr = w["game"] / "backups" / "k1p_probe_latest.json"
    return Path(json.loads(ptr.read_text(encoding="utf-8"))["manifest"])


def load_manifest(w: dict) -> dict:
    return json.loads(manifest_of(w).read_text(encoding="utf-8"))


def override_names(w: dict) -> set:
    return {p.name for p in (w["game"] / "override").iterdir()}


def install_ok(root: Path) -> dict:
    w = make_world(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    return w


@test
def test_plan_writes_nothing_and_is_default(root):
    w = make_world(root)
    before = snapshot(root)
    r = run(root, *common(w), "--base-sha256", BASE_SHA)  # no subcommand: plan
    check(r.returncode == 0, f"plan rc {r.returncode}: {r.stderr}")
    check("dry run" in r.stdout and "verdict: READY" in r.stdout, f"plan verdict missing: {r.stdout}")
    check("would copy 5 file(s)" in r.stdout, f"plan should list 5 files: {r.stdout}")
    r = run(root, "plan", *common(w))  # no --base-sha256: the replaced file must be reported as blocked
    check(r.returncode == 0 and "BLOCK target override/appearance.2da" in r.stdout, f"plan w/o base: {r.stdout}")
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA)  # no --yes: dry run
    check(r.returncode == 0 and "dry run" in r.stdout, f"install dry run: {r.stdout} {r.stderr}")
    check(snapshot(root) == before, "plan and install without --yes must write nothing")
    check(not (w["game"] / "backups").exists(), "no backups dir may be created by a dry run")


@test
def test_install_blocked_by_validator(root):
    w = make_world(root, tex="K1P_COMMM04")  # uppercase texture name: k1p requires lowercase
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 1, f"validator finding must refuse: rc {r.returncode} {r.stdout}")
    check("BLOCK validate k1p_commm (.mdl+.mdx)" in r.stdout and "NAME_NOT_LOWERCASE" in r.stdout,
          f"validator block not reported: {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_install_blocked_by_flagged_file(root):
    w = make_world(root, flagged=True)  # the staged .mdl is ledger-flagged (pending author approval)
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 1, f"flagged file must refuse: rc {r.returncode} {r.stdout}")
    check("BLOCK prune gate" in r.stdout and "blocked 1" in r.stdout, f"gate block not reported: {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_install_blocked_without_ledger(root):
    w = make_world(root)
    (w["stage"] / "ledger.json").unlink()  # an absent ledger would make the gate pass everything
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 1 and "BLOCK stage ledger" in r.stdout, f"missing ledger must refuse: {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_install_blocked_by_running_game(root):
    w = make_world(root)
    (w["proc"] / "4242").mkdir()
    (w["proc"] / "4242" / "cmdline").write_bytes(b"wine\0C:\\\\GOG\\\\KOTOR2\\\\swkotor2.exe\0")
    (w["proc"] / "4243").mkdir()
    (w["proc"] / "4243" / "cmdline").write_bytes(b"python3\0k1_port_probe.py\0")  # decoy: must not block
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 1, f"running game must refuse: rc {r.returncode} {r.stdout}")
    check("BLOCK swkotor2 not running: pids [4242]" in r.stdout, f"running block not reported: {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_install_blocked_by_foreign_file(root):
    w = make_world(root)
    foreign = w["game"] / "override" / "k1p_commm.mdl"
    foreign.write_bytes(b"someone else's model")
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 1, f"foreign file must refuse: rc {r.returncode} {r.stdout}")
    check("BLOCK target override/k1p_commm.mdl" in r.stdout, f"foreign block not reported: {r.stdout}")
    check(foreign.read_bytes() == b"someone else's model", "foreign file must be untouched")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_install_success_manifest_sha(root):
    w = make_world(root)
    r = run(root, "status", *game_args(w))
    check(r.returncode == 0 and "NOT INSTALLED" in r.stdout, f"status before install: {r.stdout}")
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    m = load_manifest(w)
    check(m["schema"] == "k1p_probe/1" and m["base_sha256"] == BASE_SHA, f"manifest header: {m}")
    by_name = {e["name"]: e for e in m["installed"]}
    check(sorted(by_name) == sorted(CREATED + ["appearance.2da"]), f"manifest names: {sorted(by_name)}")
    for name in CREATED:
        e = by_name[name]
        check(e["action"] == "created" and e["backup"] is None, f"{name} must be created: {e}")
        staged = next(p for p in (w["stage"] / "models" / name, w["stage"] / "override" / name,
                                  w["stage"] / "probe_T12h" / "override" / name) if p.is_file())
        check(e["sha256"] == kp.sha256_of(staged), f"{name} manifest sha != staged sha")
        check(kp.sha256_of(w["game"] / "override" / name) == e["sha256"], f"{name} installed sha drift")
    app = by_name["appearance.2da"]
    check(app["action"] == "replaced" and app["backup"] == "original/appearance.2da", f"appearance entry: {app}")
    check(app["backup_sha256"] == BASE_SHA, "backup sha must be the base sha")
    check((manifest_of(w).parent / "original" / "appearance.2da").read_bytes() == BASE_2DA, "backup content")
    check((w["game"] / "override" / "appearance.2da").read_bytes() == PROBE_2DA, "probe appearance installed")
    check((w["game"] / "override" / "bystander.utc").read_bytes() == b"UTC bystander, not ours", "bystander touched")
    r = run(root, "status", *game_args(w))
    check(r.returncode == 0 and "state: INSTALLED" in r.stdout, f"status after install: {r.stdout}")


@test
def test_rollback_removes_exactly_created(root):
    w = install_ok(root)
    r = run(root, "rollback", *game_args(w))
    check(r.returncode == 0 and "dry run" in r.stdout, f"rollback dry run: {r.stdout}")
    check(set(CREATED) <= override_names(w), "dry run rollback must not remove anything")
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 0, f"rollback rc {r.returncode}: {r.stdout} {r.stderr}")
    check(override_names(w) == {"appearance.2da", "bystander.utc"}, f"override after rollback: {override_names(w)}")
    check((w["game"] / "override" / "appearance.2da").read_bytes() == BASE_2DA, "base appearance restored")
    check((w["game"] / "override" / "bystander.utc").read_bytes() == b"UTC bystander, not ours", "bystander touched")
    check("rolled_back" in load_manifest(w), "manifest must be marked rolled back")


@test
def test_rollback_idempotent(root):
    w = install_ok(root)
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 0, f"first rollback rc {r.returncode}: {r.stdout}")
    after_first = snapshot(root)
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 0 and "already rolled back" in r.stdout, f"second rollback: {r.stdout}")
    check(snapshot(root) == after_first, "second rollback must be a no-op")
    # Without the marker, every entry is already absent or already restored, so the run must change no file.
    m = load_manifest(w)
    m.pop("rolled_back")
    manifest_of(w).write_text(json.dumps(m), encoding="utf-8")
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 0 and "already absent" in r.stdout and "already restored" in r.stdout,
          f"unmarked rerun must report per-entry no-ops: rc {r.returncode} {r.stdout}")
    files_only = lambda snap: {k: v for k, v in snap.items() if k.startswith("game/override")}  # noqa: E731
    check(files_only(snapshot(root)) == files_only(after_first), "unmarked rerun must not change override/")


@test
def test_replaced_base_mismatch_blocked(root):
    w = make_world(root)
    before = snapshot(root)
    r = run(root, "install", *common(w), "--base-sha256", "0" * 64, "--yes")
    check(r.returncode == 1 and "live sha256" in r.stdout and "!= --base-sha256" in r.stdout,
          f"wrong base must refuse: rc {r.returncode} {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")
    r = run(root, "install", *common(w), "--yes")  # no --base-sha256 at all
    check(r.returncode == 1 and "pass --base-sha256" in r.stdout, f"missing base must refuse: {r.stdout}")
    check(snapshot(root) == before, "refused install must write nothing")


@test
def test_sha_drift_refuses_then_force(root):
    w = install_ok(root)
    drifted = w["game"] / "override" / "k1p_commm04.tpc"
    drifted.write_bytes(b"hand edited texture")
    r = run(root, "status", *game_args(w))
    check("DRIFT" in r.stdout and "drift" in r.stdout, f"status must report drift: {r.stdout}")
    before = snapshot(root)
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 1 and "override/k1p_commm04.tpc: sha256 differs" in r.stdout,
          f"drift must refuse: rc {r.returncode} {r.stdout}")
    check(snapshot(root) == before, "refused rollback must change nothing (all or nothing)")
    r = run(root, "rollback", *game_args(w), "--yes", "--force")
    check(r.returncode == 0, f"forced rollback rc {r.returncode}: {r.stdout} {r.stderr}")
    check(not drifted.exists(), "forced rollback removes the drifted file")
    check(override_names(w) == {"appearance.2da", "bystander.utc"}, f"override after forced: {override_names(w)}")
    check((w["game"] / "override" / "appearance.2da").read_bytes() == BASE_2DA, "base restored after force")


def make_saves(w: dict) -> dict:
    """Fake Steam cloudsaves: one user dir with each slot as a directory of files. Returns slot -> src dir."""
    out = {}
    for slot in ("000000 - QUICKSAVE", "000001 - AUTOSAVE"):
        d = w["game"] / "cloudsaves" / "76561198000000001" / slot
        (d / "nested").mkdir(parents=True, exist_ok=True)
        (d / "savegame.sav").write_bytes(f"save data {slot}".encode("utf-8"))
        (d / "nested" / "screen.tga").write_bytes(b"\x00\x01" + slot.encode("utf-8"))
        out[slot] = d
    return out


def tree_hash_of(root: Path) -> str:
    """The spec restated: sha256 over 'relpath NUL file-sha256 LF' lines, sorted by relpath."""
    rows = sorted((p.relative_to(root).as_posix(), sha_of(p.read_bytes()))
                  for p in root.rglob("*") if p.is_file())
    h = hashlib.sha256()
    for rel, sha in rows:
        h.update(f"{rel}\0{sha}\n".encode("utf-8"))
    return h.hexdigest()


def has_line(text: str, *needles: str) -> bool:
    return any(all(n in line for n in needles) for line in text.splitlines())


@test
def test_save_backup_copies_verifies_and_records(root):
    w = make_world(root)
    slots = make_saves(w)
    saves_before = snapshot(w["game"] / "cloudsaves")
    r = run(root, "plan", *common(w), "--base-sha256", BASE_SHA)
    check(r.returncode == 0 and all(has_line(r.stdout, "PASS", f"save slot {s}") for s in slots),
          f"plan must PASS both save slots: {r.stdout}")
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    m = load_manifest(w)
    by_slot = {s["slot"]: s for s in m.get("saves", [])}
    check(sorted(by_slot) == sorted(slots), f"manifest saves: {m.get('saves')}")
    bdir = manifest_of(w).parent
    for slot, src in slots.items():
        e = by_slot[slot]
        check(e["src"] == str(src) and e["backup"] == f"saves/{slot}", f"save entry paths: {e}")
        check(e["files"] == 2, f"save entry file count: {e}")
        check(e["tree_sha256"] == tree_hash_of(src), f"manifest tree hash != source tree hash: {slot}")
        check(tree_hash_of(bdir / "saves" / slot) == e["tree_sha256"], f"backup tree hash != source: {slot}")
    check(snapshot(w["game"] / "cloudsaves") == saves_before, "install must leave the save source untouched")
    check((w["game"] / "override" / "appearance.2da").read_bytes() == PROBE_2DA, "override install still happens")


@test
def test_status_reports_save_backups(root):
    w = make_world(root)
    slots = make_saves(w)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    r = run(root, "status", *game_args(w))
    check(all(has_line(r.stdout, "present", s) for s in slots), f"status must show both backups present: {r.stdout}")
    shutil.rmtree(manifest_of(w).parent / "saves" / "000001 - AUTOSAVE")
    r = run(root, "status", *game_args(w))
    check(has_line(r.stdout, "MISSING", "000001 - AUTOSAVE") and has_line(r.stdout, "present", "000000 - QUICKSAVE"),
          f"status must flag the removed backup: {r.stdout}")


@test
def test_save_absent_warns_and_install_succeeds(root):
    w = make_world(root)  # no cloudsaves/ at all
    r = run(root, "plan", *common(w), "--base-sha256", BASE_SHA)
    check(r.returncode == 0 and "verdict: READY" in r.stdout and has_line(r.stdout, "WARN", "save slot 000000 - QUICKSAVE"),
          f"absent cloudsaves must WARN and stay READY: {r.stdout}")
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    check(load_manifest(w).get("saves") == [], f"no save entries expected: {load_manifest(w)}")
    check((w["game"] / "override" / "k1p_commm04.tpc").is_file(), "install must still copy the files")


@test
def test_rollback_leaves_saves_and_prints_cp_hints(root):
    w = make_world(root)
    slots = make_saves(w)
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    bdir = manifest_of(w).parent
    saves_before = snapshot(w["game"] / "cloudsaves")
    backups_before = snapshot(bdir / "saves")
    hints = [f'cp -a "{bdir / "saves" / slot}" "{src.parent}/"' for slot, src in slots.items()]
    r = run(root, "rollback", *game_args(w))  # dry run still prints the hints
    check(r.returncode == 0 and all(h in r.stdout for h in hints), f"dry-run rollback hints: {r.stdout}")
    r = run(root, "rollback", *game_args(w), "--yes")
    check(r.returncode == 0 and all(h in r.stdout for h in hints),
          f"rollback hints: rc {r.returncode} {r.stdout} {r.stderr}")
    check(snapshot(w["game"] / "cloudsaves") == saves_before, "rollback must not touch the save slots")
    check(snapshot(bdir / "saves") == backups_before, "rollback must not touch the save backups")
    check(override_names(w) == {"appearance.2da", "bystander.utc"}, f"override after rollback: {override_names(w)}")


@test
def test_no_save_backup_skips(root):
    w = make_world(root)
    make_saves(w)
    r = run(root, "plan", *common(w), "--base-sha256", BASE_SHA, "--no-save-backup")
    check(r.returncode == 0 and "skipped by --no-save-backup" in r.stdout and "save slot 000000" not in r.stdout,
          f"plan with --no-save-backup: {r.stdout}")
    r = run(root, "install", *common(w), "--base-sha256", BASE_SHA, "--yes", "--no-save-backup")
    check(r.returncode == 0, f"install rc {r.returncode}: {r.stdout} {r.stderr}")
    check(not (manifest_of(w).parent / "saves").exists(), "--no-save-backup must not create a saves dir")
    check(load_manifest(w).get("saves") == [], f"no save entries expected: {load_manifest(w)}")


@test
def test_failed_save_backup_aborts_before_any_override(root):
    import k1_port_probe as kprobe  # in-process so the copy can be corrupted; the CLI path is covered above
    w = make_world(root)
    make_saves(w)
    saves_before = snapshot(w["game"] / "cloudsaves")
    override_before = snapshot(w["game"] / "override")
    args = kprobe.build_parser().parse_args(["install", *common(w), "--base-sha256", BASE_SHA, "--yes"])
    plan = kprobe.compute_plan(args, "20260101_0000")
    check(not plan.failed() and len(plan.saves) == 2, f"plan must be clean with 2 save slots: {plan.checks}")
    real_copytree = kprobe.shutil.copytree

    def corrupting_copytree(src, dst, *a, **kw):
        out = real_copytree(src, dst, *a, **kw)
        if Path(src).name == "000001 - AUTOSAVE":  # the copy diverges from the source after the copy
            (Path(dst) / "savegame.sav").write_bytes(b"corrupted during copy")
        return out

    kprobe.shutil.copytree = corrupting_copytree
    try:
        kprobe.install_files(plan, w["game"], w["stage"], plan.base)
    except kprobe.ProbeError as e:
        msg = str(e)
    else:
        raise AssertionError("a save copy that fails verification must abort the install")
    finally:
        kprobe.shutil.copytree = real_copytree
    check("install undone after failure" in msg and "save backup of" in msg, f"abort message: {msg}")
    check(override_names(w) == {"appearance.2da", "bystander.utc"}, f"no override file may be created: {override_names(w)}")
    check(snapshot(w["game"] / "override") == override_before, "override/ must be untouched")
    check(snapshot(w["game"] / "cloudsaves") == saves_before, "source saves must be untouched")
    bdir = plan.backup_dir
    check(not (bdir / "saves").exists() or not any((bdir / "saves").iterdir()), "failed save copies must be removed")
    check(not (bdir / "manifest.json").exists() and not (w["game"] / "backups" / "k1p_probe_latest.json").exists(),
          "no manifest or pointer may be written")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_probe_test_"))
        try:
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
