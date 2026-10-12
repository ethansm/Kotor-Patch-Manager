#!/usr/bin/env python3
"""Tests for k1_port_prune.py. Tempdirs only; the real ledger and stage dirs are never touched.

Every CLI call passes --ledger and K1PORT_LEDGER pointing into the test's tempdir. Runs the CLI under python -I.
Run: python3 -u test_k1_port_prune.py   (cwd may be anything; exit 0 = all PASS)
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
CLI = HERE / "k1_port_prune.py"
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
os.environ.pop("K1PORT_SOURCES", None)
os.environ.pop("K1PORT_LEDGER", None)
import k1_port_prune as kp  # noqa: E402
import k1_port_sources as ks  # noqa: E402
from k1_port_sources import Ledger  # noqa: E402

TESTS = []
UPSCALE = "upscale_plus_v0.52"
VANILLA = "vanilla_k1"
LEILUKIN = "leilukin_k1_twilek_f_2179"


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def sha_of(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def new_stage(root: Path):
    stage = root / "stage"
    stage.mkdir(parents=True, exist_ok=True)
    return stage, Ledger(stage / "ledger.json")


def put(led: Ledger, path: Path, source_id: str, data: bytes, kind: str = "texture") -> Path:
    """Write a real file and register it (sha256 computed) under source_id. Saves the ledger."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    led.add(str(path), kind, source_id, sha256=sha_of(data))
    led.save()
    return path


def raw_entries(ledger_path: Path) -> list:
    return json.loads(ledger_path.read_text(encoding="utf-8"))["entries"]


def entry_files(ledger_path: Path) -> list:
    return [e["file"] for e in raw_entries(ledger_path)]


def run_cli(root: Path, *args, ledger: Path = None):
    led = ledger or (root / "stage" / "ledger.json")
    env = dict(os.environ)
    env.pop("K1PORT_SOURCES", None)
    env["K1PORT_LEDGER"] = str(led)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    return subprocess.run([sys.executable, "-I", "-B", "-u", str(CLI), *args, "--ledger", str(led)],
                          cwd=str(root), env=env, capture_output=True, text=True, timeout=120)


@test
def test_list_table_and_json(root):
    stage, led = new_stage(root)
    a = put(led, stage / "up" / "a.tpc", UPSCALE, b"AAAA")
    b = put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    r = run_cli(root, "list")
    check(r.returncode == 0, f"list rc {r.returncode}: {r.stderr}")
    check("source_id" in r.stdout and "sha256" in r.stdout, f"table header missing: {r.stdout}")
    check(str(a) in r.stdout and str(b) in r.stdout, "list table must name both files")
    check(sha_of(b"AAAA")[:12] in r.stdout, "sha256 prefix missing from table")
    r = run_cli(root, "list", "--json")
    check(r.returncode == 0, f"list --json rc {r.returncode}: {r.stderr}")
    data = json.loads(r.stdout)
    by_file = {e["file"]: e for e in data["entries"]}
    check(by_file[str(a)]["flagged"] is True and by_file[str(a)]["approved"] is False, f"a flags: {by_file[str(a)]}")
    check(by_file[str(a)]["exists"] is True and by_file[str(a)]["sha256"] == sha_of(b"AAAA"), "a exists/sha")
    check(by_file[str(b)]["flagged"] is False and by_file[str(b)]["removal_group"] is None, "b must be unflagged")
    r = run_cli(root, "list", "--flagged-only", "--json")
    flagged = json.loads(r.stdout)["entries"]
    check([e["file"] for e in flagged] == [str(a)], f"--flagged-only should return only a: {flagged}")


@test
def test_env_default_ledger(root):
    stage = root / "env_stage"
    led = Ledger(stage / "ledger.json")
    put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    env = dict(os.environ)
    env["K1PORT_LEDGER"] = str(stage / "ledger.json")
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    env.pop("K1PORT_SOURCES", None)
    r = subprocess.run([sys.executable, "-I", "-B", str(CLI), "list", "--json"], cwd=str(root), env=env,
                       capture_output=True, text=True, timeout=120)
    check(r.returncode == 0, f"env-default list rc {r.returncode}: {r.stderr}")
    data = json.loads(r.stdout)
    check(os.path.realpath(data["ledger"]) == os.path.realpath(stage / "ledger.json"),
          f"env ledger not used: {data['ledger']}")
    check(len(data["entries"]) == 1, "env ledger should hold one entry")


@test
def test_prune_dry_run_changes_nothing(root):
    stage, led = new_stage(root)
    a = put(led, stage / "up" / "a.tpc", UPSCALE, b"AAAA")
    ledger_path = stage / "ledger.json"
    before = ledger_path.read_bytes()
    r = run_cli(root, "prune")
    check(r.returncode == 0, f"dry-run rc {r.returncode}: {r.stdout} {r.stderr}")
    check(a.exists() and a.read_bytes() == b"AAAA", "dry-run must not delete")
    check(ledger_path.read_bytes() == before, "dry-run must not rewrite the ledger")
    check("would delete" in r.stdout and str(a) in r.stdout, f"dry-run should name the file: {r.stdout}")


@test
def test_prune_apply_deletes_only_flagged(root):
    stage, led = new_stage(root)
    a = put(led, stage / "up" / "a.tpc", UPSCALE, b"AAAA")
    b = put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    r = run_cli(root, "prune", "--source-id", VANILLA, "--apply")
    check(r.returncode == 0, f"source-filtered prune rc {r.returncode}: {r.stderr}")
    check(a.exists() and b.exists(), "prune for the unflagged source must delete nothing")
    check(len(entry_files(stage / "ledger.json")) == 2, "no entry may be dropped by a no-op prune")
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 0, f"prune --apply rc {r.returncode}: {r.stdout} {r.stderr}")
    check(not a.exists(), "flagged file must be deleted")
    check(b.exists() and b.read_bytes() == b"BBBB", "unflagged vanilla file must survive")
    check(entry_files(stage / "ledger.json") == [str(b)], f"ledger should keep only b: {entry_files(stage / 'ledger.json')}")
    check("deleted" in r.stdout, f"summary should report deletion: {r.stdout}")


@test
def test_prune_refuses_outside_stage(root):
    stage, led = new_stage(root)
    outside = put(led, root / "outside" / "x.tpc", UPSCALE, b"OUT")
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 1, f"outside-stage prune should exit 1, got {r.returncode}")
    check("REFUSED" in r.stdout and "outside stage root" in r.stdout, f"refusal not reported: {r.stdout}")
    check(outside.exists() and outside.read_bytes() == b"OUT", "file outside stage must not be deleted")
    check(str(outside) in entry_files(stage / "ledger.json"), "refused entry must stay in the ledger")


@test
def test_prune_refuses_symlink_escape(root):
    stage, led = new_stage(root)
    target = put(led, root / "outside" / "target.tpc", UPSCALE, b"TGT")
    link = stage / "link.tpc"
    link.symlink_to(target)
    led.add(str(link), "texture", UPSCALE, sha256=sha_of(b"TGT"))
    led.save()
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 1, f"symlink escape should exit 1, got {r.returncode}: {r.stdout}")
    check("REFUSED" in r.stdout, f"symlink refusal not reported: {r.stdout}")
    check(target.exists() and target.read_bytes() == b"TGT", "symlink target outside stage must survive")
    check(os.path.islink(link), "the symlink itself must survive a refusal")


@test
def test_prune_refuses_repo_path(root):
    repo = root / "repo"
    (repo / ".git").mkdir(parents=True)  # marker only: the stage sits inside a fake repo
    stage = repo / "stage"
    led = Ledger(stage / "ledger.json")
    a = put(led, stage / "x.tpc", UPSCALE, b"REPO")
    r = run_cli(root, "prune", "--apply", ledger=stage / "ledger.json")
    check(r.returncode == 1, f"repo-path prune should exit 1, got {r.returncode}: {r.stdout}")
    check("git repo" in r.stdout, f"repo refusal not reported: {r.stdout}")
    check(a.exists() and a.read_bytes() == b"REPO", "file inside a git repo must not be deleted")
    # classify only (never deletes): the KOTOR-II repo this script lives in must be refused.
    if kp.repo_root_of(HERE) is not None:
        c = kp.classify_entry(str(HERE / "k1_port_sources.py"), HERE)
        check(c["action"] == "refuse" and "git repo" in c["reason"], f"repo file not refused: {c}")


@test
def test_prune_refuses_steam_and_compatdata(root):
    stage, led = new_stage(root)
    a = put(led, stage / "steamapps" / "common" / "x.tpc", UPSCALE, b"STEAM")
    c = put(led, stage / "compatdata" / "y.tpc", UPSCALE, b"PFX")
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 1, f"steam/compatdata prune should exit 1, got {r.returncode}")
    check(r.stdout.count("REFUSED") == 2, f"both paths must be refused: {r.stdout}")
    check(a.exists() and c.exists(), "steamapps/common and compatdata files must survive")


@test
def test_prune_drops_missing(root):
    stage, led = new_stage(root)
    gone = stage / "gone.tpc"
    led.add(str(gone), "texture", UPSCALE, sha256=None)
    led.save()
    r = run_cli(root, "prune")
    check(r.returncode == 0 and "would drop" in r.stdout, f"dry-run should report missing: {r.stdout}")
    check(str(gone) in entry_files(stage / "ledger.json"), "dry-run must keep the missing entry")
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 0, f"prune --apply rc {r.returncode}: {r.stderr}")
    check(entry_files(stage / "ledger.json") == [], "missing file's entry must be dropped")
    check("dropped (missing)" in r.stdout, f"drop not reported: {r.stdout}")


@test
def test_approved_entries_survive_prune(root):
    stage, led = new_stage(root)
    a = put(led, stage / "up" / "a.tpc", UPSCALE, b"AAAA")
    led.mark_approved(UPSCALE, "author replied on the forum, 2026-10-11")
    led.save()
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 0, f"prune rc {r.returncode}: {r.stdout}")
    check(a.exists(), "author-approved file must survive prune")
    check(entry_files(stage / "ledger.json") == [str(a)], "approved entry must stay in the ledger")


@test
def test_approve_cli_flips_flags_prune_skips(root):
    stage, led = new_stage(root)
    a = put(led, stage / "up" / "a.tpc", UPSCALE, b"AAAA")
    r = run_cli(root, "approve", "--source-id", UPSCALE, "--note", "RedRob41 approved in DM 2026-10-11")
    check(r.returncode == 0 and "1 ledger entries" in r.stdout, f"approve rc {r.returncode}: {r.stdout} {r.stderr}")
    entry = raw_entries(stage / "ledger.json")[0]
    check(entry["author_approved"] is True and entry["removal_group"] is None, f"approve must flip flags: {entry}")
    r = run_cli(root, "prune", "--apply")
    check(r.returncode == 0 and a.exists(), "prune after approve must skip the approved file")
    check("nothing to prune" in r.stdout, f"approved entry should not be selected: {r.stdout}")


@test
def test_approve_rules(root):
    stage, led = new_stage(root)
    put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    r = run_cli(root, "approve", "--source-id", "no_such_source", "--note", "x")
    check(r.returncode == 1 and "unregistered source id" in r.stderr, f"unknown source: {r.returncode} {r.stderr}")
    r = run_cli(root, "approve", "--source-id", VANILLA, "--note", "x")
    check(r.returncode == 1 and "does not require author approval" in r.stderr, f"vanilla approve: {r.stderr}")
    r = run_cli(root, "approve", "--source-id", UPSCALE, "--note", "   ")
    check(r.returncode == 1 and "must not be empty" in r.stderr, f"blank note: {r.stderr}")
    r = run_cli(root, "prune", "--source-id", "no_such_source")
    check(r.returncode == 1 and "unregistered source id" in r.stderr, f"prune unknown source: {r.stderr}")


@test
def test_gate_catches_renamed_copy(root):
    stage, led = new_stage(root)
    put(led, stage / "up" / "a.tpc", UPSCALE, b"FLAGGED-CONTENT")
    copy = root / "scan" / "deep" / "renamed.txt"
    copy.parent.mkdir(parents=True)
    copy.write_bytes(b"FLAGGED-CONTENT")
    r = run_cli(root, "gate", str(root / "scan"))
    check(r.returncode == 1, f"renamed flagged copy must block, rc {r.returncode}: {r.stdout}")
    check(str(copy) in r.stdout and "sha256 match" in r.stdout, f"offending path not printed: {r.stdout}")


@test
def test_gate_same_name_unknown_content_warns(root):
    """Flagged entry has a sha256; a same-named file with other, unknown bytes is a warning (exit 0), not a block."""
    stage, led = new_stage(root)
    put(led, stage / "up" / "a.tpc", UPSCALE, b"FLAGGED-CONTENT")
    other = root / "scan" / "a.tpc"
    other.parent.mkdir(parents=True)
    other.write_bytes(b"re-encoded, different bytes")
    r = run_cli(root, "gate", str(other))
    check(r.returncode == 0, f"same-name unknown content must not block, rc {r.returncode}: {r.stdout} {r.stderr}")
    check("BLOCKED" not in r.stdout, f"same-name unknown content must not be BLOCKED: {r.stdout}")
    check("same name as flagged asset, content unknown" in r.stderr and str(other) in r.stderr,
          f"unknown-content warning missing: {r.stderr}")
    r = run_cli(root, "gate", "--strict", str(other))
    check(r.returncode == 1, f"--strict must fail same-name unknown content, rc {r.returncode}")
    check("same name as flagged asset, content unknown" in r.stderr, f"strict run must still warn: {r.stderr}")


@test
def test_gate_same_name_scenario_two_copies(root):
    """Real stage layout: approval-free override/k1p_commm04.tpc and flagged _hd/override/k1p_commm04.tpc."""
    stage, led = new_stage(root)
    clean_bytes, hd_bytes = b"VANILLA-COMMM04", b"UPSCALE-HD-COMMM04"
    clean = put(led, stage / "override" / "k1p_commm04.tpc", VANILLA, clean_bytes)
    hd = put(led, stage / "_hd" / "override" / "k1p_commm04.tpc", UPSCALE, hd_bytes)
    r = run_cli(root, "gate", str(clean))
    check(r.returncode == 0, f"approval-free copy must pass, rc {r.returncode}: {r.stdout} {r.stderr}")
    check("BLOCKED" not in r.stdout and "WARNING" not in r.stderr,
          f"approval-free copy must pass silently: {r.stdout} {r.stderr}")
    r = run_cli(root, "gate", str(hd))
    check(r.returncode == 1, f"flagged HD copy must fail, rc {r.returncode}: {r.stdout} {r.stderr}")
    check("BLOCKED" in r.stdout and "sha256 match" in r.stdout, f"HD copy must be blocked by sha: {r.stdout}")
    r = run_cli(root, "gate", str(stage))
    check(r.returncode == 1, f"gate of the whole stage must fail, rc {r.returncode}: {r.stdout}")
    blocked = [ln for ln in r.stdout.splitlines() if ln.startswith("BLOCKED")]
    check(len(blocked) == 1 and str(hd) in blocked[0], f"only the HD copy may be blocked: {blocked}")
    # renamed copies are judged by content alone
    renamed_hd = root / "scan" / "copied_as_other.tpc"
    renamed_hd.parent.mkdir(parents=True)
    renamed_hd.write_bytes(hd_bytes)
    r = run_cli(root, "gate", str(renamed_hd))
    check(r.returncode == 1 and "sha256 match" in r.stdout, f"renamed flagged copy must block: {r.stdout}")
    renamed_clean = root / "scan" / "copied_clean.tpc"
    renamed_clean.write_bytes(clean_bytes)
    r = run_cli(root, "gate", str(renamed_clean))
    check(r.returncode == 0 and "BLOCKED" not in r.stdout, f"renamed approval-free copy must pass: {r.stdout} {r.stderr}")
    # HD bytes copied over the approval-free path: content is flagged, so the gate must block it
    clean.write_bytes(hd_bytes)
    r = run_cli(root, "gate", str(clean))
    check(r.returncode == 1 and "sha256 match" in r.stdout, f"HD bytes over the clean path must block: {r.stdout}")


@test
def test_gate_no_sha_flagged_entry_still_name_matches(root):
    stage, led = new_stage(root)
    led.add(str(stage / "up" / "legacy.tpc"), "texture", UPSCALE, sha256=None)
    led.save()
    other = root / "scan" / "legacy.tpc"
    other.parent.mkdir(parents=True)
    other.write_bytes(b"anything at all")
    r = run_cli(root, "gate", str(other))
    check(r.returncode == 1 and "BLOCKED" in r.stdout and "name match" in r.stdout,
          f"no-sha flagged entry must basename-match and block: rc {r.returncode}: {r.stdout}")
    check(str(other) in r.stdout, f"offending path not printed: {r.stdout}")


@test
def test_gate_strict_fails_unknown_same_name_and_unregistered(root):
    stage, led = new_stage(root)
    put(led, stage / "up" / "a.tpc", UPSCALE, b"FLAGGED-CONTENT")
    put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    new = root / "scan" / "new.tpc"
    new.parent.mkdir(parents=True)
    new.write_bytes(b"never registered")
    r = run_cli(root, "gate", str(new))
    check(r.returncode == 0 and "unregistered asset" in r.stderr, f"default: unregistered is a warning: {r.stderr}")
    r = run_cli(root, "gate", "--strict", str(new))
    check(r.returncode == 1 and "unregistered asset" in r.stderr, f"--strict must fail unregistered: rc {r.returncode}")
    check("(--strict: warnings fail)" in r.stdout, f"strict summary missing: {r.stdout}")
    same = root / "scan" / "a.tpc"
    same.write_bytes(b"different bytes, same name")
    r = run_cli(root, "gate", "--strict", str(same))
    check(r.returncode == 1 and "content unknown" in r.stderr, f"--strict must fail unknown same-name: rc {r.returncode}")


@test
def test_gate_strict_passes_known_and_clean(root):
    stage, led = new_stage(root)
    vanilla = put(led, stage / "override" / "k1p_commm04.tpc", VANILLA, b"VANILLA-COMMM04")
    put(led, stage / "_hd" / "override" / "k1p_commm04.tpc", UPSCALE, b"UPSCALE-HD-COMMM04")
    known = root / "scan" / "k1p_commm04.tpc"
    known.parent.mkdir(parents=True)
    known.write_bytes(vanilla.read_bytes())
    r = run_cli(root, "gate", "--strict", str(known))
    check(r.returncode == 0, f"--strict must pass known approval-free content: rc {r.returncode}: {r.stdout} {r.stderr}")
    check("WARNING" not in r.stderr and "BLOCKED" not in r.stdout, f"known content must be silent: {r.stderr}")
    clean = root / "scan" / "readme.txt"
    clean.write_bytes(b"nothing to see here")
    r = run_cli(root, "gate", "--strict", str(clean))
    check(r.returncode == 0 and "WARNING" not in r.stderr, f"--strict must pass a clean non-asset: {r.stdout} {r.stderr}")


@test
def test_gate_passes_clean(root):
    stage, led = new_stage(root)
    put(led, stage / "up" / "a.tpc", UPSCALE, b"FLAGGED-CONTENT")
    put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    clean = root / "scan" / "readme.txt"
    clean.parent.mkdir(parents=True)
    clean.write_bytes(b"nothing to see here")
    r = run_cli(root, "gate", str(clean))
    check(r.returncode == 0, f"clean file must pass, rc {r.returncode}: {r.stdout} {r.stderr}")
    check("BLOCKED" not in r.stdout and "WARNING" not in r.stderr, "clean non-asset must not warn or block")


@test
def test_gate_warns_unregistered(root):
    stage, led = new_stage(root)
    put(led, stage / "up" / "a.tpc", UPSCALE, b"FLAGGED-CONTENT")
    vanilla = put(led, stage / "vn" / "b.tpc", VANILLA, b"BBBB")
    new = root / "scan" / "new.tpc"
    new.parent.mkdir(parents=True)
    new.write_bytes(b"never registered")
    known = root / "scan" / "known.tpc"
    known.write_bytes(vanilla.read_bytes())
    r = run_cli(root, "gate", str(new))
    check(r.returncode == 0, f"unregistered asset is a warning, not a block: rc {r.returncode}")
    check("unregistered asset" in r.stderr and str(new) in r.stderr, f"no unregistered warning: {r.stderr}")
    r = run_cli(root, "gate", str(known))
    check(r.returncode == 0 and "unregistered asset" not in r.stderr, "registered vanilla asset must not warn")
    r = run_cli(root, "gate", str(root / "scan" / "missing.tpc"))
    check(r.returncode == 1 and "no such file" in r.stderr, f"missing path must error: {r.stderr}")


@test
def test_adopt_registers_and_rejects(root):
    stage, led = new_stage(root)
    vn = stage / "vn" / "k.tpc"
    vn.parent.mkdir(parents=True)
    vn.write_bytes(b"KKKK")
    r = run_cli(root, "adopt", "--file", str(vn), "--kind", "texture", "--source-id", VANILLA)
    check(r.returncode == 0, f"adopt vanilla rc {r.returncode}: {r.stderr}")
    e = [x for x in raw_entries(stage / "ledger.json") if x["file"] == str(vn)]
    check(len(e) == 1 and e[0]["sha256"] == sha_of(b"KKKK") and e[0]["kind"] == "texture", f"adopt entry: {e}")
    check(e[0]["removal_group"] is None and e[0]["author_approved"] is False, "vanilla adopt must not be flagged")
    up = stage / "up" / "u.mdl"
    up.parent.mkdir(parents=True)
    up.write_bytes(b"UUUU")
    r = run_cli(root, "adopt", "--file", str(up), "--kind", "model", "--source-id", UPSCALE)
    check(r.returncode == 0, f"adopt upscale rc {r.returncode}: {r.stderr}")
    e = [x for x in raw_entries(stage / "ledger.json") if x["file"] == str(up)][0]
    check(e["removal_group"] == "pending_author_approval", f"upscale adopt must be flagged: {e}")
    r = run_cli(root, "adopt", "--file", str(vn), "--kind", "texture", "--source-id", "no_such_source")
    check(r.returncode == 1 and "unregistered source id" in r.stderr, f"unknown source adopt: {r.stderr}")
    outside = root / "outside.tpc"
    outside.write_bytes(b"OOO")
    r = run_cli(root, "adopt", "--file", str(outside), "--kind", "texture", "--source-id", VANILLA)
    check(r.returncode == 1 and "outside stage root" in r.stderr, f"outside adopt must be refused: {r.stderr}")


@test
def test_credits_aggregation(root):
    stage, led = new_stage(root)
    used = [LEILUKIN, VANILLA, UPSCALE]
    led.add(str(stage / "twi" / "lena_h.mdl"), "model", LEILUKIN, sha256=None)
    led.add(str(stage / "twi" / "lena_b.mdl"), "model", LEILUKIN, sha256=None)
    led.add(str(stage / "vn" / "bioware.tpc"), "texture", VANILLA, sha256=None)
    led.add(str(stage / "up" / "up.tpc"), "texture", UPSCALE, sha256=None)
    led.save()
    expected = {}
    for sid in used:
        for c in ks.get_source(sid)["credits"]:
            expected.setdefault(c, set()).add(sid)
    expected_uncredited = sorted(sid for sid in used if not ks.get_source(sid)["credits"])
    r = run_cli(root, "credits", "--json")
    check(r.returncode == 0, f"credits rc {r.returncode}: {r.stderr}")
    data = json.loads(r.stdout)
    got = {row["credit"]: set(row["sources"]) for row in data["credits"]}
    check(got == expected, f"credit -> sources mismatch: got {got}, expected {expected}")
    check("Leilukin" in got and "BioWare" in got, "known credits must be listed")
    check(sorted(data["uncredited_sources"]) == expected_uncredited, f"uncredited: {data['uncredited_sources']}")
    r = run_cli(root, "credits")
    check(r.returncode == 0 and "SpaceAlex" in r.stdout, f"credits table: {r.stdout}")


def main() -> int:
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_prune_test_"))
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
