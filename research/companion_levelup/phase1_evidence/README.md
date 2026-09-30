# Phase 1 raw evidence (2026-09-30)
Decompile output of the Steam LAA exe, read-only Ghidra. Function names in the files are Ghidra `FUN_xxxxxxxx` (Steam VAs).
- `dec_3ab.txt`: creature-stats core + LevelUp/ValidateLevelUp/CanLevelUp/AutoLevelup/CanChooseFeat/CanChooseSpell (0x6b8280 ... 0x6bedf0).
- `dec_b2.txt`: server msg handler 0x661c30, SetExperience 0x6b6bd0, DoAutoLevelup/StartAutoLevelup/ShowLevelUpGUI, AutoLevelUpNPCs, DistributeExperience,
  pending struct helpers 0x6f58b0.., GetRecommended* 0x6f28b0/2970/2d50, LoadFeatGain, LevelUpHenchman 0x66c7a0, client CSWCLevelUpStats, SendPlayerToServer_LevelUp.
  NOTE: the block headed `FUN_005e5510` is a mis-target (I passed a GOG address); ignore it. `FUN_007e2aa0` likewise (GOG addr).
- `dec_b3.txt` (BeginPlayGame, CharGen ctor), `dec_b4.txt` (Powers/Skills accept handlers), `dec_b5.txt` (CExoString compare/c_str).
- `asm_auto.txt`: raw disassembly of AutoLevelup (ecx for GetRecommended* = class row at [EBP-0x44]).
- `callers1.txt`, `callers2.txt`: xref lists. `gog_dump.tsv`: GOG DB rows for the level-up classes. `delta_maps/`: gog_cluster_delta_map outputs (cols: steam, gog, name, method, retn).
