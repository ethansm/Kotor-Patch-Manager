# Session Handoff & Plan State: patch_manager_mods/41_custom_voice_and_events.md

## 0. Next-Phase Direction
Execute V-C2 (shrunken): R1 recon of GetKeyEntry/ServiceFromImage signatures for the doc 44 VFS session, voice-probe 0.1.1 PlaySound hook install prep (kplog.ini section), and a trigger design that writes no loose game files. ON HOLD until the doc 44 VFS lands; H1 GetKeyEntry observer belongs to the VFS session.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/41_custom_voice_and_events.md`
- **Waves:** .claude/waves/WI-41-a1/
- **Waves:** .claude/waves/WI-41-a2/
- **Waves:** .claude/waves/WI-41-b1/
- **Waves:** .claude/waves/WI-41-b2/
- **Waves:** .claude/waves/w11-41-VB/
- **Branch:** `KOTOR-II` (5e59d44 docs(33): QL12-QL14, QL16 INSTALLED (LIVE 10-17 01:30) + play rows MODE1, CST1)
- **Generated:** 2026-10-17 01:36:23 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
5e59d44 docs(33): QL12-QL14, QL16 INSTALLED (LIVE 10-17 01:30) + play rows MODE1, CST1
 .../handoffs/33_ui_queue_hotbar_fixes.handoff.md   | 46 ++++++++++------------
 patch_manager_mods/33_ui_queue_hotbar_fixes.md     |  8 ++--
 patch_manager_mods/PLAY_TEST_QUEUE.md              |  2 +
 3 files changed, 26 insertions(+), 30 deletions(-)
```
- **Dirty Files (182):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M CLAUDE.md`
  - ... +172 more (run `git status --short`)

## 2. Immediate Objective (V-C)
**Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)**

Status: PARTIAL
Task: Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)

## 2b. Upcoming Batches in Pipeline
- **V-C2** : Native logging + a working trigger for the live voice probe: plan `patch_manager_mods/kvoice/V-C2_HOOK_LOGGING_PLAN.md` (sections 9-10). E1 PASS and E2a/b/c done 2026-10-15/16 via the retired heartbeat trigger. H1 (GetKeyEntry observer) moved to the doc 44 VFS session; remaining here: R1 recon, H2 PlaySound hook (voice-probe 0.1.1 on KpLog), trigger design. ON HOLD until the VFS lands
- **V-D** : Silent-commoner SSF fix (needs T-SSF) and murmur bed prototype (ideas 1-2)
- **V-E** : Voice spike: Qwen3 VoiceDesign speed/VRAM, 200-line consistency, FX chain, RVC timing on the 5070, Day-5 sound-alike screen (`w3/b5.md` 5-day plan)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/41_custom_voice_and_events.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate V-C` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
### Provenance fixes
- The auto-generated "Waves: WI-41-*" lines in section 1 are unrelated dev-work-item waves (substring match on "41"). The real evidence lives in `.claude/waves/w10-voice/` (gitignored): `COMMON_BRIEF.md`, `W1_FACTS.md`, `W2_FACTS.md` (digested facts incl. wave-3 corrections), `BRAINSTORM_BRIEF.md`, `CHECK_BRIEF.md`, `tmp/b9/` (kvoice prototype `kvoice_proto.py`, `kvoice_proto_test.py`, `kvoice_ledger_proto.py`, `roundtrip.json`), `tmp/u11/lipcheck.py` (reference `.lip` parser/writer), `tmp/apply41.py`. All durable findings are committed in `.claude/research/voice_audio/{w1,w2,w3,w4,catalog}/` (commit 139639c). Start with doc 41 sections 2, 3, 6, 7; the kvoice spec is `.claude/research/voice_audio/w3/b9.md`.

### Dead ends and anti-patterns
- Do NOT trust Gemini's `~/kotor-custom-audio.pdf`: scorecard in doc 41 section 1 (470-byte VO header, `.lip` table, SSF format, NoZoomIn, watermarks all wrong).
- Do NOT use `ffmpeg -i` durations on StreamSounds files (7.4x too long: 470-byte MP3 prefix + RIFF PCM); decode with the stream decoder / strip prefix. VO: `tail -c +59 f.wav | ffmpeg -f mp3 -i pipe:0 ...`.
- Do NOT follow directory symlinks when scanning `StreamVoice/` (588 case-alias links, a followed scan counts 38,697 wavs). `Override` is a symlink to `override`; run `readlink -f` before staging.
- Do NOT overwrite vanilla files or reuse vanilla resrefs (Steam verify restores them). New names: lowercase, <= 16 chars, prefix `kpv`.
- Do NOT run any live test (E1-E8, T-SSF, T-TIMER) without explicit user GO, a forged save, and the D0 live-run pause lifted (memory feedback-pause-live-fix-runs).
- Do NOT upload extracted actor audio to cloud voice services; K1/K2 audio is personal-use only, `kvoice` must force `personal_only` for any vanilla parent (k1_port gate currently classes vanilla as approval-free and has no audio kind).
- Chatterbox always watermarks; F5/XTTS/IndexTTS-2/Fish/Higgs/kNN-VC weights are not public-share safe.
- REA sessions drop after ~22-40 calls; `rea-recon` agents have no Read/Bash, so paste facts into the brief and give it an output path that does not exist yet.
- Some Haiku agents refuse to write report files and reply inline instead (b6, b8, b10, v10): check the file exists after each wave and transcribe if not.
- `wave_tools.py add`: put `<wave_id>` BEFORE `--entries` (argparse swallows it otherwise).
- Soundset persistence is NOT proven: r3 shows save-side serialization of `+0x10EC` and `+0x314` only; the load-side reader is unresolved (V6.md:176 says LoadCreature 0x6AFED0).

- V-B lessons (2026-10-13): `mark-done` and `handoff` need `--plan patch_manager_mods/41_custom_voice_and_events.md` (without it `mark-done` picked doc 40). Haiku wave w11-41-VB overshot the 60k budget on all 5 units (84k-135k): split card scopes to one module each. Real vanilla VO is peaky (-19.9 LUFS / -0.5 dBTP), so `kvoice_norm` accepts a clip up to 3 LU under target with `tp_limited` true; do not tighten that back to a hard error.
- V-B facts: `kvoice.py clip` needs `--source-id` and one of `--lip-tsv`/`--no-lip`; it writes 2 ledger rows (wav + lip). Output length = source + 0.15 s lead-in, rounded up to a 1152-sample frame (<=36 ms). `kvoice` has no PCM-RIFF or 470-byte-prefix writer yet (E2 variants a and d): V-C adds them.
- V-C trigger route: scripts via `runscript` (console `~`, patch live, never targeted-tested). Compile test NSS with nwnnsscomp only (never pykotor `compile_nss`) and check `check_ncs_arg_order.py`.

- V-C host prep lessons (2026-10-13): plan bug, `kpve8_seventeenchrs` is 19 chars; the 17-char E8 name is `kpve8_seventeench`. `check_ncs_arg_order.py` takes a game dir (`<dir>/override`, use `--override-only`) and needs the pykotor venv `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python`, not file args. Compile with `~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh <out.ncs> <src.nss>`. Haiku bug caught in review: a bare ini name in `GetPrivateProfileIntA` is searched in the Windows dir (fixed with an absolute path). E4 uses strref 36878 (`p_bastila_rprty`); its wav is not at the top level of StreamVoice or StreamSounds, confirm before the live run. Runbook: `patch_manager_mods/kvoice/V-C_RUNBOOK.md`; trigger route CORRECTED: `additional-console-commands` and `k2-visible-console` ARE installed live (kpm_status.py does not list them; check `patch_config.toml`), `runscript` untested; user chose it as the trigger route 2026-10-13 (fallbacks: native hotkey RunScript 0x6FD8D0, then commands/*.toml). E4 wav confirmed at `StreamSounds/P_BASTILA_RPRTY.wav` (uppercase, 470-byte prefix + RIFF PCM). `k2-mp3-playback-fix` is live (loose .mp3 plays, beats same-named .wav).

- V-C live lessons (2026-10-15/16, E1/E2 run): console `runscript` NEVER ran a script (no `.ncs` open, no credits, no error). The only working trigger was an Override replacement of `k_hen_heartbt01` (every ~6 s per companion, hot-swappable because the engine reloads the NCS each run), but it hits EVERY save and is retired by the no-loose-files rule. Results: E1 PASS (loose RIFF PCM16 22050 Hz mono WAV plays via PlaySound); E2b (58-byte container named .wav) opened and read but SILENT; E2c (bare .mp3) NEVER looked up. Host observer used: `inotify` via ctypes on the live Override dir (no native probe needed). A new file added while the game runs may not be indexed; restart after staging. A global script override with a counter guard (SetLocalNumber 28) did not hold: no guards on global overrides.
- V-C2 / doc 44 collisions (2026-10-17): do not add a second owner for GetKeyEntry 0x712360 (the VFS session owns it); Demand 0x711C20 and Malloc 0x712F30 are perf-lab's; 0x70CAA0 is InitializeSource (mp3-fix hooks inside); RunScript 0x6FD8D0 is companion-ai's. Details: `patch_manager_mods/kvoice/VFS_COLLISION_FEEDBACK.md`. `pkill`/`kill` and some `rm -r` forms are permission-gated: ask the user to run `! pkill -f <name>`. Wave `w13-vfs-collide` (6 Haiku, all PARTIAL; u06 used 105k tokens, over the 100k ceiling: keep KPM-source recon to one narrow question).

### Verification commands
- Host tests (after V-B): `python3 scripts/run_tests.py -k kvoice` (always scoped) and `python3 scripts/compat_check.py patch_manager_mods/k1_port/k1_port_sources.py` after touching the ledger.
- Prototype baseline: `python3 -I .claude/waves/w10-voice/tmp/b9/kvoice_proto_test.py` (15/15 expected; needs a vanilla K2 clip copied into `tmp/b9/vanilla/`, delete it afterwards).
- Container check on a real file: `python3 -I` header compare against the 58-byte layout in `w3/b9.md`; counts to reproduce: StreamVoice 17,299 wav / 1,023 mp3 (991 bare) ; StreamSounds 832 RIFF@470 ; K1 streamwaves 13,799.
- Plan state: `python3 scripts/plan_manager.py resume patch_manager_mods/41_custom_voice_and_events.md`.
