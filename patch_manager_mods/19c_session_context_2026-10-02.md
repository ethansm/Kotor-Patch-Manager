# 19c. Session context: shared logging + RFC #180 registry (2026-10-02)

Read this first when resuming the shared-logging / patch-registry work. Then read doc 19 (design, rev 2), 19b (cross-patch survey), 19a (held RFC comment) and the Phase 1 plan file (below). Everything decided in this session is on disk; nothing lives only in chat.

## Goal (user)
Standardize logging for the **Kotor Patch Manager framework** so mods are compatible and performant. This is an upstream standard, not just our repo. #180's original purpose, patch-to-patch communication, is first-class in the same work.

## Decisions (user-confirmed 10-02)
1. The logging service lives in the **KotorPatcher core** (`KotorPatcher.dll`/`.so`/`.dylib`). It is **not** a separate logging patch or sink DLL; rev 1's per-patch `KpLog.h`-only design and optional `kp-log` DLL are dropped.
2. It rides on **upstream RFC LaneDibello/Kotor-Patch-Manager#180** ("how should patches talk to each other?", @ProjectSynchro, 2026-08-26, OPEN, 0 comments, unimplemented):
   - A plain-C `KPatchApi { struct_size, abi_version, provide(name, ver, iface), require(name, ver) }`, pushed to each patch through an optional export `KPatchInit(const KPatchApi*)`.
   - Logging is the first patcher-provided interface, `"kpatch.log"` v1. **We implement the registry ourselves, in the same PR.**
   - The full registry ships, including patch-to-patch `provide`/`require` with test provider and consumer patches.
3. Decisions 1-5 of doc 19:
   - Per-patch files by default, merged log opt-in.
   - Byte caps with one rotation plus a rate limit; no line caps.
   - Built for upstream from the start.
   - The frame counter lives in the core and is advanced by `FrameTick()` from a patch with a frame hook; `f=-` until then.
   - One flush thread, in the core.
4. **The RFC #180 comment is held.** Draft: `19a_rfc180_comment_DRAFT.md`. Post it only together with the PR containing the implemented registry + logging. Never post upstream without the user's go.
5. **Rollout** (doc 19 section 8a):
   - Registry + `kpatch.log` go upstream first, as their own PR.
   - All other shared code is consolidated on our fork now and goes upstream inside the first mod PR that uses it. No speculative Common PRs.
   - Mod PR order follows the dependencies.
6. Sibling-owned patches migrate only with their owner's agreement. Never edit their sources from this thread.

## Facts established (verified on disk)
- **Fork:** `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`, remotes `origin` = ethansm/Kotor-Patch-Manager and `upstream` = LaneDibello/Kotor-Patch-Manager (push disabled).
- **Core:** `src/KotorPatcher/`, with the platform seam `include/platform.h` (Log, AllocExec, LoadModule, GetSymbol, SetEnv, ...), backends `platform_win32.cpp`/`platform_posix.cpp`, and entries `dllmain.cpp`/`soentry.cpp`/`dylibentry.cpp`.
- **Patcher behaviour:**
  - The patcher has **no intentional exports**. Correction 10-02: with no `.def` and no `dllexport`, MinGW auto-exports every symbol (upstream's DLL already exports 706 internal ones), and the Linux `.so` uses default visibility. Patches must not rely on any of them.
  - It already publishes `KOTOR_VERSION_SHA` through `Platform::SetEnv` (patcher.cpp:122).
  - It may defer the apply to a worker thread under SteamStub (patcher.cpp:137).
- **Patch loading (`ApplyPatch`, patcher.cpp:223-300):**
  - `LoadModule` runs **once per DETOUR hook entry** (a 20-hook DLL is loaded 20 times with the same handle), so `KPatchInit` must be deduped per handle.
  - Modules stay in `g_loadedPatches` until `CleanupPatcher` (patcher.cpp:155-175), including after a failed hook, so interfaces stay valid until shutdown.
  - A patch's `DllMain` runs inside `LoadModule`, before `KPatchInit`; the header needs a pre-init buffer.
- **Linux:** `KotorPatcher.so` is a DT_NEEDED of the game. The POSIX `LoadModule` uses `dlopen(RTLD_NOW|RTLD_LOCAL)`.
- **Upstream `Patches/Common/Common.h`:** `debugLog()`/`debugHexDump()` go to OutputDebugString, and about 29 upstream patches use them. `Common.h` includes `windows.h`.
- **Patch builds:** `Patches/create-patch.py` builds a patch with `-I Patches/Common -I lib`. Common `.cpp` files go into a cached static library; the cache key covers every file under Common.
- **Upstream activity:** maintainers LaneDibello and ProjectSynchro, with recent focused PRs. No upstream issue or PR covers logging except #180.
- **None of our patches exist upstream yet.** Several have untracked `PULL_REQUEST.md` drafts.

## Done this session (commits)
| Commit | Repo | What |
|---|---|---|
| aa64614 | local-setup | Doc 19 rev 2: logging service in the core, aligned with #180 |
| aad0a14 | KPM fork `ethansm/local-mods` | Phase 0: `SafeRead.h`/`GuiKit.h` canonical in `Patches/Common`; `Patches/_shared/*.h` are forwarding headers |
| cd6554e | local-setup | Phase 0: `scripts/check_shared_headers.py` (drift check; `--sync` copies `_shared_src` to the fork Common + rewrites the forwarders) + 19a draft |
| eeaf6dc | local-setup | Doc 19/19a: patch-to-patch rules (once-per-module init, lifetime, duplicates, missing provider, introspection, tests) |
| a1c6d76 | local-setup | Doc 19b: cross-patch sharing survey (registry candidates A1-A6, Common candidates B1-B8, existing coupling) |
| fd0da77 | local-setup | Every mod doc gets a "Planned shared state and shared code" section; doc 19 section 8a rollout |

Phase 0 verification:
- BuffDurationHUD and InventorySortFilter rebuild OK through the forwarders.
- GuiKit syntax-checks with both include styles.
- The drift check detects an injected change, and `--sync` fixes it.

**Not done:** hotbar `IsBadReadPtr` -> SafeRead. It is sibling-owned with uncommitted work, so it is left to the owner.

## Survey highlights (doc 19b)
- **Registry candidates:**
  - **A1 `k2.guiinput`** is the best first real interface: 4 stacked WndProc subclasses in inv3d, equip3d, effects and hotbar.
  - **A2 `k2.charscreen`:** level-up and the effects panel co-tenant the character screen.
  - **A3 `k2.enginestate`:** load-hang is the provider, perf-lab the consumer.
  - **A4 `k2.effects`:** buff HUD and the effects panel.
  - **A5 `k2.rangedai`:** a latent bug where companion AI misses stance 16.
- **Common candidates:**
  - B1: identical icon tables, about 600 lines.
  - B2: GuiKit adoption, about 500 lines.
  - B3: ini helper, about 200 lines; it merges with the `kplog.ini` reloader.
- Because none of these patches are upstream, the #180 PR uses **test** provider/consumer patches. A1 ships on our fork first.

## Uncommitted state left alone (other sessions own it)
- `07_force_power_hotbar.md`, the hotbar/level-up sources, research logs, and many untracked `PULL_REQUEST.md` files.
- `Patches/MouseProbe/`, untracked in the KPM fork.

## Next
**Phase 2 is DONE (10-03).** Plan + execution log: `19e_phase2_plan.md`. Phase 1 (fa67a3b) and Phase 2 (fc11fa3, 706b150) are merged into `ethansm/local-mods`; the live core `36f10b7f` already has `kpatch.log` (installed by the portrait-slots session). Multilib is installed; gate = "GATE PASS (full, multilib)".

Open items, in order:
1. **User test of mouse-probe 0.2.0** (staged, not installed): follow 19e "Step U" (hot-swap DLL + optional `[mouse-probe]` kplog.ini section; rollback listed). Needs the user's go; game closed; never launch it.
2. Read the test log with `scripts/kplog_parse.py --stats`; look for `DROPPED`; update doc 18.
3. Other mods migrate per `19f_kplog_migration_guide.md` (owning sessions; memory `kotor2-kplog-adoption`); track in the 19e adoption table.
4. **Upstream:** the logger PR (`19g_upstream_pr_draft.md` + 19a comment) goes up with the first mod that ships with the logger. Rebase onto upstream/master first. Posting needs the user's go.
5. Untested here: MSVC build of KpLog.h (preprocessor, /W4, `_fsopen`); first Windows/CI build covers it.

(Phase 1 hand-off text kept below for history.)

## Session 2 hand-off (Phase 1 execution, 10-02 13:50 to 10-03)
- Phase 1 is fully executed. Branch state, commits, gate and decisions are in `19d_phase1_plan.md` "Execution log". The gate is `scripts/kpm_registry_gate.sh`; it writes its logs to `/tmp/kpm_registry_gate`.
- Nothing is pending from Phase 1 except "Next" items 1-5 above. All of them wait on the user: multilib install, live smoke go, merge, Phase 2.
- **Open user report, unresolved (10-02 ~14:55):** the user wrote only "it is very dark now".
  - I checked the game dir (`~/.local/share/Steam/steamapps/common/Knights of the Old Republic II`, maxdepth 2): no files were modified in the 3 h before 14:55. This session installed nothing.
  - I asked whether it means the game (whole screen or areas/menus, since when, screenshot) or something else. No answer yet.
  - Candidates if it is the game: other sessions' 10-02 installs (shader fixes on the vanilla exe at 12:02, colorshift_k2 at 12:14, robustness 1.1.0 + texture-memory at 12:50) and ReShade. Use the `kotor2-crash-triage` skill and the per-mod memories. Don't touch live files mid-session.
- The scratchpad files from that session (smoke driver, export-diff lists) are disposable. The findings they produced are recorded in 19d.
