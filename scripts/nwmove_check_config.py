#!/usr/bin/env python3
"""Offline check of a hand-edited patch_config.toml + kpm_install_state.json pair for the npc-world move.

  python3 scripts/nwmove_check_config.py --config <patch_config.toml> --state <kpm_install_state.json> \
      --npcworld-hooks patch_manager_mods/npc_world_src/kotor2-steam-aspyr.hooks.toml [--exports <exports.def>]
  python3 scripts/nwmove_check_config.py --selftest

The three world detours (CSWSObject AddToArea, LoadModuleInProgress, MainLoop) moved out of k1-spawn-probe into
npc-world. Checks (each failure is one line `ERROR <file>:<line>: <message>`):
  1. block order: npc-world, then k1-spawn-probe, then npc-world-probe (all present)
  2. addresses 5671888 (0x568BD0), 5612640 (0x55A460), 5453872 (0x533830) each appear exactly once across all
     blocks, and that hook is inside npc-world
  3. npc-world hooks are exactly NW_OnAddToArea, NW_OnLoad, NW_OnTick; each is exported in exports.def; the
     (address, function) pairs equal the hooks toml; each original_bytes is 55 8B EC 6A FF
  4. k1-spawn-probe: dll patches/k1-spawn-probe.dll and zero hooks
  5. npc-world-probe: dll patches/npc-world-probe.dll and zero hooks; npc-world dll patches/npc-world.dll
  6. ids unique across blocks; 45 blocks
  State: InstalledPatches has 45 distinct ids, includes npc-world and npc-world-probe, and equals the config id set.

Line numbers come from scanning the raw text for the `id = "<id>"` or `address = <n>` line; 0 when unknown.
Read-only on its inputs. Exit 1 on any ERROR, else prints OK and exits 0.
"""
import argparse
import json
import re
import sys
import tomllib
from collections import Counter
from pathlib import Path

ORDER = ("npc-world", "k1-spawn-probe", "npc-world-probe")
DLL_FOR = {
    "npc-world": "patches/npc-world.dll",
    "k1-spawn-probe": "patches/k1-spawn-probe.dll",
    "npc-world-probe": "patches/npc-world-probe.dll",
}
MOVED_ADDRESSES = (5671888, 5612640, 5453872)  # 0x568BD0 AddToArea, 0x55A460 OnLoad, 0x533830 OnTick
NPC_WORLD_FUNCS = ("NW_OnAddToArea", "NW_OnLoad", "NW_OnTick")
STOLEN_BYTES = [85, 139, 236, 106, 255]  # 55 8B EC 6A FF = PUSH EBP / MOV EBP,ESP / PUSH -1
BLOCK_COUNT = 45
STATE_KEY = "InstalledPatches"

_ADDR_RE = re.compile(r"^\s*address\s*=\s*(0[xX][0-9a-fA-F]+|[0-9]+)\s*(#.*)?$")


def _err(errs, name, line, msg):
    errs.append(f"ERROR {name}:{line}: {msg}")


def _first_line(lines):
    return lines[0] if lines else 0


def _parse_int(text):
    try:
        return int(text, 0)
    except ValueError:
        return None


def _id_lines(text, ident):
    """1-based lines of `id = "<ident>"` in the config text."""
    if text is None:
        return []
    rx = re.compile(r'^\s*id\s*=\s*"' + re.escape(ident) + r'"\s*(#.*)?$')
    return [n for n, line in enumerate(text.splitlines(), 1) if rx.match(line)]


def _address_lines(text, address):
    """1-based lines of `address = <n>` (decimal or 0x hex) whose value equals address."""
    out = []
    for n, line in enumerate((text or "").splitlines(), 1):
        m = _ADDR_RE.match(line)
        if m:
            value = _parse_int(m.group(1))
            if value is not None and value == address:
                out.append(n)
    return out


def _json_line(text, value):
    """1-based line of the first JSON string equal to value, 0 when unknown."""
    if text is None:
        return 0
    needle = json.dumps(value)
    for n, line in enumerate(text.splitlines(), 1):
        if needle in line:
            return n
    return 0


def _toml_error_line(exc):
    m = re.search(r"at line (\d+)", str(exc))
    return int(m.group(1)) if m else 0


def _hook_pairs(hooks_text, name, errs):
    """(address, function) of each [[hooks]] entry in the npc-world hooks toml; None if it does not parse."""
    try:
        data = tomllib.loads(hooks_text)
    except tomllib.TOMLDecodeError as exc:
        _err(errs, name, _toml_error_line(exc), f"TOML parse error: {exc}")
        return None
    hooks = data.get("hooks")
    if not isinstance(hooks, list):
        _err(errs, name, 0, "no [[hooks]] entries")
        return None
    return [(h.get("address"), h.get("function")) for h in hooks if isinstance(h, dict)]


def _export_names(exports_text):
    """Names in the EXPORTS section of a .def file. ';' starts a comment; 'Name @1' and 'Alias = Target' keep the name."""
    names, in_section = set(), False
    for raw in (exports_text or "").splitlines():
        line = raw.split(";", 1)[0].strip()
        if not line:
            continue
        if not in_section:
            in_section = line.upper() == "EXPORTS"
            continue
        names.add(re.split(r"[\s@=]", line, maxsplit=1)[0])
    return names


def _config_blocks(cfg, text, name, errs):
    """[(id, dll, hooks)] for each [[patches]] block; None when there is no patches array."""
    patches = cfg.get("patches")
    if not isinstance(patches, list):
        _err(errs, name, 0, "no [[patches]] array")
        return None
    blocks = []
    for blk in patches:
        if not isinstance(blk, dict):
            _err(errs, name, 0, "a patches entry is not a [[patches]] table")
            blocks.append((None, None, []))
            continue
        ident = blk.get("id")
        if not isinstance(ident, str):
            _err(errs, name, 0, f"patch block has no string id (got {ident!r})")
            ident = None
        hooks = blk.get("hooks", [])
        if not isinstance(hooks, list):
            _err(errs, name, 0, f"block '{ident}' hooks is not an array of tables")
            hooks = []
        blocks.append((ident, blk.get("dll"), hooks))
    return blocks


def _check_config(cfg, text, name, ref_pairs, exports, errs, hooks_name, exports_name):
    """Checks 1-6 on the parsed config. Returns the block list, or None when there is no patches array."""
    blocks = _config_blocks(cfg, text, name, errs)
    if blocks is None:
        return None
    ids = [b[0] for b in blocks]
    first = {}
    for pos, ident in enumerate(ids):
        if ident is not None:
            first.setdefault(ident, pos)

    # 6: ids unique, 45 blocks
    for ident, n in Counter(i for i in ids if i is not None).items():
        if n > 1:
            lines = _id_lines(text, ident)
            _err(errs, name, lines[1] if len(lines) > 1 else 0, f"id '{ident}' appears {n} times, ids must be unique")
    if len(blocks) != BLOCK_COUNT:
        _err(errs, name, 0, f"patch block count is {len(blocks)}, expected {BLOCK_COUNT}")

    # 1: block order
    for ident in ORDER:
        if ident not in first:
            _err(errs, name, 0, f"missing block '{ident}'")
    for a, b in zip(ORDER, ORDER[1:]):
        if a in first and b in first and first[a] > first[b]:
            _err(errs, name, _first_line(_id_lines(text, a)),
                 f"block '{a}' must come before '{b}' (order: {' -> '.join(ORDER)})")

    # 2: each moved address exactly once, inside npc-world
    sites = [(b[0], h) for b in blocks for h in b[2] if isinstance(h, dict)]
    for addr in MOVED_ADDRESSES:
        owners = [bid for bid, h in sites if h.get("address") == addr]
        lines = _address_lines(text, addr)
        if len(owners) != 1:
            _err(errs, name, lines[1] if len(lines) > 1 else 0,
                 f"address {addr} (0x{addr:08X}) appears {len(owners)} times across all blocks, "
                 "expected exactly once, in 'npc-world'")
        elif owners[0] != "npc-world":
            _err(errs, name, _first_line(lines),
                 f"address {addr} (0x{addr:08X}) is in block '{owners[0]}', must be in 'npc-world'")

    # 3: npc-world hook functions, exports, (address, function) pairs, original bytes
    if "npc-world" in first:
        _, _, nhooks = blocks[first["npc-world"]]
        nline = _first_line(_id_lines(text, "npc-world"))
        hooks = [h for h in nhooks if isinstance(h, dict)]
        names = sorted(map(str, (h.get("function") for h in hooks)))
        if names != sorted(NPC_WORLD_FUNCS):
            _err(errs, name, nline,
                 f"npc-world hook functions are {names}, must be exactly {', '.join(NPC_WORLD_FUNCS)}")
        for func in NPC_WORLD_FUNCS:
            if func not in exports:
                _err(errs, exports_name, 0, f"{func} is not exported (npc-world needs it)")
        cfg_pairs = [(h.get("address"), h.get("function")) for h in hooks]
        if ref_pairs is not None:
            for addr, func in cfg_pairs:
                if (addr, func) not in ref_pairs:
                    _err(errs, name, _first_line(_address_lines(text, addr)),
                         f"npc-world hook {func} @ {addr} does not match {hooks_name}")
            for addr, func in sorted(ref_pairs, key=repr):
                if (addr, func) not in cfg_pairs:
                    _err(errs, name, nline, f"npc-world block lacks {func} @ {addr} from {hooks_name}")
        for h in hooks:
            if h.get("original_bytes") != STOLEN_BYTES:
                _err(errs, name, _first_line(_address_lines(text, h.get("address"))),
                     f"npc-world hook {h.get('function')} original_bytes must be {STOLEN_BYTES}, "
                     f"got {h.get('original_bytes')}")

    # 4 and 5: dll per block; k1-spawn-probe and npc-world-probe carry no hooks
    for ident in ORDER:
        if ident in first:
            _, dll, _ = blocks[first[ident]]
            if dll != DLL_FOR[ident]:
                _err(errs, name, _first_line(_id_lines(text, ident)),
                     f"block '{ident}' dll must be '{DLL_FOR[ident]}', got {dll!r}")
    for ident in ("k1-spawn-probe", "npc-world-probe"):
        if ident in first:
            _, _, hooks = blocks[first[ident]]
            for h in hooks:
                if isinstance(h, dict):
                    _err(errs, name, _first_line(_address_lines(text, h.get("address"))),
                         f"block '{ident}' must have no hooks, found {h.get('function')} @ {h.get('address')}")
    return blocks


def _check_state(state_obj, state_text, name, cfg_ids, errs):
    """State checks. cfg_ids is the config id set, or None when the config did not parse."""
    if not isinstance(state_obj, dict) or STATE_KEY not in state_obj:
        _err(errs, name, 0, f"missing '{STATE_KEY}' list")
        return
    installed = state_obj[STATE_KEY]
    if not isinstance(installed, list) or not all(isinstance(x, str) for x in installed):
        _err(errs, name, 0, f"'{STATE_KEY}' must be a list of id strings")
        return
    for ident, n in Counter(installed).items():
        if n > 1:
            _err(errs, name, _json_line(state_text, ident), f"'{ident}' listed {n} times in {STATE_KEY}")
    installed_set = set(installed)
    if len(installed_set) != BLOCK_COUNT:
        _err(errs, name, 0, f"{STATE_KEY} has {len(installed_set)} distinct ids, expected {BLOCK_COUNT}")
    for ident in ("npc-world", "npc-world-probe"):
        if ident not in installed_set:
            _err(errs, name, 0, f"{STATE_KEY} is missing '{ident}'")
    if cfg_ids is not None:
        for ident in sorted(cfg_ids - installed_set):
            _err(errs, name, 0, f"config id '{ident}' is missing from {STATE_KEY}")
        for ident in sorted(installed_set - cfg_ids):
            _err(errs, name, _json_line(state_text, ident), f"{STATE_KEY} id '{ident}' is not in the config")


def check(config_text, state_obj, hooks_text, exports_text, *, config_name="patch_config.toml",
          state_name="kpm_install_state.json", hooks_name="kotor2-steam-aspyr.hooks.toml",
          exports_name="exports.def", state_text=None) -> list[str]:
    """Every problem with the config/state pair as `ERROR <file>:<line>: <message>` strings. Empty list = OK."""
    errs = []
    ref_pairs = _hook_pairs(hooks_text, hooks_name, errs)
    exports = _export_names(exports_text)
    cfg_ids = None
    try:
        cfg = tomllib.loads(config_text)
    except tomllib.TOMLDecodeError as exc:
        _err(errs, config_name, _toml_error_line(exc), f"TOML parse error: {exc}")
    else:
        blocks = _check_config(cfg, config_text, config_name, ref_pairs, exports, errs, hooks_name, exports_name)
        if blocks is not None:
            cfg_ids = {b[0] for b in blocks if b[0] is not None}
    _check_state(state_obj, state_text, state_name, cfg_ids, errs)
    return errs


# ---- synthetic fixtures for --selftest and the unit tests -------------------------------------------------------

NW_HOOKS = [(5671888, "NW_OnAddToArea"), (5612640, "NW_OnLoad"), (5453872, "NW_OnTick")]
FILLER_IDS = [f"ctrl-{i:02d}" for i in range(42)]

_SYNTH_HOOKS = """\
# synthetic copy of the npc-world hook table (same addresses, functions and bytes as the real file)
[[hooks]]
address = 0x00568BD0
type = "detour"
function = "NW_OnAddToArea"
original_bytes = [0x55, 0x8B, 0xEC, 0x6A, 0xFF]

[[hooks]]
address = 0x0055A460
type = "detour"
function = "NW_OnLoad"
original_bytes = [0x55, 0x8B, 0xEC, 0x6A, 0xFF]

[[hooks]]
address = 0x00533830
type = "detour"
function = "NW_OnTick"
original_bytes = [0x55, 0x8B, 0xEC, 0x6A, 0xFF]
"""

_SYNTH_EXPORTS = "LIBRARY NpcWorld\nEXPORTS\n    KPatchInit\n    NW_OnLoad\n    NW_OnAddToArea\n    NW_OnTick\n"


def _hook_text(addr, func, ob=STOLEN_BYTES):
    return (f'[[patches.hooks]]\naddress = {addr}\noriginal_bytes = {json.dumps(ob)}\n'
            f'type = "detour"\nfunction = "{func}"\n'
            '[[patches.hooks.parameters]]\nsource = "ecx"\ntype = "pointer"\n')


def _block_text(ident, dll, hooks=()):
    """One [[patches]] block; each hook is (address, function) or (address, function, original_bytes)."""
    return f'[[patches]]\nid = "{ident}"\ndll = "{dll}"\n' + "".join(_hook_text(*h) for h in hooks)


def _synth_blocks():
    """45 (id, text) blocks in the right order: the three target blocks, then 42 dll-only filler blocks."""
    blocks = [
        ("npc-world", _block_text("npc-world", DLL_FOR["npc-world"], NW_HOOKS)),
        ("k1-spawn-probe", _block_text("k1-spawn-probe", DLL_FOR["k1-spawn-probe"])),
        ("npc-world-probe", _block_text("npc-world-probe", DLL_FOR["npc-world-probe"])),
    ]
    blocks += [(i, _block_text(i, f"patches/{i}.dll")) for i in FILLER_IDS]
    return blocks


def _render(blocks):
    sha = "00" * 32
    return f'target_version_sha = "{sha}"\n\n' + "".join(text for _, text in blocks)


def _state(blocks):
    return {STATE_KEY: [ident for ident, _ in blocks]}


def _with(blocks, ident, text):
    return [(i, text if i == ident else t) for i, t in blocks]


def _drop(blocks, ident):
    return [(i, t) for i, t in blocks if i != ident]


def synthetic_config():
    """(config_text, state_obj) for a valid 45-block pair."""
    blocks = _synth_blocks()
    return _render(blocks), _state(blocks)


def selftest_cases():
    """(name, config_text, state_obj, hooks_text, exports_text, expected); expected None means no errors."""
    base = _synth_blocks()

    def case(name, blocks, expect, *, state=None, cfg_text=None, exports=_SYNTH_EXPORTS):
        return (name,
                cfg_text if cfg_text is not None else _render(blocks),
                state if state is not None else _state(blocks),
                _SYNTH_HOOKS, exports, expect)

    swapped = [base[1], base[0]] + base[2:]
    return [
        case("pass: reference 45-block pair", base, None),
        case("wrong order: k1-spawn-probe before npc-world", swapped, "must come before"),
        case("k1 block still carries a moved hook",
             _with(base, "k1-spawn-probe",
                   _block_text("k1-spawn-probe", DLL_FOR["k1-spawn-probe"], [(5671888, "K1SpawnOnAddToArea")])),
             "must have no hooks"),
        case("duplicate hook address",
             _with(base, "ctrl-05", _block_text("ctrl-05", "patches/ctrl-05.dll", [(5612640, "NW_OnLoad")])),
             "appears 2 times"),
        case("state missing an id", base, "is missing from InstalledPatches",
             state={STATE_KEY: [i for i, _ in base if i != "ctrl-07"]}),
        case("block count 44", _drop(base, "ctrl-41"), "block count is 44", state=_state(_drop(base, "ctrl-41"))),
        case("npc-world hook set lacks NW_OnTick",
             _with(base, "npc-world", _block_text("npc-world", DLL_FOR["npc-world"], NW_HOOKS[:2])),
             "hook functions are"),
        case("npc-world original_bytes wrong",
             _with(base, "npc-world", _block_text("npc-world", DLL_FOR["npc-world"],
                                                  [NW_HOOKS[0], (5612640, "NW_OnLoad", [85, 139, 236, 106, 0]),
                                                   NW_HOOKS[2]])),
             "original_bytes must be"),
        case("npc-world function not exported", base, "NW_OnTick is not exported",
             exports=_SYNTH_EXPORTS.replace("    NW_OnTick\n", "")),
        case("npc-world-probe dll wrong",
             _with(base, "npc-world-probe", _block_text("npc-world-probe", "patches/npc-world-probe.v2.dll")),
             "dll must be"),
        case("hook address paired with wrong function",
             _with(base, "npc-world", _block_text("npc-world", DLL_FOR["npc-world"],
                                                  [(5671888, "NW_OnAddToArea"), (5453872, "NW_OnLoad"),
                                                   (5453872, "NW_OnTick")])),
             "does not match"),
        case("duplicate block id", base + [("ctrl-03", _block_text("ctrl-03", "patches/ctrl-03.dll"))],
             "appears 2 times", state=_state(base)),
        case("config is not valid TOML", base, "TOML parse error", cfg_text="[[patches]\nid = \n",
             state=_state(base)),
        case("state has an id the config lacks", base, "is not in the config",
             state={STATE_KEY: _state(base)[STATE_KEY] + ["ctrl-99"]}),
    ]


def selftest():
    cases = selftest_cases()
    passed = 0
    for name, cfg, state, hooks, exports, expect in cases:
        errs = check(cfg, state, hooks, exports)
        if expect is None:
            ok = not errs
        else:
            ok = bool(errs) and all(e.startswith("ERROR ") for e in errs) and any(expect in e for e in errs)
        if ok:
            passed += 1
        else:
            print(f"FAIL {name}: expected {expect!r}, got {errs}")
    total = len(cases)
    print(f"selftest {'PASS' if passed == total else 'FAIL'} {passed}/{total}")
    return 0 if passed == total else 1


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Offline check of the npc-world move in patch_config.toml "
                                             "and kpm_install_state.json.")
    ap.add_argument("--config", type=Path, help="patch_config.toml to check")
    ap.add_argument("--state", type=Path, help="kpm_install_state.json to check")
    ap.add_argument("--npcworld-hooks", type=Path, help="npc-world kotor2-steam-aspyr.hooks.toml (source of truth)")
    ap.add_argument("--exports", type=Path,
                    help="exports.def (default: exports.def beside the hooks file)")
    ap.add_argument("--selftest", action="store_true", help="run the built-in synthetic cases and exit")
    args = ap.parse_args(argv)
    if args.selftest:
        return selftest()
    required = [("--config", args.config), ("--state", args.state), ("--npcworld-hooks", args.npcworld_hooks)]
    missing = [flag for flag, val in required if val is None]
    if missing:
        ap.error("required: " + ", ".join(missing))

    exports_path = args.exports or args.npcworld_hooks.with_name("exports.def")
    texts, bad = {}, False
    for label, path in (("config", args.config), ("state", args.state),
                        ("hooks", args.npcworld_hooks), ("exports", exports_path)):
        try:
            texts[label] = path.read_text(encoding="utf-8-sig")
        except OSError as exc:
            print(f"ERROR {path}:0: cannot read: {exc.strerror}")
            bad = True
    if bad:
        return 1
    try:
        state_obj = json.loads(texts["state"])
    except json.JSONDecodeError as exc:
        print(f"ERROR {args.state}:{exc.lineno}: invalid JSON: {exc.msg}")
        return 1

    errs = check(texts["config"], state_obj, texts["hooks"], texts["exports"],
                 config_name=str(args.config), state_name=str(args.state),
                 hooks_name=str(args.npcworld_hooks), exports_name=str(exports_path),
                 state_text=texts["state"])
    for e in errs:
        print(e)
    if errs:
        return 1
    print("OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
