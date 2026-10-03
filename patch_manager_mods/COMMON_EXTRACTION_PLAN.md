# Extracting shared mod logic into `Patches/Common`

Status 2026-10-03. Survey of all 20 `patch_manager_mods/*_src` dirs (about 14k lines). User decision (2026-10-02): extract common logic into
the framework's `Patches/Common` for local development and testing now. Each upstream PR then carries only the Common pieces it needs.

## Workflow

- **Canonical copy.** The framework clone `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`, folder
  `Patches/Common/` (precedent: aad0a14 moved `GuiKit.h`/`SafeRead.h` there; `Patches/_shared/` keeps forwarding headers). This repo mirrors
  it in `patch_manager_mods/_shared_src/` for history and review. Commit both, and never let them drift: diff before every build.
- **Header-only, no hooks.** Common is a static library compiled into every patch DLL, so it holds helpers only. Hooks stay in the patches.
  Shared hook points need a framework feature (see doc 07, "shared save binding").
- **One mod per change.** Each migration is behaviour-neutral, verified with:
  - a warning-clean build;
  - host tests;
  - a log diff, plus a screenshot for visual mods;
  - one install and live check.
- **Who migrates which mod.**
  - The session that owns a mod migrates it.
  - Quiet mods (untouched since the 10-02 checkout) are safe first adopters.
  - Never edit a mod another session changed in the last few hours: check `ls -lt` / `git log -1 -- <dir>`.
- **PRs.** A PR branch cherry-picks the Common headers its patch includes, at the version it was tested with. The headers' first upstream PR
  explains them, and later PRs only extend them.
- **Addresses.** Keep Steam constants until the address DB rows exist (Steam DB PR3, `addressdb/k2-steam-acc`). Then migrate to
  `GameAPI/GameVersion` lookups.

## Ranked extraction list

| # | Header | API sketch | Adopters (copies today) | Risk / notes |
|---|---|---|---|---|
| 1 | `Common/Log.h` | `Log::open(path, cap, rotate)`, `Log::line(fmt, ...)`, `Log::verbose(...)`, `Log::setFrame(n)` | all 16 mods (`logf` hand-copied, caps 5000-60000, stamps differ) | Low. Keep per-line reopen and each mod's cap and stamp format. Rotation per session (the hotbar's `.1.txt` rule). |
| 2 | `Common/GameAddr.h` | one canonical name per engine address (doc table) | 19 addresses defined in 2-8 mods each, e.g. 0xA1B4A4 under 6 names | Low. No renames inside the mods yet. Name collision: `FnSetAlign` is 0x416FA0 (hotbar) vs 0x85D430 (equip3d). No value mismatches found. |
| 3 | `Common/SafeRead.h` adoption + `readExo` | `saferead::readExo(site, addr, maxLen)`, thin `rdInt` | hotbar, equip3d, inv3d still use IsBadReadPtr copies; buff and effects partial | Low. Unique site ids per mod. Avoids the Wine IsBadReadPtr page-fault storm (lesson 234). |
| 4 | `Common/IniFile.h` | `trim`, `parseFloats`, `stamp(path)`, `reloadDue(&stamp, path)`, `each(path, cb)`; the hotbar's `.bak`/warnings Save path | effects, equip3d, inv3d (near identical), sort_filter, ranged_ai, companion_ai, level_up | Low-medium. Keep each mod's key syntax. |
| 5 | `Common/WindowHook.h` | `gameWindow()`, `installWheel(rectFn, cb)`, `restoreIfOnTop()`, cursor helpers | effects, inv3d, equip3d, hotbar (4 near-identical wheel subclasses) | Medium. The chain order matters when two mods subclass one HWND. |
| 6 | `Common/CodePatch.h` | `writeCode(at, expect, repl, n)` with original-byte check, state codes | activation_fix, equip_icon_fix, robustness_fixes, perf_lab | Medium. Verifying the original bytes is mandatory. |
| 7 | `Common/GuiKit.h` adoption (+ `showTip`, `inRect`) | existing GuiKit API | hotbar, effects, equip3d, sort_filter, inv3d (own copies; only level_up uses GuiKit) | Medium-high: live visual mods. Gate each with screenshots. |
| 8 | `Common/GameState.h` | `gameplayHudActive()`, `controlledCreature()`, `partyMember(i)` | hotbar, companion_ai, buff, effects | Medium. Depends on #2. |
| 9 | `Common/SaveBinding.h` | per-save `onSave/onLoad/onNewGame` + `playthroughId()` | hotbar (per-save files), level_up (GUID globals) | High: the live save path. Also needs framework save events. |
| 10 | `Common/Textures.h` | `registerRgba(name, w, h, px)`, `heal()` | hotbar only | Defer until a second user. |
| 11 | `Common/TestHarness.h` | fake engine and fake memory | perf_lab harness, 5 pure-logic tests | Design first. |
| - | file readers `TwoDA.h`, `Tlk.h` | as in the hotbar | hotbar (level_up has its own table parser) | Move with #4 if level_up adopts them. |

## Order

1. `Log.h` + `GameAddr.h`. First adopters are the quiet small mods: EquipIconFix, ActivationFix, TextureMemoryRelease, RangedAI, MouseProbe,
   LoadHangProbe.
2. `SafeRead.h` adoption: equip3d, inv3d, Buff.
3. `IniFile.h`: equip3d and inv3d first.
4. `WindowHook.h`: equip3d, inv3d, effects.
5. `CodePatch.h`: the three byte-patch mods.
6. `GuiKit.h` adoption and `GameState.h`, one visual mod at a time.
7. Later: `SaveBinding.h`, `Textures.h`, the test harness.

**Hotbar (Mod 3).** It adopts SafeRead, Log, IniFile, WindowHook and GuiKit during its 1.0.0 Phase T (tests/refactor), instead of a private
`HotbarLogic.h` for the generic parts. Its 1.0.0 PR then carries those headers. It is the most active mod, so it goes after the first two
headers exist.

## Address duplication (for `GameAddr.h`)

| Address | Mods | Names today |
|---|---|---|
| 0x00A1B4A4 | 8 | AppGlobal, G_App, GuiGlobal, ItemMgrGlobal, SaveRequestHolder, WorldTimerSingletonGlobal |
| 0x00733780 / 0x00733570 | 7 / 6 | CExoString dtor / ctor |
| 0x00416E30, 0x009876B4 | 5 | CSWGuiText::Set, text vtable |
| 0x0040F620, 0x00919723, 0x00419740, 0x009878BC, 0x0047EB60, 0x009875BC | 5 | InitControl, engine new, label ctor/vtable, LoadImage, border vtable |
| 0x007E5DA0, 0x0077D800 | 3 | party member / controlled creature accessors |
| 0x00A1B4E0, 0x009F42B4, 0x0098C45C, 0x0098B5CC, 0x009A3D34, 0x0099EE14 | 2 | flags, GateGlobal, scene vtables, creature vtable |

The two-mod pairs also include the equip3d/inv3d scene functions, 0x73FB90, 0x5453C0, 0x51AD20, 0x9A5C84 and 0x4261CF/D6/E6.
