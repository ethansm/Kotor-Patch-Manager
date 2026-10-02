# Phase 4 evidence (0.3.0 build: hold mode, instant levelling, save binding, Configure UI; plan `../PHASE4_PLAN.md`)
One line per evidence file (raw Ghidra/tool output; conclusions live in doc 08 "Phase 4 M0 facts" and later Phase 4 sections).
Only Ghidra/static agents write here; one Ghidra headless run at a time across all sessions (`../tools/ghidra_run.sh`).

- S1_sites.txt: hook sites for hold-veto (13 CanLevelUp call sites incl. 0x756B70/0x752050/0x7C9DF0/0x75FC30) + options-accessor sites; bytes, frame slots, overlap scan, VETO/LEAVE table (stats+0x24 = creature POINTER).
- S1_gui_guards.txt: ShowLevelUpGUI / StartAutoLevelup / DoAutoLevelup callers; recommended guards 0x84FD2F (GUI), 0x84F9FE (Auto), 0x850062 (OK backstop); leader-creature chain.
- S1_setstats.txt: SetStats 0x84E6F0 frame (panel [ebp-0x288], stats [ebp-0x28]), result drives BOTH buttons, callers (Update, vtable slot, ShowLevelUpGUI).
- S1_globals.txt: global-var table API (Add/Find/Get/Set Boolean+Number, signatures), saver 0x656640 / post-load 0x656758 captures, pointer chain [[[0xA1B4A4]+8]+4]+0x100FC, server-recreate hook 0x401C52.
- S1_template_gui.txt: live character_p.gui controls, BTN extents, template tag recommendation (LBL_EXPERIENCE_STAT plain, LBL_XP_BACK framed).
- S1_control_lifetime.txt: panel dtor chain 0x84DEF0 -> 0x84DF70 -> 0x40E860 never frees panel+0x24 entries; ownership rule (append once, never free, key on panel pointer).
- S1_raw_*: raw outputs (disasm ranges, callers/refs, overlap scan + script, live-vs-vanilla bytes, character_p dump, dtor decomp).
