# Porting GOG-Only Patches to Steam Aspyr

**Status:** Phase 2 DONE (2026-09-28) — Ghidra environment stood up,
`K2AspyrLoadingScreenLineFix` (candidate #1) ported and installed live
alongside the existing 9 mods (10 total now active). See `05_build_plan.md`'s
"Phase 2" outcome section for the Ghidra setup itself. **Key discovery,
now confirmed across two candidates**: GOG and Steam Aspyr codegen is
**byte-for-byte identical** at every hook site checked so far — same
instructions, same immediate values, same stack offsets, just at a
different address. Expect literal byte matches to be findable, not just
structurally-similar decompiled shapes requiring re-derivation. Still
confirm per-candidate rather than assume.

**`K2AspyrMapAspectFix` (candidate #2 of 4/6, first of the remaining) —
DONE (2026-09-28), ported and live, installed alongside the existing 10
mods (11 total now active).** All 3 hook addresses found, independently
reviewed (a second Opus pass re-checked every claim against the raw Ghidra
logs before this was trusted), confirmed byte-identical to GOG with
matching stack/struct offsets, and **zero C++ changes were needed** — see
candidate #2's section below for exact addresses and the supporting
evidence. Steam log confirmed all 3 `Applied DETOUR hook` lines +
`SUCCESS: Patcher initialized`, `AR_ERROR.LOG` unchanged from baseline,
byte-diff-cluster proximity check clear (nearest cluster ~950KB+ away
after correct per-section VA→file-offset math — see below). **User
confirmed in-game: both the full area map and minimap are no longer
horizontally stretched, and this resolves the
[[kotor2-ultrawide-gui]] "area map can't be fixed via `LBL_Map`" item** —
correct aspect ratio now, filling the ultrawide width is fine once the
ratio itself is right. No regressions spotted in the other 10 mods.

**Not a clean install, worth recording**: the live install hung twice
while loading a save before succeeding on the 3rd attempt (identical
`NtGetContextThread`/`GetCurrentPackageFullName` spin signature both
times, no exception, not pointing at any of the 3 new hook addresses) —
full triage in `CRASH_INVESTIGATION.md`'s "2026-09-28: recurrence, hang
while loading a save" section. Root cause not confirmed; leaning toward a
flaky Proton/Wine startup race rather than a `K2AspyrMapAspectFix` bug,
since the fix itself works correctly once the game loads and the 2nd
hang's spin signature was identical to an unrelated pre-existing hang
from earlier the same day (before this patch was ever installed). Watch
for recurrence in future sessions.

**Important methodology correction from this session, applies to any
future candidate**: `GhidraScript.findBytes(Address,String,int)` takes a
**regex** string, not plain hex — a literal byte must be written `\xNN`
(e.g. `\xC7\x85\x7C`). Passing plain space-separated or unspaced hex (as
several scripts in this project did) gets silently interpreted as a
literal-ASCII-text regex that can never match binary bytes, producing
consistent, confident-looking 0-hit results regardless of whether the
target bytes exist. This caused several wrong "GOG/Steam codegen has
diverged" conclusions across two different candidates this session (all
now corrected) before being caught. **For exact byte-array search, use
`find(Address,byte[])` in a manual loop instead** (advance `start` to
`hit.add(1)` each iteration) — it takes a raw byte array with no escaping
pitfall. Reserve `findBytes()` for genuine wildcard needs, and always use
proper `\xNN` escaping when you do. **Also important: an exact byte match
is not sufficient on its own to confirm a hook site** — the patterns
involved matched 5-76 other places in the binary each; what actually
confirms the right site is the surrounding context (what's computed right
before/after, matching stack/struct offsets) matching the mod's own C++
assumptions. Full detail, including the specific hit counts and context
evidence for each of this patch's 3 hooks, is in the `ghidra_knowledge`
DB's `lessons` and `hooks_ported` tables — see
[[kotor2-ghidra-knowledge-db]].

## Why this is a good starting point (better than our two custom mods)

Both [`01_inventory_3d_viewport.md`](01_inventory_3d_viewport.md) and
[`02_buff_duration_hud.md`](02_buff_duration_hud.md) require inventing new
behavior from scratch: find an engine hook point that doesn't currently do
what we want, understand it well enough via Ghidra to write new C++ logic,
and hope it's correct. That's a lot of unknowns stacked on each other.

Porting an existing GOG-only patch to Steam is a strictly easier version of
the same skillset: **the behavior is already proven correct** (real authors
already shipped it, it already works on GOG Aspyr) — the only unknown is
*where the same bytes live in a different, but closely related, binary*.
This is the natural place to build Ghidra fluency and validate the
"binary-diffing" workflow before spending that effort on genuinely new
hooks for our own mods. It also gets Phase 2 of
[`05_build_plan.md`](05_build_plan.md) (Ghidra environment setup) started
for free, since it needs the exact same environment.

## How this actually works in Patch Manager (confirmed on disk)

**These are not separate mods, and nothing gets "forked."** A single patch
is one `manifest.toml` (one `id`) that can carry *multiple* `hooks.toml`
files side by side — one per distribution/platform — inside the same patch
directory. We already have a working example of exactly this:
`K2AspyrVisibleConsole` ships `kotor2-gog-aspyr.hooks.toml`,
`kotor2-steam-aspyr.hooks.toml`, `kotor2-steam-aspyr-linux.hooks.toml`, and
`kotor2-steam-aspyr-macos.hooks.toml` — four hook files, one patch id,
one shared C++ source file, one manifest listing all four hashes under
`[patch.supported_versions]`.

So "adding Steam support" to a GOG-only patch means, concretely:

1. **Add a hash entry** to the existing `manifest.toml`:
   ```toml
   [patch.supported_versions]
   kotor2_gog_aspyr = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
   kotor2_steam_aspyr = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"   # new
   ```
2. **Add a new `kotor2-steam-aspyr.hooks.toml`** in the same directory,
   with the same hook *count*, *types*, and *purpose* as the GOG file —
   just different `address`/`original_bytes`/`replacement_bytes` values
   found via Ghidra against the Steam binary.
3. **No new C++, in most cases.** `create-patch.py` compiles one
   `binaries/windows_x86.dll` per patch and stages it for *any* Windows
   version's `hooks.toml` to reference by exported function name (that's
   what `function = "..."` in a `detour` hook resolves against). GOG and
   Steam Aspyr are both 32-bit Windows PE targets, so the same compiled DLL
   serves both — the Steam hooks.toml just needs to point the same
   exported function at wherever that code lives in the Steam binary. The
   C++ logic itself doesn't need to change unless Ghidra reveals the Steam
   build's calling convention or stack layout at that point genuinely
   differs (possible, not yet confirmed either way).

**Is this "separate mods to post" or "versions of the existing mod"?**
Versions of the existing mod, structurally — same `id`, same `manifest.toml`,
same C++ (if any), one additional `hooks.toml`. Whether to actually publish
that addition anywhere is a separate, later decision:
- **Local-only**: keep the new `kotor2-steam-aspyr.hooks.toml` in our own
  clone (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/`) and use
  it for our own install. Nothing to coordinate with anyone.
- **Upstream contribution**: submit it as a PR to
  `LaneDibello/Kotor-Patch-Manager` adding the file to the existing patch's
  folder (not creating a new one). The README explicitly invites this
  (Discord contact, GitHub issues). Benefits the wider KOTOR2-on-Steam
  modding community, since as far as we've found, **no existing Steam
  Aspyr hooks exist for any of these four** patches anywhere in the repo.
  Your call whether that's a goal here or a distraction from our own
  install — not something to default into.

## Candidates found (GOG-only K2 patches with a real, shipped GOG hooks.toml)

Checked every patch directory for "`kotor2-gog-aspyr.hooks.toml` present,
`kotor2-steam-aspyr.hooks.toml` absent." Four real candidates (a handful of
others turned out to be KOTOR1-only and don't apply here at all).

**Correction (2026-09-28, later session): that filename-grep search had a
real gap.** It only matched the literal name `kotor2-gog-aspyr.hooks.toml`
(dash-separated), but hook-file naming isn't standardized across this
repo's patch authors — two more real K2-GOG-only candidates were missed
because they use different filenames for the exact same
`kotor2_gog_aspyr` version-support declaration:
- `Post-Combat Movement Fix` → `kotor2_gog_aspyr.hooks.toml` (underscores,
  not dashes)
- `Movie Patch` → `kotor2_hooks.toml` (a different convention entirely)

Found this time by reading `manifest.toml`'s `[patch.supported_versions]`
directly across all 60 `Patches/*/` dirs (`kotor2_gog_aspyr` present,
`kotor2_steam_aspyr` absent) rather than grepping for one specific hooks
filename. **If ever re-auditing this list again, grep manifests, not
hooks filenames** — the version-support declaration in `manifest.toml` is
the one place every patch author uses consistently; the hooks `.toml`
filename is not. Both new candidates are added below as #5 and #6 (kept
GOG-to-Steam candidates #1-4's original numbering intact rather than
renumbering by priority, since #1-2 are already done/in-progress).

### 1. `K2AspyrLoadingScreenLineFix` — DONE (2026-09-28), ported and live
- **Stayed a `simple` hook, zero C++, exactly as scoped** — no forced
  switch to `replace`. Steam address `0x00885197`, inside
  `CSWGuiLoadScreen`'s constructor (`FUN_008850b0`) — found via literal
  disassembly comparison after a raw byte-pattern search appeared to miss.
  **Correction (found during the `K2AspyrMapAspectFix` port, see below):
  that "miss" was a tooling bug, not real codegen divergence** —
  `GhidraScript.findBytes()` takes a regex string (literal bytes need
  `\xNN` escaping), and the plain hex string passed here was silently
  interpreted as literal ASCII text, so it could never have matched
  regardless of whether the bytes existed. The match was still correctly
  located by following the `"loadscreen_p"`/`"LBL_LOADING"` string
  cross-references from Ghidra's Defined Strings into
  `CSWGuiLoadScreen`'s constructor, then reading its disassembly by hand —
  that method remains valid and reusable, just don't read a `findBytes()`
  miss as evidence of anything about the two builds' codegen.
- **The found 31-byte instruction sequence is byte-for-byte identical**
  to GOG's `original_bytes` — same opcodes, same stack offset
  (`[ebp-0xAC]`), same struct field offset (`+0x48`). Only the
  replacement's embedded global address differs: Steam's framebuffer-height
  global is `0x009F42A8` (found via the resolution-tier logic
  immediately following the hook site — `if (DAT_009f42a8 < 0x4b0) ...`
  selecting between "1024x768"/"1280x960"/"1280x1024"/"1600x1200"
  loading backgrounds) vs GOG's `0x009F22A8`.
- Verified: mechanically via `patch_config.toml`'s exact byte match on an
  isolated copy (not a file-diff — see methodology note below), then live
  via the steam log's `[KotorPatcher] Applied SIMPLE hook at 0x00885197
  (31 bytes replaced)` and `SUCCESS: Patcher initialized`, all 10 patches
  active, `AR_ERROR.LOG` unchanged from baseline. User checked in-game and
  saw no issues, though noted this is confounded with earlier separate
  loading-screen image-upscaling/resolution work — not a clean isolated
  visual confirmation of this specific 1px-line fix.
- Byte-diff-cluster proximity check (per "Open items" below): nearest
  known mystery hand-patch cluster is the gamma-shader color-correction
  patch at `0x9889ba`, over 1MB away from `0x00885197`. No collision.
- **Methodology correction for Step 5 of the generalized procedure
  below**: this framework does not statically rewrite the exe on disk for
  `simple`/`detour` hooks — it's runtime DLL injection (`KotorPatcher.dll`
  via a hijacked `binkw32.dll`), confirmed already in
  `kotor2-patch-manager` memory but re-confirmed concretely here: an
  isolated-copy install left the copied `swkotor2.exe`'s hash completely
  unchanged (0 diff bytes) despite the launcher reporting success. The
  real verification is (a) the written `patch_config.toml`'s hook bytes
  match what was intended, and (b) the steam log's `Applied ... hook at
  0x...` line once the patch actually runs — which in practice means the
  live install step *is* the verification, not a separate isolated-copy
  step beforehand. Update step 7 below accordingly for future candidates.

### 2. `K2AspyrMapAspectFix` — highest value for us specifically
- 3 `detour` hooks (`preserveAreaMapAspect`, `preserveAreaMapProjection`,
  `preserveMiniMapAspect`), each taking a single `ebp` pointer parameter —
  consistent calling convention across all three, a good sign the Steam
  build's codegen for this function will look structurally similar.
- Fixes horizontally stretched full map and minimap — **this is already
  flagged in [[kotor2-ultrawide-gui]] memory as a possible fix for our own
  open "area map can't be fixed via LBL_Map" item.** Worth testing
  specifically for that purpose once ported, not just as a generic port.
- Has a small C++ file; per the mechanism above, likely doesn't need
  changes — only new addresses.

**Progress (2026-09-28, this session, independently re-verified by a
second Opus review pass) — all 3 Steam addresses confirmed, byte-identical
to GOG, no C++ changes needed:**

| Hook function | GOG address | Steam address | Containing Steam function | Exact-byte hits binary-wide |
|---|---|---|---|---|
| `preserveAreaMapAspect` | `0x005633A5` | `0x00894435` | `CSWGuiInGameMap::FUN_00894320` (per-frame map update: fog visibility + marker hit-testing) | 76 |
| `preserveAreaMapProjection` | `0x00564DDD` | `0x00895E6D` | `FUN_00895cc0` (fog-of-war pixel-grid rendering — a *different* function than hook1, not the same one initially assumed) | 5 |
| `preserveMiniMapAspect` | `0x0052202A` | `0x007508AA` | `CSWGuiMainInterface::FUN_00750790` (minimap resize/redraw) | 16 |

**An exact byte match alone does not uniquely identify a hook site** —
each pattern above also matches 4-75 *other* places in a 6.5MB binary.
What actually confirms each address is the surrounding context matching
the C++'s own assumptions, checked individually for each hook (full
evidence with instruction addresses in the `ghidra_knowledge` DB's
`hooks_ported` table):
- **Hook 1**: sits right after `xScale` (`-0x14`) finishes computing
  (`0089435e`/`008943dc`) and right before it's used to size a child rect
  via a `vtable+4` call (`0089448a`). `this` confirmed stored at
  `[EBP-0x180]` (`0089434b`), matching `MapObjectFrameOffset` exactly.
- **Hook 2**: sits right after `xScale`(`-0x14`)/`yScale`(`-0x20`) finish
  computing (`00895dfa`/`00895e50`) and right before their first reads
  (`00895ecd`/`00895edc`) — both offsets match GOG's
  `MapXScaleFrameOffset`/`MapYScaleFrameOffset` exactly.
- **Hook 3**: `FUN_00750790` is the *only* one of the 16 hit-containing
  functions that also references `MiniMapControlOffset`(`0x5ab4`)/
  `MiniMapScaledWidthOffset`(`0x5d54`)/`MiniMapScaledHeightOffset`(`0x5d58`)
  (a sibling function has the offsets but not this hook's byte pattern).
  `this` confirmed stored at `[EBP-0xa8]` (`00750799`), matching
  `MainInterfaceFrameOffset` exactly — this was directly verified via
  disassembly, not assumed from the calling convention.
- **Independent corroboration**: the address gap from hook 1 to hook 2 is
  `0x1A38` in *both* GOG (`0x564DDD-0x5633A5`) and Steam
  (`0x895E6D-0x894435`) — strong evidence these are the right pair of
  sites, not a coincidence.
- **Detour safety**: no branch jumps into the middle of any of the 3
  patched byte ranges in the disassembly checked; one branch
  (`00894382 JZ 0x00894435`) targets hook 1's first byte exactly, which is
  safe (the patch still runs on that path).

Class identified via Ghidra's RTTI/demangler analyzer (`CSWGuiInGameMap`,
`CSWGuiMainInterface` — both had namespace-qualified vtable symbols;
finding them required `Symbol.getName(true)`, not the bare name — see
`ghidra_knowledge` DB lessons). `CSWGuiMainInterface::FUN_00750790`'s
class membership is inferred from its frame layout (not one of the 28
vtable slots dumped for that class) — high-confidence, not RTTI-certain,
same caveat as `FUN_00895cc0`. Full vtable dumps, stack-offset tables,
and the `findBytes()` API gotcha are persisted in
`ghidra_knowledge/kotor2_ghidra_knowledge.db` — see
[[kotor2-ghidra-knowledge-db]].

**Outcome (2026-09-28, DONE)**: `kotor2-steam-aspyr.hooks.toml` written
with the 3 confirmed addresses (both pristine and LAA hashes in
`target_versions` from the start), `manifest.toml` updated, built cleanly
via `create-patch.py` (real MinGW cross-compile — first genuine
new-C++-port build this session, not a no-op), isolated-copy
`patch_config.toml` confirmed exact address/byte/DLL match before going
live. Byte-diff-cluster proximity check: hooks are in `.text` (delta
`0xC00` from `ImageBase`), the nearest known mystery hand-patch cluster
(`0x9889ba`) is in `.rdata`/`.data` (delta `0x1400`/`0x2200`) — worst-case
section-delta correction is only ~5.6KB, and the naive VA-to-VA gap alone
is already ~950KB-2.3MB across all 3 hooks, so no collision by a wide
margin. Live install (all 11 patches) succeeded: steam log shows all 3
`Applied DETOUR hook at 0x00894435/0x00895E6D/0x007508AA ->
preserveAreaMapAspect/preserveAreaMapProjection/preserveMiniMapAspect`
lines + `SUCCESS: Patcher initialized`, `AR_ERROR.LOG` unchanged from
baseline. Install triggered 2 transient hangs while loading a save before
a 3rd attempt succeeded — see `CRASH_INVESTIGATION.md`, not attributed to
this patch's own logic (full writeup above and in that doc). **User
confirmed in-game: full map and minimap both correctly proportioned, no
other mods regressed, and this resolves the ultrawide "area map can't be
fixed via `LBL_Map`" limitation** — the map now uses the right aspect
ratio and is fine filling the ultrawide width at that ratio.

### 3. `AdditionalConsoleCommands` — moderate
- 2 hooks: 1 `detour` (registers a `commands/` folder mechanism other
  patches can hook into) + 1 `simple` (flips a byte so console history
  persists to disk instead of being session-only).
- Useful independent of our two custom mods — other future patches
  (including our own, eventually) could register console commands through
  it rather than each hand-rolling console integration.

### 4. `ExpandedKeyboardControl` — hardest, do last if at all
- 6 `detour` hooks spanning action-bar input, the movie-window Delete key,
  context-cancel dispatch, HUD update, HUD teardown, and mouse-focus
  cancellation — six separate places to re-locate in Ghidra instead of
  one to three.
- Touches HUD/action-bar/Pazaak/party-selection screens we've already
  custom-positioned for ultrawide. No known technical conflict (GFF layout
  edits and exe hook addresses don't collide), but worth an in-game
  functional check — does keyboard nav still land correctly on our
  ultrawide-repositioned controls — once ported, not assumed clean.

### 5. `Post-Combat Movement Fix` — newly found, likely the easiest port in the whole project
- **Found 2026-09-28 (later session)** via the manifest-grep correction
  above — missed by the original filename search because its GOG hook
  file is `kotor2_gog_aspyr.hooks.toml` (underscores).
- **Single `replace` hook, zero C++, zero DLL** — no `.cpp`/`.h`/
  `exports.def` in the patch directory at all, just 3 `hooks.toml`
  variants (`kotor1_103`, `kotor2_gog_aspyr`, `kotor2-steam-aspyr-linux`)
  + `manifest.toml`. This is a smaller footprint than even
  `K2AspyrLoadingScreenLineFix` (which was `simple`-not-`replace` but
  still zero-C++) — no `create-patch.py` cross-compile step needed at
  all, no address-database/LAA-DLL-clone concern (framework only copies
  `AddressDatabases` when some *installed* patch has a non-empty `dll`
  field — true regardless here since other live patches carry DLLs, but
  this patch itself needs nothing from that mechanism).
- GOG hook: address `0x0045BDA8`, 30-byte `original_bytes` → 17-byte
  `replacement_bytes` (x87 FPU trick: loads both forward/back raw input
  locals, takes `fabs` of each, `fcomi`+`fcmovb` to keep the
  larger-magnitude one, discards the other — replaces a slower original
  sequence with a shorter one, well within the "detour needs ≥5 bytes /
  replace needs matching intent" constraint). Well-commented: "KOTOR 2
  uses the same cancellation state machine as KOTOR 1, but this
  unoptimized executable stores raw forward/back input in EBP-based
  locals."
- **Already proven to work on `kotor2_gog_aspyr` and even ported once
  already to `kotor2-steam-aspyr-linux`** (a *different* Steam-family
  binary than our Windows target, but proof the same underlying
  fix/approach survives a GOG→Steam-family port at all) — only our
  Windows Steam Aspyr address is missing.
- Fixes a real, user-facing bug: movement locks after combat
  actions/Force powers/item use at high FPS — not a cosmetic port like
  #1-2, a functional correctness fix.
- **Recommendation: do this one next**, before `AdditionalConsoleCommands`
  — same single-hook simplicity class as candidate #1
  (`K2AspyrLoadingScreenLineFix`), but with zero build step at all since
  there's no C++ to (re)compile, and it fixes an actual gameplay bug
  rather than a visual polish item.

**Progress (2026-09-28, batch-3 session) — CONFIRMED, ready to install,
NOT YET LIVE.** Steam address `0x007AF198` found via exact 30-byte
pattern search — **a single unique hit in the entire 6.5MB binary**, the
highest-confidence match of any hook found in this project so far (no
ambiguity to resolve via context, unlike every other hook ported to
date). `kotor2_steam_aspyr.hooks.toml` written (both pristine and LAA
hashes from the start) and `manifest.toml` updated in
`Patches/Post-Combat Movement Fix/`. Built cleanly via `create-patch.py`
(type `SIMPLE`, no C++/DLL — `dll = ""` confirmed in the isolated
`patch_config.toml`). Isolated pre-live check passed: address, all 30
original bytes, and all 16 replacement bytes match exactly. Proximity
check: `0x007AF198` is in `.text`, ~1.85MB from the nearest mystery
cluster by direct VA comparison — safely clear.

**LIVE-INSTALLED 2026-09-28 (same session).** User chose to install this
one alone rather than wait to batch it with `AdditionalConsoleCommands`
(both were reasonable per the "batch when simple" direction — forcing the
easy one to wait on the still-unresolved hard one wasn't the point of
batching). Installed as the 12th patch alongside the existing 11 (full
patch-ID list passed to `KPatchLauncher`, never just the new one). Steam
log confirmed `Applied REPLACE hook at 0x007AF198 (16 bytes code, 31
bytes replaced)` + `SUCCESS: Patcher initialized`; `AR_ERROR.LOG`
unchanged from baseline; process healthy post-install (CPU/elapsed ratio
declined from 94% toward 60% over ~2.5 minutes, log kept advancing with
normal input-polling activity — no hang). **User confirmed general
stability 2026-09-29**: game started successfully, no issues running
around and going through menus — no regressions to the other patches.
This is a general playtest, not yet a *targeted* spot check that
post-combat/Force-power/item-use movement no longer locks at high FPS —
that remains open.

### 3a. `AdditionalConsoleCommands` — reclassified: NOT simple, real codegen divergence found

**Correction (2026-09-28, batch-3 session)**: originally ranked
"moderate" based on hook *count* (2) and confirmed based on an
address-database check (every named function/global-pointer this
patch's C++ calls already exists in `kotor2_steam_aspyr_laa.db` — see
below). **That DB check is necessary but not sufficient** — it only
proves the compiled DLL's own logic would resolve correctly at runtime;
it says nothing about how hard the *hook site itself* is to locate. For
this patch, unlike every hook ported so far this project
(`K2AspyrLoadingScreenLineFix`, `K2AspyrMapAspectFix`'s 3 hooks,
`Post-Combat Movement Fix`), **the hook site is NOT byte-identical
between GOG and Steam** — this is the first genuine codegen-divergence
case found.

**Address-database check (still valid, not the blocker)**: every symbol
`ConsoleCommands.cpp` needs — `ConsoleFunc`'s 3 constructors + destructor,
all 7 `RENDER_*` global pointers, `CServerExoApp`/`CVirtualMachine`/
`CClientExoApp`/`CClientOptions`/`CSWSCreatureStats` methods, and the
`CSWSObject`/`CSWSCreatureStats` struct offsets used by inline accessors
— is already present in `kotor2_steam_aspyr_laa.db` (confirmed function
by function, not just class by class). No new `AddressDatabases` work
needed for the DLL side of this port, same conclusion as
`K2AspyrMapAspectFix`.

**hook1 (`InitializeAdditionalCommands`, GOG `0x00925133`, detour, 5
bytes `68 00 00 40 00` = `push 0x00400000`) — investigated, NOT resolved,
real work remains.** Exact-byte search found only 3 hits in the whole
Steam binary, and disassembly/decompile analysis proved **all 3 are false
positives**: 2 are inside `__IsNonwritableInCurrentImage` (an unrelated
CRT/DEP helper), and the 3rd is inside `___tmainCRTStartup` but is
provably just `WinMain`'s `hInstance` argument (`FUN_00407920` is
`WinMain`, confirmed via a full decompile — Ghidra library-matched this
function to Visual Studio 2008 Release CRT source). The decompile also
revealed the real likely location of static `ConsoleFunc` construction:
`__cinit(1)`, the standard MSVC static-initializer dispatcher, called
just before `WinMain` — `__cinit` itself is a library function, not
inlined, so its internals (and the actual static-init array bounds in
this binary) are still unexamined. A side investigation found 35 tiny
(~0x30-byte) functions at `0x983ea0`-`0x984500`, each calling exactly one
of the 3 `ConsoleFunc` constructors — very likely the game's own
built-in console commands' static initializers — but the follow-on
hypothesis of an 87-entry function-pointer table at `0x9866fc`-`0x986854`
that something loops over is **unconfirmed and possibly a false lead**:
zero references to the table's start address were found anywhere in the
program, via both Ghidra's reference graph and a raw memory byte-scan.
Full evidence trail, including exact addresses and the specific
disassembly/decompile findings, is in the `ghidra_knowledge` DB's
`lessons` table (scope `AdditionalConsoleCommands`) — read that before
resuming, don't re-derive.

**hook1 — RECLASSIFIED 2026-09-28 (follow-on session): likely resolved,
NOT a real codegen divergence after all.** Both of the prior session's
"not done" next-steps were completed and together overturn the false-
positive verdict:
- Memory Map has no `.CRT$XC*`-named blocks (only `Headers`/`.text`/
  `.rdata`/`.data`/`.rsrc`/`tdb`) — expected, MSVC merges those into
  `.rdata`; a negative-but-expected result, not a dead end.
- Decompiled the first 3 of the 35 wrapper functions at
  `0x983ea0`-`0x984500`: `FUN_00983ed0` calls the `StringConstructor`
  with the literal `"setcomputeruse"`, `FUN_00983f00` with
  `"setdemolitions"` — direct, independent confirmation these are real
  built-in `ConsoleFunc` static initializers, not an inferred guess.
- **Re-examined the `0x0091d52d` hit** (previously dismissed as "just
  `WinMain`'s `hInstance` argument, unrelated to console commands").
  Disassembly shows it's the *only* call site to the `WinMain`-shaped
  function anywhere in `___tmainCRTStartup`, sitting immediately after
  `__cinit(1)` runs all 35 now-confirmed static initializers and
  immediately before `WinMain` is called — exactly the semantic position
  both this patch's own README ("shortly after the existing console
  commands are initialized") and its K1 sibling hook's README ("in the
  EXE entry point, shortly after all other static structures are
  initialized, but before the main program begins") describe. The
  earlier dismissal correctly noted the instruction's own meaning has
  nothing to do with consoles — but that was never the right test; the
  hook only needs to be the right *injection point*, and this is it.
  **GOG and Steam are byte-identical here too** (`68 00 00 40 00`, same
  5 bytes, same address role, just a different absolute address —
  `0x0091d52d` on Steam vs. `0x00925133` on GOG) — consistent with every
  other hook ported in this project. Full trail in the `ghidra_knowledge`
  DB's `lessons` table (id 9) and `hooks_ported` table
  (`additional-console-commands-hook1`).
**Both hooks built, isolated-verified, and LIVE-INSTALLED (2026-09-29,
follow-on session) — DONE.** Written into `kotor2_steam_aspyr.hooks.toml`
(both pristine and LAA hashes), `.kpatch` built via `create-patch.py`
(real MinGW cross-compile of `ConsoleCommands.cpp`), isolated
`patch_config.toml` verified byte-exact for both hooks, byte-diff-cluster
proximity check clear for both (nearest cluster ~429KB from hook1,
>5.1MB from hook2). Installed live as the 13th patch alongside the
existing 12 (full patch-ID list). Steam log confirmed both exact
expected lines (`Applied DETOUR hook at 0x0091D52D ->
InitializeAdditionalCommands`, `Applied SIMPLE hook at 0x004752D8 (7
bytes replaced)`) + `SUCCESS: Patcher initialized`; `AR_ERROR.LOG`
unchanged from baseline; process healthy (no hang). **User confirmed
general stability 2026-09-29**: game started successfully, no issues
running around and going through menus — no regressions to the other 12
live patches. This is a general playtest, not yet a *targeted* test of
this patch's own commands (`runscript`, `teleport`, `walkmeshrender`,
etc.) or console-history persistence — those remain open.

**hook2 (console-history-save flag, GOG `0x00867248`, simple, 7 bytes
`0F B6 15 D0 98 A2 00` = `movzx edx, byte ptr [0x00A298D0]`) — RESOLVED.**
No GOG copy exists on this machine, so GOG-side cross-referencing wasn't
an option. The two methods tried in the prior session (Defined Strings
for "hist": zero hits; raw structural search for `0F B6 15`: 311 hits
across ~190 functions, too generic) were refined rather than abandoned:
1. Decoded each of the 311 hits' actual `imm32` operand and kept only
   ones targeting `.data` (a mutable flag almost never lives in
   `.rdata`/`.text`) — narrowed to 65.
2. Filtered those 65 to addresses referenced from **exactly one**
   function (a single boolean guard, unlike globals checked pervasively
   from many unrelated places) — narrowed to 9.
3. Decompiling those 9 immediately surfaced the answer: `FUN_004752a0`
   is the console command parser itself (trims input, has a
   duplicate-check history-append block, looks up the command by name,
   prints `"Function %s not found"` on miss, dispatches to the handler)
   — and it reads `DAT_00a30828` right before that history-append block
   via `movzx edx, byte ptr [0x00a30828]; test edx,edx; jz <skip>`,
   textbook match for GOG's own "Set Console History to save" comment.

Raw bytes at `0x004752d8` confirmed byte-for-byte opcode-identical to
GOG (`0F B6 15` prefix), only the embedded address differs (Steam
`0x00a30828` vs GOG `0x00A298D0`) — same situation as
`K2AspyrLoadingScreenLineFix`'s framebuffer-height global, not a real
codegen divergence. This 3-step refinement (decode+filter-by-section,
then filter-by-reference-uniqueness, then decompile the short survivor
list) is now logged as a reusable general technique in the
`ghidra_knowledge` DB's `lessons` table (`general Ghidra methodology`
scope) for any future overly-broad structural search.

### 6. `Movie Patch` — newly found, moderate-to-hard, higher payoff
- **Found 2026-09-28 (later session)**, same manifest-grep correction —
  missed because its GOG hook file is `kotor2_hooks.toml` (a third naming
  convention, distinct from both `Post-Combat Movement Fix`'s and the
  original 4 candidates').
- 2 `detour` hooks (`blitMovieMitchellNetravaliKotor2` at `0x00920AF1`,
  32-byte anchor; `releaseMovieFilterKotor2` at `0x0091FD10`, 9-byte
  anchor) **plus real C++** — `better_movie_playback.cpp` (29KB),
  `mitchell_netravali_filter.cpp` (image-resampling math), `export.cpp`.
  Meaningfully bigger than any candidate ported so far: not just
  address-porting, this needs the same "does the Steam build's C++
  assumptions (stack offsets, calling convention) still hold" diligence
  as `K2AspyrMapAspectFix`, but across 2 hook sites instead of 3, with a
  nontrivial compiled filter rather than pure pointer arithmetic — closer
  in shape to `Movie Patch`'s own domain than to the simple byte-patch
  candidates.
- Real payoff if it works: bicubic-filtered movie playback instead of
  whatever KOTOR2 does natively, aspect-ratio-correct in the configured
  game window — plausibly relevant to the same ultrawide-correctness
  thread as `K2AspyrMapAspectFix`, worth checking once ported whether
  movies already look right or are stretched/cropped on this ultrawide
  setup (not yet checked either way).
- **Recommendation: after `AdditionalConsoleCommands`, before
  `ExpandedKeyboardControl`** — more complex than either `Post-Combat
  Movement Fix` or `AdditionalConsoleCommands`, but likely less risky
  than `ExpandedKeyboardControl`'s 6-hook HUD/input surface.

**Revised priority order (2026-09-28, later session) for the 4 remaining
candidates**: `Post-Combat Movement Fix` (5) → `AdditionalConsoleCommands`
(3) → `Movie Patch` (6) → `ExpandedKeyboardControl` (4). Not yet started
on any of these three — this is a survey/documentation update only, no
Ghidra work done, per [[feedback-phase-sequencing]] (current phase,
`K2AspyrMapAspectFix`, is still mid-flight pending user in-game
verification; do not overlap a new build/Ghidra phase with it).

## Track B — general engine bug fixes surveyed, not applicable to this K2 Windows install

While re-auditing all 60 `Patches/*/` manifests for the correction above,
also inventoried every patch whose description reads like a real
engine-level bug fix (crash/leak/rendering-correctness, not a
feature/preference toggle), to answer "what bug-fix patches exist that we
haven't prioritized." **None of these are installable candidates for our
setup right now** — each targets a version we don't run:
- **`GrassMemorySafety`** (`grass-memory-safety`) — fixes a double-free in
  `CreateArrays`/`DestroyGrassPolys`/`~CAurTriangleBin` (two fields alias
  the same allocation, both get freed). **KOTOR1-only**
  (`kotor1_gog_103`/`kotor1_cdcrack_103`).
- **`GrassRenderFix`** (`grass-render-fix`) — fixes grass rendering as
  stretched geometry across the sky (`RenderGrassPolys` passes a
  zero-valued field to `glVertexPointer` instead of the real vertex
  buffer on the no-bump-shader path) + restores leaked GL client-state.
  **KOTOR1-only.** Not the same bug as our already-installed
  `k2_grass_fog_fix` (that's a K2 color-angle issue, this is a K1
  geometry issue) — no overlap, no relevance to K2.
- **`SaveGameMemoryLeak`** (`save_mem_leak`) — fixes a leak on every save.
  **KOTOR1-only** (`kotor1_gog_103`/`kotor1_steam_103`/`kotor1_cdcrack_103`).
- **`TextureBucketSafety`** (`texture-bucket-safety`) — bounds-checks 3
  texture-bucket arrays, fixes 2 named crash addresses
  (`ClearBuckets`/`AddPartToMeshBuckets`). **KOTOR1-only.**
- **`CubeMapFix`**, **`SoftShadowsFix`**, **`FrameLimiter`**,
  **`Alt Tab Patch`** — all **KOTOR1-only**, all real fixes (Intel
  reflection detection, non-NVIDIA soft shadows, frame cap, clean
  alt-tab), none apply to K2.
- **`K2AspyrControllerFixes`**, **`K2AspyrWindowingFixes`** — genuinely
  K2, but target `kotor2_steam_aspyr_linux` specifically (Aspyr's
  **native Linux** build) — a different binary than the Windows
  `swkotor2.exe` we run under Proton. Not applicable to this install;
  would only matter if we ever switched from Proton to the native Linux
  Aspyr build, not currently planned.
- **`ShaderDebug`** — GOG-only K2 dev/debug tool (ARB shader hash-dumps +
  in-game overlay), not a bug fix and not really a "port this" candidate
  in the same sense — a diagnostic tool, potentially useful *if* a future
  Ghidra/shader session needs it, not prioritized for its own sake.

**Bottom line on Track B**: the "general bug fixes we haven't
prioritized" instinct was right that they exist, but every real one found
in this repo is scoped to KOTOR1 or the Linux-native K2 build, neither of
which this install runs. There is no free "just port this crash fix"
candidate sitting unaddressed for our actual K2 Windows/Proton setup
beyond the GOG→Steam list above. Finding a *new*, K2-Steam-Windows-
specific bug fix that doesn't already exist as a shipped patch would be
from-scratch Ghidra research (same category of effort as the two custom
mods, `01_inventory_3d_viewport.md`/`02_buff_duration_hud.md`), not a
porting task — a real option, just a different, bigger kind of project
than "port the next candidate," and not started here.

## Generalized port procedure (per candidate)

1. Read the GOG `hooks.toml`'s comments and hook types/parameters to
   understand *what* each hook does before touching Ghidra — the comments
   in these files are unusually good (see `K2AspyrVisibleConsole`'s and
   `AdditionalConsoleCommands`'s for the standard this repo's authors hold).
2. Open the Steam Aspyr `swkotor2.exe` in Ghidra (community GZF/SARIF
   database check first, same as every other Ghidra step in this project —
   don't reverse from scratch if coverage already exists).
3. Use the GOG hook's `original_bytes` sequence *and* its surrounding
   disassembly context as the search anchor in the Steam binary — GOG and
   Steam Aspyr share the same underlying codebase (confirmed: both are
   tagged "aspyr", per our conversation on version landetting — just
   different DRM/Steamworks glue), so the target function should still
   exist, just at a different address, with the same bytes (confirmed
   byte-for-byte identical at every hook site checked across 2 candidates
   / 4 hooks so far).
   - **Use `find(Address, byte[])` in a manual loop for the byte search,
     NOT `findBytes(Address, String, int)`** — the latter takes a Java
     regex, and plain hex like `"c7 85 7c ff ff ff"` gets silently
     interpreted as literal ASCII text, producing a confident-looking but
     meaningless 0-hit result every time, regardless of whether the bytes
     exist. This cost real time across two different candidates this
     session before being caught (see the `ghidra_knowledge` DB's
     `lessons` table). If wildcard matching is genuinely needed,
     `findBytes()` works fine *with proper `\xNN` escaping* for literal
     bytes.
   - **An exact byte match is a candidate list, not an answer** — the same
     short sequence commonly recurs elsewhere in a 6+ MB binary (as many
     as 76 times, observed on `K2AspyrMapAspectFix`'s hook 1 pattern).
     Confirm the right hit via surrounding context: what's computed right
     before/after the candidate address, and whether the struct/stack
     offsets the mod's C++ hardcodes actually appear nearby, matching
     GOG's values. Decompiled-string cross-referencing (Defined Strings →
     xrefs → the containing function's decompiled C) remains the reliable
     fallback when the byte search returns too many candidates to eyeball,
     or when a real codegen difference means the exact bytes genuinely
     don't appear (confirm this by checking with a byte-confirmed-present
     pattern first, per the `DebugFindBytes.java` diagnostic approach,
     before concluding the bytes are actually absent).
4. Confirm the found sequence is long enough and matches the hook type's
   requirement (`simple` needs identical original/replacement length;
   `detour` needs ≥5 bytes at the jump point).
5. Write the new `kotor2-steam-aspyr.hooks.toml`, mirroring the GOG file's
   hook count, types, comments, and parameter sources — only the
   `address`/`original_bytes`/`replacement_bytes` values change.
6. Add the Steam hash to `manifest.toml`.
7. Rebuild via `create-patch.py` (no-op recompile for
   `K2AspyrLoadingScreenLineFix` — zero C++). **Corrected methodology**:
   this framework patches the running process's memory via
   `KotorPatcher.dll` injected through a hijacked `binkw32.dll` — it does
   **not** statically rewrite the exe on disk (confirmed concretely on
   candidate #1: an isolated-copy install left the copied exe's hash
   completely unchanged despite the launcher reporting success). So an
   isolated-copy *file-diff* proves nothing. What an isolated copy **can**
   still verify cheaply: the written `patch_config.toml`'s hook
   byte arrays match what was intended (parses the `.kpatch` correctly).
   The real proof the hook applies is the steam log's `[KotorPatcher]
   Applied ... hook at 0x...` line, which only appears once the patch
   actually runs against the live exe.
8. Install on the live game directly (same LAA-hash two-part recipe as
   any other patch, `05_build_plan.md`'s Phase 1a section) and check the
   steam log for the `Applied ... hook` + `SUCCESS: Patcher initialized`
   lines as the actual confirmation, plus `AR_ERROR.LOG` against baseline.
   There is no safe way to get a stronger pre-live signal than the
   `patch_config.toml` byte check in step 7 for this framework's
   architecture — budget for reverting via the pre-install backup if the
   live steam log shows a problem, rather than expecting to catch it
   beforehand.

## Open items before starting

- **LAA-hash mismatch — RESOLVED as of Phase 1a/1b (2026-09-28), recipe
  proven twice.** No longer a blocker requiring a decision; it's now a
  known, repeatable two-part mechanical step for *any* patch we want on
  the live exe: (1) add `kotor2_steam_aspyr_laa = "<live hash>"` to the
  patch's `manifest.toml` and to its `kotor2-steam-aspyr.hooks.toml`'s
  `target_versions`, (2) clone the matching `AddressDatabases/*.db` and
  retarget its `sha256_hash` row to the same hash (only needed if the
  patch ships a DLL — `detour`/some `simple` hooks that resolve exported
  functions at runtime). Full worked example: `05_build_plan.md`'s Phase
  1a section. Applies the same way once a GOG-ported patch has a working
  `kotor2-steam-aspyr.hooks.toml` — no new unknowns here.
- **Ghidra environment — RESOLVED (Phase 2, 2026-09-28).** Ghidra 12.1.4 +
  JDK 21 installed, project at
  `~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects/KOTOR2_SteamAspyr`
  (imported + auto-analyzed `backups/vanilla_clean/swkotor2.exe`). No
  longer a blocker for candidates #2-4. See `05_build_plan.md`'s "Phase 2"
  outcome section for setup details, and this doc's candidate #1 entry
  above for the actual working method (headless `analyzeHeadless` +
  custom Ghidra scripts, not the GUI — no `xdotool`/`wmctrl` on this
  machine to drive the Swing GUI reliably, and headless scripting proved
  faster and fully reproducible anyway).
- Checked for candidate #1 (no community GZF/SARIF database covers the
  Steam Aspyr build) — see `05_build_plan.md`'s Phase 2 section. Not
  re-checked per-candidate for #2-4; worth a quick look each time in case
  coverage has appeared since, but don't expect it.
- **New from Phase 1b, worth checking before porting `K2AspyrMapAspectFix`
  or `K2AspyrLoadingScreenLineFix` specifically**: the live exe has at
  least 2 confirmed and 2 more suspected hand-patches sitting in `.rdata`/
  `.data` shader regions (see `05_build_plan.md`'s Phase 1b writeup and
  the `kotor2-patch-manager` memory for exact addresses/clusters). Before
  writing a new `kotor2-steam-aspyr.hooks.toml` for any candidate here,
  re-run the same byte-diff-cluster proximity check against those regions
  (correctly, through the real PE section table — see the methodology
  note in Phase 1b, a flat `VA - 0x400000` conversion is wrong for
  anything outside `.text`) once the new hook addresses are known from
  Ghidra, the same way Phase 1b screened the 19 existing candidates.
