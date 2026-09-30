# KOTOR II New-Game Crash Investigation — working notes

Live investigation log. Written mid-session so nothing is lost across context
compaction. Update this file as the investigation progresses; don't rely on
conversation memory for any of this.

## Symptom timeline (chronological)

1. "crashes when I go to start a new game" — very first report.
2. Diagnosed & fixed: a prior agent ("Agy") had run `rm -rf Modules` based on a
   wrong premise (thought `modules` was a duplicate dir; it was actually a
   symlink `modules -> Modules`, so this deleted the ONLY copy of all 254
   `.mod` files). **Fixed**: restored `Modules/` from
   `~/Games/KOTOR2_Mod_Workspace/backups/full_modded_20260923/Modules`.
3. Still crashed ("got a little further but still crashed"). Root-caused via
   `logs/errors_pykotor.log` (in this project dir): a real pykotor bug —
   `pykotor/extract/capsule.py`'s `LazyCapsule.__init__` did
   `c_filepath = Path(path)`, which drops `CaseAwarePath`'s case-insensitive
   `__fspath__` resolution, so `c_filepath.stat()` failed with
   `FileNotFoundError` whenever a mod's `changes.ini` referenced a module
   capsule with different case than what's actually on disk. Affected 5
   capsules across the whole build (confirmed via grepping
   `logs/errors_pykotor.log` for every "No such file or directory" instance):
   `602DAN.mod`, `605DAN.mod`, `610DAN.mod`, `510OND.mod`, `020EBO.mod` (the
   last one — Ebon Hawk — twice). **Fixed**: patched `capsule.py` line ~58 to
   `c_filepath = Path(os.fspath(path))` (added `import os` at top of file).
   Verified fix with a direct `LazyCapsule` open test on all 5 — all now
   resolve correctly.
   - Traced the 5 broken capsules back to 7 source mods via
     `grep -rliE "602dan\.mod|605dan\.mod|610dan\.mod|510ond\.mod|020ebo\.mod" --include="*.ini"`
     across `~/Games/KOTOR2_Mod_Workspace/01_extracted`: mods #80
     (Skyboxes), #98 (PartySwap, `Default` option), #99 (Extended Enclave,
     `TSLRCM`/English option), #100 (PartySwap/EEnclave Compatch,
     `WithoutM478` option), #107 (Jedi Masters), #120 (For Mandalore,
     `SnigarooCut` option), #137 (Hologram Models, `Base_Install` option).
     Re-ran all 7 through the fixed pykotor — 0 errors, 534 total patches.
     **Caveat**: accidentally ran this recovery script TWICE (once live, once
     again to double check output) — this means mod #137's `AddRow`
     operations against `appearance.2da` ran twice, producing ~27 duplicate
     rows (see below, needs dedup).
4. STILL crashed — this time confirmed via the user watching the actual
   loading bar get "maybe 1/3" through before crashing. This ruled out a
   pre-3D-load issue and pointed at real module/resource loading.
5. Tried to get a real crash log:
   - Steam launch-options redirect (`... %command% > log 2>&1`) **does not
     work** — Steam's process chain (steam-launch-wrapper → reaper →
     pressure-vessel → proton → wine) re-execs into new process generations
     that don't inherit the shell's redirected stdout. Log just cuts off
     early (before the game even finishes starting).
   - Running wine directly from a terminal, bypassing Steam entirely, **also
     doesn't work** for this game — it exited in ~0.2s, before the game
     window even opened. Bypassing Steam loses required Steamworks/session
     env vars. Do not use this approach for this game.
   - **What worked**: Steam launch option `PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" taskset -c 0 mangohud %command%`.
     Proton itself writes `~/steam-208580.log` regardless of the sandboxing
     re-exec chain. This is the correct way to get real Wine-level crash
     output for this game.
   - **Current launch command (2026-09-27)**: `PULSE_LATENCY_MSEC=60
     PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm"
     MANGOHUD_CONFIGFILE=/home/ethansm/.config/MangoHud/wine-swkotor2.conf
     gamemoderun taskset -c 0,6 mangohud %command%`. `gamemoderun` is a no-op
     on this system — `libgamemode.so: cannot open shared object file` is
     printed to the log every launch — not the cause of any crash, just dead
     weight in the launch line; leave or drop it, doesn't matter functionally.
   - **Other log files worth checking alongside `steam-<appid>.log`** (all in
     the game install dir, `.../Knights of the Old Republic II/`):
     `AR_ERROR.LOG` (the Aurora engine's own error log — usually just resource
     name-mismatch warnings, but it's overwritten every session, and its mtime
     is a wall-clock anchor for the otherwise-monotonic wine log timestamps)
     and `ReShade.log` (if ReShade is installed — also wall-clock timestamped,
     useful for the same anchoring, and shows what ReShade injected/hooked
     that session).

## Real crash signature (confirmed, hard evidence)

From `~/steam-208580.log`: an **unhandled exception, code `c0000094`**
(`STATUS_INTEGER_DIVIDE_BY_ZERO`), unwound through 6 exception-handler frames
all inside `swkotor2.exe`'s own module address range (0x400000–0xab9000),
before propagating unhandled. This is a genuine divide-by-zero in the game's
own native code, not a driver/rendering issue. `winedbg` could not produce a
symbolic backtrace ("No backtrace available") — no debug symbols for this old
binary, so we can't get more precise than "somewhere in game logic."

### Ruled out (do not retry these)

- **"Fullscreen bug" / cutscene crash** (from KOTOR community FAQ
  https://kotor.neocities.org/faq/k2): diagnostic was to set
  `Disable Movies=1` + `Disable Intro Movies=1` in `swkotor2.ini`. Tested —
  **still crashed** at the same point (loading bar ~1/3). Per the FAQ's own
  words: "If you disable movies and you're still crashing, the fullscreen bug
  isn't the problem." Setting was reverted back to `0`/`0` (confirmed not the
  cause, no reason to keep cutscenes disabled).
- **`Frame Buffer`, `Soft Shadows`, `DisableVertexBufferObjects` ini
  settings**: another AI agent ("Agy") suggested changing these, citing the
  KOTOR FAQ. Verified the FAQ quotes against the live page via WebFetch — the
  quotes about disabling movies/windowed-mode were accurate, but the
  VBO/frame-buffer/soft-shadows advice was pulled from the FAQ's **framerate/lag**
  section, not the crash section, and presented misleadingly as if it were
  part of the same crash-troubleshooting hierarchy. These are rendering
  toggles, mechanistically unrelated to an integer divide-by-zero in game
  logic. **Do not apply these** — not touched, and shouldn't be.
- Agy also cited a fabricated setting name, `DisableVertexBufferShaders`,
  which does not exist anywhere in `swkotor2.ini` — a clear sign that agent's
  output needs verification before acting on it, which is why everything
  above was independently checked against the live FAQ page and the actual
  ini file rather than trusted at face value.
- **Duplicate `.tga`/`.tpc` collision** (a real, documented K2 crash cause):
  checked directly — 0 duplicate basenames in `override/`. Not the cause
  here (this had already been handled by the dedup pass earlier in the
  build).

## Root cause of the divide-by-zero: `appearance.2da` corruption (CONFIRMED, fix in progress)

`override/appearance.2da` (783 rows, 94 cols) has its **entire vanilla-range
content wiped**: rows 0–670 (671 rows total — matches vanilla's real row
count) are **100% blank across every column, including `label`** (confirmed:
0 out of 671 rows have a non-blank label). Only rows appended by mods (671+)
have real data.

This matters because Peragus (`101PER.mod`, the actual New-Game starting
module — confirmed via its `.mod` capsule's GIT resource) places creatures
whose `Appearance_Type` points straight into the now-blank range:
- `p_t3m4` (T3-M4) → row 2 → should be `Party_NPC_T3M3`, blank in override.
- `p_atton` (Atton) → row 452 → should be `Party_NPC_Atton`, blank.
- `g_assassindrd002/003/005` → row 460 → should be `Droid_Mining_Mark_One_A`, blank.
- `n_commf004` → row 522 → should be `PER_Holo_Miner_Main`, blank.

A blank `walkdist`/`rundist` (locomotion columns) on the very first creatures
the engine tries to place is a highly plausible source of a divide-by-zero
during module load.

### What's been ruled out as the mechanism

- **Not a uint16 offset-overflow bug** in the binary `2DA V2.b` writer/reader
  (`pykotor/resource/formats/twoda/io_twoda.py` uses `write_uint16`/
  `read_uint16` for cell offsets, capped at 65535 bytes). Measured real
  vanilla `appearance.2da`'s unique-value blob size directly: only ~19,225
  bytes. Nowhere near the limit. Also the corruption pattern (early rows
  blank, latest-added rows fine) is backwards from what an overflow would
  produce (later-written data would be the first to overflow, not survive).
- **Not a pykotor round-trip bug** in normal operation: tested extensively —
  a single `AddRow` + write + re-read cycle is clean; tested 6 consecutive
  generations of `AddRow`+write+read chained together, still 100% clean, no
  degradation. `ChangeRow`/`CopyRow`/`AddColumn` were not yet tested this way
  but `AddRow` (the most common operation) is confirmed innocent.
- **Predates this session entirely.** Confirmed by checking pykotor's own
  automatic pre-patch backups for mod #137 (Hologram Models) at THREE
  different timestamps: `2026-09-23_16.50.20` (the mod's *very first* run,
  during the original ~154-mod build, long before this session started),
  `2026-09-23_22.36.36` and `2026-09-23_22.37.25` (my two recovery re-runs
  today). **All three** already show rows 0/2/452/460/522 blank — including
  the earliest one. So mod #137 (and my re-running it) is NOT the cause —
  it's innocent; it just kept faithfully building on an already-corrupted
  table.
- Real vanilla `appearance.2da` was extracted directly from the game's own
  `chitin.key`/`2DA.bif` (via `pykotor.extract.installation.Installation`
  pointed at `~/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean`) and saved
  to `/tmp/vanilla_appearance.2da` (671 rows, 94 cols) — confirmed correct:
  row 2 = `Party_NPC_T3M3`, row 452 = `Party_NPC_Atton`, row 460 =
  `Droid_Mining_Mark_One_A`, row 522 = `PER_Holo_Miner_Main`. This file is a
  trustworthy clean baseline for the fix.

### Leading (unconfirmed) theory for the ORIGINAL cause

`pykotor/tslpatcher/patcher.py`'s `lookup_resource()`:
```python
if patch.replace_file or not exists_at_output_location:
    return self.load_resource_file(self.mod_path / patch.sourcefolder / patch.sourcefile)
```
When a `[2DAList]` patch's target file doesn't exist yet in `override/`
(true for whichever mod is FIRST in the whole 154-mod chain to touch
`appearance.2da`), pykotor falls back to loading **the mod's own bundled
reference copy** of `appearance.2da` from its `tslpatchdata/` folder. Real
TSLPatcher's actual documented behavior for 2DA patches is to fall back to
the **game's true vanilla copy** (from `chitin.key`/BIF) in this situation,
using the mod's bundled copy only as a label-matching aid for building the
diff, not as literal starting content. If some early mod in the chain ships
a "diff-style" reference 2DA that's deliberately sparse/blank outside the
rows it actually cares about, and pykotor treats it as literal starting
content, that would explain exactly this corruption pattern. **Not yet
proven** — haven't identified which specific mod was first to touch
`appearance.2da` chronologically (would require log archaeology beyond what
survived log rotation). This is a real latent pykotor bug worth fixing
properly (`lookup_resource` should special-case `Modifications2DA`/
`ModificationsGFF` to fall back to a real `Installation`-resolved vanilla
resource, not the mod's own bundled file), but doing that fix AND re-running
the whole 154-mod chain from scratch is a large undertaking — not the
immediate priority. The immediate priority is a targeted data fix (below).

## Fix APPLIED (status: deployed, awaiting in-game test)

The merge fix described below was built and deployed:
- `/tmp/vanilla_appearance.2da` (671 rows, real vanilla, headers verified
  identical to live file) merged with live `override/appearance.2da`'s rows
  671-782 (mod-added content, kept as-is, duplicates and all — see note
  below on why dedup was deferred).
- Verified merged table round-trips correctly (re-read after write: row 2 =
  `Party_NPC_T3M3`, row 452 = `Party_NPC_Atton`, row 460 =
  `Droid_Mining_Mark_One_A`, row 522 = `PER_Holo_Miner_Main`, all correct).
- Original corrupted file backed up to
  `~/Games/KOTOR2_Mod_Workspace/backups/appearance.2da.corrupted_backup_<timestamp>`
  before overwriting, in case comparison is needed later.
- Deployed to live `override/appearance.2da` (783 rows, 171935 bytes).
- **Deferred, not blocking**: rows 671-782 contain the DP_-hologram block
  (Admiral_Onasi through Holo_Revan, 27 rows) duplicated 3x (1 legitimate
  from the original build + 2 from accidentally running the mod #137
  recovery script twice today). This is cosmetic/wasteful, not
  crash-causing (`appearance.2da` has no hard row-count cap unlike
  `upgrade.2da`/`placeables.2da`), and was intentionally left as-is to avoid
  risk while the crash fix was the priority. Worth cleaning up later.
- **NEXT STEP: user needs to test launching the game / New Game again** to
  confirm this actually resolves the divide-by-zero crash. Not yet
  confirmed working as of this checkpoint.

## Fix plan (original plan, now applied above — kept for reference)

Since rows 0–670 in the live corrupted file are **100% blank with nothing
legitimate to lose** (confirmed count: 0/671 non-blank labels), the safe,
lossless fix is:

1. Take real vanilla `appearance.2da` (671 rows) from `/tmp/vanilla_appearance.2da`
   as the base for rows 0–670.
2. Take the current live `override/appearance.2da`'s rows 671+ (mod-added
   content: hologram rows from mod #137, etc.) and append them on top.
   **Must dedup first** — mod #137's `AddRow` ops were accidentally applied
   twice today (see step 3 above), so live height is 783 but the correct
   pre-duplicate count was 756 (confirmed via the 3 backup timestamps: 694 →
   729 → 756 → 783, where 756→783 was my own accidental double-run of the
   recovery script). Need to either de-duplicate by label (`DP_Bao-Dur_Holo`,
   `DP_Republic_Soldier_Helmet_M_C01`, `Unique_DP_Holo_Revan`, etc. — drop
   the second copy of each) or just truncate to height 756 if the tail 27
   rows are confirmed to be an exact repeat of the prior 27.
3. Write the merged table to `override/appearance.2da`.
4. Re-test: launch game (via Steam, with `PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm"`
   still in launch options), attempt New Game, see if it gets past the
   loading screen this time.
5. **Important followup**: this exact corruption pattern (blank vanilla-range
   rows, only latest-mod-added rows intact) could affect OTHER 2DA files
   that many mods touch across this whole build — not just
   `appearance.2da`. Worth systematically checking `heads.2da`,
   `portraits.2da`, `classes.2da`, `spells.2da`, `feat.2da`, `upgrade.2da`,
   and any other heavily-multi-mod-touched 2DA before declaring the game
   stable. Use the same method: extract real vanilla copy via
   `Installation('.../vanilla_clean').resource(name, ResourceType.TwoDA)`,
   compare row-by-row against the live `override/` copy for the vanilla-range
   rows.

## Key reusable commands/paths for this investigation

- Game root: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II`
- Vanilla backup (clean, unmodified): `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean`
- Full-modded backup (post-original-build, pre-this-session's-recovery):
  `/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/full_modded_20260923`
- Extracted mod sources: `/home/ethansm/Games/KOTOR2_Mod_Workspace/01_extracted/`
- pykotor venv: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/.venv/bin/python3`
- Patched files this session (both in the venv's installed pykotor, NOT
  upstreamed — redo if venv gets recreated):
  - `pykotor/extract/capsule.py`: `LazyCapsule.__init__`, `c_filepath = Path(os.fspath(path))` fix (added `import os`).
  - (3 prior patches from before this session, documented in
    `~/orca/workspaces/hardware-tools-docs/popos/scripts/kotor2/pykotor_patches/README.md`)
- Extracting a real vanilla 2DA for comparison:
  ```python
  import logging; logging.disable(logging.CRITICAL)
  from pykotor.extract.installation import Installation
  from pykotor.resource.type import ResourceType
  inst = Installation('/home/ethansm/Games/KOTOR2_Mod_Workspace/backups/vanilla_clean')
  res = inst.resource('appearance', ResourceType.TwoDA)  # takes ~30-90s, loads full Modules scan
  open('/tmp/vanilla_appearance.2da', 'wb').write(res.data)
  ```
- Getting a real Wine crash log through Steam (the only method that actually
  works — see above for what doesn't):
  Steam launch options: `PROTON_LOG=1 PROTON_LOG_DIR="/home/ethansm" taskset -c 0 mangohud %command%`
  → produces `~/steam-208580.log` after a crash.
- Checking a 2DA for blank vanilla-range rows:
  ```python
  import logging; logging.disable(logging.CRITICAL)
  from pykotor.resource.formats.twoda import read_2da
  twoda = read_2da('.../override/<name>.2da')
  # compare row.get_string('label') for rows 0..<vanilla_height-1> against known-good labels
  ```

## Update: appearance.2da fix alone did NOT stop the crash — real root cause found

After deploying the vanilla-merge fix to `appearance.2da`, user re-tested: **still
crashed**. A second AI agent ("Agy") independently investigated and reported a
specific mechanism. Cross-checked every claim directly against the actual bytes on
disk (not trusted blindly) — verdict: **Agy's file-level "12 files are wiped" list
was correct; Agy's proposed row-content details for `appearance.2da` were also
correct (rows 671-677 blank); but Agy's stated read-mechanism was subtly wrong** in
a way that mattered for how the fix was implemented. Here's what was actually
verified:

### The real bug, confirmed byte-for-byte

`pykotor`'s binary 2DA reader (`io_twoda.py`) tries a Kaitai-generated parser first,
falling back to a hand-written "legacy" parser only on exception
(`TwoDABinaryReader.load()`, `io_twoda.py:166-170`). The Kaitai parser
(`bioware_kaitai_formats/twoda.py:49-50`) reads a trailing `uint16` field
(`len_cell_values_section`) and takes it **literally** as the byte length of the
cell-string blob that follows, reading exactly that many bytes and discarding the
rest of the file.

Manually parsed the raw bytes of TSLRCM's own shipped
`01_extracted/tslrcm/override/appearance.2da` (150,492 bytes, untouched by any of
this session's tooling) to check this directly:
- Declared `len_cell_values_section` field: **`0`**
- Actual remaining bytes in the file after that field: **19,489** (real string data —
  confirmed by reading the first few hundred bytes: `Jedi_Council_Member_Male`,
  `P_FEM_C_SML_04`, etc.)
- Max cell offset referenced: `19,473` — nowhere near a uint16 overflow; the field
  is just a plain `0`, ignored by the real game engine and by pykotor's own legacy
  reader (which seeks by absolute offset and never even looks at this field's
  value), but taken at face value by Kaitai.

So: Kaitai reads `0` bytes into the string blob, then every single cell offset is
`>= len(raw) == 0`, so every cell in the table silently resolves to `""` — **with no
exception raised**, so the existing `except (KaitaiStructError, ValueError)`
fallback in `load()` never triggers. This is a real, confirmed, upstream pykotor
bug: `TwoDABinaryReader.load()` prefers a parser that silently produces a fully
wiped table (not a fully absent/error one) whenever it opens a shipped BioWare/TSLRCM
2DA that uses this `0`-placeholder convention for that trailing field.

**Where Agy's mechanism claim was wrong**: Agy asserted `_load_twoda_legacy` gives
*healthy* data on the corrupted files and only Kaitai gives blanks — tested this
directly on the actual corrupted `override/appearance.2da` backup
(`~/Games/KOTOR2_Mod_Workspace/backups/appearance.2da.corrupted_backup_20260923_231144`)
and both loaders returned **identical blanks** for that specific file. Reason: that
file is a *second-generation* artifact — some earlier mod install already read a
`0`-length-field source through the buggy Kaitai path, got blanks, and then wrote
those blanks back out with a *correct* (non-zero, matching) data-size field. By the
time that file was inspected, the corruption was already fully baked into the bytes
themselves, so of course both readers agreed. The live bug only shows up when
reading a file that *still has* the `0` placeholder trick — which TSLRCM's own
shipped copy does, and which is exactly what pykotor loads as the "existing
destination file" the first time any mod patches one of these tables after TSLRCM
is installed.

### Fix applied (pykotor patch, in the venv)

`pykotor/resource/formats/twoda/io_twoda.py`, in `_load_twoda_from_kaitai`: after
computing `raw_cells`, added a check — if any cell offset is `>= len(raw_cells)`,
raise `ValueError`. This forces `TwoDABinaryReader.load()`'s existing
`except (KaitaiStructError, ValueError): self._twoda = _load_twoda_legacy(...)` to
kick in and use the (verified-correct) legacy reader instead. Verified: loading
TSLRCM's shipped `appearance.2da` through the public `read_2da()` API now correctly
returns `Party_NPC_Kreia_Seppuku` at row 671, `Masked_Duros` at row 677, etc.,
instead of blanks.

### Files repaired on disk (all backed up before overwrite, all verified round-trip after)

Confirmed via direct scan (compare live `override/<name>.2da` row-by-row against a
correct baseline) that **12 files** were wiped in their vanilla/TSLRCM-range rows,
matching Agy's list exactly:
`appearance`, `classes`, `feat`, `featgain`, `globalcat`, `heads`, `modulesave`,
`placeables`, `portraits`, `spells`, `upgrade`, `visualeffects`.

Repair method per file:
- **9 files** (`upgrade`, `portraits`, `modulesave`, `placeables`, `feat`, `spells`,
  `heads`, `featgain`, `globalcat`): TSLRCM ships its own correct loose-file copy at
  `~/Games/KOTOR2_Mod_Workspace/01_extracted/tslrcm/override/<name>.2da` — used that
  (read through the now-fixed reader) as the baseline for rows `0..tslrcm_height-1`.
  Where `live_rows > tslrcm_rows` (later mods added rows), appended the live file's
  tail rows, **deduplicated by full row content** (first occurrence wins) to strip
  out accidental re-run duplication. `feat`/`featgain` had `live_rows == tslrcm_rows`
  exactly (no mod added rows) — replaced outright with TSLRCM's copy, no merge
  needed.
- **2 files** (`classes`, `visualeffects`): not shipped by TSLRCM; `live_rows`
  equaled vanilla row count exactly for both, so replaced outright with a fresh
  vanilla extraction (via `Installation`), no tail merge needed.
- **`appearance.2da`**: rebuilt as TSLRCM's 678-row baseline (rows 0-677, includes
  the previously-blank 671-677 Kreia/Handmaiden/Masked_Duros rows) + the live file's
  tail (678-782) deduplicated by content. This dropped the accidental triple-copy of
  mod #137's hologram-row block (105 tail rows scanned → 48 unique kept). **Known
  follow-up risk, not yet resolved**: mod #137's own UTC/GFF edits (which reference
  specific `appearance.2da` row *numbers* via `#2DAMEMORY#` tokens resolved at
  install time) may have been baked against whichever duplicate copy's row index was
  written *last* by the accidental double-run of `recovery_reinstall.py` earlier this
  session. If so, some hologram-model NPCs added by mod #137 could point at a
  now-nonexistent or wrong appearance row. This only affects mod #137's specific
  hologram content (not the Peragus/New-Game start), so deferred — clean fix would
  be to fully revert mod #137's output (2DA rows + its GFF/UTC edits together) and
  re-run its installer exactly once from a clean pre-mod-137 state.
- Backups of every pre-fix file: `~/Games/KOTOR2_Mod_Workspace/backups/<name>.2da.pre_fix_20260923_232331` (and one earlier `appearance.2da.pre_tslrcm_fix_20260923_232148` from the intermediate vanilla-only merge attempt).

### Verification swept the whole override/ directory

Re-scanned **every** `.2da` file in `override/` (not just the 12 flagged ones) for
the same >50%-blank-first-column pattern. One false positive
(`weapondischarge.2da`, 56% blank in its `droid` column) — checked directly against
vanilla and it's byte-identical, just legitimately sparse data. No other corrupted
files found.

## RESOLVED — confirmed fixed

User tested New Game after the full fix (reader patch + all 12 rebuilt 2DA
files): **game now loads past the crash point and into character creation.**
`swkotor2.exe` stayed up, `~/steam-208580.log` showed no unhandled exception.
Root cause and fix stand as documented above.

## Folded into the shareable docs/tooling repo

Everything generalizable from this investigation has been folded into
`~/orca/workspaces/hardware-tools-docs` (branch `kotor2-modding-docs`), ahead
of that content eventually moving to its own standalone public repo (see
`popos/docs/kotor2_new_repo_plan.md` in that repo for the migration plan):

- `popos/scripts/kotor2/pykotor_patches/README.md` — added patch #4 (capsule
  case-resolution) and patch #5 (the binary-2DA-reader Kaitai fallback fix,
  i.e. the actual crash root cause), matching the patches applied to this
  project's venv.
- `popos/scripts/kotor2/verify_2da.py` — new script, the blank-first-column
  corruption fingerprint check, generalized from this session's manual
  investigation. Tested against both a known-corrupted backup (catches it,
  rows 0-677) and this project's live fixed `override/` (clean).
- `popos/scripts/kotor2/pk_install.py` — now surfaces `installer.log.errors`
  on the command line (previously silent) and tracks successful installs in
  a manifest to prevent the exact double-run-a-mod mistake that tripled
  `appearance.2da`'s hologram rows this session.
- `popos/scripts/kotor2/nexus_download.py` — new script, Premium-API /
  free-tier-manual-fallback download helper (was previously fully manual for
  every Nexus mod).
- `popos/docs/kotor2_mod_build.md` — new "Critical bug: pykotor can silently
  wipe a 2DA table" section, new "Nexus Mods: Premium vs. free" section, new
  lessons-learned entries, and a corrected ReShade depth-buffer note (the
  manual `RESHADE_DEPTH_INPUT_*` tweak wasn't actually needed on the version
  used for this build — it auto-configured correctly).

Mod #137 hologram-row/UTC-reference risk noted above: **checked 2026-09-24,
clean** (see the next section).

## Capsule revert bug (pykotor #6) + replay — DONE (2026-09-24)

**Symptom:** mod edits inside `Modules/*.mod` were missing. Example: Onderon
News (#90) should set `501OND.mod/reporter.dlg` `EntryList[10].VO_ResRef` =
`501reporter099`, but live had `501reporter009`.

**Root cause:** pykotor 2.3.12 `Capsule.add()` re-added the old copy of a resource
after the patched one. Every edit to an existing capsule resource was reverted.
Fixed in the venv (patch #6); see `pykotor_patches/README.md`.

**Fix (applied):** rebuilt `Modules/` by replaying every pykotor run, in order,
into a scratch dir with the fixed pykotor
(`hardware-tools-docs/popos/scripts/kotor2/replay_capsule_edits.py`), then
swapped it in at 11:59. Details, commands and rollback are in
`~/Games/KOTOR2_Mod_Workspace/replay/README.md`; the write-up is in
`popos/docs/kotor2_mod_build.md` ("Repair: replay capsule edits").

Results:
- All runs replayed with 0 errors except seq 15 (Twi'lek Heads, below). 0
  token remaps; every TLK strref and final 2DA equals live.
- `check_capsule_edits.py`: 1265 capsule set-field edits. Before the swap,
  156 were present (12.3%) and 1048 missing. After, 1212 are present (95.8%),
  20 superseded, 33 skipped by design, **0 missing**. The #90 line reads
  `501reporter099`.
- The romance mod's PartySwap sections for `650jedi.dlg`, `krehand1.dlg` and
  `kredisc3.dlg` are skipped. The romance's EE readme installs Extended
  Enclave after it, so EE's dialogs win.
- **Echoes of the Republic** (installed 02:13-02:47, kept): its 7-module merge
  (`04_merge_modules.py`, pointed at scratch) was re-applied and its 128 modules
  copied in before the swap; all verified. Its docs' rollback
  (`rm -rf` the game + a `.tar` that doesn't exist) was corrected.
- **appearance.2da row 693** (duplicate `Handmaiden_Sister`) is kept as an
  unused placeholder; nothing references it.
- **Hologram/UTC check (was deferred above):** all 5132 UTCs in `Modules/` and
  `override/` point at existing, labelled appearance rows. None uses row 693,
  and all 51 #137 creatures point at hologram rows.
- **Lost ChangeRow edits:** the 23:21/23:23 2DA repairs rebuilt tables from
  TSLRCM and dropped 822 cells of mods' ChangeRow edits across 8 tables. They
  include K2CP's walk/run speeds (91 appearance rows), Alignment Force Powers'
  spell scripts, Crystal Attunement, the attack tables, and PFHC03's
  near-empty player-head rows 683-685. All restored (backup:
  `backups/override_2da_pre_a6_20260924/`); `verify_2da.py` clean.
- **Twi'lek Heads (#40)** had never been installed (wrong mod_path); installed
  properly at 12:07.
- pykotor's case lookup doesn't resolve directory components, so the game-dir
  case-alias symlinks (`Lips -> lips` etc.) are required.
- Extras installed afterwards: Dahnis Flirt, Visas' Dancer Outfit and Dance,
  Citadel Station Signage, Curvier Underwear (JC's Republic Soldier Fix is
  already in K2CP).

**Current state (2026-09-24 ~12:45):** live `Modules/` = the rebuilt one + Echoes
+ the extras' capsule edits; override 2DAs restored; `verify_2da.py`,
`dedup_tga_tpc.py --dry-run` and Agy's `verify_all_compatch.py` clean.
`replay/scratch/Modules` is stale (the extras patched live 303nar/306nar after
the swap), so never swap it in again. Spells row 285 `forcefriendly` was fixed
24 -> 34 (it duplicated row 260; mod120 meant `high()`). **Open:** the in-game
test (New Game to Peragus, load a later save). Hand-off doc:
`~/Games/KOTOR2_Mod_Workspace/replay/README.md` ("READ FIRST").

**Update (2026-09-24 ~13:30, PR #2 test review):** live `override/` had 79
pairs of one file name in two spellings (`TEL_SFtr.tpc` + `tel_sftr.tpc`),
74 of them with different content. Wine loads either spelling, so the game
could pick up a stale texture/model/dialog. They're a possible source of
visual or dialog oddities, not a known crash cause. Collapsed to the later
mod's file (`case_dedup.py`; decisions in
`backups/override_case_dedup_keep_20260924.txt`, originals in
`backups/override_case_variants_pre_dedup_20260924/`), and the installers now
prevent it. Also found: `kotor2_toolset.py dedup-textures --fix` would have
deleted both spellings of 9 `.tpc` textures; fixed before it was ever run.
`kotor2_toolset.py verify` passes on live. The Proton crash-log method from
this investigation is now written up for the community in
`hardware-tools-docs/popos/docs/kotor2_mod_build.md`, "Getting a crash log
under Proton". **Still open:** the in-game test.

## 2026-09-27: glu32 crash in Peragus dormitory (one-off, not reproduced — no fix applied)

**Symptom:** game crashed while playing (first crash since the divide-by-zero
fix above actually got the user into a real playthrough). Location: Peragus
dormitory, second room, the one with the Mk2 sentry droid (module `103PER` —
confirmed via `strings 103PER.mod | grep -i mk2`, hits on `Mk2Droid`,
`Mk2Droid1`, `Mk2Droid2`, `mk2ambush`). Immediately preceded by looting and
equipping a visor.

### Log correlation method (reusable)

Wine/Proton log timestamps (`2434027.212:...`) are **seconds since boot**, not
wall-clock. To line a crash up with "what was I doing at the time," cross-
reference the mtimes of `AR_ERROR.LOG` and `ReShade.log` in the game dir
(both wall-clock, both rewritten fresh every session) against the monotonic
offset of the exception in `steam-<appid>.log`. That's how this session's
crash was confirmed to be ~21:33-21:35 wall clock on 2026-09-27, matching the
user's report.

### Crash signature (confirmed, hard evidence)

Two related unhandled exceptions, ~0.5s apart, both inside `glu32.dll`
(module range `7a080000-7a0ab000` in this run — OpenGL Utility Library,
classically the `gluBuild2DMipmaps` runtime mip generator):

1. `0xc0000417 STATUS_INVALID_CRUNTIME_PARAMETER` — preceded by
   `err:msvcrt:_invalid_parameter (null):0 (null): (null) 0`, i.e. some CRT
   function got called with a bad argument. Backtrace:
   `glu32(+0x8ec0) / glu32(+0xb3b8) ← swkotor2(+0x32e3e → +0xb5e82 → ... →
   +0x3f3763 → ...)` — a long native call chain entirely inside the game's
   own module, bottoming out in `glu32`.
2. `0xc0000094 EXCEPTION_INT_DIVIDE_BY_ZERO` at `glu32+0xb56c` — same module,
   consistent with the mip-size computation dividing by a zero dimension
   right after the first fault left something in a bad state.
3. Both faults are `winedbg --auto`-caught (not a hard segfault), so the
   process didn't fully die on its own that time — see "the hang" below.

### What was checked and ruled out as the cause

Working theory going in: the ongoing ultrawide/world-texture upscale project
(see `[[kotor2-ultrawide-gui]]` memory) deployed a bad texture with a
degenerate mip level, and the visor (just looted/equipped, so its icon or
model would be freshly rendered) was the trigger. Checked directly, all
ruled out:

- **`i_bpercvisor.tpc`** (the looted visor's icon — found by grepping the mod
  workspace for `visor`, sourced from the "Ultimate Character Overhaul" Nexus
  mod, deployed via the Phase 7 world-texture rollout): loaded through
  pykotor (`read_tpc`), dumped its full mip chain (10 levels, 512×512 down to
  1×1) and diffed against the untouched source mod TPC — **byte-identical
  dimensions/sizes at every mip**, only the TXI mean-alpha header float
  differs (expected — that's `tone_fixed`/mean-alpha behavior, not damage).
- **Every deployed "world" texture that's uncompressed** (only an
  uncompressed RGB/RGBA texture can force the engine into `glu32`'s runtime
  mip-build path at all — a DXT-compressed TPC ships its mips pre-baked and
  never touches `glu32`): queried `out_world/world_all_flags.json`
  (`format` field) — only 8 of 996 deployed world textures are uncompressed,
  and all 8 are minigame HUD dashboard textures (`mg_dash*`, `mg_numbers`,
  `lmg_hud01`), nowhere near Peragus or headgear.
- **`per_*` (Peragus-prefixed) and `pmh*`/`pfh*` (player head) textures** in
  the same flags file: all DXT1/DXT5, only cosmetic quality flags (`clamp`,
  `grain-loss`, `alpha-lanczos`, `decal`, `blending` — documented, expected
  flags from the upscale pipeline, not corruption markers).

**Conclusion: not pinned on our own asset pipeline.** Every candidate texture
in the blast radius (icon, world set, head set) is intact and DXT-compressed,
which structurally can't reach the `glu32` mip-build code path that actually
faulted. No fix was applied. Remaining unconfirmed suspects, cheapest first:
ReShade's injected `opengl32.dll` proxy interfering with the real
`glu32`↔OpenGL path (visible as a duplicate `opengl32` module in the crash
dump's module list); a base-game/Aspyr `glu32` quirk in the inventory
paperdoll 3D-preview render path, independent of any mod; or an appearance-
changing model/texture for this specific visor that wasn't identified (no
`baseitems.2da` appearance link was traced for it this session). **Treat as
a one-off unless it recurs** — game has run fine on every relaunch since.

### The hang (separate from the crash, but caused by it)

On the next relaunch (loading the same save), the game came up to a black
screen and didn't respond. `ps aux` showed `swkotor2.exe` alive and pegged at
**128% CPU for 5+ minutes** (state `RNsl`, i.e. actually running/spinning,
not just stuck asleep) — consistent with the render thread having been left
in a bad state by the earlier `winedbg`-caught faults not actually killing
the process. Fix: `kill -TERM <swkotor2.exe pid>` (found via
`ps aux | grep -i swkotor`) was sufficient — exited cleanly within 3s, and
the entire Proton/wine process tree (`reaper`, `wineserver`, `winedevice`,
`xalia.exe`, `gameoverlayui`) tore itself down automatically once the actual
game process died. Killing `swkotor2.exe` directly (not `reaper`, not
`wineserver`, not the Steam client) is the right target — Steam's own
`waitforexitandrun` chain is what detects the exit and cleans up the rest.
No `SIGKILL` was needed this time; escalate to it only if `SIGTERM` doesn't
land within a few seconds.

### Reusable log-processing notes (huge-log triage)

The `steam-<appid>.log` from this session was 3MB / 27,670 lines; earlier
sessions have had `debug_pykotor.log` files well over 20MB. Full-file reads
are wasteful and blow context. Faster path, confirmed to work:

1. **Don't `WINEDEBUG=+all` or leave `+unwind` on for routine play.** `+unwind`
   trace-level logging (function-table unwind-info dumps, one block per stack
   frame per exception) is what makes these logs huge — it's useful *only*
   when you're actively trying to hand-decode a backtrace without symbols.
   For normal crash-watching, `+seh` (exception dispatch, no unwind detail)
   is enough signal at a fraction of the size. `+timestamp,+pid,+seh` is a
   reasonable "leave this on" set.
2. **Jump straight to the signal, don't scroll.** `grep -n -iE
   "unhandled exception|backtrace: --- Exception|EXCEPTION_|err:module|
   err:seh|Backtrace:"` finds every exception dispatch and the final
   `winedbg`-produced backtrace in one pass, regardless of file size. The
   crash is (almost) always near the end of the file, but grepping means you
   don't have to assume that.
3. **`err:` prefixed lines are real; `warn:`/`fixme:`/`trace:` mostly aren't.**
   Wine's own convention. Skim `err:` lines first.
4. **A `winedbg`-caught fault (`starting debugger...` in the log) does not
   mean the process died.** Check `ps aux` for the actual exe after a
   reported crash — as this session showed, it can keep running in a
   degraded/spinning state.
5. No good off-the-shelf log-viewer exists for this specific format (checked
   the WineHQ forums — closest hit was an old "log viewer for Wine/Proton
   error messages" thread with no maintained tool attached); for anything
   bigger than a quick `grep` pass, a purpose-built ingest into a trace
   viewer (e.g. Trace Compass, which has been used successfully on ~1GB
   `WINEDEBUG=+all` logs) is the documented path, but hasn't been needed for
   this project's log sizes yet.

Sources consulted: [How to Use PROTON_LOG to Diagnose Game
Crashes](https://eglug.org/using-protonlog-to-diagnose-why-a-proton-game-crashes-on-startup/),
[Log viewer for Wine/Proton error messages —
WineHQ Forums](https://forum.winehq.org/viewtopic.php?t=37470), [Debugging
Logs on the Steam Deck — Tux Explorer](https://tuxexplorer.com/debugging-logs-on-the-steam-deck).

This workflow (locate log → grep for signal → cross-reference mod-pipeline
outputs → check `ps aux` before assuming the process is dead) is now codified
as a project skill: `.claude/skills/kotor2-crash-triage/SKILL.md`.

## 2026-09-28: stuck-loading-screen hang after Phase 1b batch install (transient, not reproduced on retry — no fix applied)

**Symptom**: after installing 9 Patch Manager mods in one batch
(`k2-visible-console`, `good-alignment-heads`, `k2-aspyr-normal-map-fix`,
`k2-aspyr-saber-hilt-texture-fix`, `k2-finesse-melee-fix`,
`k2_grass_fog_fix`, `max-lights`, `semi-transparent-letterbox`,
`k2-mp3-playback-fix` — see `patch_manager_mods/05_build_plan.md`'s "Phase
1b" section) and playing normally into Peragus, the Peragus loading-screen
overlay got stuck on top of the in-game view — character visible standing
in the world underneath it, loading bar present but static, keyboard input
completely unresponsive. Screenshot evidence:
`~/Documents/Screenshot_2026-09-28_18-16-57.png`.

**Triage (per this skill)**: `ps aux` showed `swkotor2.exe` alive, state
`RNsl`/`SNsl`, 76-79% CPU — actively spinning, not asleep, matching the
skill's "hang, not crash" signature. `~/steam-208580.log` was actively
growing (2,182 new lines in 4 seconds) but the tail was a tight repeating
loop of `trace:seh:NtGetContextThread` and `fixme:ver:GetCurrentPackageFullName
(...): stub` on thread `020c` — no actual level-load progress, no
exception, no `err:` line. Consistent with a genuine hang (busy-spin in
some polling/version-check path), not legitimate disk-bound loading.

**Fix**: killed the correct PID directly (`kill -TERM` on the exact
`swkotor2.exe` process, found by matching the full Windows-path command
line, not just the string `swkotor2.exe` — a naive `pgrep`/`grep` on that
string over-matches the parent `reaper`/`proton`/`steam-launch-wrapper`
command lines, which all contain the exe path as an argument). `SIGTERM`
was sufficient; the whole Proton/wine tree (`reaper`, `wineserver`,
`winedevice`, `xalia.exe`, `steam.exe`) tore itself down automatically.
One `gameoverlayui -pid <dead-pid>` process was left pointing at the killed
PID — harmless, Steam reaps it on its own once it notices its target is
gone; not worth chasing.

**Relaunched** via `steam steam://run/208580` (not through
`KPatchLauncher` — patches were already installed, no reinstall needed).
Came up clean on retry: normal load, responsive input, ran around without
issue. **User noted PMEM (dedicated video/process memory per MangoHud) was
higher than before the patch batch (~600MB vs. previous ~400MB), still
well within headroom** — worth keeping an eye on across a longer play
session given `Max Lights` (raises the dynamic-light cap 3→7) is a
plausible, though unconfirmed, contributor; not treated as a problem yet.

**Not root-caused** — did not reproduce on the immediate retry, so no
specific patch was implicated or ruled out. If this recurs, next steps:
(a) narrow by uninstalling patches one at a time via `KPatchLauncher`
(remembering its uninstall-then-install-fresh behavior, per the
`kotor2-patch-manager` memory) rather than guessing which of the 9 is
responsible, (b) capture `WINEDEBUG=+seh,+module` around the hang instead
of the noisier default, since the `NtGetContextThread` spin didn't point
at a specific hooked function, (c) check whether the hang correlates with
any specific in-game trigger (area transition, specific NPC, HUD element)
rather than being time-based.

## 2026-09-28: recurrence, hang while loading a save (same signature, still not reproduced/root-caused)

**Symptom**: shortly after the live install of an 11th patch
(`k2-aspyr-map-aspect-fix`/`K2AspyrMapAspectFix`, see
`patch_manager_mods/06_gog_to_steam_conversions.md`), user reported the
game hung while loading a save.

**Triage**: `ps aux` showed `swkotor2.exe` (PID 2997136) alive 13m18s,
main thread at 94.5% CPU for 12m35s of that lifetime — pegged/spinning,
not asleep. `~/steam-208580.log` tail was the identical signature to the
Phase 1b incident above: tight repeating `trace:seh:NtGetContextThread` +
`fixme:ver:GetCurrentPackageFullName (...): stub` loop, this time on
thread `0208`, main thread `013c` separately busy on repeated
`nls:get_dummy_preferred_ui_language`/`RtlGetThreadPreferredUILanguages`
calls. No `err:` line, no `Unhandled exception`, no `Backtrace:` — a
`grep -c NtGetContextThread` over the whole log returned 18,510 hits. The
enabled trace channels (`seh`/`loaddll`/`threadname`/`ver`/`mscoree`/`nls`)
don't cover file I/O, so no direct visibility into save-load progress
either way.

**Read on cause**: this is very likely the *same pre-existing transient
hang* as the Phase 1b incident above, not something
`K2AspyrMapAspectFix` introduced — the identical spin signature
(`NtGetContextThread`/`GetCurrentPackageFullName` stub loop, no exception)
occurred earlier the same day, before `K2AspyrMapAspectFix` (or even
`K2AspyrLoadingScreenLineFix`) were ever installed. Not proof of
innocence — still unconfirmed root cause either time, and this is now 2/2
recurrences specifically **on a save load** (today's install) vs. Phase
1b's **area-transition loading screen** — worth noting the common thread
might be "loading-screen-adjacent" rather than any specific patch's hook
firing, but that's still a theory, not confirmed.

**Fix**: identical recovery to the Phase 1b incident — `kill -TERM` on
the exact `swkotor2.exe` PID (matched by full path, not a bare string
match), full Proton/wine tree torn itself down automatically (one
`[gameoverlayui] <defunct>` zombie left behind, harmless), relaunched via
`steam steam://run/208580` (patches already installed, no reinstall
needed). Awaiting user's retry to confirm whether the same save now loads
cleanly.

**Still not root-caused.** If this recurs a third time, especially again
correlated with a loading screen specifically, escalate to the Phase 1b
section's unfinished next steps: bisect by uninstalling patches
one-at-a-time via `KPatchLauncher`, or capture `WINEDEBUG=+seh,+module`
around a reproduction instead of the default trace set.

**Update — recurred a 2nd time, then succeeded on the 3rd attempt.** Retry
#2 (same save, same relaunch method) reproduced the identical signature
(`NtGetContextThread`/`GetCurrentPackageFullName` spin on thread `020c`,
no exception) but **much faster** — hung by 1m22s elapsed vs. 13m18s the
first time. Killed the same way (`kill -TERM`, clean exit, one harmless
`[gameoverlayui] <defunct>` left over). Relaunched a 3rd time via `steam
steam://run/208580` — **this attempt succeeded**: game loaded the same
save cleanly, ran at a healthy 45% CPU (not pegged), and the user
confirmed the actual fix works — the full-screen area map is no longer
horizontally stretched.

**Net read**: 2 hangs then a clean load, all on the identical save/patch
set, rules out "deterministic, every-time" but the faster 2nd
reproduction argues against pure chance too. Root cause still unconfirmed
— leaning toward some flaky Proton/Wine-layer startup race (the spin
signature never once pointed at any of our hook addresses or at
loading-screen-adjacent code specifically) rather than a bug in
`K2AspyrMapAspectFix`'s logic itself, given the *actual* visual fix landed
correctly once the game got past the load. Not treated as resolved — if
this pattern recurs on a normal (non-post-install) launch in a later
session, revisit with the Phase 1b bisection/`WINEDEBUG=+seh,+module`
escalation above.

## 2026-09-29: 3rd recurrence, same signature, during Buff Duration HUD item-3 empirical testing

**Symptom**: game froze during/after the user's in-game test of the
throwaway `BuffDurationHUDSpellIdProbe` diagnostic hook (cast Force
Speed/Force Valor, used some items, quit without saving), then again
after it was reinstalled as the normal 13-patch set (probe removed) for
the immediate follow-up relaunch. User reported "game is currently
froze, kill it".

**Triage**: `ps aux` found the real `swkotor2.exe` process (PID
3467969, `S<sl` at 91.7% CPU) among the usual Proton/reaper/
pressure-vessel wrapper chain — matched by full path, per
[[kotor2-crash-triage]]'s method, not a bare string match. `grep -c
NtGetContextThread ~/steam-208580.log` returned **13,713 hits**, and the
log's last ~1,200 lines were a tight repeating `trace:seh:
NtGetContextThread` / `fixme:ver:GetCurrentPackageFullName (...): stub`
loop on thread `0138:020c`, ending right at the point the process was
killed — the **identical signature** to both prior documented incidents
(2026-09-28 Phase 1b, and the 2026-09-28 save-load recurrence above). No
`err:`/`Unhandled exception`/`Backtrace:` line anywhere near the end.

**Fix**: `kill -TERM` on the exact PID — process exited cleanly within 3
seconds, no orphaned `swkotor2.exe` left behind. The 13-patch set was
already reinstalled (probe removed) before this hang, so no further
reinstall was needed once the user relaunches.

**Read on cause**: still unattributed to any specific patch — this is now
**3/3 recurrences** with the exact same spin signature, across three
different patch-set states (Phase 1b's 9-patch install, the 10/11-patch
GOG-port session, and today's 13-patch baseline both with and without
the throwaway probe). The probe hook itself is an extremely unlikely
cause (a single additive detour that only does a `fopen`/`fprintf`/
`fclose` per `ApplyEffect` call, and the hang recurred even after it was
already removed) — this continues to look like a pre-existing,
loading-screen/save-load-adjacent Proton/Wine timing issue independent
of Patch Manager, not something to keep re-suspecting each time a new
hook is tested. Not escalating to the Phase 1b bisection plan this
session — no new information to narrow it with, and it has never
reproduced on retry. If it starts happening on *every* launch rather
than intermittently, that would be the trigger to escalate.

## 2026-09-29 -- load-time hang, 2 launches in a row with `buff-duration-hud` installed (unattributed)

**Symptom**: after installing the Buff Duration HUD diagnostic build as a 14th patch, two consecutive launches hung during load (`swkotor2.exe` `RNsl`, 90-116% CPU, black/frozen; the known `NtGetContextThread` spin class from 2026-09-28). Killed with the PID-targeted `kill -TERM` (skill Step 4), relaunched; the third and every later launch (about 4 more) loaded and played normally.
**Evidence**: Proton log has no `Unhandled exception`/`err:` beyond the usual winebth/service lines; `AR_ERROR.LOG` unchanged; mod log `buffhud_log.txt` showed only `buffhud loaded` + `snapshot` lines (UpdatePortraits hook ran) and NO `row`/`CALL` lines -> `RetargetBuffIcon` had not run when the hang happened, so the hang is not in the render hook. Verbose file logging (fopen per call) in `DllMain` and hooks existed in those builds; not ruled out as a factor.
**Not done**: no baseline comparison (13 patches, no buffhud) of hang rate. Rate with the patch: 2 of ~6 launches. Escalation trigger: if it keeps happening at this rate, run 5+ launches on the 13-patch baseline and 5+ with the patch (logging off) before attributing.


## 2026-09-29 (later) -- another load-time hang with `buff-duration-hud` installed (unattributed)

**Symptom**: hang while loading a save after installing the redesign build (visible: partial 3D model render, a cloaked-looking ghost of the character animating over the loading screen; Esc hid it with the menu sound). `swkotor2.exe` `RNsl` ~72% CPU at 2:23 elapsed.
**Evidence**: no `Unhandled exception`/`err:` in `~/steam-208580.log`, `AR_ERROR.LOG` md5 unchanged; `buffhud_log.txt` had only `snapshot` lines and ZERO `COLUMN#` lines, i.e. `DrawBuffColumn` had never run. Saved as `buffhud_log.hang4.txt` (scratchpad).
**Action**: killed with `kill -TERM` (clean exit), relaunched via the launcher; next launch loaded fine. Tally with the patch installed: 3 hangs in ~10 launches; baseline comparison (13 patches, 5+ launches) still not done. Not attributed.


## 2026-09-29 20:25 -- hang #8, first fully captured (see patch_manager_mods/09_load_hang_investigation.md section 3b)

**Symptom**: transient freeze at load, `swkotor2.exe` `SNsl` 91% CPU at 50 s. **Captured** `~/hang_20260929_202500.log` before `kill -TERM` (clean exit, process tree gone). Buff HUD v1.0.0 build (logging off, so no mod log).
**Key evidence**: the `NtGetContextThread`/`GetCurrentPackageFullName` "spin" is the `wine_dinput_worker` thread handling raw mouse events (not a loader thread); the main thread's last output is a burst of 8 handled `IsBadReadPtr` access violations on garbage pointers, then 36 s of silence at ~50% CPU. Not attributed; next step = rerun with `buffhud_debug.txt` and compare healthy-launch `IsBadReadPtr` counts.
**Correction / follow-up (22:00)**: the installed set was **16** patches (the 14 + `force-power-hotbar` + `inventory-3d-viewport`, per `kpm_install_state.json`). The healthy 21:50 session with the same set has **0** `IsBadReadPtr ... caused` lines, so the burst is hang-specific. All 8 faults share one caller frame; the VEHs called are `force-power-hotbar.dll`, `additional-console-commands.dll` (both MinGW runtime handlers) and `KotorPatcher.dll`, all passive. Lead: stale cached creature pointer read after load (Buff HUD `g_slots`). Follow-up plan (engine bad-pointer map, `probeRead`, hang watchdog): `09_load_hang_investigation.md` sections 3b, 9, 10.
