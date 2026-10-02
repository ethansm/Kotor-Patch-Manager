# Phase 3 evidence (static research for R4-R9; plan `~/.claude/plans/dazzling-brewing-quasar.md`)
One line per evidence file (raw Ghidra/tool output; conclusions live in doc 08 "Phase 3 research").

- E_entry_point_notes.txt: static notes on hotbar GUI framework, character_p controls/hooks, StartAutoLevelup message box, persistence, options/main-menu GUIs (entry-point research for R3/R6/R9).
- `A1_dec_72fb00_744f00.txt`: decompile of 0x72FB00 (returns *(this+4)+4 = options object) and 0x744F00 (portrait panel flag setter)
- `A1_callers_72fb00.txt`: ~264 callers of 0x72FB00 (generic options accessor)
- `A1_callers_5fedd0.txt`: callers of AutoLevelUpNPCs 0x5FEDD0 (only 0x7B5EF0)
- `A1_dec_5fedd0_callers.txt`: decompile of 0x7B5EF0 (SetAutoLevelUpNPCs: writes +8 bit0, calls AutoLevelUpNPCs) and 0x73F830
- `A1_disasm_74ee90.txt`: raw disassembly of UpdatePortraits 0x74EE90
- `A2_callers_7b5ef0_8e0c50.txt`: callers of SetAutoLevelUpNPCs 0x7B5EF0 (ini reader 0x7B88B0, GUI handler 0x8E0C50) and refs to 0x8E0C50
- `A2_disasm_7b88b0_full.txt`: full disassembly of options ini reader 0x7B88B0 (reads [Game Options] "Auto Level Up NPCs")
- `A2_callers_7b88b0.txt`: callers of 0x7B88B0 (0x79F6F0)
- `A2_dec_79f6f0_79f6c0.txt`: decompile of options load (0x79F6F0) and save (0x79F6C0) wrappers
- `A2_tagxrefs_autolevel.txt`: xrefs+decompiles for strings "Auto Level Up NPCs" and CB_LEVELUP (large; 0x7B88B0, 0x7B5F50 writer, 0x8DF930 GUI panel)
- `A2_fieldwrites_1200.txt`, `A2_offset_users_1200.txt`: writers of creature +0x1200 (PlayerCreated)
- `A2_dec_574180_6afed0_561f30.txt`, `A2_disasm_1200_writers.txt`: creature GFF loader 0x574180 context for +0x1200
- `B1_dec_hud.txt`: decompile of HUD draw 0x74B3F0 and buff-icon row gate 0x745770 (counter gate +0xddc/+0xddd; not otherwise analyzed)
- `B1_callers_canlevelup.txt`: all 21 call sites of CanLevelUp 0x6B9790 (15 distinct callers)
- `B1_dec_canlevelup_callers.txt`: decompile of previously undecompiled CanLevelUp callers 0x63EDC0, 0x63F550, 0x6415E0, 0x75FC30, 0x7C9DF0, 0x752050, 0x756B70
- `B1_tagxrefs_levelup.txt`: xrefs+decompile for BTN_LEVELUP/BTN_AUTO (only ctor 0x84C3A0, control offsets, event-0x27 AddEvent thunks); LBL_LEVELUP exact string not found (tags are LBL_LEVELUP%d format)
- `B1_dec_charscreen.txt`: decompile of ShowLevelUpGUI 0x84FCD0, SetStats 0x84E6F0 (BTN visibility from CanLevelUp at 0x84F221), HandleInputEvent 0x84F900, Update 0x84FBB0
- `B1_disasm_charscreen.txt`: disasm of SetStats button-visibility sites (0x84F1F0-0x84F270), ctor tail AddEvent registrations (0x84D8C0-0x84DD00), thunks 0x7FCF80/0x762E80/0x762E90, rotate handler head 0x850330
- `B1_disasm_panel_slots.txt`: disasm of panel vtable slot targets 0x40E7E0 (->HandleInputEvent 0x27), 0x40E800 (->0x29), 0x8878D0 (->0x28)
- `B1_disasm_744f00_counters.txt`: disasm of FUN_00744F00 0x744F00-0x7450C0 (overlay bits, counter zeroing at 0x74507F/0x7450A3)
- `C1_callers_addclass.txt`: callers of AddClass 0x6BDCD0 (only FUN_0066c7a0 x2)
- `C1_dec_addclass.txt`: decompile of AddClass 0x6BDCD0 (count cap 2, level arg), AddExperience-with-penalty 0x6B7060, 0x51C8D0
- `C1_addmulticlass.txt`: AddMultiClass handler 0x66C7A0 (copy of dec_b2 decompile) + annotated flow
- `C1_dec_levelup_gui.txt`: decompile of client CSWCLevelUpStats class accessors 0x844E40/0x844E80, 0x844860, SendPlayerToServer_LevelUp 0x87A770 (class = last class of working copy), 0x844BA0
- `C1_scripts_addmulticlass.txt`: vanilla scripts a_makejedi.nss / a_pc_jedi.nss (only AddMultiClass callers) + class ids
- `C1_hold_rule.txt`: proposed hold-mode rule function
