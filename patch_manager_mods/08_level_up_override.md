# 08 - Level-up override (per-character fixed level-up path)

Phase 1 (read-only Ghidra investigation) record. Plan: `research/companion_levelup/PHASE1_PLAN.md`; overall:
`research/companion_levelup/MASTER_PLAN.md`. Target: Steam Aspyr LAA `swkotor2.exe` (sha256 6A522E71... = `.unpatched_vanilla`; the
live exe 4AB72FC1... is the patched one - bytes at every shortlisted site are identical in both). Ghidra image base 0x400000; file
offset of a .text VA = VA - 0x400C00. All Steam functions below were read in a decompile (2026-09-30), not just address-matched.

## Status
**Product requirements (user, 2026-10-02):** (R1) the mod must NOT override a player's manual level-up picks - it pre-determines the
**Auto level-up** (Auto button / Auto Level Up NPCs) so players can use auto-level and still get their desired build; (R2) builds are the player's choice
per playthrough (not one fixed path); (R3) configuration through an in-game GUI - a page in the game settings or a new main-menu page (to be researched:
CSWGuiMainMenu / options panels; no Ghidra work on them yet; DB has only DisplayMainMenu 0x73F230 / ShutDownToMainMenu 0x78E0F0 address matches).
0.2.1 implements R1 (rewrite only when AutoLevelup 0x6BEDF0 is on the call chain; manual GUI -> `SKIP manual ... player's picks kept`).
(R4, user 2026-10-02) **Full auto mode:** with the whole path mapped, a configured character should level with NO player interaction (no level-up screen,
no Auto button). (R5) **Hold mode:** if the plan is to bank levels until a class becomes available (e.g. Jedi), the mod holds the level-up (XP keeps accruing)
and hides the character's level-up indicator, which otherwise persists and stops buff icons rendering. Static leads: O6 - portrait refresh 0x74EE90 already
calls AutoLevelup(1) for companions not under player control and otherwise shows the level-up indicator (find that branch); XP-award tail 0x5FD480 ->
AutoLevelup(1); CanLevelUp 0x6B9790 / ValidateLevelUp 0x6B9640 are veto points. CORRECTION (Phase 3, 2026-10-02): a vanilla option EXISTS - swkotor2.ini `[Game Options] Auto Level Up NPCs` / options GUI checkbox `CB_LEVELUP` (see "Phase 3 research" item 1).
(R6, user 2026-10-02) **Character-screen entry point:** a "Configure auto level-up" button on the character screen; a character the player has not
configured keeps vanilla behaviour. (R7) Instant level-ups (R4) are **opt-in per character**, not the default. (R8) Builds come from **shipped preset paths**
per character and/or a guided **"wizard"** level-up UX where the player plans the build. (R9, user 2026-10-02) **Configure nudge:** when the mod is installed
and a character is NOT configured, the level-up UX (level-up / Auto level-up buttons and their prompt) should be interwoven with a reminder/option to configure
the mod for that character when it is ready to level up (encourage, never block vanilla levelling).
(R11, user 2026-10-02 after the M2 install) **Held indicator:** while a character is HELD (has the XP to level but the mod is banking levels for the
configured path), show something telling the player so (e.g. "holding for Jedi" on the portrait / character screen) - the vanilla level-up indicator is
hidden by the hold gate, so without this the player cannot tell banked XP from no XP. (User 2026-10-02: M2 NOT tested yet; test EVERYTHING TOGETHER -
build M3 save binding + M4 Configure UI/wizard + R11 offline, then one combined live test.)
Phase 3 research plan: `~/.claude/plans/dazzling-brewing-quasar.md` (static only; evidence `research/companion_levelup/phase3_evidence/`).
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
| Options accessor (P3) | - | 0x0072FB00 | `return *(*(this+4)+4)`; obj +8 bit0 = "Auto Level Up NPCs" | fastcall | high |
| SetAutoLevelUpNPCs (P3) | - | 0x007B5EF0 | writes +8 bit0; if set -> AutoLevelUpNPCs 0x5FEDD0 | thiscall ret 4 | high |
| Options ini reader / writer (P3) | - | 0x007B88B0 / 0x007B5F50 | [Game Options] "Auto Level Up NPCs" | - | high / medium |
| Portrait overlay SetPortraitOverlay (P3) | - | 0x00744F00 | (slot,visible,mask,buffs,debuffs); bit4 = LBL_LEVELUP; any mask zeroes buff counters | - | high |
| CSWGuiInGameCharacter ctor / SetStats / HandleInputEvent (P3) | - | 0x0084C3A0 / 0x0084E6F0 / 0x0084F900 | buttons +0x34F0 AUTO, +0x36C0 LEVELUP; SetStats CanLevelUp @0x84F221 | thiscall | high / high / medium |
| AddMultiClass handler (P3, DB "LevelUpHenchman") | - | 0x0066C7A0 | AddClass + 1-level XP bank + option-gated AutoLevelup @0x66C976 | - | medium |
| CSWSCreatureStats::AddClass (P3) | - | 0x006BDCD0 | count +0x8D capped 2 (overwrites last), level 0 | thiscall ret 8 | high |
| CSWGuiLevelUpCharGen accept / BeginPlayGame / cancel (P3) | - | 0x009048A0 / 0x008F8070 / 0x008F82C0 | accept -> BeginPlayGame -> SendPlayerToServer_LevelUp 0x87A770; cancel restores live from temp (+0x68) | fastcall | high |
| CServerInfo ctor (P3) | - | 0x007EB5C0 | MaxLevel +0x94 = 50 (CanLevelUp via 0x51BD40) | - | medium |

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

## Phase 2 Step 4 (0.2.0 apply code, 2026-10-02, built + offline-verified, NOT installed)
- `LevelUpOverride.cpp` 0.2.0 (`ApplyEnabled=true`): `computeRewrite` unchanged except `Result.nKeptFeats/nKeptSpells` (entries of `in` kept, in order, as
  the prefix of out); DRY string unchanged. New: `readPend`/`logPend` (entry dump and AFTER re-read share one reader; engine pointers read as uint32),
  `probeWritable` (VirtualQuery, committed + RW/WC/XRW/XWC, no guard), `applyRewrite`, `samePend`, `Engine g_eng = {0x6F5B40, 0x6F5B00, 0x83EA60, checkCode}`.
- `applyRewrite` preconditions, all before the first write (failure -> `APPLY ABORTED <why> - vanilla (no write)`): skill count 8; pending 0x30 writable;
  feats/spells counts still == the values read; kept prefix in range; feats/spells arrays writable for the read counts; deltas 8 bytes writable; the three
  helpers' first bytes match (live exe: AddFeat `55 8B EC 51 89 4D FC`, SetSkillDelta `55 8B EC 83 EC 08 89 4D F8`, push `55 8B EC 83 EC 08 89 4D FC`).
  Writes: feats kept prefix in place + dword count +0x1C, then `AddFeat(pending, id)`; spells kept prefix + count +0x04, then `PushU32(pending+0x00, id)`;
  spells-remove untouched; `SetSkillDelta(pending, i, d)` i=0..7; ushort +0x28; byte +0x2A. No allocation of our own.
- `onLevelUp` for allow-listed tags with mode=rewrite: `already` -> `ALREADY (replay) ... - no write`; class-mismatch / no-record / negative-unspent /
  skipped-unreadable -> `SKIP <status> ... - vanilla`; `rewrite` + apply=0 -> `WOULD REWRITE ... (apply=0)`; else apply, `saferead::beginScope()`, re-read,
  `APPLIED tag= key= ok=0|1` (ok = re-read == computeRewrite out) + `after:` dump (same format as `pend:`). Creature feat lists +0x00/+0x18 logged per call (verbose).
- Config: new `apply=1` (default 1) second kill switch; CONFIG line prints it. Manifest 0.2.0.
- Offline: mingw `-Wall -Wextra` clean; `parity.py` 263/0 (DRY unchanged); new `tools/parity/apply_test.cpp` (run by parity.py on the same cases): fake engine
  helpers (AddFeat dedup+push, grow 16/x2; SetSkillDelta idx<8; uint32 push) in an `mmap(MAP_32BIT)` arena (no `g++ -m32` libstdc++, no wine on this box);
  per rewrite case 3 variants (cap==count -> forced growth, cap 16, empty {0,0,0}): applied 356, AFTER == out, untouched bytes (spells-remove, deltas ptr,
  HP/class/force/padding) unchanged, engine call count exact, replay -> `already` 356/356, abort path (count changed) writes nothing 194/194, 0 failures.
  Mutation check: dropping the feat count write, pushing to the wrong array, and removing the count precheck all fail the test (kept-prefix off-by-one is an
  equivalent mutant: AddFeat re-adds the entry).
- asm check (`-O2 -S`): AddFeat call = `mov ecx,pending; [esp]=id; call 0x6F5B40; push` (callee cleanup), PushU32 ecx=pending (+0x00), SetSkillDelta ecx=pending,
  [esp]=idx [esp+4]=value. Export `LevelUpOverrideEntry`; imports KERNEL32 + msvcrt only; hooks.toml original_bytes `55 8B EC 81 EC 80 00 00 00` == live exe.
- Build: `LevelUpOverride.kpatch` md5 c58520e7..., DLL (`binaries/windows_x86.dll`) md5 bcdc9862....
- **INSTALLED 2026-10-02 00:53 (user consent):** backup `~/Games/KOTOR2_Mod_Workspace/backups/pre_levelup_020_20261002_005342/` (live DLL md5s + state
  re-checked unchanged right before; prior UpdatedAt 00:42:15 by a sibling), stage `$T/levelup_stage2/` (25 live ids from hotbar_stage8 md5-matched + 0.2.0,
  PROBLEMS 0), "Applied 26 patch(es)", `verify` 0 mismatches, UpdatedAt 00:53:50, live dll bcdc9862. Game dir `levelup_override.ini` = `mode=rewrite tags=atton
  apply=1 verbose=1`; `levelup_paths.ini` == runtime table. Log was 46 lines before the spike. Rollback: `mode=off` (instant) or reinstall the backup's set.

## Phase 2 Step 4b (0.2.1, 2026-10-02): auto-only gate (R1)
- `viaAutoLevelup(ret, frames)`: true when the direct return or any of the 4 EBP frames lies in AutoLevelup 0x6BEDF0 (run 1 chains: Auto = `0x6C00AF` in frame 0,
  manual = HandleLevelUpMsg chain without it). Allowed tag + mode=rewrite + not via AutoLevelup -> `SKIP manual <tag> key= flow= - player's picks kept`, no write.
  SetExperience replays of manual structs are therefore never rewritten; replays of rewritten auto structs report ALREADY.
- apply_test.cpp also asserts the gate on the run 1 chains. parity 263/0, apply_test 0 failures, -Wall clean. kpatch c03922f2, dll 693053d9. 0.2.0 (bcdc9862) is
  live from 00:53 and still rewrites manual picks - replace before the spike.
- **INSTALLED 0.2.1 2026-10-02 00:58 (user consent):** backup `backups/pre_levelup_021_20261002_005828/`, stage `$T/levelup_stage3/` (26 ids, PROBLEMS 0), "Applied 26",
  verify 0 mismatches, live dll 693053d9, UpdatedAt 00:58:35. Config unchanged (`mode=rewrite tags=atton apply=1 verbose=1`).

## Phase 2 live results - run 3 (rewrite spike, 2026-10-02 ~01:02, 0.2.1, `mode=rewrite tags=atton`; log copy (local, untracked) `research/companion_levelup/logs/live/levelup_override_log_run3_20261002.txt`)
User: one Auto-button level-up of Atton 3 -> 7 (4 LevelUp calls in one tick, flow auto-button, chain via AutoLevelup 0x6C00AF). Game stable.
- **All 4 calls `APPLIED ok=1`** (re-read == computeRewrite out). No ABORTED / SKIP. Engine helpers called live without fault (thiscall plumbing CONFIRMED).
- **Skill deltas took effect:** creature ranks before each next call = previous ranks + REWRITTEN deltas: [3,4,4,5,0,0,5,0] +[0,0,2,2,..] -> [3,4,6,7,0,0,5,0]
  +[0,0,2,1,0,0,1,0] -> [3,4,8,8,0,0,6,0] +[0,0,1,1,0,0,2,0] -> [3,4,9,9,0,0,8,0] (vanilla Auto picks were [0,1,3,..], [0,2,2,..], [0,3,1,..]).
- **Feat replacement took effect:** L5 Auto pick 113 replaced by path feat 20 (`after.feats [20]`); the creature feat list at the next call has 20 and no 113.
- **Grants unaffected:** Scoundrel grants 62 (L5) / 63 (L7) appear in the creature list directly (already before the LevelUp call of that level - AutoLevelup
  adds them first, O3), never in pending; the rewrite does not touch them.
- Ability: L4 record 1 == Auto pick 1; other levels 6 (none) kept. Spells empty (Scoundrel). HP/force/class bytes unchanged in `after.raw`.
- Not covered by this run: manual GUI level-up (expect `SKIP manual`), save/reload persistence (no LevelUp call on load; user sheet check), path feat 77 at L8,
  Kreia powers, Auto Level Up NPCs / XP-award flows, SetExperience replay (ALREADY).

## Phase 3 research (R4-R9, static, 2026-10-02; evidence `research/companion_levelup/phase3_evidence/`)
### Item 1 - the auto-vs-indicator gate (Step 1, agent A, spot-checked)
- **Vanilla option exists.** The `0x72FB00`-returned options object (`__fastcall return *(*(this+4)+4)`, called with `[0xA1B4A4]+4`; ~264 callers; class
  name unconfirmed, probably CClientOptions; differs from DB "GetClientOptions" 0x73F830 which returns `*(this+4)+8`) holds **+8 bit0 = "Auto Level Up NPCs"**.
  Setter `0x7B5EF0` (`+8 = +8&~1 | v&1`; if set and `[0xA1B4A4]+8` != 0 -> `0x51C8D0` + AutoLevelUpNPCs 0x5FEDD0 = immediate catch-up of the party)
  (`A1_dec_5fedd0_callers.txt:35-46`). Ini reader `0x7B88B0` (section "Game Options", key "Auto Level Up NPCs" @0x9A038C, push at 0x7B89B1, setter call
  0x7B8A5C; `A2_disasm_7b88b0_full.txt:102,148`), called from options-load 0x79F6F0; saver 0x7B5F50 via 0x79F6C0. GUI: `CB_LEVELUP` in optgameplay_p, handler
  `0x8E0C50` toggles via the setter (registered in 0x8DF930; `A2_tagxrefs_autolevel.txt:35-39`). Vanilla ini default `Auto Level Up NPCs=0`.
  AutoLevelUpNPCs 0x5FEDD0 has ONE caller: the setter (`A1_callers_5fedd0.txt`).
- **Polarity:** bit0=1 -> UpdatePortraits takes `AutoLevelup(1)`; bit0=0 -> indicator (mask|=4).
- **UpdatePortraits 0x74EE90 branch** (`A1_disasm_74ee90.txt:342-383`): `0x74F3C7 call CanLevelUp` -> `jz 0x74F4E3`; `0x74F3E9 call 0x72FB00`
  (`e8 12 07 fe ff`); `0x74F3FA mov edx,[ecx+8]; and edx,1; 0x74F400 jz 0x74F481` (`74 7f`, indicator); player-creature compare `0x74F443`/`0x74F449 jz`;
  `0x74F44B` creature(`[ebp-0x38]`)+0x1200 test, `0x74F461 jnz 0x74F481` (`75 1e`); `0x74F472 push 1; 0x74F47A call 0x6BEDF0` (`e8 71 f9 f6 ff`); `jmp
  0x74F4DE`; `0x74F481..0x74F485 or edx,4` (indicator bit into byte [ebp-0x31]).
- **local_3c = the creature** (`[ebp-0x38]`, stats at +0x1198). **creature+0x1200 = GFF "PlayerCreated"** (loader 0x574180, store 0x57432C; other writers
  0x6B2BA7 in 0x6AFED0, zero-init 0x5630CD). PlayerCreated creatures and the player creature always get the indicator, never auto-level.
- **Answer to the user ("is auto level-up an option to enable?"):** yes - Options > Gameplay > "Auto Level Up NPCs" (ini `Auto Level Up NPCs=1`). It is
  GLOBAL (all companions, not the PC), and the mod's 0.2.1 rewrite already applies to those auto level-ups (AutoLevelup on the chain).
- **R4 hook candidates** (live == vanilla bytes, no hook within +-16 bytes across 84 Steam hook TOMLs; nearest = BuffDurationHUD 0x74F25A/0x74F8AB):
  | VA | bytes | use |
  |---|---|---|
  | 0x74F3E9 | e8 12 07 fe ff | **preferred**: detour the call; return a fake/real options ptr whose +8 bit0 = 1 for configured+opted-in tags (creature in [ebp-0x38]) |
  | 0x74F400 | 74 7f | global force (= option ON) - not per creature |
  | 0x74F461 | 75 1e | let PlayerCreated companions auto-level |
  | 0x74F47A | e8 71 f9 f6 ff | the AutoLevelup call itself (observation / per-creature veto) |
- **R4 design choice (Opus):** per-creature detour of the 0x72FB00 call at 0x74F3E9 (and the equivalent gate sites in XP tail 0x5FD480 / DistributeExperience
  0x5FE0C0 - addresses to collect in Step 4). Reasons: reuses the engine's own auto path (same frame cadence, same AutoLevelup(1) call that 0.2.1 already
  rewrites), no new per-frame logic, unconfigured characters see the real option value (R6/R7 vanilla default). Calling AutoLevelup ourselves from another
  per-frame hook was rejected: duplicates the gate (CanLevelUp, player, PlayerCreated) and risks double-levelling in the same tick.
- Residual: class name of the 0x72FB00 object (decompile 0x796E00 if needed); whether 0x5FD480/0x5FE0C0 also test PlayerCreated (Step 4).

### Item 2 - level-up indicator, buff-icon bug, CanLevelUp callers, character-screen buttons (Step 2, agent B, spot-checked)
- **Indicator = mask bit 4** in `FUN_00744f00(slot, visible, mask, buffCnt, debuffCnt)`: sets bit1 (visible) of the control flags at +0x448 (LBL_LEVELUP)
  when `visible && mask&4` (`B1_disasm_744f00_counters.txt:125-130`).
- **R5 buff-icon bug mechanism (CONFIRMED):** `0x745061 movzx edx,[ebp+0xc]; test; jz` -> if the WHOLE mask byte is nonzero, both counters are stored as 0:
  `0x74507F mov [ecx+0xddc],dl` (buffs), `0x7450A3 mov [edx+0xddd],al` (debuffs) (`B1_disasm_744f00_counters.txt:148-167`). Any overlay (levelup 4,
  debilitated 2, bit 1) hides the buff/debuff rows. Clearing ONLY bit 4 restores the icons when no other bit is set.
- **CanLevelUp 0x6B9790 callers (21 sites, `B1_callers_canlevelup.txt`):** UpdatePortraits 0x74F3C7 (indicator / AutoLevelup / debilitated branch);
  secondary portrait updater 0x756B70 (4 sites, buff counting + same overlay logic; partly inferred); character screen SetStats 0x84F221 (button
  visibility); 0x752050 (HUD flags, inferred); 0x7C9DF0 (opens GUI on tab 2, inferred); 0x75FC30 (one-shot "k_level_dlg" for PlayerCreated,
  `B1_dec_canlevelup_callers.txt:1507-1569`); XP tail 0x5FD480, DistributeExperience 0x5FE0C0, 0x66C7A0 (auto-level when option on); AutoLevelUpNPCs 0x5FEDD0
  (unconditional, no option test); SetExperience 0x6B6BD0 (writes XP +0x68 first, then applies stored pending levels + FeedbackMessage(0xb)); message
  handler 0x661C30 and ValidateLevelUp 0x6B9640 (real manual level-up paths); 0x63EDC0/0x63F550/0x6415E0 (client/server stats sync caches CanLevelUp).
- **Veto effect (CanLevelUp -> 0 for a held creature):** removes the portrait indicator and both character-screen buttons and stops every auto path; XP keeps
  accruing (SetExperience writes +0x68 before the check). Side effects: no FeedbackMessage(0xb), no k_level_dlg, stored pending levels not replayed in
  SetExperience, the sync message carries 0. No saved state found (runtime caches only). Debilitated branch `0x74F4E3..0x74F4F8` (CanLevelUp==0 &&
  IsDebilitated(0) -> mask|=2) is the correct vanilla overlay; a held + debilitated creature shows the debilitated overlay (vanilla-consistent).
- **Character screen (CSWGuiInGameCharacter):** ctor 0x84C3A0 binds BTN_AUTO (panel+0x34F0) and BTN_LEVELUP (panel+0x36C0) (`B1_tagxrefs_levelup.txt:462-467`).
  SetStats 0x84E6F0 (every tick via Update 0x84FBB0) calls CanLevelUp at `0x84F221` (`e8 6a a5 e6 ff`) and writes bit1 into both buttons' +0x48 flags
  (`B1_disasm_charscreen.txt:35-60`). Clicks: `AddEvent(0x27, panel, thunk)` - BTN_AUTO at 0x84DB6A -> thunk 0x7FCF80 -> vslot+0x5C 0x40E800 ->
  HandleInputEvent 0x84F900 (0x29) -> StartAutoLevelup 0x84FE20; BTN_LEVELUP at 0x84DBB9 -> thunk 0x762E80 -> vslot+0x54 0x40E7E0 -> HandleInputEvent(0x27)
  -> ShowLevelUpGUI 0x84FCD0. **HandleInputEvent does NOT test CanLevelUp** - hotkeys (0x61/0x78) reach ShowLevelUpGUI even with hidden buttons: a hold veto
  must also guard ShowLevelUpGUI/StartAutoLevelup (or rely on ShowLevelUpGUI's own checks - verify in Step 6).
- **R5 hook candidates** (live == vanilla, no toml hook within +-16; nearest = LevelUpOverride 0x6B9870, 14 bytes past CanLevelUp's end):
  | VA | bytes | use |
  |---|---|---|
  | 0x6B9790 | 55 8B EC 83 EC 20 | **REJECTED (Step 4): HANG HAZARD** - AutoLevelup's loop (`asm_auto.txt:1127-1150`) jumps back to 0x6BEED7 when ValidateLevelUp->CanLevelUp fails; loop condition is only XP>=table -> infinite loop. Never veto inside CanLevelUp/ValidateLevelUp/LevelUp |
  | 0x74F3C7 | E8 C4 A3 F6 FF | UpdatePortraits-only veto |
  | 0x84F221 | E8 6A A5 E6 FF | char-screen button visibility only (also the R9 hook point: show a Configure button when CanLevelUp && unconfigured) |
  | 0x74F481 | 0F B6 55 CF 83 CA 04 | clear only the indicator bit (7-byte hook) |
- **Design note:** a return-address allowlist is fragile; alternative = veto for held tags at every caller EXCEPT the manual-apply paths (0x6B9640/0x661C30)
  and SetExperience replay (decide in Phase 3 design; Step 4 checks banking/replay).
- **Buff HUD note:** the level-up indicator (and any overlay bit) zeroes buff counters at 0x74507F/0x7450A3 - the Buff HUD session owns that area; this doc is
  the pointer, no Buff HUD code changed.
- Residual: ECX at 0x84F221 = stats ptr (likely; read the `[ebp-0x28]` assignment); 0x7CEEF0 one-shot at 0x74F4B4 (short decompile); 0x7C9DF0 / 0x752050
  effects (live log-only probe with a held creature).

### Item 3 - class availability (Jedi) and the AutoLevelup class choice (Step 3, agent C, spot-checked)
- **Jedi/prestige classes are added ONLY by script** (AddMultiClass): vanilla callers are just `a_makejedi.nss` (Atton/Mira -> Sentinel 5, Bao-Dur/Handmaiden ->
  Guardian 3, Disciple -> Consular 4) and `a_pc_jedi.nss` (PC prestige 0xB-0x10), no level/XP handling in the scripts (`C1_scripts_addmulticlass.txt:1-113`).
  The level-up GUI has no class picker: the client copy levels its LAST class (`C1_dec_levelup_gui.txt:190-199`); server LevelUp would append a pending class
  that is absent, with no prestige test (`dec_3ab.txt:~440-456`) - so never inject a class via the pending struct.
- **Handler 0x66C7A0** (DB name "LevelUpHenchman", inferred = ExecuteCommandAddMultiClass: pops (int class, object); sole caller of AddClass) (`C1_addmulticlass.txt`).
  **AddClass 0x6BDCD0:** class count `stats+0x8D` increments only while < 2, else the LAST slot is overwritten; new class level 0 (`C1_dec_addclass.txt:35-47`).
  Class entry i: `stats + i*0x28 + 0xAB` id, `+0xAC` level. After AddClass the handler raises a companion's XP to `XPTable[total level]` if below (banks
  exactly ONE level; SetExperience never lowers XP), then - only if the vanilla option is ON and CanLevelUp - calls AutoLevelup(1) once.
- **AutoLevelup picks the class once, before its loop: `idx = count-1` (newest class)**, then loops `while XP >= table[total level]`, setting pending +0x2C
  (`dec_3ab.txt:~818-843`). So ALL banked levels go to the newest class in one call.
- **"Class available" test:** 0.2.1's `record.class == pending class` is correct; "class in list" is necessary but not sufficient (only the last class levels).
- **Hold-mode rule** (`C1_hold_rule.txt`):
  ```python
  def autolevel_class(classes):      # [(class_id, level)] in stats order
      return classes[-1][0] if classes else None
  def level_now(classes, record):    # record = next path record for key level+1, or None
      return record is not None and autolevel_class(classes) == record["class"]
  ```
  Anything else = hold (XP banks). Prestige needs no special case. Preset constraints: after conversion every record is the Jedi class; max 2 classes.
- Residual: classes.2da ids for 0xB-0x10 (read the 2DA); AddMultiClass routine id (routine table entry for 0x66C7A0); which .dlg calls a_makejedi at what level.

### Item 4 - banking, level cap, XP-award gates (Step 4, agent D; hang hazard spot-checked)
- **HANG HAZARD (CONFIRMED, `phase1_evidence/asm_auto.txt:1127-1150`):** inside AutoLevelup 0x6BEDF0, `0x6C00AA call ValidateLevelUp; jnz 0x6C0114`, failure frees
  the record and falls to `0x6C0114 jmp 0x6BEED7` (loop head); the loop test is only `XP >= rules+0x38[level]` (0x6BEF15/0x6BEF1B). A veto inside CanLevelUp
  / ValidateLevelUp / LevelUp leaves the level unchanged -> infinite loop (game freeze). **The hold veto must sit at CALLER gates only**, before AutoLevelup.
- **Level cap:** CanLevelUp reads `*(*(app+4)+0x10004)+0x94` via 0x51BD40 (`D1_dec_51bd40.txt:20`); server-info ctor 0x7EB5C0 sets +0x90 MinLevel=1, +0x94
  MaxLevel=0x32 (50) (`D1_dec_7eb5c0.txt:68-69,210-218`). Global, no PC/companion difference. Residual: overwrite from save/ini not checked.
- **XP table:** CSWRules ctor 0x69DEA0 loads 2DA EXPTABLE, 51 rows, into rules(0xA1B4D0)+0x38+4*i (`D1_dec_exptable_loader.txt`). Live values: row 1 = 1000,
  row 14 = 105000, row 15 = 120000, row 49 = 1225000, row 50 = 0xFFFFFFFF (`D1_xp_table.txt`).
- **XP-award gates** (`D1_disasm_xp_gates.txt`), all: call 0x72FB00 -> +8 bit0 test -> CanLevelUp -> push 1 -> AutoLevelup; none tests player/PlayerCreated:
  | path | 0x72FB00 call | jz | CanLevelUp call | AutoLevelup call | creature |
  |---|---|---|---|---|---|
  | XP tail 0x5FD480 | 0x5FD7A0 | 0x5FD7B7 | 0x5FD7CE | 0x5FD7EE | [ebp-0x14] |
  | DistributeExperience 0x5FE0C0 | 0x5FE281 | 0x5FE292 | 0x5FE2A3 | 0x5FE2BD | [ebp-0x18] |
  | AddMultiClass 0x66C7A0 (non-PC branch, +0x1200==0) | 0x66C93A | 0x66C94B | 0x66C95C | 0x66C976 | [ebp-8] |
  | AutoLevelUpNPCs 0x5FEDD0 (no option test) | - | - | 0x5FEE32 | 0x5FEE4C | |
  | char-screen DoAutoLevelup 0x850010 (OK callback; AutoLevelup(0), no CanLevelUp/option test) | - | - | - | 0x850081 | |
  All 5-byte `E8 rel32`; live == vanilla; +-16 overlap scan vs 77 hook files / 431 hooks clean (`D1_hook_overlap_scan.txt`). CanLevelUp caller sites take ECX =
  stats (creature = stats+0x24) -> **safe per-creature hold veto = detour each caller-site CanLevelUp call** (0x74F3C7, 0x84F221, 0x5FD7CE, 0x5FE2A3,
  0x66C95C, 0x5FEE32) + guard 0x850081 (button path) and the hotkey path (Step 6). **Per-creature R4 force = detour the 0x72FB00 calls** (0x74F3E9, 0x5FD7A0,
  0x5FE281, 0x66C93A).
- **Banking:** one AutoLevelup call applies ALL banked levels (loop); keys are sequential level+1 per LevelUp (class index fixed at loop start, level rises
  each iteration) - no skipped or doubled key (CONFIRMED). DoAutoLevelup / StartAutoLevelup do not loop themselves (StartAutoLevelup only precomputes the target
  level for the box text) (`D1_dec_doautolevelup.txt`). SetExperience replays `list[+0x28][totalLevel]` via LevelUp(rec,0) without re-appending; ValidateLevelUp
  appends applied records; a companion's list count = its level, so replay does not fire in normal flows (inferred) (`D1_setexperience_replay_notes.txt`).
- **Live-test expectation:** Atton held 8 -> XP ~105000 (row 14), then a_makejedi adds Sentinel. Option ON (or R4 force for atton): the 0x66C976 gate fires in
  the same frame as AddMultiClass -> one AutoLevelup, 7 LevelUp calls, keys 9..15, all class Sentinel (5), stops at 15 (XP < 120000). Option OFF / not
  forced: XP stays banked until the toggle (0x5FEDD0), the char-screen Auto button (0x850081), a portrait-tick AutoLevelup (if forced) or manual level-ups.
  Note: with hold mode the path's records for keys 9..15 must be Sentinel records; Scoundrel records for those keys would be skipped as class mismatch.
- Residual: the PC exclusion in 0x5FD480/0x5FE0C0 (decompile 0x58A9F0/0x6B7060); AddMultiClass XP-penalty arithmetic with banked XP (`table-XP` negative ->
  "Cannot reduce XP", likely harmless; log XP at 0x66C95C live); MaxLevel overwrite (FindFieldWrites 94 on the server-info object).

### Item 5 - configuration entry point (R3/R6/R9) (Step 5, agent E, `E_entry_point_notes.txt`)
- Character screen is already hooked by Mod 14 (`active_effects_panel_src`): ctor bind 0x84D871, per-tick Update 0x84FBB0, dtor 0x84DEF0. Live character_p.gui:
  BTN_AUTO id 65 [362,260,94,52], BTN_LEVELUP id 66 [362,325,94,40]; free space under BTN_LEVELUP (~y 370) or beside BTN_EXIT (x 256-344, y 516); 800x600 units.
- Hotbar framework (reusable, copy-pasted per mod today; only `_shared_src/SafeRead.h` is shared): label bind `new(0x148)` + CSWGuiLabel ctor 0x419740 + InitControl
  0x40F620, text 0x416E30, rect via label vtable slot 1, clicks by GetAsyncKeyState polling against pixel extents, hit-test toggled via control +0x48 bit5.
- StartAutoLevelup 0x84FE20 builds a CSWGuiMessageBox (`confirm_p`, OK/Cancel only) - a 3-way popup is not available without a new panel.
- Storage: sidecar INI in the game dir keyed by creature tag (case-insensitive; LevelUpOverride.cpp:121-194 already does this). No playthrough/save id exists yet
  (R2 needs one: save-folder name or a "profile" selector).
- **Recommendation:** (1) "Configure" label/button on the character screen (3-5 days, Mod 14 hooks + hotbar picker pattern); (2) R9 nudge = show it only when
  CanLevelUp (the 0x84F221 result) && tag unconfigured, plus a hint line in the Auto confirm box (+1-2 days). Click-intercept popup 3-4 days; options or
  main-menu page 1-2 weeks each (new panel class, wiring unknown).

### Item 6 - wizard feasibility (R8) (Step 6, agent F, spot-checked)
- **CORRECTION to Phase 1 wording:** the level-up GUI does NOT edit a throwaway copy. The CSWGuiLevelUpCharGen ctor makes a temp creature (baseline, this+0x68),
  then mutates the LIVE client creature: 0x8F7FE0 bumps its last-class level (`F1_dec_accept.txt:380-397`), 0x8F65D0 writes HP/FP. Accept (0x9048A0 ->
  BeginPlayGame 0x8F8070 -> SendPlayerToServer_LevelUp 0x87A770, its only caller chain, `F1_callers_87a770.txt`) sends live-minus-temp. Cancel 0x8F82C0
  CopyCreatureStats(temp -> live) restores (`dec_b2.txt:1335+`).
- **ShowLevelUpGUI 0x84FCD0 guards only** `GetGuiInGame()+300 != 0` and `GetCharacterChangeInProgress()==0` (`B1_dec_charscreen.txt:50-51`): no CanLevelUp,
  no XP. HandleInputEvent event 0x27 has the same guards -> **a held creature can still open the level-up GUI via hotkey** (R5 must guard 0x84FCD0 and
  StartAutoLevelup/DoAutoLevelup 0x850081 for held tags). The GUI level = total class levels + 1 (0x844860), so it opens repeatedly without XP.
- **Plan-mode wizard (single level):** hook the BeginPlayGame 0x8F8070 entry, read the diff (live vs this+0x68, the fields 0x87A770 reads), write it as the
  next path record, then jump to cancel 0x8F82C0 instead. Returning early from 0x87A770 alone is UNSAFE (BeginPlayGame still closes and sets live +0x458
  bit2 -> client at L+1, server at L). Effort ~3-5 days. Multi-level (reopen without cancel, chained baseline, restore own snapshot via 0x7645F0 + 0x8F6FE0
  at the end) ~1.5-2 weeks, risk: server updates clobbering the live client creature mid-chain. Custom picker (hotbar framework, re-implements prereq /
  slot / skill rules) 2-4 weeks. Presets only: days (15 presets now exist: `paths/presets/`).
- **Verdict:** ship presets first, then the single-level vanilla-GUI wizard; multi-level optional.
- Residual: 0x8F8070 calling convention (likely fastcall ECX=this), meaning of +0x458 / 0x73F810 (one headless run), server clobber (log-only probe).

### Preset paths (R8, 2026-10-02, `research/companion_levelup/paths/presets/`)
**Update (wizard matrix, commits 9c91ab2..ae4c46d):** 54 presets with `[meta.wizard]` answers (schema `paths/presets/WIZARD_SCHEMA.md`): Atton 7, Kreia 7,
Bao-Dur 8, Disciple 8, Handmaiden 8, Mira 8, Visas 8; all `check_paths.py` 0 errors. Coverage tables + open questions per character in `<char>.md`.
0.3.0 follow-ups found by the preset agents: (1) `check_paths.py` ignores "Class Skill: X" feats (184/186/187/188 etc.) -> false cross-class costs / unspent
warnings (Kreia, Bao-Dur); (2) `hold_until_class` semantics + the early-conversion re-base rule (Item 7 note); (3) mod #142 alignment source (companion's own vs
Exile's) unverified -> wizard should offer `dark` only when it makes sense; (4) neutral caster presets run out of universal powers ~char 25 (filled with light).
(Original 15-preset summary follows.)
15 presets, 3 per character, all `check_paths.py` 0 errors, all obey "newest class only / max 2 classes" (Item 3); per-character notes `<char>.md` list sources
and open questions. Bao-Dur: early_guardian (convert 12) / late_guardian (22) / tech (30). Disciple: caster (7) / melee (16) / support (13). Handmaiden:
unarmed_jedi / saber_guardian (convert_at 7, `hold_until_class = 3`) / unarmed_soldier. Mira: early_sentinel (7) / ranged_sentinel (17) / scout. Visas (tag
`VisasMarr`): duelist / force_support / dark (conflicts with mod #142 unless dark). Not covered: HK-47, T3-M4, G0-T0, Mandalore, Hanharr (validator lacks
classes 6-8). Paths end at char 30 (engine cap is 50, EXPTABLE 51 rows; practical cap of a playthrough ~30).

### Phase 3 synthesis - mechanisms for 0.3.0
- **R4 instant auto levelling (opt-in per tag, R7):** detour the options-accessor calls (0x74F3E9 portraits, 0x5FD7A0 XP tail, 0x5FE281 DistributeExperience,
  0x66C93A AddMultiClass) to return "+8 bit0 = 1" for opted-in tags; everyone else sees the real option. The existing 0.2.1 rewrite applies the path.
  PlayerCreated creatures and the PC still take the indicator branch in 0x74EE90 (0x74F449/0x74F461); PC is out of scope unless the user asks.
- **R5 hold (per tag, rule = Item 3 `level_now`):** veto at CALLER sites only (never inside CanLevelUp/ValidateLevelUp/LevelUp - hang, Item 4): CanLevelUp
  calls 0x74F3C7, 0x84F221, 0x5FD7CE, 0x5FE2A3, 0x66C95C, 0x5FEE32 (+ the 4 sites in 0x756B70 and 0x752050 to check) and guard 0x84FCD0 / 0x850081 for held
  tags. Hiding the indicator restores buff icons (Item 2). When the Jedi class appears, the next AutoLevelup applies all banked levels in one loop.
- **Unconfigured characters (R6):** untouched - vanilla option, vanilla indicator, vanilla buttons, plus the R9 nudge.
- **Proposed 0.3.0 build order:** (1) hold mode + indicator fix (caller-site vetoes, held-tag guard on the GUI entry points), offline harness + log-only live
  check; (2) opt-in instant levelling (accessor detours); (3) per-tag config storage + character-screen "Configure" button + R9 nudge; (4) preset selection
  UI, then the single-level vanilla-GUI wizard.
- **Open user decisions:** (a) presets vs wizard vs both (recommend presets now, wizard later); (b) entry point (recommend character-screen button + R9
  nudge); (c) default for unconfigured characters = vanilla (R6, assumed); (d) PC included? (e) per-character preset choices + the per-character questions in
  `paths/presets/<char>.md`; (f) R2 playthrough id (save-folder name vs profile selector); (g) droid/non-Jedi companions in scope?

### User decisions (2026-10-02, after Phase 3 research)
- (a) **Presets AND wizard** - both. (b) **Configure button on the character screen: yes** (+ R9 nudge). (c) **Unconfigured characters stay vanilla: yes.**
- (d) **Player character: NO** for this mod. Future idea (R10, separate mod): a PC level-up **filter** that hides skill/feat/power options not aligned with the
  chosen PC path, to reduce clutter while keeping real choice (not a forced path).
- (e) **Presets:** generate MULTIPLE logical presets per character from the build research, organised by the **wizard questions** (the wizard maps answers ->
  preset; dynamic per-answer values can be integrated later). Schema: `research/companion_levelup/paths/presets/WIZARD_SCHEMA.md`.
- (f) **Playthrough identity: tied to the save file** (R2) - static research item "save binding" (evidence `phase3_evidence/G1_*`).
- (g) Droids / non-Jedi companions: **yes** (user 2026-10-02) - presets for HK-47 5, T3-M4 6, G0-T0 5, Mandalore 5, Hanharr 5 (check_paths now accepts
  classes 6/7, commit 06bd7dc). **Total: 80 presets / 12 companions**, all 0 errors.

### Item 7 - save binding for per-playthrough config (decision (f); agent G, spot-checked; evidence `phase3_evidence/G1_*.txt`)
- **Server load** 0x52FB80 (`thiscall(app, slot, CExoString* name, a4, a5)`, builds `SAVES:%06d - %s`) -> PARTYTABLE 0x5FBF60 -> **GLOBALVARS load 0x656710**
  -> LoadModule 0x531C50; thunk 0x51BE20 from msg handler 0x660040 (minor 5 load / 4 save). Autosave-restore 0x8529E0 also calls 0x656710 -> single post-load point.
  **Save:** StartSave 0x52F610 -> SaveGameInProgress 0x534600 -> globals saver **0x656640** (call 0x5348F5); DoPCAutosave 0x53AF20 also calls it (0x53B219).
  savenfo (AREANAME, LASTMODULE, TIMEPLAYED, SAVEGAMENAME, TIMESTAMP, PCNAME, ...) has no unique playthrough id (`G1_dec_save_paths.txt`).
- **Global variable table** at CServerExoAppInternal+0x100FC: Boolean cap 900, Number 1000 (1 byte, 0-255), Location 100, String 5; names <= 21 chars. Saved by
  name (Cat/Val lists). **On load 0x655B00 looks each saved name up (0x6545B0) and ADDS unknown names (0x654430 bool / 0x654490 number / 0x654550 string)**
  (`G1_dec_globalvar_load_named.txt:114-120`) -> a DLL-added global round-trips through saves without a globalcat.2da row. Add-var `thiscall(table, name)`;
  Number setter 0x654CC0, String setter 0x654F00. Live globalcat.2da: 973/1000 Number, 176/900 Boolean, 3/5 String used (`G1_globalcat_live_counts.txt`).
- **New game:** main-menu handler 0x882230 -> 0x401BC0 recreates CServerExoApp (globals zeroed, globalcat via 0x51C450) -> chargen, module 001ebo. Load screen
  0x853A00 also goes through 0x401BC0 -> GUID 0 after module entry = new playthrough (or a pre-mod save); no leak from the previous session.
- **Design (chosen):** playthrough GUID in DLL-added save-persisted globals (e.g. 4-8 Booleans/Numbers named `LUO_GUIDn`, generated on first config use when
  0), config in a sidecar INI keyed by GUID. Hooks: globals saver entry 0x656640 (add+set before save) and post-load 0x656758 (read after the load call;
  bytes `89 45 F0 C7 45 FC FF FF FF FF`). Follows the save across slots and Save As; old saves get a GUID on first use; uninstall leaves harmless orphan globals.
  All candidate sites live == vanilla, +-16 overlap clean (`G1_hook_overlap_scan.txt`).
- Residual: hook-framework param capture at 0x656758 and save-writer acceptance of a name added at 0x656640 entry -> one log-only live probe.
- **Preset design gap (from Atton presets):** if the story converts a character EARLIER than a preset's `convert_at`, the remaining base-class records never
  match the newest (Jedi) class -> held forever. 0.3.0 needs a re-base rule (Jedi present -> continue from the preset's Jedi records). Converting later is fine (banked).

## Phase 4 M0 facts (static, 2026-10-02, agent S1 spot-checked; evidence `research/companion_levelup/phase4_evidence/S1_*.txt`)
- **Hold-veto sites** (CanLevelUp `E8` calls, no stack args, CanLevelUp plain RET, only EAX used after, live == vanilla, +-16 clean; param `ecx` = stats;
  `stats+0x24` is a creature POINTER (0x6B97F4), tag = `*(*(stats+0x24)+0x18)` (server creature confirmed; client side inferred)): 0x74F3C7, 0x84F221, 0x5FD7CE,
  0x5FE2A3, 0x66C95C, 0x5FEE32, 0x756DD4, 0x7575B5, 0x757808, 0x75798C (0x756B70), 0x75218B (0x752050), 0x7C9F3B (0x7C9DF0), 0x75FCB8 (0x75FC30 k_level_dlg).
  LEAVE: 0x6B6E33/0x6B6EB6 (SetExperience), 0x6B9672 (ValidateLevelUp), 0x661FA6 (msg handler), sync callers 0x63F003/0x63F68A/0x63F6E2/0x64166E (`S1_sites.txt`).
- **Instant sites** (0x72FB00 calls, only `[ret+8]&1` read, creature slot written before the call): 0x74F3E9 [ebp-0x38], 0x5FD7A0 [ebp-0x14], 0x5FE281 [ebp-0x18],
  0x66C93A [ebp-8]; param `ecx` + `ebp`.
- **GUI guards** (`S1_gui_guards.txt`): ShowLevelUpGUI 0x84FCD0 and AutoLevelup take a pushed arg + RET 4 -> never skip their call sites. Guard instead
  the GetCharacterChangeInProgress 0x740FC0 call: **0x84FD2F** `e8 8c 12 ef ff` (inside ShowLevelUpGUI; button, hotkey AND script 0x7CB8E6; return nonzero =
  vanilla early-out) and **0x84F9FE** `e8 bd 15 ef ff` (HandleInputEvent case 0x29 -> confirm box never built); backstop **0x850062** `e8 99 d7 f2 ff`
  (DoAutoLevelup: 0x77D800 call returns the creature; return 0 -> jz skips AutoLevelup). Creature at the guards = leader via
  `0x77D800(0x7E5DA0(0x73FB90([A1B4A4]+4), 0))`, stats `+0x1198` (same chain as Mod 14 `leaderClient()`). Hotkey->event mapping not located (only path is
  HandleInputEvent, so covered).
- **SetStats** 0x84E6F0 (`S1_setstats.txt`): at 0x84F221 panel = `[ebp-0x288]`, ECX = `[ebp-0x28]` = leader stats; one CanLevelUp result drives BOTH
  buttons; also reached from vtable slot 0x84E6C0 and from ShowLevelUpGUI (not only per tick).
- **Globals** (`S1_globals.txt`): AddBoolean 0x654430 (no dup check -> Find 0x6545B0 first), SetBoolean 0x654BB0 `(table,name,v)` RET 8, GetBoolean 0x654740
  `(table,name,int*)` RET 8, SetNumber 0x654CC0, GetNumber 0x654860 `(table,name,byte*)`; all thiscall, name = CExoString*. Saver 0x656640 prologue confirmed,
  writer loops the live table. Post-load 0x656758: table = `[ebp-0x1C]`. Chain: table = `*(*(*(0xA1B4A4)+8)+4) + 0x100FC` (address). Server recreate signal:
  hook **0x401C52** `e8 f9 a7 11 00` (call 0x51C450 in 0x401BC0); pointer polling is unreliable (same address reuse).
- **UI** (`S1_template_gui.txt`, `S1_control_lifetime.txt`): template label `LBL_EXPERIENCE_STAT` (plain text; green text) / `LBL_XP_BACK` (framed);
  BTN_AUTO [362,260,94,52], BTN_LEVELUP [362,325,94,40], BTN_EXIT [159,516,89,28]. Panel dtor 0x84DEF0 -> 0x84DF70 -> 0x40E860 never frees panel+0x24
  entries -> append once per panel, never free, key state on the panel pointer.
- **Unresolved -> M2/M3 live logs:** client creature tag offset at the guards (log tag; unreadable tag = pass through); save writer accepting names added at
  the 0x656640 entry + the 0x656758 capture (M3 probe); 0x756B70 slot controls #2-#4 roles; 0x75FD90 target.
- **Hold rule correction (Opus, supersedes the plan's char-level-first rule):** char-level-first lookup re-applies the first Jedi record after an early
  conversion (creature rebased onto the Jedi L1 record at char 7; at char 8 the char-8 record is again Jedi L1 and matches the newest class). Rule v2,
  keyed by CLASS level: `rec = preset[(newest class, its class level + 1)]`; found -> LEVEL (tag `PATH` if `rec.char_level == key` else `REBASE`);
  else if the preset has a Jedi class (a record class different from the base class) that the creature lacks and `hold=1` -> `HOLD`; else `VANILLA`
  (`NOREC`). On-track it equals the char-level lookup. The table records carry an explicit class level (7th field) from M1 on.

## Phase 4 M1 results (data + tooling, 2026-10-02)
- **Runtime table v2** (`gen_runtime_table.py`, `runtime/levelup_paths.ini`): `[classskills]`, `[presets]` index
  `<tag>__<variant>=role|style|alignment|convert|skills|convert_at|hold_class|short`, one section per path, `L<n>=` records with a 7th field
  `class_level` (creature level in that class after the record). 82 paths (dev `atton__live`/`kreia__live` + 80 presets), 2047 records, max 27 per preset,
  max char level 30, max per record: 1 feat / 2 powers / 7 granted / 7 skill entries, max line 120 chars, 70830 bytes -> C++ caps for M2.
  `hold_class` = first record class differing from the utc's newest class (Atton/Mira 5, Bao-Dur/Handmaiden 3, Disciple 4, all others 0); matches
  `meta.hold_until_class` everywhere. Every preset has `[meta] short` (<= 60 chars, wizard display).
- **Tags** (`phase4_evidence/M1_tags.txt`): all 12 preset tags match the utc Tags case-insensitively; saves store them lowercase (atton, kreia, t3m4 seen)
  -> case-insensitive match required. Save slot `availnpc7` (Mira) holds a placeholder tag `3cfd` before recruitment - never match on it.
- **Class Skill feats** (`levelup_rules.class_skill_feats()`, check_paths): 183 Awareness, 184 Computer Use, 185 Demolitions, 186 Repair, 187 Security,
  188 Stealth, 189 Treat Injury (no Persuade feat). Kreia's utc has 188 (Stealth class-cost from the start); presets taking 185: g0t0 ranged stealth (L15),
  g0t0 support (L12), t3m4 support single (L12). **M2 requirement:** the DLL's unspent recomputation must OR the per-class skill mask with the creature's
  feats 183-189 (plus feats in the record being applied) - else it charges 2 per rank and loses points. 82/82 paths 0 errors; warnings 199 -> 277: five
  presets planned with the old cost now leave a point unspent per level (open: re-plan them; harmless, points stay unspent).
- **Python twin** (`simulate_rewrite.py`): `parse_table_v2`, `level_now(classes, preset, key, hold, hold_class)` = hold rule v2 (class-level keyed; HOLD
  only when the preset's records for the current base class are used up, the hold class is missing and hold=1 - below the path start = VANILLA),
  `simulate_auto_levelup` (engine loop twin; a HOLD inside the loop becomes `HOLD_MIDLOOP` = engine picks, because the LevelUp hook can never veto).
  Self-test cases: on track, early conversion (no re-apply regression), late conversion hold on/off, droid, unconfigured, below path start, Atton banked
  7 -> Sentinel added -> keys 8..15 PATH, mid-loop. `python3 simulate_rewrite.py` 0 failures. `tools/parity/parity.py` is EXPECTED to fail until M2
  (the 0.2.1 C++ parser rejects 7-field records).

## Phase 4 M2 build - 0.3.0-a (2026-10-02, built + offline-verified, NOT installed)
- **Code** (`level_up_override_src/LevelUpOverride.cpp`): table v2 parser (MaxPresets 128, 256 KB buffer), per-tag dev config in `levelup_override.ini`
  (`preset.<tag>=<id>`, `instant.<tag>=0|1` default 0, `hold.<tag>=0|1` default 1; legacy `tags=` -> `<tag>__live`, hold 0, instant 0 = 0.2.1 behaviour;
  `tagConfig()` carries the M3 TODO), `levelNow` (hold rule v2 twin), LevelUp hook now logs `DECIDE tag preset key decision rec` (HOLD there =
  `HOLD_MIDLOOP` -> vanilla, never a veto), `skillCost` honours Class Skill feats 183-189 the creature already has (`csExtraMask`). Gates (pass-through on
  anything unreadable/unconfigured/PC/mode off; mode=log logs `WOULD HOLD` without acting; logs only on change; config/table stat throttled to 1 s):
  hold gate x13 (`LuoHold_<VA>`, param ecx), instant gate x4 (`LuoInstant_<VA>`, ecx+ebp; returns a 16-byte static copy of the options object with +8 bit0
  set), GUI guards `LuoGuardGui_84FD2F` / `LuoGuardAuto_84F9FE` (return 1 = refuse; log `GUARD-PROBE` with the tag read via creature and via stats, 300
  lines max, to settle the client-tag question), Auto OK backstop `LuoAutoOk_850062`. Engine function prologue bytes are checked once; mismatch -> all gates
  pass through (logged `ENGINE ...`). Toml: 21 hooks (LevelUp entry + 20 call-site detours, `skip_original_bytes=true`, `exclude_from_restore=["eax"]`).
  manifest 0.3.0.
- **Gates (all green):** mingw `-Wall -Wextra -fsyntax-only` (with `-I $T/Patches/LevelUpOverride`, the `../_shared/SafeRead.h` include resolves there);
  `parity.py`: DRY 18426 cases 0 mismatches, levelNow 25366 cases (all 82 presets) 0 mismatches (PATH 7366 / REBASE 14820 / HOLD 160 / VANILLA 3020),
  apply_test 0 failures, NEW `tools/parity/gate_test.cpp` 72 checks 0 failures (calls the shipped exports with fake engine functions);
  `simulate_rewrite.py` 0 failures; mutation spot-check 10/10 killed (levelNow key/clvl/top/hold/has conditions, newest-class index, rewrite-vs-log veto,
  instant hold check, instant bit offset). Independent Sonnet review: no blocker; wrapper_x86.cpp confirms params from the PUSHAD copy, ESP restored from
  EBX (cdecl safe), EAX kept, jump to site+5; all 21 original_bytes live == vanilla; +-16 overlap clean vs 132 tomls + live patch_config.toml; no hold
  site reachable from AutoLevelup (direct-call graph depth 6). Review SHOULD-FIX "thread safety" = moot (all logged LevelUp calls tid 316, one thread);
  NITs (UNHOLD gap for unconfigured, caps untested) accepted.
- check_paths: Class Skill feats now count from the NEXT level (skills are picked before feats in the level-up GUI; matches the DLL). 82/82 0 errors,
  275 warnings.

### M2 install (2026-10-02 10:32, user-consented)
- LIVE: `level-up-override` 0.3.0-a, dll md5 9e8cbbff, 28 ids, kpm UpdatedAt 2026-10-02T10:32:03, stage `$T/levelup_stage4/`, verify 0 mismatches.
  Backup (= 0.2.1 set + old ini/table): `~/Games/KOTOR2_Mod_Workspace/backups/pre_levelup_030a_20261002_103155/`.
- Game dir: `levelup_paths.ini` = table v2 (repo `runtime/levelup_paths.ini`), `levelup_override.ini` = `mode=rewrite apply=1 verbose=1
  preset.atton=atton__melee_dual_light_early hold.atton=1 instant.atton=0` (switched from atton__live, which stays Scoundrel to char 19 and would
  not hold a level-7 Atton). Log `levelup_override_log.txt` had 175 lines before the M2 live test.
- Rollback: `mode=off` (hot) or reinstall the backup set.

## Phase 5 UX research (2026-10-02, two Explore reports over Mod 3 / Mod 14 source; plan `research/companion_levelup/PHASE5_PLAN.md`)
- **Runtime controls need the layout open.** InitControl 0x40F620 from a template tag needs the panel layout GFF (panel+0x30), which is only open inside
  the panel ctor before StopLoadFromLayout 0x40F5A0 (DB lessons 67/113/283) -> the per-tick SetStats hook CANNOT create template labels. Mod 14 owns the
  ctor bind hook 0x84D871 and ships a custom `character_p.gui` (`LBL_EFX_*` ids 69..150, generator `scripts/mirror_character_page.py`) -> we need our own
  ctor-time hook site (U1).
- **Label recipe** (Mod 3 `ForcePowerHotbar.cpp` `createLabel` 982-1043): new(0x148) + ctor 0x419740 (vtable 0x9878BC); InitControl(panel, ctl,
  templateTag, addToList=0), id = control count; rect via vtable slot 1, text 0x416E30 (ECX = label+0xF0), font 0x416DE0, align 0x416FA0, fill
  border(+0x60)+0x74, tint border+0x28, alpha border+0x24, text colour 0x417140; append via 0x83EA60 on panel+0x24. Flags +0x48: bit1 visible, bit5 0x20
  click-through (keep 0x20 set while hidden, clear while shown so clicks never reach the world - lesson 192). Draw order = list index (append = on top);
  hit-test walks from the highest index. DLL-set text draws left-aligned (lesson 265). Runtime extents are engine pixels.
- **Templates** in the live `character_p.gui`: `LBL_EXPERIENCE_STAT` (id 25, plain, green text), `LBL_XP_BACK` (id 3, framed).
- **Free area** under BTN_LEVELUP ~ x 362..456, y 370..430 (800x600 units). Mod 14's panel occupies x 536-752 -> keep the wizard in x ~150..530.
- **Placement:** Mod 14 method (`CharEffectsPanel.cpp` `ux`/`uy` 368-375) = reference label pixel rect + (unit - refX) * scale; handles aspect + ultrawide.
- **Clicks:** `GetAsyncKeyState(VK_LBUTTON)` edge detection gated on a foreground-PID check (Mod 3 `cursorInGame` 476, `wHit` 729, poll 829-832, hover
  sound `playHoverSound` 608), hit-test against the control extent (+4/+8/+0xC/+0x10). Esc closes the whole character screen (vanilla); the wizard resets
  on panel close -> no Esc-swallow hook.
- **Wizard data** (`runtime/levelup_paths.ini` `[presets]`): 5-8 presets per companion; <= 5 answers on any page (fits 6 cells); auto-skip one-answer pages;
  jump to the result as soon as one preset remains.
- **Build:** `create-patch.py` compiles every `*.cpp` in the patch dir + subdirs; shared headers live in `patch_manager_mods/_shared_src/`, copied to
  `$T/Patches/_shared/` (included as `../_shared/X.h`).

### User decision R11 (2026-10-02): status line + banked message
- R11 = a **character-screen status line** ("Holding N level(s) for <Jedi class> - <short>" / "Auto path: <short>") **plus a one-time message when a
  level is banked**. NO portrait badge, NO tinted vanilla level-up icon (the indicator would suppress Buff HUD's icons). Portrait badge = open-later idea.

## Phase 5 U facts (static, 2026-10-02, Sonnet agent, Opus spot-checked; evidence `research/companion_levelup/phase5_evidence/U1-U4_findings.txt`)
- **U1 ctor-time creation site = 0x84D82C** `68 DC 39 9A 00` (push imm32 of "LBL_STATSBORDER" for the last InitControl 0x84D85D; plain relocatable
  instruction, statement boundary, nothing pending on the stack). Panel = `[ebp-0x254]` for the whole ctor; layout open (StopLoad call 0x84D877); 69 bytes
  before Mod 14's 0x84D871 (`8B 8D AC FD FF FF`, captures `[ebp-0x254]`, binds `LBL_EFX_*` with InitControl addToList=1 and EXPECTS ids 69..150).
  InitControl with addToList=1 writes `list[id]` (pads / OVERWRITES) -> **we create with addToList=0 at 0x84D82C and append to panel+0x24 on the first
  SetStats tick** (after the ctor, so after Mod 14). Alternatives 0x84D83C / 0x84D84B (6 B). Re-opening the layout (StartLoad 0x40EE40 from SetStats)
  rejected (re-parses + re-scales the panel); template-less label recipe recorded in U1_findings as fallback. Live == vanilla, +-16 clean.
- **U2 banked message:** vanilla FeedbackMessage(0xB) = server 0x584DA0 `thiscall(creature, u16 id, payload*)` RET 8, delivered ONLY to the controlled
  creature (companions never see it). Client list append **0x7BE090** `thiscall(GuiInGame = *(clientInternal+0x40), CExoString* text, int 0x80, byte 0)`
  RET 0xC (clientInternal = `*(*(*(0xA1B4A4)+4)+4)`); the engine's id-0xCC path (= SendMessageToPC) is 0x73F100(clientApp, 0xCC, payload, CExoString*)
  -> 0x78F160 -> 0x7BE090. **Chosen: call 0x7BE090 directly** (no payload ownership), guarded on the non-null chain; no engine rate limit (list cap 0x3F)
  -> the mod de-dups per (GUID, tag, banked count). Single game thread (server + client) -> calling it from the server XP gates is safe.
- **U3 save binding:** saver 0x656640 entry `55 8B EC 6A FF` (5 B, 3 instr) ECX = table, [esp+4] = save path, RET 4. CExoString ctor from char* 0x733570
  `thiscall(buf8, const char*)` RET 4, dtor 0x733780, empty ctor 0x733540. Post-load: hook **0x65675B** `C7 45 FC FF FF FF FF` (one 7-byte instruction)
  with params `[ebp-0x1c]` (table) + `eax` (load result) - simpler than 0x656758 (3+7 B). 0x656710 has exactly two callers (0x52FD0A server load,
  0x852F77 autosave-restore 0x8529E0) -> covers all loads. **0x401C52** `E8 F9 A7 11 00` call 0x51C450 (thiscall, no args, plain RET), ECX = new server
  app, EAX unused by 0x401BC0 and its 5 callers. All live == vanilla, +-16 clean (`U3_site_check.txt`). Still live-unproven: framework capture at
  0x65675B, save writer accepting names added at 0x656640.
- **U4:** class ids 0 Soldier, 1 Scout, 2 Scoundrel, 3 Jedi Guardian, 4 Jedi Consular, 5 Jedi Sentinel, 6 Combat Droid, 7 Expert Droid, 8 Minion, 9 Tech
  Specialist (live classes.2da + dialog.tlk; 11-16 prestige). XP table `rules(0xA1B4D0)+0x38+4*i` = XP to REACH level i+1 (51 rows, row 50 = 0xFFFFFFFF);
  banked = `while L < MaxLevel && table[L] <= XP: L++, b++`.
- DB: name_map source `level-up-P5-static-confirmed` (7 rows) + 3 lessons.

## Open items
- Live confirmation only (Steps 3-4): real pending dumps per flow (manual / Auto button / Auto NPCs / XP award), that `ecx`/`[esp+4]` behave as documented in the
  hook framework, and that the sheet shows rewritten picks. Everything else statically closed.
- Phase 3 residuals (all cheap): 0x72FB00 object class name; PC exclusion in 0x5FD480/0x5FE0C0; MaxLevel overwrite; AddMultiClass XP-penalty with banked XP;
  0x756B70/0x752050/0x7C9DF0 effects under a veto; 0x8F8070 convention, +0x458, 0x73F810; server clobber during a plan-mode GUI. Close with one log-only live probe + 1-2 headless runs.
- Unresolved, not needed for Option B: LevelUpHenchman NWScript opcode id (likely = AddMultiClass, Item 3); who fills class-row recommended lists; exact hidden ECX of `AddGrantedPowers` param_2.
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

## Session notes - Phase 2 Step 4/5 (2026-10-02 00:40 .. 01:15, so a context clear loses nothing)
- Commits: 783abf3 (plan pointer), 668a6a0 (0.2.0 apply code + apply_test), ccff838 (0.2.0 install), 361b673 (0.2.1 auto-only gate + R1-R3), f2b2f03 (0.2.1
  install), 83ecb30/67ff2e8 (run 3), 2dbdb6e (R4/R5). Never push/amend; `git commit -- <paths>`.
- LIVE NOW: `level-up-override` 0.2.1 (dll 693053d9), 26 ids, UpdatedAt 2026-10-02T00:58:35, stage `$T/levelup_stage3/`, backups
  `backups/pre_levelup_020_20261002_005342/` (pre-0.2.0 = 0.1.0 set) and `backups/pre_levelup_021_20261002_005828/` (0.2.0 set). Game dir
  `levelup_override.ini` = `mode=rewrite tags=atton apply=1 verbose=1` (the rewrite is ACTIVE for Atton auto level-ups). `$T` = `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`.
- Phase 2 status: run 3 proved the Auto-flow rewrite. Still open (user check, no code needed): manual level-up -> `SKIP manual`, sheet after save/reload, L8 feat 77,
  Kreia powers, Auto NPCs / XP award flows, SetExperience replay -> ALREADY. Log lines before run 3 = 46; run 3 = lines 47-105 (local copy
  `research/companion_levelup/logs/live/levelup_override_log_run3_20261002.txt`, untracked).
- Next phase (user-requested): static Ghidra research for R4/R5 items 1-4 (auto-vs-indicator branch in 0x74EE90 + "not under player control"; where the level-up
  indicator is drawn and what it reads; class-selectable test (Jedi); multi-level banking + XP cap). Plan file: see MASTER_PLAN status line.
- Phase 3 research plan: `~/.claude/plans/dazzling-brewing-quasar.md` (items 1-6 = R4/R5 + R6/R8/R9 scouting; static only).
- Tooling gotchas this session: (1) the host apply test cannot use `g++ -m32` (no 32-bit libstdc++) and wine is absent -> fakes live in an `mmap(MAP_32BIT)` arena
  and engine pointers are read as uint32_t; (2) mutation builds of a copied .cpp failed to resolve `../_shared/SafeRead.h` from the scratchpad (/tmp) - run such
  copies inside the repo tree (e.g. `tools/parity/_mut/`, delete afterwards); (3) `tools/kdb_add.py add_lesson` hard-codes date 2026-09-30 - insert with python
  sqlite3 and the real date instead; (4) the kpatch DLL is `binaries/windows_x86.dll` inside the zip; (5) check `-O2 -S -masm=intel` asm for thiscall call sites.

## Session notes - Phase 3 research + presets (2026-10-02 ~01:20 .. 03:00, so a context clear loses nothing)
- Commits (level-up only): 7956288 (R6-R9), 7e2f2ed (Step 5 E), f5d12b2/4e0060a (Step 1), 3384514 (Step 2), b214b62 (Step 3), 0705a7f (Step 4, HANG hazard),
  da09b30 (first 15 presets), f61e2b6 (Steps 6-7 synthesis), dc692a3 (user decisions + WIZARD_SCHEMA.md), 9c91ab2 (save binding G1 + Atton/Bao-Dur
  presets), 1336746 / ae4c46d (Handmaiden/Kreia/Mira/Visas/Disciple presets), 8b0ab90 (preset summary), 06bd7dc (check_paths accepts classes 6/7),
  07dea97 (non-Jedi presets), 31c5f8b (decision g). Sibling sessions commit on the same branch (Mod 3/14, shield bar, doc 10/17): never `git add -A`.
- Live state unchanged by this session: `level-up-override` 0.2.1 still installed (a sibling reinstall at 01:38:59 added k2-wrap-measure-fix; kpm
  UpdatedAt is now 2026-10-02T01:38:59, later sibling installs may move it again - re-md5 before any install). Game-dir `levelup_override.ini` =
  mode=rewrite tags=atton apply=1 verbose=1.
- Outputs: doc 08 "Phase 3 research" Items 1-7 + synthesis + "User decisions" (a-g); evidence `research/companion_levelup/phase3_evidence/` (A1/A2, B1, C1, D1,
  E, F1, G1 + README); 80 presets `research/companion_levelup/paths/presets/<tag>__*.toml` + `<char>.md` (12 companions, all check_paths 0 errors);
  `paths/presets/WIZARD_SCHEMA.md`; DB name_map rows source `level-up-P3-decompile-confirmed` + lessons auto-level-up-option, portrait-overlay-hides-buffs,
  autolevel-class-choice, autolevelup-veto-hang, levelup-gui-live-edit, save-bound-mod-data.
- Preset tags: atton, kreia, baodur, disciple, handmaiden, mira, VisasMarr, hk47, t3m4, g0t0, mand, hanharr (verify each against the live creature Tag
  in the 0.3.0 table generator; runtime lookup is case-insensitive). `gen_runtime_table.py` currently reads ONLY `paths/*.toml` (atton, kreia live paths),
  not `paths/presets/`.
- Known gaps for 0.3.0: (1) early-conversion re-base rule; (2) `hold_until_class` semantics (presets use it inconsistently: Handmaiden/Mira/Bao-Dur/Disciple
  set it, Atton converting presets set 5); (3) check_paths ignores "Class Skill: X" feats and attack-bonus prereqs; (4) mod #142 alignment source unverified;
  (5) StrategyWiki returns 503 intermittently (use archive.org or elkripper).
- Next: 0.3.0 design/build plan = `research/companion_levelup/PHASE4_PLAN.md` (repo copy of `~/.claude/plans/dazzling-brewing-quasar.md`, Phase 4).

## Session notes - Phase 4 (0.3.0 build, started 2026-10-02, so a context clear loses nothing)
- Plan: `research/companion_levelup/PHASE4_PLAN.md` (authoritative; the ~/.claude plan file is reused per phase). Evidence: `research/companion_levelup/phase4_evidence/`.
- Step 0 (preflight) done (746aaf3). M0 done: "Phase 4 M0 facts" above (+ hold rule v2 by class level), DB name_map source
  `level-up-P4-static-confirmed` + 2 lessons. Next: M1 (Python agents: gen_runtime_table v2 + `[presets]` + class-level field, check_paths Class Skill feats,
  tag verification, simulate_rewrite level_now twin). M1 DONE: "Phase 4 M1 results". Next: M2 C++ (0.3.0-a hold + instant, ini config).
- M2 BUILT + INSTALLED 10:32 (dll 9e8cbbff, see "M2 install"); next = user plays the M2 live checklist (PHASE4_PLAN M2), Opus reads the log from line 176.
- Session end 2026-10-02 (~10:45): commits 746aaf3 (Step 0), 3b684b2 (M0), 6126d99 (M1), 5f47741 (M2 build), c0851ad (M2 install), 8c6e0d5 (dev preset).
  LIVE: 0.3.0-a dll 9e8cbbff, 28 ids, stage `$T/levelup_stage4/`, backup `backups/pre_levelup_030a_20261002_103155/`; game-dir ini `mode=rewrite apply=1
  verbose=1 preset.atton=atton__melee_dual_light_early hold.atton=1 instant.atton=0`; table v2 copied to game dir. Log had 175 lines before any 0.3.0 run.
  **User has NOT tested 0.3.0-a** and wants ONE combined live test after M3 + M4 + R11 (held indicator) are built - so the next phase is offline build of
  all three (UX research first), then a single install + combined checklist. The M2 checklist items stay in the combined test.
- New requirement R11 (Status): visible "held" indicator. UX research for R6/R8/R9/R11 continues in the next phase (plan file named in the MASTER_PLAN status).
- Gotchas this session: (1) the `~/.claude/plans/dazzling-brewing-quasar.md` slug is reused per phase -> repo copy `research/companion_levelup/PHASE4_PLAN.md`
  (next phase: copy the new plan to `research/companion_levelup/PHASE5_PLAN.md`); (2) new untracked files must be `git add -- <path>` before
  `git commit -- <paths>`; (3) mingw syntax check needs `-I $T/Patches/LevelUpOverride` (the `../_shared/SafeRead.h` include resolves under $T/Patches);
  (4) mutation test = in-place mutate + restore from a scratch copy, run `parity.py` (10/10 killed); (5) `patch_manager_mods/level_up_override_src/PULL_REQUEST.md`
  is untracked and NOT ours (created 03:57 by another session) - leave it; (6) `atton__live` stays Scoundrel to char 19 (Sentinel from 20), so use a
  convert-at-8 preset to see a hold on a level-7 Atton; (7) review finding: all hooks run on one thread (tid 316 in logs) - config/table swap is safe.


## Session notes - Phase 5 (UX + M3 save binding + M4 Configure UI + R11 -> one 0.3.0 build, started 2026-10-02)
- Plan: `research/companion_levelup/PHASE5_PLAN.md` (authoritative). Evidence: `research/companion_levelup/phase5_evidence/` (README lists U1-U4 prefixes).
- Step 0 (preflight) done: plan copied, "Phase 5 UX research" + R11 decision written above. Next: U (Ghidra static, one Sonnet agent) || P (preset data fix).
- Done: wizard logic 9cd0fa5 (WizardLogic.h + twin, 264-line parity), GuiKit.h e11221f (49 host checks), P preset re-plan 61aa066 (warnings 275 -> 199), U facts (above).
- M3 save binding DONE 29e3763 (0.3.0-b): GUID = 32 Boolean globals LUO_G00..31; hooks pre-save 0x656640 (ecx), post-load 0x65675B ([ebp-0x1c], eax),
  new server 0x401C52 (skip, calls 0x51C450); `levelup_playthroughs.ini` (`[g<hex>]`, `<tag>=<preset>|instant=|hold=`; .tmp + MoveFileExA; lossy-parse
  refuses writes); tagConfig: GUID section wins outright, else dev keys; `ptApply` creates the GUID + live-writes it (read back) on first use, `ptRemove`
  keeps an empty section. Tests: gate_test 110, playthrough_test 244. Next: M4 LevelUpUi.h (Sonnet draft running) -> Opus review -> C gates.
- Live state unchanged: 0.3.0-a dll 9e8cbbff installed, untested by user; the combined test happens after the 0.3.0 build (step C).
