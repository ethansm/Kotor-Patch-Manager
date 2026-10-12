# R5 Prior art and risks: ambient NPCs in vanilla KOTOR II areas

Paths below are relative to `/home/ethansm/Games/KOTOR2_Mod_Workspace/01_extracted/` unless shown otherwise.

## 1. Local prior art (best 3)

1. **mod115 Harbinger Arrival (GIT edit, closest match)**
   - `mod115_harbinger_arrival/Harbinger_Arrival_Enhancement_for_TSLRCM_v1.1/Automatic Installation/tslpatchdata/changes.ini`
   - `[GFFList] File0=104per.git`; `[InstallList] install_folder0=modules\104per.mod`
   - `[gff_104per_Creature List_2_0]`: FieldType=Struct, Path=Creature List, TypeId=4, AddField for X/Y/Z position, X/Y orientation, TemplateResRef.
   - The UTC ships in the same tslpatchdata (`n_darthsion001.utc`). Read-only pykotor check: Tag=DarthSion, FactionID=0, Appearance_Type=458.
   - Mechanism: append a struct to the GIT Creature List plus a UTC. The GIT goes into the MOD archive, not `override/`.
2. **mod127 Repair HK-47 (script spawn)**
   - `mod127_repair_hk47/sourcescripts/a_spawn_hk.nss` (28 lines): `CreateObject(1, "p_hk048" | "n_hk51_001" | "n_hk50_002", Location(...), 0)`, gated on `GetGlobalNumber("299TEL_HK51_Core" | "299TEL_HK50" | "HK_REPAIR_STATE")`, then `ChangeObjectAppearance`.
   - Wiring: `tslpatchdata/changesTSLRCM.ini` `[a_spawn_hk.nss] !Destination=modules\902mal.mod`; `[CompileList] Replace1=a_spawn_hk.nss`.
   - Mechanism: area script compiled in, no GIT edit. Spawns use existing templates.
3. **mod148 Obscured Tomb (script spawn via include)**
   - `mod148_obscured_tomb/source/a_spawn_mand.nss` (11 lines): `FAB_Spawn("g_mand_first", 0..5)` plus `DelayCommand`.
   - `k_inc_fab` is not in the mod source, so it is assumed to be a vanilla include. Not verified.
   - Also seen: mod121 `changes_armor.ini` `[101per.git]` appends Placeable List structs (same struct-append pattern, placeables only). mod112 LJJT `[304nar.git]` edits existing Creature List positions only, adds nothing.

## 2. Community technique and limits

- Local: route 1 above is the only local creature-add via GIT. No local waypoint or walk recipe was found. The `Waypoint` grep hit five changes.ini files (mod113, mod98, mod12, mod115); I did not inspect them.
- Web (one standard WebSearch, snippets only, low confidence): GIT CreatureList is recommended for always-present NPCs. Module content is stored in the save on first entry. No TSLPatcher CreatureList syntax was found.
  - https://deadlystream.com/topic/6914-creature-spawning/
  - https://deadlystream.com/topic/9052-how-to-edit-maps-in-kotor-2-like-placing-npc/
- Limits: the GIT entry needs a resolvable UTC ResRef. Whether an Override `.git` beats `modules\*.mod` is unproven (doc 26 "in_party edits UNPROVEN"; `patch_manager_mods/08_level_up_override.md:975`).

## 3. Risks

- **Save state (highest):** standing NPCs appear in both SAVEGAME and the module `.git` Creature List, with the same data and a different ObjectId (`patch_manager_mods/26_FORCED_STATE_TESTING.md:28`). Whether a GIT edit reaches an already-visited area is unproven (M0 Q1). The PoC must use a save made before the visit.
- **Object count and performance:** no local measurement found. OPEN.
- **npc_diversity hook 2 (Wookiee rows):**
  - Rows read via `scripts/check_diversity_assets.py` `load_appearance` on `npc_diversity_src/override/appearance.2da`: 50 Alien_Wookie_Female_01 (N_WookieF), 51 Alien_Wookie_Male_01 (N_WookieM), 271-278 Alien_Wookie_Male/Female_02-05, 327 Lite_Wookie_Male, 427 Lite_Wookie_Female (N_WookieM model).
  - None appear in `DiversityTables.h` (explicit case labels only).
  - `FamilyOf` returns None, and `RemapAppearance` returns `currentApp` unchanged (`DiversityLogic.h` final return).
  - `ShouldDiversifyTexture` needs `IsCommonerTexture`, which matches only the `n_commf` / `n_commm` prefix (`DiversityLogic.h:164-174`). `N_WookieM` does not match.
  - So a Wookiee on one of these rows is untouched. A row from the generic lists (e.g. 14, 17) would be remapped. The doc 34 §7 item 7 "test range" problem is absent from the current table; not re-verified at runtime.
- **Faction and hostility:** no local evidence on FactionID semantics. Precedent UTC uses 0. Meaning unverified.
- **Plot flags:** precedent UTC has a Plot field (value not read). Avoid plot-global gating in the PoC.
- **KPM rule:** `tools/kotor-patch-manager/Patches/create-patch.py` lines 852-870 package only `manifest.toml`, `*hooks.toml` and `binaries/*`. `override/` and `modules/` never enter the kpatch (BUILD_AND_INSTALL.md line 54). Wookiee assets therefore need a separate TSLPatcher or manual install, outside KPM rollback.
- **Textures:** `N_WookieM06.tpc` exists in `nexus_mod144_uco_redux` (variant texture, not the vanilla model). Live game not inspected (rule).

## 4. Recommended PoC (cheapest, evidence-backed)

Route 1 first: GIT Creature List append, following mod115.
- Minimum artifacts:
  1. `tslpatchdata/changes.ini` with `[GFFList] File0=<area>.git`, `[InstallList] install_folder0=modules\<area>.mod`, and a Struct AddField block (TemplateResRef, X, Y, Z, orientation).
  2. One Wookiee UTC in `override/` with a unique ResRef, cloned from a vanilla Alien_Wookie row (row 51 or 50), FactionID copied from a vanilla commoner.
  3. A never-visited Citadel module and a new save made before entering it.
- Walk of 3 waypoints: OPEN. Phase 2 uses a compiled NCS with three `ActionMoveToLocation` calls (nwnnsscomp, per repo note). Route 2 (script spawn) is the fallback if route 1 does not spawn. No DLL route is needed.

Sources: deadlystream URLs listed in section 2.
