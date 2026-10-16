#!/usr/bin/env python3
"""Build the data files of one imported K1 character (body A + body B + head) from the live game. Read only.

Writes ONLY into --out (default <stage>/char_<tag>/override):
  appearance.2da     live table + one row (copy of --template-row, then race/modela/texa/modelb/texb/normalhead set)
  heads.2da          live table + one row (head = --head, every texture column empty)
  <utc>.utc          creature template copied from a module UTC (probe default: c_ithorian003 in 204tel.mod) with
                     the new appearance row, soundset, gender, tag/resref and a literal first name
  char_build.json    row numbers, soundset, sha256 of every output and of the live base files
Every check runs before the first write. A refusal raises BuildError (CLI: 'error: ...' on stderr, exit 1).

--new-row must equal the live appearance.2da row count and --new-head-row the live heads.2da row count: the rows
are appended, so a live table that grew since the plan (another mod added rows) is refused, not overwritten.
The game dir is never written (an --out inside it is refused). Nothing here installs anything.

Needs pykotor; the CLI re-executes itself under the k1port venv (python -I) when it is not importable.
Usage: k1_port_char_build.py --game-dir DIR --tag juhani --label K1P_Juhani --new-row 818 --new-head-row 249
       --body-a k1p_juhaniba --body-b k1p_juhanibb --head k1p_juhanih --soundset 57 --gender 1 --name Juhani [--json]
"""
import argparse
import json
import os
import re
import sys
from pathlib import Path
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.dont_write_bytecode = True
import k1_port_probe_build as pb  # noqa: E402  (shared primitives: BuildError, _resolve_dirs, venv re-exec)

BuildError = pb.BuildError
DEFAULT_STAGE = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage").expanduser()
TEMPLATE_ROW = 17  # N_CommM human NPC row, the row the spawn probe copied (modeltype B)
CLEARED = ("texaevil", "texbevil", "backuphead")
RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
APPEARANCE_REL, HEADS_REL, SOUNDSET_REL = "override/appearance.2da", "override/heads.2da", "override/soundset.2da"


def _pykotor() -> SimpleNamespace:
    api = pb._pykotor()
    from pykotor.common.language import LocalizedString
    api.LocalizedString = LocalizedString
    from pykotor.common.misc import ResRef
    api.ResRef = ResRef
    return api


def _check(args: dict) -> None:
    for key in ("body_a", "body_b", "head", "utc"):
        if not RESREF_RE.fullmatch(args[key]):
            raise BuildError(f"{key} must be 1-16 chars of [a-z0-9_], got {args[key]!r}")
    for key in ("new_row", "new_head_row", "template_row", "soundset", "texture_var"):
        v = args[key]
        if isinstance(v, bool) or not isinstance(v, int) or v < 0:
            raise BuildError(f"{key} must be an int >= 0, got {v!r}")
    if args["gender"] not in (0, 1):
        raise BuildError(f"gender must be 0 (male) or 1 (female), got {args['gender']!r}")
    if not re.fullmatch(r"[A-Za-z0-9_]{1,32}", args["label"]):
        raise BuildError(f"label must be 1-32 chars of [A-Za-z0-9_], got {args['label']!r}")
    if not (1 <= len(args["name"]) <= 64 and args["name"].isprintable()):
        raise BuildError("name must be 1-64 printable characters")


def _read_table(api, game: Path, rel: str) -> tuple[bytes, object]:
    p = game / rel
    if not p.is_file():
        raise BuildError(f"missing live file: {p}")
    raw = p.read_bytes()
    return raw, api.read_2da(raw)


def build(game_dir, out_dir, *, tag: str, label: str, new_row: int, new_head_row: int, body_a: str, body_b: str,
          head: str, soundset: int, gender: int, name: str, utc: str | None = None, texture_var: int = 1,
          template_row: int = TEMPLATE_ROW, module: str = pb.PROBE_MODULE, template_utc: str = pb.PROBE_UTC,
          expect_appearance: int = pb.PROBE_EXPECT, chain_from=None, conversation: str | None = None,
          soundset_table=None) -> dict:
    """Write the character data files into out_dir. Returns the char_build.json dict. Writes nothing on refusal.

    chain_from: a previous package's override dir. Its appearance.2da/heads.2da replace the live tables as the base
    (so --new-row/--new-head-row count on from that package); install that package first. Soundset, module and
    the template UTC still come from the live game.
    conversation: DLG resref for the UTC (default: the template's, e.g. the K2 Ithorian dialog).
    soundset_table: a soundset.2da file used for the --soundset range check instead of the live one (a staged
    table with appended rows, installed alongside)."""
    utc = utc or f"k1p_{tag}"
    _check(dict(body_a=body_a, body_b=body_b, head=head, utc=utc, new_row=new_row, new_head_row=new_head_row,
                template_row=template_row, soundset=soundset, texture_var=texture_var, gender=gender, label=label,
                name=name))
    if conversation is not None and not RESREF_RE.fullmatch(conversation):
        raise BuildError(f"conversation must be 1-16 chars of [a-z0-9_], got {conversation!r}")
    game, out = pb._resolve_dirs(game_dir, out_dir)
    api = _pykotor()
    base = game
    if chain_from is not None:
        base = Path(chain_from).expanduser().resolve()
        for rel in (APPEARANCE_REL, HEADS_REL):
            if not (base / Path(rel).name).is_file():
                raise BuildError(f"--chain-from {base} lacks {Path(rel).name}")
    rel_app, rel_heads = (Path(APPEARANCE_REL).name, Path(HEADS_REL).name) if chain_from else (APPEARANCE_REL, HEADS_REL)
    app_raw, app = _read_table(api, base, rel_app)
    heads_raw, heads = _read_table(api, base, rel_heads)
    if soundset_table is None:
        _sraw, sounds = _read_table(api, game, SOUNDSET_REL)
    else:
        st = Path(soundset_table).expanduser()
        if not st.is_file():
            raise BuildError(f"missing soundset table: {st}")
        sounds = api.read_2da(st.read_bytes())
    if app.get_height() != new_row:
        raise BuildError(f"live appearance.2da has {app.get_height()} rows but --new-row is {new_row}; the table "
                         "changed (another mod added rows?): re-plan with the live count")
    if heads.get_height() != new_head_row:
        raise BuildError(f"live heads.2da has {heads.get_height()} rows but --new-head-row is {new_head_row}; re-plan")
    if template_row >= app.get_height():
        raise BuildError(f"template row {template_row} is outside 0..{app.get_height() - 1}")
    if soundset >= sounds.get_height():
        raise BuildError(f"soundset {soundset} is outside soundset.2da 0..{sounds.get_height() - 1}")

    app.add_row(str(new_row), {c: app.get_cell(template_row, c) for c in app.get_headers() if c != "label"})
    # Vanilla shape: the row id (first column) stays the numeric index; the name goes in the `label` data column.
    for col, val in (("label", label), ("race", body_b), ("modela", body_a), ("texa", body_a), ("modelb", body_b), ("texb", body_b),
                     ("normalhead", str(new_head_row))):
        app.set_cell(new_row, col, val)
    for col in CLEARED:
        app.set_cell(new_row, col, "")
    heads.add_row(str(new_head_row), {c: "" for c in heads.get_headers() if c != "head"})
    heads.set_cell(new_head_row, "head", head)
    if app.get_height() != new_row + 1 or heads.get_height() != new_head_row + 1:
        raise BuildError("row append did not grow the tables by one")

    mod = game / f"modules/{module}.mod"
    if not mod.is_file():
        raise BuildError(f"missing live file: {mod}")
    hits = [r for r in api.read_erf(mod.read_bytes())
            if r.resref.get().lower() == template_utc and r.restype == api.ResourceType.UTC]
    if len(hits) != 1:
        raise BuildError(f"expected one {template_utc}.utc in {mod.name}, found {len(hits)}")
    u = api.read_utc(hits[0].data)
    if u.appearance_id != expect_appearance:
        raise BuildError(f"{template_utc}.utc appearance_id is {u.appearance_id}, expected {expect_appearance}")
    u.appearance_id, u.texture_variation, u.soundset_id, u.gender_id = new_row, texture_var, soundset, gender
    u.tag = u.resref = utc
    u.first_name = api.LocalizedString.from_english(name)
    if conversation is not None:
        u.conversation = api.ResRef(conversation)

    files = {"appearance.2da": api.bytes_2da(app), "heads.2da": api.bytes_2da(heads), f"{utc}.utc": api.bytes_utc(u)}
    info = {"tag": tag, "label": label, "appearance_row": new_row, "heads_row": new_head_row, "utc": utc,
            "template_row": template_row, "body_a": body_a, "body_b": body_b, "head": head, "soundset": soundset,
            "gender": gender, "texture_var": texture_var, "name": name, "conversation": str(u.conversation),
            "sha256": {n: pb.sha256_hex(b) for n, b in files.items()},
            "chained_from": None if chain_from is None else str(base),
            "base_sha256": {"appearance.2da": pb.sha256_hex(app_raw), "heads.2da": pb.sha256_hex(heads_raw)}}
    try:
        out.mkdir(parents=True, exist_ok=True)
        for n, b in files.items():
            (out / n).write_bytes(b)
        (out / "char_build.json").write_text(json.dumps(info, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    except OSError as exc:
        raise BuildError(f"cannot write into {out}: {exc}") from exc
    return info


def _reexec_under_venv(argv: list) -> None:
    """If pykotor is not importable, re-run THIS file under the k1port venv python -I. Does not return then."""
    try:
        import pykotor  # noqa: F401
        return
    except ImportError:
        pass
    venv = pb.VENV_PY
    if Path(sys.prefix).resolve() == venv.parent.parent.resolve():
        raise BuildError(f"pykotor not importable inside the venv {venv.parent.parent}")
    if not venv.exists():
        raise BuildError(f"pykotor not importable and the venv python is missing: {venv}")
    os.execv(str(venv), [str(venv), "-I", str(Path(__file__).resolve()), *argv])


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Build appearance/heads rows and a UTC for one imported K1 character "
                                             "(game dir is read only).")
    ap.add_argument("--game-dir", required=True, metavar="DIR", help="KOTOR II install dir (read only)")
    ap.add_argument("--tag", required=True, help="character tag, e.g. juhani (UTC resref/tag default k1p_<tag>)")
    ap.add_argument("--label", required=True, help="appearance.2da label of the new row, e.g. K1P_Juhani")
    ap.add_argument("--new-row", type=int, required=True, help="must equal the live appearance.2da row count")
    ap.add_argument("--new-head-row", type=int, required=True, help="must equal the live heads.2da row count")
    ap.add_argument("--body-a", required=True, metavar="RESREF", help="modela/texa (no armour body)")
    ap.add_argument("--body-b", required=True, metavar="RESREF", help="modelb/texb and race")
    ap.add_argument("--head", required=True, metavar="RESREF", help="head model of the new heads.2da row")
    ap.add_argument("--soundset", type=int, required=True, help="existing K2 soundset.2da row (read only)")
    ap.add_argument("--gender", type=int, required=True, choices=(0, 1), help="0 male, 1 female")
    ap.add_argument("--name", required=True, help="literal first name")
    ap.add_argument("--utc", default=None, help="UTC resref/tag (default k1p_<tag>)")
    ap.add_argument("--texture-var", type=int, default=1, help="UTC texture_variation (default 1: runtime spawns get 1)")
    ap.add_argument("--template-row", type=int, default=TEMPLATE_ROW, help="appearance row index to copy")
    ap.add_argument("--module", default=pb.PROBE_MODULE, help="module capsule holding the template UTC")
    ap.add_argument("--template-utc", default=pb.PROBE_UTC)
    ap.add_argument("--expect-appearance", type=int, default=pb.PROBE_EXPECT)
    ap.add_argument("--chain-from", default=None, metavar="DIR",
                    help="a previous package's override dir: its appearance/heads tables are the base (install it first)")
    ap.add_argument("--out", default=None, metavar="DIR", help="output dir (default <stage>/char_<tag>/override)")
    ap.add_argument("--conversation", default=None, metavar="RESREF", help="UTC dialog resref (default: the template's)")
    ap.add_argument("--soundset-table", default=None, metavar="FILE",
                    help="soundset.2da used for the --soundset range check (default: the live table)")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args(argv)
    out = args.out or str(DEFAULT_STAGE / f"char_{args.tag}" / "override")
    try:
        _reexec_under_venv(sys.argv[1:] if argv is None else list(argv))
        res = build(args.game_dir, out, tag=args.tag, label=args.label, new_row=args.new_row,
                    new_head_row=args.new_head_row, body_a=args.body_a, body_b=args.body_b, head=args.head,
                    soundset=args.soundset, gender=args.gender, name=args.name, utc=args.utc,
                    texture_var=args.texture_var, template_row=args.template_row, module=args.module,
                    template_utc=args.template_utc, expect_appearance=args.expect_appearance,
                    chain_from=args.chain_from, conversation=args.conversation, soundset_table=args.soundset_table)
    except (BuildError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(res, indent=2, sort_keys=True))
    else:
        print(f"{res['label']}: appearance row {res['appearance_row']}, heads row {res['heads_row']}, "
              f"utc {res['utc']}, out {out}")
        for n, h in sorted(res["sha256"].items()):
            print(f"  {n}: {h}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
