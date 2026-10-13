# Session Handoff & Plan State: patch_manager_mods/41_custom_voice_and_events.md

## 0. Next-Phase Direction
Execute V-C: Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/41_custom_voice_and_events.md`
- **Waves:** .claude/waves/WI-41-a1/
- **Waves:** .claude/waves/WI-41-a2/
- **Waves:** .claude/waves/WI-41-b1/
- **Waves:** .claude/waves/WI-41-b2/
- **Waves:** .claude/waves/w11-41-VB/
- **Branch:** `KOTOR-II` (145565f feat(41:V-C): host prep for live voice probe (E-matrix builder, PlaySound logger, probe scripts, runbook))
- **Generated:** 2026-10-13 01:37:14 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
145565f feat(41:V-C): host prep for live voice probe (E-matrix builder, PlaySound logger, probe scripts, runbook)
 .../handoffs/41_custom_voice_and_events.handoff.md |  42 ++--
 patch_manager_mods/41_custom_voice_and_events.md   |   4 +-
 patch_manager_mods/kvoice/V-C_RUNBOOK.md           |  81 ++++++++
 patch_manager_mods/kvoice/kvoice_ematrix.py        | 213 +++++++++++++++++++++
 patch_manager_mods/kvoice/test_kvoice_ematrix.py   | 209 ++++++++++++++++++++
 patch_manager_mods/voice_probe_src/VoiceProbe.cpp  |  74 +++++++
 patch_manager_mods/voice_probe_src/exports.def     |   3 +
 .../voice_probe_src/kotor2-steam-aspyr.hooks.toml  |  18 ++
 patch_manager_mods/voice_probe_src/manifest.toml   |  13 ++
 patch_manager_mods/voice_probe_src/nss/README.md   |  18 ++
 .../voice_probe_src/nss/build/kpv_e1.ncs           | Bin 0 -> 76 bytes
 .../voice_probe_src/nss/build/kpv_e2a.ncs          | Bin 0 -> 77 bytes
 .../voice_probe_src/nss/build/kpv_e2b.ncs          | Bin 0 -> 77 bytes
 .../voice_probe_src/nss/build/kpv_e2c.ncs          | Bin 0 -> 77 bytes
 .../voice_probe_src/nss/build/kpv_e2d.ncs          | Bin 0 -> 77 bytes
 .../voice_probe_src/nss/build/kpv_e4.ncs           | Bin 0 -> 337 bytes
 .../voice_probe_src/nss/build/kpv_e7a.ncs          | Bin 0 -> 110 bytes
 .../voice_probe_src/nss/build/kpv_e7b.ncs          | Bin 0 -> 185 bytes
 .../voice_probe_src/nss/build/kpv_e8.ncs           | Bin 0 -> 217 bytes
 patch_manager_mods/voice_probe_src/nss/kpv_e1.nss  |   8 +
 patch_manager_mods/voice_probe_src/nss/kpv_e2a.nss |   7 +
 patch_manager_mods/voice_probe_src/nss/kpv_e2b.nss |   7 +
 patch_manager_mods/voice_probe_src/nss/kpv_e2c.nss |   7 +
 patch_manager_mods/voice_probe_src/nss/kpv_e2d.nss |   7 +
 patch_manager_mods/voice_probe_src/nss/kpv_e4.nss  |  23 +++
 patch_manager_mods/voice_probe_src/nss/kpv_e7a.nss |   9 +
 patch_manager_mods/voice_probe_src/nss/kpv_e7b.nss |  14 ++
 patch_manager_mods/voice_probe_src/nss/kpv_e8.nss  |  17 ++
 patch_manager_mods/voice_probe_src/voice_probe.ini |   4 +
 29 files changed, 749 insertions(+), 29 deletions(-)
```
- **Dirty Files (153):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ... +143 more (run `git status --short`)

## 2. Immediate Objective (V-C)
**Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)**

Status: PARTIAL
Task: Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)

## 2b. Upcoming Batches in Pipeline
- **V-D** : Silent-commoner SSF fix (needs T-SSF) and murmur bed prototype (ideas 1-2)
- **V-E** : Voice spike: Qwen3 VoiceDesign speed/VRAM, 200-line consistency, FX chain, RVC timing on the 5070, Day-5 sound-alike screen (`w3/b5.md` 5-day plan)
- **V-F** : Event system (cast from the diversity pool, cooldowns, templates T4, T3, then movement templates T1; b2 ranking) merged into doc 40 V5/V3

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

- V-C host prep lessons (2026-10-13): plan bug, `kpve8_seventeenchrs` is 19 chars; the 17-char E8 name is `kpve8_seventeench`. `check_ncs_arg_order.py` takes a game dir (`<dir>/override`, use `--override-only`) and needs the pykotor venv `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python`, not file args. Compile with `~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh <out.ncs> <src.nss>`. Haiku bug caught in review: a bare ini name in `GetPrivateProfileIntA` is searched in the Windows dir (fixed with an absolute path). E4 uses strref 36878 (`p_bastila_rprty`); its wav is not at the top level of StreamVoice or StreamSounds, confirm before the live run. Runbook: `patch_manager_mods/kvoice/V-C_RUNBOOK.md`; trigger route CORRECTED: `additional-console-commands` and `k2-visible-console` ARE installed live (kpm_status.py does not list them; check `patch_config.toml`), `runscript` untested. E4 wav confirmed at `StreamSounds/P_BASTILA_RPRTY.wav` (uppercase, 470-byte prefix + RIFF PCM). `k2-mp3-playback-fix` is live (loose .mp3 plays, beats same-named .wav).

### Verification commands
- Host tests (after V-B): `python3 scripts/run_tests.py -k kvoice` (always scoped) and `python3 scripts/compat_check.py patch_manager_mods/k1_port/k1_port_sources.py` after touching the ledger.
- Prototype baseline: `python3 -I .claude/waves/w10-voice/tmp/b9/kvoice_proto_test.py` (15/15 expected; needs a vanilla K2 clip copied into `tmp/b9/vanilla/`, delete it afterwards).
- Container check on a real file: `python3 -I` header compare against the 58-byte layout in `w3/b9.md`; counts to reproduce: StreamVoice 17,299 wav / 1,023 mp3 (991 bare) ; StreamSounds 832 RIFF@470 ; K1 streamwaves 13,799.
- Plan state: `python3 scripts/plan_manager.py resume patch_manager_mods/41_custom_voice_and_events.md`.
