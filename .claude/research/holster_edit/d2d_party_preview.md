# D2D recon: party-member 3D preview for holster-editor (Haiku, 2026-10-16)

Read-only recon. `EQ` = patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp. Decomp = ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<xx>/<addr>.c (xx = addr bytes 2-3).

## 1. Which creature, and how to enumerate party members

- equip3d `real` = `panel+0x68` (`PanelCharOff`): EQ:54, read at EQ:934. This is the client CSWCCreature* that the equip panel was given (the shown character).
- `realValid` = vtable at +0 equals `ClientCreatureVtable` 0x0099EE14 (GameAddr.h:103): EQ:555-559.
- Outside the equip screen, enumerate via `_shared_src/Party.h`:
  - `party::table()` Party.h:33-41: app (0xA1B4A4) -> +4 -> +4 -> +0x270.
  - `party::partyCount(t)` Party.h:44 (dword at t+0).
  - `party::member(t, i)` Party.h:80: PartyGetAt 0x7E5DA0, thiscall(table, idx). Index 0 = controlled. Returns a client CSWCCreature*, the same type as `real`.
  - `party::controlledCreature()` Party.h:51: GetControlledCreature 0x7E5D80.
  - `party::serverCreature(member)` Party.h:89: MemberServerCreature (CSWSCreature*).
- Existing users of the party chain: `active_effects_panel_src/CharEffectsPanel.cpp:346` (`leaderClient()` = PartyGetAt(table,0), "what SetStats shows"); `portrait_slots_src/common_contract_test.cpp:91,239`; `companion_ai_src/CompanionAI.cpp:72-73` (own copies of 0x73FB90 / 0x7E5DA0); `level_up_override_src/LevelUpOverride.cpp:1059`. queue_hud and companion_selfheal were not checked.
- Display name: no verified client-side first-name accessor.
  - Server: `CSWSCreature::GetFirstName` Steam 0x5632F0 (R9_map_areamod.tsv:361, vote, retn=0, GOG 0x67BF20). Doc 38 (line 167) says it reads stats+0x34.
  - Conflicting vote: `CSWSObject::GetFirstName` Steam 0x4598B0 -> GOG 0x6638C0 (U3_map_CSWSObject.tsv:5). Neither mapping is REA-confirmed.
  - Suggested path: `serverCreature(member)` then 0x5632F0 once REA-verified. Until then the picker can show a fallback label.
- Alignment for idle: client stats = creature+0x310 (EQ:59), short at stats+0x80 (EQ:565).

## 2. Inputs buildPreview reads from `real` (EQ:706-751)

| Input | Read at | Valid for a non-selected party member? |
|---|---|---|
| appearance ptr creature+0x224, 0x3C-byte copy | readApp EQ:560-567 | Yes: per-creature data. Failure -> build returns false (EQ:716). |
| stats+0x80 alignment (neutral 50 fallback) | EQ:565 | Yes. |
| appearance copy +0x10 = head item id | EQ:721-722, helmet EQ:748 | Yes. Head id is blanked before the engine build (EQ:722); helmet is re-attached from our own Gob. |
| right/left item ids creature+0x238 / +0x23C | EQ:643, EQ:676 | Yes per creature, but see OPEN (engine hand handles are +0x278/+0x27C: K2Addr.h:385-386). |
| vtable check (realValid) | EQ:557 | Yes. |
| `FnSetupScene(pv, block, real, 0, 1)` 0x77EC20 | EQ:739 | UNVERIFIED: the engine reads `real` internally. Confirm it does not depend on equip-panel state. |
| camera-hook tag | none | No camera-hook tag choice exists in EQ (grep: no `camerahook`). Fixed names only: "rhand"/"lhand" (EQ:644), "GoggleHook" (EQ:518), part ids 0xff body / 0xfe head. |

Panel-specific values that Preview3d.h must take as parameters or viewer fields:
- `g_weaponsOn` (WEAP toggle): EQ:674, EQ:746. Becomes `Viewer.weaponsOn` or a param.
- `g_cfg.helmet` (EQ:748), `g_cfg.fov` (EQ:689): config struct `Cfg`.
- Drag/zoom cfg `dragDegPx, panPerPx, panMin, panMax, wheelStep, zoomMin, zoomMax` (EQ:813-820) and `hitFlipY, screenH` (EQ:800): `Cfg`.
- `overAnyButton` (EQ:323) and `GetForegroundWindow()==g_hwnd` (EQ:803): host supplies `blocked` and `focused`.
- Menu (`menuReal`, EQ:406, EQ:435), flourish, spin (EQ:983): equip-only, stay in host.

## 3. Proposed Preview3d.h API

```cpp
namespace preview3d {
struct Cfg { float fov; bool helmet; float dragDegPx, panPerPx, panMin, panMax, wheelStep, zoomMin, zoomMax;
             unsigned site; /* saferead site id, per DLL (EQ:110 SiteEquip) */ void (*log)(const char*, ...); };
struct Viewer {
  unsigned char* block; unsigned char* creature; bool creatureBad; int real; bool built; int builds; int baseModels;
  unsigned long lastBuild; unsigned char app[AppCopyBytes]; short align;
  void* wpn[2]; char wpnNames[2][28]; int wpnIds[2]; bool wpnApplied, stanceOn, weaponsOn;
  char stanceName[12], idleName[12], baseIdle[12];
  void* helm; char helmName[17]; int helmId; bool helmApplied;
  float yaw, zoom, panZ, appliedPanZ;
  bool dragging, wasL, wasM; float lastX, lastY, startX, startY; int axis;
};
bool build(Viewer&, const Cfg&, int real, const char* why);        // = buildPreview EQ:706-751
void attachWeapons(Viewer&, const Cfg&, int real, const char* why); // EQ:637-667
void refreshWeapons(Viewer&, const Cfg&, int real, const char* why);// EQ:669-686
void detachWeapons(Viewer&, const char* why);                        // EQ:465-478
void destroy(Viewer&, const char* why);                              // EQ:524-535 (+ detachHelmet EQ:483-494)
void applyView(Viewer&, const Cfg&);                                 // EQ:687-705
bool tickDrag(Viewer&, const Cfg&, bool blocked, bool focused, bool lbtn, bool mbtn, float cx, float cy, bool inside, long wheel); // EQ:795-826
}
```

Line ranges each function replaces (equip3d keeps the rest):
- Moved as-is: `FnNew/FnGetItemObj/FnCreateModel` consts EQ:38-43; `CrRightItemOff/CrLeftItemOff/ItemModelResOff` EQ:41-42; creature consts EQ:54-66; typedefs EQ:68-77; `AttachFn` EQ:464.
- Helpers: `getItemObj` EQ:479-482, `allocCreature` EQ:537-553, `realValid` EQ:555-559, `readApp` EQ:560-567, `partGob` EQ:569-574, `playPart` EQ:576-580, `chooseIdle` EQ:628-634.
- Helmet (optional, `cfg.helmet`): `attachHelmet` EQ:498-523, `detachHelmet` EQ:483-494.
- Weapons: `detachWeapons` EQ:465-478, `attachWeapons` EQ:637-667, `refreshWeapons` EQ:669-686.
- Build: `build` EQ:706-751. `destroy` = `destroyCreature` EQ:524-535. Camera: `applyView` EQ:687-705.
- Input: `tickDrag` = `tickInput` EQ:795-826, minus the cursor/rect fetch and `overAnyButton`, which stay in the host.

Equip keeps: `BindEquipView`/`EquipTick`/`EquipDestroy` EQ:845-999 (spin at EQ:983); toolbar, menu and tip EQ:286-459 (`menuBuild` EQ:610-627); flourish EQ:753-794 with RNG EQ:753-755; cover labels EQ:276-285; `Slot` shell (panel, usable, seq, ticks, flourish fields).

Globals and statics that must become Viewer fields or Cfg / params:
- `g_weaponsOn` (EQ:674, 746) -> `Viewer.weaponsOn`.
- `g_cfg.fov` (EQ:689), `g_cfg.helmet` (EQ:748), drag/zoom/hit cfg (EQ:800, 813-820) -> `Cfg`.
- `static bool wasL, wasM` in tickInput (EQ:796) -> `Viewer.wasL/wasM`.
- `logf`/`dlogf`/`logw` and `g_logLines`/`g_dbgLines` -> `Cfg.log` callback.
- `SiteEquip` (EQ:110) -> `Cfg.site`.
- `s->creature`, `s->real`, `s->block`, `s->yaw`, `s->zoom`, `s->panZ`, `s->app`, `s->align`, `s->wpn*`, `s->helm*`, `s->stanceName`, `s->idleName`, `s->baseIdle`, `s->drag*` -> the matching Viewer fields.

## 4. Weapon model name

- equip3d: `eq3d::weaponModelName(base, var)` = `"<cls>_%03d"` from the generated table: Equip3dLogic.h:172-177; `EquipWeaponTable.inc` line 1 (generated from baseitems.2da by scripts/gen_equip3d_weapon_table.py). `base` = item+0x0C (EQ:41, EQ:648), `var` = item+0x11C (EQ:41, EQ:648). The letter byte at item+0x11D is ignored.
- Engine item ctor `00801fa0` (decomp/80/00801fa0.c:66, 102-105): writes variation to item+0x11C and letter to item+0x11D, then calls `006d5000` (decomp/6d/006d5000.c). `006d5000` = `sprintf("%s_%03d", row+0x3C, var)`, or `"%s_%c_%03d"` when row+0x74 != 0 and letter != 0. The same ctor has a default-model fallback ("Model %s nor the default model %s could be loaded", decomp 00801fa0.c ~:113-122).
- Live in-world: holster C5 (`patch_manager_mods/ghidra_knowledge/rea_findings/20261016_holster-c5-weapon-identity.tsv`) and Holster.cpp:540-545 confirm `[Gob+0x84]+0x08` = "w_Lngswrd_001", "w_SonicPstl_002". The Gob is made by `FUN_00462320` (= FnCreateModel) through resref lookup `0x48E160`.
- In-world attach `007637E0` (decomp/76/007637e0.c:47 param_3 = creature; :119 "rhand"; :121-122 vt+0x7C on the holder from `FUN_007ED830` = [ECX+0x68], i.e. EQ:60 CreatureHolderOff; WEAPONSCALE lookup :69-71). Its name arg `local_60` is never assigned in this dump, so the in-world name source is UNRESOLVED.
- `0086A430` is NOT the weapon-model path: it creates "dummyright"/"dummyleft" holder Gobs (decomp/86/0086a430.c ~:52-56, :84-88), called from `008684E0`.
- Engine accessor: none verified for Steam. `GetModelResRef` exists in GOG (0x4EADA0, name_map) and Linux (CSWBaseItem__GetModelResRef), but has no Steam mapping in the repo.
- Recommendation: read the engine-chosen ResRef from item+0x128 (16 bytes; EQ:42, EQ:504-505 already does this for helmets). Fall back to `weaponModelName` only when empty or `i_null`. This gives the name the item ctor chose, including the default-model fallback.
- Match: formula == item+0x128 when the letter byte is 0 and no fallback fired. Both match `w_Lngswrd_001`-style names in the normal case.

OPEN
1. In-world weapon ResRef passed to vt+0x7C (007637e0 `local_60`): REA on the call site or a live log comparing `[Gob+0x84]+0x08` with item+0x128 for one item.
2. Equality of creature+0x238/+0x23C (equip3d) and +0x278/+0x27C (engine hand handles, K2Addr.h:385-386). Check live.
3. C5 TSV calls creature+0x224+0x18 a "baseitem key", but equip3d treats it as the appearance row (EQ:587). WEAPONSCALE is an appearance.2da column, so the appearance-row reading is likelier. Confirm.
4. Client-side first-name accessor for the picker (REA on 0x5632F0 and 0x4598B0).
5. `FnSetupScene` (0x77EC20) side effects on `real` (EQ:739).
