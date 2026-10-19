#!/usr/bin/env python3
"""Build the data files of one imported K1 character (body A + body B + head) from the live game. Read only.

Writes ONLY into --out (default <stage>/char_<tag>/override):
  appearance.2da     live table + one row (copy of --template-row, then race/modela/texa/modelb/texb/normalhead set,
                     identity columns cleared, blank model<L>/tex<L> pairs a..n filled; see fill_blank_letters)
  heads.2da          live table + one row (head = --head, every texture column empty); not written for a
                     single-model (modeltype F/L/S) template, which has no head
  <utc>.utc          creature template copied from a module UTC (probe default: c_ithorian003 in 204tel.mod) with
                     the new appearance row, soundset, gender, tag/resref and a literal first name
  char_build.json    row numbers, soundset, sha256 of every output and of the live base files
Every check runs before the first write. A refusal raises BuildError (CLI: 'error: ...' on stderr, exit 1).

--new-row must equal the live appearance.2da row count and --new-head-row the live heads.2da row count: the rows
are appended, so a live table that grew since the plan (another mod added rows) is refused, not overwritten.
The game dir is never written (an --out inside it is refused). Nothing here installs anything.

--template-row should be the character's vanilla K2 party row (4 Bastila, 6 Carth, 8 Juhani, 9 Zaalbar, 10 Mission):
it brings the right body class (female PFB* armour bodies, Mission's small PFB*S) and the physics columns (height,
headtrack, prefatckdist, body_bag, ...). A template of modeltype F/L/S (Zaalbar, row 9) is a single-model row: only
`race` (and modela/texa, as the vanilla row) name the new model; --body-b may be omitted (or equal --body-a) and
--head/--new-head-row are not needed. For a modeltype B row every model<L>/tex<L> pair for L = a..n ends non-empty.

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
# Columns that would name the template's vanilla character: always emptied on the new row. The party rows 4/6/8/9/10 have
# these empty today (verified), but vanilla Mira (row 456) has texaevil/texbevil/texlevil set, so every evil texture column
# is listed: a different template row must not leak its character.
CLEARED = ("texaevil", "texbevil", "texievil", "texlevil", "texnevil", "backuphead")
SINGLE_MODEL_TYPES = ("F", "L", "S")  # the engine reads the `race` cell only (decomp 0085c330), no body letters, no head
LETTERS = "abcdefghijklmn"
# Vanilla K2 has a female slave-outfit body (letter L = BodyVariation 12; the PC female rows 91..102 use it) and no male one.
FEMALE_SLAVE_BODY = ("PFBL", "PFBLA")  # MDL pfbl in chitin, texture PFBLA01 in swpc_tex_tp*.erf
RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
APPEARANCE_REL, HEADS_REL, SOUNDSET_REL = "override/appearance.2da", "override/heads.2da", "override/soundset.2da"


def _pykotor() -> SimpleNamespace:
    api = pb._pykotor()
    from pykotor.common.language import LocalizedString
    api.LocalizedString = LocalizedString
    from pykotor.common.misc import ResRef
    api.ResRef = ResRef
    return api


def _blank(v) -> bool:
    return not str(v).strip()


def fill_blank_letters(row: dict, *, body_b: str, female: bool) -> dict:
    """Pure. row: appearance column -> value of a modeltype B row. Returns {column: value} for the cells to set so that
    every model<L>/tex<L> pair, L = a..n, is non-empty (a blank Model<L> cell reaches the model loader as an empty name:
    invisible/fallback/crash is unverified, h9 sec 3). A pair is filled when its model<L> cell is blank:
      L (slave outfit) on a female character -> the vanilla female slave body PFBL / PFBLA;
      any other blank letter (and L on a male: vanilla K2 has no male slave body) -> the character's clothes body
      (body_b, texture body_b), the vanilla Kreia pattern (every letter one model).
    A pair whose model is set is left alone (a set model with a blank texture is left for check_no_blank_letters)."""
    fills = {}
    for letter in LETTERS:
        m, t = f"model{letter}", f"tex{letter}"
        if m not in row or t not in row or not _blank(row[m]):
            continue
        if letter == "l" and female:
            fills[m], fills[t] = FEMALE_SLAVE_BODY
        else:
            fills[m], fills[t] = body_b, body_b
    return fills


def check_no_blank_letters(row: dict) -> None:
    """Guard: a modeltype B row must have every model<L>/tex<L> cell, L = a..n, non-empty. Raises BuildError."""
    bad = [c for letter in LETTERS for c in (f"model{letter}", f"tex{letter}") if c not in row or _blank(row[c])]
    if bad:
        raise BuildError("appearance row has blank body cells " + ", ".join(bad) +
                         ": every model<L>/tex<L> for L = a..n must be set (pick a template row that fills them)")


def _check(args: dict) -> None:
    for key in ("body_a", "body_b", "head", "utc"):
        if args[key] is None and key in ("body_b", "head"):  # optional: single-model (F/L/S) template
            continue
        if not RESREF_RE.fullmatch(args[key]):
            raise BuildError(f"{key} must be 1-16 chars of [a-z0-9_], got {args[key]!r}")
    for key in ("new_row", "new_head_row", "template_row", "soundset", "texture_var"):
        v = args[key]
        if v is None and key == "new_head_row":
            continue
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


def build(game_dir, out_dir, *, tag: str, label: str, new_row: int, new_head_row: int | None, body_a: str,
          body_b: str | None, head: str | None, soundset: int, gender: int, name: str, utc: str | None = None, texture_var: int = 1,
          template_row: int = TEMPLATE_ROW, module: str = pb.PROBE_MODULE, template_utc: str = pb.PROBE_UTC,
          expect_appearance: int = pb.PROBE_EXPECT, chain_from=None, conversation: str | None = None,
          soundset_table=None) -> dict:
    """Write the character data files into out_dir. Returns the char_build.json dict. Writes nothing on refusal.

    chain_from: a previous package's override dir. Its appearance.2da/heads.2da replace the live tables as the base
    (so --new-row/--new-head-row count on from that package); install that package first. Soundset, module and
    the template UTC still come from the live game.
    body_b / head / new_head_row: required for a modeltype B template. A template of modeltype F/L/S is a single-model
    row (race = body_a, modela/texa = body_a, nothing else named): body_b may be None or equal body_a, head and
    new_head_row may be None (then heads.2da is neither read nor written) or are accepted and ignored for the row
    (the heads row is still appended if head is given).
    gender: 1 female / 0 male; picks the slave-outfit fallback body for a blank letter L (see fill_blank_letters).
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
        for rel in (APPEARANCE_REL, HEADS_REL) if head is not None else (APPEARANCE_REL,):
            if not (base / Path(rel).name).is_file():
                raise BuildError(f"--chain-from {base} lacks {Path(rel).name}")
    rel_app, rel_heads = (Path(APPEARANCE_REL).name, Path(HEADS_REL).name) if chain_from else (APPEARANCE_REL, HEADS_REL)
    app_raw, app = _read_table(api, base, rel_app)
    heads_raw = heads = None
    if head is not None:
        if new_head_row is None:
            raise BuildError("new_head_row is required when a head is given")
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
    if heads is not None and heads.get_height() != new_head_row:
        raise BuildError(f"live heads.2da has {heads.get_height()} rows but --new-head-row is {new_head_row}; re-plan")
    if template_row >= app.get_height():
        raise BuildError(f"template row {template_row} is outside 0..{app.get_height() - 1}")
    modeltype = app.get_cell(template_row, "modeltype").strip().upper()
    single = modeltype in SINGLE_MODEL_TYPES
    if single and body_b not in (None, body_a):
        raise BuildError(f"template row {template_row} is modeltype {modeltype} (one model, race cell only) but "
                         f"body_b {body_b!r} differs from body_a {body_a!r}")
    if not single and (body_b is None or head is None):
        raise BuildError(f"template row {template_row} is modeltype {modeltype!r}: body_b and head are required")
    if soundset >= sounds.get_height():
        raise BuildError(f"soundset {soundset} is outside soundset.2da 0..{sounds.get_height() - 1}")

    app.add_row(str(new_row), {c: app.get_cell(template_row, c) for c in app.get_headers() if c != "label"})
    # Vanilla shape: the row id (first column) stays the numeric index; the name goes in the `label` data column.
    # Single-model template (Zaalbar row 9, modeltype F): the engine reads the `race` cell only; the vanilla row also
    # names the model in modela/texa and leaves modelb..n blank, so do the same and keep the template's other cells.
    sets = [("label", label), ("race", body_a if single else body_b), ("modela", body_a), ("texa", body_a)]
    if not single:
        sets += [("modelb", body_b), ("texb", body_b)]
    if head is not None:
        sets.append(("normalhead", str(new_head_row)))
    for col, val in sets:
        app.set_cell(new_row, col, val)
    for col in CLEARED:
        app.set_cell(new_row, col, "")
    filled = {}
    if not single:
        row = {c: app.get_cell(new_row, c) for c in app.get_headers()}
        filled = fill_blank_letters(row, body_b=body_b, female=bool(gender))
        for col, val in filled.items():
            app.set_cell(new_row, col, val)
        check_no_blank_letters({c: app.get_cell(new_row, c) for c in app.get_headers()})
    grew = app.get_height() == new_row + 1
    if heads is not None:
        heads.add_row(str(new_head_row), {c: "" for c in heads.get_headers() if c != "head"})
        heads.set_cell(new_head_row, "head", head)
        grew = grew and heads.get_height() == new_head_row + 1
    if not grew:
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

    files = {"appearance.2da": api.bytes_2da(app), f"{utc}.utc": api.bytes_utc(u)}
    if heads is not None:
        files["heads.2da"] = api.bytes_2da(heads)
    base_sha = {"appearance.2da": pb.sha256_hex(app_raw)}
    if heads_raw is not None:
        base_sha["heads.2da"] = pb.sha256_hex(heads_raw)
    info = {"tag": tag, "label": label, "appearance_row": new_row, "heads_row": new_head_row if head is not None else None,
            "modeltype": modeltype, "filled_cells": filled, "utc": utc,
            "template_row": template_row, "body_a": body_a, "body_b": body_b, "head": head, "soundset": soundset,
            "gender": gender, "texture_var": texture_var, "name": name, "conversation": str(u.conversation),
            "sha256": {n: pb.sha256_hex(b) for n, b in files.items()},
            "chained_from": None if chain_from is None else str(base),
            "base_sha256": base_sha}
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
    ap.add_argument("--new-head-row", type=int, default=None,
                    help="must equal the live heads.2da row count (required with --head)")
    ap.add_argument("--body-a", required=True, metavar="RESREF", help="modela/texa (no armour body)")
    ap.add_argument("--body-b", default=None, metavar="RESREF",
                    help="modelb/texb and race (required unless the template row is single-model F/L/S)")
    ap.add_argument("--head", default=None, metavar="RESREF",
                    help="head model of the new heads.2da row (required unless the template row is single-model F/L/S)")
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
