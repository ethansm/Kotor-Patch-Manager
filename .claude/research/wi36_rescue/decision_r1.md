# WI-36 rescue round 1 — decision (2026-10-27)

Outcome: SPLIT. WI-36 is superseded by:
- WI-163 (WI-36a, RE): per-frame tick site + prologue bytes for feats (0x909E00) and powers (0x9074E0) panels, a label template tag present in both panel .gui layouts, calling convention of 0x90C230/0x90BD50/0x9098D0/0x909360, chargen-vs-levelup identity of 0x909E00, grid offsets +0x1BF4/+0x1BF8.
- WI-164 (WI-36b, build, blocked on WI-163): ctor + tick hooks in kotor2-steam-aspyr.hooks.toml, Filter button, rebuild on click, play-queue row.

Why: recon u01..u07 (.claude/waves/WI-36-rescue/) — doc 25:162 needs panel GUI RE first; no tick/template/hook bytes; hooks.toml outside WI-36 scope; char-screen tick 0x84F221 stops while a level-up panel is open (LevelUpOverride.cpp:2092-2094); Verify 1 had a placeholder suite.

User decision: 4-mode cycle All > Build (Dim) > Combat > Force; Build (Hide) skipped until batch 2c.

Residual rows:
- Doc 25:82 says CSWGuiFeatSelection; DB says CSWGuiFeatsCharGen::Constructor -> fix in WI-163.
- R10Filter.h:44-46 nextMode still includes BuildHide -> WI-164 uses its own 4-step cycle (or skips BuildHide) and pins it in r10_filter_test.cpp.
