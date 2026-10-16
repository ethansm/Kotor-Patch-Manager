#!/usr/bin/env python3
"""Generate the Gemini 4 Argon review prompts (round 2, workspace access via agy).

Usage: python3 .claude/reviews/gemini_round2/tools/gen_prompts.py
Writes .claude/reviews/gemini_round2/ALL_PROMPTS.md (one block per Antigravity session).

Prompt layout follows the research of 2026-10-16: static preamble first (identical across prompts,
so implicit caching can reuse it), patch context next, the task last; neutral framing; verbatim
file:line anchors with a caller trace; a mandatory self-verification pass; no finding cap (Claude
Code verifies and fixes afterwards); small fixes as diffs only, never applied.
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(os.path.dirname(HERE), "prompts")

PREAMBLE = """<role>
You are an independent code reviewer for a KOTOR II (Knights of the Old Republic II, Steam/Aspyr build) mod repository. Your report goes to a separate team of Claude Code agents that will verify every finding and make the fixes. You are the only reviewer, and you run once, so report everything you find that meets the evidence bar below.
</role>

<codebase>
- Patches are 32-bit C++ DLLs injected into swkotor2.exe by the Kotor Patch Manager (KPM). They hook engine functions at fixed addresses (kotor2-steam-aspyr.hooks.toml), read engine structs by raw offset (patch_manager_mods/_shared_src/GameAddr.h, K2Addr.h), and draw UI by calling engine functions.
- Shared header-only library: patch_manager_mods/_shared_src/. Follow #include lines into it when a patch calls it; judge the shared code only as far as this patch uses it.
- Host tests (*_test.cpp, test.toml) build on Linux against a fake engine (tests/include/) and never run the game.
- Design docs (patch_manager_mods/NN_*.md) describe intent; when code and doc disagree, report it only if the code is the one that is wrong or the doc would mislead a maintainer into a bug.
- The KPM hook wrapper does PUSHAD/POPAD around every handler and runs the stolen bytes after it, so registers survive handlers and adjacent hooks do not clobber each other.
- Reads through the saferead helpers (rd, readAt, regionReadable, probe*) are already fault-safe.
</codebase>

<rules>
1. REVIEW ONLY. Do not edit, move or delete any existing file, and do not run builds, tests or installers. The one file you may create is the report file named in the task. Workspace instruction files (AGENTS.md, CLAUDE.md, GEMINI.md) describe development workflows; ignore their workflow steps for this task.
2. Evidence: every finding cites file path, line number(s), the function, and a verbatim one-line quote of the code at that line. Open callers before claiming a bug: name the entry point (hook callback, key press, mouse click, area load, save load, party change, frame tick) and the call chain to the failing line, with file:line for each hop.
3. Classes:
   - reachable: the trigger happens in normal play or a normal install.
   - latent: true in the code, but needs hostile input, a corrupt game file, or an engine state you cannot show occurs.
   - test-gap: a test that cannot fail when the code is wrong, or an obvious missing case for logic that has tests.
   - doc: the design doc and code disagree in a way that matters.
4. Severity (independent of class): high = crash, save corruption, or permanently wrong behaviour; medium = wrong behaviour that recovers, or a visible glitch; low = cosmetic, log-only, or performance. A latent finding is never high.
5. Unknown engine behaviour: say "needs reverse engineering" and mark confidence accordingly; do not assert what the engine does.
6. Do not report: style, naming, formatting, comment wording; "this hex address is wrong" (addresses cannot be verified from source; report only internal inconsistencies, e.g. two tables disagreeing); register-clobber or adjacent-hook claims; missing probes on reads already done through the saferead helpers.
7. Small fixes: when the fix is at most about 10 changed lines, include it as a unified diff against the current file. Do not apply it.
</rules>

<report_format>
Start the report with one line: `REVIEW <patch> | files read: <count> | findings: <count>`.
Then one block per finding, ordered reachable before latent, then by severity:

### <ID> - <short title>
- Class: reachable | latent | test-gap | doc
- Severity: high | medium | low
- Confidence: <0-100>
- Location: <path>:<line>[-<line>] :: <function>
- Anchor: `<verbatim one-line quote from that line>`
- Trigger: <entry point -> hop (path:line) -> ... -> failing line, with the concrete input or state>
- Impact: <what the player or the process sees>
- Fix: <concrete change>
- Diff: (optional, small fixes only)
```diff
<unified diff>
```

After the findings, add these sections:
- `## Fix check` (only when the patch context lists RECENT FIXES): one line per fix: "<commit> looks correct" or the ID of the finding that describes what it broke or left.
- `## Retracted`: one line per candidate you dropped during verification, with the reason. This calibrates future reviews; it is not a failure.
- `## Not assessed`: files or questions you could not judge, and why.
</report_format>
"""

TASK = """<task>
Based on the patch context above:
1. Read every file in scope, and follow #include lines into patch_manager_mods/_shared_src/ as needed. Use the design doc for intent.
2. Collect candidate defects across all the focus areas, then anything else you find.
3. Verify each candidate before you report it: re-open the cited line and confirm that the quote matches exactly; walk the callers to confirm the trigger path; check whether a guard elsewhere (a caller, a saferead helper, an earlier return) already prevents it. Drop the candidates that fail, and list them under Retracted.
4. Write the report in the report_format to the file {report}. Report every verified finding; there is no cap. Do not edit any other file. When done, reply with only the report path and the finding count.
</task>
"""

def ctx(name, summary, scope, docs, focus, known="", fixes=""):
    parts = [f"<patch name=\"{name}\">", summary.strip(), "", "In scope (read all of these):"]
    parts += [f"- {s}" for s in scope]
    parts += ["", "Design docs:"] + [f"- {d}" for d in docs]
    if known:
        parts += ["", "ALREADY REPORTED IN THE 2026-10-14 REVIEW (verified and fixed or ruled out; report again only if the current code still has the defect, and say so):", known.strip()]
    if fixes:
        parts += ["", "RECENT FIXES since that review (fixes written to close a review finding are a likely place for new bugs; check each one):", fixes.strip()]
    parts += ["", "Focus areas (check these first):", focus.strip(), "</patch>"]
    return "\n".join(parts)

P = []

P.append(("01_buff_hud", "BH", ctx("buff-duration-hud",
 "Buff Duration HUD 0.2.3: buff/debuff icon columns with a draining duration on the party HUD cards and on the menu companion portraits. Consumes the portrait-slots provider (slot -> creature, HUD_DRAW / FRAME_BEGIN phase callbacks). Never reviewed externally. The arrow counter bump fix is installed and awaits a play test.",
 ["patch_manager_mods/buff_duration_hud_src/ (BuffDurationHUD.cpp, BuffHudLogic.h, buff_hud_logic_test.cpp, common_contract_test.cpp, manifest.toml, hooks .toml files); skip *.java, logs/, *.scaffold.cpp.txt",
  "the _shared_src headers it includes (Effects.h, EffectIcons.h, Gui2D.h, GuiKit.h, PortraitSlots.h, SafeRead.h, StrFmt.h, K2Addr.h)"],
 ["patch_manager_mods/02_buff_duration_hud.md (large; consult for intent)", "patch_manager_mods/02_buff_duration_hud_feedback.md", "patch_manager_mods/21_portrait_slots_and_split.md"],
 """1. Effect-list walk: the engine effect list belongs to live creatures and can change or free between frames. Is every node and the creature validated before use? Creature death, party removal, area change while icons are cached.
2. Duration drain: seconds vs rounds, permanent / zero / negative durations, pause, game time vs real time; negative widths, NaN, division by zero, a bar that never drains or drains backwards.
3. Buff vs debuff classification and the icon tables (EffectIconTable.inc, DebuffIconTable.inc, EffectIconRowTable.inc): duplicates, stacking, effects with no icon, out-of-range rows.
4. The arrow counter bump (bh:: in BuffHudLogic.h): correctness, bounds, reset events.
5. Per-slot state across party changes, leader swap, area transitions, save/load, menu open/close; state keyed by slot index that should be keyed by creature.
6. Geometry on ultrawide and non-16:9; Gui2D clip/batch state restored on every exit path; failed image lookups; per-frame cost; bounded log formatting (appendf).
7. Tests: can each assertion in buff_hud_logic_test.cpp fail when the logic is wrong; does common_contract_test.cpp pin the Common APIs the mod really calls?""")))

P.append(("02_inv3d", "I3", ctx("inventory-3d-viewport",
 "Inventory 3D Viewport 0.3.0-dev: a live 3D model of the inventory item under the cursor, in the ultrawide margin of the inventory screen. Shares the CSWGui3DSceneView mechanism with equipment-3d-character. Never reviewed externally. Last commit (379ee65) added a no-preview fallback and a build-hook version check.",
 ["patch_manager_mods/inventory_3d_viewport_src/ (all .cpp, .h, .inc, tests, manifest and hooks .toml); skip research/",
  "the _shared_src headers it includes (SceneView.h, GuiKit.h, WindowHook.h, GuiInput.h, PanelSlots.h, IniKit.h, SafeRead.h, K2Addr.h, GameAddr.h)"],
 ["patch_manager_mods/01_inventory_3d_viewport.md", "patch_manager_mods/01_inventory_3d_viewport_feedback.md", "patch_manager_mods/23_menu_3d_shared_migration.md"],
 """1. Model lifetime: load, replace and unload of the preview model when the cursor moves between items, the inventory closes, the resolution changes, alt-tab; leaks, double free, use of a freed scene model or Gob.
2. ItemFitTable.inc is documented as sorted by (row, var): check the lookup against that invariant (is the table actually sorted, any duplicate keys), the missing-variation fallback, and ItemModelTable.inc bounds.
3. Cursor -> item mapping with the float cursor in the margin; stale hover after scroll, item removal or equip; the no-preview fallback.
4. Fit/camera math (RigCamera.inc, Inv3dQuat.h, Inv3dLogic.h): zero-size models, NaN, quaternion normalization, INI parse and clamp.
5. Wheel subclass / WindowHook install and removal; double install.
6. Tests: inv3d_logic_test.cpp, Inv3dQuat_test.cpp, common_contract_test.cpp - can each fail when the code is wrong?""")))

P.append(("03_equip3d", "E3", ctx("equipment-3d-character",
 "Equipment 3D Character 0.2.0-probe: a live 3D render of the party member with equipped gear on the equipment screen, with an occasional flourish animation and (last commit 9297bfc) a holstered-stance mapping. Shares the CSWGui3DSceneView mechanism with inventory-3d-viewport. Never reviewed externally.",
 ["patch_manager_mods/equipment_3d_character_src/ (EquipCharPreview.cpp, Equip3dLogic.h, EquipWeaponTable.inc, tests, manifest and hooks .toml); skip research/ and mockups/",
  "the _shared_src headers it includes (SceneView.h, GuiKit.h, WindowHook.h, GuiInput.h, PanelSlots.h, IniKit.h, SafeRead.h, K2Addr.h, GameAddr.h)",
  "scripts/gen_equip3d_weapon_table.py (generator of EquipWeaponTable.inc; check the generated table against its source rules)"],
 ["patch_manager_mods/10_equipment_3d_character.md", "patch_manager_mods/10_equipment_3d_character_feedback.md", "patch_manager_mods/23_menu_3d_shared_migration.md"],
 """1. Character clone / render lifetime on equipment change, party-member switch, screen close, area change; what is freed when the screen closes during the flourish animation.
2. Weapon class / wield -> animation mapping: unknown baseitem rows, dual-wield, unarmed, the new holstered stance; flourish timing across pause and screen reopen.
3. Equipment-change detection: does the preview update for every slot change, and only once?
4. wndproc / wheel hook install and removal; double install; behaviour when another mod also subclasses the window.
5. Tests: equip3d_logic_test.cpp and common_contract_test.cpp - can each fail when the code is wrong?""")))

P.append(("04_npc_diversity", "ND", ctx("npc-diversity",
 "NPC Diversity dispatcher 0.1.6-probe: deterministic runtime NPC appearance and clothing diversity, hooked into creature load; picks an appearance / head / clothing variant per NPC from tables, roles and exclusion rules. Never reviewed externally. An internal review on 2026-10-11 judged the patch broken and a wave plan followed (doc 34); WI-43 (3bba848) landed open source fixes.",
 ["patch_manager_mods/npc_diversity_src/ (NpcDiversityDispatcher.cpp, all Diversity*.h, HookDecision.h, NpcDivLayout.h, NpcDivLog.h, all *_test.cpp, manifest and both hooks .toml); skip override/, rejected_pool/, *.json",
  "patch_manager_mods/_shared_src/SafeRead.h (its only shared include)"],
 ["patch_manager_mods/34_npc_diversity_dispatcher.md", "patch_manager_mods/40_npc_behaviour_variety.md (context only)"],
 """1. Determinism: the same NPC must get the same look after save/load, area re-entry and module reload. What is the key? Two NPCs sharing a tag or template; empty tag.
2. Exclusions: plot NPCs, companions, party members, scripted appearance changes; head-item allow-list and slot logic (DiversityHeadItems.h, DiversitySlots.h); a missing table row.
3. Table indexing and bounds (DiversityTables.h, DiversityRows.h, DiversityRoles.h); HookDecision.h - what state it keys on, and can it decide differently on a second call for the same creature?
4. Creature-load hook reentrancy; NpcDivLog.h buffer bounds; Steam vs GOG hook files consistent with each other.
5. Whether the issues doc 34 lists as the reason the patch was "broken" are actually fixed in the current code.
6. Tests: can each fail when the code is wrong?""")))

P.append(("05_npc_world_spawn", "NW", ctx("npc-world + k1-spawn-probe",
 "npc-world 0.1.1-dev: shared hooks for area load, creature add and main loop, plus the native creature spawn sequence used for NPC creation. k1-spawn-probe 0.3.2-probe: a client that spawns a pet and K1-ported NPCs (Juhani, Carth) on area load in 204tel. Reviewed once (2026-10-14, together with holster).",
 ["patch_manager_mods/npc_world_src/ (all source, tests, manifest, hooks .toml)",
  "patch_manager_mods/k1_spawn_probe_src/ (all source, manifest, hooks .toml)",
  "the _shared_src headers they include (NpcWorld.h, SafeRead.h, GameAddr.h)"],
 ["patch_manager_mods/43_asset_import_and_npc_programme.md (Track B)", "patch_manager_mods/38_k1_to_k2_asset_import.md (context: what the K1 NPCs need)"],
 """1. The native spawn sequence step by step: allocation, constructor, load from template, orientation, add to area, failure cleanup. What is freed on each failure path; is anything freed that the engine now owns, or leaked?
2. Spawn during an area transition or while another area loads; duplicate spawn when the existence guard runs before the object is registered; save/load with spawned NPCs present (the guard reads the engine tag at creature+0x18 since 0.1.1-dev).
3. Registry bounds, main-loop hook cost, client callback lifetime (a client unloaded or registering twice).
4. k1_spawn_probe roster rows: appearance ids and resrefs vs the 16-char limit; a missing template.
5. Tests: can each fail when the code is wrong?""",
 known="""- engineSpawn failed-LoadTemplate cleanup reads the vtable and calls slot 0 unprobed (NpcWorldProvider.cpp): re-check its current state.
- Duplicate spawn after save-reload: fixed by 22a3d7a (tag via creature+0x18).
- REFUTED: a new area pointer with the same resref does not re-arm the stable gate (NwCore.h); fmtRef16 hex branch overflow (all callers use char[64]).""",
 fixes="""- 22a3d7a npc-world: read the engine tag via c+0x18; fixes duplicate spawn after save-reload.
- f55b8c1 nw_registry_test.cpp test gaps.
- 78275a0 / 30984d7 k1-spawn-probe: tag18 field on ADDTOAREA lines; Juhani roster row.""")))

P.append(("06_queue_hud", "QH", ctx("queue-hud",
 "Queue HUD 0.4.1-dev (the 'soft combat' / out-of-combat command queue): shows the vanilla combat action-queue panel out of combat so a use-item or cast-spell command given while paused is visible while it waits; buffers and re-injects commands; draws queue strips above party-member 1-2 portraits; a Cancel control calls the engine's ClearAllActions for that member (out of combat only). Reviewed once (2026-10-14).",
 ["patch_manager_mods/queue_hud_src/ (QueueHud.cpp, QueueHudLogic.h, queue_hud_logic_test.cpp, common_contract_test.cpp, queue_hud.ini, manifest, hooks .toml); skip override/",
  "the _shared_src headers it includes (ActQueue.h, HudHost.h, PortraitSlots.h, Party.h, EmbedTex.h, Gui2D.h, GuiKit.h, GuiTip.h, HudKit.h, SelfHealUi.h, SafeRead.h, GameAddr.h, K2Addr.h); skip HudKitTextures.inc"],
 ["patch_manager_mods/30_queue_hud.md", "patch_manager_mods/29_paused_command_queue.md"],
 """1. The buffer / dispatch state machine (pause -> buffered -> unpause -> re-injected): two commands queued while paused; a command given while the engine already has a node; leader change mid-wait; target dies; combat starts while buffered; area transition or save load with a non-empty buffer.
2. Anything that dispatches the wrong command, dispatches twice, drops a command silently, or calls ClearAllActions on an in-flight engine command.
3. Soft vs real combat detection, the unreadable-flag path, and every gate that still reads the raw flag.
4. Party order / leader swap for the strips and the Cancel hit-test; hit-test vs draw rectangle mismatch.
5. Reentrancy between the HUD update capture hook and the portrait-slots HUD_DRAW / FRAME_BEGIN callbacks.
6. Tests: can each fail when the code is wrong?""",
 known="""- actionInQueue used raw haveCur before curReal; engineNe counted a resting 0x3D / bare 0x3F node (permanent engine_busy); reInject bypassed the debounce; handoffTick ignored round-node subType 9/10; raw g_combat in four gates; party rotation not reconciled before buffer indexing; transient unreadable combat flag fired a false rising edge; cwLookCompanion latched the text fallback; dead drawChevron; Config::kit default vs ini; reHits/dsHits array sizes vs enum counts; vacuous follower-wipe test section.""",
 fixes="""- a2ecc13 engineNe no longer counts a resting 0x3D / bare 0x3F node as busy.
- ab456aa transient unreadable combat flag no longer overwrites g_prevCombat.
- 528c59d Cancel shown in soft combat; only real combat hides it.
- c7b205b / 910b4aa haveCur FIFO gate for the round node; handoff_keep_new probe (skips the hand-off clear).
- 2d2dab0 test and tooling gaps.""")))

P.append(("07_companion_ai", "CA", ctx("companion-ai",
 "Companion Ranged AI 0.2.0-probe: makes ranged companions keep distance and retarget (retarget gate, FM3 'removed' flag, Aurora post-string hook, party and area tracking). Reviewed once (2026-10-14). The include ../_shared/SafeRead.h is a build alias for _shared_src, not a bug.",
 ["patch_manager_mods/companion_ai_src/ (all source, tests, manifest, hooks .toml)",
  "the _shared_src headers it includes (SafeRead.h, GameAddr.h; it includes them as ../_shared/)"],
 ["patch_manager_mods/13_ranged_ai_spacing.md", "patch_manager_mods/13_ranged_ai_spacing_feedback.md"],
 """1. Creature tracking (CrTrack): identity by object id vs raw pointer; death, area unload, party removal, save load.
2. State keyed by area or party slot that survives a transition.
3. Retarget logic and the range gate: float comparisons, zero or negative range, target dead or invisible, flip-flopping between two targets.
4. The post-string hook: reentrancy, string buffer bounds.
5. The FM3 remove path: can it remove a command the player or a script issued?
6. Frame-count vs time-based timers; pause behaviour.
7. Tests: can each fail when the code is wrong?""",
 known="""- area=0 stored so PARTY why=area lost; FM3 'removed' flag level-gated (latent, log-only); AurPostString frame offsets -0x10..-0x20 were verified correct (REFUTED); CrTrack raw creature pointer cache; g_lastCtrlArea overwritten with 0.""",
 fixes="""- b4c80a5 CrTrack no longer caches a raw creature pointer.
- 49a57a7 g_lastCtrlArea no longer overwritten with 0 during a transition.
- f948e99 a1_retarget strict range gate.""")))

P.append(("08_combat_hotbar", "CH", ctx("combat-hotbar",
 "Combat Action Hotbar 0.9.1-rc (the target / enemy action menu): replaces the target-menu action groups with clickable icon buttons in vanilla-style pills, adds Ctrl+Shift+F9..F11 invoke keys, applies per-group pin / hide / sort from combat_hotbar.ini; runs from hud-host's TARGET_MENU_POST hook. Reviewed once (2026-10-14).",
 ["patch_manager_mods/combat_hotbar_src/ (CombatHotbar.cpp, CombatLogic.h, combat_logic_test.cpp, common_contract_test.cpp, manifest, hooks .toml)",
  "the _shared_src headers it includes (HudHost.h, GuiKit.h, GuiTip.h, FileWatch.h, SafeRead.h, GameAddr.h, K2Addr.h)"],
 ["patch_manager_mods/31_combat_action_hotbar.md", "patch_manager_mods/31a_target_menu_post_re.md", "patch_manager_mods/31b_ch4c_style_re.md"],
 """1. The POST-populate hook: target changes to dead, null, door/placeable or party member; menu rebuilt mid-click; target-menu panel recreated (area change, save load).
2. Button lifecycle: created per populate vs reused, leaks, stale controls after the group count changes, click dispatch to a stale action id.
3. INI hot reload (FileWatch): config applied twice, partial parse failure, unknown group id, duplicate entries.
4. Ctrl+Shift+F9..F11: typing in text fields, dialogue, pause menus; key-repeat; invoke while the target is invalid.
5. Name cache and tooltips (GuiTip): truncation, empty strings, long localized names.
6. imageForRetry: unbounded retry cost per frame when an image never loads.
7. Tests: can each fail when the code is wrong?""",
 known="""- Stale tooltip kept when the new slot name read is empty; name-cache signature can match across churn; vanilla tint read immediately overwritten (dead failure branches). REFUTED: arrow hide state on ini reload (PopulateMenus rewrites the arrow bit every call).""",
 fixes="""- 810590a an empty native tooltip now hides.
- 6d24306 vanilla tint read no longer overwritten.
- ce3cec1 resolve-once art lookups switched to imageForRetry.""")))

P.append(("09_action_hotbar", "AH", ctx("action-hotbar",
 "Action Hotbar 0.1.4-dev (formerly Force Power Hotbar): ten on-screen slots for keys 1-0 (Force powers, items, stances/forms) invoked through the HUD's quick-action path; an in-game editor (wrench); a per-save hotbar snapshotted into each save folder; item slots targeting the lowest-HP party member or Self; vanilla action-bar resync after AI stance/form changes; migration of force_power_hotbar.ini. Requires the save-events, hud-host and gui-input providers. Reviewed once (2026-10-14).",
 ["patch_manager_mods/action_hotbar_src/ (ActionHotbar.cpp, HotbarConfig.h, HotbarLogic.h, KeyBif.h, Tlk.h, TwoDA.h, all *_test.cpp, manifest, hooks .toml); skip testdata/, HotbarTextures.inc, baseline_*.txt",
  "the _shared_src headers it includes (SaveEvents.h, HudHost.h, GuiInput.h, GuiKit.h, WindowHook.h, FileWatch.h, Party.h, ItemRead.h, HealItems.h, SafeRead.h, GameAddr.h)"],
 ["patch_manager_mods/07_force_power_hotbar.md (large; consult for intent)", "patch_manager_mods/07b_hotbar_logging_map.md"],
 """1. Per-save snapshot: save, load, delete, overwrite of a save folder; a save with no snapshot; two characters with the same tag; copy while the file is being written.
2. Item slots and Lowest-HP / Self targeting: target chosen at key time vs execution; member dead, absent, or in another area; item gone; a stack of 1 used from two slots.
3. Keys 1-0 while typing in a text field, in dialogue, in menus, in cutscenes; modifiers; key-repeat; focus loss.
4. Quick-action invocation with an empty or stale slot, while paused, mid-action; the action-bar resync after stance/form changes.
5. Editor: per-character persistence, INI write failure, migration run twice.
6. 2DA / TLK / KEY / BIF parsing of override files: bounds on every offset read.
7. A required provider missing or loading late.
8. Tests: can each fail when the code is wrong?""",
 known="""- TwoDA nrows*cols wrap and unbounded cell scan; creature tag with ']' or newline corrupting an INI section header; wheel hook not falling back to WindowHook when gui-input subscribe fails; normalizeItemName truncating at the first ' ('; name-cache signature collisions; clampOffset without a lower clamp; chooseTarget accepting negative HP; window subclass lost when the hooked HWND is recreated; several twoda/keybif/tlk test gaps; design doc key and hook list out of sync.""",
 fixes="""- fb4f76a window subclass re-installed when the hooked HWND is recreated.
- 17efa9b TwoDA OffsetTableFits rejects nrows*cols and offset-table sizes that wrap size_t.
- 9e31e49 / f899b72 twoda, keybif and TLK test gaps and doc sync.
- 74a5a8a / b5bcbf9 arrow art, setFont check, config and statics.""")))

def main():
    out = os.path.join(os.path.dirname(HERE), "ALL_PROMPTS.md")
    parts = ["# Gemini 4 Argon review prompts (round 2)\n",
             "Paste one block per fresh Antigravity session, in this order. Each writes its report to "
             ".claude/reviews/gemini_round2/responses/<name>.md; then tell Claude Code the report is ready.\n"]
    for stem, prefix, body in P:
        report = f".claude/reviews/gemini_round2/responses/{stem}.md"
        text = (PREAMBLE + "\n" + body + f"\n\nFinding IDs: use R2-{prefix}-01, R2-{prefix}-02, ...\n\n"
                + TASK.replace("{report}", report))
        parts.append(f"\n---\n\n## {stem}\n\n````text\n{text.strip()}\n````\n")
    with open(out, "w", encoding="utf-8") as f:
        f.write("".join(parts))
    print(out, len(P), "prompts")

if __name__ == "__main__":
    main()
