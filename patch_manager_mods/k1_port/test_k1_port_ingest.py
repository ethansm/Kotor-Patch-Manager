#!/usr/bin/env python3
"""Tests for k1_port_ingest.py. Tempdirs only; nothing is written next to the repo or in the real stage.

Drives the CLI through the patched pykotor venv (K1PORT_PY, default
~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python). If that interpreter is missing the whole file SKIPs (exit 0).
Each run gets a temp registry (K1PORT_SOURCES), a temp ledger (K1PORT_LEDGER), a temp stage (--stage-dir) and a
missing K1 install (--install-k1), so only the two synthetic sources (approval-free and flagged) are in play.
The subprocess tests run no model conversion (--no-model, or dry-run decisions). The model validation gate is
tested in-process (stubbed(): converter, texture stager, TPC reader and validator stubs; nothing real is run).
"""
import contextlib
import io
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
DRIVER = HERE / "k1_port_ingest.py"
PY = os.environ.get("K1PORT_PY") or str(Path("~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python").expanduser())
sys.path.insert(0, str(HERE))  # the in-process tests import the driver and its stdlib libraries
import k1_port_ingest as ing  # noqa: E402  stdlib only at import
import k1_port_sources as ks  # noqa: E402
import k1_port_textures as kt  # noqa: E402
import k1_port_validate as kv  # noqa: E402
TESTS = []
CLASSES = {"vanilla": 0, "open": 1, "credit_required": 2, "unknown": 8, "personal_use_only": 9,
           "permission_required": 10}


def test(fn):
    TESTS.append(fn)
    return fn


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


class Skip(Exception):
    """A requirement is missing. Reported as SKIP and counted as a pass."""


def make_tga(path: Path, w: int, h: int, seed: int) -> Path:
    """Uncompressed 32-bit TGA. read_tpc needs >= 128 bytes to detect TGA."""
    header = struct.pack("<BBBHHBHHHHBB", 0, 0, 2, 0, 0, 0, 0, 0, w, h, 32, 8)
    pixels = bytes((i * 37 + seed) % 256 for i in range(w * h * 4))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + pixels)
    return path


def make_tpcs(root: Path, pairs: list) -> None:
    """Real TPC files via pykotor, one interpreter run. pairs: [(tga_in, tpc_out), ...]."""
    code = ("import sys; from pykotor.resource.formats.tpc import read_tpc, write_tpc\n"
            "args = sys.argv[1:]\n"
            "for i in range(0, len(args), 2):\n"
            "    write_tpc(read_tpc(args[i]), args[i + 1])\n")
    for _src, out in pairs:
        Path(out).parent.mkdir(parents=True, exist_ok=True)
    flat = [str(p) for pair in pairs for p in pair]
    r = subprocess.run([PY, "-B", "-c", code, *flat], cwd=str(root), env=driver_env(root),
                       capture_output=True, text=True, timeout=300)
    check(r.returncode == 0, f"make_tpcs failed: {r.stderr[-500:]}")
    for _src, out in pairs:
        check(Path(out).exists(), f"TPC not written: {out}")


def driver_env(root: Path) -> dict:
    env = dict(os.environ)
    env["K1PORT_LOG_DIR"] = str(root / "pylogs")  # pykotor logs stay out of the sandbox and the repo
    env["K1PORT_LEDGER"] = str(root / "ledger.json")  # never the real ledger
    env["K1PORT_SOURCES"] = str(root / "sources.json")  # never the real registry
    return env


def run_ingest(root: Path, *args) -> subprocess.CompletedProcess:
    """Runs the driver with a temp stage and a missing K1 install unless the caller passes its own."""
    extra = []
    if "--stage-dir" not in args:
        extra += ["--stage-dir", str(root / "stage")]
    if "--install-k1" not in args:
        extra += ["--install-k1", str(root / "no_install")]
    cmd = [PY, "-I", "-B", "-u", str(DRIVER), *extra, *[str(a) for a in args]]
    return subprocess.run(cmd, cwd=str(root), env=driver_env(root), capture_output=True, text=True, timeout=600)


def write_registry(root: Path) -> None:
    reg = {
        "schema": 1,
        "classes": CLASSES,
        "sources": {
            "vanilla_k1": {"name": "vanilla", "author": "BioWare", "license_class": "vanilla",
                           "requires_author_approval": False, "credits": ["BioWare"],
                           "local_dir": str(root / "no_install")},
            "test_credit": {"name": "credit source", "author": "Credit Author", "license_class": "credit_required",
                            "requires_author_approval": False, "credits": ["Credit Author"],
                            "local_dir": str(root / "credit")},
            "test_permission": {"name": "flagged source", "author": "Perm Author",
                                "license_class": "permission_required", "requires_author_approval": True,
                                "credits": ["Perm Author"], "local_dir": str(root / "perm")},
        },
    }
    (root / "sources.json").write_text(json.dumps(reg, indent=2), encoding="utf-8")


def world(root: Path, tex_in=("credit", "perm")) -> dict:
    """Two sources. The approval-free one has a LOWER-quality texture (8x8, nested) and the flagged one a higher
    one (32x32). Both hold a model pair (n_test). tex_in picks which sources hold the texture."""
    write_registry(root)
    for src in ("credit", "perm"):
        for ext in (".mdl", ".mdx"):
            (root / src / f"n_test{ext}").parent.mkdir(parents=True, exist_ok=True)
            (root / src / f"n_test{ext}").write_bytes(b"not a real model")
    tga_small = make_tga(root / "src" / "small.tga", 8, 8, seed=1)
    tga_big = make_tga(root / "src" / "big.tga", 32, 32, seed=2)
    pairs = []
    if "credit" in tex_in:
        pairs.append((tga_small, root / "credit" / "deep" / "N_TestTex.tpc"))
    if "perm" in tex_in:
        pairs.append((tga_big, root / "perm" / "N_TestTex.tpc"))
    if pairs:
        make_tpcs(root, pairs)
    return {"small": root / "credit" / "deep" / "N_TestTex.tpc", "big": root / "perm" / "N_TestTex.tpc"}


def ledger_entries(root: Path) -> list:
    return json.loads((root / "ledger.json").read_text(encoding="utf-8"))["entries"]


def tree(root: Path) -> list:
    """Every file under root except the pykotor log dir: a before/after snapshot."""
    out = []
    for dirpath, _dirs, files in os.walk(root):
        if Path(dirpath).name == "pylogs" or "pylogs" in Path(dirpath).parts:
            continue
        for fn in files:
            out.append(str((Path(dirpath) / fn).relative_to(root)))
    return sorted(out)


def expect_rc(r, want: int, what: str):
    check(r.returncode == want, f"{what}: exit {r.returncode}, wanted {want}\nstdout: {r.stdout[-600:]}\n"
                                f"stderr: {r.stderr[-800:]}")


def texture_decision(summary: dict) -> dict:
    check(len(summary["textures"]) == 1, "expected one texture decision")
    return summary["textures"][0]


@test
def dry_run_prefers_approval_free_over_higher_quality(root):
    world(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json")
    expect_rc(r, 0, "dry-run")
    s = json.loads(r.stdout)
    t = texture_decision(s)
    check(t["chosen"]["source_id"] == "test_credit", f"texture chosen {t['chosen']['source_id']}, want test_credit")
    check(t["chosen"]["quality"] < t["rejected"][0]["quality"], "chosen should be the lower-quality one here")
    rej = {x["source_id"]: x["reason"] for x in t["rejected"]}
    check("requires author approval" in rej.get("test_permission", ""), f"no approval reason: {rej}")
    check(s["model"]["chosen"]["source_id"] == "test_credit", "model should also prefer the approval-free source")
    check(t["status"] == "planned" and not t["hd"], "dry-run texture should be planned with no HD variant")
    check(all(not r_["flagged"] for r_ in s["outputs"]), "approval-free outputs must not be flagged")
    check(s["flagged_files"] == [], "no flagged files expected")


@test
def all_flagged_case_warns(root):
    world(root, tex_in=("perm",))
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json")
    expect_rc(r, 0, "all-flagged dry-run")
    s = json.loads(r.stdout)
    t = texture_decision(s)
    check(t["chosen"]["source_id"] == "test_permission", "only the flagged source holds the texture")
    check(t["chosen"]["requires_author_approval"] is True, "chosen should be flagged")
    check("all candidates require author approval" in (t["warning"] or ""), f"no warning: {t['warning']}")
    check(any(o["flagged"] and o["kind"] == "texture" for o in s["outputs"]), "planned texture should be flagged")
    text = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run")
    expect_rc(text, 0, "all-flagged text dry-run")
    check("WARN all candidates require author approval" in text.stdout, "text output lacks the WARN line")


@test
def also_hd_stages_flagged_higher_quality_variant(root):
    world(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--no-model", "--also-hd",
                   "--json")
    expect_rc(r, 0, "also-hd ingest")
    s = json.loads(r.stdout)
    stage = root / "stage"
    main_file = stage / "override" / "k1p_testtex.tpc"
    hd_file = stage / "_hd" / "override" / "k1p_testtex.tpc"
    check(main_file.exists(), "approval-free texture not staged")
    check(hd_file.exists(), "flagged higher-quality HD variant not staged")
    check(main_file.read_bytes() == root.joinpath("credit", "deep", "N_TestTex.tpc").read_bytes(),
          "main texture must be the approval-free (lower quality) source bytes")
    check(hd_file.read_bytes() == root.joinpath("perm", "N_TestTex.tpc").read_bytes(),
          "HD file must be the flagged (higher quality) source bytes")
    by_file = {e["file"]: e for e in ledger_entries(root)}
    main_e = by_file[str(main_file)]
    hd_e = by_file[str(hd_file)]
    check(main_e["removal_group"] is None and main_e["source_id"] == "test_credit", f"main entry: {main_e}")
    check(hd_e["removal_group"] == "pending_author_approval" and hd_e["source_id"] == "test_permission",
          f"HD entry must be flagged: {hd_e}")
    check(s["flagged_files"] == [str(hd_file)], f"flagged_files: {s['flagged_files']}")
    check(sorted(s["credits_needed"]) == ["Credit Author", "Perm Author"], f"credits: {s['credits_needed']}")
    check(not (stage / "models").exists(), "--no-model must not write models/")


@test
def dry_run_writes_nothing(root):
    world(root)
    before = tree(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--also-hd", "--dry-run")
    expect_rc(r, 0, "dry-run with HD")
    check("dry-run: nothing written" in r.stdout, "text summary should say dry-run")
    check(tree(root) == before, f"dry-run changed the tree: {set(tree(root)) ^ set(before)}")
    check(not (root / "stage").exists() and not (root / "ledger.json").exists(), "stage or ledger created")


@test
def rerun_reports_already_ingested(root):
    world(root)
    args = ["--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--no-model", "--also-hd", "--json"]
    first = run_ingest(root, *args)
    expect_rc(first, 0, "first ingest")
    count = len(ledger_entries(root))
    stage_bytes = (root / "stage" / "override" / "k1p_testtex.tpc").read_bytes()
    second = run_ingest(root, *args)
    expect_rc(second, 0, "second ingest")
    s = json.loads(second.stdout)
    t = texture_decision(s)
    check(t["status"] == "already_ingested", f"texture status {t['status']}")
    check(t["hd"]["status"] == "already_ingested", f"HD status {t['hd']['status']}")
    check(len(ledger_entries(root)) == count, "re-run must not add ledger entries")
    check((root / "stage" / "override" / "k1p_testtex.tpc").read_bytes() == stage_bytes, "re-run rewrote a file")
    check(s["flagged_files"] == [str(root / "stage" / "_hd" / "override" / "k1p_testtex.tpc")],
          f"flagged list after re-run: {s['flagged_files']}")


@test
def ledger_absent_output_is_flagged(root):
    """An existing staged output the ledger never recorded has unknown provenance: flag it, never call it clean."""
    world(root)
    out_file = root / "stage" / "override" / "k1p_testtex.tpc"
    out_file.parent.mkdir(parents=True)
    out_file.write_bytes(b"pre-placed, not in the ledger")
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--no-model", "--json")
    expect_rc(r, 0, "ingest over a ledger-absent output")
    s = json.loads(r.stdout)
    check(texture_decision(s)["status"] == "already_ingested", f"status: {texture_decision(s)['status']}")
    check(s["flagged_files"] == [str(out_file)], f"ledger-absent output must be flagged: {s['flagged_files']}")
    row_ = [o for o in s["outputs"] if o["path"] == str(out_file)]
    check(len(row_) == 1 and row_[0]["flagged"] and row_[0]["removal_group"] == ks.REMOVAL_PENDING,
          f"row: {row_}")
    check(any(str(out_file) in n and "not in the ledger; flagged for review" in n for n in s["notes"]),
          f"note must say flagged: {s['notes']}")
    check(out_file.read_bytes() == b"pre-placed, not in the ledger", "existing output must not be rewritten")


@test
def no_candidates_is_a_clear_error(root):
    world(root)
    before = tree(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_Missing=k1p_missing", "--dry-run")
    expect_rc(r, 1, "missing texture")
    check("no candidates for N_Missing" in r.stderr, f"stderr lacks the clear error: {r.stderr[-400:]}")
    check(tree(root) == before, "refused run changed the tree")


@test
def bad_texture_argument_exit_2(root):
    world(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "no_equals_sign", "--dry-run")
    expect_rc(r, 2, "malformed --texture")


@test
def help_runs_under_system_python(root):
    r = subprocess.run([sys.executable, "-I", "-B", str(DRIVER), "--help"], capture_output=True, text=True,
                       timeout=120)
    expect_rc(r, 0, "--help under system python")
    check("usage:" in r.stdout, "no usage text")


VARIANT = "Optional - Upscaled Textures"


def variant_world(root: Path, model_dirs: tuple) -> None:
    """One approval-free source (credit). A root-level 8x8 .tga N_TestTex next to a 32x32 .tpc N_TestTex under
    VARIANT/ (the bug shape: a .tpc normally beats a .tga). n_test.mdl/.mdx go in each dir of model_dirs
    ("" = the source root)."""
    write_registry(root)
    src = root / "credit"
    make_tga(src / "N_TestTex.tga", 8, 8, seed=1)
    make_tpcs(root, [(make_tga(root / "src" / "big.tga", 32, 32, seed=2), src / VARIANT / "N_TestTex.tpc")])
    for d in model_dirs:
        for ext in (".mdl", ".mdx"):
            p = src / d / f"n_test{ext}"
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_bytes(b"not a real model")


def variant_notes(s: dict) -> list:
    return [n for n in s["notes"] if "chosen from variant folder" in n]


@test
def root_file_beats_variant_folder(root):
    variant_world(root, model_dirs=("", VARIANT))
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json")
    expect_rc(r, 0, "variant dry-run")
    s = json.loads(r.stdout)
    t = texture_decision(s)
    chosen = Path(t["chosen"]["path"])
    check(chosen.name == "N_TestTex.tga" and chosen.parent == root / "credit",
          f"texture should be the root .tga, got {chosen}")
    check(VARIANT not in chosen.parts, "variant folder must not win over the root .tga")
    mdl = Path(s["model"]["chosen"]["path"])
    check(mdl.parent == root / "credit", f"root model should beat the variant model, got {mdl}")
    check(variant_notes(s) == [], f"no variant pick, so no note: {s['notes']}")


@test
def allow_variant_folders_restores_tpc_preference(root):
    variant_world(root, model_dirs=())
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json",
                   "--no-model", "--allow-variant-folders")
    expect_rc(r, 0, "allow-variant dry-run")
    s = json.loads(r.stdout)
    t = texture_decision(s)
    chosen = Path(t["chosen"]["path"])
    check(chosen.name == "N_TestTex.tpc" and VARIANT in chosen.parts,
          f"with --allow-variant-folders the variant .tpc should win, got {chosen}")
    check(variant_notes(s) == [], f"allowed variant pick must not be noted: {s['notes']}")


@test
def variant_only_model_is_still_a_candidate_with_note(root):
    variant_world(root, model_dirs=(VARIANT,))
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json")
    expect_rc(r, 0, "variant-only model dry-run")
    s = json.loads(r.stdout)
    check(s["model"] is not None, "a model present only under a variant folder must still be a candidate")
    mdl = Path(s["model"]["chosen"]["path"])
    check(VARIANT in mdl.parts and mdl.name == "n_test.mdl", f"variant model expected, got {mdl}")
    check(len(variant_notes(s)) == 1, f"want one variant note, notes: {s['notes']}")
    check("credit" in variant_notes(s)[0] and "--allow-variant-folders" in variant_notes(s)[0],
          f"note lacks the source or the flag: {variant_notes(s)[0]}")
    text = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run")
    expect_rc(text, 0, "variant-only model text dry-run")
    check("--allow-variant-folders" in text.stdout, "text output must show the variant note")


MDL_BYTES, MDX_BYTES = b"synthetic mdl", b"synthetic mdx"
FINDING = kv.Finding("BAD_POINTER", "stub: MDX array runs outside the file", 0x40)


@contextlib.contextmanager
def patched(obj, name, value):
    """Swaps obj.name for the duration of the block."""
    old = getattr(obj, name)
    setattr(obj, name, value)
    try:
        yield
    finally:
        setattr(obj, name, old)


def _register(ledger: Path, file: Path, kind: str, source_id: str) -> dict:
    led = ks.Ledger(ledger)
    entry = led.add(str(file), kind, source_id)
    led.save()
    return entry


def _not_tpc(raw: bytes):
    """Stands in for pykotor's read_tpc: rejects every file, so the texture quality is its byte size."""
    raise ValueError("stub reader: not a TPC")


calls_map: list = []  # supermodel_map argument of each fake run_model call


def fake_run_model(calls: list):
    """Stands in for k1_port_model.py: writes a synthetic MDL/MDX pair and registers it as the converter does."""
    def run(stage, ledger, install, cand, resref, new, pairs, allow_overlay=False, supermodel_map=None):
        calls_map.append(supermodel_map)
        out_dir = stage / ing.MODEL_DIR
        out_dir.mkdir(parents=True, exist_ok=True)
        (out_dir / f"{new}.mdl").write_bytes(MDL_BYTES)
        (out_dir / f"{new}.mdx").write_bytes(MDX_BYTES)
        entry = _register(ledger, out_dir / f"{new}.mdl", "model", cand["source_id"])
        _register(ledger, out_dir / f"{new}.mdx", "model", cand["source_id"])
        calls.append(new)
        return {"out_dir": str(out_dir), "source_id": cand["source_id"], "removal_group": entry["removal_group"],
                "credits": entry["credits"], "match": True, "compare": {"tex": True}}
    return run


def fake_stage_texture(stage, ledger, install, cand, old, new, tmp, allow_overlay=False) -> dict:
    """Stands in for k1_port_textures.py: writes one synthetic TPC under stage/override and registers it."""
    path = stage / "override" / f"{new}.tpc"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"synthetic tpc")
    entry = _register(ledger, path, "texture", cand["source_id"])
    return {"stage_dir": str(stage), "files": [{"staged_rel_path": f"override/{path.name}",
                                               "source_id": entry["source_id"],
                                               "removal_group": entry["removal_group"],
                                               "credits": entry["credits"]}]}


@contextlib.contextmanager
def stubbed(root: Path):
    """In-process stand-ins: the registry in root, the converter, the texture stager and the TPC reader.
    Yields the list of model conversions run. Nothing outside root is written."""
    calls: list = []
    real_load = ks.load_registry
    with contextlib.ExitStack() as st:
        st.enter_context(patched(ks, "load_registry", lambda path=None: real_load(root / "sources.json")))
        st.enter_context(patched(ing, "run_model", fake_run_model(calls)))
        st.enter_context(patched(ing, "stage_texture", fake_stage_texture))
        st.enter_context(patched(kt, "import_read_tpc", lambda errs: _not_tpc))
        yield calls


def ingest_main(root: Path, *extra: str) -> SimpleNamespace:
    """Runs the driver's main() in-process for the synthetic model. Returns returncode, stdout and stderr."""
    argv = ["--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--stage-dir", str(root / "stage"),
            "--install-k1", str(root / "no_install"), "--ledger", str(root / "ledger.json"), *extra]
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = ing.main(argv)
    return SimpleNamespace(returncode=rc, stdout=out.getvalue(), stderr=err.getvalue())


def model_world(root: Path) -> None:
    """One approval-free source (test_credit) holding n_test.mdl, n_test.mdx and N_TestTex.tpc. The bytes are
    placeholders: the stubs stand in for the tools that would read them."""
    write_registry(root)
    src = root / "credit"
    src.mkdir(parents=True, exist_ok=True)
    for name in ("n_test.mdl", "n_test.mdx", "N_TestTex.tpc"):
        (src / name).write_bytes(b"placeholder")


@test
def validation_finding_rejects_model_and_registers_nothing(root):
    model_world(root)
    with stubbed(root) as calls, patched(kv, "validate", lambda *a, **k: [FINDING]):
        r = ingest_main(root, "--json")
        expect_rc(r, 1, "validation finding")
        s = json.loads(r.stdout)
        m = s["model"]
        check(m["status"] == "rejected", f"model status {m['status']}")
        check(m["validation"]["status"] == "failed", f"validation: {m['validation']}")
        check(m["validation"]["findings"] == [{"code": "BAD_POINTER", "message": FINDING.message, "offset": 0x40}],
              f"findings not reported: {m['validation']}")
        check(any("BAD_POINTER" in e for e in s["errors"]), f"error lacks the finding code: {s['errors']}")
        models = root / "stage" / "models"
        check(not (models / "k1p_n_test.mdl").exists() and not (models / "k1p_n_test.mdx").exists(),
              "rejected model files must be removed")
        entries = ledger_entries(root)
        check(not [e for e in entries if e["kind"] == "model"], f"rejected model is registered: {entries}")
        check([e["kind"] for e in entries] == ["texture"], f"only the texture should be registered: {entries}")
        check((root / "stage" / "override" / "k1p_testtex.tpc").exists(), "the staged texture should stay")
        check(all(o["kind"] == "texture" for o in s["outputs"]), f"model rows in report: {s['outputs']}")
        # A re-run converts and validates the model again (no already_ingested residue); text names the finding.
        r2 = ingest_main(root)
        expect_rc(r2, 1, "re-run after rejection")
        check(len(calls) == 2, f"converter should run again, ran {len(calls)} times")
        check("finding BAD_POINTER @0x40" in r2.stdout, f"text report lacks the finding: {r2.stdout[-400:]}")
        check("rejected by validation" in r2.stderr, f"error lacks the rejection: {r2.stderr[-300:]}")


@test
def skip_validate_registers_model_with_warning(root):
    model_world(root)
    seen: list = []

    def finding(*a, **k):
        seen.append(a)
        return [FINDING]

    with stubbed(root), patched(kv, "validate", finding):
        r = ingest_main(root, "--json", "--skip-validate")
    expect_rc(r, 0, "--skip-validate ingest")
    check(seen == [], "--skip-validate must not run the validator")
    check("WARNING" in r.stderr and "--skip-validate" in r.stderr, f"no loud warning: {r.stderr!r}")
    s = json.loads(r.stdout)
    check(s["model"]["status"] == "staged" and s["model"]["validation"]["status"] == "skipped",
          f"model {s['model']['status']} / {s['model']['validation']}")
    check(any("--skip-validate" in n for n in s["notes"]), f"no note: {s['notes']}")
    models = sorted(Path(e["file"]).name for e in ledger_entries(root) if e["kind"] == "model")
    check(models == ["k1p_n_test.mdl", "k1p_n_test.mdx"], f"model entries: {models}")
    check((root / "stage" / "models" / "k1p_n_test.mdl").exists(), "the skipped model stays on disk")


@test
def clean_validation_registers_model(root):
    model_world(root)
    seen: list = []

    def clean(mdl, mdx=None, **kw):
        seen.append((mdl, mdx, kw))
        return []

    with stubbed(root), patched(kv, "validate", clean):
        r = ingest_main(root, "--json")
    expect_rc(r, 0, "clean ingest")
    check(len(seen) == 1, f"validator calls: {len(seen)}")
    mdl, mdx, kw = seen[0]
    check(mdl == MDL_BYTES and mdx == MDX_BYTES, "validator must receive the written MDL and MDX bytes")
    check(kw == {"game": "K2", "require_lower_k1p": True}, f"validator options: {kw}")
    s = json.loads(r.stdout)
    check(s["model"]["status"] == "staged" and s["model"]["validation"]["status"] == "passed",
          f"model {s['model']}")
    check("WARNING" not in r.stderr, f"clean run warned: {r.stderr!r}")
    models = sorted(Path(e["file"]).name for e in ledger_entries(root) if e["kind"] == "model")
    check(models == ["k1p_n_test.mdl", "k1p_n_test.mdx"], f"model entries: {models}")


@test
def supermodel_map_reaches_the_model_tool(root):
    model_world(root)
    calls_map.clear()
    with stubbed(root), patched(kv, "validate", lambda mdl, mdx=None, **kw: []):
        r = ingest_main(root, "--json", "--supermodel-map", "P_OldBB=k1p_newbb")
    expect_rc(r, 0, "ingest with --supermodel-map")
    check(calls_map == [["P_OldBB=k1p_newbb"]], f"run_model supermodel_map: {calls_map}")
    s = subprocess.run([PY, "-I", "-B", str(DRIVER), "--help"], capture_output=True, text=True, timeout=60)
    check("--supermodel-map" in s.stdout, "help lacks --supermodel-map")


@test
def overlay_install_refused_without_flag(root):
    world(root)
    ov = root / "overlay"
    ov.mkdir()
    (ov / "K1_HD_OVERLAY.json").write_text(json.dumps({"source_id": "test_permission"}), encoding="utf-8")
    before = tree(root)
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--install-k1", ov,
                   "--dry-run", "--json")
    errors = " ".join(json.loads(r.stdout)["errors"])
    check(r.returncode == 1 and "K1 HD overlay" in errors and "--allow-overlay-install" in errors,
          f"overlay not refused: rc={r.returncode} out={r.stdout[-400:]}")
    check(tree(root) == before, "refused run wrote files")


@test
def prefer_source_flips_choice_to_flagged_source(root):
    world(root)
    base = ["--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--dry-run", "--json"]
    plain = texture_decision(json.loads(run_ingest(root, *base).stdout))
    check(plain["chosen"]["source_id"] == "test_credit", "without the flag the approval-free source must win")
    r = run_ingest(root, *base, "--prefer-source", "test_permission")
    expect_rc(r, 0, "prefer")
    s = json.loads(r.stdout)
    t = texture_decision(s)
    check(t["chosen"]["source_id"] == "test_permission", f"prefer ignored: {t['chosen']['source_id']}")
    check(t["chosen"]["requires_author_approval"], "preferred flagged pick lost its flag")
    check(any(x["source_id"] == "test_credit" and "prefer-source" in x["reason"] for x in t["rejected"]),
          f"no prefer reason: {t['rejected']}")
    check(s["model"]["chosen"]["source_id"] == "test_credit", "--prefer-source must not change the model choice")
    check(any(o["flagged"] for o in s["outputs"]), "preferred flagged texture should be listed as flagged")


@test
def updates_folder_beats_same_name_in_base_shard(root):
    write_registry(root)
    base = make_tga(root / "src" / "a.tga", 8, 8, seed=1)
    upd = make_tga(root / "src" / "b.tga", 8, 8, seed=2)
    make_tpcs(root, [(base, root / "credit" / "party" / "N_TestTex.tpc"),
                     (upd, root / "credit" / "updates" / "N_TestTex.tpc")])
    r = run_ingest(root, "--resref", "n_test", "--texture", "N_TestTex=k1p_testtex", "--no-model", "--dry-run",
                   "--json")
    expect_rc(r, 0, "dry-run")
    path = texture_decision(json.loads(r.stdout))["chosen"]["path"]
    check("/updates/" in path, f"updates/ should win the same-name tie, chose {path}")


def main() -> int:
    if not Path(PY).exists():
        print(f"SKIP: patched pykotor python not found at {PY} (set K1PORT_PY, see setup_venv.sh)")
        return 0
    passed = 0
    for fn in TESTS:
        root = Path(tempfile.mkdtemp(prefix="k1port_ingest_test_"))
        try:
            fn(root)
            print(f"PASS {fn.__name__}")
            passed += 1
        except Skip as e:
            print(f"SKIP {fn.__name__}: {e}")
            passed += 1
        except Exception as e:  # noqa: BLE001  report every failure, keep going
            print(f"FAIL {fn.__name__}: {type(e).__name__}: {e}")
        finally:
            shutil.rmtree(root, ignore_errors=True)
    print(f"{passed}/{len(TESTS)} PASS")
    return 0 if passed == len(TESTS) else 1


if __name__ == "__main__":
    sys.exit(main())
