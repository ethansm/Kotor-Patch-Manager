#!/usr/bin/env python3
"""Recipe and user-config loader/validator for the install-time K1 -> K2 character importer (doc 38 s26).

A recipe (recipes/<id>.toml) is committed per-NPC instructions: resrefs, texture list, template row, gender,
soundset, and optionally an [audio] table. It carries NO assets and NO 2DA row numbers (appearance/heads/soundset
row ids are assigned by the builder from the live K2 tables).

[models] body_b and head go together: a body-type B template (Bastila, Carth, Juhani, Mission) needs both, a
single-model template (modeltype F/L/S, Zaalbar row 9) takes neither, so a recipe with only body_a is a head-less
single-model character (no heads.2da row is assigned or built). Only one of the two is a recipe error. The recipe cannot
see the template's modeltype at parse time; k1_port_char_build.py refuses a B template without body_b/head at build time.

[audio] (optional) imports the character's own K1 bark audio: k1_ssf = the K1 SSF resref (e.g. p_carth), k2_ssf =
the K2 SSF resref the pack serves. Both required. The builder then appends a NEW soundset.2da row for the character
and the pack serves the SSF plus the K1 voice wavs. `character.soundset` stays required: it is the fallback K2 row
used when the recipe has no [audio] (and documents the stand-in voice); with [audio] it is ignored for the UTC and pack.

[dialog] (optional) rips the character's K1 conversation, voice-over and lips (k1_port_dialog.py): k1_dlg = the K1 DLG
resref (optional; default = the conversation of the K1 creature template, found at build time), k2_dlg = the K2 DLG
resref the pack serves (required; the UTC conversation and the pack's default_dlg become it, so `character.dlg` must be
empty or the same), max_voiced = the voice budget in distinct VO lines, 0..80 (default 30; each voiced line is a wav plus
a lip in the shared asset pool). The user config (k1_imports.example.toml -> your private copy) names the game dirs,
the output dir and the build order. [build] characters entries are recipe ids (recipes/<id>.toml, which always
wins) or `utc:<resref>` for any K1 creature template (the importer derives its recipe at build time, see
k1_port_scan.py); a plain id with no recipe file is tried as a K1 UTC resref of that name.

API: load_recipe(path) -> Recipe, load_recipes(dir, ids) -> list[Recipe], check_collisions(recipes), load_config(path)
-> Config, utc_entry_id(resref), RecipeError. Needs Python >= 3.11 (tomllib); use the repo .venv Python otherwise.
Run: python3 k1_port_recipe.py <recipe.toml|config.toml>   (validates and prints a one-line summary)
"""
from __future__ import annotations

import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

HERE = Path(__file__).resolve().parent
DEFAULT_RECIPES_DIR = HERE / "recipes"

RESREF_RE = re.compile(r"[a-z0-9_]{1,16}")
ID_RE = re.compile(r"[a-z0-9_]{1,31}")
UTC_PREFIX = "utc:"
CONVERTERS = ("pykotor", "mdlops")
TXI_MAX = 1024

CHARACTER_KEYS = ("id", "display_name", "tag", "utc", "dlg", "soundset", "template_row", "gender")
CHARACTER_REQUIRED = ("id", "display_name", "utc", "soundset", "template_row", "gender")
MODELS_KEYS = ("converter", "body_a", "body_b", "head", "keep_dangly")
MODEL_ENTRY_KEYS = ("k1", "k2")
TEXTURE_KEYS = ("k1", "k2", "force_opaque", "txi")
TOP_KEYS = ("character", "models", "textures", "audio", "dialog")
BUILD_KEYS = ("out", "characters", "recipes_dir", "texture_source", "first_appearance_row", "first_head_row", "first_soundset_row")
TEXTURE_SOURCES = ("vanilla", "cr")

ROW_HINT = "row ids are assigned by the builder from the live K2 tables"
AUDIO_KEYS = ("k1_ssf", "k2_ssf")
DIALOG_KEYS = ("k1_dlg", "k2_dlg", "max_voiced")
DIALOG_MAX_VOICED = 80
DIALOG_DEFAULT_VOICED = 30


class RecipeError(ValueError):
    """A recipe or config violates the schema (message names the file and key path)."""


@dataclass(frozen=True)
class ModelRef:
    k1: str
    k2: str


@dataclass(frozen=True)
class TextureRef:
    k1: str
    k2: str
    force_opaque: bool = False
    txi: str = ""


@dataclass(frozen=True)
class AudioRef:
    k1_ssf: str  # K1 SSF resref read from the K1 install
    k2_ssf: str  # K2 SSF resref the pack serves (the new soundset.2da row's resref column)


@dataclass(frozen=True)
class DialogRef:
    k1_dlg: str  # K1 DLG resref read from the K1 install; "" = the conversation of the K1 creature template
    k2_dlg: str  # K2 DLG resref the pack serves (also the UTC conversation and the pack's default_dlg)
    max_voiced: int = DIALOG_DEFAULT_VOICED  # voice budget: distinct VO lines (a wav + a lip each)


@dataclass(frozen=True)
class Recipe:
    id: str
    display_name: str
    tag: str
    utc: str
    dlg: str
    soundset: int
    template_row: int
    gender: int
    converter: str
    body_a: ModelRef
    body_b: ModelRef | None
    head: ModelRef | None
    textures: tuple[TextureRef, ...]
    warnings: tuple[str, ...]
    source: str  # path the recipe was loaded from (diagnostics only)
    audio: AudioRef | None = None
    dialog: DialogRef | None = None
    keep_dangly: bool = False  # [models] keep_dangly: pykotor keeps danglymesh nodes (hair/flap sway) instead of demoting them

    def k2_names(self) -> tuple[str, ...]:
        """Every k2 resref this recipe introduces (models, textures, utc, ssf), in declaration order."""
        names = [m.k2 for m in (self.body_a, self.body_b, self.head) if m is not None]
        names += [t.k2 for t in self.textures]
        names.append(self.utc)
        if self.audio is not None:
            names.append(self.audio.k2_ssf)
        if self.dialog is not None:
            names.append(self.dialog.k2_dlg)
        return tuple(names)


@dataclass(frozen=True)
class Config:
    k1_dir: Path
    k2_dir: Path
    out: Path
    characters: tuple[str, ...]
    recipes_dir: Path
    source: str
    utc_entries: dict = field(default_factory=dict)  # {recipe id: K1 UTC resref} for the `utc:<resref>` characters entries
    texture_source: str = "vanilla"          # [build] texture_source: "vanilla" (K1 install) or "cr" (Characters Revisited)
    first_appearance_row: int | None = None  # [build] first_appearance_row: start of the assigned appearance.2da rows
    first_head_row: int | None = None        # [build] first_head_row: start of the assigned heads.2da rows
    first_soundset_row: int | None = None    # [build] first_soundset_row: start of the assigned soundset.2da rows ([audio] recipes)


def _toml_module():
    try:
        import tomllib
    except ModuleNotFoundError:
        raise RecipeError("tomllib is missing (Python >= 3.11 needed); run with the repo .venv Python") from None
    return tomllib


def _read_toml(path: Path) -> dict:
    path = Path(path)
    tomllib = _toml_module()
    try:
        raw = path.read_bytes()
    except OSError as e:
        raise RecipeError(f"{path}: cannot read: {e}") from None
    try:
        return tomllib.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, tomllib.TOMLDecodeError) as e:
        raise RecipeError(f"{path}: invalid TOML: {e}") from None


class _Ctx:
    """File name + error helper so every message reads '<file>: <key.path>: <problem>'."""

    def __init__(self, path: Path):
        self.path = str(path)

    def err(self, key: str, msg: str):
        return RecipeError(f"{self.path}: {key}: {msg}")

    def table(self, value: Any, key: str) -> dict:
        if not isinstance(value, dict):
            raise self.err(key, "must be a table")
        return value

    def unknown(self, table: dict, allowed: tuple[str, ...], where: str) -> None:
        for k in table:
            if k in allowed:
                continue
            hint = ""
            kl = str(k).lower()
            if "row" in kl or kl in ("appearance", "heads", "appearance_id", "head_id"):
                hint = f" ({ROW_HINT})"
            elif kl == "texture":
                hint = " (use [[textures]])"
            path = f"{where}.{k}" if where else str(k)
            raise self.err(path, f"unknown key{hint}")

    def require(self, table: dict, keys: tuple[str, ...], where: str) -> None:
        for k in keys:
            if k not in table:
                raise self.err(f"{where}.{k}" if where else k, "missing required key")

    def string(self, value: Any, key: str) -> str:
        if not isinstance(value, str):
            raise self.err(key, f"must be a string, got {type(value).__name__}")
        return value

    def resref(self, value: Any, key: str, allow_empty: bool = False) -> str:
        s = self.string(value, key)
        if allow_empty and s == "":
            return s
        if not RESREF_RE.fullmatch(s):
            raise self.err(key, f"bad resref {s!r}: must match [a-z0-9_]{{1,16}} (lowercase)")
        return s

    def printable(self, value: Any, key: str, lo: int, hi: int) -> str:
        s = self.string(value, key)
        if not (lo <= len(s) <= hi) or not all(0x20 <= ord(c) <= 0x7E for c in s):
            raise self.err(key, f"must be {lo}..{hi} printable ASCII characters, got {s!r}")
        return s

    def integer(self, value: Any, key: str, lo: int = 0, hi: int | None = None) -> int:
        if isinstance(value, bool) or not isinstance(value, int):
            raise self.err(key, f"must be an integer, got {value!r}")
        if value < lo or (hi is not None and value > hi):
            rng = f">= {lo}" if hi is None else f"{lo}..{hi}"
            raise self.err(key, f"must be {rng}, got {value}")
        return value


def _model(ctx: _Ctx, value: Any, key: str) -> ModelRef:
    t = ctx.table(value, key)
    ctx.unknown(t, MODEL_ENTRY_KEYS, key)
    ctx.require(t, MODEL_ENTRY_KEYS, key)
    return ModelRef(k1=ctx.resref(t["k1"], f"{key}.k1"), k2=ctx.resref(t["k2"], f"{key}.k2"))


def parse_recipe(data: dict, path: Path) -> Recipe:
    """Validate an already-parsed recipe table. `path` supplies the file stem and error context."""
    path = Path(path)
    ctx = _Ctx(path)
    ctx.unknown(data, TOP_KEYS, "")
    warnings: list[str] = []
    audio: AudioRef | None = None
    if "audio" in data:
        at = ctx.table(data["audio"], "audio")
        ctx.unknown(at, AUDIO_KEYS, "audio")
        ctx.require(at, AUDIO_KEYS, "audio")
        audio = AudioRef(k1_ssf=ctx.resref(at["k1_ssf"], "audio.k1_ssf"), k2_ssf=ctx.resref(at["k2_ssf"], "audio.k2_ssf"))

    dialog: DialogRef | None = None
    if "dialog" in data:
        dt = ctx.table(data["dialog"], "dialog")
        ctx.unknown(dt, DIALOG_KEYS, "dialog")
        ctx.require(dt, ("k2_dlg",), "dialog")
        dialog = DialogRef(k1_dlg=ctx.resref(dt.get("k1_dlg", ""), "dialog.k1_dlg", allow_empty=True),
                           k2_dlg=ctx.resref(dt["k2_dlg"], "dialog.k2_dlg"),
                           max_voiced=ctx.integer(dt.get("max_voiced", DIALOG_DEFAULT_VOICED), "dialog.max_voiced", 0, DIALOG_MAX_VOICED))

    if "character" not in data:
        raise ctx.err("character", "missing required table")
    ch = ctx.table(data["character"], "character")
    ctx.unknown(ch, CHARACTER_KEYS, "character")
    ctx.require(ch, CHARACTER_REQUIRED, "character")

    rid = ctx.string(ch["id"], "character.id")
    if not ID_RE.fullmatch(rid):
        raise ctx.err("character.id", f"bad id {rid!r}: must match [a-z0-9_]{{1,31}}")
    if rid != path.stem:
        raise ctx.err("character.id", f"id {rid!r} must equal the file stem {path.stem!r}")
    display_name = ctx.printable(ch["display_name"], "character.display_name", 1, 63)
    tag = ctx.printable(ch.get("tag", ""), "character.tag", 0, 31)
    utc = ctx.resref(ch["utc"], "character.utc")
    dlg = ctx.resref(ch.get("dlg", ""), "character.dlg", allow_empty=True)
    if dialog is not None:
        if dlg not in ("", dialog.k2_dlg):
            raise ctx.err("character.dlg", f"{dlg!r} conflicts with [dialog] k2_dlg {dialog.k2_dlg!r}: leave character.dlg empty "
                          "(the UTC conversation is the [dialog] k2_dlg)")
        dlg = dialog.k2_dlg
    sound = ch["soundset"]
    if isinstance(sound, str):
        if sound == "auto":
            raise ctx.err("character.soundset", '"auto" is reserved: use an [audio] table (k1_ssf, k2_ssf) for a characters own barks')
        raise ctx.err("character.soundset", f"must be an integer row, got {sound!r}")
    soundset = ctx.integer(sound, "character.soundset", 0)
    template_row = ctx.integer(ch["template_row"], "character.template_row", 0)
    gender = ctx.integer(ch["gender"], "character.gender", 0, 1)

    if "models" not in data:
        raise ctx.err("models", "missing required table")
    md = ctx.table(data["models"], "models")
    ctx.unknown(md, MODELS_KEYS, "models")
    ctx.require(md, ("converter", "body_a"), "models")
    converter = ctx.string(md["converter"], "models.converter")
    if converter not in CONVERTERS:
        raise ctx.err("models.converter", f"must be one of {list(CONVERTERS)}, got {converter!r}")
    body_a = _model(ctx, md["body_a"], "models.body_a")
    body_b = _model(ctx, md["body_b"], "models.body_b") if "body_b" in md else None
    head = _model(ctx, md["head"], "models.head") if "head" in md else None
    if (body_b is None) != (head is None):  # a B-type character has both, a single-model (F/L/S) one neither
        have, miss = ("body_b", "head") if head is None else ("head", "body_b")
        raise ctx.err(f"models.{miss}", f"missing: models.{have} is set, but body_b and head go together (both for a body-type B template, "
                      f"neither for a single-model F/L/S template such as Zaalbar row 9)")
    keep_dangly = md.get("keep_dangly", False)
    if not isinstance(keep_dangly, bool):
        raise ctx.err("models.keep_dangly", f"must be true or false, got {keep_dangly!r}")
    if keep_dangly and converter != "pykotor":
        warnings.append(f"{rid}: models.keep_dangly only applies to converter \"pykotor\"; ignored for converter {converter!r}")
        keep_dangly = False

    textures: list[TextureRef] = []
    raw_tex = data.get("textures", [])
    if not isinstance(raw_tex, list):
        raise ctx.err("textures", "must be an array of tables ([[textures]])")
    for i, item in enumerate(raw_tex):
        key = f"textures[{i}]"
        t = ctx.table(item, key)
        ctx.unknown(t, TEXTURE_KEYS, key)
        ctx.require(t, ("k1", "k2"), key)
        fo = t.get("force_opaque", False)
        if not isinstance(fo, bool):
            raise ctx.err(f"{key}.force_opaque", f"must be true or false, got {fo!r}")
        txi = ctx.string(t.get("txi", ""), f"{key}.txi")
        if len(txi) > TXI_MAX or not all(c in "\r\n\t" or 0x20 <= ord(c) <= 0x7E for c in txi):
            raise ctx.err(f"{key}.txi", f"must be <= {TXI_MAX} printable ASCII characters (newlines allowed)")
        textures.append(
            TextureRef(
                k1=ctx.resref(t["k1"], f"{key}.k1"),
                k2=ctx.resref(t["k2"], f"{key}.k2"),
                force_opaque=fo,
                txi=txi,
            )
        )

    seen_k1: dict[str, int] = {}
    for i, t in enumerate(textures):
        if t.k1 in seen_k1:
            raise ctx.err(f"textures[{i}].k1", f"duplicate k1 texture {t.k1!r} (also textures[{seen_k1[t.k1]}])")
        seen_k1[t.k1] = i

    seen_k2: dict[str, str] = {}

    def claim(name: str, where: str) -> None:
        if name in seen_k2:
            raise ctx.err(where, f"duplicate k2 name {name!r} (also {seen_k2[name]})")
        seen_k2[name] = where

    claim(utc, "character.utc")
    claim(body_a.k2, "models.body_a.k2")
    if body_b is not None:
        claim(body_b.k2, "models.body_b.k2")
    if head is not None:
        claim(head.k2, "models.head.k2")
    for i, t in enumerate(textures):
        claim(t.k2, f"textures[{i}].k2")
    if audio is not None:
        claim(audio.k2_ssf, "audio.k2_ssf")
    if dialog is not None:
        claim(dialog.k2_dlg, "dialog.k2_dlg")

    return Recipe(
        id=rid,
        display_name=display_name,
        tag=tag,
        utc=utc,
        dlg=dlg,
        soundset=soundset,
        template_row=template_row,
        gender=gender,
        converter=converter,
        body_a=body_a,
        body_b=body_b,
        head=head,
        textures=tuple(textures),
        warnings=tuple(warnings),
        source=str(path),
        audio=audio,
        dialog=dialog,
        keep_dangly=keep_dangly,
    )


def load_recipe(path) -> Recipe:
    path = Path(path)
    return parse_recipe(_read_toml(path), path)


def load_recipes(directory, ids) -> list[Recipe]:
    """Load directory/<id>.toml for each id (order kept) and reject k2-name collisions across recipes."""
    directory = Path(directory)
    ids = list(ids)
    if not ids:
        raise RecipeError(f"{directory}: no recipe ids given")
    if len(set(ids)) != len(ids):
        dup = sorted({i for i in ids if ids.count(i) > 1})
        raise RecipeError(f"{directory}: duplicate recipe ids {dup}")
    recipes: list[Recipe] = []
    for rid in ids:
        if not isinstance(rid, str) or not ID_RE.fullmatch(rid):
            raise RecipeError(f"{directory}: bad recipe id {rid!r}: must match [a-z0-9_]{{1,31}}")
        path = directory / f"{rid}.toml"
        if not path.is_file():
            raise RecipeError(f"{path}: recipe not found")
        recipes.append(load_recipe(path))
    check_collisions(recipes)
    return recipes


def check_collisions(recipes) -> None:
    """Reject a k2 resref (model, texture, utc, ssf) introduced by two different recipes. Works on any mix of
    committed and auto-derived recipes (the message names the later recipe's source file)."""
    owner: dict[str, str] = {}
    for r in recipes:
        for name in r.k2_names():
            if name in owner and owner[name] != r.id:
                raise RecipeError(f"{r.source}: k2 name {name!r} collides with recipe {owner[name]!r}")
            owner[name] = r.id


def _dir_value(ctx: _Ctx, table: dict, key: str, where: str, required: bool = True) -> Path | None:
    full = f"{where}.{key}"
    if key not in table:
        if required:
            raise ctx.err(full, "missing required key")
        return None
    s = ctx.string(table[key], full)
    if s.strip() == "":
        if required:
            raise ctx.err(full, "must not be empty")
        return None
    return Path(s).expanduser()


def utc_entry_id(resref: str) -> str:
    """Recipe id of a `utc:<resref>` entry: the resref without a leading `p_` (p_carth -> carth). Same as k1_port_scan.auto_id."""
    r = resref
    return r[2:] if r.startswith("p_") and len(r) > 2 else r


def load_config(path) -> Config:
    path = Path(path)
    data = _read_toml(path)
    ctx = _Ctx(path)
    ctx.unknown(data, ("k1", "k2", "build"), "")
    for name in ("k1", "k2", "build"):
        if name not in data:
            raise ctx.err(name, "missing required table")
    k1 = ctx.table(data["k1"], "k1")
    k2 = ctx.table(data["k2"], "k2")
    build = ctx.table(data["build"], "build")
    ctx.unknown(k1, ("dir",), "k1")
    ctx.unknown(k2, ("dir",), "k2")
    ctx.unknown(build, BUILD_KEYS, "build")
    k1_dir = _dir_value(ctx, k1, "dir", "k1")
    k2_dir = _dir_value(ctx, k2, "dir", "k2")
    out = _dir_value(ctx, build, "out", "build")
    if "characters" not in build:
        raise ctx.err("build.characters", "missing required key")
    chars = build["characters"]
    if not isinstance(chars, list) or not chars:
        raise ctx.err("build.characters", "must be a non-empty list of recipe ids or utc:<resref> entries")
    utc_entries: dict[str, str] = {}
    ids: list[str] = []
    for i, c in enumerate(chars):
        key = f"build.characters[{i}]"
        if isinstance(c, str) and c.startswith(UTC_PREFIX):
            resref = c[len(UTC_PREFIX):]
            if not RESREF_RE.fullmatch(resref):
                raise ctx.err(key, f"bad K1 creature template in {c!r}: {resref!r} must match [a-z0-9_]{{1,16}} (lowercase), "
                              "for example \"utc:p_bastila\" (npcs --grep NAME lists them)")
            c = utc_entry_id(resref)
            utc_entries[c] = resref
        if not isinstance(c, str) or not ID_RE.fullmatch(c):
            raise ctx.err(key, f"bad recipe id {c!r}: must match [a-z0-9_]{{1,31}}")
        ids.append(c)
    chars = ids
    if len(set(chars)) != len(chars):
        dup = sorted({c for c in chars if chars.count(c) > 1})
        raise ctx.err("build.characters", f"ids must be unique, duplicated: {dup}")
    recipes_dir = _dir_value(ctx, build, "recipes_dir", "build", required=False) or DEFAULT_RECIPES_DIR
    texture_source = ctx.string(build.get("texture_source", "vanilla"), "build.texture_source")
    if texture_source not in TEXTURE_SOURCES:
        raise ctx.err("build.texture_source", f"must be one of {list(TEXTURE_SOURCES)}, got {texture_source!r}")
    first_app = ctx.integer(build["first_appearance_row"], "build.first_appearance_row", 0) \
        if "first_appearance_row" in build else None
    first_head = ctx.integer(build["first_head_row"], "build.first_head_row", 0) \
        if "first_head_row" in build else None
    first_sound = ctx.integer(build["first_soundset_row"], "build.first_soundset_row", 0) \
        if "first_soundset_row" in build else None
    return Config(
        k1_dir=k1_dir,
        k2_dir=k2_dir,
        out=out,
        characters=tuple(chars),
        recipes_dir=recipes_dir,
        source=str(path),
        utc_entries=utc_entries,
        texture_source=texture_source,
        first_appearance_row=first_app,
        first_head_row=first_head,
        first_soundset_row=first_sound,
    )


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print("usage: k1_port_recipe.py <recipe.toml|config.toml>", file=sys.stderr)
        return 2
    p = Path(argv[1])
    try:
        data = _read_toml(p)
        if "character" in data:
            r = parse_recipe(data, p)
            print(f"OK recipe {r.id}: converter={r.converter} keep_dangly={r.keep_dangly} textures={len(r.textures)} "
                  f"audio={r.audio.k1_ssf + '->' + r.audio.k2_ssf if r.audio else 'none'} "
                  f"dialog={(r.dialog.k1_dlg or 'utc') + '->' + r.dialog.k2_dlg + ' voiced<=' + str(r.dialog.max_voiced) if r.dialog else 'none'} "
                  f"warnings={list(r.warnings)}")
        else:
            c = load_config(p)
            files = [i for i in c.characters if (c.recipes_dir / f"{i}.toml").is_file()]
            if files:
                load_recipes(c.recipes_dir, files)
            auto = [i for i in c.characters if i not in files]
            print(f"OK config: characters={list(c.characters)} recipes_dir={c.recipes_dir} "
                  f"auto-derived (K1 UTC, checked at build)={auto}")
    except RecipeError as e:
        print(f"ERROR {e}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
