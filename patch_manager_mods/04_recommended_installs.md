# Recommended Existing Patch Manager Mods

Based on `03_framework_research.md`. None of these are installed yet.
**Recommendation overall: pilot on a test/backup profile first** — this
build has never run Patch Manager before, and the framework is beta with
58 open issues. Consistent with [[feedback-deploy-debug-live]] (safe
rollback preferred over waiting, but the choice is still the user's) and
[[feedback-phase-sequencing]] (verify, commit, then move on) — don't
bundle these with the two custom mods' own testing.

## Safe to install (low risk, no known conflicts)

Pure engine bugfix/QoL patches, no GUI file overwrites, no overlap with
anything in our 154-mod build:

- `K2AspyrFogReflectionFix`
- `K2AspyrShaderFixes`
- `K2AspyrWindowingFixes`
- `K2AspyrLoadingScreenLineFix`
- `K2AspyrMusicFix`
- `BorderlessFullscreen`
- `FrameLimiter`
- `SaveGameMemoryLeak`
- `GrassMemorySafety`
- `SoftShadowsFix`
- `AdditionalConsoleCommands`
- `ScriptExtender`

Good candidates for Phase 1 of the build plan (`05_build_plan.md`) —
installing 2-3 of these first validates the launcher/injection workflow
end-to-end under our Proton setup before we build anything custom.

## Verify before installing (name suggests a fix, but confirm on disk)

- **`K2AspyrColorCorrection`** — could visibly change color grading across
  every texture we've upscaled/retoned in the ultrawide project
  ([[kotor2-ultrawide-gui]]'s `tone_fixed` work assumed vanilla color
  behavior). Compare a screenshot before/after rather than assuming
  compatibility.
- **`K2AspyrMapAspectFix`** — may resolve the open "area map can't be
  fixed via LBL_Map" item logged in [[kotor2-ultrawide-gui]]. Worth
  testing specifically *for* that purpose, but diff what it actually
  patches first per [[feedback-calibrate-detectors]] rather than
  installing on the strength of the name.

## Do not install

- **Patch Manager's own `4GBPatch`.** Directly redundant with mod #4 (4GB
  Patcher) already applied to this exe. Installing both risks two
  independent LAA-flag writes to the same PE header field.

## Install with caution / needs a plan first

- **`FullPartySelect`.** Hand-copies `partyselect_p.gui` into Override,
  which would clobber our ultrawide fix for that exact screen (one of the
  four Phase-4 3D-view GUIs). Also: mod #97 (PartySwap, already installed)
  may already solve a related "keep more party members" problem via a
  different, older mechanism — **check for functional overlap before
  installing**, and if we do install it, merge its `partyselect_p.gui`
  changes into our ultrawide-fixed version by hand rather than letting it
  overwrite ours.

## Not recommended yet

- **"K2 Active Party Extension"** (community WIP, deadlystream.com/topic/
  12201). Explicitly unfinished — missing level-up icons/portraits for
  party slots 4-5. Revisit once the author reports it feature-complete;
  installing a known-incomplete UI mod now just creates a support burden
  matching the [[kotor2-crash-triage]] pattern we've already dealt with
  once this build (the pykotor capsule-case bug, the arg-order compiler
  bug) — new, less-mature tooling is exactly where those surfaced.
