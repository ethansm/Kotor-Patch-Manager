#!/usr/bin/env python3
"""Tests for k1_port_import.py (install-time K1 -> K2 character importer driver).

Plain python3, stdlib only: no pykotor, no numpy, no game install, no Proton. A fake K1 dir and a fake K2 dir live in
a tempdir. The step scripts (resolve, ingest, char_build, tex_tpc, prune, validate, mdlops and the driver's own
_twoda helper) run for real as subprocesses, but K1PORT_PY / REPO_VENV_PY point at a tiny fake interpreter that
answers each script with plausible output files. Fake 2DA files are JSON ({"headers": [...], "rows": [[...]]}).

Run: python3 -u test_k1_port_import.py   (exit 0 = all PASS)
"""
from __future__ import annotations

import contextlib
import io
import json
import os
import struct
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "kvoice"))
import k1_port_import as imp  # noqa: E402
import k1_port_pack as kp  # noqa: E402
import k1_port_scan as kscan  # noqa: E402
import test_k1_port_dialog as tdlg  # noqa: E402  node/graph builders + the JSON backend of the fake interpreter's _dialog helper
import test_k1_port_scan as tscan  # noqa: E402  K1Data + FixtureReader (the fake interpreter's _scan helper reads those fixtures)

FAKE_PY = '''#!{python}
import hashlib, json, os, pathlib, sys

a = sys.argv[1:]
if os.environ.get("FAKE_LOG"):
    with open(os.environ["FAKE_LOG"], "a") as fh:
        fh.write(json.dumps(a) + "\\n")
while a and a[0] in ("-I", "-B", "-u", "-E", "-S"):
    a.pop(0)
if a and a[0] == "--version":
    print("Python 3.12.fake")
    sys.exit(0)
if a and a[0] == "-c":
    mod = "pykotor" if "pykotor" in a[1] else "numpy"
    if mod in os.environ.get("FAKE_MISSING", "").split(","):
        print("ModuleNotFoundError: No module named '%s'" % mod, file=sys.stderr)
        sys.exit(1)
    sys.exit(0)
script = pathlib.Path(a[0]).name
args = a[1:]


def opt(name, default=None):
    return args[args.index(name) + 1] if name in args else default


def multi(name):
    return [args[i + 1] for i, x in enumerate(args) if x == name]


def after(name):
    out = []
    if name in args:
        for x in args[args.index(name) + 1:]:
            if x.startswith("--"):
                break
            out.append(x)
    return out


def sha(b):
    return hashlib.sha256(b).hexdigest()


def read2da(p):
    return json.loads(pathlib.Path(p).read_text())


def write2da(p, t):
    pathlib.Path(p).write_text(json.dumps(t))


if script == "k1_port_resolve.py":
    k1 = pathlib.Path(opt("--k1-dir"))
    items = [x for x in args if "." in x and not x.startswith("/")]
    rows, missing = [], []
    for it in sorted(set(items)):
        stem, ext = it.rsplit(".", 1)
        for origin, rel, lic in (("override", "Override", "unverified_override"), ("chitin", "_bif", "vanilla_k1")):
            f = k1 / rel / it
            if f.is_file():
                b = f.read_bytes()
                rows.append({"resref": stem, "ext": ext, "origin": origin, "path": rel + "/" + it, "sha256": sha(b),
                             "licence": lic, "flagged": lic != "vanilla_k1"})
                if ext == "mdl" and "--scan" in args:
                    rows[-1]["contains"] = [n for n in multi("--scan") if b"\\0" + n.encode() + b"\\0" in b.lower()]
                break
        else:
            missing.append(it)
    print(json.dumps({"k1_dir": str(k1), "items": rows, "missing": missing}))
    sys.exit(1 if missing else 0)

if script == "k1_port_ingest.py":
    resref, new = opt("--resref"), opt("--new-resref")
    stage = pathlib.Path(opt("--stage-dir"))
    no_model = "--no-model" in args
    cr = "characters_revisited_main" in after("--prefer-source") and os.environ.get("FAKE_CR") == "1"
    if os.environ.get("FAKE_INGEST_FAIL") == resref:
        print(json.dumps({"status": "failed", "errors": ["model %s rejected by validation (1 finding(s): BAD)" % resref]}))
        sys.exit(1)
    outs, texs = [], []
    ext = os.environ.get("FAKE_TEX_EXT", ".tga")
    for t in multi("--texture"):
        old, nw = t.split("=")
        p = stage / "override" / (nw.lower() + ext)
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_bytes(os.environ.get("FAKE_TEX_BYTES", "FAKETGA:" + old).encode("latin1"))
        sid = "characters_revisited_main" if cr else "vanilla_k1"
        outs.append({"path": str(p), "kind": "texture", "status": "staged", "source_id": sid,
                     "removal_group": "pending_author_approval" if cr else None, "flagged": cr})
        texs.append({"name": old, "new": nw, "chosen": {"source_id": sid, "requires_author_approval": cr}})
    if not no_model:
        (stage / "models").mkdir(parents=True, exist_ok=True)
        for e in (".mdl", ".mdx"):
            p = stage / "models" / (new + e)
            p.write_bytes(("MODEL:" + resref + e).encode())
            outs.append({"path": str(p), "kind": "model", "status": "staged", "source_id": "vanilla_k1",
                         "removal_group": None, "flagged": False})
    led = stage / "ledger.json"
    if not led.exists():
        led.write_text(json.dumps({"schema": 1, "entries": [], "approvals": {}}))
    print(json.dumps({"status": "ok", "resref": resref, "new_resref": new, "textures": texs,
                      "model": None if no_model else {"chosen": {"source_id": "vanilla_k1"}}, "outputs": outs,
                      "flagged_files": [o["path"] for o in outs if o["flagged"]], "credits_needed": [], "notes": [],
                      "errors": []}))
    sys.exit(0)

if script == "k1_port_char_build.py":
    game = pathlib.Path(opt("--game-dir"))
    chain = opt("--chain-from")
    base = pathlib.Path(chain) if chain else game / "override"
    app, heads = read2da(base / "appearance.2da"), read2da(base / "heads.2da")
    nr, nh = int(opt("--new-row")), int(opt("--new-head-row"))
    if len(app["rows"]) != nr:
        print("error: live appearance.2da has %d rows but --new-row is %d" % (len(app["rows"]), nr), file=sys.stderr)
        sys.exit(1)
    if len(heads["rows"]) != nh:
        print("error: live heads.2da has %d rows but --new-head-row is %d" % (len(heads["rows"]), nh), file=sys.stderr)
        sys.exit(1)
    sst = opt("--soundset-table")
    snd_path = pathlib.Path(sst) if sst else game / "override" / "soundset.2da"
    if snd_path.is_file() and int(opt("--soundset")) >= len(read2da(snd_path)["rows"]):
        print("error: soundset %s is outside soundset.2da 0..%d" % (opt("--soundset"), len(read2da(snd_path)["rows"]) - 1), file=sys.stderr)
        sys.exit(1)
    app["rows"].append([opt("--label"), opt("--body-b"), opt("--body-a"), opt("--body-a"), opt("--body-b"),
                        opt("--body-b"), str(nh), "", "", "", "B"])
    heads["rows"].append([opt("--head"), "", ""])
    out = pathlib.Path(opt("--out"))
    out.mkdir(parents=True, exist_ok=True)
    write2da(out / "appearance.2da", app)
    write2da(out / "heads.2da", heads)
    (out / (opt("--utc") + ".utc")).write_bytes(("UTC:" + opt("--utc") + ":" + opt("--name") + ":ss=" + opt("--soundset")).encode())
    (out / "char_build.json").write_text("{}")
    print(json.dumps({"appearance_row": nr, "heads_row": nh, "chained_from": chain, "sha256": {"appearance.2da": "x"}}))
    sys.exit(0)

if script == "k1_port_tex_tpc.py":
    src, dst = pathlib.Path(args[1]), pathlib.Path(args[2])
    b = b"FAKETPC:" + src.read_bytes()
    if "--txi" in args:
        b += b"|TXI:" + pathlib.Path(opt("--txi")).read_bytes()
    if "--force-opaque" in args:
        b += b"|OPAQUE"
    dst.write_bytes(b)
    print("wrote", dst)
    sys.exit(0)

if script == "k1_port_prune.py":
    if args[0] == "gate":
        blocked = 1 if os.environ.get("FAKE_GATE_BLOCK") else 0
        if blocked:
            print("BLOCKED  /x/k1p_a.tga  sha256 match")
        print("gate: scanned 3 file(s), blocked %d, warned 0" % blocked)
        sys.exit(1 if blocked else 0)
    print("adopted")
    sys.exit(0)

if script == "k1_port_validate.py":
    sys.exit(1 if os.environ.get("FAKE_VALIDATE_FAIL") else 0)

if script == "k1_port_mdlops.py":
    if args[0] == "pass2":
        run = pathlib.Path(opt("--run"))
        (run / "pass2").mkdir(parents=True, exist_ok=True)
        for pair in after("--map"):
            nw = pair.split("=")[1]
            (run / "pass2" / (nw + "-ascii-k2-bin.mdl")).write_bytes(("MDLOPS:" + nw + ".mdl").encode())
            (run / "pass2" / (nw + "-ascii-k2-bin.mdx")).write_bytes(("MDLOPS:" + nw + ".mdx").encode())
    print("ok", args[0])
    sys.exit(0)

if script == "k1_port_import.py" and args and args[0] == "_twoda":
    op = args[1]
    if op == "height":
        print(json.dumps({"height": len(read2da(args[2])["rows"])}))
    elif op == "rows":
        table, rest = args[2].split("=", 1)
        path, row = rest.rsplit(":", 1)
        t = read2da(path)
        print(json.dumps({table: {row: dict(zip(t["headers"], t["rows"][int(row)]))}}))
    elif op == "pad":
        t = read2da(args[2])
        while len(t["rows"]) < int(args[4]):
            t["rows"].append([""] * len(t["headers"]))
        write2da(args[3], t)
        print(json.dumps({"height": len(t["rows"])}))
    elif op == "soundset":
        t = read2da(args[2])
        row = int(args[4])
        if len(t["rows"]) > row:
            print("error: _twoda soundset: base table has %d rows, above the planned row %d" % (len(t["rows"]), row), file=sys.stderr)
            sys.exit(1)
        while len(t["rows"]) < row:
            t["rows"].append([""] * len(t["headers"]))
        cells = {"label": args[5], "resref": args[6], "strref": "-1", "gender": args[7]}
        t["rows"].append([cells.get(h, "") for h in t["headers"]])
        write2da(args[3], t)
        print(json.dumps({"height": len(t["rows"])}))
    sys.exit(0)

if script == "k1_port_import.py" and args and args[0] == "_scan":
    sys.path.insert(0, "{here}")
    import k1_port_scan, test_k1_port_scan
    sys.exit(k1_port_scan.helper_main(args[1:], reader_factory=test_k1_port_scan.FixtureReader))

if script == "k1_port_import.py" and args and args[0] == "_dialog":
    sys.path.insert(0, "{here}")
    import k1_port_dialog, test_k1_port_dialog
    sys.exit(k1_port_dialog.helper_main(args[1:], backend_factory=test_k1_port_dialog.FakeBackend))

if script == "k1_port_import.py" and args and args[0] == "_audio":
    import base64
    if args[1] == "ssf":
        k1 = pathlib.Path(args[2])
        for rel in ("_bif", "Override"):
            f = k1 / rel / (args[3] + ".ssf")
            if f.is_file():
                print("pykotor log noise")
                print(json.dumps({"data_b64": base64.b64encode(f.read_bytes()).decode()}))
                sys.exit(0)
        print("error: _audio ssf: %s.ssf not found" % args[3], file=sys.stderr)
        sys.exit(1)
    if args[1] == "tlk":
        tlk = json.loads(pathlib.Path(args[2]).read_text())
        print(json.dumps({r: tlk.get(r) for r in args[3:]}))
        sys.exit(0)

print("fake interpreter: unhandled script %s %s" % (script, args), file=sys.stderr)
sys.exit(3)
'''

APP_HEADERS = ["label", "race", "modela", "texa", "modelb", "texb", "normalhead", "backuphead", "texaevil", "texbevil", "modeltype"]
HEAD_HEADERS = ["head", "alttexture", "headtexe"]
LIVE_APP, LIVE_HEAD = 20, 5

RECIPE = """\
[character]
id = "{id}"
display_name = "{name}"
tag = "k1p_{id}"
utc = "k1p_{id}"
dlg = ""
soundset = 11
template_row = 17
gender = 0

[models]
converter = "{conv}"
body_a = {{ k1 = "p_{id}ba", k2 = "k1p_{id}ba" }}
body_b = {{ k1 = "p_{id}bb", k2 = "k1p_{id}bb" }}
head   = {{ k1 = "p_{id}h",  k2 = "k1p_{id}h" }}

[[textures]]
k1 = "p_{id}ba01"
k2 = "k1p_{id}ba01"
force_opaque = true

[[textures]]
k1 = "p_{id}bb01"
k2 = "k1p_{id}bb01"
force_opaque = true

[[textures]]
k1 = "p_{id}h01"
k2 = "k1p_{id}h01"
force_opaque = true
txi = "clamp 1"
"""


def snapshot(root: Path, skip=("fake.log",)) -> dict:
    snap = {}
    for dirpath, dirs, files in os.walk(root):
        for n in dirs + files:
            if n in skip:
                continue
            p = Path(dirpath) / n
            st = p.lstat()
            snap[str(p.relative_to(root))] = (st.st_size, st.st_mtime_ns, p.is_dir())
    return snap


class Base(unittest.TestCase):
    chars = ("tst", "tsu")
    converters = {}
    extra_build = ""
    override_items: tuple = ()

    def setUp(self):
        self._td = tempfile.TemporaryDirectory(prefix="k1port_import_test_")
        self.addCleanup(self._td.cleanup)
        self.root = Path(self._td.name).resolve()
        self.k1, self.k2 = self.root / "k1", self.root / "k2"
        self.out = self.root / "out"
        self.recipes = self.root / "recipes"
        self.log = self.root / "fake.log"
        self.fakepy = self.root / "fakepy"
        self.fakepy.write_text(FAKE_PY.replace("{python}", sys.executable).replace("{here}", str(HERE)))
        self.fakepy.chmod(0o755)
        self.recipes.mkdir()
        for cid in self.chars:
            (self.recipes / f"{cid}.toml").write_text(
                RECIPE.format(id=cid, name=cid.capitalize(), conv=self.converters.get(cid, "pykotor")))
        (self.k1 / "_bif").mkdir(parents=True)
        (self.k1 / "Override").mkdir()
        (self.k1 / "chitin.key").write_bytes(b"KEY")
        for cid in self.chars:
            for stem in (f"p_{cid}ba", f"p_{cid}bb", f"p_{cid}h"):
                for ext in ("mdl", "mdx"):
                    self.put_k1(f"{stem}.{ext}", f"K1:{stem}.{ext}\0{stem.upper()}01\0".encode())
                self.put_k1(f"{stem}01.tpc", f"K1TEX:{stem}01".encode())
        (self.k2 / "override").mkdir(parents=True)
        (self.k2 / "patches").mkdir()
        (self.k2 / "swkotor2.exe").write_bytes(b"MZ")
        (self.k2 / "chitin.key").write_bytes(b"KEY")
        self.write_table("appearance.2da", APP_HEADERS, LIVE_APP)
        self.write_table("heads.2da", HEAD_HEADERS, LIVE_HEAD)
        self.config = self.root / "k1_imports.toml"
        self.write_config()
        self.env = mock.patch.dict(os.environ, {"K1PORT_PY": str(self.fakepy), "REPO_VENV_PY": str(self.fakepy),
                                                "K1PORT_SYS_PY": str(self.fakepy), "FAKE_LOG": str(self.log)})
        self.env.start()
        self.addCleanup(self.env.stop)
        for var in ("FAKE_MISSING", "FAKE_CR", "FAKE_INGEST_FAIL", "FAKE_GATE_BLOCK", "FAKE_TEX_EXT", "FAKE_TEX_BYTES",
                    "FAKE_VALIDATE_FAIL", "K1PORT_CR_DEST"):
            os.environ.pop(var, None)

    # -- fixtures
    def put_k1(self, name, data, where="_bif"):
        (self.k1 / where / name).write_bytes(data)

    def write_table(self, name, headers, n, base=None):
        rows = [[f"r{i}"] * len(headers) for i in range(n)]
        (Path(base or self.k2 / "override") / name).write_text(json.dumps({"headers": headers, "rows": rows}))

    def write_config(self, out=None, build_extra=None, chars=None):
        chars = chars or list(self.chars)
        self.config.write_text(textwrap.dedent(f"""\
            [k1]
            dir = "{self.k1}"
            [k2]
            dir = "{self.k2}"
            [build]
            out = "{out or self.out}"
            characters = {json.dumps(chars)}
            recipes_dir = "{self.recipes}"
            {self.extra_build if build_extra is None else build_extra}
            """))

    # -- running
    def run_cli(self, *argv, config=True):
        args = list(argv)
        if config:
            args += ["--config", str(self.config)]
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            rc = imp.main(args)
        return rc, out.getvalue(), err.getvalue()

    def calls(self):
        if not self.log.exists():
            return []
        return [json.loads(x) for x in self.log.read_text().splitlines()]

    def script_calls(self, name):
        return [c for c in self.calls() if any(Path(x).name == name for x in c)]

    def load_pack(self, cid):
        return kp.read_pack((self.out / f"{cid}.k2pack").read_bytes())

    def manifest(self, cid):
        return json.loads((self.out / f"{cid}.import_manifest.json").read_text())

    def build_ok(self, *extra):
        rc, out, err = self.run_cli("build", *extra)
        self.assertEqual(rc, 0, out + err)
        return out, err


class DoctorTests(Base):
    def test_all_good(self):
        rc, out, err = self.run_cli("doctor")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("OK   K1 dir", out)
        self.assertIn("OK   k1port python", out)
        self.assertIn("doctor: 0 failure(s)", out)

    def test_missing_config(self):
        rc, out, err = self.run_cli("doctor", "--config", str(self.root / "nope.toml"), config=False)
        self.assertEqual(rc, 1)
        self.assertIn("FAIL config", out)
        self.assertIn("fix:", out)

    def test_bad_k1_and_k2_dirs(self):
        (self.k1 / "chitin.key").unlink()
        (self.k2 / "swkotor2.exe").unlink()
        (self.k2 / "chitin.key").unlink()
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL K1 dir", out)
        self.assertIn("no chitin.key", out)
        self.assertIn("FAIL K2 dir", out)
        self.assertIn("no swkotor2.exe or chitin.key", out)
        self.assertEqual(out.count("fix:"), 2)

    def test_python_and_numpy_failures(self):
        os.environ["FAKE_MISSING"] = "pykotor,numpy"
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL k1port python", out)
        self.assertIn("import pykotor failed", out)
        self.assertIn("setup_venv.sh", out)
        self.assertIn("FAIL repo .venv python", out)
        self.assertIn("import numpy failed", out)

    def test_missing_interpreters(self):
        os.environ["K1PORT_PY"] = str(self.root / "no_such_python")
        os.environ["REPO_VENV_PY"] = str(self.root / "no_such_venv_python")
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("no_such_python not found", out)
        self.assertIn("no_such_venv_python not found", out)

    def test_missing_live_tables(self):
        (self.k2 / "override" / "heads.2da").unlink()
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL K2 heads.2da", out)

    def test_unsafe_out_dir_fails_doctor(self):
        (self.root / "work" / ".git").mkdir(parents=True)
        self.write_config(out=self.root / "work" / "packs")
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL out dir", out)
        self.assertIn("git work tree", out)

    def test_juhani_char_warns(self):
        (self.k2 / "patches" / "juhani-char.dll").write_bytes(b"MZ")
        (self.k2 / "patch_config.toml").write_text('[[patches]]\nid = "juhani-char"\ndll = "patches/juhani-char.dll"\n')
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 0, out)
        self.assertIn("WARN hand-path characters", out)
        self.assertIn("juhani-char", out)
        self.assertIn("818-820", out)

    def test_mdlops_tools_only_when_a_recipe_uses_them(self):
        import k1_port_mdlops as km
        with mock.patch.object(km, "PROTON", self.root / "no_proton"):
            rc, out, _ = self.run_cli("doctor")
            self.assertEqual(rc, 0, out)
            self.assertNotIn("Proton", out)
            (self.recipes / "tsu.toml").write_text(RECIPE.format(id="tsu", name="Tsu", conv="mdlops"))
            rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL Proton (converter mdlops: tsu)", out)
        self.assertIn("no_proton not found", out)


class SafetyTests(Base):
    def assert_refused(self, out_dir, needle):
        self.write_config(out=out_dir)
        before = (snapshot(self.k1), snapshot(self.k2))
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 2, out + err)
        self.assertIn(needle, err)
        self.assertFalse(Path(out_dir).exists())
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)
        self.assertEqual(self.script_calls("k1_port_ingest.py"), [])

    def test_out_inside_git_tree(self):
        (self.root / "repo" / ".git").mkdir(parents=True)
        self.assert_refused(self.root / "repo" / "sub" / "packs", "git work tree")

    def test_out_inside_git_worktree_file(self):
        (self.root / "wt").mkdir()
        (self.root / "wt" / ".git").write_text("gitdir: /elsewhere\n")  # a worktree has a .git FILE
        self.assert_refused(self.root / "wt" / "packs", "git work tree")

    def test_out_under_steamapps_common(self):
        self.assert_refused(self.root / "steamapps" / "common" / "x" / "packs", "steamapps/common")

    def test_out_under_compatdata(self):
        self.assert_refused(self.root / "compatdata" / "208580" / "packs", "compatdata")

    def test_out_inside_game_dir(self):
        self.assert_refused(self.k2 / "packs", "game dir")

    def test_out_through_symlink(self):
        real = self.root / "real"
        real.mkdir()
        link = self.root / "link"
        link.symlink_to(real)
        self.assert_refused(link / "packs", "symlink")

    def test_unsafe_out_refused_with_dry_run_and_install(self):
        (self.root / "repo" / ".git").mkdir(parents=True)
        self.write_config(out=self.root / "repo" / "packs")
        self.assertEqual(self.run_cli("build", "--dry-run")[0], 2)
        self.assertEqual(self.run_cli("install")[0], 2)

    def test_out_safety_function(self):
        self.assertEqual(imp.out_safety(self.root / "fine" / "deeper"), [])
        self.assertTrue(imp.out_safety(self.root / "fine", [self.root]))  # inside a protected dir


class BuildTests(Base):
    def test_two_characters_chained_rows_and_readable_packs(self):
        out, err = self.build_ok()
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign, {"tst": {"appearance": LIVE_APP, "head": LIVE_HEAD},
                                  "tsu": {"appearance": LIVE_APP + 1, "head": LIVE_HEAD + 1}})
        # 2nd character chained on the 1st character's tables
        cb = self.script_calls("k1_port_char_build.py")
        self.assertEqual(len(cb), 2)
        self.assertNotIn("--chain-from", cb[0])
        self.assertEqual(cb[1][cb[1].index("--chain-from") + 1], str(self.out / "tst" / "override"))
        for cid, (a, h) in (("tst", (LIVE_APP, LIVE_HEAD)), ("tsu", (LIVE_APP + 1, LIVE_HEAD + 1))):
            info, assets, cells = self.load_pack(cid)
            self.assertEqual((info.character_id, info.display_name, info.default_tag, info.default_utc, info.default_dlg),
                             (cid, cid.capitalize(), f"k1p_{cid}", f"k1p_{cid}", ""))
            self.assertEqual((info.appearance_id, info.head_id, info.soundset_id), (a, h, 11))
            names = sorted((x.resref, kp.RES_TYPES[x.res_type]) for x in assets)
            want = sorted([(f"k1p_{cid}{p}", e) for p in ("ba", "bb", "h") for e in ("mdl", "mdx")]
                          + [(f"k1p_{cid}{p}01", "tpc") for p in ("ba", "bb", "h")] + [(f"k1p_{cid}", "utc")])
            self.assertEqual(names, want)
            by = {(c.table, c.row, c.column): c for c in cells}
            self.assertEqual(by[("appearance", a, "label")].value, f"K1P_{cid.capitalize()}")
            self.assertEqual(by[("appearance", a, "label")].template_row, 17)
            self.assertEqual(by[("appearance", a, "modela")].value, f"k1p_{cid}ba")
            self.assertEqual(by[("heads", h, "head")].value, f"k1p_{cid}h")
            self.assertEqual(by[("heads", h, "head")].template_row, -1)
            # texture conversion flags reached the tpc step: opaque always, txi only for the head
            tpcs = {x.resref: x.data for x in assets if x.res_type == 3007}
            self.assertTrue(tpcs[f"k1p_{cid}ba01"].endswith(b"|OPAQUE"))
            self.assertTrue(tpcs[f"k1p_{cid}h01"].endswith(b"|TXI:clamp 1\n|OPAQUE"))
            self.assertNotIn(b"|TXI", tpcs[f"k1p_{cid}ba01"])

    def test_models_get_their_own_textures_and_supermodel_map(self):
        self.build_ok("--only", "tst")
        ing = {Path(c[c.index("--resref") + 1]).name: c for c in self.script_calls("k1_port_ingest.py")}
        self.assertEqual(sorted(ing), ["p_tstba", "p_tstbb", "p_tsth"])
        a = ing["p_tstba"]
        self.assertEqual([a[i + 1] for i, x in enumerate(a) if x == "--texture"], ["p_tstba01=k1p_tstba01"])
        self.assertEqual(a[a.index("--supermodel-map") + 1], "p_tstbb=k1p_tstbb")
        self.assertNotIn("--supermodel-map", ing["p_tstbb"])
        self.assertEqual(a[a.index("--prefer-source") + 1], "vanilla_k1")
        self.assertEqual(a[a.index("--install-k1") + 1], str(self.k1))
        self.assertEqual(a[a.index("--stage-dir") + 1], str(self.out / "tst" / "stage"))

    def test_manifest_contents(self):
        self.build_ok()
        m = self.manifest("tst")
        recipe_file = self.recipes / "tst.toml"
        self.assertEqual(m["recipe"]["path"], str(recipe_file))
        self.assertEqual(m["recipe"]["sha256"], imp.sha256_file(recipe_file))
        self.assertEqual(m["k1_dir"], str(self.k1))
        self.assertEqual(m["assigned"], {"appearance": LIVE_APP, "head": LIVE_HEAD})
        self.assertEqual(m["flagged"], [])
        self.assertEqual({(s["resref"], s["ext"]) for s in m["sources"]},
                         {(f"p_tst{p}", e) for p in ("ba", "bb", "h") for e in ("mdl", "mdx")}
                         | {(f"p_tst{p}01", "tpc") for p in ("ba", "bb", "h")})
        # resolve exits 1 when an optional alternative (the .tga of a .tpc texture) is absent; every other step must be 0
        self.assertTrue(all(c["rc"] == 0 for c in m["commands"] if not c["label"].startswith("resolve")))
        labels = [c["label"] for c in m["commands"]]
        for want in ("resolve tst", "ingest p_tstba", "char_build tst", "prune gate", "tex k1p_tsth01"):
            self.assertIn(want, labels)
        pack = self.out / "tst.k2pack"
        self.assertEqual(m["pack"]["sha256"], imp.sha256_file(pack))
        self.assertEqual(m["pack"]["size"], pack.stat().st_size)
        self.assertIn("scripts", m["tools"])
        self.assertTrue((self.out / "tst" / "licence_report.json").is_file())
        self.assertTrue(list((self.out / "tst" / "logs").glob("*.log")))

    def test_flagged_override_sources_warn_and_are_recorded(self):
        self.put_k1("p_tstba.mdl", b"MODS-OVERRIDE\0P_TSTBA01\0", where="Override")
        out, err = self.build_ok("--only", "tst")
        self.assertIn("WARN", err)
        self.assertIn("flagged source p_tstba.mdl", err)
        m = self.manifest("tst")
        self.assertEqual([f["item"] for f in m["flagged"]], ["p_tstba.mdl"])
        self.assertEqual(m["flagged"][0]["licence"], "unverified_override")
        self.assertIn("flagged sources allowed", m["flag_policy"])
        report = json.loads((self.out / "tst" / "licence_report.json").read_text())
        self.assertEqual(len(report["flagged"]), 1)
        self.assertTrue((self.out / "tst.k2pack").is_file())

    def test_characters_revisited_textures_are_flagged_and_registry_is_private(self):
        cr = self.root / "cr"
        cr.mkdir()
        os.environ["K1PORT_CR_DEST"] = str(cr)
        os.environ["FAKE_CR"] = "1"
        self.write_config(build_extra='texture_source = "cr"')
        out, err = self.build_ok("--only", "tst")
        self.assertIn("pending_author_approval", err)
        m = self.manifest("tst")
        self.assertEqual(m["texture_source"], "cr")
        self.assertEqual(sorted(f["item"] for f in m["flagged"]),
                         ["k1p_tstba01.tga", "k1p_tstbb01.tga", "k1p_tsth01.tga"])
        ing = self.script_calls("k1_port_ingest.py")[0]
        self.assertEqual(ing[ing.index("--prefer-source") + 1], "characters_revisited_main")
        reg = json.loads((self.out / "tst" / "work" / "sources.json").read_text())["sources"]
        self.assertEqual(reg["characters_revisited_main"]["local_dir"], str(cr))
        self.assertEqual(reg["vanilla_k1"]["local_dir"], str(self.k1))
        self.assertEqual([k for k, v in reg.items() if v["local_dir"] and k not in ("vanilla_k1", "characters_revisited_main")], [])

    def test_cr_dir_missing_is_a_failure(self):
        os.environ["K1PORT_CR_DEST"] = str(self.root / "no_cr")
        self.write_config(build_extra='texture_source = "cr"')
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("Characters Revisited dir not found", err)
        self.assertIn("Fix:", err)

    def test_missing_k1_model_is_reported(self):
        (self.k1 / "_bif" / "p_tstbb.mdx").unlink()
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("p_tstbb.mdx", err)
        self.assertIn("not found in the K1 install", err)
        self.assertIn("Fix:", err)
        self.assertFalse((self.out / "tst.k2pack").exists())

    def test_missing_k1_texture_is_reported(self):
        (self.k1 / "_bif" / "p_tsth01.tpc").unlink()
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("p_tsth01.tpc/.tga", err)

    def test_ingest_failure_stops_the_build(self):
        os.environ["FAKE_INGEST_FAIL"] = "p_tstbb"
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("rejected by validation", err)
        self.assertFalse((self.out / "tst.k2pack").exists())
        self.assertFalse((self.out / "assignments.json").exists())

    def test_prune_gate_result_is_recorded_not_fatal(self):
        os.environ["FAKE_GATE_BLOCK"] = "1"
        self.build_ok("--only", "tst")
        gate = self.manifest("tst")["prune_gate"]
        self.assertEqual((gate["ran"], gate["rc"], gate["blocked"]), (True, 1, 1))
        self.assertTrue(any(x.startswith("BLOCKED") for x in gate["output"]))

    def test_bad_staged_tpc_is_reported(self):
        os.environ["FAKE_TEX_EXT"] = ".tpc"
        os.environ["FAKE_TEX_BYTES"] = "not a tpc"
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("shorter than the 128-byte header", err)
        self.assertEqual(self.script_calls("k1_port_tex_tpc.py"), [])

    def test_rebuild_cleans_the_character_work_dir(self):
        self.build_ok("--only", "tst")
        stale = self.out / "tst" / "stage" / "stale.txt"
        stale.write_text("x")
        self.build_ok("--only", "tst")
        self.assertFalse(stale.exists())

    def test_build_leaves_k1_and_k2_untouched(self):
        before = (snapshot(self.k1), snapshot(self.k2))
        self.build_ok()
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)

    def test_dry_run_writes_nothing(self):
        before = snapshot(self.root)
        rc, out, err = self.run_cli("build", "--dry-run")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(snapshot(self.root), before)
        self.assertFalse(self.out.exists())
        self.assertIn("dry run: nothing is written", out)
        self.assertIn("char_build", out)
        self.assertIn("--new-row 20", out)
        self.assertIn("--new-row 21", out)
        self.assertIn("tst.k2pack", out)
        for name in ("k1_port_ingest.py", "k1_port_char_build.py", "k1_port_tex_tpc.py"):
            self.assertEqual(self.script_calls(name), [], name)
        self.assertTrue(self.script_calls("k1_port_resolve.py"))

    def test_dry_run_reports_flagged_override(self):
        self.put_k1("p_tsth.mdx", b"OV", where="Override")
        rc, out, err = self.run_cli("build", "--dry-run", "--only", "tst")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("flagged source p_tsth.mdx", err)

    def test_bad_k1_dir_and_missing_live_table(self):
        (self.k2 / "override" / "heads.2da").unlink()
        rc, _, err = self.run_cli("build")
        self.assertEqual(rc, 1)
        self.assertIn("heads.2da", err)
        self.assertIn("Fix:", err)
        (self.k1 / "chitin.key").unlink()
        rc, _, err = self.run_cli("build")
        self.assertEqual(rc, 1)
        self.assertIn("no chitin.key", err)
        self.assertFalse(self.out.exists())

    def test_unknown_only_id(self):
        rc, _, err = self.run_cli("build", "--only", "nobody")
        self.assertEqual(rc, 1)
        self.assertIn("nobody", err)

    def test_head_is_required(self):
        text = (self.recipes / "tst.toml").read_text().replace('head   = { k1 = "p_tsth",  k2 = "k1p_tsth" }\n', "")
        (self.recipes / "tst.toml").write_text(text)
        rc, _, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("needs a head model", err)


class AssignmentTests(Base):
    def test_stable_across_rebuild(self):
        self.build_ok()
        first = (self.out / "assignments.json").read_text()
        sha1 = imp.sha256_file(self.out / "tst.k2pack")
        self.build_ok()
        self.assertEqual((self.out / "assignments.json").read_text(), first)
        self.assertEqual(imp.sha256_file(self.out / "tst.k2pack"), sha1)
        info, _, _ = self.load_pack("tsu")
        self.assertEqual(info.appearance_id, LIVE_APP + 1)

    def test_live_count_change_is_detected_and_refused(self):
        self.build_ok()
        pack_before = imp.sha256_file(self.out / "tst.k2pack")
        self.write_table("appearance.2da", APP_HEADERS, LIVE_APP + 1)  # another mod appended a row
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("WARN: assigned rows no longer fit", err)
        self.assertIn("--reassign", err)
        self.assertEqual(imp.sha256_file(self.out / "tst.k2pack"), pack_before)
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"]["appearance"], LIVE_APP)

    def test_reassign_takes_the_new_rows(self):
        self.build_ok()
        self.write_table("appearance.2da", APP_HEADERS, LIVE_APP + 2)
        self.build_ok("--reassign")
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"]["appearance"], LIVE_APP + 2)
        self.assertEqual(assign["tsu"]["appearance"], LIVE_APP + 3)
        self.assertEqual(assign["tst"]["head"], LIVE_HEAD)  # heads table did not move
        self.assertEqual(self.load_pack("tsu")[0].appearance_id, LIVE_APP + 3)

    def test_reassign_with_only_is_refused(self):
        rc, _, err = self.run_cli("build", "--reassign", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("--reassign", err)

    def test_config_order_change_is_detected(self):
        self.build_ok()
        self.write_config(chars=["tsu", "tst"])
        rc, _, err = self.run_cli("build")
        self.assertEqual(rc, 1)
        self.assertIn("stored row ids differ from the plan", err)

    def test_first_row_keys_pad_above_the_live_tables(self):
        self.write_config(build_extra="first_appearance_row = 23\nfirst_head_row = 7")
        self.build_ok()
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"], {"appearance": 23, "head": 7})
        self.assertEqual(assign["tsu"], {"appearance": 24, "head": 8})
        pads = [c for c in self.calls() if "_twoda" in c and "pad" in c]
        self.assertEqual(len(pads), 2)  # appearance + heads, first character only; the 2nd chains on the 1st
        info, _, cells = self.load_pack("tst")
        self.assertEqual((info.appearance_id, info.head_id), (23, 7))
        self.assertTrue(all(c.row in (23, 7) for c in cells))
        cb = self.script_calls("k1_port_char_build.py")
        self.assertEqual(cb[0][cb[0].index("--chain-from") + 1], str(self.out / "tst" / "base"))
        self.assertEqual(cb[1][cb[1].index("--chain-from") + 1], str(self.out / "tst" / "override"))

    def test_first_row_below_live_count_is_refused(self):
        self.write_config(build_extra="first_appearance_row = 3")
        rc, _, err = self.run_cli("build")
        self.assertEqual(rc, 1)
        self.assertIn("first_appearance_row = 3", err)
        self.assertIn("overwrite", err)

    def test_only_second_character_pads_from_the_live_table(self):
        self.build_ok("--only", "tsu")
        self.assertEqual(json.loads((self.out / "assignments.json").read_text()), {"tsu": {"appearance": LIVE_APP + 1, "head": LIVE_HEAD + 1}})
        cb = self.script_calls("k1_port_char_build.py")[0]
        self.assertEqual(cb[cb.index("--chain-from") + 1], str(self.out / "tsu" / "base"))
        self.assertEqual(self.load_pack("tsu")[0].appearance_id, LIVE_APP + 1)


AUDIO_RECIPE = """
[audio]
k1_ssf = "p_{id}"
k2_ssf = "k1p_{id}ssf"
"""
SND_HEADERS = ["label", "resref", "strref", "gender", "type"]
LIVE_SND = 12


class AudioTests(Base):
    """Recipes with [audio]: K1 SSF + TLK + streamsounds wavs -> ssf asset, STREAM wavs, a new soundset row."""
    audio_chars = ("tst", "tsu")
    base_ref = {"tst": 1000, "tsu": 2000}

    def setUp(self):
        super().setUp()
        self.write_table("soundset.2da", SND_HEADERS, LIVE_SND)
        (self.k1 / "StreamSounds").mkdir()  # mixed case on purpose: the lookup is case-insensitive
        tlk = {}
        for cid in self.chars:
            if cid in self.audio_chars:
                (self.recipes / f"{cid}.toml").write_text(
                    RECIPE.format(id=cid, name=cid.capitalize(), conv="pykotor") + AUDIO_RECIPE.format(id=cid))
                tlk.update(self.put_audio(cid))
        for d in (self.k1, self.k2):
            (d / "dialog.tlk").write_text(json.dumps(tlk))

    def ssf_bytes(self, cid):
        """172-byte SSF V1.1: 5 voiced slots (slots 3 and 4 share one voiceover), the rest empty (0xFFFFFFFF)."""
        base = self.base_ref[cid]
        refs = [base + i for i in range(5)] + [0xFFFFFFFF] * 23
        return b"SSF V1.1" + struct.pack("<I", 12) + struct.pack("<28I", *refs) + struct.pack("<12I", *([0xFFFFFFFF] * 12))

    def put_audio(self, cid):
        data = self.ssf_bytes(cid)
        self.assertEqual(len(data), 172)
        self.put_k1(f"p_{cid}.ssf", data)
        base, tlk = self.base_ref[cid], {}
        names = [f"p_{cid}_bat1", f"p_{cid}_bat2", f"p_{cid}_hit1", f"p_{cid}_dead", f"p_{cid}_dead"]
        for i, vo in enumerate(names):
            tlk[str(base + i)] = vo
        for vo in sorted(set(names)):
            (self.k1 / "StreamSounds" / f"{vo.upper()}.wav").write_bytes(b"\xff\xfbK1MP3:" + vo.encode())
        return tlk

    def vos(self, cid):
        return [f"p_{cid}_bat1", f"p_{cid}_bat2", f"p_{cid}_dead", f"p_{cid}_hit1"]

    def table(self, path):
        return json.loads(Path(path).read_text())

    def test_pack_has_ssf_stream_wavs_soundset_cells_and_utc_row(self):
        self.build_ok()
        for cid, row in (("tst", LIVE_SND), ("tsu", LIVE_SND + 1)):
            info, assets, cells = self.load_pack(cid)
            self.assertEqual(info.soundset_id, row)  # the new row, not the recipe fallback 11
            ssf = [a for a in assets if a.res_type == 2060]
            self.assertEqual([(a.resref, a.flags, a.data) for a in ssf], [(f"k1p_{cid}ssf", kp.FLAG_RESMAN, self.ssf_bytes(cid))])
            wavs = [a for a in assets if a.res_type == kp.RES_TYPE_WAV]
            self.assertEqual(sorted(a.resref for a in wavs), self.vos(cid))
            self.assertTrue(all(a.flags == kp.FLAG_STREAM == 1 for a in wavs))
            self.assertTrue(all(a.data.startswith(b"\xff\xfbK1MP3:") for a in wavs))
            by = {(c.table, c.row, c.column): c for c in cells if c.table == "soundset"}
            self.assertEqual({col: c.value for (_, r, col), c in by.items() if r == row},
                             {"label": f"K1P_{cid.capitalize()}", "resref": f"k1p_{cid}ssf", "strref": "-1", "gender": "0", "type": ""})
            self.assertEqual({c.template_row for c in by.values()}, {-1})
            self.assertEqual({r for (_, r, _) in by}, {row})
            utc = next(a for a in assets if a.res_type == 2027)
            self.assertTrue(utc.data.endswith(f":ss={row}".encode()), utc.data)
            self.assertEqual(len(assets), 6 + 3 + 1 + 1 + 4)  # models + tpc + utc + ssf + wavs
            self.assertEqual(len(cells), 11 + 3 + 5)  # appearance row + heads row + soundset row

    def test_soundset_tables_chained_and_passed_to_char_build(self):
        self.build_ok()
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"], {"appearance": LIVE_APP, "head": LIVE_HEAD, "soundset": LIVE_SND})
        self.assertEqual(assign["tsu"], {"appearance": LIVE_APP + 1, "head": LIVE_HEAD + 1, "soundset": LIVE_SND + 1})
        cb = self.script_calls("k1_port_char_build.py")
        for c, cid, row in ((cb[0], "tst", LIVE_SND), (cb[1], "tsu", LIVE_SND + 1)):
            self.assertEqual(c[c.index("--soundset") + 1], str(row))
            self.assertEqual(c[c.index("--soundset-table") + 1], str(self.out / cid / "override" / "soundset.2da"))
        t_tst = self.table(self.out / "tst" / "override" / "soundset.2da")["rows"]
        t_tsu = self.table(self.out / "tsu" / "override" / "soundset.2da")["rows"]
        self.assertEqual((len(t_tst), len(t_tsu)), (LIVE_SND + 1, LIVE_SND + 2))
        self.assertEqual(t_tsu[LIVE_SND][:2], ["K1P_Tst", "k1p_tstssf"])  # tsu chained on tst's staged table
        self.assertEqual(t_tsu[LIVE_SND + 1][:2], ["K1P_Tsu", "k1p_tsussf"])
        self.assertEqual(self.table(self.k2 / "override" / "soundset.2da")["rows"][:LIVE_SND], t_tst[:LIVE_SND])

    def test_rebuild_reuses_the_soundset_assignment(self):
        self.build_ok()
        first = (self.out / "assignments.json").read_text()
        sha1 = imp.sha256_file(self.out / "tst.k2pack")
        self.build_ok()
        self.assertEqual((self.out / "assignments.json").read_text(), first)
        self.assertEqual(imp.sha256_file(self.out / "tst.k2pack"), sha1)

    def test_only_second_character_pads_the_soundset_table(self):
        self.build_ok("--only", "tsu")
        self.assertEqual(json.loads((self.out / "assignments.json").read_text())["tsu"]["soundset"], LIVE_SND + 1)
        rows = self.table(self.out / "tsu" / "override" / "soundset.2da")["rows"]
        self.assertEqual(len(rows), LIVE_SND + 2)
        self.assertEqual(rows[LIVE_SND], [""] * 5)  # the gap row the first character would own
        self.assertEqual(self.load_pack("tsu")[0].soundset_id, LIVE_SND + 1)

    def test_first_soundset_row_pads_above_the_live_table(self):
        self.write_config(build_extra=f"first_soundset_row = {LIVE_SND + 3}")
        self.build_ok()
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"]["soundset"], LIVE_SND + 3)
        self.assertEqual(assign["tsu"]["soundset"], LIVE_SND + 4)
        self.assertEqual(self.load_pack("tst")[0].soundset_id, LIVE_SND + 3)

    def test_first_soundset_row_below_live_count_is_refused(self):
        self.write_config(build_extra=f"first_soundset_row = {LIVE_SND - 1}")
        rc, _, err = self.run_cli("build")
        self.assertEqual(rc, 1)
        self.assertIn(f"first_soundset_row = {LIVE_SND - 1}", err)
        self.assertIn("overwrite", err)

    def test_live_soundset_count_change_is_refused_then_reassigned(self):
        self.build_ok()
        pack_before = imp.sha256_file(self.out / "tst.k2pack")
        self.write_table("soundset.2da", SND_HEADERS, LIVE_SND + 2)  # another mod appended soundset rows
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("soundset.2da", err)
        self.assertIn("--reassign", err)
        self.assertEqual(imp.sha256_file(self.out / "tst.k2pack"), pack_before)
        self.build_ok("--reassign")
        self.assertEqual(self.load_pack("tst")[0].soundset_id, LIVE_SND + 2)

    def test_recipe_gaining_audio_keeps_rows_and_adds_a_soundset(self):
        for cid in self.chars:
            (self.recipes / f"{cid}.toml").write_text(RECIPE.format(id=cid, name=cid.capitalize(), conv="pykotor"))
        self.build_ok()
        before = json.loads((self.out / "assignments.json").read_text())
        self.assertNotIn("soundset", before["tst"])
        self.assertEqual(self.load_pack("tst")[0].soundset_id, 11)  # the recipe fallback
        for cid in self.chars:
            (self.recipes / f"{cid}.toml").write_text(
                RECIPE.format(id=cid, name=cid.capitalize(), conv="pykotor") + AUDIO_RECIPE.format(id=cid))
        self.build_ok()
        after = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(after["tst"], {**before["tst"], "soundset": LIVE_SND})
        self.assertEqual(self.load_pack("tst")[0].soundset_id, LIVE_SND)

    def test_manifest_records_ssf_wavs_and_row(self):
        self.build_ok()
        m = self.manifest("tst")
        a = m["audio"]
        self.assertEqual((a["k1_ssf"], a["k2_ssf"], a["origin"]), ("p_tst", "k1p_tstssf", "vanilla_k1"))
        self.assertEqual(a["ssf_sha256"], imp.sha256_bytes(self.ssf_bytes("tst")))
        self.assertEqual(a["strrefs"][:5], [1000, 1001, 1002, 1003, 1004])
        self.assertEqual([w["resref"] for w in a["wavs"]], self.vos("tst"))
        for w in a["wavs"]:
            staged = self.out / "tst" / "audio" / "streamsounds" / f"{w['resref']}.wav"
            self.assertEqual((w["sha256"], w["origin"], w["pack_flags"]), (imp.sha256_file(staged), "vanilla_k1", 1))
            self.assertEqual(imp.sha256_file(self.k1 / "StreamSounds" / f"{w['resref'].upper()}.wav"), w["sha256"])
        self.assertEqual(m["assigned"]["soundset"], LIVE_SND)
        self.assertEqual(a["missing_optional"], [])
        self.assertTrue((self.out / "tst" / "audio" / "k1p_tstssf.ssf").is_file())

    def test_missing_wav_warns_and_is_recorded(self):
        (self.k1 / "StreamSounds" / "P_TST_BAT2.wav").unlink()
        out, err = self.build_ok("--only", "tst")
        self.assertIn("WARN: tst: K1 voice wav p_tst_bat2.wav is not in streamsounds", err)
        m = self.manifest("tst")
        self.assertEqual(m["audio"]["missing_optional"], ["p_tst_bat2.wav"])
        self.assertIn("p_tst_bat2.wav", m["missing_optional"])
        _, assets, _ = self.load_pack("tst")
        self.assertEqual(sorted(a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV),
                         ["p_tst_bat1", "p_tst_dead", "p_tst_hit1"])

    def test_no_wavs_at_all_fails(self):
        for f in (self.k1 / "StreamSounds").glob("P_TST_*.wav"):
            f.unlink()
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("none of the 4 K1 voice wavs", err)
        self.assertFalse((self.out / "tst.k2pack").exists())

    def test_symlinked_wav_is_not_used(self):
        real = self.root / "elsewhere.wav"
        real.write_bytes(b"x")
        (self.k1 / "StreamSounds" / "P_TST_BAT1.wav").unlink()
        (self.k1 / "StreamSounds" / "P_TST_BAT1.wav").symlink_to(real)
        out, err = self.build_ok("--only", "tst")
        self.assertIn("p_tst_bat1.wav is not in streamsounds", err)

    def test_k2_tlk_missing_the_row_fails(self):
        tlk = json.loads((self.k2 / "dialog.tlk").read_text())
        del tlk["1002"]
        (self.k2 / "dialog.tlk").write_text(json.dumps(tlk))
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("strref 1002", err)
        self.assertIn("missing in the K2 dialog.tlk", err)
        self.assertIn("this K2 TLK lacks the K1 row", err)
        self.assertFalse((self.out / "tst.k2pack").exists())
        self.assertFalse((self.out / "tst").exists())  # failed before the work dir was created

    def test_k2_tlk_with_a_different_voiceover_fails(self):
        tlk = json.loads((self.k2 / "dialog.tlk").read_text())
        tlk["1001"] = "p_other_bat2"
        (self.k2 / "dialog.tlk").write_text(json.dumps(tlk))
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("strref 1001", err)
        self.assertIn("voiceover differs", err)

    def test_k1_tlk_missing_the_row_fails(self):
        tlk = json.loads((self.k1 / "dialog.tlk").read_text())
        del tlk["1000"]
        (self.k1 / "dialog.tlk").write_text(json.dumps(tlk))
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("not in the K1 dialog.tlk", err)

    def test_bad_ssf_and_missing_ssf_fail(self):
        self.put_k1("p_tst.ssf", b"SSF V1.1" + b"\0" * 100)
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("expected 172 bytes", err)
        (self.k1 / "_bif" / "p_tst.ssf").unlink()
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("p_tst.ssf not found", err)

    def test_build_leaves_k1_and_k2_untouched(self):
        before = (snapshot(self.k1), snapshot(self.k2))
        self.build_ok()
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)
        wavs = sorted(p.name for p in (self.out / "tst" / "audio" / "streamsounds").iterdir())
        self.assertEqual(wavs, [f"{v}.wav" for v in self.vos("tst")])  # lowercase staging names

    def test_dry_run_prints_the_audio_plan_and_writes_nothing(self):
        before = snapshot(self.root)
        rc, out, err = self.run_cli("build", "--dry-run")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(snapshot(self.root), before)
        self.assertIn("soundset row 12", out)
        self.assertIn("soundset row 13", out)
        self.assertIn("audio: K1 p_tst.ssf (172 bytes, 5 strrefs 1000..1004", out)
        self.assertIn("4/4 K1 voice wavs from streamsounds/ -> STREAM assets", out)
        self.assertIn("audio: new soundset.2da row 12 (label K1P_Tst, resref k1p_tstssf, gender 0)", out)
        self.assertIn("--soundset 12", out)
        self.assertIn("--soundset-table", out)
        self.assertEqual(self.script_calls("k1_port_char_build.py"), [])

    def test_list_remove_and_assignments(self):
        self.build_ok()
        self.assertEqual(self.run_cli("install")[0], 0)
        rc, out, _ = self.run_cli("list", "--json")
        self.assertEqual(rc, 0)
        listed = {c["id"]: c for c in json.loads(out)["characters"]}
        self.assertEqual(listed["tst"]["assigned"]["soundset"], LIVE_SND)
        rc, out, _ = self.run_cli("list")
        self.assertIn(f"rows {LIVE_APP}/{LIVE_HEAD}/{LIVE_SND}", out)
        rc, out, err = self.run_cli("remove", "tst")
        self.assertEqual(rc, 0, out + err)
        self.assertIn(f"soundset {LIVE_SND}", out)
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertNotIn("tst", assign)
        self.assertEqual(assign["tsu"]["soundset"], LIVE_SND + 1)

    def test_doctor_checks_the_live_soundset_table_only_with_audio(self):
        (self.k2 / "override" / "soundset.2da").unlink()
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL K2 soundset.2da", out)

    # -- extra barks: every <k1_ssf>_*.wav the SSF does not reference
    def put_wav(self, name, data=None):
        (self.k1 / "StreamSounds" / name).write_bytes(data if data is not None else b"\xff\xfbK1MP3:" + name.lower().encode())

    def test_extra_barks_are_served_and_recorded_separately(self):
        self.put_wav("P_TST_TIA.WAV")
        self.put_wav("P_TSTX_FOO.WAV")           # another prefix: not this character's
        self.put_wav("P_TST_WAYTOOLONGNAME.WAV")  # not a valid 16-char resref
        self.put_wav("P_TST_NOTES.TXT")           # not a wav
        out, err = self.build_ok("--only", "tst")
        _, assets, _ = self.load_pack("tst")
        wavs = sorted(a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV)
        self.assertEqual(wavs, sorted(self.vos("tst") + ["p_tst_tia"]))
        self.assertTrue(all(a.flags == kp.FLAG_STREAM for a in assets if a.res_type == kp.RES_TYPE_WAV))
        a = self.manifest("tst")["audio"]
        self.assertEqual([w["resref"] for w in a["wavs"]], self.vos("tst"))  # the SSF-referenced ones, unchanged
        self.assertEqual([w["resref"] for w in a["extra"]], ["p_tst_tia"])
        w = a["extra"][0]
        staged = self.out / "tst" / "audio" / "streamsounds" / "p_tst_tia.wav"
        self.assertEqual((w["sha256"], w["origin"], w["pack_flags"], w["file"]),
                         (imp.sha256_file(staged), "vanilla_k1", 1, "streamsounds/P_TST_TIA.WAV"))
        self.assertEqual(a["extra_dropped"], ["p_tst_waytoolongname.wav"])
        self.assertIn("1 extra K1 bark wav(s) left out", err)
        self.assertEqual(a["missing_optional"], [])
        self.assertIn("(1 extra)", out)

    def test_no_extra_barks_records_an_empty_list(self):
        self.build_ok("--only", "tst")
        a = self.manifest("tst")["audio"]
        self.assertEqual((a["extra"], a["extra_dropped"]), ([], []))

    def test_extra_barks_are_capped_by_the_pack_asset_limit(self):
        for n in ("P_TST_AAA.WAV", "P_TST_MMM.WAV", "P_TST_ZZZ.WAV"):
            self.put_wav(n)
        # 6 model files + 3 tpc + utc + ssf + 4 SSF wavs = 15 assets; a limit of 16 leaves room for one extra
        with mock.patch.object(kp, "MAX_ASSETS", 16):
            out, err = self.build_ok("--only", "tst")
        _, assets, _ = self.load_pack("tst")
        self.assertEqual(len(assets), 16)
        a = self.manifest("tst")["audio"]
        self.assertEqual([w["resref"] for w in a["extra"]], ["p_tst_aaa"])  # sorted: the cut is deterministic
        self.assertEqual(a["extra_dropped"], ["p_tst_mmm.wav", "p_tst_zzz.wav"])
        self.assertIn("2 extra K1 bark wav(s) left out to stay under the pack limits", err)

    def test_extra_barks_are_capped_by_bytes(self):
        recipe = imp.krecipe.load_recipe(self.recipes / "tst.toml")
        index = {}
        for n, size in (("p_tst_a.wav", 10), ("p_tst_b.wav", 10), ("p_tst_c.wav", 2)):
            f = self.root / n
            f.write_bytes(b"x" * size)
            index[n] = f
        have = self.root / "have.wav"
        have.write_bytes(b"y" * 10)
        with mock.patch.object(kp, "MAX_FILE_BYTES", 2 * (10 + 12)):  # half = 22 bytes: 10 used by the SSF wavs, 12 left
            extra, dropped = imp.extra_barks(recipe, index, {"p_tst_bat1": have}, {"p_tst_bat1"})
        self.assertEqual(sorted(extra), ["p_tst_a", "p_tst_c"])  # b does not fit, the smaller c after it still does
        self.assertEqual(dropped, ["p_tst_b.wav"])

    def test_dry_run_mentions_the_extra_barks(self):
        self.put_wav("P_TST_TIA.WAV")
        rc, out, err = self.run_cli("build", "--dry-run", "--only", "tst")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("(+1 extra p_tst_*.wav)", out)

    def test_audio_recipe_without_it_in_a_plain_config_is_unaffected(self):
        for cid in self.chars:
            (self.recipes / f"{cid}.toml").write_text(RECIPE.format(id=cid, name=cid.capitalize(), conv="pykotor"))
        self.build_ok()
        self.assertEqual([c for c in self.calls() if "_audio" in c], [])
        self.assertIsNone(self.manifest("tst")["audio"])


class AutoBase(Base):
    """A config with one committed recipe (tst) and K1 creature template p_aut with NO recipe file (auto-derived)."""
    chars = ("tst",)
    entries = ["tst", "utc:p_aut"]

    def setUp(self):
        super().setUp()
        self.write_table("soundset.2da", SND_HEADERS, LIVE_SND)
        self.data = tscan.K1Data()
        self.data.creature("aut", first="Autman")
        self.data.write(self.k1)
        refs = [2000, 2001, 2002] + [0xFFFFFFFF] * 25
        self.put_k1("p_aut.ssf", b"SSF V1.1" + struct.pack("<I", 12) + struct.pack("<28I", *refs) + struct.pack("<12I", *([0xFFFFFFFF] * 12)))
        (self.k1 / "StreamSounds").mkdir()
        tlk = {"2000": "p_aut_bat1", "2001": "p_aut_bat2", "2002": "p_aut_dead"}
        for vo in tlk.values():
            (self.k1 / "StreamSounds" / f"{vo.upper()}.wav").write_bytes(b"\xff\xfbK1MP3:" + vo.encode())
        (self.k1 / "StreamSounds" / "P_AUT_TIA.wav").write_bytes(b"\xff\xfbK1MP3:p_aut_tia")  # not in the SSF
        for d in (self.k1, self.k2):
            (d / "dialog.tlk").write_text(json.dumps(tlk))
        self.write_config(chars=self.entries)
        self.auto_file = self.out / "recipes_auto" / "aut.toml"

    def derived_text(self, rid="aut"):
        return kscan.derive_text(kscan.gather_facts(tscan.FixtureReader(self.k1), "p_aut"), rid)[1]

    def scan_calls(self):
        return [c for c in self.calls() if "_scan" in c]


class AutoTests(AutoBase):
    def test_build_derives_writes_and_records_the_recipe(self):
        out, err = self.build_ok()
        self.assertTrue(self.auto_file.is_file())
        self.assertEqual(self.auto_file.read_text(), self.derived_text())
        man = self.manifest("aut")
        self.assertEqual(man["recipe"]["path"], str(self.auto_file))
        self.assertEqual(man["recipe"]["sha256"], imp.sha256_file(self.auto_file))
        self.assertEqual(man["recipe"]["auto"], {"k1_utc": "p_aut", "sha256": imp.sha256_file(self.auto_file)})
        self.assertEqual(man["recipe"]["converter"], "mdlops")
        self.assertIsNone(self.manifest("tst")["recipe"]["auto"])
        self.assertEqual(self.manifest("tst")["recipe"]["path"], str(self.recipes / "tst.toml"))
        self.assertIn("[aut] auto recipe: derived from K1 creature template p_aut", out)
        self.assertEqual(len(self.scan_calls()), 1)  # one pykotor start for every auto character

    def test_auto_character_gets_rows_pack_audio_and_mdlops(self):
        self.build_ok()
        assign = json.loads((self.out / "assignments.json").read_text())
        self.assertEqual(assign["tst"], {"appearance": LIVE_APP, "head": LIVE_HEAD})
        self.assertEqual(assign["aut"], {"appearance": LIVE_APP + 1, "head": LIVE_HEAD + 1, "soundset": LIVE_SND})
        info, assets, cells = self.load_pack("aut")
        self.assertEqual((info.character_id, info.display_name, info.default_utc, info.default_tag), ("aut", "Autman", "k1p_aut", "k1p_aut"))
        self.assertEqual((info.appearance_id, info.head_id, info.soundset_id), (LIVE_APP + 1, LIVE_HEAD + 1, LIVE_SND))
        wavs = sorted(a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV)
        self.assertEqual(wavs, ["p_aut_bat1", "p_aut_bat2", "p_aut_dead", "p_aut_tia"])  # 3 SSF-referenced + 1 extra
        self.assertEqual([a.resref for a in assets if a.res_type == 2060], ["k1p_autssf"])
        self.assertEqual(len(assets), 6 + 3 + 1 + 1 + 4)
        self.assertEqual(len(cells), 11 + 3 + 5)  # appearance + heads + soundset cells: the cell budget sees the auto pack
        utc = next(a for a in assets if a.res_type == 2027)
        self.assertTrue(utc.data.endswith(f":Autman:ss={LIVE_SND}".encode()), utc.data)
        subs = [c[next(i for i, x in enumerate(c) if x.endswith("k1_port_mdlops.py")) + 1] for c in self.script_calls("k1_port_mdlops.py")]
        self.assertEqual(subs, ["extract", "pass1", "rename", "pass2"])
        a = self.manifest("aut")["audio"]
        self.assertEqual([w["resref"] for w in a["extra"]], ["p_aut_tia"])
        self.assertEqual(self.manifest("aut")["assigned"]["soundset"], LIVE_SND)

    def test_auto_character_with_a_k1_conversation_gets_a_dialog(self):
        g = tdlg.graph([tdlg.node("Greetings, Commander.", "", [tdlg.link(0)]), tdlg.node("Gone.", "")],
                       [tdlg.node("Farewell.", "")], [tdlg.link(0, active="k_swg_x"), tdlg.link(0)])
        self.put_k1("k_haut_dialog.dlg.json", json.dumps(g).encode())
        self.build_ok()
        self.assertIn('[dialog]\nk1_dlg = "k_haut_dialog"\nk2_dlg = "k1p_autdlg"', self.auto_file.read_text())
        info, assets, _ = self.load_pack("aut")
        self.assertEqual(info.default_dlg, "k1p_autdlg")
        self.assertEqual([a.resref for a in assets if a.res_type == 2029], ["k1p_autdlg"])
        d = self.manifest("aut")["dialog"]
        self.assertEqual((d["k1_dlg"], d["stats"]["nodes_kept"], d["stats"]["voiced_lines"]), ("k_haut_dialog", 2, 0))  # the entry + its reply; "Gone." is unreachable
        self.assertIsNone(self.manifest("tst")["dialog"])
        cb = [c for c in self.script_calls("k1_port_char_build.py") if "aut" in c]
        self.assertEqual(cb[0][cb[0].index("--conversation") + 1], "k1p_autdlg")

    def test_auto_character_without_a_dlg_has_no_dialog(self):
        self.build_ok()
        self.assertNotRegex(self.auto_file.read_text(), r"(?m)^\[dialog\]")
        self.assertIsNone(self.manifest("aut")["dialog"])
        self.assertEqual(self.load_pack("aut")[0].default_dlg, "")

    def test_cell_budget_counts_the_auto_pack(self):
        out, _ = self.build_ok()
        self.assertIn(f"2DA cell budget: {11 + 3 + 11 + 3 + 5}/1024", out)

    def test_rebuild_is_stable(self):
        self.build_ok()
        before = (self.auto_file.read_bytes(), imp.sha256_file(self.out / "aut.k2pack"), self.auto_file.stat().st_mtime_ns)
        self.build_ok()
        self.assertEqual((self.auto_file.read_bytes(), imp.sha256_file(self.out / "aut.k2pack"), self.auto_file.stat().st_mtime_ns), before)

    def test_dry_run_writes_nothing(self):
        before = snapshot(self.root)
        rc, out, err = self.run_cli("build", "--dry-run")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(snapshot(self.root), before)
        self.assertFalse((self.out / "recipes_auto").exists())
        self.assertIn("[aut] auto recipe: derived from K1 creature template p_aut (would write", out)
        self.assertIn("[aut]   audio: K1 p_aut.ssf", out)
        self.assertIn("(+1 extra p_aut_*.wav)", out)
        self.assertIn("mdlops", out)

    def test_committed_recipe_wins(self):
        (self.recipes / "aut.toml").write_text(RECIPE.format(id="aut", name="Hand", conv="pykotor"))
        self.build_ok()
        self.assertEqual(self.scan_calls(), [])  # nothing derived
        self.assertFalse((self.out / "recipes_auto").exists())
        man = self.manifest("aut")
        self.assertEqual((man["recipe"]["path"], man["recipe"]["auto"], man["recipe"]["converter"]),
                         (str(self.recipes / "aut.toml"), None, "pykotor"))
        self.assertEqual(self.load_pack("aut")[0].display_name, "Hand")

    def test_plain_id_without_a_recipe_file_is_tried_as_a_utc(self):
        self.write_config(chars=["tst", "p_aut"])
        self.build_ok()
        self.assertTrue((self.out / "recipes_auto" / "p_aut.toml").is_file())
        info = self.load_pack("p_aut")[0]
        self.assertEqual((info.character_id, info.default_utc), ("p_aut", "k1p_p_aut"))
        rc, out, _ = self.run_cli("list", "--json")
        self.assertEqual({c["id"]: c["auto"] for c in json.loads(out)["characters"]}, {"tst": False, "p_aut": True})

    def test_unknown_utc_fails_with_a_hint_and_writes_nothing(self):
        self.write_config(chars=["tst", "utc:p_nope"])
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("p_nope", err)
        self.assertIn("not found in the K1 install", err)
        self.assertIn("npcs --grep", err)
        self.assertFalse(self.out.exists())

    def test_plain_id_with_no_recipe_and_no_utc_names_both(self):
        self.write_config(chars=["tst", "nope"])
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("no recipe file", err)
        self.assertIn("nope.toml", err)
        self.assertFalse(self.out.exists())

    def test_non_humanoid_utc_fails_the_build(self):
        self.data.creature("drd", modeltype="S")
        self.data.write(self.k1)
        self.write_config(chars=["tst", "utc:p_drd"])
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("model type 'S'", err)
        self.assertFalse(self.out.exists())

    def test_k2_names_may_not_collide_with_another_character(self):
        self.data.files[("_bif", "p_aut2.utc.json")] = self.data.files[("_bif", "p_aut.utc.json")]  # same models as p_aut
        self.data.write(self.k1)
        self.write_config(chars=["utc:p_aut", "utc:p_aut2"])
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("collides with recipe 'aut'", err)
        self.assertFalse(self.out.exists())

    def test_auto_recipes_dir_symlink_is_refused(self):
        (self.root / "elsewhere").mkdir()
        self.out.mkdir()
        (self.out / "recipes_auto").symlink_to(self.root / "elsewhere")
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 2, out + err)
        self.assertIn("symlink", err)
        self.assertEqual(list((self.root / "elsewhere").iterdir()), [])

    def test_build_leaves_k1_and_k2_untouched(self):
        before = (snapshot(self.k1), snapshot(self.k2))
        self.build_ok()
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)

    def test_only_selects_an_auto_character(self):
        self.build_ok("--only", "aut")
        self.assertTrue((self.out / "aut.k2pack").is_file())
        self.assertFalse((self.out / "tst.k2pack").exists())
        self.assertEqual(json.loads((self.out / "assignments.json").read_text())["aut"]["appearance"], LIVE_APP + 1)

    def test_list_install_remove_work_from_the_written_recipe(self):
        self.build_ok()
        n = len(self.scan_calls())
        rc, out, _ = self.run_cli("list")
        self.assertEqual(rc, 0)
        self.assertIn("auto from K1 p_aut", out)
        rc, out, _ = self.run_cli("list", "--json")
        listed = {c["id"]: c for c in json.loads(out)["characters"]}
        self.assertEqual((listed["aut"]["auto"], listed["aut"]["auto_utc"], listed["aut"]["built"], listed["aut"]["stale"]),
                         (True, "p_aut", True, False))
        self.assertEqual((listed["tst"]["auto"], listed["tst"]["auto_utc"]), (False, None))
        self.assertEqual(self.run_cli("install")[0], 0)
        rc, out, err = self.run_cli("remove", "aut")
        self.assertEqual(rc, 0, out + err)
        self.assertNotIn("aut", json.loads((self.out / "assignments.json").read_text()))
        self.assertEqual(len(self.scan_calls()), n)  # list / install / remove never re-read the K1 install
        self.assertFalse((self.k2 / "k1_imports" / "aut.k2pack").exists())
        self.assertTrue((self.k2 / "k1_imports" / "tst.k2pack").is_file())

    def test_list_before_a_build_derives_in_memory(self):
        rc, out, err = self.run_cli("list", "--json")
        self.assertEqual(rc, 0, out + err)
        listed = {c["id"]: c for c in json.loads(out)["characters"]}
        self.assertEqual((listed["aut"]["auto"], listed["aut"]["built"]), (True, False))
        self.assertFalse(self.out.exists())

    def test_stale_when_the_auto_recipe_changes(self):
        self.build_ok()
        self.auto_file.write_text(self.auto_file.read_text() + "# edited\n")
        rows = {r["id"]: r for r in json.loads(self.run_cli("list", "--json")[1])["characters"]}
        self.assertTrue(rows["aut"]["stale"])

    def test_doctor_lists_auto_recipes_and_checks_mdlops(self):
        import k1_port_mdlops as km
        with mock.patch.object(km, "PROTON", self.root), mock.patch.object(km, "TOOL_SRC", self.root), \
                mock.patch.object(km, "PREFIX", self.root):
            rc, out, err = self.run_cli("doctor")
            self.assertEqual(rc, 0, out + err)
            self.assertIn("OK   auto recipes: aut <- K1 p_aut", out)
            self.assertIn("converter mdlops: aut", out)
            self.assertIn("OK   K2 soundset.2da", out)  # the auto recipe has [audio]
        with mock.patch.object(km, "PROTON", self.root / "no_proton"):
            rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL Proton (converter mdlops: aut)", out)

    def test_doctor_reports_an_unknown_utc_as_a_config_failure(self):
        self.write_config(chars=["tst", "utc:p_nope"])
        rc, out, _ = self.run_cli("doctor")
        self.assertEqual(rc, 1)
        self.assertIn("FAIL config", out)
        self.assertIn("p_nope", out)

    def test_install_dry_run_needs_a_built_auto_pack(self):
        rc, out, err = self.run_cli("install", "--dry-run")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("pack not built", err)


class ScanCliTests(AutoBase):
    def scan(self, *argv, config=True):
        return self.run_cli("scan", *argv, config=config)

    def test_prints_the_derived_recipe(self):
        rc, out, err = self.scan("p_aut")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(out, self.derived_text())
        self.assertNotIn("WARN", err)
        self.assertFalse(self.out.exists())  # read only

    def test_id_option(self):
        rc, out, _ = self.scan("p_aut", "--id", "mine")
        self.assertEqual(rc, 0)
        self.assertIn('id = "mine"', out)
        self.assertIn('utc = "k1p_mine"', out)
        self.assertEqual(self.scan("p_aut", "--id", "Bad Id")[0], 1)

    def test_write_default_dir_is_out_recipes_auto(self):
        rc, out, err = self.scan("p_aut", "--write")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(self.auto_file.read_text(), out)
        self.assertIn(f"wrote {self.auto_file}", err)
        self.assertEqual(self.scan("p_aut", "--write")[0], 0)  # the default dir is regenerated freely

    def test_write_to_a_dir_refuses_to_overwrite_a_different_file(self):
        d = self.root / "mine"
        rc, out, err = self.scan("p_aut", "--write", str(d))
        self.assertEqual(rc, 0, out + err)
        self.assertEqual((d / "aut.toml").read_text(), out)
        self.assertEqual(self.scan("p_aut", "--write", str(d))[0], 0)  # identical: fine
        (d / "aut.toml").write_text("# hand edit\n")
        rc, out, err = self.scan("p_aut", "--write", str(d))
        self.assertEqual(rc, 1)
        self.assertIn("exists and differs", err)
        self.assertIn("--force", err)
        self.assertEqual((d / "aut.toml").read_text(), "# hand edit\n")
        self.assertEqual(self.scan("p_aut", "--write", str(d), "--force")[0], 0)
        self.assertEqual((d / "aut.toml").read_text(), self.derived_text())

    def test_write_into_a_game_dir_is_refused(self):
        for d in (self.k2 / "override", self.k1 / "Override"):
            rc, out, err = self.scan("p_aut", "--write", str(d))
            self.assertEqual(rc, 2, out + err)
            self.assertIn("game dir", err)
            self.assertFalse((d / "aut.toml").exists())

    def test_written_recipe_is_a_valid_recipes_dir_override(self):
        d = self.root / "mine"
        self.scan("p_aut", "--write", str(d))
        (d / "tst.toml").write_text((self.recipes / "tst.toml").read_text())
        self.recipes = d
        self.write_config(chars=["tst", "aut"])
        n = len(self.scan_calls())
        self.build_ok()
        self.assertEqual(len(self.scan_calls()), n)  # the written file is a plain committed-style recipe now
        self.assertEqual(self.manifest("aut")["recipe"]["path"], str(d / "aut.toml"))

    def test_without_a_config_the_k1_dir_is_an_option(self):
        with mock.patch.object(imp, "DEFAULT_CONFIG", str(self.root / "no_such_config.toml")):
            rc, out, err = self.scan("p_aut", "--k1-dir", str(self.k1), config=False)
            self.assertEqual(rc, 0, out + err)
            self.assertEqual(out, self.derived_text())
            rc, out, err = self.scan("p_aut", "--k1-dir", str(self.k1), "--write", config=False)
            self.assertEqual(rc, 1)
            self.assertIn("no DIR given and no config", err)
            rc, out, err = self.scan("p_aut", "--k1-dir", str(self.root / "nowhere"), config=False)
            self.assertEqual(rc, 1)
            self.assertIn("no chitin.key", err)

    def test_unknown_and_bad_resrefs(self):
        rc, out, err = self.scan("p_nope")
        self.assertEqual(rc, 1)
        self.assertIn("not found in the K1 install", err)
        self.assertIn("npcs --grep", err)
        rc, out, err = self.scan("P Bad")
        self.assertEqual(rc, 1)
        self.assertIn("bad K1 creature template", err)

    def test_upper_case_resref_is_accepted(self):
        self.assertEqual(self.scan("P_AUT")[0], 0)

    def test_non_humanoid_is_refused_with_a_fix(self):
        self.data.creature("drd", modeltype="S")
        self.data.write(self.k1)
        rc, out, err = self.scan("p_drd")
        self.assertEqual(rc, 1)
        self.assertIn("model type 'S'", err)
        self.assertIn("Fix:", err)
        self.assertEqual(out, "")

    def test_notes_are_warnings_on_stderr(self):
        self.data.creature("mut", soundset=False)
        self.data.write(self.k1)
        rc, out, err = self.scan("p_mut")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("WARN: mut: no [audio]", err)

    def test_scan_leaves_k1_and_k2_untouched(self):
        before = (snapshot(self.k1), snapshot(self.k2))
        self.scan("p_aut")
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)


class NpcsCliTests(AutoBase):
    def setUp(self):
        super().setUp()
        self.data.creature("bastila", first="Bastila", tag="Bastila", gender=1)
        self.data.creature("carth", first="Carth", tag="Carth")
        self.data.write(self.k1)

    def test_lists_all_templates_with_columns(self):
        rc, out, err = self.run_cli("npcs")
        self.assertEqual(rc, 0, out + err)
        lines = out.splitlines()
        self.assertTrue(lines[0].startswith("utc"))
        for col in ("tag", "name", "app", "models", "soundset", "conversation"):
            self.assertIn(col, lines[0])
        row = next(x for x in lines if x.startswith("p_carth "))
        self.assertIn("p_carthba+p_carthbb/p_carthh", row)
        self.assertIn("k_hcarth_dialog", row)
        self.assertIn("3 of 3 K1 creature template(s)", out)

    def test_grep_and_limit(self):
        rc, out, _ = self.run_cli("npcs", "--grep", "BASTI")
        self.assertEqual(rc, 0)
        self.assertIn("p_bastila", out)
        self.assertNotIn("p_carth", out)
        self.assertIn("1 of 1 K1 creature template(s) matching 'BASTI'", out)
        rc, out, _ = self.run_cli("npcs", "--limit", "1")
        self.assertIn("1 of 3 K1 creature template(s)", out)
        self.assertEqual(len([x for x in out.splitlines() if x.startswith("p_")]), 1)

    def test_json_and_no_match(self):
        rc, out, _ = self.run_cli("npcs", "--json", "--grep", "autman")  # first name
        got = json.loads(out)
        self.assertEqual([r["utc"] for r in got["rows"]], ["p_aut"])
        self.assertEqual(got["total"], 1)
        rc, out, _ = self.run_cli("npcs", "--grep", "zzzz")
        self.assertEqual(rc, 0)
        self.assertIn("0 of 0", out)

    def test_read_only_and_no_config_needed(self):
        before = (snapshot(self.k1), snapshot(self.k2))
        with mock.patch.object(imp, "DEFAULT_CONFIG", str(self.root / "no_such_config.toml")):
            rc, out, err = self.run_cli("npcs", "--k1-dir", str(self.k1), config=False)
        self.assertEqual(rc, 0, out + err)
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), before)
        self.assertFalse(self.out.exists())


class DialogTests(Base):
    """Recipes with [dialog]: the K1 DLG (script-stripped, text embedded), streamwaves VO as STREAM assets, lips from lips/*.mod."""
    chars = ("tst",)
    max_voiced = 30
    k1_dlg_line = 'k1_dlg = "k_htst_dialog"\n'
    VO = ("nm01tst001_", "nm01tst002_", "nm01tst003_")

    def setUp(self):
        super().setUp()
        self.write_recipe()
        self.tlk = {"10": "Hello there.", "11": "Second line.", "12": "Hidden line.", "13": "Go on.", "14": "Bye."}
        self.put_k1("k_htst_dialog.dlg.json", json.dumps(self.dlg_graph()).encode())
        (self.k1 / "dialog.tlk").write_text(json.dumps(self.tlk))
        wdir = self.k1 / "StreamWaves" / "M01" / "TST"  # mixed case + nested on purpose
        wdir.mkdir(parents=True)
        for vo in self.VO:
            (wdir / f"{vo.upper()}.WAV").write_bytes(b"\x00" * 58 + b"\xff\xfbVO:" + vo.encode())
        (self.k1 / "lips").mkdir()
        self.lips = {vo: tdlg.GOOD_LIP for vo in self.VO}
        self.write_lips()

    def write_recipe(self, extra=""):
        (self.recipes / "tst.toml").write_text(
            RECIPE.format(id="tst", name="Tst", conv="pykotor") + extra
            + f'\n[dialog]\n{self.k1_dlg_line}k2_dlg = "k1p_tstdlg"\nmax_voiced = {self.max_voiced}\n')

    def write_lips(self):
        (self.k1 / "lips" / "a_loc.mod").write_text(json.dumps({k: v.hex() for k, v in self.lips.items()}))

    def dlg_graph(self):
        n, l = tdlg.node, tdlg.link
        ents = [n("", "NM01TST001_", [l(0)], strref=10, script="k_swg_do_it", quest="q_tst"),
                n("", "nm01tst002_", [l(1)], strref=11),
                n("", "nm01tst003_", [], strref=12),
                n("Embedded, no strref.", "nm01tst004_", [])]   # no wav in the K1 install
        reps = [n("", "", [l(1)], strref=13), n("", "", [l(2, active="k_cond_x"), l(3)], strref=14)]
        return tdlg.graph(ents, reps, [l(0, active="k_swg_start"), l(0)])

    def dlg_out(self, cid="tst"):
        _, assets, _ = self.load_pack(cid)
        dlg = next(a for a in assets if a.res_type == 2029)
        self.assertTrue(dlg.data.startswith(b"FAKEDLG:"))
        return dlg, json.loads(dlg.data[len(b"FAKEDLG:"):])

    def test_pack_serves_dlg_streamvoice_wavs_lips_and_the_conversation(self):
        out, err = self.build_ok()
        info, assets, _ = self.load_pack("tst")
        self.assertEqual(info.default_dlg, "k1p_tstdlg")
        dlg = [a for a in assets if a.res_type == 2029]
        self.assertEqual([(a.resref, a.flags) for a in dlg], [("k1p_tstdlg", kp.FLAG_RESMAN)])
        wavs = sorted((a.resref, a.flags, a.data) for a in assets if a.res_type == kp.RES_TYPE_WAV)
        self.assertEqual([w[:2] for w in wavs], [("nm01tst001_", kp.FLAG_STREAM), ("nm01tst002_", kp.FLAG_STREAM)])
        self.assertEqual([w[2] for w in wavs], [b"\x00" * 58 + b"\xff\xfbVO:" + vo.encode() for vo in self.VO[:2]])  # bytes unchanged
        lips = sorted((a.resref, a.flags, a.data) for a in assets if a.res_type == 3004)
        self.assertEqual(lips, [("nm01tst001_", kp.FLAG_RESMAN, tdlg.GOOD_LIP), ("nm01tst002_", kp.FLAG_RESMAN, tdlg.GOOD_LIP)])
        self.assertEqual(len(assets), 6 + 3 + 1 + 1 + 2 + 2)  # models + tpc + utc + dlg + wavs + lips
        cb = self.script_calls("k1_port_char_build.py")[0]
        self.assertEqual(cb[cb.index("--conversation") + 1], "k1p_tstdlg")
        self.assertTrue((self.out / "tst" / "dialog" / "streamvoice" / "nm01tst001_.wav").is_file())
        self.assertTrue((self.out / "tst" / "dialog" / "lips" / "nm01tst002_.lip").is_file())
        self.assertTrue((self.out / "tst" / "dialog" / "k1p_tstdlg.dlg").is_file())

    def test_dlg_has_literal_text_no_scripts_and_only_unconditional_nodes(self):
        self.build_ok()
        _, g = self.dlg_out()
        self.assertEqual([e["text"] for e in g["entries"]], ["Hello there.", "Second line.", "Embedded, no strref."])
        self.assertEqual([r["text"] for r in g["replies"]], ["Go on.", "Bye."])
        self.assertEqual([e["vo"] for e in g["entries"]], ["nm01tst001_", "nm01tst002_", ""])  # lowercased; no wav = silent
        self.assertEqual(g["starters"], [{"index": 0}])
        self.assertNotIn("k_swg", json.dumps(g))
        self.assertNotIn("q_tst", json.dumps(g))
        self.assertNotIn("k_cond_x", json.dumps(g))
        self.assertEqual([l["index"] for l in g["replies"][1]["links"]], [2])  # the conditional link to "Hidden line." is gone

    def test_manifest_records_the_dialog(self):
        self.build_ok()
        d = self.manifest("tst")["dialog"]
        self.assertEqual((d["k1_dlg"], d["k2_dlg"], d["max_voiced"]), ("k_htst_dialog", "k1p_tstdlg", 30))
        st = d["stats"]
        self.assertEqual((st["nodes_total"], st["nodes_kept"], st["nodes_dropped_by_condition"], st["starters_total"],
                          st["starters_kept"], st["voiced_lines"], st["silent_no_wav"], st["lips_ok"], st["asset_count"]),
                         (6, 5, 1, 2, 1, 2, 1, 2, 5))
        self.assertEqual(st["condition_scripts_dropped"], ["k_cond_x", "k_swg_start"])
        self.assertIn("scripts stripped", d["policy"])
        self.assertEqual(d["dlg"]["sha256"], imp.sha256_file(self.out / "tst" / "dialog" / "k1p_tstdlg.dlg"))
        self.assertEqual(d["dlg"]["origin"], "vanilla_k1")
        self.assertEqual([w["resref"] for w in d["wavs"]], ["nm01tst001_", "nm01tst002_"])
        for w in d["wavs"]:
            staged = self.out / "tst" / "dialog" / "streamvoice" / f"{w['resref']}.wav"
            self.assertEqual((w["sha256"], w["origin"], w["pack_flags"]), (imp.sha256_file(staged), "vanilla_k1", 1))
            self.assertEqual(w["file"], f"StreamWaves/M01/TST/{w['resref'].upper()}.WAV")
        self.assertEqual([(l["resref"], l["file"], l["origin"]) for l in d["lips"]],
                         [("nm01tst001_", "lips/a_loc.mod", "vanilla_k1"), ("nm01tst002_", "lips/a_loc.mod", "vanilla_k1")])
        self.assertEqual(self.manifest("tst")["pack"]["asset_count"], 6 + 3 + 1 + 1 + 4)

    def test_dry_run_prints_the_stats_and_writes_nothing(self):
        before = snapshot(self.root)
        rc, out, err = self.run_cli("build", "--dry-run")
        self.assertEqual(rc, 0, out + err)
        self.assertEqual(snapshot(self.root), before)
        self.assertIn("[tst]   dialog: K1 k_htst_dialog.dlg -> k1p_tstdlg.dlg", out)
        self.assertIn("dialog: nodes 6 total (4 entries, 2 replies): 5 kept, 1 dropped by condition, 0 shadowed, 0 unreachable", out)
        self.assertIn("dialog: starters 2 total, 1 without a condition, 1 kept", out)
        self.assertIn("dialog: voice: 2/30 voiced line(s)", out)
        self.assertIn("lips 2 ok, 0 missing, 0 failed", out)
        self.assertIn("pack would hold 15/192 assets", out)  # 6 model files + 3 tpc + utc + dlg + 2 wavs + 2 lips
        self.assertIn("nm01tst004_", err)  # the node whose wav is missing is a WARN
        self.assertEqual([c for c in self.calls() if "_dialog" in c and "write" in c], [])  # no DLG is built on a dry run

    def test_budget_voices_the_first_lines_breadth_first_and_keeps_text(self):
        self.max_voiced = 1
        self.write_recipe()
        self.build_ok()
        _, g = self.dlg_out()
        self.assertEqual([e["vo"] for e in g["entries"]], ["nm01tst001_", "", ""])
        self.assertEqual(len(g["entries"]), 3)
        _, assets, _ = self.load_pack("tst")
        self.assertEqual([a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV], ["nm01tst001_"])
        st = self.manifest("tst")["dialog"]["stats"]
        self.assertEqual((st["voiced_lines"], st["silent_budget"], st["silent_no_wav"]), (1, 1, 1))

    def test_zero_budget_is_text_only(self):
        self.max_voiced = 0
        self.write_recipe()
        out, err = self.build_ok()
        _, assets, _ = self.load_pack("tst")
        self.assertEqual(len([a for a in assets if a.res_type in (kp.RES_TYPE_WAV, 3004)]), 0)
        self.assertEqual([c for c in self.calls() if "_dialog" in c and "lips" in c], [])

    def test_a_lip_that_fails_the_check_ships_without_it(self):
        self.lips["nm01tst002_"] = tdlg.BAD_LIP
        self.write_lips()
        out, err = self.build_ok()
        _, assets, _ = self.load_pack("tst")
        self.assertEqual([a.resref for a in assets if a.res_type == 3004], ["nm01tst001_"])
        self.assertEqual([a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV], ["nm01tst001_", "nm01tst002_"])  # the voice still plays
        self.assertIn("lip nm01tst002_ fails the LIP structure check", err)
        st = self.manifest("tst")["dialog"]["stats"]
        self.assertEqual((st["lips_ok"], [f["name"] for f in st["lips_failed"]]), (1, ["nm01tst002_"]))

    def test_a_missing_lip_warns(self):
        del self.lips["nm01tst001_"]
        self.write_lips()
        out, err = self.build_ok()
        _, assets, _ = self.load_pack("tst")
        self.assertEqual([a.resref for a in assets if a.res_type == 3004], ["nm01tst002_"])
        self.assertIn("have no lip in <K1>/lips/*.mod", err)
        self.assertEqual(self.manifest("tst")["dialog"]["stats"]["lips_missing"], ["nm01tst001_"])

    def test_no_streamwaves_dir_is_silent_text_with_a_warning(self):
        import shutil
        shutil.rmtree(self.k1 / "StreamWaves")
        out, err = self.build_ok()
        self.assertIn("no voice wavs found", err)
        _, g = self.dlg_out()
        self.assertEqual({e["vo"] for e in g["entries"]}, {""})
        self.assertEqual(self.manifest("tst")["dialog"]["stats"]["voiced_lines"], 0)

    def test_a_symlinked_wav_is_not_used(self):
        real = self.root / "elsewhere.wav"
        real.write_bytes(b"x" * 100)
        f = self.k1 / "StreamWaves" / "M01" / "TST" / "NM01TST002_.WAV"
        f.unlink()
        f.symlink_to(real)
        self.build_ok()
        _, assets, _ = self.load_pack("tst")
        self.assertEqual([a.resref for a in assets if a.res_type == kp.RES_TYPE_WAV], ["nm01tst001_"])

    def test_pack_asset_cap_is_a_failure_before_anything_is_written(self):
        with mock.patch.object(kp, "MAX_ASSETS", 12):  # 10 base assets + dlg + wavs + lips = 15
            rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("would hold 15 assets, over the 12-asset pack cap", err)
        self.assertIn("lower [dialog] max_voiced", err)
        self.assertFalse((self.out / "tst").exists())
        with mock.patch.object(kp, "MAX_ASSETS", 12):
            rc, out, err = self.run_cli("build", "--dry-run")
        self.assertEqual(rc, 1, out + err)

    def test_missing_dlg_and_missing_tlk_fail_clearly(self):
        (self.k1 / "_bif" / "k_htst_dialog.dlg.json").unlink()
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("K1 k_htst_dialog.dlg", err)
        self.assertIn("not found in the K1 install", err)
        self.assertFalse((self.out / "tst").exists())
        self.put_k1("k_htst_dialog.dlg.json", json.dumps(self.dlg_graph()).encode())
        (self.k1 / "dialog.tlk").unlink()
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("no dialog.tlk in the K1 dir", err)

    def test_every_starter_conditional_fails(self):
        g = self.dlg_graph()
        g["starters"] = [tdlg.link(0, active="k_a"), tdlg.link(0, active="k_b")]
        self.put_k1("k_htst_dialog.dlg.json", json.dumps(g).encode())
        rc, out, err = self.run_cli("build")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("none of its 2 starter(s) is free of a condition script", err)

    def test_k1_dlg_defaults_to_the_conversation_of_the_creature_template(self):
        self.k1_dlg_line = ""
        self.write_recipe()
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("no [dialog] k1_dlg", err)
        self.assertIn("set k1_dlg", err)
        self.put_k1("p_tst.utc.json", json.dumps({"conversation": "K_HTST_DIALOG"}).encode())  # recipe id tst -> K1 UTC p_tst
        out, err = self.build_ok("--only", "tst")
        self.assertEqual(self.manifest("tst")["dialog"]["k1_dlg"], "k_htst_dialog")

    def test_extra_barks_leave_room_for_the_dialog(self):
        self.write_table("soundset.2da", SND_HEADERS, LIVE_SND)
        self.write_recipe(AUDIO_RECIPE.format(id="tst"))
        refs = [1000 + i for i in range(5)] + [0xFFFFFFFF] * 23
        self.put_k1("p_tst.ssf", b"SSF V1.1" + struct.pack("<I", 12) + struct.pack("<28I", *refs) + struct.pack("<12I", *([0xFFFFFFFF] * 12)))
        names = ["p_tst_bat1", "p_tst_bat2", "p_tst_hit1", "p_tst_dead", "p_tst_dead"]
        tlk = dict(self.tlk)
        (self.k1 / "StreamSounds").mkdir()
        for i, vo in enumerate(names):
            tlk[str(1000 + i)] = vo
        for vo in sorted(set(names)) + ["p_tst_aaa", "p_tst_mmm", "p_tst_zzz"]:
            (self.k1 / "StreamSounds" / f"{vo.upper()}.wav").write_bytes(b"\xff\xfbK1MP3:" + vo.encode())
        for d in (self.k1, self.k2):
            (d / "dialog.tlk").write_text(json.dumps({k: v for k, v in tlk.items()}))
        # 6 model files + 3 tpc + utc + ssf + 4 SSF wavs = 15; the dialog adds 1 dlg + 2 wavs + 2 lips = 20; a cap of 22 leaves 2 extras
        with mock.patch.object(kp, "MAX_ASSETS", 22):
            out, err = self.build_ok()
        _, assets, _ = self.load_pack("tst")
        self.assertEqual(len(assets), 22)
        a = self.manifest("tst")["audio"]
        self.assertEqual([w["resref"] for w in a["extra"]], ["p_tst_aaa", "p_tst_mmm"])
        self.assertEqual(a["extra_dropped"], ["p_tst_zzz.wav"])
        self.assertIn("asset budget: 22/192", out)

    def test_extra_barks_reserved_bytes(self):
        self.write_recipe(AUDIO_RECIPE.format(id="tst"))
        recipe = imp.krecipe.load_recipe(self.recipes / "tst.toml")
        index = {}
        for n in ("p_tst_a.wav", "p_tst_b.wav"):
            f = self.root / n
            f.write_bytes(b"x" * 10)
            index[n] = f
        have = self.root / "have.wav"
        have.write_bytes(b"y" * 10)
        with mock.patch.object(kp, "MAX_FILE_BYTES", 2 * 30):  # half = 30: 10 used, 5 reserved for the dialog, 15 left
            extra, dropped = imp.extra_barks(recipe, index, {"p_tst_bat1": have}, {"p_tst_bat1"}, (0, 5))
        self.assertEqual((sorted(extra), dropped), (["p_tst_a"], ["p_tst_b.wav"]))

    def test_list_doctor_and_remove_show_the_dialog(self):
        self.build_ok()
        rc, out, _ = self.run_cli("list")
        self.assertEqual(rc, 0)
        self.assertIn("dialog k1p_tstdlg (5/6 nodes, 2 voiced, 2 lips)", out)
        rc, out, _ = self.run_cli("list", "--json")
        row = json.loads(out)["characters"][0]
        self.assertEqual(row["dialog"]["k2_dlg"], "k1p_tstdlg")
        self.assertEqual(row["dialog"]["built"]["voiced_lines"], 2)
        rc, out, err = self.run_cli("doctor")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("OK   K1 dialog.tlk [dialog] (tst)", out)
        self.assertIn("OK   K1 streamwaves/ [dialog] (tst)", out)
        self.assertIn("OK   K1 lips/ [dialog] (tst)", out)
        self.assertEqual(self.run_cli("install")[0], 0)
        rc, out, err = self.run_cli("remove", "tst")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("the pack also served dialog k1p_tstdlg (2 voice wav(s), 2 lip(s))", out)

    def test_doctor_warns_when_the_voice_dirs_are_missing(self):
        import shutil
        shutil.rmtree(self.k1 / "lips")
        shutil.rmtree(self.k1 / "StreamWaves")
        rc, out, err = self.run_cli("doctor")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("WARN K1 streamwaves/ [dialog] (tst)", out)
        self.assertIn("WARN K1 lips/ [dialog] (tst)", out)

    def test_build_reads_but_never_writes_the_game_dirs(self):
        k1_before, k2_before = snapshot(self.k1), snapshot(self.k2)
        self.build_ok()
        self.assertEqual((snapshot(self.k1), snapshot(self.k2)), (k1_before, k2_before))

    def test_rebuild_is_deterministic(self):
        self.build_ok()
        sha = imp.sha256_file(self.out / "tst.k2pack")
        self.build_ok()
        self.assertEqual(imp.sha256_file(self.out / "tst.k2pack"), sha)

    def test_a_recipe_without_dialog_is_unaffected(self):
        (self.recipes / "tst.toml").write_text(RECIPE.format(id="tst", name="Tst", conv="pykotor"))
        self.build_ok()
        self.assertEqual([c for c in self.calls() if "_dialog" in c], [])
        self.assertIsNone(self.manifest("tst")["dialog"])
        self.assertEqual(self.load_pack("tst")[0].default_dlg, "")


class InstallTests(Base):
    def test_install_copies_only_the_packs(self):
        self.build_ok()
        k1_before = snapshot(self.k1)
        k2_before = snapshot(self.k2)
        rc, out, err = self.run_cli("install")
        self.assertEqual(rc, 0, out + err)
        self.assertIn("Close the game", out)
        imports = self.k2 / "k1_imports"
        self.assertEqual(sorted(p.name for p in imports.iterdir()), ["tst.k2pack", "tsu.k2pack"])
        for cid in ("tst", "tsu"):
            self.assertEqual((imports / f"{cid}.k2pack").read_bytes(), (self.out / f"{cid}.k2pack").read_bytes())
        rec = json.loads((self.out / "installed.json").read_text())
        self.assertEqual(sorted(rec), ["tst", "tsu"])
        self.assertEqual(rec["tst"]["sha256"], imp.sha256_file(self.out / "tst.k2pack"))
        self.assertEqual(rec["tst"]["path"], str(imports / "tst.k2pack"))
        self.assertEqual(snapshot(self.k1), k1_before)
        after = snapshot(self.k2)
        added = {k for k in after if k not in k2_before}
        self.assertEqual(added, {"k1_imports", "k1_imports/tst.k2pack", "k1_imports/tsu.k2pack"})
        self.assertEqual({k: v for k, v in after.items() if k in k2_before}, k2_before)

    def test_install_only_and_dry_run(self):
        self.build_ok()
        k2_before = snapshot(self.k2)
        rc, out, _ = self.run_cli("install", "--dry-run")
        self.assertEqual(rc, 0)
        self.assertIn("would copy", out)
        self.assertEqual(snapshot(self.k2), k2_before)
        self.assertEqual(self.run_cli("install", "--only", "tsu")[0], 0)
        self.assertEqual([p.name for p in (self.k2 / "k1_imports").iterdir()], ["tsu.k2pack"])

    def test_install_needs_a_built_valid_pack(self):
        rc, _, err = self.run_cli("install")
        self.assertEqual(rc, 1)
        self.assertIn("pack not built", err)
        self.build_ok()
        (self.out / "tst.k2pack").write_bytes(b"garbage")
        rc, _, err = self.run_cli("install")
        self.assertEqual(rc, 1)
        self.assertIn("not a valid .k2pack", err)
        self.assertFalse((self.k2 / "k1_imports").exists())

    def test_install_detects_pack_changed_after_build(self):
        self.build_ok("--only", "tst")
        pack = self.out / "tst.k2pack"
        info, assets, cells = kp.read_pack(pack.read_bytes())
        kp.write_pack(pack, info, assets[:-1] + [kp.Asset("extra_one", 2022, 0, b"X")], cells)
        rc, _, err = self.run_cli("install", "--only", "tst")
        self.assertEqual(rc, 1)
        self.assertIn("changed after the build", err)

    def test_install_refuses_symlinked_imports_dir(self):
        self.build_ok("--only", "tst")
        real = self.root / "elsewhere"
        real.mkdir()
        (self.k2 / "k1_imports").symlink_to(real)
        rc, _, err = self.run_cli("install", "--only", "tst")
        self.assertEqual(rc, 2)
        self.assertEqual(list(real.iterdir()), [])

    def test_install_warns_when_juhani_char_is_present(self):
        self.build_ok("--only", "tst")
        (self.k2 / "patches" / "juhani-char.dll").write_bytes(b"MZ")
        rc, _, err = self.run_cli("install", "--only", "tst")
        self.assertEqual(rc, 0)
        self.assertNotIn("juhani-char", err)  # assigned rows are clear of 818-820 / 249-251
        assign = self.out / "assignments.json"
        a = json.loads(assign.read_text())
        a["tst"]["appearance"] = 819
        assign.write_text(json.dumps(a))
        rc, _, err = self.run_cli("install", "--only", "tst")
        self.assertEqual(rc, 0)
        self.assertIn("juhani-char", err)

    def test_remove_deletes_only_that_file(self):
        self.build_ok()
        self.run_cli("install")
        stray = self.k2 / "k1_imports" / "other.k2pack"
        stray.write_bytes(b"not ours")
        rc, out, err = self.run_cli("remove", "tst")
        self.assertEqual(rc, 0, out + err)
        self.assertFalse((self.k2 / "k1_imports" / "tst.k2pack").exists())
        self.assertTrue((self.k2 / "k1_imports" / "tsu.k2pack").exists())
        self.assertEqual(stray.read_bytes(), b"not ours")
        self.assertEqual(sorted(json.loads((self.out / "installed.json").read_text())), ["tsu"])
        self.assertEqual(sorted(json.loads((self.out / "assignments.json").read_text())), ["tsu"])
        self.assertIn("stay unused", out)
        self.assertTrue((self.out / "tst.k2pack").exists())  # the built pack stays

    def test_remove_refuses_unlisted_ids(self):
        self.build_ok()
        self.run_cli("install", "--only", "tst")
        victim = self.k2 / "k1_imports" / "tsu.k2pack"
        victim.write_bytes(b"placed by hand")
        rc, _, err = self.run_cli("remove", "tsu")
        self.assertEqual(rc, 1)
        self.assertIn("not listed as installed", err)
        self.assertEqual(victim.read_bytes(), b"placed by hand")
        self.assertEqual(self.run_cli("remove", "../escape")[0], 1)

    def test_remove_refuses_a_tampered_record(self):
        self.build_ok("--only", "tst")
        self.run_cli("install", "--only", "tst")
        target = self.root / "precious.k2pack"
        target.write_bytes(b"keep")
        rec = json.loads((self.out / "installed.json").read_text())
        rec["tst"]["path"] = str(target)
        (self.out / "installed.json").write_text(json.dumps(rec))
        rc, _, err = self.run_cli("remove", "tst")
        self.assertEqual(rc, 2)
        self.assertEqual(target.read_bytes(), b"keep")


class ListTests(Base):
    def test_list_json_and_text(self):
        rc, out, _ = self.run_cli("list", "--json")
        self.assertEqual(rc, 0)
        rows = {r["id"]: r for r in json.loads(out)["characters"]}
        self.assertEqual((rows["tst"]["built"], rows["tst"]["installed"], rows["tst"]["assigned"]), (False, False, None))
        self.build_ok()
        self.run_cli("install", "--only", "tst")
        rc, out, _ = self.run_cli("list", "--json")
        rows = {r["id"]: r for r in json.loads(out)["characters"]}
        self.assertTrue(rows["tst"]["built"] and rows["tst"]["installed"] and rows["tst"]["installed_matches_build"])
        self.assertEqual(rows["tst"]["pack_sha256"], imp.sha256_file(self.out / "tst.k2pack"))
        self.assertFalse(rows["tsu"]["installed"])
        self.assertEqual(rows["tsu"]["assigned"], {"appearance": LIVE_APP + 1, "head": LIVE_HEAD + 1})
        rc, text, _ = self.run_cli("list")
        self.assertEqual(rc, 0)
        self.assertIn("tst", text)
        self.assertIn("rows 20/5", text)
        self.assertIn("not installed", text)

    def test_stale_after_recipe_edit(self):
        self.build_ok("--only", "tst")
        with open(self.recipes / "tst.toml", "a") as fh:
            fh.write("\n# edited\n")
        rows = {r["id"]: r for r in json.loads(self.run_cli("list", "--json")[1])["characters"]}
        self.assertTrue(rows["tst"]["stale"])


class MdlopsTests(Base):
    converters = {"tsu": "mdlops"}

    def test_mdlops_branch_uses_rename_pass2_and_its_own_finish(self):
        self.build_ok("--only", "tsu")
        calls = self.script_calls("k1_port_mdlops.py")
        subs = [c[next(i for i, x in enumerate(c) if x.endswith("k1_port_mdlops.py")) + 1] for c in calls]
        self.assertEqual(subs, ["extract", "pass1", "rename", "pass2"])  # never `finish`/`all`: those write the shared stage
        rename = next(c for c in calls if "rename" in c)
        self.assertIn("p_tsuba=k1p_tsuba", rename)
        self.assertIn("p_tsuh01=k1p_tsuh01", rename)
        run = str(self.out / "tsu" / "mdlops_run")
        self.assertTrue(all(c[c.index("--run") + 1] == run for c in calls))
        self.assertEqual(len(self.script_calls("k1_port_validate.py")), 3)
        _, assets, _ = self.load_pack("tsu")
        mdl = next(a for a in assets if a.resref == "k1p_tsuba" and a.res_type == 2002)
        self.assertEqual(mdl.data, b"MDLOPS:k1p_tsuba.mdl")
        # textures come from the textures-only ingest
        ing = self.script_calls("k1_port_ingest.py")
        self.assertEqual(len(ing), 1)
        self.assertIn("--no-model", ing[0])

    def test_mdlops_validation_failure_stops(self):
        os.environ["FAKE_VALIDATE_FAIL"] = "1"
        rc, _, err = self.run_cli("build", "--only", "tsu")
        self.assertEqual(rc, 1)
        self.assertIn("validate k1p_tsuba", err)
        self.assertFalse((self.out / "tsu.k2pack").exists())


class MeshTextureTests(Base):
    def ingest_textures(self, resref):
        for c in self.script_calls("k1_port_ingest.py"):
            if "--resref" in c and c[c.index("--resref") + 1] == resref and "--no-model" not in c:
                return sorted(c[i + 1] for i, x in enumerate(c) if x == "--texture")
        return None

    def test_shared_head_texture_goes_to_both_meshes(self):
        # Carth: body B also uses the head texture (P_CarthH01); ingest needs every texture the mesh references
        self.put_k1("p_tstbb.mdl", b"K1:p_tstbb.mdl\0P_TSTBB01\0junk\0P_TSTH01\0")
        self.build_ok("--only", "tst")
        self.assertEqual(self.ingest_textures("p_tstbb"), ["p_tstbb01=k1p_tstbb01", "p_tsth01=k1p_tsth01"])
        self.assertEqual(self.ingest_textures("p_tsth"), ["p_tsth01=k1p_tsth01"])
        self.assertEqual(self.ingest_textures("p_tstba"), ["p_tstba01=k1p_tstba01"])

    def test_mesh_without_recipe_texture_fails(self):
        self.put_k1("p_tstbb.mdl", b"K1:p_tstbb.mdl\0OTHER01\0")
        rc, out, err = self.run_cli("build", "--only", "tst")
        self.assertEqual(rc, 1, out + err)
        self.assertIn("references none of the recipe", err)


class CellBudgetTests(Base):
    def put_manifest(self, cid, cells):
        self.out.mkdir(exist_ok=True)
        (self.out / f"{cid}.import_manifest.json").write_text(json.dumps({"pack": {"cell_count": cells}}))

    def budget(self):
        err = io.StringIO()
        with contextlib.redirect_stderr(err), contextlib.redirect_stdout(io.StringIO()):
            total = imp.warn_cell_budget(imp.load_all(str(self.config)))
        return total, err.getvalue()

    def test_over_pool_warns(self):
        half = imp.CELL_POOL // 2 + 1
        self.put_manifest("tst", half)
        self.put_manifest("tsu", half)
        total, err = self.budget()
        self.assertEqual(total, 2 * half)
        self.assertIn("2DA cell budget", err)

    def test_within_pool_quiet(self):
        self.put_manifest("tst", 102)
        total, err = self.budget()
        self.assertEqual(total, 102)
        self.assertNotIn("WARN", err)

    def test_build_reports_budget(self):
        out, _ = self.build_ok()
        self.assertIn("2DA cell budget", out)


class TpcAdaptTests(unittest.TestCase):
    @staticmethod
    def tpc(w=8, h=8, mips=4, ptype=2, trailer=b""):
        import struct
        size = 0
        sizes = []
        for k in range(mips):
            sz = (-(-max(1, w >> k) // 4)) * (-(-max(1, h >> k) // 4)) * 8
            sizes.append(sz)
        size = sizes[0]
        hdr = struct.pack("<IfHHBB", size, 1.0, w, h, ptype, mips)
        return hdr + b"\0" * (128 - len(hdr)) + b"\1" * sum(sizes) + trailer

    def test_trailer_split_and_txi_replace(self):
        tex = imp.krecipe.TextureRef("a", "b", True, "clamp 1")
        self.assertEqual(imp.tpc_split(self.tpc(), "t"), b"")
        out = imp.adapt_tpc(self.tpc(), tex, "t")
        self.assertEqual(imp.tpc_split(out, "t"), b"clamp 1\n")
        self.assertEqual(imp.adapt_tpc(out, tex, "t"), out)  # already present: unchanged
        has = self.tpc(trailer=b"clamp   1\n")
        self.assertEqual(imp.adapt_tpc(has, tex, "t"), has)  # whitespace-insensitive match
        k1_head = self.tpc(trailer=b"clamp 3\r\n")  # K1 head TPCs carry `clamp 3`: the recipe text wins
        self.assertEqual(imp.tpc_split(imp.adapt_tpc(k1_head, tex, "t"), "t"), b"clamp 1\n")
        plain = imp.krecipe.TextureRef("a", "b", True, "")
        self.assertEqual(imp.adapt_tpc(self.tpc(), plain, "t"), self.tpc())
        self.assertEqual(imp.adapt_tpc(k1_head, plain, "t"), k1_head)  # no recipe txi: trailer kept

    def test_non_dxt1_is_refused(self):
        with self.assertRaises(imp.Failure) as cm:
            imp.tpc_split(self.tpc(ptype=4), "t")
        self.assertIn("not DXT1", str(cm.exception))
        with self.assertRaises(imp.Failure):
            imp.tpc_split(self.tpc()[:150], "t")


if __name__ == "__main__":
    unittest.main(verbosity=1)
