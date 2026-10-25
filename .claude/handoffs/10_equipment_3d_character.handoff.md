# Session Handoff & Plan State: patch_manager_mods/10_equipment_3d_character.md

## 0. Next-Phase Direction
Execute EQ2: Weapons in hands, engine wield-style rule, flourishes with weapons

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/10_equipment_3d_character.md`
- **Waves:** .claude/waves/WI-10-a1/
- **Waves:** .claude/waves/WI-10-a2/
- **Waves:** .claude/waves/WI-10-b1/
- **Waves:** .claude/waves/WI-10-b2/
- **Waves:** .claude/waves/WI-10-c1/
- **Branch:** `KOTOR-II` (dfa6dfde PLAY_TEST_QUEUE: EQ10-P1 equip3d Hide/Show + Holster/Draw (0.3.0-probe, not installed))
- **Generated:** 2026-10-25 18:32:45 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
dfa6dfde PLAY_TEST_QUEUE: EQ10-P1 equip3d Hide/Show + Holster/Draw (0.3.0-probe, not installed)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 1 +
 1 file changed, 1 insertion(+)
```
- **Dirty Files (184):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ` M CLAUDE.md`
  - ... +174 more (run `git status --short`)

## 2. Immediate Objective (EQ2)
**Weapons in hands, engine wield-style rule, flourishes with weapons**

Status: BUILT
Task: Weapons in hands, engine wield-style rule, flourishes with weapons

## 2b. Upcoming Batches in Pipeline
- **EQ4** : Toolbar column: Animate, Spin (split play/pause + Reset), Weapons, Emotes pop-up
- **EQ6** : Doc 20 extraction: Equip3dLogic.h, host test, F14-F19 clamps and INI hygiene
- **EQ7** : Common extraction: saferead C3a, SceneView C3b, vertical pan C3c

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/10_equipment_3d_character.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate EQ2` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
EQ10 recon 2026-10-25 (wave `.claude/waves/w1-10-EQ10/` u01..u04.md; read those, not the sources):
- User request (verbatim): "In the equipment screen, we should default to showing weapons and having them drawn. The weapon toggle should be to hide/show weapons and we should add Holster/Draw weapon toggle button as well. Maybe another 2/3 1/3 row where the 2/3 hide weapon 1/3 holster/draw toggle. When weapon toggled off, the row is 3/3 one button to show weapons, auto draw weaposn (disable holster to start) when toggleing weapons on". Keyboard key: holster already has toggle_key (default 0x77 = F8, world draw/stow); no new key unless the user asks for a whole-system on/off.
- Fact: g_weaponsOn is a process global defaulting false (EquipCharPreview.cpp:287), click at :434, no INI key (Equip3dLogic.h:122 rejects unknown keys). Tooltip kTips[2] at :341.
- Fact: all toolbar/menu/tip controls are GFF clones written by `scripts/add_equip_3dview_control.py` into the live Override equip_p.gui (ids 49..78, pinned in tests/static/gui_ids.toml); NO spare control. A new Holster/Draw button needs a new tag (id 79 appended after LBL_E3D_TIP so existing ids do not move) + gui_ids.toml + re-running the script on the live gui (live write: needs GO).
- Fact: toolbar column is 62 px wide (x 684, y 174..300); the spin row split is 30%/4% gap (Equip3dLogic.h:231 layoutSpin, integer math). 2/3 + 1/3 of 62 = ~40 + ~20 px: text "Holster"/"Hide weapons" will not fit; needs short text, an icon, or a separate row (Emotes' original GUI cell y=278 is empty: Emotes is moved onto the Reset row at :904).
- Fact: k2.holster preview: RegisterPreview(weaponGob, beh=nullptr, left) (editor HolsterEditor.cpp:2947); placement runs from holster H7 hook 0x504EF5 (attach-behavior refresh) for any registered Gob, gated only by holster ini editor=1 (default) — not by probe. 8-slot registry shared by all consumers (HolsterPreview.h kPreviewMax). SetPreviewDraw is global over all registered entries, needs no BeginEdit, returns the request echo; new entries start stowed. Must UnregisterPreview(gob) before every weapon free (equip3d frees at EquipCharPreview.cpp:470-473 via detachWeapons; also destroyCreature/evict/re-bind paths :525, :867, :870). Never UnregisterPreview(NULL) (clears the editor's entries and ends its edit).
- Fact: equip3d attaches weapons itself (FnCreateModel 0x00462320, vt[0x50](g, body, "rhand"/"lhand", 0)), not through preview3d. UNVERIFIED: whether that attach refreshes via 0x504D40 so holster's H7 fires for it -> first build step must be a probe-logged check (holster logs "API previewdraw"/preview placement lines).
- Fact: stanceStyle(holstered,...) (Equip3dLogic.h:167) exists with tests but has no call site; attachWeapons calls styleFor directly (:664). Holstered = stance off = alignment idle.
- Fact: optional interface = omit from requires_interfaces + lazy connect (enemy_effects_strip/manifest.toml:9); consumer helper k2holster::Client in _shared_src/HolsterApi.h (canDraw() :139).
- Verification: `python3 scripts/run_tests.py equip3d` ; `python3 scripts/build_check.py --mod equipment_3d_character_src` ; `python3 scripts/compat_check.py <shared files>` ; `scripts/pre_install_gate.sh --mod equipment_3d_character_src`.
