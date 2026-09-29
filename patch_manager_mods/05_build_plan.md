# Build Plan: Our Two Patch Manager Mods

Phased, each phase gated on the previous one succeeding and being
committed before the next starts — no overlapping a background build with
a coding phase, per [[feedback-phase-sequencing]]. Nothing here has been
started; this is the plan only.

## Phase 0 — Prove the toolchain (blocks everything else)

**Status: DONE, proven with one caveat, 2026-09-28.** Full details below;
see also [[kotor2-patch-manager]] memory for the summary.

Before writing a single hook for either mod, prove Patch Manager itself
works in *this* environment, the same way [[kotor2-script-compiler]]
required proving nwnnsscomp-under-Wine byte-for-byte before trusting it.

1. Install the .NET 8 SDK, MinGW-w64 i686 cross-compiler, Python 3, and
   32-bit dev libs (per `03_framework_research.md`).
2. Build the framework's own trivial example patch (e.g.
   `AdditionalConsoleCommands`) via `python3 create-patch.py`.
3. Install via `KPatchLauncher`, launch the game under our current Proton
   setup, confirm `KotorPatcher.dll` actually injects via the proxy-DLL
   path (`binkw32.dll`/`sqlite3.dll` staging) — check Proton/Wine logs per
   the [[kotor2-crash-triage]] skill's log-locating method if anything
   looks wrong.
4. Confirm the console command hook actually fires in-game.
5. Confirm no interaction with the already-applied 4GB LAA patch (mod #4)
   — launch, check the game doesn't fail to start or behave differently
   than before.
6. **Gate**: do not proceed to Phase 1 until this works cleanly. If the
   Proton proxy-DLL injection path doesn't work reliably, stop and
   re-evaluate — including the native Aspyr Linux build (`KotorPatcher.so`)
   as a fallback — before sinking time into either mod's C++.

### Outcome (2026-09-28)

**Toolchain, installed via apt (no Microsoft feed needed — both packages
are in Ubuntu 24.04's own repos):**
- `dotnet-sdk-8.0` → `.NET 8.0.131`
- `g++-mingw-w64-i686` (provides `i686-w64-mingw32-g++-posix`, GCC
  13-posix — the specific binary `build-mingw.sh` looks for first)
- The native-Linux 32-bit dev libs (`build-linux.sh`'s `glibc-devel.i686`
  equivalents) were **not** needed — that path is the fallback only, out
  of scope here.

**Blocker found and resolved before any install:** our live `swkotor2.exe`
already has its PE header modified by the 4GB Patcher (mod #4, LAA flag).
Patch Manager does a **hard, exact SHA-256 match** of the whole exe against
each patch's `supported_versions` (`GameVersionValidator.ValidateGameVersion`,
`src/KPatchCore/Validators/GameVersionValidator.cs:16` — no bypass in the
matching logic itself, though the CLI does expose an undocumented
`--bypass-hash` flag we did not use). That single header byte means our
live exe's hash matches **no** known version string — not just for our
future custom patches, for *any* patch, including the "safe" Phase 1
candidates. `backups/vanilla_clean/swkotor2.exe` (already in the project)
hashes to exactly the framework's known `kotor2_steam_aspyr` value
(`6a522e71...dbbffef`), confirming it's the same underlying build, just
without the LAA flag flipped. **This is unresolved for Phase 1**: installing
any real patch onto the live game will hit the same wall until we either
add a `kotor2_steam_aspyr_laa` entry with our exe's actual hash to the
manifests we want to use (hook addresses are unaffected by the header flag,
so this should be safe — decide this explicitly before Phase 1, don't
default into it), or find another path.

**Also found:** the plan's suggested example, `AdditionalConsoleCommands`,
turned out to only declare support for the GOG builds (`kotor2_gog_aspyr`)
and KOTOR1 — no Steam Aspyr `hooks.toml` exists for it at all. Used
`K2AspyrVisibleConsole` instead (explicit `kotor2_steam_aspyr` support,
single well-documented purpose — makes the Aspyr build's stripped-invisible
cheat console render text again).

**Build:** clean. `dotnet build KotorPatchManager.Managed.slnf` succeeded;
`./build-mingw.sh` cross-compiled `KotorPatcher.dll` + the `binkw32.dll`
proxy with no errors; `python3 ../create-patch.py` packaged
`K2AspyrVisibleConsole.kpatch` for `windows_x86`.

**Install:** proven safe. `KPatchLauncher <exe> --patches <dir> <id>` (CLI
mode) staged the proxy `binkw32.dll`, `KotorPatcher.dll`, `sqlite3.dll`,
`patch_config.toml`, and `patches/k2-visible-console.dll` correctly into an
isolated copy (`backups/vanilla_clean` copied to a scratch dir) — verified
byte-for-byte that the **live** game directory was never touched (hash
comparison before/after).

**Injection: proven working.** The same CLI's automatic post-install
"launch" step does **not** run the exe you point it at — for a Steam
distribution on Linux it shells out to `steam://run/<appid>`, which hands
off to Steam and starts the app from its *registered* library path. That
launched the live, unpatched game by accident (harmless — no patches were
staged there — but caught and killed cleanly via the
[[kotor2-crash-triage]] skill's PID-targeted `kill -TERM` method; **worth
knowing for Phase 1**: KPatchLauncher's own launch button/CLI-follow-launch
is not usable for testing an out-of-place copy). Testing the isolated copy
instead required invoking `Proton - Experimental` directly
(`proton run <exe>`, `STEAM_COMPAT_DATA_PATH` pointed at the real
`compatdata/208580` prefix so it's the actual Proton build/prefix in use).
Under that direct launch, the game hit an early
`EXCEPTION_ACCESS_VIOLATION` — but Wine's crash-time module dump is the
proof we needed: `kotorpatcher` and `k2-visible-console` both appear
**loaded in the live process**, alongside `binkw32hooked` (original,
renamed) and our `binkw32` proxy. Evidence saved in
`tools/kotor-patch-manager/phase0_evidence/`.

**The crash itself is not a patch bug.** A control run — the exact same
pristine `vanilla_clean` copy, zero patches installed, launched the exact
same way — hit the **identical** crash (same address `0047afa8`, identical
backtrace, identical register state pattern). A `steam_appid.txt` workaround
didn't change it either. Root cause: launching a copy from outside Steam's
registered library path for AppID 208580 fails a Steamworks
DRM/ownership check, independent of Patch Manager entirely. This is an
artifact of the *isolated-copy test methodology*, not of the framework —
confirmed, not guessed.

**Not yet confirmed:** the actual visible-console behavior in-game (Step 7,
"open the console and confirm") and a clean run to the main menu, since both
of our test copies hit the DRM crash above before reaching that point. Given
the choice between (a) stopping here with injection proven and the crash
positively attributed to test methodology, or (b) setting up a Steam
non-Steam-game shortcut to get a DRM-clean launch of the isolated copy, the
user chose (a). This specific gap — full in-game confirmation — will close
naturally in Phase 1, which installs onto the live game (once the hash
question above is resolved) and launches through the real, already-working
Steam launch command.

**Resolved Proton-version question:** `config.vdf`'s `CompatToolMapping`
for `208580` reads `"name": "proton_experimental"` — the active build is
**Proton - Experimental**, not Proton 9.0 (Beta). No GUI check was needed;
this was answered directly from the Steam config file on disk.

**Gate verdict: PASS, with one open item carried into Phase 1** — the
LAA-hash mismatch above must be explicitly decided before installing any
patch (ours or from `04_recommended_installs.md`) onto the live game.

## Phase 1a — Pilot one Steam-compatible mod on the live game

**Status: DONE, success, 2026-09-28.** Closed Phase 0's one unfinished
item — full in-game visual confirmation of a patch on the real,
DRM-clean launch path.

**LAA-hash workaround: worked exactly as planned, plus one gate the
original plan didn't anticipate.** The plan's Step 2 (add a
`kotor2_steam_aspyr_laa` entry to `K2AspyrVisibleConsole`'s
`manifest.toml` and `kotor2-steam-aspyr.hooks.toml`) was necessary but
not sufficient. A second, separate version gate exists:
`PatchApplicator`'s address-database step
(`src/KPatchCore/Applicators/PatchApplicator.cs:604-654`) matches the
live exe's SHA-256 against a `sha256_hash` column in per-version SQLite
files under `AddressDatabases/` (one file per exact hash, single-row
`game_version` table), independent of the manifest/hooks version check.
Installing failed with `No address database found for game version SHA:
4AB72FC1...` even after the manifest/hooks edit. Fixed the same way as
the manifest/hooks edit — non-destructively, by addition rather than
modification: copied `AddressDatabases/kotor2_steam_aspyr.db` to a new
sibling `kotor2_steam_aspyr_laa.db` and updated only its `sha256_hash`
row to the live exe's hash, reasoning that the LAA header-flag flip and
the still-unidentified embedded shader-string edit (see Phase 0/Known
open risk notes) don't relocate any code, so every address in the
existing database stays valid for the LAA build too — consistent with
the manifest/hooks edit reusing the *same* hook file (same addresses)
for both hashes. Copied into both the canonical source dir
(`AddressDatabases/`, so future rebuilds pick it up via the existing
`.csproj` copy rule) and the already-built `bin/Debug/AddressDatabases/`
(so the current CLI binary could use it without a rebuild). **Record
this as a repeatable two-part recipe for the next mod**: (1) add the
hash to `manifest.toml` + `<version>.hooks.toml`'s `target_versions`,
(2) clone the matching `AddressDatabases/*.db` and retarget its
`sha256_hash` — both are required, and the second one is easy to miss
since it fails at a different step with a differently-worded error.

**Install:** `KPatchLauncher` CLI staged everything correctly into the
**live** game directory (verified via file listing): proxy `binkw32.dll`
+ renamed original `binkw32Hooked.dll`, `KotorPatcher.dll`, `sqlite3.dll`,
`patch_config.toml`, `addresses.db`, `patches/k2-visible-console.dll`.

**Launch: proven working, no DRM issue this time** — confirming Phase
0's diagnosis that the earlier crash was purely an artifact of testing an
out-of-place copy. `KPatchLauncher`'s auto-launch (`steam://run/208580`)
started the real Steam-registered install; `swkotor2.exe` ran normally
under Proton (confirmed via `ps aux`, active CPU usage, no premature
exit).

**Proton log (`~/steam-208580.log`) confirms clean hook application:**
```
[KotorPatcher] Loaded 3 patches from config
[KotorPatcher] Set KOTOR_VERSION_SHA = 4AB72FC1AB082F42...
Loaded k2-visible-console.dll at 79E30000: native
[KotorPatcher] Applied DETOUR hook at 0x004748BA -> DrawPrintStringHook
[KotorPatcher] Applied DETOUR hook at 0x004082E0 -> DrawPrintStringHook
[KotorPatcher] Applied DETOUR hook at 0x004083C5 -> DrawPrintStringHook
[KotorPatcher] SUCCESS: Patcher initialized
```
No crash/exception signal in the log (a handful of benign `System.
ComponentModel.Win32Exception: Invalid window handle` lines are from
`KPatchLauncher`'s own post-launch window-focus logic, not the game
process — cosmetic). `AR_ERROR.LOG` only shows a routine, unrelated
`Names Differ: v_mnstun cryomine` line.

**In-game confirmation: PASS, user-confirmed with a screenshot.** One
extra step the plan didn't call out: the live `swkotor2.ini` had no
`EnableCheats` key at all (not `=0`, just absent). Added
`EnableCheats=1` under `[Game Options]` before the user could test —
console input itself doesn't require this (the patch's own code comment
notes Aspyr's console "still reads input" pre-patch), but it removed a
confounding variable before testing cheat *command* execution.
Screenshot shows `~` opening the console with `test` typed and `Function
test not found` rendered back in the restored bitmap font — exactly the
text-rendering behavior this patch restores, and the one thing Phase 0
could not confirm. Game also reached the main menu and ran normally in a
loaded save (per user: "Game looked good otherwise, loaded and ran
around before exiting") — no regression observed.

**Rollback:** not needed — no issues encountered. `KPatchLauncher`'s
`PatchRemover.RemoveAllPatches` path remains available and untested in
this phase; the Step 1 backup
(`backups/patch_manager_phase1a_pre_install_20260928_174827/`) is the
fallback if ever needed.

**Gate verdict: PASS.** Full real-world workflow validated end to end:
edit manifest/hooks + address-db for a new hash → rebuild `.kpatch` →
install via CLI onto the live game → real Steam launch → in-game visual
confirmation. This unblocks either resuming
`06_gog_to_steam_conversions.md` or starting Phase 2 (Ghidra setup) for
the two custom mods — **user's choice, not decided here.**

## Phase 1b — Batch install of all remaining Steam-compatible mods

**Status: DONE, success, 2026-09-28.** User asked to install every
Steam-Aspyr-compatible mod in the repo. Surveyed all 19 candidates
declaring `kotor2_steam_aspyr` support, checked every hook address for
proximity to the live exe's 7 byte-diff clusters (regenerated directly —
see below), and cross-checked descriptions for functional conflicts.

**Critical correction made mid-analysis**: the first safety pass used a
flat `file_offset = VA - 0x400000` conversion, matching the one worked
example in the Phase 0/1a notes. This is **wrong for anything outside
`.text`** — each PE section has its own VA↔file-offset delta (parsed
directly from the section table: `.text` delta `0xC00`, `.rdata`
`0x1400`, `.data` `0x2200`, `.rsrc` `0x70400`; `ImageBase 0x400000`).
Using the flat formula understated real proximity for `.rdata`/`.data`
hooks by exactly that delta, which is why `K2AspyrColorCorrection`
initially looked ~4,855 bytes clear when its true distance is **10
bytes**. **For any future hook-safety check on this exe, always resolve
VA→file-offset through the actual section table, never assume a flat
image-base offset works past `.text`.**

**Excluded, hard conflicts (unchanged from Phase 1a's notes)**:
`4GBPatch` (exact byte conflict with mod #4, already applied),
`K2AspyrShaderFixes` and `K2AspyrFogReflectionFix` (hooks land 0-4,034
bytes from diff clusters — confirmed unsafe with the corrected math too),
`DeSteamify` (removes Steamworks gating entirely — wrong tool for a
Steam-launched setup), `FullPartySelect` (would clobber the
ultrawide-fixed `partyselect_p.gui` — needs hand-merge first, tracked
separately). User also declined `BorderlessFullscreen` (needs
`Fullscreen=0`/`AllowWindowedMode=1`, a real display-mode change) and
both `StoreConfirmThreshold2500`/`StoreNoConfirmThreshold` (mutually
exclusive with each other, user wants neither) — **follow-up for later,
not forgotten.**

**Two "caution-tier" mods the user approved for inclusion were blocked by
the framework's own validation, not by our proximity heuristic** —
significant finding, closes part of the "unidentified shader edit"
mystery from Phase 0/1a:
- `K2_music_fix` (disables audio ducking) wanted bytes `D8 4D D8` at
  `0x0070E130`; found `90 90 90` — **audio ducking is already NOP'd out
  by hand** on this exe. This is exactly the second mystery cluster
  found in Phase 1a (`VA 0x70d530-0x70d532`) — now identified.
- `colorshift_k2` (removes Aspyr's yellow tint) wanted the stock
  gamma-only ARB fragment shader at `0x00989DB0`/file `0x5889B0`; found
  one already rewritten with `MUL r0.r,r0.r,.90; MUL r0.g,r0.g,.85; MUL
  r0.b,r0.b,.90` spliced in before the `POW` gamma calls — **yellow-tint
  correction is already hand-patched in**, at file offset 10 bytes from
  the first Phase 0/1a mystery cluster. Now identified as specifically a
  color-correction edit, though who/when is still unknown.
Both hand patches already deliver approximately the effect their
respective Patch Manager mods would add, so both were dropped from this
batch rather than reconciled — reconciling would mean rewriting the
mod's `original_bytes`/`replacement_bytes` to build on top of an
already-modified shader/instruction stream, a deliberate future decision
if ever wanted, not a default action. The two other, still-unidentified
mystery byte clusters (`0x98ec2a-0x98edea`, `0x98f522-0x98f66f`,
`0x98fad2-0x98fbd9`, `0x9f5993-0x9f6935`) remain unexplained but are
exactly where `K2AspyrFogReflectionFix`/`K2AspyrShaderFixes` hooks land
— consistent with "something shader-related," still not blocking since
neither mod is part of any planned install.

**Operational discovery — record for every future install**:
`KPatchLauncher`'s apply-CLI (`RunApplyCli`) always calls
`PatchRemover.RemoveAllPatches` *before* validating/installing the
requested set (`Program.cs:187`). If the new set then fails validation,
**the game is left fully uninstalled, not rolled back to its previous
patch state.** This happened here: the first batch attempt (including
the two blocked mods) silently removed Phase 1a's working
`K2AspyrVisibleConsole` install before failing, leaving the live game
briefly patch-free. Not a bug to work around — just always expect a
CLI apply call to be all-or-nothing against a clean slate, never
additive on top of a prior partial install.

**Final installed set (9 patches, LAA-hash manifest/hooks edit applied
to each the same way as Phase 1a)**: `k2-visible-console`,
`good-alignment-heads`, `k2-aspyr-normal-map-fix`,
`k2-aspyr-saber-hilt-texture-fix`, `k2-finesse-melee-fix`,
`k2_grass_fog_fix`, `max-lights`, `semi-transparent-letterbox`,
`k2-mp3-playback-fix`. Backup taken first at
`backups/patch_manager_phase1b_pre_install_20260928_180802/`. Install
succeeded: 34 individual hooks applied (`REPLACE`/`SIMPLE`/`DETOUR`
mixed), Proton log shows `[KotorPatcher] SUCCESS: Patcher initialized`
with no errors, game launched and ran actively under the real Steam path,
`AR_ERROR.LOG` unchanged from baseline.

**Gate verdict: PASS.** Full batch-install workflow validated, including
a real failure case (blocked-hook validation) handled safely with zero
data loss. Two hand-patch mysteries identified; two more remain open,
tracked, not blocking.

## Phase 1 — Pilot install of existing safe mods (SUPERSEDED by 1a+1b)

Original plan text kept for history only — Phase 1a and Phase 1b above
did this (and considerably more: all 19 Steam-Aspyr-compatible mods in
the repo were surveyed, not just 2-3) on the **live** game directly
rather than a backup/test profile, since 1a already validated the launch
workflow end to end. Do not re-run this phase as originally scoped.

Original text: from `04_recommended_installs.md`'s "safe to install"
list, install 2-3 (suggest `FrameLimiter`, `BorderlessFullscreen`,
`ScriptExtender` — varied hook types, low risk) on a backup/test profile.

## Phase 1c — Remaining Steam-compatible mods to reconsider later (not started)

Everything below was deliberately **not** installed in Phase 1b. This is
the checklist for "investigate the next round of patches," so a future
session doesn't need to re-derive the Phase 1b analysis:

- **`BorderlessFullscreen`** (id `borderless_fullscreen`) — hook-safe
  (483,403 bytes from any diff cluster), user declined only because it
  requires `Fullscreen=0`/`AllowWindowedMode=1` in `swkotor2.ini`, a real
  display-mode change from the current exclusive-fullscreen setup. Revisit
  if the user wants to discuss that tradeoff specifically.
- **`StoreConfirmThreshold2500`** / **`StoreNoConfirmThreshold`** (mutually
  exclusive, repo-declared `conflicts`) — user said skip both, vanilla
  store-confirmation behavior is fine. Only revisit if that preference
  changes; nothing technical blocks either.
- **`FullPartySelect`** (id `full-party-select`) — hook-safe (963,233
  bytes clear) but requires installing its own `partyselect_p.gui` into
  `Override/`, which would clobber the already ultrawide-fixed version of
  that screen (`[[kotor2-ultrawide-gui]]`). Needs a hand-merge of the two
  `.gui` files before this is a clean install, not a default "just
  install it" candidate.
- **`K2_music_fix`** / **`colorshift_k2`** — blocked by real pre-existing
  hand patches already on the live exe (audio ducking already NOP'd out;
  yellow-tint color correction already spliced into the gamma shader — see
  Phase 1b's writeup for exact bytes/addresses). Only installable by
  rewriting each mod's own `original_bytes`/`replacement_bytes` to build on
  top of the existing hand patch instead of assuming vanilla Aspyr bytes —
  a deliberate reverse-engineering task, not a default action, and
  arguably unnecessary since the desired effect already exists.
- **`K2AspyrShaderFixes`** / **`K2AspyrFogReflectionFix`** — hook
  addresses land 0-4,034 bytes from the known byte-diff clusters (the
  worst case, `K2AspyrShaderFixes`, has one hook only 10 bytes from a
  cluster boundary). Not safe to install as-is; would need the same kind
  of byte-level investigation as `K2_music_fix`/`colorshift_k2` above
  before attempting, since something already modified that region and we
  don't yet know what.
- **`4GBPatch`** — permanently excluded, direct conflict with the
  already-applied 4GB Patcher (mod #4). Not a candidate, ever, unless
  mod #4 is removed first (out of scope, no reason to do this).
- **`DeSteamify`** — permanently excluded, wrong category of tool
  entirely (removes Steamworks gating for non-Steam launchers). Not a
  candidate for this Steam-launched setup regardless of hook safety.

**Also**: 40 other patches in `Patches/` were never evaluated at all
because they don't declare `kotor2_steam_aspyr` support in `manifest.toml`
(KOTOR1-only, GOG-only, or otherwise not applicable to this build) — see
the `04_recommended_installs.md`/`03_framework_research.md` docs, or
re-run `grep -qE '^\s*kotor2_steam_aspyr\s*=' manifest.toml` across
`Patches/*/manifest.toml` to regenerate that candidate list if pursuing
GOG→Steam ports (`06_gog_to_steam_conversions.md`, still deferred).

## Phase 2 — Ghidra environment

**Status: DONE, 2026-09-28** — stood up alongside porting
`K2AspyrLoadingScreenLineFix` (see `06_gog_to_steam_conversions.md`),
since that port needed the exact same environment "for free."

1. Check for an existing community GZF/SARIF database covering the K2
   Aspyr Steam build before reversing anything from scratch — a lot of
   the KOTOR modding community's Ghidra work may already be shared.
2. If none exists or coverage is thin, set up Ghidra with the exe, confirm
   we're analyzing the exact same build our `manifest.toml` will target
   (checksum match, per the framework's own per-build SHA-256 story).

### Outcome (2026-09-28)

- **Community coverage checked, none found.** No full GZF/SARIF
  disassembly database exists for the Steam Aspyr `swkotor2.exe`
  specifically. One narrower, unrelated project exists —
  [K2SE](https://github.com/renanaugustomacena-ux/K2SE) ("KOTOR 2 Script
  Extender," same 1.0.2.0 Aspyr/Steam build) ships
  `tools/k2_routine_table.csv` labeling all 655 NWScript action-routine
  handlers — not relevant to GUI/panel-sizing work, noted for later if
  NWScript-VM-adjacent hooks ever come up.
- **Toolchain**: no Java or Ghidra pre-existing on this machine.
  Installed `openjdk-21-jdk` via apt (Ubuntu 24.04's own repo, no
  external feed needed — Ghidra 12.x requires JDK 21, refuses to run on
  17 or older). Ghidra release: **12.1.4** (2026-08-18), downloaded from
  the official GitHub release
  (`ghidra_12.1.4_PUBLIC_20260921.zip`, ~544MB) to
  `~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC/`.
- **GUI sanity check**: `ghidraRun` launches cleanly under the existing
  `DISPLAY=:1` X11 session (`application.log` clean, no errors, process
  idle waiting for input) — confirms the environment itself is sound.
- **Actual analysis work done headless, not via the GUI.** This machine
  has no `xdotool`/`wmctrl` to drive Ghidra's Swing UI programmatically,
  so instead of the originally-planned manual File→New Project/Import
  clicking, used `support/analyzeHeadless` for project creation, PE
  import, and auto-analysis (`-import`), then `-process` +
  `-postScript` with small custom Java `GhidraScript`s for the actual
  reverse-engineering (byte-pattern search, string cross-reference
  listing, targeted decompilation/disassembly dumps) — see
  `06_gog_to_steam_conversions.md`'s candidate #1 writeup for how this
  found the port target. This is fully reproducible/scriptable (no
  manual clicking to redo), which turned out to be strictly better for
  this kind of work than the GUI would have been, not just a workaround.
  Scripts live in
  `kotor-patch-manager/tools/ghidra_scripts/` alongside the framework's
  own `ExportFunctionsCsv.java`:
  `FindLoadingScreenPattern.java`, `FindLoadStrings.java`,
  `DecompileTargets.java`, `DumpFunctionDisasm.java` (all one-off/ad-hoc,
  kept for reference — expect to write similar throwaway scripts per
  future candidate rather than reusing these verbatim).
- **Project**: non-shared Ghidra project `KOTOR2_SteamAspyr` at
  `~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects/KOTOR2_SteamAspyr`,
  single imported/analyzed program `swkotor2.exe` — imported from
  `backups/vanilla_clean/swkotor2.exe` (hash
  `6a522e71...04dbbffef`, confirmed matching the pristine
  `kotor2_steam_aspyr` manifest hash), not the live exe, per the
  reasoning in `06_gog_to_steam_conversions.md`. Auto-analysis (default
  analyzers) took ~5 minutes for this 6.5MB 32-bit PE.
- The framework's own `tools/ghidra_scripts/ExportFunctionsCsv.java` /
  `tools/SqliteTools/` (for populating `AddressDatabases/*.db`) were
  **not needed** for this specific port (a `simple` hook with no DLL
  never reads `addresses.db`) but the script directory was added to the
  headless script path so they're one `-scriptPath` flag away for the
  next candidate that does ship a DLL.
- **This same headless environment/script setup carried straight through
  to candidate #2, `K2AspyrMapAspectFix`, now also DONE (2026-09-28) and
  live (11 patches total)** — no re-setup needed, confirming the Phase 2
  investment here was reusable as intended. Full port writeup, install
  outcome, and the ultrawide-area-map resolution are in
  `06_gog_to_steam_conversions.md`'s candidate #2 section; a transient
  install-time hang (not attributed to this patch) is in
  `CRASH_INVESTIGATION.md`.

## Phase 3a — Buff Duration HUD (lower effort, build first)

Per `02_buff_duration_hud.md`:
1. Find `CGameEffect`/`ActiveEffects`/`DurationRemaining` memory layout —
   likely already documented given how central effects are engine-wide.
2. Pull the *installed* `spells.2da` (not a reference copy) for effect IDs,
   since TSLRCM + several mechanics mods in this build (#8, #102-109,
   #117-119, #139-142) change force-power rows.
3. Find the per-frame update loop and `CUIHUD::Render()` (or equivalent)
   addresses.
4. Apply our existing `HUD_DESC_COEFF` ultrawide coordinate transform
   ([[kotor2-ultrawide-gui]]) to overlay placement — don't reinvent it.
5. Build, install, verify: buff appears/disappears at correct times, wipe
   animates smoothly, no HUD-slot collision with the still-open slot 0/
   slot 5 calibration item from the ultrawide project.

## Phase 3b — Inventory 3D Viewport (higher effort)

Per `01_inventory_3d_viewport.md`, using `upgrade_p` (workbench) as the
primary reference rather than the character screen:
1. Diff mod #153's existing `upgrade_p` camera changes first.
2. Copy `CONTROL_3DVIEW` from `upgrade_p.gui` into `inventory.gui`,
   positioned per the ultrawide margin convention.
3. Ghidra: trigger (workbench item-list selection handler), data
   resolution (`.uti` → `.mdl`), render call (`CUI3DView` item-rendering
   path, not the creature path) — compare against `upgrade_p`'s own calls
   to these as the known-working reference.
4. Build hook, verify against items with no distinct model (stacked
   consumables) — must degrade gracefully, not crash.
5. Verify resolved `.mdl`/`.mdx` respects the Ultimate Character Overhaul /
   HD portrait override chain (mods #15-16, #46-47), not vanilla chitin.

## Phase 4 — Full-build integration test

Run both mods together with the full 154-mod build active. Use existing
verification tooling (`kotor2_toolset.py verify`) plus a MangoHud
before/after perf comparison, matching the monitoring setup already in
place from the ultrawide project ([[kotor2-ultrawide-gui]]).

## Explicitly out of scope for this plan

- Building/installing `FullPartySelect` or the "K2 Active Party Extension"
  — tracked separately in `04_recommended_installs.md`, not blocking these
  two mods.
- Any change to the native vs. Proton build decision — noted as a
  fallback option only, not something to switch without a separate,
  explicit decision.
