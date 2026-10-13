# Session Handoff & Plan State: patch_manager_mods/41_custom_voice_and_events.md

## 0. Next-Phase Direction
41 custom voice and events: next phase V-B (kvoice host build) then V-C live probe

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/41_custom_voice_and_events.md`
- **Waves:** .claude/waves/WI-41-a1/
- **Waves:** .claude/waves/WI-41-a2/
- **Waves:** .claude/waves/WI-41-b1/
- **Waves:** .claude/waves/WI-41-b2/
- **Branch:** `KOTOR-II` (139639c docs(41): custom voice, barks and voiced events research (w10-voice: 35 Haiku reports + 6 fact-checks, Gemini scorecard, test ladder E1-E8, kvoice spec))
- **Generated:** 2026-10-13 01:07:49 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
139639c docs(41): custom voice, barks and voiced events research (w10-voice: 35 Haiku reports + 6 fact-checks, Gemini scorecard, test ladder E1-E8, kvoice spec)
 .../voice_audio/catalog/gemini_creature_claims.tsv |   58 +
 .../voice_audio/catalog/gemini_module_claims.tsv   |   33 +
 .../voice_audio/catalog/k1_streamwaves_groups.tsv  |  604 ++++++
 .../research/voice_audio/catalog/k2_avo_bank.tsv   |  199 ++
 .../research/voice_audio/catalog/k2_soundsets.tsv  | 1961 ++++++++++++++++++++
 .../voice_audio/catalog/k2_utc_voice_map.tsv       |   46 +
 .claude/research/voice_audio/w1/u01.md             |   30 +
 .claude/research/voice_audio/w1/u02.md             |   28 +
 .claude/research/voice_audio/w1/u03.md             |   41 +
 .../research/voice_audio/w1/u03_soundset_rows.tsv  |   71 +
 .claude/research/voice_audio/w1/u04.md             |   35 +
 .claude/research/voice_audio/w1/u05.md             |   48 +
 .claude/research/voice_audio/w1/u06.md             |   25 +
 .claude/research/voice_audio/w1/u07.md             |   21 +
 .claude/research/voice_audio/w1/u08.md             |   28 +
 .claude/research/voice_audio/w1/u09.md             |   38 +
 .claude/research/voice_audio/w1/u10.md             |   22 +
 .claude/research/voice_audio/w1/u11.md             |   24 +
 .claude/research/voice_audio/w1/u12.md             |   47 +
 .claude/research/voice_audio/w2/r1.md              |   53 +
 .claude/research/voice_audio/w2/r2.md              |   53 +
 .claude/research/voice_audio/w2/v03.md             |   35 +
 .claude/research/voice_audio/w2/v04.md             |   42 +
 .claude/research/voice_audio/w2/v05.md             |   32 +
 .claude/research/voice_audio/w2/v06.md             |   56 +
 .claude/research/voice_audio/w2/v07.md             |   41 +
 .claude/research/voice_audio/w2/v08.md             |   59 +
 .claude/research/voice_audio/w2/v09.md             |   36 +
 .claude/research/voice_audio/w2/v11.md             |   29 +
 .claude/research/voice_audio/w2/v12.md             |   51 +
 .claude/research/voice_audio/w3/b1.md              |   60 +
 .claude/research/voice_audio/w3/b10.md             |   30 +
 .claude/research/voice_audio/w3/b11.md             |  311 ++++
 .claude/research/voice_audio/w3/b2.md              |   60 +
 .claude/research/voice_audio/w3/b3.md              |   60 +
 .claude/research/voice_audio/w3/b4.md              |   57 +
 .claude/research/voice_audio/w3/b5.md              |   86 +
 .claude/research/voice_audio/w3/b6.md              |   37 +
 .claude/research/voice_audio/w3/b7.md              |   58 +
 .../research/voice_audio/w3/b7_persona_cards.toml  |  196 ++
 .claude/research/voice_audio/w3/b8.md              |   37 +
 .claude/research/voice_audio/w3/b9.md              |   28 +
 .claude/research/voice_audio/w3/r3.md              |   51 +
 .claude/research/voice_audio/w4/c1.md              |   47 +
 .claude/research/voice_audio/w4/c2.md              |   49 +
 .claude/research/voice_audio/w4/c3.md              |   38 +
 .claude/research/voice_audio/w4/c4.md              |   46 +
 .claude/research/voice_audio/w4/c5.md              |   50 +
 .claude/research/voice_audio/w4/c6.md              |   30 +
 patch_manager_mods/40_npc_behaviour_variety.md     |    2 +-
 patch_manager_mods/41_custom_voice_and_events.md   |  111 ++
 51 files changed, 5289 insertions(+), 1 deletion(-)
```
- **Dirty Files (148):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/texture_diversity_research.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ... +138 more (run `git status --short`)

## 2. Immediate Objective (V-B)
**`kvoice` host build: container/loudness/lip/ledger/verifier + tests + a KPM test-hook build step (no game); add `audio` kind and ONE source id to `k1_port` (`k2_install_reference` class `vanilla_reference` per b4, or `vanilla_k2` per b9: pick one)**

Status: TODO
Task: `kvoice` host build: container/loudness/lip/ledger/verifier + tests + a KPM test-hook build step (no game); add `audio` kind and ONE source id to `k1_port` (`k2_install_reference` class `vanilla_reference` per b4, or `vanilla_k2` per b9: pick one)

## 2b. Upcoming Batches in Pipeline
- **V-C** : Live tests E1-E8 incl. E5, E6, plus T-SSF and T-TIMER, with a tiny probe (needs user GO and the D0 pause lifted)
- **V-D** : Silent-commoner SSF fix (needs T-SSF) and murmur bed prototype (ideas 1-2)
- **V-E** : Voice spike: Qwen3 VoiceDesign speed/VRAM, 200-line consistency, FX chain, RVC timing on the 5070, Day-5 sound-alike screen (`w3/b5.md` 5-day plan)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/41_custom_voice_and_events.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate V-B` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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
