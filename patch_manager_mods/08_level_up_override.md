# 08 - Level-up override (per-character fixed level-up path)

Phase 1 (read-only Ghidra investigation) record. Plan: `research/companion_levelup/PHASE1_PLAN.md`; overall:
`research/companion_levelup/MASTER_PLAN.md`. Target: Steam Aspyr LAA `swkotor2.exe` (sha256 6A522E71... = `.unpatched_vanilla`; the
live exe 4AB72FC1... is the patched one - bytes at every shortlisted site are identical in both). Ghidra image base 0x400000; file
offset of a .text VA = VA - 0x400C00. All Steam functions below were read in a decompile (2026-09-30), not just address-matched.

## Status
2026-09-30: Phase 1 + Phase 1b static investigation COMPLETE (see 'Phase 1b results'; O1-O7 closed statically, live confirmation pending user consent for a log-only probe).
2026-09-30 (earlier): Phase 1 static investigation COMPLETE. No game change, no install, no probe run. Q1-Q8 answered below (Q4 feats-panel
gating and the client-refresh behaviour after a server-side rewrite are the only residual doubts -> Phase 2 spike).

## One-paragraph answer
Every level-up (manual GUI, Auto button, `Auto Level Up NPCs`, XP award, NWScript `LevelUpHenchman`) ends in
`CSWSCreatureStats::LevelUp` (0x006B9870), which applies a **pending level-up struct** (0x30 bytes: class, skill deltas, feats,
spells, skill points) **without re-validating any pick**. `ValidateLevelUp` (0x006B9640) only checks `CanLevelUp` (level cap + XP).
So Option B (own picker) is feasible with ONE server-side hook at `LevelUp`'s entry: rewrite the pending struct from
`paths/<tag>.toml`, then let the original run. Option A (a per-creature "package") is NOT available: the auto picks come from lists
stored in the shared CLASS row, and the creature's package field is never consulted by `AutoLevelup`.

## Address table (Steam)
Delta for the CSWSCreatureStats cluster vs the GOG DB = **-0x3C480** (91/129 by vote, remaining 38 land on function starts at the same
delta). "Conf" = decompile-confirmed unless stated. Convention: `__thiscall`, ECX = this, stack args, callee cleans (`ret N`).

| Function | GOG addr | Steam addr | Evidence | Conv / ret | Conf |
|---|---|---|---|---|---|
| CSWSCreatureStats::AddFeat | 0x6F4700 | **0x006B8280** | (this,short) dedups vs feat list, appends 4-byte {id,uses}; NOT 0x6F5B40 (that is PendingLevelUp::AddFeat) | thiscall ret 4 | high |
| ::SetSkillRank | 0x6F4640 | 0x006B81C0 | (this,byte skill,char rank) writes `*(this+0x174)[skill]` | thiscall ret 8 | high |
| ::HasFeat | 0x6F4870 | 0x006B83F0 | two list lookups via 0x6C0AA0 | thiscall ret 4 | medium |
| ::RemoveFeat | 0x6F4A70 | 0x006B85F0 | backwards scan of feat list | thiscall ret 4 | high |
| ::AddKnownSpell | 0x6F4B20 | 0x006B86A0 | (this,byte classPos,spellId), classPos < this+0x8D | thiscall ret 8 | high |
| ::GetClassLevel | 0x6E9770 | 0x006AD2F0 | (this,byte pos,int subNeg) | thiscall ret 8 | high |
| ::GetLevel | 0x6E1280 | 0x006A4E00 | total level accessor (byte) | thiscall ret 4 | medium |
| ::SetClass / ::SetClassLevel | 0x6F2E60 / 0x6F2EA0 | 0x006B69E0 / 0x006B6A20 | write class id / level bytes | thiscall ret 8 | high |
| ::GetTag | 0x6F2FB0 | 0x006B6B30 | fastcall, returns `*(this+0x24)+0x18` (CExoString*) | fastcall | high |
| ::GetExpNeededForLevelUp | 0x6F3760 | 0x006B72E0 | `rules+0x38 + GetLevel*4` | thiscall ret 0 | high |
| ::CanLevelUp | 0x6F5C10 | 0x006B9790 | level < cap and XP(+0x68) >= table | fastcall | high |
| ::ValidateLevelUp | 0x6F5AC0 | **0x006B9640** | CanLevelUp -> send msg 0x25 -> LevelUp(p,1); no pick checks | thiscall ret 4 | high |
| ::LevelUp | 0x6F5CF0 | **0x006B9870** | applies pending struct (see below) | thiscall ret 8 | high |
| ::AutoLevelup | 0x6FB270 | 0x006BEDF0 | builds pending, GetRecommended* loops, ValidateLevelUp | thiscall ret 4 | high |
| ::CanChooseFeat / ::CanChooseSpell | 0x6FA5D0 / 0x6FAA70 | 0x006BE150 / 0x006BE5F0 | called from AutoLevelup with pending | thiscall ret 20 / 16 | medium |
| ::SetExperience | 0x6F3050 | 0x006B6BD0 | on XP rise applies stored per-level pending structs via LevelUp(p,0) | thiscall ret 8 | high |
| ::AddClass / ::AddExperience | 0x6FA150 / 0x6F34E0 | 0x006BDCD0 / 0x006B7060 | delta lands on starts; called by LevelUpHenchman | thiscall | medium |
| ::GetFeatGainCounts (unnamed in GOG) | - | 0x006BDF80 | featgain _REG/_BON for next level from class row | thiscall | medium |
| Global::GetRecommendedSkill/Feat/Spell | 0x511020/0x510F60/0x511400 | 0x006F2970/0x006F28B0/0x006F2D50 | (ecx=CLASS ROW, n) -> nth entry of class-row list | thiscall ret 4/4/8 | medium (read in full) |
| CSWClass::LoadFeatGain | 0x50F380 | 0x006F0CD0 | error string "CSWClass::LoadFeatGain: can't load featgain.2da" | cdecl | high |
| PendingLevelUp ctor / dtor | - | 0x006F58B0 / 0x006F59B0 | 0x30-byte struct | fastcall | high / medium |
| PendingLevelUp::GetSkillDelta / SetSkillDelta / AddFeat | - | 0x006F5AC0 / 0x006F5B00 / 0x006F5B40 | array accessors | thiscall | high / high / medium |
| CSWSMessage::HandlePlayerToServerLevelUpMessage | 0x8079D0 | **0x00661C30** | builds pending from message, ValidateLevelUp, Confirmation; GOG delta does not hold (Aspyr edits) | (msg*,char) ret 8 | high |
| CSWSMessage::SendServerToPlayerLevelUp_Confirmation | 0x7E2B70 | 0x0064C450 | vote + called by handler | ret 8 | high |
| CSWCMessage::SendPlayerToServer_LevelUp | 0x506050 | 0x0087A770 | serialises DIFF original vs edited client stats | cdecl-like ret 8 | high |
| CSWCLevelUpStats::Constructor (client edited copy) | 0x490650 | 0x00845AC0 | vftable CSWCLevelUpStats | thiscall ret 4 | high |
| CSWGuiLevelUpCharGen::BeginPlayGame (accept) | 0x5EAB50 | 0x008F8070 | only caller of 0x87A770 | fastcall | high |
| CSWGuiInGameCharacter::ShowLevelUpGUI | 0x5995D0 | 0x0084FCD0 | constructs CSWGuiLevelUpCharGen (0x8F4AF0) | thiscall ret 4 | high |
| CSWGuiInGameCharacter::StartAutoLevelup / DoAutoLevelup | 0x599720 / (GOG DB row wrong) | 0x0084FE20 / 0x00850010 | confirm dialog -> callback calls AutoLevelup(0) | | high |
| CSWPartyTable::AutoLevelUpNPCs / DistributeExperience | 0x704780 / 0x703A70 | 0x005FEDD0 / 0x005FE0C0 | CanLevelUp -> AutoLevelup | fastcall / thiscall | high |
| Global::ExecuteCommandLevelUpHenchman (by behaviour) | - | 0x0066C7A0 | 2 args; sets XP, AddClass, AutoLevelup(1) if option | cdecl-like | medium (opcode id not checked) |
| CExoString::CompareNoCase(char*) | - | 0x00734F60 | `__stricmp` | thiscall | high |

Other AutoLevelup callers (purpose not read): 0x005FD480, 0x0074EE90. GOG-DB mislabels found: 0x6B8DA0 is `GetWeaponFinesse`
(see finesse patch), `CSWGuiInGameCharacter::DoAutoLevelup` GOG-drift row 0x73EF30 is wrong.

## Struct offsets (Steam)
**CSWSCreatureStats** (`this`): +0x00 FULL feat list (`ushort*` ids, count +0x04; append 0x6C0A70, contains 0x6C0AA0), +0x18 second feat list
(`ushort*`, count +0x1C; HasFeat 0x6B83F0 checks both) [corrected 2026-10-01 from the live log]; +0x0C uses-per-day records only (`{ushort id, byte uses}*` ptr array,
count +0x10; only feats whose row +0x33 flag is set);
+0x24 owner CSWSCreature* (creature: +4 object id, +0x18 CExoString Tag, +0xE4 max HP, +0x1200 is-player flag);
+0x28/+0x2C stored per-level pending structs (LvlStatList) ptr array / count; +0x68 XP (uint); +0x6C has-client/player flag;
+0x8D ClassCount (byte); class entry i at `this + i*0x28`: +0xAB class id, +0xAC level, +0xAD negative levels;
+0xE0 ushort "package" row index into `rules+0x128` (0x34/row) - computed in AutoLevelup but NEVER USED afterwards;
+0xED/+0xEF/+0xF1/+0xF3/+0xF5/+0xF7 STR/DEX/CON/INT/WIS/CHA base (matches GOG 237..247); +0x12E force points; +0x170 ushort unspent
skill points; +0x174 skill ranks `byte*` (length rules+0x123); +0x18A ushort alignment-ish value used for spell choice.
GOG `starting_package` +140 (=0x8C) is NOT what AutoLevelup reads on Steam.
**Rules singleton** `DAT_00A1B4D0`: +0x38 uint XP table by level; +0x11C ushort feat count; +0x123 byte skill count;
+0x124 class table (0x238/row); +0x128 package table (0x34/row); +0x12C feat table (0x20/row: +0x1C flags, +0x2D min level).
**Class row** (0x238): +0x54 HP gain, +0x55 force gain, +0x56 skill points base, +0x177+L / +0x1A9+L featgain _BON/_REG bytes,
+0x1DC recommended-skill list (ptr, ushort count +0x1E0, 0xC/entry, rank at +8), +0x1E4 recommended feats (count +0x1E8, 0xC/entry),
+0x1EC recommended spells (count +0x1F0, 0x10/entry, type byte +0xC), +0x1F2 ability-increase index, +0x234 force-user pointer.
**Pending level-up struct** (0x30, ctor 0x6F58B0): +0x00 spell ids to add (`uint*`, byte count at +4); +0x0C spells to remove
(`uint*`, count +0x10); +0x18 feat ids (`ushort*`, ushort count at +0x1C); +0x24 skill delta bytes (len = rules+0x123);
+0x28 ushort unspent skill points to store; +0x2A byte ability index (6 = none); +0x2B HP; +0x2C class id; +0x2D force points.
Containers are CExoArrayList-like (ptr,count) pairs grown via 0x0083EA60 / 0x00593450 - do not push manually; call
`PendingLevelUp::AddFeat` (0x6F5B40) or edit in place.

## Q1-Q8

**Q1 - shared routine / hook point.** Call graph:
```
Manual GUI:  ShowLevelUpGUI 0x84FCD0 -> CSWGuiLevelUpCharGen panels (client edits a CSWCLevelUpStats copy)
             -> BeginPlayGame 0x8F8070 -> SendPlayerToServer_LevelUp 0x87A770 (diff vs original)
             -> server HandlePlayerToServerLevelUpMessage 0x661C30 -> ValidateLevelUp 0x6B9640 -+
Auto button: StartAutoLevelup 0x84FE20 -> (confirm) DoAutoLevelup 0x850010 -> AutoLevelup 0x6BEDF0 -+ (AutoLevelup builds its own
Auto NPCs:   AutoLevelUpNPCs 0x5FEDD0 / DistributeExperience 0x5FE0C0 -> AutoLevelup             |  pending and calls 0x6B9640)
NWScript:    LevelUpHenchman 0x66C7A0 -> AddClass + XP -> AutoLevelup(1)                          v
XP path:     SetExperience 0x6B6BD0 -> stored per-level pending structs ---------------> LevelUp 0x6B9870 (SINK)
```
Best single hook: **`CSWSCreatureStats::LevelUp` 0x006B9870 entry** (reached by every path). Single-player runs client and server
in one process, so the server object is directly readable.

**Q2 - package selection.** There is no creature->package mapping in the level-up path. `AutoLevelup` uses the class row
(`rules+0x124 + classId*0x238`, classId = last class entry) and the shared lists at class+0x1DC/+0x1E4/+0x1EC via
GetRecommendedSkill/Feat/Spell. The creature's +0xE0 package index is loaded into a local and dropped. Consequence: Option A cannot be
made per-character without a hook; changing `packages.2da` changes all creatures of that class. (Who fills the class-row lists from
packages.2da was not traced - not needed for Option B.)

**Q3 - server trust.** `HandlePlayerToServerLevelUpMessage` rebuilds the pending struct straight from the message and calls
`ValidateLevelUp`, which only calls `CanLevelUp` (level < cap, XP >= threshold). `LevelUp` applies feats via `AddFeat`, spells via
`AddKnownSpell`, skill deltas via `SetSkillRank` with no prerequisite or slot-count checks. The server accepts arbitrary picks =>
rewriting the pending struct at `LevelUp` is sufficient (Option B). Granted feats: the GUI path carries them in the pending feat list,
`AutoLevelup` adds grants directly to the creature (AddFeat 0x6B8280) - only rewrite CHOSEN picks, leave grants alone.

**Q4 - unspent picks.** Server: nothing blocks (no completeness check; `+0x170` just stores the remaining skill points).
GUI: Skills panel accept with points left (+0x4CE8 > 0) shows a confirm dialog whose Yes callback (0x90FB00) proceeds, so skill
points may be left unspent; Powers panel accept with picks remaining shows a dialog with NO proceed callback (must pick all powers);
Feats panel accept (0x90B430) NOT read (open). With a `LevelUp`-entry rewrite the GUI gating only matters for what the user is forced to click.

**Q5 - runtime identity.** Server stats: `tag = *(char**)(*(uint8_t**)(stats+0x24) + 0x18)` (CExoString; `GetTag` 0x6B6B30 returns
the CExoString*). Engine itself compares tags with `__stricmp` (0x734F60), e.g. AutoLevelup's hardcoded `"atton"`, `"kreia"`,
`"baodur"`, `"handmaiden"`, `"hanharr"` feat-grant branches - so compare case-insensitively.

**Q6 - layouts.** See "Struct offsets". Skill ranks are a flat byte array indexed by skill id; feats a ptr list of 4-byte records;
classes a stride-0x28 array; pending struct as above.

**Q7 - LevelUpHenchman.** YES shares `AutoLevelup` (0x66C7A0: sets XP to next-level threshold, `AddClass`, then `AutoLevelup(1)` when the
auto-level option flag is set); therefore also reaches `LevelUp`. Identified by behaviour; the NWScript opcode id was not verified.
It parses only 2 arguments (nPackage is unused in K2).

**Q8 - hook overlap.** Scanned all 35 `kotor2-steam-aspyr*.hooks.toml` (166 hook addresses; framework `Patches/` + repo mods).
No hook lies within +-16 bytes of `LevelUp`, `ValidateLevelUp`, `CanLevelUp`, `AutoLevelup`, `SetExperience`, the message handler,
`LevelUpHenchman`, DoAutoLevelup, AutoLevelUpNPCs, DistributeExperience, AddFeat, SendLevelUp or BeginPlayGame.
`k2-finesse-melee-fix` patches 0x006B8DF9 inside `GetWeaponFinesse` (0x6B8DA0) - no conflict.

## Hook shortlist (bytes read from the unpatched exe; identical in the live exe)
1. **`CSWSCreatureStats::LevelUp` 0x006B9870** (file 0x2B8C70). Bytes `55 8B EC 81 EC 80 00 00 00` - a whole-instruction boundary needs
   **9 bytes** (`push ebp` / `mov ebp,esp` / `sub esp,0x80`; 5 bytes would cut the `sub`). thiscall: ECX = stats, `[esp+4]` = pending*,
   `[esp+8]` = addToList int, `ret 8`. Capture `ecx` (pointer), `[esp+4]` (pointer), `[esp+8]`. May read/write the pending struct
   (feats/skills/spells/skill points/ability index) and read the creature tag. Runs once per level actually applied (also from
   SetExperience for stored structs). Risks: `skip_original_bytes=false` runs our code BEFORE the original bytes (lesson 31) - fine for mutation; hidden ECX
   must be captured before the wrapper clobbers it; reentrancy none seen.
2. **`CSWSCreatureStats::ValidateLevelUp` 0x006B9640** (file 0x2B8A40): `55 8B EC 6A FF` (5 bytes, clean). ECX = stats, `[esp+4]` = pending,
   `ret 4`. Reached only by the GUI-message and AutoLevelup flows (not SetExperience). Can also veto by making CanLevelUp fail. Backup site.
3. **`CSWSCreatureStats::AutoLevelup` 0x006BEDF0** (file 0x2BE1F0): `55 8B EC 6A FF` (5 bytes). ECX = stats, `[esp+4]` = notify flag, `ret 4`.
   Only useful if the auto path must be short-circuited before picks are computed; unnecessary when hook 1 rewrites the result.
Hook syntax: `source="ecx", type="pointer"`, `source="[esp+4]", type="pointer"` (bracket form derefs; esp forms work - lesson 29).

## Recommended design and Phase 2 spike
Option B with hook 1: on `LevelUp` entry, look up the creature by case-insensitive tag; if a path exists for its (class, next level),
replace the pending struct's feat list (chosen picks only), power list and skill-delta array with the path picks (use
`PendingLevelUp::AddFeat` 0x6F5B40 / `SetSkillDelta` 0x6F5B00 / array edits, never touch the arrays' allocation directly), set +0x28
to the leftover skill points, then fall through. Spike (Phase 2, needs user consent for a logging-only probe): on a copy of a save in a
new slot, log `LevelUp` entry (tag, class, level, pending dump) for (a) manual GUI level-up, (b) Auto button, (c) Auto NPCs; then
rewrite one feat and confirm (1) the creature gets it, (2) what the client GUI/character sheet shows afterwards (client copy refresh is
the unverified part), (3) feats-panel gating. Static-only was sufficient for Q1-Q8, so no dynamic probe was run in Phase 1.

## Phase 1b results (2026-09-30, static, evidence `research/companion_levelup/phase1_evidence/phase1b_*.txt`)
- **O1 client refresh.** The level-up GUI (ctor 0x8F4AF0) builds a TEMP `CSWCCreature` (0x7645F0, with its own `CSWCLevelUpStats` 0x845AC0) and
  fills it from the live client creature via `CopyCreatureStats` 0x8F6FE0; accept sends the DIFF (0x87A770) and the temp copy is discarded.
  So the GUI can never overwrite a server-side rewrite, and the character sheet reads the real client creature, which the server updates
  through the normal object-update path (same path as script-granted feats). Residual: confirm live that the sheet shows rewritten picks (Step 4).
- **O2 gating.** Feats accept 0x90B430 and Powers accept 0x908B50 only show a popup when picks remain (no hard block for feats; powers popup has
  no proceed callback in the manual flow). Server never checks. So under a rewrite the player can be forced to click through the vanilla picker
  (any legal picks), then the DLL replaces them. Option B UX = "picker is decorative"; making the panels skip needs an extra hook (later, optional).
- **O3 grants vs picks.** `LevelUp` applies EVERY pending feat/power blindly. GUI seeding adds class grants into the pending lists
  (`AddGrantedPowers` 0x847A70: forms/prestige powers 0x90-0xAB, 0x102-0x109 by class+Jedi level; feat grants via the class cls_feat tables),
  `AutoLevelup` adds grants directly (AddFeat 0x6B8280) and only picks go through pending. Rewrite rule: pending feats := (orig entries that are
  in the level's granted set) U path feats; pending powers := (orig entries in the hard-coded engine grant set {0x90-0x94,0x9C-0x9E,0xA4-0xAB,0x102-0x104,0x109})
  U path powers. Granted feat ids per (class, class level) come from feat.2da `<cls>_granted` (emitted by `gen_runtime_table.py`).
- **O4 counts/recommend.** Slot counts: `CalcNumberFeats` 0x847E50 = class row +0x1A9/+0x177 [level-1] (regular, bonus); powers left =
  `ComputeNumberKnownSpellsLeft` 0x847F90 (Jedi class ids only, via 0x6F09D0). Skill purchase 0x846690: class skill 1 pt/rank, cross-class 2 pts per rank,
  max rank (level+3, halved for cross-class). Recommend buttons use `AddDefaultPackage*` 0x8489F0/0x848870/0x848AE0 over the class-row lists (Option A
  dead end unchanged). Rewriting at the sink leaves these GUI counts irrelevant to the final result.
- **O5 layouts.** Pending arrays are `{ptr,count,cap}` (12 bytes each): spells add +0x00/+4/+8, spells remove +0x0C/+0x10/+0x14, feats +0x18/+0x1C/+0x20
  (ushort ids; the "ushort count at +0x1C" reads only the low half - the field is a dword). Push helpers: 0x83EA60 (uint32) / 0x593450 (uint16), ECX = array
  (grow 16 then x2). `PendingLevelUp::AddFeat` = 0x6F5B40 thiscall (ECX=pending, ushort, ret 4; dedups then pushes). GetSkillDelta 0x6F5AC0 ret 4,
  SetSkillDelta 0x6F5B00 ret 8. Spell ids = spells.2da rows (uint32), applied with the class position `local_6`; removal list applied first (0x6B8760).
  Skill bytes are DELTAS; +0x28 is the ABSOLUTE unspent count written to stats+0x170; `addToList != 0` stores the pending POINTER into the stats+0x28 array
  (so never free it; rewrite in place and keep it idempotent - SetExperience later re-applies stored structs via LevelUp(p,0)).
- **O6 callers.** 0x5FD480 = XP award tail (XP >= next threshold and CanLevelUp -> AutoLevelup(1)); 0x74EE90 = party-portrait refresh (companions not under
  player control -> AutoLevelup(1), otherwise shows the level-up indicator). Both funnel into the same sink. LevelUpHenchman opcode id and the class-row list
  filler remain unresolved (not needed for Option B).
- **O7 path semantics -> runtime table.** `research/companion_levelup/gen_runtime_table.py` emits `runtime/levelup_paths.ini`
  (`L<char_level>=class|ability|feats|powers|skill:ranks|granted feats`, plus `cs<class>` class-skill masks). Key at LevelUp time = lowercase tag +
  (total level before LevelUp)+1; class at that level comes from pending +0x2C and must equal the record's class, else do nothing (vanilla). Skill pending
  bytes = record ranks; unspent = orig_unspent + cost(orig deltas) - cost(record ranks) with the class-skill masks (cost 1/2). "Hold until level N" =
  a record with empty lists (feat/power slots simply lapse; only skill points carry over); a missing record or class mismatch = vanilla. Multi-class
  (Atton Scoundrel 19 -> Sentinel 1 at char 20) works because the record carries its class id.

## Phase 2 build (2026-09-30, stage 0 = log + dry run; NOT installed)
- Preflight: live exe bytes at 0x6B9870 = `55 8B EC 81 EC 80 00 00 00` (= vanilla); no hook within +-16 in any `kotor2-steam-aspyr*.hooks.toml`
  (repo + framework, incl. LevelUpLimit); live set 20 ids (patch_config.toml; now incl. `perf-lab` + `load-hang-probe`), kpm_install_state UpdatedAt
  2026-09-30T21:17:18-04:00, 12 DLLs in `patches/` (equipment-3d-character dcbdfde3, perf-lab 82b307af, load-hang-probe 77be826f). Game closed.
- Patch id `level-up-override` (Name `LevelUpOverride`, 0.1.0-probe). Source of record `patch_manager_mods/level_up_override_src/`, build copy
  `$T/Patches/LevelUpOverride/` -> `LevelUpOverride.kpatch` md5 515a915e..., DLL md5 acd1ce14... (mingw i686, `-Wall -Wextra` clean, export `LevelUpOverrideEntry`).
- Hook params (framework `wrapper_context.h` confirms `ebp` = game EBP from PUSHAD and `[esp+N]` = original ESP): `ecx` stats, `[esp+0]` return
  address, `[esp+4]` pending, `[esp+8]` addToList, `ebp` caller frame -> `LevelUpOverrideEntry(stats, ret, pending, addToList, ebp)`.
- Behaviour: never writes game memory (`ApplyEnabled=false`). Every call (mode log|rewrite): SafeRead-guarded dump to `levelup_override_log.txt`
  (game dir; rotated to `.old.txt` > 4 MB; 20000 lines/session): flow (direct ret + 4-frame EBP walk mapped to the doc 08 caller functions ->
  manual-gui / auto-button / auto-npcs / xp-award / distribute-xp / portrait-refresh / levelup-henchman / setexp-stored / auto-other / unknown),
  tag, classes `id:lvl`, total level, key, XP, stats unspent +0x170, pending fields, skill deltas, feats/spells add/remove with count/cap, raw 0x30 hex,
  creature skill ranks and feat ids (stride 4). For tags with a table section: `DRY tag= key= status= feats=[] spells=[] deltas=[] unspent= ability=`
  (status rewrite / already / no-record / class-mismatch / negative-unspent / skipped-unreadable) and `WOULD REWRITE` when mode=rewrite + allow-listed.
- Config `levelup_override.ini` (hot reload by mtime+size): `mode=log|rewrite|off`, `tags=`, `verbose=`, `table=levelup_paths.ini`; missing = log.
  The table must be copied to the game dir (`research/companion_levelup/runtime/levelup_paths.ini`); parse error -> `TABLE ERROR`, dry run off.
- Dry-run rule (twin `research/companion_levelup/simulate_rewrite.py`): feats = orig in granted set + path feats; spells = orig in engine grant set +
  path powers; deltas = record ranks; unspent = orig + cost(orig deltas) - cost(record) (class-skill masks; no mask = cost 1); <0 -> negative-unspent,
  no change; ability = record unless 6 (keep pending); identical result -> `already` (SetExperience replay idempotency).
- Offline checks: `python3 simulate_rewrite.py` (52 records + edge cases, 0 failures); `python3 tools/parity/parity.py` compiles the SAME
  LevelUpOverride.cpp natively against `tools/parity/stub/windows.h` and diffs the DRY lines vs Python: 263 cases, 0 mismatches.
- Console cheats for quick tests (EnableCheats=1; evidence `phase1_evidence/phase2_console_cheats.txt`): `addexp N` = handler 0x7AB690 ->
  `DistributeExperience(N,1)` 0x5FE0C0 (party XP; any resulting companion auto-level logs as flow `distribute-xp`). `addlevel` = 0x7AC1E0 -> XP needed for the
  PC's next level (`GetExpNeededForLevelUp` 0x6B72E0) -> `DistributeExperience`; no-op at PC level 20, or at 15 when the PC has no Force class (0x6F2A30).
  Neither calls LevelUp directly: manual/Auto-button level-ups still need the GUI; natural combat/quest XP is the `xp-award` (0x5FD480) flow.
- **INSTALLED 2026-09-30 21:38 (user consent, Step 3):** backup `~/Games/KOTOR2_Mod_Workspace/backups/pre_levelup_probe_20260930_213810/`
  (kpm_install_state.json, patch_config.toml, patches/; live DLLs == backup re-hashed immediately before), stage `$T/levelup_stage/` (20 live kpatches
  from perflab3_stage md5-matched + LevelUpOverride.kpatch), `KPATCH_NO_LAUNCH=1 ... KPatchLauncher` -> "Applied 21 patch(es)"; `verify` 0 mismatches;
  UpdatedAt 21:38:18; live `patches/level-up-override.dll` md5 acd1ce14. Game dir: `levelup_paths.ini` (copy of runtime table), `levelup_override.ini`
  (`mode=log tags=atton verbose=1`). Rollback: reinstall the 20 ids from `$T/levelup_stage` without `level-up-override`, or `mode=off` (inert).

## Phase 2 live results - run 1 (2026-10-01 18:42, `levelup_override_log.txt`, 4 calls, Atton only)
User: manual GUI level-up Atton 3->4, then Auto button 4->7 (one DoAutoLevelup, 3 LevelUp calls in the same tick). Game stable.
- **Hook plumbing CONFIRMED:** `ecx` = stats, `[esp+0]` = ret 0x6B9770 (inside ValidateLevelUp) for BOTH flows, `[esp+4]` = pending, `[esp+8]` = addToList = 1
  in both (so pending pointers go into the stats+0x28 list), `ebp` walk works: manual = `HandleLevelUpMsg 0x661F26 <- 0x65FC91 <- 0x531585 <- 0x51C442`,
  Auto = `AutoLevelup 0x6C00AF <- DoAutoLevelup 0x850086 <- 0x75BA5A <- 0x75CE6B`. Flow labels manual-gui / auto-button correct.
- **Key arithmetic CONFIRMED:** classes `2:3` at entry for the 3->4 level-up (class level is pre-increment), key = sum+1 = 4; successive calls saw 2:4, 2:5, 2:6.
- **Skill deltas CONFIRMED as deltas:** stats ranks before each call = previous ranks + previous pending deltas (e.g. [3,4,4,5,0,0,5,0] + [0,1,3,..] -> [3,5,7,5,..]).
  Atton spends exactly 4 points/level (class-skill cost 1); `unspent28` = `unspent170` = 0 throughout -> path records cost 4 too, so DRY unspent = 0.
- **Ability idx:** manual L4 = 1 (DEX, the user's pick; path also 1); auto non-ability levels = 6. 6 = none CONFIRMED.
- **Feats:** manual L4 pending feats empty (no Scoundrel feat slot at L4). Auto L5 pending feats = [113] (the auto pick) WITHOUT the L5 grant 62 -> O3 confirmed for
  Auto (grants go straight to the creature). DRY L5 = feats [20] (drop 113, add path feat 20) - correct. Manual-flow grant handling still unobserved (needs a manual
  level-up at a grant level, e.g. Atton 7 (grant 63) or any Jedi level).
- **Spells:** empty in every call (Scoundrel). Force-power grant ids still unobserved (Kreia / Sentinel).
- **Pending raw:** +0x2B HP = 6, +0x2D force = 0, +0x2E/+0x2F garbage (padding). Empty arrays are {0,0,0} - adding to them must go through the engine push helpers.
- **Bug in the probe's creature dump:** `stats.feats count=0` - it read the uses-per-day list (+0x0C). Fixed layout above (Ghidra: `phase1_evidence/phase2_featlist_*.txt`).
  Next build logs stats+0x00 and +0x18 lists.
- Not yet covered: Kreia manual (e), Auto Level Up NPCs (c), natural XP award (d), SetExperience replay, mode=rewrite.
- 0.1.1-probe built (kpatch md5 121b8a25, NOT installed): creature feat dump fixed (+0x00 and +0x18 lists). Live is still 0.1.0 (acd1ce14).

## Phase 2 live results - run 2 (2026-10-02 00:32, new game session, 2 calls, player character)
Live set is now 26 ids (a sibling session reinstalled 2026-10-01 23:36, adding character-effects-panel, k2-activation-fix, k2-equip-icon-fix, companion-ai,
ranged-ai); `level-up-override` survived unchanged (dll acd1ce14, 0.1.0).
- Both calls = the PC (tag empty -> logged `tag=?`; no table section, no DRY line), Jedi Sentinel (class 5) 7 -> 8, same XP 33272 and same starting ranks, but
  DIFFERENT stats pointers (0b581280 manual-gui, then 1036e2a8 auto-button 16 s later) - consistent with a reload between the two (not yet confirmed with user).
- **Force powers CONFIRMED in pending spells-add** as uint32 spells.2da rows: manual pick [0x87], Auto pick [0x2F]; count 1 cap 16. No engine-grant power at
  Sentinel 8 (none expected). spellsRem empty.
- +0x2D force points gain = 6, +0x2B HP = 8, ability = 1 in both (Auto also picked DEX at an ability level). Skill deltas manual [1,0,0,1,1,0,1,1] / Auto [0,0,5,..]
  (5 points each, unspent 0). No feat entries at char level 8.
- Still uncovered: manual level at a feat-grant level, Kreia (path tag) powers + DRY on spells, Auto NPCs, natural XP award, SetExperience replay.

## Open items
- Live confirmation only (Steps 3-4): real pending dumps per flow (manual / Auto button / Auto NPCs / XP award), that `ecx`/`[esp+4]` behave as documented in the
  hook framework, and that the sheet shows rewritten picks. Everything else statically closed.
- Unresolved, not needed for Option B: LevelUpHenchman NWScript opcode id; who fills class-row recommended lists; exact hidden ECX of `AddGrantedPowers` param_2.
- Alternative kept as fallback: stored per-level structs (`stats+0x28` "LvlStatList", consumed by SetExperience) - data-only route, unverified for companions.

## Commands run
- `python3 patch_manager_mods/ghidra_knowledge/gog_cluster_delta_map.py --selftest` (PASS 10/10); then per class regex `^CLASS$` with `--out` for
  CSWSCreatureStats, CSWCLevelUpStats, CSWPartyTable, CSWGuiInGameCharacter, CSWGuiLevelUpCharGen, CSWGuiPowersLevelUp, CSWGuiFeatsCharGen,
  CSWGuiSkillsCharGen, CSWGuiAbilitiesCharGen, CSWGuiInGameGameplay, CSWCMessage, CSWSMessage, CClientOptions, CGuiInGame.
- Ghidra (`analyzeHeadless ... KOTOR2_SteamAspyr -process swkotor2.exe -noanalysis -readOnly`, serialised, no lock issues): `DecompileArgs.java`
  (5 batches, ~60 functions), `Callers.java` (2 batches), `DisasmRanges.java 6bedf0-6c0330`.
- PE VA->file-offset read of `.unpatched_vanilla` and live exe for `original_bytes`; regex scan of all `kotor2-steam-aspyr*.hooks.toml`.
- Knowledge DB: ~40 `name_map` rows (source `level-up-P1-decompile-confirmed`) and 5 `lessons` rows added.

## Session notes / tooling (so a context clear loses nothing)
- Helper scripts: `research/companion_levelup/tools/` (ghidra_run.sh, kdb_add.py, pebytes.py). Raw decompiles/asm/xrefs/delta maps:
  `research/companion_levelup/phase1_evidence/`. Ghidra runs take ~7 s each when no other session holds the project.
- Gotchas hit: (1) `DecompileArgs.java` takes Steam VAs, not GOG addresses - passing a GOG address decompiles an unrelated function;
  (2) Ghidra's decompiler mis-names locals in AutoLevelup (class row is really `[EBP-0x44]`; the package row `[EBP-0x4C]` is written once, never read);
  (3) `steam_functions.tsv` columns are addr,size,retn,name; (4) the repo `.venv` python is the one with pykotor; system python3 was enough here;
  (5) `git commit -- <paths>` is required because another session's staged file (`11_inventory_sort_filter.md`) sits in the index.
- The game install dir is `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/`; `swkotor2.exe.unpatched_vanilla` == Ghidra program (sha 6A522E71...).

## Session notes - Phase 1b (2026-09-30, so a context clear loses nothing)
- Commit `19a2af2` holds Phase 1b (docs, DB rows, evidence, generator). Preflight verified: live `swkotor2.exe` (md5 c9a1e3c5...) has identical bytes to `.unpatched_vanilla` at 0x6B9870/0x6B9640/0x6BEDF0/0x661C30; rollback exe backup `swkotor2.exe.backup.20260930_111425` (same size/date as live).
- Tool usage learned: `tools/pebytes.py [swkotor2.exe] VA...` (the optional first arg must end in `exe`; default = unpatched_vanilla; do NOT pass a full path). `tools/ghidra_run.sh DecompileArgs.java out.txt <hexVAs>`, `DisasmRanges.java out.txt 6f5ac0-6f5b80,...` (ranges `start-end`), ~7 s per run, prints BUSY if another analyzeHeadless runs. Evidence files: `phase1b_gui_gating|grants|containers|client_refresh|callers|disasm|misc.txt`.
- Runtime table: `cd research/companion_levelup && ../../.venv/bin/python gen_runtime_table.py 2>/dev/null` (refuses paths with check errors). Format documented in the script docstring. Kreia path has 21 warnings (unused slots), Atton 0.
- Granted-feat sets in the table: e.g. Atton L20 (Sentinel 1) grants 39,43,44,55,98,107,207; L5 Scoundrel grants 62. Engine force-form power grants (0x847A70) are NOT in the table (hard-coded id set, see O3).
- Not yet verified live (everything below is the ground truth the probe must supply): hook param passing for `ecx`/`[esp+4]`/`[esp+8]`, whether `GetLevel` (0x6A4E00) is pre-increment at LevelUp entry (it is called before LevelUp bumps class level, so key = level+1), what `AddGrantedPowers` param_2/ECX really is, whether feat grants appear in the manual-flow pending list.
- Untracked-but-modified noise to ignore when committing: `research/companion_levelup/logs/debug_pykotor.log*`, `__pycache__`. Always `git commit -- <explicit paths>`.

## Session notes - Phase 2 (2026-09-30 .. 10-02, so a context clear loses nothing)
- Plan for Step 4/5 (apply code 0.2.0 + Atton rewrite spike): `~/.claude/plans/dazzling-brewing-quasar.md`.
- Commits: 023f41f (stage 0 DLL + simulate_rewrite.py + parity), 768bfbd (install record), fec34f8 (console cheats), 25513d0 (run 1 + feat-list fix, 0.1.1),
  6c70444 (run 2). Branch KOTOR-II has sibling commits interleaved; never push/amend; `git commit -- <paths>`.
- Build: edit `patch_manager_mods/level_up_override_src/` (source of record) -> `cp` the 4 files to `$T/Patches/LevelUpOverride/` -> `cd` there,
  `python3 ../create-patch.py` (works without KPATCH_ADDRESS_DATABASES now). Warning check: `i686-w64-mingw32-g++-posix -std=c++17 -Wall -Wextra -fsyntax-only
  -I $T/Patches/LevelUpOverride $T/Patches/LevelUpOverride/LevelUpOverride.cpp`. Parity: `cd research/companion_levelup && python3 tools/parity/parity.py`
  (compiles the real .cpp natively with `tools/parity/stub/windows.h`; the harness `#include`s the .cpp, so new Win32 calls need stubs there, and any
  engine-calling apply code must be kept out of the pure functions or stubbed). `python3 simulate_rewrite.py` = Python twin self-test.
- Install (user-gated): backup `$G/{kpm_install_state.json,patch_config.toml,patches/}` to `~/Games/KOTOR2_Mod_Workspace/backups/<name>_<ts>/`;
  `python3 patch_manager_mods/mods678_research/stage_and_verify_install.py stage $T/<stage> <LevelUpOverride.kpatch>` (finds every LIVE id's kpatch by DLL md5;
  live set is 26 ids as of 10-01 23:36 - sibling sessions change it, ALWAYS re-read); re-hash live DLLs vs the backup right before; `KPATCH_NO_LAUNCH=1
  $T/bin/NoLaunch/KPatchLauncher "$G/swkotor2.exe" --patches $T/<stage> <IDS printed by stage>`; then `... verify $T/<stage>` (0 mismatches).
  Game-dir runtime files: `levelup_paths.ini` (copy of `research/companion_levelup/runtime/levelup_paths.ini`), `levelup_override.ini`
  (`mode=log tags=atton verbose=1 table=levelup_paths.ini`), log `levelup_override_log.txt` (new game session appends after a `== ... loaded` header).
- Console cheats for testing (EnableCheats=1): `addexp N`, `addlevel` (both -> DistributeExperience; level-up then still via GUI/Auto). Player tag is empty -> `tag=?`.
- Gotchas: `pgrep -f analyzeHeadless` inside a bash -c command matches itself (BUSY false positive) - use `pgrep -f '^java.*analyzeHeadless'` or ignore;
  `tools/pebytes.py swkotor2.exe <va>` (first arg literally `swkotor2.exe`); `tools/kdb_add.py` has `add_names(rows, src)` / `add_lesson(scope, topic, note)`
  (import from python, no CLI); sqlite3 CLI is not installed (use python sqlite3).
- Runtime facts to design Step 4 on (all live-confirmed unless noted): pending arrays `{ptr,count,cap}`, empty = {0,0,0} (pushes must use engine helpers so the engine
  allocator owns the memory); Auto pending feats = chosen only (grants added directly); manual-flow grant presence UNOBSERVED; spells-add = uint32 spells.2da rows;
  addToList=1 in manual + Auto (pending pointer stored in stats+0x28 -> rewrite must be idempotent); SetExperience replay UNOBSERVED.
