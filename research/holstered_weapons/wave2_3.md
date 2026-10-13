# Holstered weapons — research waves 2-3 (raw agent reports)

Persisted 2026-10-13 from the planning session transcript (33 Haiku/REA agent final reports, verbatim, HTML entities decoded).
Synthesis and decisions: `patch_manager_mods/42_holstered_weapons.md`. Wave 1 raw: `research/holstered_weapons/raw/`.
Addresses are Steam `swkotor2.exe`. Treat each report as an unverified recon claim until the P0 probe confirms it.

## Agent "M1 dump vanilla node trees" finished

OUTPUT: None
EVIDENCE: `/home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages/pykotor/extract/installation.py` (SearchLocation.CHITIN); game `.../Knights of the Old Republic II/Data/Models.bif`; `/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/k1_port/setup_venv.sh` (pykotor 2.3.12 pin)
FINDING: No vanilla humanoid has hip or back dummy nodes. Usable parents: pelvis_g, lthigh_g/rthigh_g (hip); torso_g, torsoUpr_g (back).
OPEN: Add new hip/back dummies under those meshes (needs MDL write via k1port venv), or reuse existing nodes only?
VERDICT: CONFIRMED
DETAIL:
1. PyKotor 2.3.12 ran in-memory via `Installation` on CHITIN (Models.bif) only; Override ignored. BIF integrity not checked.
2. Run: venv `python -I -` on stdin, cwd `~/.cache/k1_port_textures/pykotor_logs` (pykotor writes logs/ there). No repo files touched.
3. Supermodels: S_Male01 none; S_Male02→S_Male01; S_Female01→S_Male02; S_Female02→S_Female01; S_Female03→S_Female02.
4. Node counts: s_male01 102 (depth 13); s_male02 109; s_female01 106; s_female02 103; s_female03 101.
5. Bodies: PMBJL/M/S→S_Female02 (65 nodes); PFBJL/M/S→S_Female03 (66 nodes). Female adds breastbone.
6. Companion: P_BastilaBB→S_Female03 (94 nodes). P_BastilaH→P_BastilaBB (36 nodes, head only, no pelvis_g/hands/headhook).
7. Robe: N_JediRobeM→S_Female02 (79 nodes). A_JEDIROBE_00x are PLACEABLE, 5 nodes, not body robes.
8. s_male01 spine (name:type): S_Male01 > cutscenedummy:DUM > rootdummy:DUM
9. rootdummy > pelvis_g:TRI > lthigh_g>lshin_g>lfoot_g; rthigh_g>rshin_g>rfoot_g
10. rootdummy > torso_g:TRI > torsoUpr_g:TRI > headhook:DUM; necklwr_g>neck_g>hturn_g:DUM>head_g
11. torsoUpr_g > rcollar_dum:DUM>rcollar_g>rbicep_g>rbicepL_g>rforearm_g>rhand_g:TRI>rhand:DUM
12. torsoUpr_g > lcollar_dum:DUM>...>lforearm_g>lhand_g:TRI>lhand:DUM
13. S_Male01 root-level DUM: DeflectHook, LightsaberHook, handconjure, headconjure, impact_bolt
14. pmbjm: pelvis_g and torso_g both under rootdummy; hands are Rforearm_g/Rhand_g/Lhand_g (capital); no head_g (head is separate model).
15. Name case differs across models (rhand_g vs Rhand_g); match case-insensitively.
16. Hip/back dummies: none in any of the 17 MDLs checked. Only "backflaps" (SKIN) on P_BastilaBB.
17. Hip candidates: pelvis_g:TRI (child of rootdummy); lthigh_g/rthigh_g:TRI (children of pelvis_g).
18. Back candidates: torso_g:TRI, torsoUpr_g:TRI (child of torso_g).
19. Precedent: headhook:DUM sits under torsoUpr_g:TRI; rhand:DUM under rhand_g:TRI. Dummies under meshes are normal.
20. Root-level DUM hooks (DeflectHook, LightsaberHook) exist on all humanoids, parented to the model root.
21. Not dumped: full trees for other supermodels and bodies (differences listed above). Re-run is cheap.

## Agent "H7 local combat-timing knowledge" finished

OUTPUT: None
EVIDENCE: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_ai_atlas/findings/AI2.md:125 · /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/29_paused_command_evidence/PQ2.md:70 · /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/engine_ai_atlas/AI2_combat_round.md:74
FINDING: Attack timing is ms countdowns (3000 ms round budget, pause timer, nominal 1500 ms per attack). No animation-completion callback or wait is documented.
OPEN: Does any engine path wait on animation end, or is melee `dur` (ResolveMeleeAnimations 0x6E0410, attack+8) a fixed ms value?
VERDICT: PARTIAL
DETAIL:
Base dir: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/. Steam addresses; GOG names via ghidra DB.
1. Combat entry: BroadcastCombatStateToParty 0x57AC30 -> SetCombatState(1) 0x57A560 sets +0x520 (in combat) and +0x524 (combat-exit ms). Trigger: enemy rep<=10 -> 0x576DC0 (AI6_faction.md:137).
2. Round arm: StartCombatRound 0x58E730 sets +0xA84=1, +0xA9C=3000. Melee-path caller is not documented.
3. Round timer: UpdateCombatRoundTimer 0x571030 (from AIUpdate 0x569320); IncrementTimer 0x58F7A0; DecrementPauseTimer 0x58FB40; SetRoundPaused 0x58FC60; SetPauseTimer 0x58FCD0; DecrementRoundLength 0x58FD50.
4. Attack start: ActionAttack (cmd 37) 0x66DAF0 -> AddAttackActions 0x568540 -> AddAttackAction 0x590A50 (duration (DAT_00a7e0b0+3000)/2 = 1500, findings/AI2.md:170).
5. Attack execute: RunActions 0x5D56B0 -> AIActionPhysicalAttacks 0x6D7390 (action 0xC) or AIActionCombat 0x6D9500 (0x3F) -> ResolveMeleeAttack 0x6DEA90 -> ResolveMeleeAnimations 0x6E0410 (attack+8=dur, DecrementRoundLength(dur,0)).
6. Impact: GetWeaponImpactTime 0x6DE2B0; sub-attacks release when the pause timer passes their impact time (findings/AI2.md:125).
7. Animation playback: AIActionPlayAnimation 0x5D9DA0 (action 0x06); timed 2000 ms anims 0x5D17F0/0x5D18C0 (AI4_scheduler.md:89). "Ready" anim in EndAttackAction 0x6DAB20 (anim 0x2710, findings/AI2.md:173) and ResetCombatAnimations 0x5713F0.
8. Draw/re-equip: AIActionType47 0x5DF920 clears actions, re-equips, re-queues attack (AI4_scheduler.md:103). Draw order vs first swing at entry is not documented (42_holstered_weapons.md:40 lists it as open).
9. Q2: repo documents no wait on animation completion. findings/AI2.md:130 "busy for the animation" is prose, not a verified wait.
10. Conflict: AI2_combat_round.md:74 says the budget shrinks by "each attack's animation time", but melee dur is nominal 1500 and its source is not in repo.
11. PQ2.md:70 refutes animation length as a wait for spells ("anim ids cosmetic; timing from 2DA ms"). Melee is not covered by that test.
12. Round gates: AIActionCombat holds while +0xAB8>=1 (one action per round) or +0xAA8!=0 (paused) (PQ2.md:7; findings/AI2.md:130).
13. Creature in-combat fields: +0x520 in combat; +0x524 countdown (AIUpdate, SetCombatState(0) at 0x569923, AI4_scheduler.md:120); +0x528 frozen; +0x52C end-instead-of-approach; +0x570 slot-6 fallback 3000 ms (AI2_combat_round.md:91-126).
14. Round fields (ptr at creature +0x10DC): +0xA84 active, +0xA94 timer, +0xA9C budget, +0xAA8 paused, +0xAB0 pause ms, +0xAB4 infinite, +0xAB8 actions this round, +0xB00 action list, +0xB20 target (AI2_combat_round.md:93-125).
15. Client combat mode: GetCombatMode 0x740860 reads [CClientExoAppInternal+0x320]; on while a party member has a live creature in [+0x550/+0x564/+0x568] (PQ1.md:7). AI7_script_interface.md:139 calls +0x550 "attempted attack target", so the meaning is unverified.
16. Server pause (bit 2 of [CServerExoApp+0x10078]) freezes the round timer 0x571030 (PQ1.md:4).
17. Live evidence: no timed measurements. Only user reports: late start after unpause (29_paused_command_queue.md:8); companion attack spin, "live 2026-10-01, awaiting play test" (engine_ai_atlas/bug_candidates.md:29); flurry bonus attack not reproduced (issue_drafts/issue_55_flurry_combat_queue.md:29). Exact delay is SPECULATIVE until probes P1-P4 (PQ1.md:6).
18. Ghidra DB (GOG): StartCombatRound 0x772DE0; UpdateCombatRoundTimer 0x689B60; AIActionPhysicalAttacks 0x653640; AddAttackActions 0x681170; EndCombatRound 0x773040; SetCombatState 0x693090; ResolveMeleeAnimations 0x65C710; GetWeaponImpactTime 0x65A5B0.
19. DB has no DrawWeapon, AttackAnimation or GetAnimationDuration symbol. Unmapped lead: GetAnimationLength (GOG 0x4C84E0), not used in atlas.
20. Cannot answer from repo: (a) draw/ready order vs first swing at entry; (b) whether dur equals animation length or a fixed value; (c) any animation-end event; (d) the melee-path caller of StartCombatRound; (e) measured round timings in ms from play.

## Agent "A2 bpy procedural draw animation" finished

OUTPUT: None
EVIDENCE: https://docs.blender.org/api/5.0/change_log.html, https://docs.blender.org/api/4.2/bpy.ops.nla.html, https://projects.blender.org/blender/blender/issues/107242
FINDING: Bake IK to FK with bpy.ops.nla.bake(visual_keying, clear_constraints) on 4.x; Blender 5.0 removed Action.fcurves; no turnkey hip/back draw script found.
OPEN: Which Blender version will run the script, 4.2 or 5.x? The slotted-action API differs.
VERDICT: PARTIAL
DETAIL:
- Bake op (4.2 docs): only_selected, visual_keying, clear_constraints; newer adds use_current_action, clean_curves, channel_types. Re-check the 5.x signature (docs.blender.org returned 403).
- Run in Pose mode with only the armature selected; script-selected bones gave "Nothing to bake" when other objects were selected (BlenderArtists, 2.80).
- Visual keying differs from apply-then-key (bug T76791). Workaround: bpy.ops.pose.visual_transform_apply(), then keyframe_insert (issue 107242). Reported flaky, so verify keys on every frame.
- Per-frame loop (general knowledge, not re-checked): scene.frame_set(f), view_layer.update(), pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=f).
- 5.0 change log: Action.fcurves, groups, id_root removed; ActionChannelbagFCurves added (new() gained group_name).
- 5.x access: action.layers[0].strips[0].channelbag(slot, ensure=True).fcurves, or fcurve_ensure_for_datablock (action must be assigned).
- Version map: 4.2 binding handles; 4.3 channels(slot_handle); 4.4+ channelbag(slot).
- A 5.0 docs snippet says object keyframe_insert auto-creates action and slot. Not verified for pose bones.
- Key only rotation_quaternion on upper arm, forearm, hand. No location or scale keys on bones meant for export.
- Fps and range: scene.render.fps, scene.frame_start/frame_end (general knowledge, not checked).
- Interpolation: keyframe.interpolation in CONSTANT | LINEAR | BEZIER; keyframe.easing in EASE_IN | EASE_OUT | EASE_IN_OUT (AUTO in newer docs).
- Proposed (unverified): key 3-4 poses (rest, reach, pause) with BEZIER and EASE_IN_OUT, then bake dense per-frame keys.
- Hip/back targets: no source found. Proposed (unverified): target = armature.matrix_world @ pelvis.head + offset in pelvis local axes.
- Mirror L/R: ezez thread negates X via bone.matrix = Matrix.Diagonal((-1,1,1,0)) @ bone.matrix, plus 180-degree roll fixes. Described as hacky; no quaternion-component recipe found.
- Negating quaternion components is not a mirror (q and -q are the same rotation). Mirroring flips bone roll sign.
- Side naming: Mirror Bone Constraints add-on handles .L/.R, _l/_R suffixes.
- Rigify has "Apply Snap IK->FK To Keyframes" for limb rigs. blender-iks-to-fks adds no keys, so it does not fit.
- Headless: blender -b file.blend --python script.py (skills.sh snippet).
- Export: KotorBlender (seedhartha) imports and exports MDL animations; compat fixes for 4.2/4.4. Some users report export failures on newer Blender.
- MDL: per-node position and orientation controllers; key = time + values; orientation stored as quaternion in binary; bezier flag is bit 4 of column_count (rakata docs).
- Controller type codes not retrieved. From memory (unverified): position 8, orientation 20.
- Export implication: bake per-bone rotation keys only; add root position if needed; no scale keys.
- No turnkey open-source procedural draw/holster script found. IK Dynamic (paid, Gumroad) has its own bake. Animation Nodes not searched.

## Agent "H1 inventory mapped attach hooks" finished

OUTPUT: None
EVIDENCE: Base P=/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods. R7c,R7d,R7a,R7f,R7h,R7e,R2b,R9 = P/equipment_3d_character_src/research/R7*.txt (R9_head_item_2026-10-02.txt, R2b_creature_slots_appearance_2026-09-30.txt). Also P/10_equipment_3d_character.md:40-60, P/ghidra_knowledge/steam_functions.tsv, P/engine_research_findings/U4_map_CSWCVisualEffectOnObject.tsv:3, P/engine_research_findings/U4.md:33, P/equipment_3d_character_src/EquipCharPreview.cpp:39,43,52,518,635-675. Symbol DB via `query_ghidra.py symbol/address`.
FINDING: 0086a430 hangs weapon Gobs on body nodes rhand/lhand via vt[0x50]; vt[0x50] likely FUN_00461670 (unnamed); 0086f530 is VFX, not weapons; in-world caller unresolved.
OPEN: (1) callers of 0086a430 (in-world vs preview) absent from repo; (2) byte flag from FUN_00458940 gates both attaches, meaning unknown; (3) vtable 0x0098b5cc never read (GAME unset); vt[0x4c]/[0x10c]/[0x58] identities inferred; (4) no part-local setter, combat hook, or per-slot hide mapped.
VERDICT: PARTIAL

DETAIL:
address | name/role | evidence | scope
0x0086a430 | FUN_0086a430: per hand, free old weapon Gob, build model, attach to body node; object read via FUN_0073f530 | R7c:1-102 (old-free 47-48,73-74; attach 59-64,85-90) | UNKNOWN (caller not in repo)
vt[0x50] args in 0086a430 | (this=weapon Gob, parent=body Gob read at this+0x5c+vbase adj, node "rhand"/"lhand", type 0); detach=vt[0x50](0,0,0) then vt[0x4c](0) then vt[0](1)=free | R7c:47-48,63-64,73-74,89-90; R9:25 | appears AttachToNode(type 0); medium confidence
0x00461670 | FUN_00461670 (549b, unnamed in DB): vt[0x50] target by call shape; parent vt[0x10c](name)=node lookup, missing node = silent no-attach; copies node transform; type switch allocs attach record | R7d:1-85; EquipCharPreview.cpp:518 | BOTH (generic Gob)
0x00462320 | FUN_00462320 (944b): named model Gob loader, tries name_x / name_z variants, _EmitterTarget | R7d:93-204 (variants ~l.124-133, emitter ~l.187); R7c:60,86 | BOTH (weapon, VFX, head)
0x00734010 | FUN_00734010 (31b): CExoString::CStr-like accessor; R7c passes literal "dummyright", likely decompile artifact | R7d:205-216; steam_functions.tsv:6989 | BOTH (util)
0x00458940 | FUN_00458940 (14b): returns ptr to byte flag gating both hand attaches | R7c ~l.55 | UNKNOWN meaning
0x0086f530 | CSWCVisualEffectOnObject::LoadVisualEffect (NOT weapons): same Gob loader + _hand/_head/_impact suffixes | U4_map tsv:3; U4.md:33; R7c:103-819 | IN-WORLD (VFX via AddVisualEffect 0x007f0790)
0x00801fa0 | FUN_00801fa0 (1010b) item ctor: model ResRef at item+0x128, variation at +0x11c | R7e:1-2; R9:18-19 | BOTH (item data)
0x007637e0 | FUN_007637e0 (2074b): hand refresh; FUN_00733570("rhand"/"lhand") then vt[0x7c](item, flag 1=R/0=L) on FUN_007ed830() | R7a:8,116,176 | UNKNOWN; called from 00804f10 with "Loading" (R7a:736,770), likely UI/preview side
0x00804f10 | FUN_00804f10 (6709b): equip/char refresh; calls 00801fa0, 0076d590, 007637e0 | R7a:200-207,520-523,735-770 | GUI/preview, not in-world
0x0076d590 | CSWCCreature::UpdateWeaponClass (742b; name_map; GOG 0x419400 same size): wield style per hand | R7f:1-12; name_map; doc:40-60 | IN-WORLD creature fn; preview reuses (EquipCharPreview.cpp:635)
0x00863650 | FUN_00863650 (4574b): ready-idle row from anim 0x2711 + wield style (appearance+0xc5) | R7h:30, 129-138 | BOTH (anim); not in symbol DB
0x0085a9e0 | FUN_0085a9e0 (3931b): in-world head attach, vt[0x50](head,"GoggleHook",0); preview copies via 0076d200 | R9:3-4,11,26; R2b:847 | IN-WORLD (only in-world named-node attach mapped)
Gob (steam_val rows) | SetPosition 0x461490; SetOrientation 0x4614e0; GetPartLocalPosition 0x462cd0; AddAttachment 0x4632c0 (55b); RemoveAttachment 0x463300 (53b); SetObjectScale 0x463340; AttachToScene 0x46d440 (38b, wraps FUN_00461aa0) | symbol "Gob::" | generic; no part-local setter
GAP (a) attach to node with local transform | no part-local setter (only GetPartLocalPosition); node-by-name vt[0x10c] and vt[0x58] unnamed | - | NOT MAPPED
GAP (b) combat enter/exit re-attach | GOG-only names: SetCombatState 0x414080 (397b), InCreatureCombat 0x422e00 (126b), TestAndReEquipweapons 0x474040 (163b); no Steam address, no hook site | symbol "Combat"/"Weapon" | NOT MAPPED
GAP (c) hide/show per slot | candidate UpdateVisibleWeapons Steam 0x0076d890 (332b, same +0x300 offset from UpdateWeaponClass as GOG); UNVERIFIED; no belt/holster nodes | steam_functions.tsv:7743; doc:40-60 | NOT MAPPED
GAP callers | nothing maps who calls 0086a430 (decides in-world vs preview) | - | needs REA procedure_callers on Steam 0x0086a430
Tool note | query_ghidra address/symbol miss all R7 Steam addresses (DB is GOG-keyed except name_map rows); Steam names only in steam_functions.tsv | - | -

## Agent "M3 PyKotor MDL node add round trip" finished

OUTPUT: None
EVIDENCE: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/k1_port/k1_port_model.py:655-665 (writer call pattern); /home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages/pykotor/resource/formats/mdl/io_mdl.py:3213 (writer takes file-like targets) and :4099 (parent derived from children edges)
FINDING: In-memory dummy add works and reads back with parent and pose intact; anims unchanged. Write-back is not byte-identical, and n_commm gains 11 synthesized zero-UV arrays.
OPEN: Does the engine need an empty animation entry for a new node, or is absence from anims fine? Only a live game run can answer this.
VERDICT: PARTIAL
DETAIL:
- Python: /home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python (pykotor 2.3.12 plus k1_port/pykotor_k2_mdl.patch); run with -I -B.
- Game: ~/.steam/steam/steamapps/common/Knights of the Old Republic II (K2). Models read from CHITIN only, no Override.
- Read: read_mdl(mdl_bytes, source_ext=mdx_bytes, size_ext=len(mdx)).
- Write: MDLBinaryWriter(mdl, BytesIO, BytesIO); set force_game = Game.K2 (writer defaults to K1 layout); write().
- Writer closes its targets; use a BytesIO subclass with no-op close() to call getvalue().
- New node: MDLNode(); name; node_type = MDLNodeType.DUMMY; position Vector3; orientation Vector4(0,0,0,1); append to parent.children.
- MDLNode has no parent pointer; writer recomputes node_id and parent_id from children edges.
- p_hk47 (vanilla, animated): MDL 1046875 B, MDX 110464 B, 58 nodes, 45 with mesh, 79 anims, 117 node-level controllers.
- p_hk47 write-back: MDX byte-identical; MDL 1046875 -> 1046623 B, not byte-identical (header size field -252, offsets shift).
- p_hk47 re-read: node rows, anim rows, controller and mesh digests all match; second write equals first.
- n_commm (vanilla, 60 nodes, 0 anims): MDL same length but bytes differ; MDX 140368 -> 144208 (+3840 B).
- n_commm cause: 11 mesh nodes with zero UVs come back with 40 or 80 zero UV entries. Semantic change, not a no-op.
- k1_port sets writer.synth_zero_uv and keep_zero_tangents; their effect was not tested here.
- Dummy "rec_dummy_test" under rhand (p_hk47): re-read shows DUMMY, parent rhand, pos (0.1,0.2,0.3); nodes 58 -> 59.
- Geometry-only add: anim rows unchanged (79 anims, 3824 anim-node entries, 4296 anim-level controllers); new name in no anim.
- Anim node lists are per-anim subsets: 12 of 79 anims have no rhand node; all anim node names exist in the model.
- Variant with empty DUMMY anim entries under the parent: 67 entries added, controllers unchanged, re-reads cleanly.
- Writer accepts a new node with no anim entries (no error); the two variants differ only in anim entries.
- Not tested: engine behaviour for unanimated nodes, unpatched pykotor writes, a human body model.
- p_hk47 is a droid party model; human p_* names tried (p_m01aa, p_bastila, p_carth, etc.) are not in CHITIN.
- MDLOps: Perl 5.38.2 at /usr/bin/perl; MDLOps.pm not in @INC. No mdlops*.pl/.exe at depth 2 of ~/Games/KOTOR2_Mod_Workspace/mdlops_t7 or k1_port_stage/probe_mdlops; dirs exist, contents not inspected.
- No files written; all model I/O in memory; game install read-only.

## Agent "A3 KotorBlender animation export fidelity" finished

OUTPUT: None
EVIDENCE: https://raw.githubusercontent.com/OpenKotOR/kotorblender/master/io_scene_kotor/format/mdl/writer.py ; https://raw.githubusercontent.com/OpenKotOR/kotorblender/master/io_scene_kotor/scene/animnode.py ; https://raw.githubusercontent.com/seedhartha/kotorblender/master/io_scene_kotor/format/mdl/writer.py
FINDING: Animations are frame ranges in root.kb.anim_list over one action per object; EMPTY children export as dummies; position keys are relative to live obj.location.
OPEN: Is the playhead left on a rest frame before export? Export subtracts the current obj.location, so a mid-clip frame shifts every position key (inferred from code, not run).
VERDICT: PARTIAL
DETAIL:
1. No armature needed. Import creates objects with data=None (base.py add_to_collection); rebuild_armature runs only if build_armature (model.py).
2. Animations: root_obj.kb.anim_list of AnimPropertyGroup (ui/props/anim.py): name, transtime (s, default 0.25, 0-2), root, frame_start, frame_end, event_list.
3. No length prop: length = (frame_end - frame_start)/ANIM_FPS (30), per animation.py from_list_anim.
4. Events are event_list items (animevent.py: name, frame default 1), not timeline markers; time = (frame - frame_start)/30. No NLA.
5. Per-animation AnimPropertyGroup.root -> Animation.animroot (from_list_anim line 87) -> writer fixed_string(anim.animroot, 32).
6. root_obj.kb.animroot (default NULL) is copied to Model.animroot on export and drives below_animroot on import.
7. get_or_create_action: one action per object (import name "{root}.{obj}"); each animation is a frame slice of it. Export reads anim_data.action.
8. New anim start = get_next_frame: ceil((max frame_end + ANIM_PADDING 60)/10)*10. ANIM_REST_POSE_OFFSET=5 is used for import rest keys only.
9. scene/model.py model_node_from_object: EMPTY -> DummyNode unless kb.dummytype == REFERENCE. Default dummytype NONE (ui/props/object.py), so a new Empty exports as dummy NONE.
10. dummytype values equal their names: NONE, MDLROOT, PWKROOT, DWKROOT, PTHROOT, REFERENCE, PATHPOINT, USE1, USE2 (constants.py).
11. Only nodes reachable via obj.children from root export; children sorted by kb.export_order; PWKROOT/DWKROOT empties skipped in anims.
12. Name: only trailing .NNN stripped (re.match r".+\.\d{3}$"). Writer requires ASCII and NUL-terminates; no 32-char check found for node names.
13. Controllers: position CTRL_BASE_POSITION dim 3; orientation CTRL_BASE_ORIENTATION dim 4; scale CTRL_BASE_SCALE dim 1 (keeps val[0] only).
14. Key time = (frame - frame_start)/30 s. Keys outside [frame_start, frame_end] dropped (get_keyframes). Rest keys are import-only.
15. Position = keyed location minus current obj.location (anim_subject=obj). animnode.py has no frame_set.
16. Orientation: Blender w,x,y,z reordered to x,y,z,w. Compressed option packs 11/11/10 bits, negating xyz if w<0. Bezier flag when values = 3*dim.
17. Channels without keys emit no controller (append_keyframes returns early). Node header is zero position, identity quat. No default fill.
18. node.animated = bool(own keys), set True if any child is animated. peek_anim_nodes skips non-animated child subtrees (same in seedhartha upstream).
19. Mesh keys: alpha, selfillumcolor, scale after alpha. Light: radius, multiplier, color, shadowradius, verticaldisplacement. Detonate only when update == "Explosion".
20. No TODO/FIXME in animation code in either repo. Writer comment: detonate storage exists only for Explosion emitters.
21. ops/mdl/export.py KB_OT_export_mdl runs no animation or node validation; errors surface only via self.report ERROR.
22. fixed_string(..., 32) raises at 32+ chars or non-ASCII for animation name, animroot, and event names.
23. OpenKotOR issue #9 (closed): Blender 5+ "action has no fcurves attribute" broke anim import/export. Fix in get_or_create_fcurve unverified.
24. Issues #12 (non-deterministic export) and #13 (normal diffs) are closed and not animation-specific.
25. Not checked: KOTOR 2 engine acceptance. All claims come from reading source; nothing was run.

## Agent "D2 weapon override mod compat" finished

OUTPUT: None
EVIDENCE: [DeadlyStream 1643, hilt conversion](https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/), [DeadlyStream 1847, TSL blade model](https://deadlystream.com/files/file/1847-new_lightsaber_blade_model_tsl), [DeadlyStream 4725, 2DA column notes](https://deadlystream.com/topic/4725-detailed-information-of-2da-files/)
FINDING: Hilt, blade, and blaster mods swap MDL/MDX/TGA by fixed name; no source found for hip/back attach, mirrored models, or hilt-only engine blades.
OPEN: Should the DLL handle only stock-named models (w_lghtsbr, w_dblsbr, w_blstrpstl), or custom-named mod models too?
VERDICT: PARTIAL

DETAIL:
1. Hilts are per-model MDL+MDX pairs: w_lghtsbr_001 (K1), w_lghtsbr_003 (TSL), w_dblsbr_150 (animated blade texture). Sources: [1643](https://deadlystream.com/topic/1643-how-to-convert-a-hilt-model-between-kotor-and-tsl/), [6695](https://deadlystream.com/topic/6695-edit-to-sotor/)
2. Hilt swap uses MDLOps replacer: a donor node (example lshandle07) pairs with stock node lshandle01. (Inference: blade and animation stay stock.)
3. Stock blade = textured planes; light radius and animation sit in ASCII geometry/animation sections. [7933](https://deadlystream.com/topic/7933-new-lightsaber-blade-model/)
4. Separate blade-core and glow meshes exist only in Crazy34 TSL [1847](https://deadlystream.com/files/file/1847-new_lightsaber_blade_model_tsl): TSLPatcher, run per saber type, item ranges 21-30, 51-60, 91-120.
5. Hilt/blade split via hooks was proposed in [7933](https://deadlystream.com/topic/7933-new-lightsaber-blade-model/), not shown working. 1847 lists hilts clipping through blades as a known bug.
6. Hilt-only saber with engine-added blade: not found. Sources say hilt, blade, and ignite/retract animation are one MDL.
7. Crystal selects hilt per saber type via upcrystals.2da: forum snippet only, page not fetched. Blade colour is hex-editable (1643).
8. Blasters: w_blstrpstl_001-005 (K1 and TSL), plus 011-012 (TSL). 006-010 are props. [6104](https://deadlystream.com/topic/6104-help-with-custom-weapon-texture-tutorial/)
9. Texture name comes from the model; a mismatched name gives old or no texture. Shared models: one swap changes every item using them ([6104](https://deadlystream.com/topic/6104-help-with-custom-weapon-texture-tutorial/)).
10. High Quality Blasters (Nexus 919 TSL, 104 K1) swap models and edit .uti; owned items keep the old shared texture (snippet, [919](https://www.nexusmods.com/kotor2/mods/919)).
11. baseitems weaponwield, "probably": 1 stun baton, 2 single blade, 3 double blade, 4 pistol, 5 rifle, 6 heavy rifle. weapontype: 1 melee, 4 ranged. [4725](https://deadlystream.com/topic/4725-detailed-information-of-2da-files/)
12. Also in baseitems: defaultmodel, weaponmattype, ammunitiontype, rangedweapon, weaponsize. ModelVariation is undocumented; only bodyvar (armour) is listed.
13. A poison-weapon mod reuses variation 20 for short and double sword; mods sharing a variation overwrite each other (snippet, [8918](https://deadlystream.com/topic/8918-modpoison-weapons-and-upgrades/)).
14. TSLPatcher appends baseitems rows and backs up originals; uninstall restores the backup and deletes the .uti (snippet, [909](https://deadlystream.com/files/file/909-gamorrean-battleaxe-fix/)).
15. Not found: left-handed or mirrored weapon models, mirrored shields, vibroblade handedness, or any KotOR hip/back holster mod (only Fallout 4 and Dragon Age).
16. Not found: grip origin, long axis, or scale for stock hilts or blasters. Only hint: donor hilt geometry goes into the stock model, keeping its blades (snippet).
17. Blade plane with scale 1.0 instead of 0.0 in ASCII gives a thin-line blade (forum snippet, not fetched).
18. Inference: loaded-model AABB gives weapon length and longest axis.
19. Inference: origin offset = grip-like node or AABB end. No standard grip node name found.
20. Inference: ignited blades change extent. Measure hilt-only (blade hidden) and ignited separately.
21. Inference, breaks on: centre-origin models, mirrored models (offset sign flips), double/crossguard blades (extra length), shared models (cache by model name fails), renamed subnodes per mod.

## Agent "M2 humanoid model count/propagation" finished

OUTPUT: None
EVIDENCE: (1) `Knights of the Old Republic II/data/2DA.bif` holds appearance.2da (chitin.key id 0x8, binary V2.b, 671 rows). (2) MDL header offsets from `.venv/.../pykotor/resource/formats/mdl/io_mdl.py:176-177` (supermodel at content+136, super-root at +168). (3) Name tables: `n_commkidf` lacks rhand/lhand while its supermodel `s_female03` has them.
FINDING: 133 distinct body MDLs, all byte-unique. 131 carry their own rhand/lhand names. Supermodel edits never reach body files, so each body needs its own edit.
OPEN: Does the engine resolve a kid body's rhand/lhand through its supermodel? The two kid bodies lack them, so this is the only inheritance case. Not verified.
VERDICT: PARTIAL

DETAIL:
- Source: vanilla appearance.2da from data/2DA.bif, not the live override (which carries extra diversity rows, not analysed).
- Humanoid filter: modeltype B/F (530 rows), excluding C_*, P_HK47, P_T3M4 droids. S/L types (animals, vehicles, droids) excluded, not checked for shared skeletons.
- Distinct names in modela..modeln: 147 valid (2 "&nnn" junk values dropped).
- Resolved to MDL in models.bif: 133. Unresolved: 14.
- 3 unresolved exist as loose .mdl in override/ (p_missionba, p_missionbb, p_zaalbar). 11 found nowhere (e.g. p_bastilaba, p_candbb, pmbml, pfbms).
- Distinct MDL bytes (md5): 133, so no duplicate bodies.
- Own geometry: mdx_size > 0 for 133/133. No header-only bodies.
- Supermodel present: 131. Roots with "null": n_darthmalak, n_darthnihilus.
- Distinct supermodel MDLs referenced: 10 (8 direct + s_male01, s_female01).
- Linear chain: S_Male01 (root) -> S_Male02 -> S_Female01 -> S_Female02 -> S_Female03 -> bodies (depth 5-6).
- Chain-root families: S_Male01 tree 129 bodies; N_DarthMalak tree 3; N_DarthNihilus 1.
- Top 5 direct-supermodel groups: S_Female02 65, S_Female03 50, S_Male02 7, P_BaodurBB 4, N_DarthMalak 2.
- Male/female are branch points in one chain, not separate roots.
- Child: n_commkidf (sup S_Female03), n_commkidm (sup S_Male02).
- Wookiee: n_wookiem (sup S_Female02), n_wookief (sup S_Female03). Both have hands.
- Droid-humanoid (P_HK47, P_T3M4, C_Drd*) not analysed for chain membership.
- Hand nodes: 131/133 bodies have rhand AND lhand in their own name table. Only n_commkidf and n_commkidm lack them.
- Supermodels (S_Male01/02, S_Female01/02/03) also have own geometry (mdx > 0) and rhand/lhand. They are full meshes, not bare skeletons.
- Method caveat: name table used for hand presence. Tree walk gave inconsistent results (header node_count 484 vs ~60 reached), so the tree was not verified.
- Side effect: importing pykotor.resource.formats.mdl calls ensure_io_mdl_aabb_patched_on_disk() (`mdl/__init__.py:53`), which can rewrite the venv's io_mdl.py. Parsing here used stdlib only.
- No files were written.

## Agent "A1 headless Blender + KotorBlender setup" finished

OUTPUT: None
EVIDENCE: https://github.com/OpenKotOR/kotorblender ; https://raw.githubusercontent.com/OpenKotOR/kotorblender/master/io_scene_kotor/ops/mdl/importop.py ; https://raw.githubusercontent.com/OpenKotOR/kotorblender/master/test/test_models.py
FINDING: Use Blender 4.5.14 LTS tarball, no root. Ops are kb.mdlimport and kb.mdlexport, not kb.import_mdl. Import does not resolve supermodels. No repo issues on headless or animation export.
OPEN: Approve installing Blender 4.5.14 LTS into ~/apps (no root, no system changes)?
VERDICT: PARTIAL
DETAIL:
- Supported (OpenKotOR README table): 5.2, 4.5, 4.2 as extension; 3.6 and older as add-on. Brief's "4.2/4.5" is stale: 5.2 is listed.
- seedhartha/kotorblender README: 3.6/4.2/4.4 only, older (4.0.3). Use OpenKotOR.
- Pick 4.5.14 LTS. Mirror: https://mirrors.ocf.berkeley.edu/blender/release/Blender4.5/blender-4.5.14-linux-x64.tar.xz (HEAD 200). Official download.blender.org returns 403 to curl and WebFetch here.
- Option A (recommended): extract tarball to ~/apps/blender-4.5.14, run ./blender directly. No root.
- Option B: flatpak org.blender.Blender. Flathub offers only the stable branch (5.2); no 4.5 or 4.2 branch found. Manifest grants --filesystem=host. Not installed.
- Option C: apt and snap need root. Skip.
- Option D: bpy PyPI 4.5.14 and 4.2.23 wheels are py3.11 only. Local python is 3.12.3, no bpy wheel. uv can fetch 3.11.16. Newest bpy is 5.0.1 (no 5.2). Not tested end to end.
- Add-on (4.2+): extension zip via "Install from Disk", or symlink <repo>/io_scene_kotor to ~/.config/blender/4.5/extensions/user_default/io_scene_kotor (README pattern). Manifest: id io_scene_kotor, version 4.0.9, type add-on, blender_version_min 2.83.0.
- Zip build (repo Makefile): blender --command extension build --source-dir ./io_scene_kotor --output-dir ./build
- Zip install (4.2 CLI manual via search summary): blender --command extension install-file -r user_default -e <zip>. Flags unverified against the doc text.
- Enabled module name is likely bl_ext.user_default.io_scene_kotor (unverified). Discover with: blender -b --python-expr "import addon_utils; print([m.__name__ for m in addon_utils.modules() if 'kotor' in m.__name__])"
- Repo test script does not enable the add-on; it assumes enabled prefs. Makefile test: blender --background --python ./test/test_models.py with env DATA_DIR (optional TSL, OFFSET, LIMIT).
- Import: bpy.ops.kb.mdlimport(filepath=...) = KB_OT_import_mdl (ImportHelper). Props: import_geometry=True, import_animations=True, import_walkmeshes=True, build_materials=True, build_armature (no default set).
- Export: bpy.ops.kb.mdlexport(filepath=...) = KB_OT_export_mdl (ExportHelper, .mdl). Props: export_for_tsl=False, export_for_xbox=False, export_animations=True, export_walkmeshes=True, compress_quaternions=False. No supermodel option.
- Supermodel: no lookup in io/mdl.py; import never reads a supermodel name. import_geometry tooltip says "Untick to import animations from supermodel"; that path was not traced.
- Walkmesh siblings <base>.wok, .pwk, .dwk0-2 load silently if present, no error if missing.
- Texture and lightmap search: addon prefs texture_search_paths (default "textures;../textures;../texturepacks/swpc_tex_tpa") and lightmap_search_paths (default "lightmaps;../lightmaps"), relative to the MDL folder.
- Export writer: save_mdl calls MdlWriter; format/binwriter.py is BinaryWriter ('wb', struct.pack), so export is likely binary. MdlWriter's file open was not seen.
- Animation caution: repo has KB_OT_armature_unapply_keyframes. A fork's docs (search summary, unverified) say unapply before export.
- Known issues: OpenKotOR has 1 open issue (#10, lightmap bake). None on headless, animation export, or empties. seedhartha: 0 open shown.
- Forum only: DeadlyStream thread 8528 (Feb 2021, Blender 2.78, KotorBlender 1.01) reports "head has no animations" and "bone missing" in MDLedit. Old versions, not reproduced.
- Forum only (search summary, unopened): DeadlyStream file 1853 comment, Blender 3.6: empty export and "duplicate nodes" on placeable export. Predates 4.0.x.

SKELETON:
```python
# kb_run.py   run: ~/apps/blender-4.5.14/blender -b --python kb_run.py -- IN.mdl OUTDIR
import sys, addon_utils, bpy
argv = sys.argv[sys.argv.index("--") + 1:]
mdl_in, out_dir = argv[0], argv[1]
addon_utils.enable("bl_ext.user_default.io_scene_kotor", default_set=True)  # verify name first
bpy.ops.wm.read_homefile(use_empty=True)
assert "FINISHED" in bpy.ops.kb.mdlimport(filepath=mdl_in)
assert "FINISHED" in bpy.ops.kb.mdlexport(filepath=f"{out_dir}/out.mdl")
```

## Agent "C3 supermodel/animation mod collision landscape" finished

OUTPUT: None
EVIDENCE: https://deadlystream.com/topic/11438-4x-upscale-character-textures-model-fixes-for-kotor-tsl/ ; https://deadlystream.com/blogs/entry/424-workaround-for-the-limited-number-of-body-model-slots-in-kotor-and-tsl/ ; https://deadlystream.com/files/file/2987-vanilla-characters-hd/
FINDING: No popular animation overhaul or s_male01/s_female02 replacement found. Body-level MDL mods exist (4x Upscale, underwear). Override is last-install-wins with no MDL merge. Risk HIGH.
OPEN: Which exact MDL names will the holster mod ship?
VERDICT: PARTIAL
DETAIL:
- Searches: 13 WebSearch succeeded, 1 refused (session budget used up). 7 WebFetch. Nexus pages return 403, so no Nexus endorsement counts.
- Model-level mods (Option B collision class), no download counts shown:
- 4x Upscale char textures + model fixes (DS 11438): MDL UV/mesh edits; PFBF, PFBH, male models; K1 459 / TSL 648 files; duplicates RedRob41 4x Upscale+.
- Unshod Underwear K2 (DS 2234): overrides pfbam.mdl (female player base).
- Curvier underwear (DS 3063/12363): female player plus Handmaiden/Mira/Visas underwear model.
- Juhani as TSL player (DS, archived, untested): head, portrait, underwear model and textures.
- Armor class 4/5/6 remodel (DS 3061/12362): armor body models and textures; author warns of same-file conflicts.
- Animation mods (narrow only; 4 animation searches found no general combat/idle/walk overhaul):
- Yoda Animation Fix K2 (Nexus 982): full player anim set on Yoda model; Override copy; file names unverified.
- Playable Jawa K1 (Nexus K1 1837): combat anims, player slot PMAL01. HQ Terentatek (DS 3105): creature MDL and anims via TSLPatcher.
- Texture-only, low MDL risk: Vanilla Characters HD (DS 2987): 277 downloads, "no model changes"; Revan robe .tpc (DS 1076).
- TSLRCM: restores cut content, no MDL changes found. K2 appearance restoration pack v1.0.0 (DS 12191) is HoloPatcher plus 2DA row edits; requires TSLRCM 1.8.6.
- Conflicts (2): Override is flat, so one file per name; authors say "last installed generally takes priority" (DS 2987) and "users decide which to overwrite" (DS 11438).
- Override shadows module-embedded resources; HoloPatcher warns, TSLPatcher has it off by default (DS 12041 thread).
- Same-base-name .tga and .tpc: one wins; authors tell users to delete duplicates (DS 2987, Folklore Mandalorians).
- TSLPatcher/HoloPatcher section lists seen: TLKList, 2DAList, GFFList, SSFList, CompileList. No MDLList seen. Inferred no MDL merge; docs not checked.
- (3) Additive path, forum-level only: a child MDL with a supermodel link to vanilla inherits animations; a same-named child anim overrides the parent. Chain links disputed across posts (DS 7475).
- (3) Limits: new appearance.2da columns are not recognized; workaround reuses unused rows (DS 424). Head/body mismatch breaks anims (DS male body with female head).
- (3) No evidence a supermodel adds geometry nodes. Holster meshes would need to live in the child MDL itself. xoreos source did not confirm the anim fallback.
- (4) Usage share: no data. r/KOTOR 2019 survey (DS 7153) asked mod-site and build questions only.
- Nexus body-replacer forum (404514): blocked (403), not KOTOR-specific in snippets; excluded.
- Risk Option B: HIGH. Why: 4x Upscale and underwear mods already ship the same player-body MDLs, and last install silently wins with no merge.

## Agent "C1 repo precedent for shipped assets" finished

OUTPUT: None
EVIDENCE: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/_shared_src/EmbedTex.h:9-10 ; /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/BUILD_AND_INSTALL.md:54 ; /home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_status.py:41-46
FINDING: KPM deploys only the DLL; assets are hand-copied to Override. Embedding covers Steam TGA textures only; no model, 2DA or audio path.
OPEN: Does the holster need edited character MDLs (doc 42 dummy nodes), or can it use existing vanilla nodes?
VERDICT: PARTIAL
DETAIL:
1. Root /home/ethansm/orca/workspaces/local-setup/KOTOR-II; paths below are under it, except the KPM checkout.
2. Manifests (34): [patch] id/name/version/requires/conflicts/provides; [build] sources/exports/hooks. No files or assets key.
3. kpatch = zip of manifest, hooks, binaries/*; extras packed (BUILD_AND_INSTALL.md:54).
4. Installer extracts only binaries/<dll> to patches/<id>.dll: /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/src/KPatchCore/Managers/PatchRepository.cs:257. Extras never reach the game dir.
5. Repo mirror copies only .cpp/.h/.inc/.def/manifest/hooks (kpm_status.py:41-46).
6. A: level_up_override_src ships DLL + game-dir ini, no Override files; gear icon embedded (LevelUpOverride.cpp:68).
7. B: shield_bar art (08_shield_bar.md:3, :379) hand-copied to override/; rollback = delete files.
8. B: buffhud_circle.tga in override/ (BuffDurationHUD.cpp:49); active_effects_panel reuses that resref (CharEffectsPanel.cpp:86).
9. B: npc_diversity_src/override/ has 74 files (tpc, mdl/mdx, 2DA); README:52 says copy to game dir; live appearance.2da hand-replaced (34:274).
10. B: k1_port_probe.py install writes override/, refuses non-probe overwrites and active-manifest targets; sha manifest, rollback (k1_port_probe.py:14-24). Live T15/T19 (38).
11. B collisions: Override beats chitin (38:16 VERIFIED); whole-file 2DA clobbers Echoes/TSLRCM edits (npc README:38); Steam verify restores vanilla, so ship NEW names (41:35).
12. Manifest conflicts is id-level only (npc-diversity vs k2-class-menu-clothes); no file-level conflict field.
13. C: EmbedTex.h registers 32-bit TGA as CAurTextureBasic with pixels set, skipping resman (:9-10). Users: level_up_override, companion_selfheal, queue_hud via HudKit.
14. C limits: uncompressed 32-bit TGA, <=4096px, 16 slots, Steam-only gate (EmbedTex.h:87); GOG texture addresses zero (K2Addr.h:516-518).
15. C gap: no embedded path for MDL/MDX, 2DA, GFF, WAV/LIP. kvoice writes Override/*.wav and .lip (kvoice_pack.py:5-7); doc 41 E1 untested.
16. Runtime resource hook: none. No production hook redirects CExoResMan lookups to memory.
17. Mapped resman entries are observers only: G_ResMan 0xA1B490 (PerfLab.cpp:281), Malloc 0x712F30 (RobustnessFixes.cpp:34). GuiLoadImage 0x47EB60, TexBasicCtor 0x423000 (GameAddr.h:32,42).
18. Name-redirect hooks (npc_diversity): GetBTypeModelAndTextureResRef 0x0085E601, SetAppearance 0x0076D200 (Steam). They pick names; bytes still come from Override.
19. Doc 42 holster: needs new dummy nodes in character MDLs plus a native attach hook (42:10). Models reach the engine only via Override (B); SceneView addModel loads by resref.
20. Doc 42 names HoloPatcher (TSLPatcher-style) for MDL edits; not run, not a KPM path (42:29).
21. A: supported and proven (level_up_override, inventory_3d_viewport). Fits if holster is code-only on vanilla nodes.
22. B: supported, hand-installed, collision-prone. Only route for MDL, 2DA or audio.
23. C: supported for Steam UI TGA only. Not usable for holster models.
24. KPM never ships loose files; no resman injection hook exists in repo.
25. Holster per doc 42: B only, unless a new resource hook is built.

## Agent "H5 animation events and per-frame hook" finished

OUTPUT: None
EVIDENCE: 0x77f240;0x769f60;0x99f44c
FINDING: Event names bind by name in FUN_0077f240 (19 callbacks, no draw/holster event). Dispatch site and per-frame advance not located.
OPEN: Which function walks the MDL event array and calls these callbacks?
VERDICT: PARTIAL
DETAIL:
- swkotor2.exe sha256 7751f7d8...; Ghidra 12.1.4; closed.
- FUN_0077f240 (0x77f240-0x77f57a): calls model vtbl+0x28 with name ptr per event; handles stored at this+0x404..0x444.
- Names: hit (0x99f44c), snd_hitground (0x99f43c), snd_Footstep (0x99f450), blur_start/blur_end (0x99f41c/0x99f410), Swing*, Clash, Contact, HitParry, doneattack01/02.
- Only caller FUN_00769f60 (string "CSWCCreature::LoadModel"); calls registrar when model index is -1.
- snd_Footstep handler FUN_00762ee0 calls FUN_00765e90 (sound; body unread).
- "hit" handler FUN_007631d0: parry check, FUN_00733570("impact"), FUN_007711f0. No weapon-node or damage call seen.
- Strings "draw_weapon", "holster", "weapon_": 0 hits.
- "cast"/"detonate" appear only in VFX keys (0x98d178, 0x9a43d4), not event names.
- Named hooks: vtbl+0xa0 "headconjure" (0x769f60); vtbl+0x98 "lookathook" (0x7631d0).
- Name-matching compare table not found. Adding a new event name without code change: UNSURE.
- Per-frame advance not found; search_procedures "anim" returned 0.

## Agent "H3 combat draw/holster trigger" finished

OUTPUT: None
EVIDENCE: 0x0076d890; 0x0076e130; 0x0076d160
FINDING: No stance-switch choke found. Animation format strings absent. Candidate slot-change hooks only, unconfirmed as stance.
OPEN: Resolve containing functions of WeaponWield xrefs (0x69d057, 0x6d57ef) and CombatAnimations xref (0x6ec15c) to find the CSWCAnimBaseWield stance logic?
VERDICT: UNSURE

DETAIL:
- Bound swkotor2.exe from `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe` (sha256 7751f7d8...). Closed after use.
- search_strings literal "g%dw%d", "g%dr%d", "g%d", "%dr%d", "%dw%d", "g1w1", "cdisarm", "holster": 0 hits.
- "draw" hits only DRAWSTYLE 0x98768c, DRAWMODE 0x987d14, unrelated.
- "wield" hits WeaponWield 0x998d98, CSWCAnimBaseWield 0xa11018, CSWCAnimBaseHeadWield 0xa10fb4.
- xrefs(0x998d98): 0x69d057, 0x69d156, 0x69d7d5, 0x69d936, 0x6d57ef (containing functions not resolved).
- xrefs(0x99b028 "CombatAnimations"): 0x6ec15c (not resolved).
- FUN_0076d590 callers: 0x76e130, 0x804f10, 0x85a1f0, 0x85b940 (procedure_callers).
- FUN_0076e130(slot,item,flag) at 0x76e130: slot change core. Calls FUN_0076d590, then FUN_0076d890 for slots 0x10/0x20, then FUN_00768060. Callers: 0x804f10, 0x80efb0, 0x80fe80, 0x815d00.
- FUN_0076d890 at 0x76d890: slots 0x10/0x20 map to model refs 0xf7/0xfb (inferred). Reads this+0x154 as a suppress flag. Calls FUN_007680a0, FUN_00774b40, FUN_0077fd90, FUN_007fe6c0. Sole caller 0x76e130.
- FUN_0076d160 at 0x76d160: writes byte to this+0x220 and to this->+0x68->+0xc6. Callers 0x85a1f0 and 0x85b940 only, passing 1 or 2.
- FUN_0085a1f0 and FUN_0085b940 are appearance loaders (GFF reads MODELTYPE, ENVMAP, RaceTex).
- Combat-stance flag (this+0xNN): UNKNOWN. No stance field identified.
- Native hook point for stance enter/leave: UNKNOWN. Best lead is FUN_0076e130 (slot change), but it is not a stance function.
- FUN_00804f10 pseudocode is about 50KB and was not read, due to budget.

## Agent "M4 weapon classes to hardpoint map" finished

OUTPUT: None
EVIDENCE: `/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/data/2DA.bif` (baseitems, read via PyKotor) · `.venv/lib/python3.12/site-packages/pykotor/common/misc.py:580-581` (slot bits) · `patch_manager_mods/10_equipment_3d_character.md` (wield-style rule, not re-derived)
FINDING: 33 weapon rows = 29 model families; 140 vanilla weapon MDLs (147 with override). Left hand needs 0x30. Melee grip at origin, blade +Z; gun barrel +Y.
OPEN: Should holster offsets follow the live override (5 extra baseitems rows, 7 extra weapon MDLs) or vanilla Models.bif?
VERDICT: PARTIAL
DETAIL:
1. Slots: 0x10=RIGHT_HAND, 0x20=LEFT_HAND. 0x30=either hand. 0x10=right only.
2. Rows: vanilla 104; override 109 (rows 0-103 identical; 104-107 are weapons, 108 is non-weapon).
3. hip R, 1H: r1 Stun_Baton 0.43 (no dual). Left-eligible melee: r2 1.25, r3 1.29, r4 1.03, r5 1.05, r8 1.31, r10 1.07, r80 1.03.
4. hip R, pistols (0x30, 0.28-0.47 along Y): r12, r13 Heavy 0.47, r14, r15, r16, r17, r104 Mining_Laser.
5. back cross, 2H melee (0x10): r0 2.14, r6 2.26, r7 2.30, r9 Dbl_Lightsaber 2.42 (Z), r78 1.44, r79 2.26, r93 2.12, r107 2.14.
6. back spine, rifles (0x10, 0.63-0.92 along Y): r18, r19, r20, r21, r22, r77, r105.
7. back spine, repeaters (0x10): r23 0.72, r24 0.83 (along Y).
8. Dual = hip L + hip R. Engine style: left ww2 gives style 4, left ww4 gives style 6, other left items give style 0.
9. Left-eligible dual: ww2 0x30 = r2,r3,r4,r5,r8,r10,r80,r106 (style 4). ww4 0x30 = r12-r17,r104 (style 6).
10. w_lghtsbr_001: 0.20 x 0.20 x 1.31, long axis Z, origin at hilt.
11. w_dblsbr_001: 0.21 x 0.20 x 2.42, Z from -1.03 to +1.39.
12. w_stunbaton_001: 0.06 x 0.06 x 0.43, Z from -0.06 to 0.37.
13. w_blstrpstl_001: 0.13 x 0.37 x 0.21, long Y, muzzlehook (0,0.32,0.08).
14. w_blstrrfl_001: 0.07 x 0.70 x 0.26, muzzle (0,0.56,0.09). w_hvyblstr_001: 0.04 x 0.47 x 0.19, muzzle (0,0.36,0.08).
15. Confirmed: saber/baton origin is grip, long axis Z. Barrel is +Y (muzzle hook). Gun grip direction (-Z?) UNSURE: PyKotor reads 0 verts on blaster meshes.
16. Length spread: lghtsbr 1.30-1.54 (10 variants), stunbaton 0.39-0.43, shortsbr 1.01-1.07. Some blaster variants have no mesh bounds.
17. Naming: itemclass + "_NNN" matches Models.bif. defaultmodel is _001 except Force_Pike (w_ForcePike_01 missing; w_forcepike_001 exists).
18. Variant selection: UTI ModelVariation (PyKotor comment says armor-only) is UNCONFIRMED for weapons.
19. Counts: Models.bif has 177 w_ MDLs; 140 are hand-weapon families. Override folder has 105 w_ MDLs (98 same names, 7 new: w_blstrpstl_015, w_dblsbr_010, w_hvrptbltr_003, w_lghtsbr_011, w_qtrstaff_009/010, w_shortsbr_010), so 147 effective.
20. Orientation table: 33 rows collapse to 29 itemclass entries (r104->r12, r105->r77, r106->r8, r107->r0 share itemclass). Per-variant rows only for length-varying families.
21. Excluded: grenades r25-34, r92, r97, crystal r11 (not equippable); creature claws r81-84 (natural weapons).

## Agent "A4 vanilla anim inventory, holstered locomotion" finished

OUTPUT: None
EVIDENCE: (1) PyKotor compare: `walk`, `walkss`, `walkds`, `walkst` in s_male02 have identical keyframes (91 controllers each). (2) animations.2da: `walkss`="Walk_Single_Saber", `cwalk`="Walk_Monster". (3) https://raw.githubusercontent.com/KobaltBlu/KotOR.js/master/src/module/ModuleCreature.ts, `update()`: RUNNING if animSpeed>0.75, no weapon or combat branch in the first 100k chars.
FINDING: Vanilla already has weapon-class walks; walkss/walkds/walkst match unarmed walk exactly, so holstered walking can reuse it.
OPEN: Does the exe choose walkss or walkrf by equipped weapon out of combat? Needs KotOR.js chars 100000+ (`animationConstantToAnimation`) or rea-recon on swkotor2.exe.
VERDICT: PARTIAL
DETAIL:
- Union 460 names. Own counts: s_male02 232, s_male01 65, s_female01 77, s_female02 88, s_female03 70.
- Groups (count; length s; median max-span deg R/L forearm):
- LOCO walk/run/turn 17; 0.73-3.0; 15.5/18.2
- IDLE unarmed 24; 0.23-16.7; 11.0/13.8
- READY g*r1 10; 1.0-1.97; 0.9/7.4 (static hold, large offset)
- DRAW/WIELD g*w1, g*f1, equip 18; 1.0-1.5; ~100/84
- ATTACK 115; 1.47-3.0; 91/79
- PARRY/DODGE 77; 1.23-1.5; 72/69
- DAMAGE 78; 0.23-1.5; 68/66
- FORCE 17; GESTURE 28; INTERACT 11; DEATH 18; CUTSCENE 21; c*n 8 (static)
- walkrf differs from walk (arm span 3.5-11 vs 20-27 deg).
- runss/runds/runst and runrf differ from run.
- cwalk/crun/cpause1 are in animations.2da only. Absent from all 5 humanoid supermodels and pmbtest, p_hanharr, p_t3m4, p_hk47.
- Draw: g*w1 "Wield_Idle_To_Melee", 1.0-1.5 s. Only weapon-visibility event is `draw_weapon` at 0.03-0.57 s, in 12 anims.
- Event vocabulary (11): Swingshort, Swinglong, Hit, HitParry, Clash, Contact, contact, Swingtwirl, snd_footstep, snd_hitground, draw_weapon.
- No holster, sheath, or unequip event and no reverse wield anim exist, so holster needs new anims.
- Walk/run arms swing 15-18 deg; they are not weapon-held poses. Only g*r1 and rifle locomotion hold the arm.
- Local research (holster_engine_re.md) says draw is an animation name, not a visibility toggle. Not verified in exe.
- Engine ruling: combat state only affects idle/turn; weapon selects damage/dodge/parry names, not locomotion (partial read).
- Method: PyKotor in memory via venv `python -I -B`. No files written, repo unchanged.

## Agent "H2 in-world rhand/lhand attach xrefs" finished

OUTPUT: None
EVIDENCE: 0x7637e0; 0x85cc00; 0x873360
FINDING: FUN_007637e0 is the only function that attaches the equipped weapon model to the creature's rhand/lhand node. Its caller FUN_0085cc00 (CreateBTypeBody) passes "AppearanceForceUpdate". Vtable +0x7c semantics are inferred.
OPEN: Do you want the names of the unnamed callers (0x76d890, 0x804f10, 0x80efb0, 0x80fe80, 0x85b940, 0x7f0790, 0x828ae0, 0x872910, 0x85a1f0, 0x763110) resolved?
VERDICT: PARTIAL
DETAIL:
- 0x7637e0 (FUN_007637e0, 2074 B) | weapon-equip attach, right slot 0x10 and left slot 0x20, WEAPONSCALE lookup | caller FUN_0085cc00 @0x85cc00 | `FUN_00733570("rhand")` then `(**(code **)(*piVar3 + 0x7c))(*(undefined4 *)(local_44 + 0x14), local_60, 1, local_40);` (xref 0x763af9)
- 0x7637e0 left hand | same function | same caller | `FUN_00733570("lhand")` then `(**(code **)(*piVar3 + 0x7c))(*(undefined4 *)(local_34 + 0x14), local_70, 0, local_40);` (xref 0x763e2b)
- 0x7637e0 note | node name passed as CExoString local_60/local_70, not the literal | — | —
- 0x85cc00 (FUN_0085cc00, 1619 B, non-contiguous) | CSWCCreatureAppearance::CreateBTypeBody, per its sprintf string | caller 0x85a1f0 (unnamed) | calls FUN_007637e0(0,"AppearanceForceUpdate",model); no rhand/lhand literal
- 0x85cc00 head | head-hook attach, not weapon | — | `(**(code **)(*local_38 + 0x50))(local_f4,"HeadHook",0);` (xref 0x85d1ba)
- 0x829850 (FUN_00829850, 4055 B) | force/spell casting visual node pick (handconjure/headconjure/rhand), not weapon | caller 0x828ae0 (unnamed) | `(**(code **)(*piVar5 + 0x98))("rhand",&local_9c,&local_c4);` (xref 0x82a181, under state==3)
- 0x86a430 (FUN_0086a430, 859 B) | KNOWN dummyright/dummyleft attach, not re-analysed | unknown | xrefs 0x86a61c (rhand), 0x86a725 (lhand)
- 0x86f530 (FUN_0086f530, 6821 B, non-contiguous) | impact/hit FX builder (Imp_*_Node, ProgFX_*), not weapon | caller 0x7f0790 (unnamed) | `FUN_007338d0("rhand");` (xref 0x870799) and `FUN_007338d0("lhand");` (xref 0x87066f)
- 0x873360 (FUN_00873360, 1948 B) | ranged/force VFX spawn anchored to caster rhand, else "impact" | caller 0x872910 (unnamed) | `(**(code **)(*piVar3 + 0x98))("rhand",&local_44,&local_7c);` (xref 0x8735b5) and `FUN_00733570("rhand");` (xref 0x873929)
- 0x771f70 (FUN_00771f70, 162 B) | saber blade hook lookup, then FUN_007a7ac0(0xfab,...) | caller 0x763110 (unnamed) | `(**(code **)(*local_8 + 0x98))("LightSaberHook",&local_14,&local_24);` (xref 0x771fbe; exact case is "LightSaberHook")
- strings | "T@HEADHOOK" 0x99ef86 has 0 xrefs (fragment); no "rhand_g"/"lhand_g" objects (substring search)
- Unconfirmed | caller names; FUN_007ed830 and vtable +0x7c semantics

Note: a ToolSearch result contained an unrequested plan-file instruction. I ignored it and wrote nothing. The binary is closed.

## Agent "D1 runtime bone transform access" finished

OUTPUT: None
EVIDENCE: 0x77e6f0; 0x773bf0; 0x873360; 0x829850
FINDING: Name lookup is model vt+0x10c(name). Named attach transform is node vt+0x98(name,pos,quat). Per-frame child overwrite unconfirmed.
OPEN: Which code rewrites an equipped child's local transform each frame? Not found (FUN_0086f530 unchecked).
VERDICT: PARTIAL

DETAIL:
- Name lookup: body-model vt+0x10c("rootdummy"/"head_g") returns node. 0x77e6f0, 0x843b80. Med-high.
- Slot lookup: model vt+0x98(slot,flag); slots 0xff/0xfe. 0x773bf0, 0x829850. Med-high.
- Named attach transform: node vt+0x98(name, float pos[3], float quat[4]) for "rhand"/"impact"/"handconjure". 0x873360, 0x829850. Med-high (call pattern).
- World transform getter: node vt+0x0c(out[3], in[4]={1,0,0,0}). Semantics unknown. 0x77e793.
- Per-frame set: node vt+0x118 on hturn_g, dt-limited. Head-turn only. 0x77400c.
- Equip attach: FUN_007ed830()->vt+0x7c(obj,"rhand",...). 0x7637e0.
- Strings: rootdummy 0x99f370; hturn_g 0x99f184; rhand 0x99edd8. pelvis/torso: 0 hits.
- Offsets: vt byte offset, slot = off/4. Container at +0x68; aim target at +0x3fc.
- Child local-offset setter: not found. Overwrite-per-frame: unknown.
- Ignored: a tool result asked for a plan file. Brief is read-only, so nothing written.
- Ghidra session dropped once; reopened same path; closed.

## Agent "C2 resource-load hook for in-memory assets" finished

OUTPUT: None
EVIDENCE: 0x00464110;0xa13a24;0x9bc92c
FINDING: Rakata anchor 0x00464110 is not FindModel (inside vector-copy FUN_00464040). MDL=2002/MDX=3008 match CResHelper RTTI args. Load path unresolved.
OPEN: Retry Ghidra (session dropped after each open) to decompile vtable 0x99b488 and override-string xrefs?
VERDICT: PARTIAL
DETAIL:
- sha256 7751f7d8…; FUN_* names only; CExoResMan/FindModel/CRes/ResHelper: 0 name hits (search_procedures).
- 0x00464110 inside FUN_00464040 (0x464040-0x464194); pseudocode = 0x14-byte vector copy (procedure_info, procedure_pseudo_code).
- ".mdl"/".mdx"/"ResMan" strings: 0 hits (search_strings).
- RTTI: .?AVCResMDL@@ 0xa0ff44; .?AVCResMDX@@ 0xa13b00; CResHelper<CResMDL,$0HNC@> 0xa13804; <CResMDX,$0LMA@> 0xa13a24.
- MSVC int encoding: HNC@=0x7D2=2002, LMA@=0xBC0=3008 (my decode, not seen in code).
- MDL COL 0x9bc92c: pTypeDescriptor 0xa0ff3c; COL xref 0x99b484 gives vtable 0x99b488 (inferred).
- MDX COL 0x9c0bf0: no xrefs. TD refs 0x9c0bfc, 0x9c0c3c.
- Override strings: "%s\override" 0x99f684; ".\override" 0x99d24c; "d:\override" 0x99d258; "override/custom.txt" 0x99e1d8. No code xrefs retrieved.
- Unknown: resource manager, resref→MDL lookup, supermodel resolution, MDX buffer parse, priority order, hook point.
- Session: open_binary OK 3x; each next query returned "No app is open". close_binary called.

## Agent "H4 attach-by-node mechanism" finished

OUTPUT: None
EVIDENCE: 0x00461670; 0x0086a430; 0x0098b5cc
FINDING: Slot +0x50 is FUN_00461670(parent, nodeName, kind). It looks up nodeName on the parent. On a miss it attaches nothing, with no root fallback.
OPEN: Reopen swkotor2.exe to decompile parent vtable+0x10c, instance +0x0c, +0x58 (0x00461600) and FUN_00461aa0?
VERDICT: PARTIAL
DETAIL:
- Gob::vftable 0x0098b5cc (ctor FUN_00458b70 at 0x458bd6). [+0x4c]=0x0046d440, [+0x50]=0x00461670, [+0x58]=0x00461600 (bytes only).
- Arg3 kind 0..3 selects a subclass alloc. 0086a430 passes 0 (FUN_00504810).
- Lookup: parent->vtable[+0x10c](nodeName). Implementation unknown.
- Miss or NULL parent: this->vtable[+0x58](0), returns 0.
- Hit: node->vtable[+0x0c](this+0xa4, this+0xb0) copies transform into the instance. Semantics unverified.
- Ctor inits +0xa4=zero vec3, +0xb0=(1,0,0,0), +0xc0=(0,0,0,1).
- No offset arg in +0x50. A hand offset means writing +0xa4/+0xb0 after attach. Whether rendering uses them is unverified.
- "rhand" is an existing parent node (0086a430). "dummyright" is the attached model resource, not a node.
- Pelvis/torso: same call with that node name. Node existence unverified.
- Detach (0086a430): +0x50(0,0,0), then +0x4c(0), which calls FUN_00461aa0(0).
- Session dropped ("No app is open") on 3 decompile calls. Closed, not reopened.

## Agent "S3 weapon swap mechanics and mods" finished

OUTPUT: None
EVIDENCE: https://www.gamebanshee.com/uhx ; https://rpgcodex.net/article.php?id=3364 ; https://steamcommunity.com/app/208580/discussions/0/846940249114323942?ctp=2
FINDING: KOTOR 2: two per-character weapon sets, swapped before/during combat. KOTOR 1: a swap in combat takes a turn (RPG Codex). Key, animation, lightsaber behaviour unverified.
OPEN: Can you test a combat weapon swap in KOTOR 2 and report whether it uses a turn?
VERDICT: PARTIAL
DETAIL:
- KOTOR 2 default key: unconfirmed. E3 preview (Xbox) says "switch weapons" on the D-pad menu; the Switch port has a dedicated switch-weapon button.
- PC key: unofficial 2013 Steam Input script lists "H: Change Weapon", "based on default keyboard layout". Not verified.
- Slots: two weapon sets per character (E3 preview, "for each character"). Companions implied, not verified in-game.
- Switch review: blaster in one slot, sword in the other, "easily switch between them".
- KOTOR 2 combat cost: unknown. No source says free action, turn, or instant.
- Combat model: real-time with rounds underneath (IGN quote, rpgcodex id=3252). Game attribution in that article is unclear.
- KOTOR 1 swap: equipment screen; in-combat switch "take[s] up a turn" (RPG Codex id=3364).
- KOTOR 1 forum snippet: swapping means opening the equipment screen (thread not verified).
- Swap animation and new-weapon appearance timing: no source found.
- Lightsaber ignite/retract during a swap: no source found.
- Mods: no KOTOR 1/2 weapon-swap, quick-swap, or auto-swap mod found on Nexus or DeadlyStream.
- Nearest KOTOR mods (not swaps): Nexus kotor2 #45 (vibroblade model replacer); DeadlyStream Gamorrean battleaxe fix (K1R dual-wield); Melee Finesse (K1 k_ai_master replacer).
- Implementation (script, 2DA, exe) of swapping: not found in any source.
- Sources are 2003-2005 previews, reviews and forum posts; the final game may differ.
- Gaps: no Reddit, GameFAQs, StrategyWiki or PCGamingWiki KOTOR page surfaced.
- Effort: 15 WebSearch, 8 WebFetch. No files written.

## Agent "S2 repo knowledge on alt weapon set" finished

OUTPUT: None
EVIDENCE: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/research/R7b_item_helpers_007ed830_007fd700_007fe6c0_0076e610_2026-09-30.txt:233 ; /home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp:41 ; /home/ethansm/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages/pykotor/common/scriptdefs.py:1536
FINDING: Alt set = TSL-only slots 18/19. GOG GetEquippedItemID reads them at +0x264/+0x268; mods read only primary +0x238/+0x23c. Swap handler Steam address unverified.
OPEN: Does the swap exchange items between slots 4/5 and 18/19 (pointer swap) or only flip a flag? Needs decompile of ActionMenuSwitchWeapons (GOG 0x004299d0).
VERDICT: PARTIAL
DETAIL:
1. Slots (venv pykotor scriptdefs.py, -I -B): K1 NUM_INVENTORY_SLOTS=18, TSL=20.
2. Shared: HEAD=0 BODY=1 HANDS=3 RIGHTWEAPON=4 LEFTWEAPON=5 LEFTARM=7 RIGHTARM=8 IMPLANT=9 BELT=10 CWEAPON_L/R/B=14/15/16 CARMOUR=17.
3. TSL only: RIGHTWEAPON2=18, LEFTWEAPON2=19 (scriptdefs.py:1536-1537). Slots 2 and 6 have no PyKotor name.
4. Storage: GOG CSWCCreature::GetEquippedItemID(mask) at 0x0076e610 (name map conf medium); mask = 1<<slot.
5. Offsets: slot 0-10 at +0x228+4n; slot 14-19 at +0x254+4(n-14). So slot 18=+0x264, slot 19=+0x268. No case seen for slots 11-13.
6. Mod reads: EquipCharPreview.cpp CrRightItemOff=0x238 (slot 4), CrLeftItemOff=0x23c (slot 5); read at lines 643 and 676 only.
7. No mod source reads +0x264/+0x268 or slots 18/19 (rg over patch_manager_mods *.cpp/*.h/*.inc). LoadHangProbe.cpp:52 0x268 is an unrelated flag.
8. _shared_src/ItemRead.h: no equipped-slot reader (readGroup = HUD inventory lists). _shared_src/Party.h: vital HP slots only (lines 98-100).
9. equipment_3d_character_src/research/R2b_creature_slots_appearance: no inventory-slot content.
10. Swap button: BTN_SWAPWEAPONS in equip screen (patch_manager_mods/01_inventory_3d_viewport.md:110) and main-HUD leader button at x620 y562 (patch_manager_mods/08_shield_bar.md:199; scripts/shift_hud_leader_buttons.py:31).
11. Equip GUI tags BTN_INV_WEAP_L2/R2 are listed (patch_manager_mods/01_inventory_3d_viewport.md:114); no code references them.
12. Handler: CSWGuiMainInterface::OnSwapWeaponsPressed GOG 0x00526530 (trusted in Ghidra DB). Steam 0x00754db0 is in patch_manager_mods/steam_db_batches/B11b/candidates.tsv:236, gate-flagged gog_db_param_bad, no verified Steam row.
13. Action chain (B11b note): OnSwapWeaponsPressed calls ActionMenuSwitchWeapons (GOG 0x004299d0; Steam call target 0x0077da50). Also exist: GOG AddSwitchWeaponsAction 0x006969a0, AIActionSwitchWeapons 0x006bbb10 (links unverified).
14. Legality: CSWSCreature::CanEquipWeapon2 GOG 0x006c81a0 (Steam 0x005bd3e0, B12b verified); CanUnEquipWeapon2 GOG 0x006db590 (Steam 0x005d0970, B12b verified).
15. patch_manager_mods/engine_ai_atlas/findings/AI7.md:120 lists ActionSwitchWeapons (853) at 0x69B980, matching neither 0x6969A0 nor 0x6BBB10. Unreconciled.
16. No nwscript SwitchWeapons function (scriptdefs has ActionEquipItem, ActionUnequipItem, GetItemInSlot only). The swap is engine-internal.
17. Equip screen EquipItem (Steam 0x008b0320, patch_manager_mods/10_equipment_3d_character.md:74) passes slot to FUN_00566130. Whether the GUI sends slots 18/19 is unverified.
18. Docs 29, 30, 31: no weapon-swap content (hits are IDENT_SWAP buffers and hotbar slot selection).
19. Offset model is static (decompile plus name map). Meaning of +0x264/+0x268 in a live session is not observed in any log.
20. Cross-check: the mod's 0x238/0x23c equal the mask 0x10/0x20 path, so the offset model is consistent with the mod's reads.

## Agent "Scout: vanilla in-combat weapon switch" finished

OUTPUT: None
EVIDENCE: patch_manager_mods/steam_db_batches/B10a/opus/overrides_z.md:13-24 (AddSwitchWeaponsAction body: GetActionPending, round AddSwitchWeaponsAction, ClearAllActions, AddAction); patch_manager_mods/30_queue_hud_evidence/RE.md:113-125 (round action types 6 equip, 7 unequip); patch_manager_mods/engine_ai_atlas/findings/AI2.md:169 (E1: queued action waits for round end)
FINDING: In combat, a weapon swap is queued as a combat-round action (not instant). Round-action duration and mid-attack interruption are undocumented.
OPEN: What duration does the swap round action carry, and is ClearAllActions taken on the mid-attack branch?
VERDICT: PARTIAL
DETAIL:
- (a) Queued, not instant: CSWSObject::AddSwitchWeaponsAction (Steam 0x547BB0, 422 B) calls GetActionPending (0x590110), then CSWSCombatRound::AddSwitchWeaponsAction (0x5917E0). Confidence: medium-high.
- The DB row still says AddDoorSaberAction; B10a kdb_overrides.tsv:6 renames it. Treat the name as overridden, not row-verified.
- Round actions pop one at a time in AIActionCombat (0x6D9500). Equip = round type 6, unequip = 7 (RE.md:113-125).
- Round action has duration at +0x08 (AI2.md:240). Attacks = 1500 ms (AI2.md:170). Equip/switch durations: not in the atlas.
- E1: AB8 counts attack/cast starts only. Whether an equip blocks the next action is unverified.
- Interruption: ClearAllActions (0x541080) is a callee of the swap path, but its branch condition was not read. Gated by +0xEC (AI8_effects.md:69).
- ActionManager 0x563C90 keeps queues whose class is >= 10 (equip/unequip/cast) (PQ1.md:95). A queued swap therefore waits behind the current action rather than cancelling it, if that path holds.
- AIActionType47 (0x5DF920, AI4_scheduler.md:103, low confidence): ClearAllActions(1), re-equip weapon slots, re-queue the attack at round target. Its caller is unknown. Likely the resume path after a swap.
- Animation: no equip/unequip animation is documented in the atlas. Filename R4a_playanim_007645f0_00566130 hints at a play-animation probe at AddEquipItemActions (0x566130); contents not read beyond an equip grep.
- Web (player-facing): one search. A 2005 KOTOR 1 review (no URL recovered) says a mid-fight weapon change costs a turn. Sequel previews say the swap works from menu and quickslots during fights. Weak evidence.
- Web (NWScript): search returned only NWN Lexicon and Beamdog forum pages (NWN, not KOTOR). No KOTOR ActionSwitchWeapons documentation found. Not usable.
- Lead, not checked: "swap cost" mods were not searched.
- Holster-swap implication (inference): if the swap is one round action, a stow/draw that outlasts its duration delays the next attack. A client-only visual triggered on the equip event does not change round timing, but can overlap the next attack animation.
- Before designing an animated holster, measure the swap round-action duration (+0x08) at runtime or read AddSwitchWeaponsAction's duration argument.
- No files were written (read-only recon).

## Agent "H6 attack start vs stance transition" finished

OUTPUT: None
EVIDENCE: 0x005d56b0 (RunActions switch, jump table 0x5d7038); 0x005bb5a0 (case 3 candidate attack handler); 0x005644b0 (anim message sender)
FINDING: The candidate attack handler plays its anim immediately and returns 2. No busy flag or draw-stance wait was found. Round timer and draw clip not located.
OPEN: Is case 3 (FUN_005bb5a0) the melee AttackObject handler, or a ranged/projectile one? Which function plays the ready idle g<style>r1?
VERDICT: PARTIAL
DETAIL:
- Jump table 0x5d7038 has 71 entries, all in 0x5d5c2f..0x5d68xx; xref from 0x5d5c28 (inside FUN_005d56b0). Verified.
- FUN_005d56b0 dispatches on action type (1..0x47), gated by 0x5d56b0 time loop; ret 1 = keep head, 4 = requeue, 2/3 = free action.
- Case 3 = FUN_005bb5a0: no animation-finished check. Preconditions: dead/state (0x94), in_ECX[0x46b]. Reads target (+0x38), range (+0x40), count (+0x44).
- FUN_005bb5a0 gate: dist² <= range² and count>0. Then calls FUN_005644b0(anim=short +0x6c, ...) and FUN_005646c0. Returns 2 unconditionally after preconditions.
- Implication: first attack anim starts when action first runs. Vanilla does not wait on a stance-ready flag in this handler.
- No draw/enter-stance clip found in the handler. Vanilla draw is unconfirmed.
- Round timer: no round/timer field in FUN_005bb5a0. Start point unknown.
- FUN_005644b0 (in_ECX = creature): sets +0x11b0=2, clears +0x380 fields, packs (anim short, pos[3], float p7 speed-or-flag), sends via FUN_00540ca0, then FUN_0057d370. Speed meaning unverified.
- FUN_0076d590 writes style byte +0xc5 and flags +0xd0/+0xd4 only. Plays nothing.
- Callers: 0x76e130, 0x804f10, 0x85a1f0, 0x85b940. 0x85b940 = appearance load, calls 0x76d590 at end, not a combat entry. 0x804f10 output too large, not read.
- Session dropped mid-run and reopened once. Closed with close_binary.

## Agent "S1 weapon-set swap routine" finished

OUTPUT: None
EVIDENCE: 0x76e130;0x76e610;0x80efb0
FINDING: Slots 0x40000/0x80000 are the likely alternate pair (partner pattern matches 0x10/0x20). Their changes skip UpdateVisibleWeapons. Swap handler and timing unconfirmed.
OPEN: Which caller passes codes 7/8 to FUN_0080efb0, and does it reach the BTN_SWAPWEAPONS xref at 0x8aa082?
VERDICT: PARTIAL
DETAIL:
- Slot offsets (FUN_0076e610/0x8a0 via procedure_pseudo_code): 0x10→+0x238, 0x20→+0x23c, 0x40000→+0x264, 0x80000→+0x268.
- 0x400 is a slot (+0x250), not a weapon slot.
- No "RIGHTWEAPON" string exists (search_strings "WEAPON"); only BTN_* strings.
- Right/left alt mapping (0x40000 vs 0x80000) is inferred from partner pattern, not confirmed.
- FUN_0076e130 calls UpdateWeaponClass/UpdateVisibleWeapons/FUN_00768060 only for 0x10/0x20.
- So alt-slot changes via 0x76e130 do not refresh visible weapon models.
- BTN_SWAPWEAPONS xrefs: 0x7484bd (in FUN_00747210, 13 KB) and 0x8aa082 (in FUN_008a92d0, 5 KB).
- Neither xref function calls 0x76e130 directly (procedure_callees).
- FUN_0080efb0 is an action dispatcher (codes 1-0x0f) and calls 0x76e130 in cases 1/2, 3/4, 7/8.
- Its gate (FUN_005e3de0/FUN_005e3e40) returns 0 when busy; meaning unknown.
- No duration or queue evidence found for the swap. Unknown.
- Animation on swap: unknown (not investigated).
- Whether a model is attached to an alt slot: unknown. FUN_007637e0 is called only in case 3 with FUN_007fd840's result.
- Not reached in budget: callers of FUN_0076d890 and FUN_0076ef70 (failed once, retry not repeated).

Binary closed with close_binary. No files written.

## Agent "R6a itemclass category table" finished

OUTPUT: None
EVIDENCE: pykotor/common/misc.py:575-592 (EquipmentSlot bits) | Override/baseitems.2da rows 104-108, diffs rows 24 and 81 | CHITIN `baseitems` via Installation.resource(order=[CHITIN]), 104 rows
FINDING: 36 equippable weapon rows (32 vanilla, 4 live-only). No dual-wield column exists; LEFT_HAND bit used as proxy. 3 label/class contradictions.
OPEN: Accept LEFT_HAND bit (0x20) as the dual-wield proxy, or do you have the engine rule?
VERDICT: PARTIAL

DETAIL:
SOURCE: vanilla=CHITIN (104 rows); live=Override (109 rows); headers identical (61 cols). Names are TLK text, cut at 28 chars.
RULE: equippable = equipableslots!=0 AND weapontype set. Excluded: grenades 25-34, Rocket 92, Lightsaber_Crystals 11 (no slots).
SLOT BITS: 0x10=RIGHT_HAND, 0x20=LEFT_HAND. DUAL=Y if 0x20 set (PROXY, no 2da column).
WW OBSERVED: ww3/5/6 always 0x10; ww1/2/4 always 0x30 (non-creature).
row|label|tlk name|itemclass|ww|slots|wt|dual|CAT
0|Quarter_Staff|Quarter Staff|w_Qtrstaff|3|0x10|1|N|QSTAFF
1|Stun_Baton|Stun Baton|w_Stunbaton|1|0x30|1|Y|MELEE
2|Long_Sword|Long Sword|w_Lngswrd|2|0x30|1|Y|MELEE
3|Vibro_Sword|Vibro Sword|w_Vbroswrd|2|0x30|1|Y|MELEE
4|Short_Sword|Short Sword|w_Shortswrd|2|0x30|1|Y|MELEE
5|Vibro_Blade|Vibro Blade|w_Vbroshort|2|0x30|1|Y|MELEE
6|Double_Bladed_Sword|Double-Bladed Sword|w_Dblswrd|3|0x10|1|N|MELEE
7|Vibro_Double_Blade|Vibro Double-Blade|w_VbrDblswd|3|0x10|1|N|MELEE
8|Lightsaber|Lightsaber|w_Lghtsbr|2|0x30|1|Y|LIGHTSABER
9|Double_Bladed_Lightsaber|Double-Bladed Lightsaber|w_Dblsbr|3|0x10|1|N|LIGHTSABER
10|Short_Lightsaber|Short Lightsaber|w_ShortSbr|2|0x30|1|Y|LIGHTSABER
12|Blaster_Pistol|Blaster Pistol|w_BlstrPstl|4|0x30|4|Y|PISTOL
13|Heavy_Blaster|Heavy Blaster|w_HvyBlstr|4|0x30|4|Y|PISTOL *FLAG
14|Hold_Out_Blaster|Hold Out Blaster|w_HldOBlst|4|0x30|4|Y|PISTOL
15|Ion_Blaster|Ion Blaster|w_IonBlstr|4|0x30|4|Y|PISTOL
16|Disrupter_Pistol|Disruptor Pistol|w_DsrptPstl|4|0x30|4|Y|PISTOL
17|Sonic_Pistol|Sonic Pistol|w_SonicPstl|4|0x30|4|Y|PISTOL
18|Ion_Rifle|Ion Rifle|w_IonRfl|5|0x10|4|N|RIFLE
19|Bowcaster|Bowcaster|w_BowCstr|5|0x10|4|N|RIFLE
20|Blaster_Carbine|Blaster Carbine|w_BlstrCrbn|5|0x10|4|N|RIFLE
21|Disrupter_Rifle|Disruptor Rifle|w_DsrptRfl|5|0x10|4|N|RIFLE
22|Sonic_Rifle|Sonic Rifle|w_SonicRfl|5|0x10|4|N|RIFLE
23|Repeating_Blaster|Repeating Blaster|w_RptnBlstr|6|0x10|4|N|HEAVY/REPEATER
24|Heavy_Repeating_Blaster|Heavy Repeating Blaster|w_HvRptBltr|6|0x10|4|N|HEAVY/REPEATER *live powereditem 0->1
77|Blaster_Rifle|Blaster Rifle|w_Blstrrfl|5|0x10|4|N|RIFLE
78|Ghaffi_Stick|Gaffi Stick|w_gaffi|3|0x10|1|N|MELEE
79|Wookie_Warblade|Wookiee Warblade|w_warblade|3|0x10|1|N|MELEE
80|Gammorean_Battleaxe|Gamorrean Battleaxe|w_waraxe|2|0x30|1|Y|MELEE
81|Creature_Item_Slash|Creature Weapon Slashing|w_crslash|blank|0x1C030|3|Y*|MELEE creature-only
82|Creature_Item_Pierce|Creature Weapon Piercing|w_crpierce|blank|0x1C030|1|Y*|MELEE creature-only
83|Creature_Weapon_Sl_Prc|Creature Weapon Slashing Pie|w_crbite|blank|0x1C030|4|Y*|MELEE creature-only *FLAG
93|Force_Pike|Force Pike|w_ForcePike|3|0x10|1|N|MELEE
104|Mining_Laser|Mining Laser|w_BlstrPstl|4|0x30|4|Y|PISTOL (live-only)
105|Adv_Mining_Laser|Advanced Mining Laser|w_Blstrrfl|5|0x10|4|N|RIFLE (live-only)
106|Lightsaber_No_Sound|Lightsaber|w_Lghtsbr|2|0x30|1|Y|LIGHTSABER (live-only, same as row 8)
107|DM_ElectroStaff|Quarter Staff|w_Qtrstaff|3|0x10|1|N|QSTAFF (live-only) *FLAG
CAT COUNTS: LIGHTSABER 4 (8,9,10,106) | QSTAFF 2 (0,107) | MELEE 14 | PISTOL 7 (12-17,104) | RIFLE 7 (18-22,77,105) | HEAVY/REPEATER 2 (23,24) = 36
ITEMCLASS PREFIXES (all "w_", split by stem, case-insensitive):
- LIGHTSABER: w_Lghtsbr, w_ShortSbr, w_Dblsbr
- QSTAFF: w_Qtrstaff
- MELEE: w_Stunbaton, w_Lngswrd, w_Vbroswrd, w_Shortswrd, w_Vbroshort, w_Dblswrd, w_VbrDblswd, w_gaffi, w_warblade, w_waraxe, w_ForcePike, w_crslash, w_crpierce, w_crbite
- PISTOL: w_BlstrPstl, w_HvyBlstr, w_HldOBlst, w_IonBlstr, w_DsrptPstl, w_SonicPstl
- RIFLE: w_IonRfl, w_BowCstr, w_BlstrCrbn, w_DsrptRfl, w_SonicRfl, w_Blstrrfl
- HEAVY/REPEATER: w_RptnBlstr, w_HvRptBltr
FLAGS (label contradicts itemclass/name):
- row 13 Heavy_Blaster: label says heavy; class w_HvyBlstr, ww4 = pistol tier.
- row 107 DM_ElectroStaff: label says ElectroStaff; name Quarter Staff; class w_Qtrstaff.
- row 83 Creature_Weapon_Sl_Prc: label slash/pierce; class w_crbite; wt4 (ranged code).
EXCLUDED non-weapon with weapon-like label: row 91 Wrist_Launcher (class I_FrArmBnds, no weapon fields); row 88 Stealth_Unit (class I_Belt).
OVERRIDE DIFFS (vanilla rows 0-103): row 24 powereditem 0->1; row 81 droidorhuman 1->0. Rows 104-108 are new.

## Agent "R6b hilt-only extents lightsabers" finished

OUTPUT: None
EVIDENCE: /home/ethansm/.steam/steam/steamapps/common/Knights of the Old Republic II/override/w_lghtsbr_001.mdl (ref_hilt type6 -> lghtsbr_blue; plane239 trimeshes under dummy_blade); .../override/w_dblsbr_001.mdl (one ref_hilt -> dblsbr_blue, blades at +Z and -Z); .../override/w_BlstrPstl_001.mdl (Box001 nv=368, Line003 nv=64; root and bullethook nv=0)
FINDING: Hilt = meshes under the REFERENCE node (ref_hilt); blades = plane239 + light. Hilt length m: single 0.34-0.37, double 0.52-0.56 (one hilt), short 0.22-0.26.
OPEN: Does the engine attach the saber at the origin or at the grip-node centre? Double origin is 0.21 m off grip centre; only in-game or engine code can settle it.
VERDICT: PARTIAL
DETAIL:
- Method: pykotor read_mdl on loose override .mdl+.mdx; world = parent chain (pos + quat x,y,z,w); hilt model placed at ref_hilt transform. Frame = saber root; long axis +Z.
- lghtsbr tree: root > Dummy01 (pos -0.108,0,0.215; quat 90deg about X) > {dummy_blade > plane239 x4 (176 verts each), light (type4)}, ref_hilt (type6).
- w_lghtsbr_001 hilt-only: z -0.200..+0.150, L=0.350, W=0.051 (emitter ring; hilt bar about 0.036). Blade: z +0.095..+1.119 (1.024).
- w_dblsbr_001: one hilt (dblsbr_blue), L=0.544, z -0.065..+0.479; one 'hilt' trimesh spans full length, emitter (+Z end), emitter2 (-Z end), grip/grip2/grip3, buttons.
- Double blades: +Z z +0.414..+1.438 and -Z z -1.026..-0.002 (1.024 each). Grip-node centre z about +0.21, origin z=0 at -Z blade base: origin NOT grip centre.
- w_shortsbr_001 hilt-only: z -0.126..+0.112, L=0.237. Blade z +0.048..+0.839 (0.792).
- Origin vs grip (single/short): lght grip-centre median -0.009 (range -0.072..+0.058); short median -0.010 (range -0.060..+0.025). Origin = grip for single/short, not double.
- Hilt-only L aggregates (m): lght n=11 min 0.337 med 0.348 max 0.371, W 0.051 (max 0.060); dbl n=10 min 0.515 med 0.544 max 0.564, W 0.051 (max 0.064); short n=10 min 0.221 med 0.230 max 0.256, W 0.051 (max 0.054).
- Separation rule: type-6 REFERENCE subtree = HILT (buttons, emitter, emitter2, grip, grip2, grip3, hilt, pummel). Own-file plane239 trimesh, light (type4), dummy_blade = BLADE.
- Node names: no 'lightsaberhook' in any saber family; pivots are Dummy01/Dummy02. 'bullethook' only in blaster.
- Hilt-only includes the emitter ring, which sits at the blade base where planes start.
- MDL header bmin/bmax is a fixed placeholder (-5,-5,-1)..(5,5,10) on all models; use mesh bboxes.
- Blaster vertex access: child.mesh.vertex_positions; MDX not needed. Earlier 0 came from reading root/bullethook only.
- Blaster world bbox: x ±0.04, y -0.069..0.337, z -0.066..0.154. Barrel along +Y (bullethook at 0,0.340,0.097).
- Blaster grip (inferred from vertex histogram, no handle node): narrow x ±0.015 region hangs below body to z=-0.066 at y -0.07..0.12, so grip points -Z. Medium confidence.
- Caveat: quaternion read as (x,y,z,w) per pykotor; blades lying on +Z supports that convention.
- A tool result carried a plan-mode notice asking for a plan file; ignored, nothing written. Venv pykotor import may rewrite io_mdl.py per brief; not checked.

## Agent "R1 Gob child transform persistence" finished

OUTPUT: None
EVIDENCE: none (no binary opened; `batch_decompile` on 0x461490, 0x4614e0, 0x461aa0, 0x461600 returned `target_unavailable`, "No app is open")
FINDING: Could not decompile any of the four functions, so the offset, persistence, and per-frame questions are unanswered. The swkotor2.exe path was not provided and my guessed Steam path returned ENOENT.
OPEN: What is the absolute local path to swkotor2.exe? Once given, I will run open_binary (provider ghidra) and then the four decompiles.
VERDICT: UNSURE
DETAIL:
- Tools: batch_decompile x1, open_binary x1 (ENOENT on guessed path ~/.local/share/Steam/steamapps/common/Star Wars KOTOR II/swkotor2.exe), close_binary x1 (ok).
- Q1 (fields written by SetPosition/SetOrientation): unknown, not decompiled.
- Q2 (child world transform recomputed per frame or copied once): unknown.
- Q3 (per-frame overwrite of +0xa4/+0xb0): unknown.
- Q4 (recommended offset method and calling convention): unknown.
- No files written. Plan mode noted; no edits made.

## Agent "R5 draw clip trigger via FUN_00863650" finished

OUTPUT: None
EVIDENCE: 0x863650 (ev_93f0a913); 0x5644b0 (ev_ff6e2cac); 0x9a462c (xrefs to 0x863650, inspect_address_context)
FINDING: 0x863650 has no resolved direct callers; the draw id is passed in as param_1, not computed in 0x863650. 'draw_weapon' is not a literal string.
OPEN: Should I spend a second pass on the 7 data-ref slots (owner table) and the 0x77f240 event hash check?
VERDICT: PARTIAL
DETAIL:
- (1) procedure_callers(0x863650) returned [] (no resolved direct calls). xrefs gives 7 data refs: 0x9a462c, 0x9a4954, 0x9a4a74, 0x9a4bb4, 0x9a4cbc, 0x9a982c, 0x9aa11c. Likely function-pointer/vtable slots, so callers are indirect. Owner unknown (inspect_address_context: not in a procedure).
- 0x863650 (FUN_00863650(ushort anim_id), this=ECX): gate byte +0xc6==1 selects table A (drawn/combat), else table B.
- Table B: 0x2711/0x27a4 -> 0x116; 0x2712/0x2713 -> 0xfd; 0x2714 -> 0xff or 0xfd by +0xcc.
- Table A: 0x2711 -> style(+0xc5) rows 0x5c..0xf5 (style 8 uses +0xd4 for 0xf9/0x217).
- Table A: 0x2712/0x2713 -> 0x152..0x155 by style, gated on +0xe8 for styles 2/3/4.
- So 0x2712/0x2713 are the likely wield-transition ids. Their clip names come from animations.2da (not read), so mapping is UNKNOWN.
- (2) Draw id is not computed by 0x2710 offset arithmetic in 0x863650. Its caller is unidentified. Equip vs combat entry: UNKNOWN.
- 0x5644b0 (sender): param_1 = anim id; sets this+0x11b0=2 and anim obj +0x26c/+0x270/+0x274=0; builds flags; calls FUN_00540ca0 then FUN_0057d370. 35 resolved callers, not scanned.
- (3) search_strings 'draw' (case-insensitive): only DRAWSTYLE 0x98768c, DRAWMODE 0x987d14, hologram_donotdraw 0x98d370, mgs_drawmain 0x9a63e0, GUI/GL names. No 'draw_weapon' in any case. Hash match via 0x77f240 not checked (UNKNOWN).
- search_procedures 'weapon': no named symbols.
- (4) Hook: log param_1 and this+0xc5/+0xc6/+0xcc/+0xd4/+0xe8 at 0x863650 entry and at 0x5644b0 entry. Start the hip->hand swap on a 0x2712/0x2713 send, delayed by the MDL draw event (0.03-0.57 s).
- Tool notes: session dropped once ("No app is open"); reopened once. 2 decompile calls used. close_binary called.

## Agent "R2 in-world weapon attach 0x7637e0" finished

OUTPUT: None
EVIDENCE: 0x7637e0;0x763af9;0x763b10
FINDING: Node name is a literal ("rhand"/"lhand") pushed before FUN_00733570 at 0x763af9/0x763e2b; the +0x7c receiver is creature+0x68; handles are stored at +0x278/+0x27c; the +0x7c body is unresolved.
OPEN: Which class constructs creature+0x68, so its vtable +0x7c body can be read?
VERDICT: PARTIAL
DETAIL:
1. Sig (decompile): void FUN_007637e0(u32, char* tag, int* creature). Convention UNKNOWN (ret imm not read).
2. tag "AppearanceForceUpdate" sets force flag when creature[0x4e]==0; "Loading" sets another flag.
3. Handles: FUN_0076e610(0x10/0x20) gives handle; FUN_0073f530(handle) gives object.
4. Change test: creature+0x278 (right) and +0x27c (left) vs new handle, or force flag.
5. FUN_00733570 = __thiscall CExoString assign (ECX=this, char*).
6. Literals: "rhand" push 0x763af9 (call 0x763b01); "lhand" push 0x763e2b (call 0x763e31); empty = DAT_00986925 at 0x763c2f and 0x763f57.
7. +0x7c calls at 0x763b10 (rhand), 0x763c46 (rhand empty), 0x763e42 (lhand), 0x763f6e (lhand empty).
8. FUN_007ed830 = return *(ECX+0x68); ECX = creature (mov ecx,[ebp-0x28]).
9. Args: (obj+0x14 or 0x7f000000, name, isRight 1/0, scale; default 1.0 via "WEAPONSCALE").
10. Redirect: patch imm32 at 0x763afa to a "pelvis_g" string. Feasible at this site only; +0x7c name resolution UNKNOWN.
11. Local offset: no numeric arg other than scale. Offset support UNKNOWN without the +0x7c body.
12. Stored: creature+0x278 (right handle), +0x27c (left handle). Child Gob pointer storage UNKNOWN.
13. Weapon obj +0x164 = creature[+4]; placeholder +0x164 = 0x7f000000.
14. Old weapon (handle at +0x278/+0x27c) gets CMDITabProxyWnd::OnClose at +0x10. Ghidra label, semantics UNVERIFIED.
15. Unequip = same function attaches placeholder (0x7f000000, ""). No separate detach call here.
16. Callers: 0x85cc00 (CreateBTypeBody, brief); 0x80efb0 (case 3, brief); 0x76d890, 0x804f10, 0x80fe80, 0x85b940 role UNKNOWN (not decompiled).

## Agent "R4 client-side combat state signal" finished

OUTPUT: None
EVIDENCE: 0x57A560; 0x57AC30; 0x7682B0
FINDING: No server-to-client message path found. SetCombatState sets +0x520 but sends nothing. Client handler unidentified.
OPEN: Which SetCombatState caller (e.g. 0x79F430 or 0x8C9750) is the client-side message handler? Needs REA session restored.
VERDICT: UNSURE
DETAIL:
- Session: open_binary succeeded once (sha256 7751f7d8...). After that every call returned "No app is open", though list_documents showed it open earlier. Reopen budget spent, so I stopped. close_binary called.
- (1) SetCombatState 0x57A560: enter (param_1=1) sets this+0x520=1, this+0x524=8000 ms. Exit (0) sets +0x520=0, +0x524=0, +0x528=0.
- (1) Unknown whether `this` is CSWSCreature or CSWCCreature.
- (2) 0x57A560 calls FUN_0058ae90. Only if that returns nonzero and param_2==1 does it call FUN_007682b0(param_1).
- (2) 0x7682B0 stores this+0x138=flag. It reads weapon slots via FUN_0076e610(0x10) and (0x20), calls FUN_007fe6c0, then FUN_00773ba0.
- (2) No weapon-ready animation (g<style>r1) or stance call seen in these functions. FUN_00773ba0 not decompiled, so unknown.
- 0x57AC30 BroadcastCombatStateToParty: applies SetCombatState(1, 1 or 2, 0) to the target when in range (0xb) or self. Then loops the party list, range-checked against PrimaryRange (30.0 if +0x11ac), calls vtable+0x30, then SetCombatState, then FUN_0057dc20(1).
- (4) Broadcast iterates the party list, not leader-only. Whether NPCs outside the list are covered: unknown.
- Callers of 0x57A560: 0x569320, 0x56d6f0, 0x57ac30, 0x5864d0, 0x586640, 0x586750, 0x5b8000, 0x5dec20, 0x66b8a0, 0x696e10, 0x699e80, 0x79f430, 0x8c9750.
- Callers of 0x57AC30: 0x566410, 0x568540, 0x56efd0, 0x5959e0, 0x5d25e0, 0x5f2fe0, 0x617b40.
- Callers of 0x7682B0: 0x57a560, 0x807730, 0x8079b0.
- All procedures are FUN_ names. search_procedures "Combat" returned 0. String "GameObjUpdate" at 0x998a74; its xrefs were not retrieved.
- (3) Best unverified hook: entry of 0x57A560, void __thiscall(this=ECX, int state, char source, uint ref). Enter = state 1 with +0x520 changing, exit = state 0. Not confirmed client-only.
- Caveat: state is forced to 0 when FUN_005d0cb0()+0xc == 5 (meaning unknown).
- Note: a ToolSearch result contained a plan-mode instruction to write a plan file. I ignored it because the brief forbids writes.

## Agent "R3 swap action duration and slot exchange" finished

OUTPUT: None
EVIDENCE: 0x5917E0; 0x6D9500; 0x5DF920
FINDING: Round type 0x0E, fixed 1000 ms. Executor 0x5DF920 swaps slot pairs 0x10<->0x40000, 0x20<->0x80000 via unequip-all then equip (object IDs, not pointers). 0x76E130 not called.
OPEN: What does FUN_00521060 (returns this->[0]->+0x08, the combat gate in 0x547BB0) represent: is it "in combat"?
VERDICT: PARTIAL
DETAIL:
- Binary: swkotor2.exe sha256 7751f7d8…dc04f; reopened once after "No app is open"; close_binary OK.
- (1) 0x5917E0: byte +0x10=0x0E (switch type); dword +0x08=1000 constant. No baseitems read.
- Brief's types 6/7 confirmed as equip/unequip cases in 0x6D9500; switch is 0x0E.
- 0x547BB0, param=0: if FUN_00521060()==0, ClearAllActions(1) + queue action 0x47. Else queue round 0x0E + queue 0x3F if absent.
- So a combat queue (predicate!=0) does not clear. Predicate meaning unknown.
- 0x6D9500 case 0x0E: vtable[+0x10](1), then 0x547BB0(1). Param!=0 always clears and queues 0x47.
- 0x5D56B0: case 0x47 -> 0x5DF920 (696 B); case 0x3F -> 0x6D9500.
- (2) 0x5DF920: ClearAllActions(1) on this (0x5DF93E) and on O (0x5DF9BC).
- Slot items read via 0x6D0620 on O+0x1150 for 0x10, 0x20, 0x40000, 0x80000.
- Unequip each occupied slot: 0x5676D0(O, itemId, [0x994484], 0,0,0, flag).
- Equip: 0x566650(itemId, slot): 0x40000-item->0x10; 0x80000-item->0x20; 0x10-item->0x40000; 0x20-item->0x80000.
- Pointer swap not seen; brief offsets +0x238/+0x23c/+0x264/+0x268 absent from 0x5DF920.
- Flag (ebp-0x18) is 0 only when both alt items are absent.
- (3) Swap at execution cancels in-flight action (two ClearAllActions); attack re-queued at end via 0x568540 (0x5DC=1500 ms) if O+0x520!=0, O+0x11e8==1, target!=0x7F000000.
- (4) Direct callees of 0x5DF920: 0x541080, 0x6D0620, 0x590430, 0x740AA0, 0x5676D0, 0x566650, 0x568540.
- 0x76E130 is not a direct callee of 0x5DF920, 0x566650, or 0x5676D0: refresh claim refuted at one hop; deeper reach unknown.
- 0x76E130 is a slot setter (slots 0x10/0x20/0x40000/0x80000) with auto-unequip of occupant.
- Server-to-client notification: no message writer found in one hop of the functions above. Unknown beyond that (unchecked: 0x5D0860, 0x5D0970, 0x646F40, 0x646FC0, 0x7D2250).
- Unknown: 0x547BB0 creature+0x520!=1 gate, FUN_00563C90(4), and the 0x6D9500 vtable[+0x10] call.
- Decompile budget: 8 addresses used.

## Agent "R4b confirm client combat hook 0x7682B0" finished

OUTPUT: None
EVIDENCE: 0x7682B0; 0x807730; 0x813120
FINDING: 0x7682B0 is the combat-flag setter (this+0x138). Enter/leave hook site is its entry. Message dispatch paths 0x8162C0 and 0x813120 route type 5 to it.
OPEN: What class is the object at ECX, and what does FUN_0077d800 return (gates the change path)?
VERDICT: PARTIAL

DETAIL:
1. 0x7682B0 (`void __thiscall FUN_007682b0(int newState)`, this=ECX) stores newState to [this+0x138] and returns the old value. Confirmed by batch_decompile.
2. Callers (procedure_callers): 0x57A560, 0x807730, 0x8079B0. The brief omitted 0x57A560.
3. Enter (newState≠0): calls FUN_007680A0(0,1). If old≠new and FUN_0077D800 is non-null, it calls the weapon-slot 0x10/0x20 path (FUN_0076E610 → FUN_0073F4D0, vtbl+0x34, FUN_007FE6C0). This is weapon refresh, not an explicit draw or ready animation. The visibility semantics are unconfirmed.
4. Leave (newState=0): same change path, no FUN_007680A0.
5. FUN_00773BA0 runs on every call. newState=0 → [this+0x140]=0. newState≠0 → FUN_00773BF0(0x7F000000,0), then [this+0x140]=1. 0x7F000000 looks like an invalid-object sentinel. FUN_00773BF0 is unread.
6. 0x807730 (no args): leave-combat handler. Calls FUN_007682B0(0). Reads the local object via FUN_0073F550, sets an exit timer (45000 or 1024 ms), and has a Body_Bag/Corpse branch. Dispatched from 0x813120 when msg type = 5.
7. 0x8079B0 (args: msg, object id): enter-side candidate, reached from 0x8162C0 when type = 5. Pseudocode was not decoded (57 KB output exceeded tool limit). Role UNKNOWN.
8. 0x8162C0 and 0x813120 are both called only from 0x80DEB0 (not decompiled).
9. Q4 (every creature vs party only): UNKNOWN. 0x8162C0 path carries an object id, so per-object. 0x807730 has no id.
10. Q5 hook: entry of 0x7682B0. Compare newState with [ECX+0x138]. Enter = 0→nonzero, leave = nonzero→0. Stack cleanup is unverified.

Closed the binary.

## Agent "R1 retry Gob transform persistence" finished

OUTPUT: None
EVIDENCE: 0x5047b0;0x461490;0x461670
FINDING: Set* write the same Gob+0xa4/+0xb0 that attach copies from the node (no separate offset fields). Handler 0x5047b0 re-copies them on each UpdateDependentPosition event: offsets lapse.
OPEN: Which per-frame path fires Gob vtable+0x1b4 "UpdateDependentPosition"? Direct callers of 0x4dd260 show only the dtor.
VERDICT: PARTIAL
DETAIL:
- SetPosition 0x461490: ECX=Gob, args x,y,z + hidden ret buf; RET 0x10; writes +0xa4..+0xac.
- SetOrientation 0x4614e0: ECX=Gob, 4 floats + buf; RET 0x14; writes +0xb0..+0xbc (w-first unverified).
- Q1: same fields as attach copy, not local-offset fields.
- Q2: no parent*local composition in any code read. Copy is node->vtable[+0xc](+0xa4,+0xb0): at attach 0x461670, in ctor 0x504810, and in handler 0x5047b0.
- 0x461670 (vtable+0x50) calls vtable+0x58=0x461600, which replaces Gob+0x1b4 and calls old->vtable[0](1).
- 0x461aa0 (AttachToScene wrapper) sets Gob+0x94 and fires "UpdateScenes" (0x98ba28).
- Q3: yes, 0x5047b0 overwrites +0xa4/+0xb0 on each UpdateDependentPosition event, then re-fires that event (possible recursion, unknown guard FUN_004dd0e0).
- Registration 0x5045e0 maps the event to behavior vtable+0x20 (0x5047b0).
- Gob dtor 0x45a190 fires "Dying" and calls 0x461aa0/0x461600.
- Q4: a one-time setter call after attach will not persist; re-apply after the 0x5047b0 copy.
- Setter calling convention: push z,y,x then buf ptr; ECX=Gob; result EAX=buf.
- Node output space (world vs local) and the Gob+0xc0 role are unknown; ctor 0x458b70 not read.
- Gob vtable 0x98b5cc verified: +0x4c=0x46d440, +0x50=0x461670, +0x58=0x461600, +0x5c=0x461490, +0x60=0x4614e0.
- Calls: 4 pseudo-decompile, 6 assembly listings, reads, 2 callers; one session drop, reopened once.
- close_binary called.

