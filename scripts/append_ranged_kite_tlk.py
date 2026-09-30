#!/usr/bin/env python3
"""Append the two "Ranged (Kiting)" strings of Mod 8 (ranged-ai, stance 16) to dialog.tlk.

Stance 16 needs a menu name (aiscripts.2da `name_strref`) and a tooltip/message string.
The vanilla ones are 123705 "Ranged" and 123710 "Ranged Behavior\\n...". This script adds

  name     Ranged (Kiting)
  message  Ranged (Kiting) Behavior / Keeps its distance, / finds a clear shot. / Melee if cornered.
           (4 lines, like vanilla 123710, so it fits the stance message box)

as two new entries at the end of the table, so they get strrefs N and N+1 (N = the old
entry count, 136538 on the modded build as of 2026-09-30). Never hard-code N: read
ranged_ai_strrefs.json (or build_ranged_kite_2da.py --strrefs-json).

Default is a DRY RUN: it reads the TLK and prints what it would append. --apply writes.

Idempotent. The whole table is searched for entries equal to each string:
  - both present as a consecutive (name, message) pair -> print their strrefs, (with
    --apply) rewrite the JSON, exit 0, append nothing;
  - only one present, or present but not adjacent      -> refuse, exit 2 (a half-applied
    or hand-edited table is not something to guess about).

Why pykotor's writer is safe here: round-tripping the live 136538-entry dialog.tlk through
read_tlk/write_tlk with no change gives a byte-identical file (md5 equal, 2026-09-30), so
existing entries cannot be re-encoded or reordered. The script still proves it on every
--apply: the string-data block of the old file must reappear byte-for-byte in the new one,
every old entry must compare equal, and the header language id must be unchanged.

--apply also copies the TLK to <backup-root>/backups_kitetlk_<YYYYmmdd_HHMM>/dialog.tlk first (md5
verified) and replaces the TLK atomically (temp file in the same dir + os.replace), after
verifying the temp file. It prints the one-line record for replay/README.md; it does not
edit the README. Close the game first (it reads dialog.tlk at start-up).

Usage:
  append_ranged_kite_tlk.py                      # dry run against the live dialog.tlk
  append_ranged_kite_tlk.py --apply              # live write (user-gated)
  append_ranged_kite_tlk.py --apply --tlk COPY --backup-root DIR --json DIR/x.json   # test
Environment: KOTOR2_ROOT, KOTOR2_WORKSPACE (see _common.py).
"""

import argparse
import datetime
import hashlib
import json
import logging
import os
from pathlib import Path
import random
import shutil
import struct
import subprocess
import sys

logging.disable(logging.CRITICAL)  # pykotor logs at import time; keep this script's output clean

from _common import GAME_DIR, WORKSPACE  # noqa: E402

try:
    from pykotor.resource.formats.tlk import read_tlk, write_tlk
except ImportError:
    print("Error: pykotor must be installed in python environment (use the repo .venv).", file=sys.stderr)
    sys.exit(1)

VANILLA_RANGED_MESSAGE = 123710  # "Ranged Behavior\n..." - the format our message copies
VANILLA_SPOT_CHECKS = (123705, 123710)  # the strings our new ones are modelled on
NAME = "Ranged (Kiting)"
MESSAGE_LINES = [
    "Ranged (Kiting) Behavior",
    "Keeps its distance,",
    "finds a clear shot.",
    "Melee if cornered.",
]
HEADER_SIZE, ENTRY_SIZE = 20, 40  # TLK V3.0: header, then one 40-byte record per entry
RANDOM_SAMPLE = 50


def md5(path) -> str:
    return hashlib.md5(Path(path).read_bytes()).hexdigest()


def raw_layout(path):
    """(language id, entry count, offset of the string-data block, raw bytes) of a TLK V3.0 file."""
    data = Path(path).read_bytes()
    if data[:8] != b"TLK V3.0":
        raise ValueError(f"{path}: not a TLK V3.0 file")
    lang, count, strings_at = struct.unpack_from("<III", data, 8)
    return lang, count, strings_at, data


def entry_key(e):
    """Everything a TLK entry carries that the game can see (text, sound resref, flags)."""
    return (e.text, str(e.voiceover), e.text_present, e.sound_present, e.soundlength_present)


def newline_style(tlk) -> str:
    """Line break used by vanilla message 123710 (\\n on this build); \\n if it is absent."""
    if VANILLA_RANGED_MESSAGE < len(tlk) and "\r\n" in tlk[VANILLA_RANGED_MESSAGE].text:
        return "\r\n"
    return "\n"


def find_pair(tlk, name, message):
    """Return ('pair', first_strref) | ('none', None) | ('partial', description)."""
    name_at = [i for i, e in enumerate(tlk.entries) if e.text == name]
    msg_at = [i for i, e in enumerate(tlk.entries) if e.text == message]
    if not name_at and not msg_at:
        return "none", None
    pairs = [i for i in name_at if i + 1 in msg_at]
    if pairs:
        if len(name_at) > 1 or len(msg_at) > 1:
            print(f"warning: duplicates exist (name at {name_at}, message at {msg_at}); using {pairs[0]}",
                  file=sys.stderr)
        return "pair", pairs[0]
    return "partial", f"name at {name_at or 'nowhere'}, message at {msg_at or 'nowhere'}"


def game_running() -> bool:
    try:
        return subprocess.run(["pgrep", "-f", "swkotor2"], capture_output=True).returncode == 0
    except OSError:
        return False


def verify_written(old_path, new_path, old_tlk, n_old, name, message):
    """Re-read `new_path` and prove it is `old_path` plus exactly the two new entries. Returns the new TLK."""
    problems = []
    old_lang, _, old_strings_at, old_raw = raw_layout(old_path)
    new_lang, new_count, new_strings_at, new_raw = raw_layout(new_path)
    new = read_tlk(new_path)

    if new_lang != old_lang:
        problems.append(f"language id {old_lang} -> {new_lang}")
    if len(new) != n_old + 2 or new_count != n_old + 2:
        problems.append(f"entry count {len(new)} / header {new_count}, expected {n_old + 2}")
    else:
        if new[n_old].text != name or new[n_old + 1].text != message:
            problems.append("new entries' text differs from the requested strings")
        for i in (n_old, n_old + 1):
            flags = struct.unpack_from("<I", new_raw, HEADER_SIZE + ENTRY_SIZE * i)[0]
            old_flags = struct.unpack_from("<I", old_raw, HEADER_SIZE + ENTRY_SIZE * VANILLA_RANGED_MESSAGE)[0]
            if flags != old_flags:
                problems.append(f"entry {i} flags {flags:#x}, vanilla {VANILLA_RANGED_MESSAGE} has {old_flags:#x}")

        # Sample named in the task: first, last old, the vanilla stance strings, 50 random.
        sample = {0, n_old - 1, *[i for i in VANILLA_SPOT_CHECKS if i < n_old]}
        sample |= set(random.Random(0).sample(range(n_old), min(RANDOM_SAMPLE, n_old)))
        for i in sorted(sample):
            if entry_key(old_tlk[i]) != entry_key(new[i]):
                problems.append(f"entry {i} changed")
        # All of them, too: it costs a second.
        changed = [i for i in range(n_old) if entry_key(old_tlk[i]) != entry_key(new[i])]
        if changed:
            problems.append(f"{len(changed)} old entries changed, first {changed[:5]}")

    # Byte level: the old string block must reappear unchanged at the start of the new one.
    old_block = old_raw[old_strings_at:]
    if new_raw[new_strings_at:new_strings_at + len(old_block)] != old_block:
        problems.append("old string-data block is not byte-identical at the start of the new one")
    if new_strings_at != old_strings_at + 2 * ENTRY_SIZE:
        problems.append("string block did not move by exactly two 40-byte records")

    if problems:
        raise RuntimeError("verification failed:\n  " + "\n  ".join(problems))
    return new


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--apply", action="store_true", help="write (default: dry run)")
    ap.add_argument("--tlk", type=Path, default=GAME_DIR / "dialog.tlk",
                    help="TLK to read/modify (default: the live dialog.tlk)")
    ap.add_argument("--backup-root", type=Path, default=WORKSPACE,
                    help="where backups_kitetlk_<YYYYmmdd_HHMM>/ goes (default: the mod workspace)")
    ap.add_argument("--json", type=Path, default=WORKSPACE / "ranged_ai_strrefs.json",
                    help="strref file to write (default: <workspace>/ranged_ai_strrefs.json)")
    args = ap.parse_args()

    tlk = read_tlk(args.tlk)
    n_old = len(tlk)
    nl = newline_style(tlk)
    message = nl.join(MESSAGE_LINES)
    try:
        message.encode("ascii")
    except UnicodeEncodeError:
        sys.exit("error: strings must be ASCII (pykotor's writer stores len(text) as the byte length)")

    print(f"TLK: {args.tlk}  ({n_old} entries, md5 {md5(args.tlk)})")
    print(f"mode: {'APPLY' if args.apply else 'dry run (nothing is written)'}")
    for ref in (123703, 123704, 123705, 123706, 126013, 123709, VANILLA_RANGED_MESSAGE, 123711):
        if ref < n_old:
            print(f"  vanilla {ref}: {tlk[ref].text!r}")
    print(f"  line break used: {nl!r}  (matches strref {VANILLA_RANGED_MESSAGE})")

    state, where = find_pair(tlk, NAME, message)
    if state == "partial":
        print(f"error: only part of the pair is in the table ({where}). Refusing to append.", file=sys.stderr)
        return 2

    previous_backup = None
    if state == "pair":
        name_ref, msg_ref = where, where + 1
        print(f"already present as a pair: name {name_ref}, message {msg_ref}; nothing to append")
        if args.json.is_file():
            previous_backup = json.loads(args.json.read_text()).get("backup")
        backup = previous_backup
    else:
        name_ref, msg_ref = n_old, n_old + 1
        backup = None
        print(("appending:" if args.apply else "would append:") + f"\n  {name_ref}: {NAME!r}\n  {msg_ref}: {message!r}")

    record = None
    if args.apply and state == "none":
        if game_running():
            print("warning: swkotor2 appears to be running; it reads dialog.tlk at start-up", file=sys.stderr)
        backup_dir = args.backup_root.expanduser() / f"backups_kitetlk_{datetime.datetime.now():%Y%m%d_%H%M}"
        backup_dir.mkdir(parents=True, exist_ok=True)
        backup_file = backup_dir / "dialog.tlk"
        if backup_file.exists():
            sys.exit(f"error: {backup_file} already exists (a second run in the same minute); not overwriting it")
        shutil.copy2(args.tlk, backup_file)
        if md5(backup_file) != md5(args.tlk):
            sys.exit(f"error: backup md5 mismatch for {backup_file}; TLK untouched")
        backup = str(backup_file)
        old_md5 = md5(args.tlk)
        print(f"backup: {backup} (md5 {old_md5}, verified)")

        old_tlk = read_tlk(args.tlk)  # untouched copy to compare against
        for text in (NAME, message):
            new_ref = tlk.add(text)
            entry = tlk[new_ref]
            # Give the new records the same flags as every vanilla entry (text + sound + sound
            # length present = 7), not pykotor's default for an entry with no voice-over.
            entry.sound_present = True
            entry.soundlength_present = True
        tmp = args.tlk.with_name(args.tlk.name + ".kite_tmp")
        try:
            write_tlk(tlk, tmp)
            verify_written(args.tlk, tmp, old_tlk, n_old, NAME, message)
            os.replace(tmp, args.tlk)
        finally:
            if tmp.exists():
                tmp.unlink()
        # Final re-read of the file in place; the old copy is the backup.
        verify_written(backup_file, args.tlk, old_tlk, n_old, NAME, message)
        new_md5 = md5(args.tlk)
        print(f"verified in place: {n_old} -> {n_old + 2} entries; all old entries equal, old string block "
              f"byte-identical, new flags match vanilla")
        record = (f"- dialog.tlk +2 entries (strrefs {name_ref} `{NAME}`, {msg_ref} Ranged (Kiting) message; "
                  f"Mod 8 stance 16), {datetime.datetime.now():%Y-%m-%d %H:%M}: "
                  f"{n_old} -> {n_old + 2} entries, md5 {old_md5[:8]} -> {new_md5[:8]}, "
                  f"`scripts/append_ranged_kite_tlk.py`. Rollback: "
                  f"`cp {backup} \"$G/dialog.tlk\"`.")

    entries_after = len(tlk) if args.apply and state == "none" else (n_old if state == "pair" else n_old + 2)
    strrefs = {"name": name_ref, "message": msg_ref, "tlk_entries_after": entries_after, "backup": backup}
    print(("wrote " if args.apply else "would write ") + f"{args.json}:\n" + json.dumps(strrefs, indent=2))
    if args.apply:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(strrefs, indent=2) + "\n")
    print(f"\nranged_ai.ini line: kite_strref_msg={msg_ref}")
    if record:
        print("\nLine for ~/Games/KOTOR2_Mod_Workspace/replay/README.md:\n" + record)
    elif not args.apply and state == "none":
        print("\nRe-run with --apply to write it (backup + verification included).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
