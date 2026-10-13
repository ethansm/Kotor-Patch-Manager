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
- **Branch:** `KOTOR-II` (d111b7a feat(kvoice): offline voice clip toolchain, container/loudness/lip/ledger/verifier/CLI (41:V-B))
- **Generated:** 2026-10-13 01:27:15 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
d111b7a feat(kvoice): offline voice clip toolchain, container/loudness/lip/ledger/verifier/CLI (41:V-B)
 patch_manager_mods/k1_port/k1_port_prune.py        |   2 +-
 patch_manager_mods/k1_port/k1_port_sources.py      |   2 +-
 patch_manager_mods/k1_port/sources.json            |  14 +
 patch_manager_mods/kvoice/README.md                |  39 +++
 patch_manager_mods/kvoice/kvoice.py                | 247 +++++++++++++++
 patch_manager_mods/kvoice/kvoice_audio.py          |  98 ++++++
 patch_manager_mods/kvoice/kvoice_container.py      | 130 ++++++++
 patch_manager_mods/kvoice/kvoice_ledger.py         |  71 +++++
 patch_manager_mods/kvoice/kvoice_lip.py            | 108 +++++++
 patch_manager_mods/kvoice/kvoice_norm.py           | 100 +++++++
 patch_manager_mods/kvoice/kvoice_pack.py           | 157 ++++++++++
 patch_manager_mods/kvoice/kvoice_verify.py         | 197 ++++++++++++
 patch_manager_mods/kvoice/test_kvoice_audio.py     | 144 +++++++++
 patch_manager_mods/kvoice/test_kvoice_cli.py       | 325 ++++++++++++++++++++
 patch_manager_mods/kvoice/test_kvoice_container.py | 193 ++++++++++++
 patch_manager_mods/kvoice/test_kvoice_ledger.py    | 218 ++++++++++++++
 patch_manager_mods/kvoice/test_kvoice_lip.py       | 175 +++++++++++
 patch_manager_mods/kvoice/test_kvoice_norm.py      | 171 +++++++++++
 patch_manager_mods/kvoice/test_kvoice_pack.py      | 223 ++++++++++++++
 patch_manager_mods/kvoice/test_kvoice_verify.py    | 333 +++++++++++++++++++++
 20 files changed, 2945 insertions(+), 2 deletions(-)
```
- **Dirty Files (143):**
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
  - ... +133 more (run `git status --short`)

## 2. Immediate Objective (V-C)
**Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)**

Status: TODO
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

### Verification commands
- Host tests (after V-B): `python3 scripts/run_tests.py -k kvoice` (always scoped) and `python3 scripts/compat_check.py patch_manager_mods/k1_port/k1_port_sources.py` after touching the ledger.
- Prototype baseline: `python3 -I .claude/waves/w10-voice/tmp/b9/kvoice_proto_test.py` (15/15 expected; needs a vanilla K2 clip copied into `tmp/b9/vanilla/`, delete it afterwards).
- Container check on a real file: `python3 -I` header compare against the 58-byte layout in `w3/b9.md`; counts to reproduce: StreamVoice 17,299 wav / 1,023 mp3 (991 bare) ; StreamSounds 832 RIFF@470 ; K1 streamwaves 13,799.
- Plan state: `python3 scripts/plan_manager.py resume patch_manager_mods/41_custom_voice_and_events.md`.
