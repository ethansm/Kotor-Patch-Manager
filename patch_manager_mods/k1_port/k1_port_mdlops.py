#!/usr/bin/env python3
"""MDLOps route for K1 -> K2 models (the recipe that rendered Juhani correctly in K2), as one script.

Pipeline per model set (run dir default ~/Games/KOTOR2_Mod_Workspace/mdlops_t7/juhani_run2):
  extract  K1 model + supermodel chain from the K1 install -> <run>/work/<name>.mdl/.mdx   (needs the k1port venv)
  pass1    MDLOps (Proton) binary -> ASCII:  <run>/work/<name>-ascii.mdl
  rename   pure Python: ASCII with the new k1p_* names -> <run>/pass2/<new>-ascii.mdl
  pass2    MDLOps -k2: ASCII -> K2 binary:   <run>/pass2/<new>-ascii-k2-bin.mdl/.mdx
  finish   validate (k1_port_validate), register in the ledger (k1_port_prune adopt), copy to
           <stage>/<stage_sub>/models/<new>.mdl/.mdx
  all      rename + pass2 + finish for the listed models (pass1 and extract are separate: they read the K1 install)

Rename rules (10-16 lesson): the model name, the root node, the setsupermodel target and the root's parent get the
new name (word-boundary, case-insensitive, one pass, so nothing is replaced twice); bitmap fields map to k1p_<x>
(lowercase, a leading p_ is dropped, NULL stays); node and bone names KEEP the K1 casing. The old build lowercased
node names, which broke exact-name matches against S_Female03 (58/64 -> 32/64); --lowercase-nodes restores that
for an A/B build.

Examples:
  k1_port_mdlops.py extract --models p_juhaniba p_juhanibb p_juhanih
  k1_port_mdlops.py pass1   --models p_juhaniba p_juhanibb p_juhanih
  k1_port_mdlops.py all     --map p_juhaniba=k1p_juhaniba p_juhanibb=k1p_juhanibb p_juhanih=k1p_juhanih \
                            --stage-sub probe_mdlops_juhani2

Only `extract` needs pykotor (run it with the k1port venv python); every other subcommand is stdlib only.
The K1 install is read, never written. Exit: 0 ok, 1 runtime failure, 2 bad arguments.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

WORKSPACE = Path("~/Games/KOTOR2_Mod_Workspace").expanduser()
DEFAULT_RUN = WORKSPACE / "mdlops_t7" / "juhani_run2"
PREFIX = WORKSPACE / "mdlops_t7" / "mdl_run1" / "prefix"
TOOL_SRC = WORKSPACE / "mdlops_t7" / "juhani_run" / "tools" / "mdlops.exe"
PROTON = Path("~/.local/share/Steam/steamapps/common/Proton - Experimental/proton").expanduser()
STAGE = WORKSPACE / "k1_port_stage"
DEFAULT_K1 = "~/.local/share/Steam/steamapps/common/swkotor"
VALIDATE = HERE / "k1_port_validate.py"
PRUNE = HERE / "k1_port_prune.py"
SUPER_OFF = 12 + 136  # supermodel name inside a binary MDL (12-byte resource header + geometry header)
SUPER_LEN = 32


class MdlopsError(Exception):
    pass


# ---- pure rename logic (unit-tested) -------------------------------------------------------------------------

def parse_map(items):
    """['p_juhaniba=k1p_juhaniba', ...] -> {'p_juhaniba': 'k1p_juhaniba'} (keys lowercase)."""
    out = {}
    for item in items or []:
        old, sep, new = item.partition("=")
        if not sep or not old or not new:
            raise MdlopsError(f"bad mapping {item!r}: expected OLD=NEW")
        out[old.lower()] = new
    return out


def default_texture(name):
    """P_JuhaniBB01 -> k1p_juhanibb01. NULL (the no-texture sentinel) is kept."""
    if name.upper() == "NULL":
        return name
    low = name.lower()
    if low.startswith("p_"):
        low = low[2:]
    return low if low.startswith("k1p_") else "k1p_" + low


def _name_regex(names):
    alt = "|".join(re.escape(n) for n in sorted(names, key=len, reverse=True))
    return re.compile(rf"(?<![A-Za-z0-9_])({alt})(?![A-Za-z0-9_])", re.IGNORECASE)


def rename_ascii(text, model_map, tex_map=None, lowercase_nodes=False):
    """Rename an MDLOps ASCII model. Returns the new text (CRLF normalised to LF).

    model_map: {old_lower: new}; texture mapping: tex_map {old_lower: new}, else default_texture().
    """
    tex_map = {k.lower(): v for k, v in (tex_map or {}).items()}
    rx = _name_regex(model_map) if model_map else None
    lines_out = []
    for line in text.replace("\r\n", "\n").replace("\r", "\n").split("\n"):
        m = re.match(r"^(\s*)(bitmap2?)(\s+)(\S+)(.*)$", line)
        if m:
            new = tex_map.get(m.group(4).lower()) or default_texture(m.group(4))
            lines_out.append(f"{m.group(1)}{m.group(2)}{m.group(3)}{new}{m.group(5)}")
            continue
        if rx is not None:
            line = rx.sub(lambda mo: model_map[mo.group(1).lower()], line)
        if lowercase_nodes:
            nm = re.match(r"^(\s*node\s+\S+\s+)(\S+)(.*)$", line)
            pm = re.match(r"^(\s*parent\s+)(\S+)(.*)$", line)
            for mo in (nm, pm):
                if mo and mo.group(2).upper() != "NULL":
                    line = mo.group(1) + mo.group(2).lower() + mo.group(3)
                    break
        lines_out.append(line)
    return "\n".join(lines_out)


def node_names(text):
    """Node names of an MDLOps ASCII model, in file order."""
    return re.findall(r"^\s*node\s+\S+\s+(\S+)\s*$", text.replace("\r", ""), re.MULTILINE)


def supermodel_of(mdl_bytes):
    raw = mdl_bytes[SUPER_OFF:SUPER_OFF + SUPER_LEN]
    return raw.split(b"\0", 1)[0].decode("latin1")


# ---- file / process plumbing ---------------------------------------------------------------------------------

def run_dirs(run):
    run = Path(run).expanduser()
    return run, run / "work", run / "pass2"


def ensure_run(run):
    run, work, pass2 = run_dirs(run)
    for d in (work, pass2, run / "tools"):
        d.mkdir(parents=True, exist_ok=True)
    exe = run / "tools" / "mdlops.exe"
    if not exe.exists():
        if not TOOL_SRC.is_file():
            raise MdlopsError(f"mdlops.exe not found at {TOOL_SRC}")
        shutil.copy2(TOOL_SRC, exe)
    return run, work, pass2


def win_path(p):
    return "Z:" + str(p).replace("/", "\\")


def run_mdlops(run, cwd, args, log):
    if not PROTON.is_file():
        raise MdlopsError(f"Proton not found: {PROTON}")
    env = dict(os.environ, STEAM_COMPAT_CLIENT_INSTALL_PATH=str(Path("~/.local/share/Steam").expanduser()),
               STEAM_COMPAT_DATA_PATH=str(PREFIX))
    cmd = [str(PROTON), "run", win_path(run / "tools" / "mdlops.exe"), *args]
    with open(log, "w") as fh:
        rc = subprocess.run(cmd, cwd=cwd, env=env, stdout=fh, stderr=subprocess.STDOUT, timeout=900).returncode
        fh.write(f"rc={rc}\n")
    return rc


# ---- subcommands ---------------------------------------------------------------------------------------------

def cmd_extract(a):
    import k1_port_model as K  # needs the k1port venv for load_pykotor
    run, work, _ = ensure_run(a.run)
    pk = K.load_pykotor()
    lib = K.Library(pk, Path(a.k1).expanduser(), None)
    todo, seen = list(a.models), set()
    while todo:
        name = todo.pop(0).lower()
        if name in seen or name == "null":
            continue
        seen.add(name)
        got = lib.from_install(name)
        if got is None:
            raise MdlopsError(f"{name}: not found in the K1 install")
        _, _, mdl, mdx = got
        (work / f"{name}.mdl").write_bytes(mdl)
        (work / f"{name}.mdx").write_bytes(mdx)
        sup = supermodel_of(mdl).lower()
        print(f"extract {name}: {len(mdl)} B mdl, {len(mdx)} B mdx, supermodel {sup or '-'}")
        if sup:
            todo.append(sup)
    return 0


def cmd_pass1(a):
    run, work, _ = ensure_run(a.run)
    for m in a.models:
        m = m.lower()
        if not (work / f"{m}.mdl").is_file():
            raise MdlopsError(f"{m}.mdl missing in {work}: run extract first")
        rc = run_mdlops(run, work, [win_path(work / f"{m}.mdl")], run / f"pass1_{m}.log")
        out = work / f"{m}-ascii.mdl"
        print(f"pass1 {m}: rc={rc} {'ok' if out.is_file() else 'NO OUTPUT'}")
        if not out.is_file():
            return 1
    return 0


def cmd_rename(a):
    run, work, pass2 = ensure_run(a.run)
    model_map = parse_map(a.map)
    tex_map = parse_map(a.tex)
    if not model_map:
        raise MdlopsError("--map OLD=NEW is required")
    for old, new in model_map.items():
        src = work / f"{old}-ascii.mdl"
        if not src.is_file():
            raise MdlopsError(f"{src} missing: run pass1 first")
        text = rename_ascii(src.read_text(encoding="latin1"), model_map, tex_map, a.lowercase_nodes)
        (pass2 / f"{new}-ascii.mdl").write_text(text, encoding="latin1", newline="\n")
        print(f"rename {old} -> {new} ({'lowercase' if a.lowercase_nodes else 'cased'} nodes)")
    # pass2 reads the K1 binary of each model's supermodel under its NEW name (k1p_juhaniba -> k1p_juhanibb.mdl)
    for old, new in model_map.items():
        for ext in (".mdl", ".mdx"):
            if (work / f"{old}{ext}").is_file():
                shutil.copy2(work / f"{old}{ext}", pass2 / f"{new}{ext}")
    # unrenamed supermodels (s_female03 ...) are read from the same dir
    for f in work.glob("*.mdl"):
        if not f.name.endswith("-ascii.mdl"):
            for ext in (".mdl", ".mdx"):
                tgt = pass2 / (f.stem + ext)
                if (work / (f.stem + ext)).is_file() and not tgt.exists():
                    shutil.copy2(work / (f.stem + ext), tgt)
    return 0


def cmd_pass2(a):
    run, _, pass2 = ensure_run(a.run)
    for new in parse_map(a.map).values():
        if not (pass2 / f"{new}-ascii.mdl").is_file():
            raise MdlopsError(f"{new}-ascii.mdl missing: run rename first")
        rc = run_mdlops(run, pass2, ["-k2", win_path(pass2 / f"{new}-ascii.mdl")], run / f"pass2_{new}.log")
        ok = (pass2 / f"{new}-ascii-k2-bin.mdl").is_file() and (pass2 / f"{new}-ascii-k2-bin.mdx").is_file()
        print(f"pass2 {new}: rc={rc} {'ok' if ok else 'NO OUTPUT'}")
        if not ok:
            return 1
    return 0


def cmd_finish(a):
    run, _, pass2 = ensure_run(a.run)
    out_dir = STAGE / a.stage_sub / "models"
    out_dir.mkdir(parents=True, exist_ok=True)
    for new in parse_map(a.map).values():
        mdl, mdx = pass2 / f"{new}-ascii-k2-bin.mdl", pass2 / f"{new}-ascii-k2-bin.mdx"
        if not (mdl.is_file() and mdx.is_file()):
            raise MdlopsError(f"{new}: pass2 output missing")
        v = subprocess.run([sys.executable, str(VALIDATE), str(mdl), "--mdx", str(mdx), "--game", "K2"],
                           capture_output=True, text=True)
        print(f"validate {new}: {v.stdout.strip().splitlines()[-1] if v.stdout.strip() else v.stderr.strip()}")
        if v.returncode != 0:
            print(v.stdout)
            return 1
        for src, ext in ((mdl, ".mdl"), (mdx, ".mdx")):
            dst = out_dir / f"{new}{ext}"
            shutil.copy2(src, dst)
            r = subprocess.run([sys.executable, str(PRUNE), "adopt", "--file", str(dst), "--kind", "model",
                                "--source-id", "vanilla_k1"], capture_output=True, text=True,
                               env=dict(os.environ, K1PORT_LEDGER=str(STAGE / "ledger.json")))
            print(r.stdout.strip() or r.stderr.strip())
            if r.returncode != 0:
                return 1
    return 0


def cmd_all(a):
    for fn in (cmd_rename, cmd_pass2, cmd_finish):
        rc = fn(a)
        if rc:
            return rc
    return 0


def build_parser():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--run", default=str(DEFAULT_RUN), help="run directory (work/, pass2/, tools/)")
    p = sub.add_parser("extract", parents=[common], help="K1 model + supermodel chain -> work/")
    p.add_argument("--models", nargs="+", required=True)
    p.add_argument("--k1", default=DEFAULT_K1)
    p.set_defaults(fn=cmd_extract)
    p = sub.add_parser("pass1", parents=[common], help="MDLOps binary -> ASCII")
    p.add_argument("--models", nargs="+", required=True)
    p.set_defaults(fn=cmd_pass1)
    for name, fn, helptext in (("rename", cmd_rename, "ASCII rename"), ("pass2", cmd_pass2, "MDLOps -k2"),
                               ("finish", cmd_finish, "validate + adopt + copy to stage"),
                               ("all", cmd_all, "rename + pass2 + finish")):
        p = sub.add_parser(name, parents=[common], help=helptext)
        p.add_argument("--map", nargs="+", required=True, metavar="OLD=NEW", help="model renames")
        p.add_argument("--tex", nargs="*", default=[], metavar="OLD=NEW", help="texture renames (default k1p_<x>)")
        p.add_argument("--lowercase-nodes", action="store_true", help="old behaviour: lowercase node names")
        p.add_argument("--stage-sub", default="probe_mdlops_juhani2", help="stage subdirectory for finish")
        p.set_defaults(fn=fn)
    return ap


def main(argv=None):
    a = build_parser().parse_args(argv)
    try:
        return a.fn(a)
    except MdlopsError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
