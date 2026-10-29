# Session Handoff & Plan State: patch_manager_mods/11_inventory_sort_filter.md

## 0. Next-Phase Direction
Execute VU7c: ISF look rows (row height, icon size, list font) from LOOK_RE.md; first close VU7b gaps (icon border of the 3 CSWGuiBorders at +0x1D4/+0x24C/+0x2C4, LB scroll step); height is set only for new rows at 0x8B0FB0, so a restyle must also hit reused pool rows

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/11_inventory_sort_filter.md`
- **Waves:** .claude/waves/WI-11-a1/
- **Waves:** .claude/waves/WI-11-a2/
- **Waves:** .claude/waves/WI-11-b1/
- **Waves:** .claude/waves/WI-11-b2/
- **Waves:** .claude/waves/w1-11-VU4/
- **Branch:** `KOTOR-II` (66894a03 mod-config 0.4.9-dev: low-priority review fixes (entry unregistered while Options is open: slot stale until next bind, page left; locked caption on disabled sliders; held arrow stops on scroll; entry description returns to the mod blurb off every row; empty entry shows a note; dead g_escConsumedAt removed). ModConfigDecl: get() answers defaults before the first build, a reopen keeps no-ini row values. PreviewCanvas: resync re-asserts click-through on still-visible labels)
- **Generated:** 2026-10-29 02:51:37 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
66894a03 mod-config 0.4.9-dev: low-priority review fixes (entry unregistered while Options is open: slot stale until next bind, page left; locked caption on disabled sliders; held arrow stops on scroll; entry description returns to the mod blurb off every row; empty entry shows a note; dead g_escConsumedAt removed). ModConfigDecl: get() answers defaults before the first build, a reopen keeps no-ini row values. PreviewCanvas: resync re-asserts click-through on still-visible labels
 patch_manager_mods/_shared_src/ModConfigDecl.h  | 12 ++++++++++++
 patch_manager_mods/_shared_src/PreviewCanvas.h  |  4 +++-
 patch_manager_mods/_shared_src/moddecl_test.cpp | 15 +++++++++++++++
 patch_manager_mods/mod_config_src/ModConfig.cpp | 19 ++++++++++++++-----
 4 files changed, 44 insertions(+), 6 deletions(-)
```
- **Dirty Files (197):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/50_k2_render_performance.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ... +187 more (run `git status --short`)

## 2. Immediate Objective (**VU7c**)
**List look rows Row height / Icon size / List font (10x10 / 16x16) + preview follows them; applied at the next inventory build (owner 10-29: "icon/font size, row height etc"). Category dividers stay in VU6**

Status: TODO
Task: List look rows Row height / Icon size / List font (10x10 / 16x16) + preview follows them; applied at the next inventory build (owner 10-29: "icon/font size, row height etc"). Category dividers stay in VU6

## 2b. Upcoming Batches in Pipeline
- ****VU0**** : Calibrate the mockup on one owner screenshot (inventory open, cycle on Usable): font scale, New colour, tab highlight, does the engine clip LBL_FILTER
- ****VU1**** : SUPERSEDED 10-29 (top bar back to vanilla; ISF bar VU4). Was: quick fix 0.3.2-dev (hot-swap): short captions + "Can Equip" rename in SortLogic.h captionWithCount (drop " P/N (n)"; counts move to the buttons); sort_logic_test vectors; mockup `current` = new captions; ISF-T11 rewritten
- ****VU2**** : Static RE ($0 first): (a) per-frame tick while the inventory is open without a second detour on 0x8A7350 (KPM one detour per address? alternative site or an inv3d/hud-host provider callback); (b) does a button cloned from a BTN_* tab template keep the event-0x27 binding to the tab handler 0x8A8CF0 (then ISF's existing SortFilterTabClick sees the click, ctrl+0x5C = our id, no tick needed); (c) how to add a non-item divider row to LB_ITEMS (protoitem clone in the list box control list vs a CreateItemEntry variant), and keep selection index, reselect, tint indices (PanelRowArrayOff) and keyboard / controller navigation skipping it; (d) for v4b: can ISF resize LB_ITEMS at panel build (guikit::setRect) without breaking its scrollbar

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/11_inventory_sort_filter.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **VU7c**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- 10-29 VU3 INSTALLED (0b4850c9; KPM fc3c6e9 header, 2b85416 mirror): hud-host 0.6.0-dev LIVE 00:45, rollback hud-host_swap_20261029_004515, play HH-PB1.
  `run_tests.py hud_host` runs only common_contract; the real suites are ids `cpp:hh_dispatch_test cpp:hh_registry_test cpp:hh_strip_test`.
  K2HudContext is now 44 B; 4 consumer contract tests (combat-hotbar, selfheal, enemy-strip, queue-hud) pinned size == 40 -> now pin v1 prefix (>= 40). Their runtime gates are all `struct_size < offsetof(..)+sizeof` (safe).
  VU4 consumer MUST gate `ctx->struct_size >= offsetof(K2HudContext, panelVtbl) + 4` before reading panelVtbl, then compare to the inventory vtable.
  Gate 10-29 failed only on other sessions: level_up_override_src WI-164 (006c6bc3: exports.def missing LuoFeats*/LuoPowers*, parity compile error LevelUpOverride.cpp:2345, raw addrs LevelUpUi.h:911) + AEP raw addrs (21). Owner waived for the hud-host swap; ask again for the ISF install if still red.
- 10-29 VU3 WIP (2bfbdbbb): `run_tests.py hud_host` PASS (host test uses repo `_shared_src/HudHost.h`), but `build_check.py --mod hud_host_src` FAILS:
- VU4 recon facts (4 Haiku units, vtable spot-checked): `.claude/waves/w1-11-VU4/facts.md` (inventory vtable 0x009A72AC, GuiKit label API, k2hud consumer pattern, ISF populate anchors).
  the KPM build reads `Patches/Common/HudHost.h`, still the old copy (K2HUD_PANEL_BUILD / panelVtbl undeclared). `check_shared_headers.py`
  reports "63 shared headers, 1 drifted". Sync is a deliberate, separately committed step (BUILD_AND_INSTALL.md:151, never inside install).
- HudHost.h is included by 15 mod sources; K2HUD_PHASE_COUNT 4 -> 5 only grows k2hud::Client::regs (no ABI field moves; panelVtbl appended).
- named_decomp.py: callee name annotation can fail with "response exceeds the stdio response budget" for big callees; the target decompiles still succeed.
- Haiku recon subagents may be refused file Writes: have them reply inline and persist the report yourself.
- System python3 has no PIL: run the mockups with `.venv/bin/python`.
- DONE 10-15: case-insensitive name key (foldCmp, ASCII fold on unsigned char; never stricmp/tolower on raw char) in 468c8c7; docs in e7d9f6b; live Common GameAddr.h synced (`check_shared_headers.py --sync`, 35/0 drift).
- Do NOT re-implement the name field / readItemName (c24be8e). Empty name stays "equal-or-first"; skipping the key when one side is empty makes the order cyclic (UB in stable_sort).
- GetString 0x7356B0 is REA UNSURE (side effects in doc 11 risk 7): first live call is the WI-21 play test in PLAY_TEST_QUEUE.md (pending, the user plays it; never block on it).
- PULL_REQUEST.md (untracked->committed e7d9f6b) is NOT posted; posting needs explicit chat approval. It says manifest v1.0.0 vs rule 0.x.y: flag only.
- No game install done. Install = rebuild + hotswap per kotor2-kpm-install, needs user GO.
- vw2-A (10-27) DONE: WI-152 31c44026 (caption "Filter: Usable 2/3 (n)"), WI-154 fd958494 (sort_order category|base|name|vanilla / 0..3), WI-155 d3f1d4e2 (mod-config Sort order row, SortOrderPage.h, KPatchInit export; list mod-config BEFORE ISF in load order), WI-153 65d5313c (bumper hooks H6e 0x8A8997 / H6f 0x8A89F7, check_windows rows added), doc 11 caef8b6a. Manifest still 0.2.3: bump at install (0.3.0, new features). Play rows ISF-T8/T9/T10/T11 pending.
- Do NOT reuse `fix_wave.py select --ids` for ISF WIs: any `out-of-repo:` note makes them ineligible; hand-write items.input.tsv. Worktree agents may be denied `git reset --hard`: WIP commit + merge HEAD instead.
- WI-161 (doc 11 WI-151 caption text) still hold; doc 45 row-list update for WI-155 skipped (doc 45 is untracked, another session's file).
- Verify: `python3 scripts/run_tests.py sort_logic_test common_contract_test`; `python3 patch_manager_mods/mods678_research/check_windows.py`; `python3 scripts/run_tests.py inventory_sort`; `python3 scripts/build_check.py --mod inventory_sort_filter_src`; `python3 scripts/check_shared_headers.py`.
