# Batch-3 GOG→Steam porting: detailed continuation plan

> **2026-10-03 note:** the 2026-09-28 LAA-alias steps below (live hash `4ab72fc1...`, `4AB72FC1` in `target_versions`, `kotor2_steam_aspyr_laa` manifest key / `_laa.db`) are OBSOLETE. The live exe is vanilla `6A522E71` + KPM `4gb-patch` (`7751F7D8`); use only `kotor2_steam_aspyr` / `6A522E71` (see `vanilla_migration/README.md`).

**Written 2026-09-28 for a context-clearing hand-off.** The user asked to
persist everything and switch to plan mode because context was getting
full. This file is the source of truth for resuming — read this instead
of re-deriving anything. Cross-references: `06_gog_to_steam_conversions.md`
(candidates #3a and #5), `ghidra_knowledge/kotor2_ghidra_knowledge.db`
(`hooks_ported` + `lessons` tables, scope `AdditionalConsoleCommands` and
`post-combat-movement-fix-v1`), [[kotor2-patch-manager]] memory.

## Why this plan exists

User direction this session (2026-09-28, after `K2AspyrMapAspectFix`
shipped): **"we've been having good luck with porting these existing
mods when the port is simple between gog and steam, I think we're safe
to start doing batches of related patches."** This authorizes batching
multiple simple GOG→Steam ports into one build+install cycle instead of
strictly one-at-a-time — but per [[feedback-phase-sequencing]], "simple"
still needs verifying per-candidate, not assumed from hook count alone.
That's exactly what happened this session: one candidate stayed simple,
one didn't.

**Live game state unchanged since last plan**: 11 patches live and
stable (`k2-visible-console`, `good-alignment-heads`,
`k2-aspyr-normal-map-fix`, `k2-aspyr-saber-hilt-texture-fix`,
`k2-finesse-melee-fix`, `k2_grass_fog_fix`, `max-lights`,
`semi-transparent-letterbox`, `k2-mp3-playback-fix`,
`loading_screen_line_fix_k2`, `k2-aspyr-map-aspect-fix`). User confirmed
in-game: map/minimap fix works, resolves the ultrawide area-map item, no
regressions. Live exe hash unchanged:
`4ab72fc1ab082f427e008cda32fc5602d27b4e12fef48c4a1a2c6f7b2f36fb5a`.
Pristine/Ghidra-analysis-target hash:
`6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef`.

**Git commit-scope policy unchanged**: only commit to this worktree
(`local-setup`/`KOTOR-II`, where `patch_manager_mods/` lives). Never
commit to the `kotor-patch-manager` clone
(`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/`) — that repo
has its own pre-existing uncommitted changes from prior phases, and no
mod is ready to contribute upstream yet.

**Ghidra environment**: 12.1.4 + JDK 21, project at
`~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects/` (project name
`KOTOR2_SteamAspyr`, program `swkotor2.exe`, full auto-analysis already
run — always open with `-process swkotor2.exe -noanalysis`, never
re-analyze). Headless invocation pattern used throughout this session:
```bash
GHIDRA=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC
"$GHIDRA/support/analyzeHeadless" \
  ~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects KOTOR2_SteamAspyr \
  -process swkotor2.exe -noanalysis \
  -scriptPath ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts \
  -postScript <ScriptName.java> \
  2>&1 | sed -n '/REPORT: Execute script/,$p'
```
Scripts written this session, all still present in
`kotor-patch-manager/tools/ghidra_scripts/` (not committed anywhere,
that repo isn't tracked for these — they just live on disk):
`FindBatch2Patterns.java`, `InspectConsoleHook1.java`,
`FindConsoleFuncCallers.java`, `FindConsoleInitCaller.java`,
`DumpConsoleInitRegion.java`, `DumpConsoleInitRegion2.java`,
`FindTablePointerRefs.java`, `DecompileCRTStartup.java`. Full
descriptions of what each does/found are in the `ghidra_knowledge` DB's
`ghidra_scripts` table — query it before writing a new script in case one
of these is reusable as-is.

**Methodology reminders that still apply** (all confirmed multiple times
this project, don't re-litigate):
- `findBytes(Address,String,int)` is a **regex** API — never pass plain
  hex. Use `find(Address,byte[])` in a manual loop (see
  `FindAllCorrected.java` / `FindBatch2Patterns.java` for the template).
- An exact byte match is a candidate list, not an answer — confirm via
  surrounding context (nearby calls, struct offsets, decompiled shape).
- VA→file-offset conversion for the byte-diff-cluster proximity check
  must go through the real PE section table
  (`ImageBase 0x400000`; `.text` delta `0xC00`, `.rdata` `0x1400`,
  `.data` `0x2200`, `.rsrc` `0x70400`) — a flat `VA - 0x400000` badly
  understates proximity outside `.text`. For a hook confirmed in `.text`
  compared against a cluster whose section is unconfirmed, the naive
  direct-VA-subtraction distance is still a valid *upper bound* on risk
  as long as it's comfortably larger than the largest possible section-delta
  spread (~0x70400 max, realistically ~0x1600 if the cluster's in
  `.rdata`/`.data` like the known ones are) — both hooks below cleared
  this with >1.8MB and >900KB margins respectively, so no exact recompute
  needed unless a much closer candidate ever comes up.
- This framework patches **runtime memory** via `KotorPatcher.dll`
  injected through a hijacked `binkw32.dll` for `simple`/`detour`/
  `replace` hooks — it does NOT rewrite the exe on disk. An isolated-copy
  file-diff proves nothing; the isolated-copy `patch_config.toml`
  byte-check is the right pre-live signal, and the live steam log's
  `Applied ... hook at 0x...` line is the real proof it worked.
- `KPatchLauncher`'s apply CLI always uninstalls everything first, then
  installs the requested set — always pass the **full** intended patch
  ID list, never just the new one(s), or you'll silently drop the other
  live patches.
- Running `KPatchLauncher` against an **isolated copy** still triggers a
  `steam://run/208580` auto-launch of the **live** game as a side effect
  — expected, harmless, just remember to close that extra instance
  afterward (check `ps aux` for `swkotor2.exe`, confirm elapsed/CPU-time
  ratio isn't a hang per [[kotor2-crash-triage]], then `kill -TERM` it if
  it was just the side-launch and not something the user was actively
  using).
- The live install earlier this session triggered a transient hang
  (loading-screen-adjacent, `NtGetContextThread` spin, no exception) that
  reproduced twice then cleared on the 3rd relaunch attempt, not
  attributed to `K2AspyrMapAspectFix`'s logic — full triage in
  `CRASH_INVESTIGATION.md`'s "2026-09-28: recurrence, hang while loading
  a save" section. If a hang recurs during the next live install, that's
  the playbook to follow (kill via PID match on the full path, relaunch,
  don't assume the new patch caused it, but do watch for a pattern).

## Candidate A: `Post-Combat Movement Fix` — DONE except live install

**Status: fully confirmed and built, zero remaining Ghidra work.**

Directory: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Post-Combat Movement Fix/`

- GOG hook: address `0x0045BDA8`, type `replace`, 30 original bytes → 17
  replacement bytes (x87 FPU trick: load both raw forward/back input
  locals, `fabs` each, `fcomi`/`fcmovb` to keep the larger-magnitude one).
- **Steam address confirmed: `0x007AF198`** — found via exact 30-byte
  pattern search (`FindBatch2Patterns.java`), **a single unique hit in
  the entire 6.5MB binary**. This is the highest-confidence match of any
  hook ported in this project — no ambiguity, no context-based
  disambiguation needed.
- `kotor2_steam_aspyr.hooks.toml` already written in the patch directory
  (both pristine `6A522E71...BFFEF` and LAA `4ab72fc1...36fb5a` hashes in
  `target_versions` from the start):
  ```toml
  [metadata]
  target_versions = ["6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF", "4AB72FC1AB082F427E008CDA32FC5602D27B4E12FEF48C4A1A2C6F7B2F36FB5A"]

  [[hooks]]
  address = 0x007AF198
  type = "replace"
  original_bytes = [
      0x8B, 0x8D, 0xA0, 0xF9, 0xFF, 0xFF,
      0x8B, 0x91, 0xA0, 0x02, 0x00, 0x00,
      0x8B, 0x85, 0xA0, 0xF9, 0xFF, 0xFF,
      0x8B, 0x88, 0xA0, 0x02, 0x00, 0x00,
      0x8B, 0x12,
      0x8B, 0x42, 0x1C,
      0xFF, 0xD0
  ]
  replacement_bytes = [
      0xD9, 0x45, 0xB8, 0xD9, 0xE1, 0xD9, 0x45, 0xB0, 0xD9, 0xE1,
      0xDB, 0xF1, 0xDA, 0xC1, 0xDD, 0xD9
  ]
  ```
- `manifest.toml` already updated with both `kotor2_steam_aspyr` and
  `kotor2_steam_aspyr_laa` entries under `[patch.supported_versions]`.
- **Built**: `Post-Combat Movement Fix.kpatch` exists in the patch
  directory (`create-patch.py` detected type `SIMPLE`, no C++/DLL,
  skipped compilation as expected).
- **Isolated pre-live check passed**: `patch_config.toml` showed address
  `8057240` (dec) = `0x7AF198`, all 30 original bytes and all 16
  replacement bytes matching exactly, `dll = ""` (correct — no DLL for
  this patch), `target_version_sha` = pristine hash.
- **Byte-diff-cluster proximity check passed**: `0x007AF198` is in
  `.text`; nearest known mystery cluster (`0x9889ba`) is ~1.85MB away by
  direct VA comparison — no collision risk.

**Nothing left except the live install** (see "Batch live-install
procedure" below) and an in-game spot-check that post-combat/Force-power/
item-use movement no longer locks up at high FPS.

## Candidate B: `AdditionalConsoleCommands` — mid-investigation, NOT ready

**Status: hook1 investigated but unresolved; hook2 not started.** This
turned out to be the first genuine GOG/Steam codegen-divergence case in
the whole project — every other hook ported so far was byte-identical.
Don't batch-install this one until both hooks are actually found.

Directory: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/AdditionalConsoleCommands/`

### Address-database prerequisite — CONFIRMED, not the blocker

Every symbol `ConsoleCommands.cpp` calls already exists in
`AddressDatabases/kotor2_steam_aspyr_laa.db` (and the non-LAA
`kotor2_steam_aspyr.db`), checked **function by function**, not just
class-by-class:
- `ConsoleFunc`'s `NoParamConstructor`(0x475c90)/`StringConstructor`
  (0x475d30... — actual raw values: 4675536/4675696/4675856)/
  `IntConstructor`/`Destructor` — all present.
- All 7 `RENDER_*` global pointers (`RENDER_AABB`, `RENDER_GUI`,
  `RENDER_WIREFRAME`, `RENDER_TRIGGERS`, `RENDER_QA_TRIGGERS`,
  `RENDER_PERSONAL_SPACE`, `RENDER_GOB_BBS`) — all present in
  `global_pointers`.
- `CServerExoApp::GetPlayerCreatureId`/`GetCreatureByGameObjectID`,
  `CVirtualMachine::RunScript`, `CClientExoApp::GetClientOptions`,
  `CClientOptions::SetCameraMode`, `CSWSCreatureStats::AddFeat`,
  `CSWSObject::AddActionToFront` — all present in `functions`.
- `GetInstance()`-style singleton accessors and `GetPosition`/
  `GetOrientation`/`GetAreaId`/`GetCreatureStats` are NOT separate
  `functions` rows — they're inline reads composed from
  `global_pointers` (`APP_MANAGER_PTR`, `VIRTUAL_MACHINE_PTR`) and
  `offsets` (`CSWSObject::Position`=148/`Orientation`=160/`AreaId`=144,
  `CSWSCreature::CreatureStats`=4504) — all those rows exist too.

**Conclusion: no new `AddressDatabases` work needed once the hook sites
are found.** This was fully re-verified this session, don't redo it.

### hook1 (`InitializeAdditionalCommands`) — investigated, unresolved

GOG: address `0x00925133`, type `detour`, function
`InitializeAdditionalCommands`, 5 original bytes `68 00 00 40 00`
(`push 0x00400000`).

**What was tried and ruled out**: exact 5-byte search on the Steam
pristine build found only 3 hits total in the whole 6.5MB binary.
Disassembly + decompilation proved **all 3 are false positives**:
1. Two hits (`0x0092a4cc`, `0x0092a4e6`) are inside
   `__IsNonwritableInCurrentImage`, a CRT/DEP validity-check helper —
   confirmed via full disassembly dump, nothing console-related nearby.
2. One hit (`0x0091d52d`) is inside `___tmainCRTStartup`. A **full
   decompile** of this function (`DecompileCRTStartup.java`) — Ghidra
   library-matched it to Visual Studio 2008 Release CRT source — proved
   this `push 0x400000` is simply the `hInstance` argument to the
   `WinMain` call: `FUN_00407920(0x400000,0,uVar2,local_6c.wShowWindow)`
   is `WinMain` itself (classic 4-arg signature). Completely unrelated to
   console commands.

**What the decompile revealed instead**: `___tmainCRTStartup` calls
`__cinit(1)` just before calling `WinMain`. `__cinit` is MSVC's standard
C++ static-initializer dispatcher — this is almost certainly where the
game's built-in `ConsoleFunc` objects get constructed, matching the
patch's own README ("these commands get initialized statically shortly
after game start-up"). **`__cinit` itself is a library function, not
inlined in this decompile — its internals were not examined.**

**Side investigation (promising but inconclusible so far)**: found 35
tiny (~0x30-byte) functions at `0x983ea0`-`0x984500`
(`FindConsoleFuncCallers.java`), each with exactly one call to one of the
3 `ConsoleFunc` constructors — very likely the game's own built-in
console commands' individual static initializers. A follow-on hypothesis
that these are collected into an 87-entry function-pointer table at
`0x9866fc`-`0x986854` (`FindConsoleInitCaller.java` →
`DumpConsoleInitRegion2.java`) is **unconfirmed, possibly a false
lead**: `FindTablePointerRefs.java` found **zero** references to that
table's start address anywhere in the program (checked both via
Ghidra's reference graph and a raw memory byte-scan for the table
start's 4-byte pointer encoding). The 87-entry bound-detection heuristic
(scanning for plausible-looking address values in a broad range) may
have over-extended into unrelated adjacent data — don't trust the table
hypothesis without independent confirmation.

**Next steps, in priority order**:
1. **Check the binary's Memory Map for named `.CRT$XC*`-style sections**
   (`.CRT$XCA`, `.CRT$XCU`, `.CRT$XCZ` are the standard MSVC names for
   the static-initializer function-pointer array boundaries — `__cinit`
   walks from the `XCA`-block address to the `XCZ`-block address calling
   everything in between). This is the *correct*, targeted way to find
   the real array, rather than the ad-hoc range-heuristic already tried.
   In Ghidra: `currentProgram.getMemory().getBlocks()` lists all named
   memory blocks/sections — look for anything with "CRT" in the name, or
   check the PE section headers directly for a segment named `.CRT`.
2. **Decompile 1-2 of the 35 wrapper functions directly** (e.g.
   `FUN_00983ea0`, entry point known) to read their embedded
   command-name string literal — this independently confirms they're
   real `ConsoleFunc` static initializers (matching a known built-in K2
   console command name), which the current evidence only infers
   indirectly via the constructor-call relationship.
3. Once the real static-init array's start/end addresses are confirmed
   (whether via the `.CRT$XC*` section approach or by validating the
   0x9866fc-0x986854 range more rigorously), the correct hook insertion
   point is almost certainly **"right after the last static initializer
   in that array runs, before `WinMain` is called"** — but the exact
   Steam-side bytes at that point still need to be read directly (they
   will NOT match GOG's `68 00 00 40 00` — this is a confirmed-divergent
   site, that pattern is proven wrong as a search anchor, don't retry it).
4. If this proves harder than expected even after the above, consider
   falling back to the Defined-Strings cross-reference method (the one
   that successfully found `K2AspyrLoadingScreenLineFix`'s hook when byte
   search initially seemed to miss) — search Ghidra's Defined Strings for
   any of the game's known built-in console command names (not yet
   identified — would need step 2 above to surface a real name to search
   for) and trace forward from there.

### hook2 (console-history-save flag) — not started at all

GOG: address `0x00867248`, type `simple`, 7 original bytes
`0F B6 15 D0 98 A2 00` (`movzx edx, byte ptr [0x00A298D0]`), 7
replacement bytes `BA 01 00 00 00 90 90` (forces the flag to 1). Comment
in the GOG hooks.toml: "Set Console History to save".

The 0-hit result already observed on Steam for this exact byte pattern
is **expected, not a problem** — the embedded global address
`0x00A298D0` is GOG-specific and will differ on Steam, exactly the same
situation as `K2AspyrLoadingScreenLineFix`'s framebuffer-height global
(`0x009F22A8` GOG vs `0x009F42A8` Steam). This needs a **fresh
investigation from scratch** on the Steam binary — no work done on it
this session. Likely approach: search for the structural shape
(`movzx reg, byte ptr [imm32]` reading some other flag global) near
console/save-related code, or find a save-path/config-related string via
Defined Strings and trace forward. Budget comparable effort to hook1
above; don't assume it's quick just because it's a `simple` (not
`detour`) hook — the previous 3 hooks' relative type didn't predict
difficulty here.

## Batch live-install procedure (once both candidates are ready)

Same backup-then-install pattern as every previous phase. **Do not run
this until `AdditionalConsoleCommands`'s hook1 AND hook2 are both found,
built, and isolated-verified** — `Post-Combat Movement Fix` alone is
ready now if the user wants to install it independently first rather
than wait for the batch to complete (their call — both are reasonable
given the "batch when simple" direction was specifically about not
over-sequencing genuinely-simple ports, not about forcing a harder one to
wait on an easier one, or vice versa).

```bash
# 1. Confirm game isn't running
ps aux | grep -i swkotor2.exe | grep -v grep

# 2. Backup
TS=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="$HOME/Games/KOTOR2_Mod_Workspace/backups/patch_manager_batch3_pre_install_${TS}"
mkdir -p "$BACKUP_DIR"
GAME_DIR="/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
cp "$GAME_DIR/swkotor2.exe" "$GAME_DIR/binkw32.dll" "$GAME_DIR/binkw32Hooked.dll" "$BACKUP_DIR/"
sha256sum "$BACKUP_DIR"/* > "$BACKUP_DIR/hashes.txt"

# 3. Stage ALL intended patches (11 existing + new ones) -- CLI uninstalls
#    everything first, so this list must be complete, never additive.
TOOL_DIR="$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager"
mkdir -p "$TOOL_DIR/batch3_live_patches"
rm -f "$TOOL_DIR/batch3_live_patches/"*.kpatch
for d in K2AspyrVisibleConsole GoodAlignmentHeads K2AspyrNormalMapsFix \
         K2AspyrSaberTextureFix K2FinesseMeleeFix K2GrassFogFix \
         "Max Lights" SemiTransparentLetterbox K2MP3PlaybackFix \
         K2AspyrLoadingScreenLineFix K2AspyrMapAspectFix \
         "Post-Combat Movement Fix" AdditionalConsoleCommands; do
  cp "$TOOL_DIR/Patches/$d/"*.kpatch "$TOOL_DIR/batch3_live_patches/" 2>&1
done
# (drop AdditionalConsoleCommands from this loop if installing
#  Post-Combat Movement Fix alone first)

# 4. Install (IDs, not directory names)
cd "$TOOL_DIR/bin/Debug"
DISPLAY=:1 ./KPatchLauncher "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe" \
  --patches "$TOOL_DIR/batch3_live_patches" \
  k2-visible-console good-alignment-heads k2-aspyr-normal-map-fix \
  k2-aspyr-saber-hilt-texture-fix k2-finesse-melee-fix k2_grass_fog_fix \
  max-lights semi-transparent-letterbox k2-mp3-playback-fix \
  loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix \
  post-combat-movement-fix-v1 additional-console-commands
  # (drop additional-console-commands from this line too if going alone)
```

Verify: `ps aux` (not hung — watch for the known transient-hang pattern,
don't panic, see methodology reminders above), `~/steam-208580.log` for
`Applied ... hook at 0x007AF198` (and the AdditionalConsoleCommands
addresses once known) + `SUCCESS: Patcher initialized`, `AR_ERROR.LOG`
unchanged from baseline (`Names Differ: v_mnstun cryomine`). Then ask the
user to in-game spot-check: post-combat/Force-power/item-use movement no
longer locks at high FPS (`Post-Combat Movement Fix`), and once ready,
the new console commands work (`runscript`, `teleport`,
`walkmeshrender`, etc. — `AdditionalConsoleCommands`).

## After live install: doc/memory closeout (same pattern as every prior phase)

1. `06_gog_to_steam_conversions.md` — mark candidate #5 (now numbered,
   was "newly found") and #3a DONE, add install/in-game outcome.
2. `ghidra_knowledge.db`'s `hooks_ported` table — update
   `post-combat-movement-fix-v1`'s row (already has the confirmed
   address, just needs an "installed live, confirmed working" note
   appended) and add rows for `AdditionalConsoleCommands`'s 2 hooks once
   found.
3. [[kotor2-patch-manager]] memory — update to reflect final installed
   count and outcome, same as done for `K2AspyrMapAspectFix`.
4. Byte-diff-cluster proximity check for AdditionalConsoleCommands' 2
   final addresses once found (not yet done — `Post-Combat Movement
   Fix`'s is already done and clear).
5. Commit to this worktree (`local-setup`/`KOTOR-II`) only — same policy
   as every prior phase, never the `kotor-patch-manager` clone.
6. `CRASH_INVESTIGATION.md` — only if anything unexpected comes up
   during the live install; skip if clean.

## Explicitly out of scope for this plan

- `Movie Patch` and `ExpandedKeyboardControl` — the other 2 remaining
  GOG-port candidates, strictly later, per the revised priority order in
  `06_gog_to_steam_conversions.md` (`Post-Combat Movement Fix` →
  `AdditionalConsoleCommands` → `Movie Patch` → `ExpandedKeyboardControl`).
- Reconciling the still-unidentified mystery hand-patch clusters —
  unrelated, tracked separately.
- Publishing anything upstream to `LaneDibello/Kotor-Patch-Manager` — not
  a default action, blocked by the user's commit-scope policy until a
  mod is ready to contribute.
- The 2 custom mods (3D inventory viewport, buff HUD) — separate track.
