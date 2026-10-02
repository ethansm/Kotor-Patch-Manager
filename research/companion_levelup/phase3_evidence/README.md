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
