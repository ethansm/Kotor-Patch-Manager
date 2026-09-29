# Framework Research: Kotor Patch Manager

Research date: 2026-09-28. Source: [github.com/LaneDibello/Kotor-Patch-Manager](https://github.com/LaneDibello/Kotor-Patch-Manager)
(repo README, `/Patches` tree via `gh api`), DeadlyStream.

## What it is

A runtime DLL-injection patching framework for KOTOR I & II — **no
permanent exe modification**; patches toggle on/off per launch, unlike our
existing TSLPatcher/GFF pipeline which writes files to disk once.
Three-part architecture:

- **`KotorPatcher.dll`** (C++) — injected into the running game, applies
  the actual hooks.
- **`KPatchCore`** (C#) — parses `.kpatch`/`manifest.toml`, checks
  `requires`/`conflicts` declarations, builds `patch_config.toml`.
- **`KPatchLauncher`** (C#/Avalonia GUI) — patch selection + game launch.

Four hook types: `simple` (byte replace), `replace` (allocate + jump),
`detour` (jump to a wrapper calling an external function, with
parameter/register handling — the type both our mod ideas need), and
`static` (applied to the exe file at install time — e.g. PE header flags,
like the LAA bit).

`manifest.toml` declares `requires`/`conflicts` per patch plus per-build
SHA-256 checksums under `supported_versions` — i.e. **the framework itself
has a version-pinning story**, so a `.kpatch` built against the wrong K2
build should refuse to apply rather than silently corrupt behavior.

## Maturity

Repo created 2025-10-16, **still being actively pushed as of today
(2026-09-28)**. ~540 commits, MIT license, 25 stars / 9 forks / **58 open
issues**. Self-described beta, expected instability. Treat as we'd treat
any pre-1.0 tool in our pipeline: verify every claim on disk before
trusting it live, per [[feedback-calibrate-detectors]].

## Build toolchain

- **Windows**: Visual Studio 2022 + .NET.
- **Linux (our environment)**: .NET 8 SDK, MinGW-w64 (i686 32-bit
  cross-compiler — the game is a 32-bit PE target), Python 3, 32-bit dev
  libs. Build via `python3 create-patch.py` (vs. `create-patch.bat` on
  Windows). **Cross-compiling a new patch entirely from Linux is
  documented and supported** — this removes what would otherwise be the
  single biggest blocker for us.

## Linux/Proton compatibility

Two distinct paths:

1. **Windows build under Proton (what our install runs today).** Native
   Wine/Proton processes can't self-inject the usual way, so the
   framework stages **proxy DLLs** (`binkw32.dll`, `sqlite3.dll` —
   game-loaded DLLs it can hijack) to get `KotorPatcher.dll` loaded. This
   is the path we'd actually use.
2. **Native Aspyr Linux build via `KotorPatcher.so`** — K2 has an official
   native Linux port (K1 does not; K1 is Proton-only). No Proton needed at
   all on this path. We are *not* currently running this build (memory
   confirms Proton + Wine per [[kotor2-ultrawide-gui]]), but it's worth
   knowing this exists as a fallback if the Proton proxy-DLL path proves
   unreliable — switching builds is a bigger decision than this research
   phase should make unilaterally.

No direct Proton compatibility testing has been done by us yet — this is
the first thing to prove in `05_build_plan.md` Phase 0, same pattern as
[[kotor2-script-compiler]]'s nwnnsscomp-under-Wine lesson: a toolchain that
"should just work" needs to be proven byte-for-byte, not assumed.

## PartySwap — already installed, but NOT built on this framework

Mod #97 in our build (`neocities_mods.json`, deadlystream.com/files/
file/544-partyswap) is a **separate, older TSLPatcher-based mod** — no
relation to Patch Manager. Patch Manager ships its **own**, narrower
`FullPartySelect` patch (lets you keep Handmaiden after Disciple joins) as
a `.kpatch` plus a hand-copied `partyselect_p.gui` into Override.

**This matters for us specifically**: `partyselect_p` is one of the four
3D-view GUIs our ultrawide pass already fixed (Phase 4, per
[[kotor2-ultrawide-gui]]: "3D views (classsel_p/maincg_p/partyselect_p/
upgrade_p)"). If `FullPartySelect` is ever installed, its hand-copied
`partyselect_p.gui` would **overwrite our ultrawide fix** for that screen
unless merged manually. See `04_recommended_installs.md` for the verdict.

## Other existing Patch-Manager-based mods found

- **Framework's own `/Patches` tree** (~65 patches, mostly engine
  bugfixes/QoL, not new content): `K2AspyrColorCorrection`,
  `K2AspyrFogReflectionFix`, `K2AspyrShaderFixes`, `K2AspyrMapAspectFix`,
  `K2AspyrWindowingFixes`, `K2AspyrLoadingScreenLineFix`,
  `K2AspyrMusicFix`, `BorderlessFullscreen`, `FrameLimiter`,
  `SaveGameMemoryLeak`, `GrassMemorySafety`, `SoftShadowsFix`,
  `4GBPatch`, `AdditionalConsoleCommands`, `ScriptExtender` (adds new
  NWScript commands — worth keeping in mind as an alternative to a full
  C++ hook for *some* future mod idea, though not for either of ours,
  which both need per-frame render hooks NWScript can't do).
- **`K2AspyrMapAspectFix`** is worth flagging: [[kotor2-ultrawide-gui]]
  logs an open, unresolved item — "the area map can't be fixed via
  LBL_Map (map_p excluded)" — from our own GUI-layer approach. This patch
  may fix exactly that gap from the engine side instead. Needs on-disk
  verification of what it actually changes before assuming it solves our
  open item (per [[feedback-calibrate-detectors]] — don't take the name
  at face value).
- **Community WIP: "K2 Active Party Extension"** (deadlystream.com/topic/
  12201) — expands the active party to 5 members. Reported "so far so
  good" alongside PartySwap, but **still missing level-up icons/portraits
  for slots 4-5** — explicitly unfinished, not production-ready.

## Compatibility warnings for this install

- **`4GBPatch` (Patch Manager's own) directly overlaps with mod #4 (4GB
  Patcher, ntcore.com), already applied.** Do not install Patch Manager's
  copy — same static LAA-flag patch, redundant at best, corrupting at
  worst if both try to flip the same PE header bit independently.
- No evidence of conflicts with TSLPatcher-based mods in general —
  different modification layers (TSLPatcher writes 2DA/GFF/override files
  at install time; Patch Manager patches the running process at launch
  time) — but **every `.kpatch`'s own `conflicts`/`requires` list must
  still be checked per-mod**, not assumed safe from the framework-level
  story alone.
- No ReShade/dinput8-hook conflict data found in the README or issues.
  **Unverified, not confirmed-safe** — flag for direct testing if we ever
  use such a tool alongside this.

Related: [[kotor2-ultrawide-gui]], [[kotor2-script-compiler]],
[[feedback-calibrate-detectors]].
