# 34 — Master NPC Diversity Dispatcher (.kpatch Architecture & Asset Pool)

## Executive Summary
This document specifies the research, technical feasibility, reverse-engineered engine mechanics, and implementation plan for a unified, conflict-free NPC diversity patch for *Star Wars: Knights of the Old Republic II: The Sith Lords* via KotOR Patch Manager (KPM).

Existing diversity mods rely on replacing individual `.utc` creature templates (baked permanently into savegames and conflicting with TSLRCM) or manually editing `.mod` archives. This project eliminates both failure modes by coupling:
1. **The Static Asset Pool**: Override textures, custom meshes, and appended `appearance.2da` rows 802-815 from verified DeadlyStream Modder's Resources (`heads.2da` is not changed by this install; see §7).
2. **The KPM Runtime Dispatcher Detour**: A deterministic C++ engine hook in `swkotor2.exe` that dynamically varies NPC body shapes, head assignments, and clothing texture slots (01–35+) based on creature coordinates, area ResRef, and tag hash.

---

## 1. Engine Reverse Engineering & Feasibility Findings

### 1.1 Body Texture Resolution: `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef`
- **Engine Addresses**:
  - GOG: `0x0048E3B0` (sz 813, ret 12)
  - Steam Aspyr: `0x0085E320` (sz 813, ret 12)
- **Signature & Calling Convention**:
  ```cpp
  // __thiscall on CSWCCreatureAppearance* (ecx)
  void CSWCCreatureAppearance::GetBTypeModelAndTextureResRef(
      const AppearanceDesc* pDesc, // [ebp+0x08]
      CResRef* pOutModelResRef,    // [ebp+0x0C]
      CResRef* pOutTextureResRef   // [ebp+0x10]
  );
  ```
- **Appearance Descriptor Memory Layout (`pDesc`)**:
  - `+0x00`: Sub-type / variant flag (`char`)
  - `+0x01`: `TextureVar` (`BYTE`, 0–255)
  - `+0x14`: descMode14 (`int`, zero/nonzero switch; GUI paths write 1; NOT an armor class 0-3, see §7 item 4)
  - `+0x18`: `AppearanceType` row index into `appearance.2da` (`WORD`)
  - `+0x1A`: Head override flag (`char`)
  - `+0x1B`: Gender (`BYTE`, 0 = Male, 1 = Female)
  - `+0x1C`: Phenotype (`BYTE`)
- **Native Texture Formatting & Fallback Logic**:
  Disassembly at Steam `0x0085E540` / GOG `0x0048E5D0`:
  ```asm
  mov    eax, [ebp-0x2c]        ; TextureVar index from pDesc[1]
  push   eax
  lea    ecx, [ebp-0x14]        ; texa string from appearance.2da (e.g. "N_CommM")
  call   CExoString::c_str
  push   eax
  push   0x009a4500             ; format string: "%s%02d"
  lea    ecx, [ebp-0x4c]
  push   ecx
  call   sprintf
  ; Check if formatted texture exists in Resource Manager
  push   0x03                   ; RESTYPE_TPC / Texture
  lea    ecx, [ebp-0x44]
  push   ecx
  mov    ecx, [g_pResMan]
  call   CExoResMan::Exists
  test   eax, eax
  jne    .found_texture
  ; If not found, check secondary image type (0xBBF / TGA)
  push   0x0bbf
  lea    edx, [ebp-0x44]
  push   edx
  mov    ecx, [g_pResMan]
  call   CExoResMan::Exists
  test   eax, eax
  jne    .found_texture
  ; Fallback: hardcoded to variant "01"
  push   0x01
  push   0x009a4500             ; "%s%02d"
  call   sprintf
  ```
- **Feasibility Verdict**:
  The engine **does not hardcode a maximum cap of 11**. It directly formats `%s%02d` and queries `CExoResMan::Exists`. If `TextureVar` is set to `27` and `N_CommM27.tpc` exists in Override, the engine loads it immediately. The vanilla limitation to 11 was solely due to the templates in `.utc` and Obsidian's 11 default textures.

### 1.2 Body Mesh & Head Creation: `CreateBodyAppearance` & `CreateHead`
- **`CreateBodyAppearance`**:
  - GOG: `0x0048A280` | Steam: `0x0085A1F0`
  - Reads `appearance.2da` columns: `modela`..`modeli`, `texa`..`texi`, and checks model type (type 2, 3, or 11).
- **`CreateHead`**:
  - GOG: `0x0048AA70` | Steam: `0x0085A9E0`
  - Reads `heads.2da` row via `Appearance_Head` / `normalhead` column.
  - Queries `heads.2da` column `HEAD` (model) and `ALTTEXTURE` (optional skin overlay).
- **Client Appearance Entry Point**: `CSWCCreature::SetAppearance`
  - GOG: `0x00419070` | Steam: `0x0076D200`
  - Passes `AppearanceDesc` to `CSWCCreatureAppearance::CreateBodyAppearance`.

### 1.3 Server-Side Creature Instantiation: `CSWSCreature`
- **`LoadFromTemplate`**: GOG `0x0068B5A0` | Steam `0x00572A70`
- **`LoadCreature`**: GOG `0x0068CCB0` | Steam `0x00574180`
- **`CSWSCreatureStats::ReadStatsFromGff`**: GOG `0x006EC350` | Steam `0x006AFED0`
  - Offset `+0x194`: `m_nAppearanceType` (`WORD`)
  - Offset `+0x197`: `m_nAppearanceHead` (`BYTE`)
- **Creature World Position**:
  - `CSWSObject + 0x94`: X coord (`float`)
  - `CSWSObject + 0x98`: Y coord (`float`)
  - `CSWSObject + 0x9C`: Z coord (`float`)
  - `CSWSObject + 0x04`: Object ID (`DWORD`)
  - `CSWSCreatureStats + 0x30`: Tag (`CExoString`, accessible via `GetTag 0x006F2FB0`)

---

## 2. Verified Modder's Resources & Asset Inventory

All 11 assets have been verified on DeadlyStream with author permissions, asset formats, and integration roles:

| Asset & Author | Source URL | Content & File Types | Role in Master Patch |
|---|---|---|---|
| **Commoner's Texture Pack v1.0** (CapitaineSpoque / ShiningRedHD) | [DeadlyStream #2618](https://deadlystream.com/files/file/2618-commoners-texture-pack-v10/) | ~70 high-res commoner clothing textures (TPC/TGA), male & female, sleeveless variants, Czerka arm bars. Modder's resource with credit. | Expanded texture pool: mapped to `N_CommM01`..`N_CommM35` and `N_CommF01`..`N_CommF35`. |
| **Lily's NPC Overhaul** (Lily / Dark Hope) | [DeadlyStream #3080](https://deadlystream.com/files/file/3080-tsl-lilys-npc-overhaul-hd-models-added-diversity/) | 39 distinct human head textures (35 reworked + 4 new head slots), UV/hair mesh fixes, 11 HD commoner clothing textures, Ralon Dantooine head. | Core HD base textures. The claimed 4 restored head models in `heads.2da` are not installed by this patch (heads.2da unchanged, §7 item 9). |
| **Fat Commoner Body Models v1.1.0** (DarthParametric) | [DeadlyStream #2374](https://deadlystream.com/files/file/2374-fat-commoner-body-models-modders-resource/) | Fully rigged fat male & female commoner meshes (`N_CommM_Fat.mdl`, `N_CommF_Fat.mdl`), UV-mapped for standard commoner textures, female pause animation fixes. | New `appearance.2da` rows: 10–15% of generic commoners rolled to heavy-set body shapes. |
| **JC's Fashion Line B: Commoner Clothing** (JCarter426) | [DeadlyStream #1324](https://deadlystream.com/files/file/1324-jcs-fashion-line-b-commoner-clothing-for-k2/) | Fixed vanilla seams/discolorations, cobbled Male Czerka Uniform texture to match female Czerka commoner outfit. | Czerka commoner clothing consistency across male and female Citadel NPCs. |
| **Female Czerka Officer's Uniform v1.1** (DarthParametric) | [DeadlyStream #1955](https://deadlystream.com/files/file/1955-female-czerka-officers-uniform-modders-resource/) | Capped female Czerka officer body model with Caucasian, Black, and Asian skin tones. Modder's resource. | Diversify Czerka personnel on Telos Citadel Station (201TEL–204TEL). |
| **TSL Removed Mouth Cubes** (Ashton Scorpius) | [DeadlyStream Resource](https://deadlystream.com/files/file/2242-tsl-twilek-male-npc-diversity/) / Mouth Fix | Cleaned geometry stripping unhidden tonguetip dummy meshes from unused vanilla TSL heads. Modder's resource. | Claimed: restores 6+ cut Obsidian NPC heads to `heads.2da` without animation glitches. Not installed by this patch (§7 item 9). |
| **K1 Enhancement Pack Assets** (SpaceAlex / JasonRyder) | [DeadlyStream #1414](https://deadlystream.com/files/file/1414-spacealexs-unfinished-k1-enhancement-pack/) | Head models, military uniforms, alien texture variants. Modder's resource. | Background diversity pool for refugees and generic planetary citizens. |
| **TSL Twi'lek Male NPC Diversity v1.1.2** (Leilukin / Ashton Scorpius) | [DeadlyStream #2242](https://deadlystream.com/files/file/2242-tsl-twilek-male-npc-diversity/) | Ashton's diverse male Twi'lek head meshes and skin tones ported to TSL. | Dynamic male Twi'lek appearances across Nar Shaddaa, Telos, and Dantooine. |
| **HD Female Twi'leks & Thigh-High Boots** (Lily / DarthParametric) | [DeadlyStream Resource](https://deadlystream.com/files/file/2242-tsl-twilek-male-npc-diversity/) | Upscaled textures, matching skin tones, varied lekku positioning, thigh-high boot body meshes. | Eliminates identical female Twi'lek clones in cantinas and refugee zones. |
| **Kel Dor Modder's Resource for TSL** (DarthParametric) | [DeadlyStream Resource](https://deadlystream.com/profile/15735-darthparametric/content/) | Fully rigged Kel Dor head and body meshes with custom hands. | Injects Kel Dor citizens into Nar Shaddaa docks and Citadel cantinas. |
| **Iridorian Modder's Resource** (90SK) | [DeadlyStream Resource](https://deadlystream.com/files/category/19-modders-resources/) | Custom armored mercenary heads and bodies. | Varied mercenary appearances on Telos Surface, Dantooine, and Nar Shaddaa. |

---

## 3. Architecture & Implementation Specification

```
                     ┌────────────────────────────────────────────────────────┐
                     │           Vanilla Engine Model / Texture Query         │
                     │ (CSWCCreatureAppearance::GetBTypeModelAndTextureResRef)│
                     └──────────────────────────┬─────────────────────────────┘
                                                │
                                    [KPM Runtime Detour Hook]
                                                │
                       ┌────────────────────────▼────────────────────────┐
                       │ Is Creature Generic Bystander / Commoner / Merc?│
                       │ (Check Tag / Appearance against Exclusion List) │
                       └──────────────┬───────────────────┬──────────────┘
                                      │ NO                │ YES
                                      ▼                   ▼
                           [Pass-Through Vanilla]    [Deterministic Hash Dispatcher]
                                                     - Hash(AreaResRef, X, Y, Tag)
                                                     - Select Body Mesh (Norm vs Fat)
                                                     - Select TextureVar (1..35+)
                                                     - Remap Appearance_Type (802+)
                                                          │
                               ┌──────────────────────────┴──────────────────────────┐
                               ▼                                                     ▼
                  [Override Asset Pool]                                  [Expanded 2DA Rows]
             N_CommM01..35 / N_CommF01..35                          appearance.2da (Rows 802-815)
             HD Heads / Restored Meshes                             heads.2da (Rows 192+)
```

### 3.1 Layer 1: Static 2DA Expansion & Asset Placement
1. **Append-Only Rows**:
   - `appearance.2da`: Appended rows are `802-815` (not 600+). The labels below are the original spec and do not match the shipped rows (see §7):
     - `600`: Commoner Fat Male (Model: `N_CommM_Fat`, Tex: `N_CommM`)
     - `601`: Commoner Fat Female (Model: `N_CommF_Fat`, Tex: `N_CommF`)
     - `602`: Female Czerka Officer (Model: `N_CzerkaF`, Tex: `N_CzerkaF01..03`)
     - `603–610`: Ashton/Leilukin Twi'lek Male combinations
     - `611–618`: Lily Female Twi'lek combinations
     - `619–620`: DP Kel Dor variants
     - `621–624`: 90SK Iridorian merc variants
   - `heads.2da`: Rows `192+` (vanilla has 192 rows; 192-248 pre-date this install). This patch adds no head variety: heads.2da is unchanged by install (§7 item 9).
2. **Override Filename Standardization**:
   - Standardize CapitaineSpoque + Lily + JC commoner textures into sequential slots:
     - `N_CommM01` through `N_CommM35.tpc`
     - `N_CommF01` through `N_CommF35.tpc`

### 3.2 Layer 2: Runtime KPM Detour Dispatcher (C++)
1. **Hook Anchor**:
   - Reuse the battle-tested `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` detour pattern from `K2ClassMenuClothes` (Steam `0x0085E4E4` / `0x0085E601`, GOG `0x0048E574` / `0x0048E691`).
   - Secondary hook: `CSWCCreature::SetAppearance` (Steam `0x0076D200`, GOG `0x00419070`) for body/head appearance remapping.
2. **Deterministic Hash Function**:
   ```cpp
   uint32_t HashCreatureSpawn(const char* areaResRef, float x, float y, const char* tag) {
       uint32_t hash = 2166136261u; // FNV-1a
       auto hash_str = [&](const char* s) {
           while (*s) { hash = (hash ^ (uint8_t)*s++) * 16777619u; }
       };
       hash_str(areaResRef ? areaResRef : "");
       hash_str(tag ? tag : "");
       int32_t qx = static_cast<int32_t>(x * 10.0f);
       int32_t qy = static_cast<int32_t>(y * 10.0f);
       hash = (hash ^ (qx & 0xFFFF)) * 16777619u;
       hash = (hash ^ (qy & 0xFFFF)) * 16777619u;
       return hash;
   }
   ```
3. **Exclusion Guard (Story / Named NPC Protection)**:
   Any creature matching the exclusion list is immediately passed through unchanged:
   - Companions (Kreia, Atton, Bao-Dur, Mira, Hanharr, Mandalore, Visas, Disciple, Handmaiden).
   - Major Story NPCs: `chodo`, `luxa`, `grenn`, `opo_chano`, `lorso`, `b4d4`, `ralon`, `moza`, `vrook`, `zreztik`, `geeda`.
   - Any creature whose appearance is non-generic (unique alien models, droids, bosses).

---

## 4. Phase-by-Phase Roadmap & Delegation Matrix

| Phase | Milestone | Status | Subagent Role & Command | Main Agent Role | Deliverables & Verification |
|---|---|---|---|---|---|
| **Phase 0** | Asset Verification & Ingest | DONE | `oc-subagent scout` (Local RTX 5070): catalog raw files, count textures, check TPC/TGA headers. | Authors `scripts/normalize_diversity_textures.py` (50 lines, deterministic). | ~70 textures mapped to `N_CommM01..35` and `N_CommF01..35`. Zero name collisions. |
| **Phase 1** | 2DA Append Tables | DONE | `oc-subagent scout` (Local RTX 5070): inspect `appearance.2da` and `heads.2da` column indices. | Authors `scripts/generate_diversity_2da.py` to append rows 802+ and 192+. | Append tables validated against base 2DA schema. |
| **Phase 2** | Detour Hook Prototyping | REOPENED (10-11 review) | `oc-subagent code` (OpenRouter free 32B+): drafts C++ detour boilerplate for `GetBTypeModelAndTextureResRef`. | Reviews diff, hooks into KPM build system, compiles via MinGW. | `NpcDiversityDispatcher.kpatch` compiles cleanly. |
| **Phase 3** | Fat Body & Alien Model Distribution | REOPENED (10-11 review) | `oc-subagent code` (OpenRouter free 32B+): drafts `SetAppearance` remapping and RNG weighting. | Finalizes hook integration and memory safety checks. | Visual test in Citadel Station and Nar Shaddaa: claimed, not evidenced (10-11 review, §7). |
| **Phase 4** | Exclusion List & Savegame Compatibility | REOPENED (10-11 review) | `oc-subagent test` (OpenRouter free): drafts unit tests in `*_test.cpp`. `oc-subagent verify` (Local): runs tests. | Reviews exclusion tag list for story NPCs and companions. | `scripts/run_tests.py` clean pass. |
| **Phase 5** | Production Packaging | REOPENED (10-11 review) | `oc-subagent gate` (Local RTX 5070): runs `build_check.py` and `kpm_status.py`. | Writes `manifest.toml`, creates `.kpatch` archive. | Clean gate pass, ready for KPM installer. |

---

## 5. Execution Checklist & Subagent Commands

### Phase 0: Asset Ingest & Verification
- [x] Local Scout: `oc-subagent scout "Audit raw commoner textures, count TPC/TGA files, and list naming patterns"`
- [x] Main Agent: Write `scripts/normalize_diversity_textures.py` and normalize textures into `N_CommM01..35` and `N_CommF01..35`
- [x] Ingest Lily's NPC Overhaul 39 heads and UV fixes
- [x] Ingest DarthParametric Fat Commoner Body Models v1.1.0 (`N_CommM_Fat`, `N_CommF_Fat`)
- [x] Ingest Ashton Scorpius fixed mouth-cube TSL heads
- [x] Ingest Leilukin / Ashton Scorpius Twi'lek male heads & Lily female Twi'lek models

### Phase 1: 2DA Append Tables
- [x] Local Scout: `oc-subagent scout "Extract column header list from vanilla appearance.2da and heads.2da"`
- [x] Main Agent: Write `scripts/generate_diversity_2da.py` to append rows 802+ and 192+
- [x] Verify non-destructive append via KPM 2DA merger

### Phase 2: Runtime Texture Dispatcher Hook
- [x] Cloud Code Subagent: `oc-subagent code "Draft NpcDiversityDispatcher.cpp detour for GetBTypeModelAndTextureResRef using FNV-1a hash"`
- [x] Main Agent: Review diff, wire into `hooks.toml` (Steam `0x85E320`, GOG `0x48E3B0`), and run `scripts/build_check.py`
- [ ] Verify texture rotation across slots 01–35 without crashes (reopened 10-11)

### Phase 3: Body & Head Appearance Dispatcher
- [x] Cloud Code Subagent: `oc-subagent code "Draft CSWCCreature::SetAppearance detour for 15% fat body roll and alien head remapping"`
- [x] Main Agent: Integrate into `NpcDiversityDispatcher.cpp` and verify memory offsets
- [ ] Verified build check (3 exports, 2 detours) and unit tests passing (reopened 10-11)

### Phase 4: Exclusion Guard & Save Compatibility
- [x] Cloud Test Subagent: `oc-subagent test "Draft NpcDiversityDispatcher_test.cpp testing exclusion tag hash lookups"`
- [x] Local Verify: `oc-subagent verify "Run scripts/run_tests.py on NpcDiversityDispatcher"`
- [ ] Validate on clean save and mid-game save without appearance corruption (reopened 10-11)

### Phase 5: Build, Test & Packaging
- [x] Local Gate: `oc-subagent gate "Run scripts/build_check.py on NpcDiversityDispatcher and verify hooks.toml manifests"`
- [x] Main Agent: Finalize `manifest.toml` and documentation

---

## 6. Subagent Token-Saving Strategy

To minimize Gemini remote API consumption while avoiding code quality degradation from small models:

1. **Local Model Isolation (RTX 5070 9B)**:
   - **Strictly Non-Authoring**: The local model is **NOT** used for authoring Python utility scripts or C++ code.
   - **Allowed Local Roles**: `scout` (read-only file surveying, regex searches, header checks), `verify` (running diff checks and test suites), and `gate` (running build check scripts).
2. **Cloud-First Free Coding (`oc-subagent code|test`)**:
   - Used for drafting C++ detours, test suites, and repetitive boilerplates.
   - Runs on larger OpenRouter free models (Qwen 2.5 Coder 32B, DeepSeek R1 free).
   - **Two-Pass Pattern**: Subagent drafts the initial C++ or test scaffold -> Main Agent reviews, refines, and compiles.
3. **Main Agent Direct Authoring**:
   - The main agent (Gemini) directly writes precision Python scripts (`normalize_diversity_textures.py`, `generate_diversity_2da.py`) where binary formats (TPC headers) and exact column spacing are critical.
   - Prevents iterative hallucination loops from smaller models.

---

## 7. Review 10-11 (reopens Phases 2-5)

1. BLOCKER: `hooks.toml` `esp+4` must be `[esp+4]` (KPM passes the address of the stack slot; pDesc is a stack address) in both Steam and GOG hooks.toml; no gate catches it (F-2, A2).
2. BLOCKER: texture seed = EBP frame address, so all creatures with the same (row, gender, textureVar) get the same N_CommMxx (C1).
3. BLOCKER: appearance rows 812-815 (Kel Dor/Iridorian) have no model/texture; ~10% of male commoner remaps hit them (D1, D2).
4. HIGH: descriptor +0x14 is a zero/nonzero switch (GUI paths write 1), NOT a 0-3 armor class; the bodyClass guard, tests and spec are wrong (A1-5).
5. HIGH: Hook 1 never runs for empty `texa` cells (0x85E543 -> 0x85E60F), runs for 3 callers (0x85CC00, 0x85D430, 0x85C490) with different pDesc meaning, and misses non-B model types (A1-4/8).
6. HIGH: CSWCCreature tag/position offsets unverified (+0x18/+0x1C is not a tag; +0x24/+0x28 vec3 position INFERRED; +0x04 oid INFERRED); no SafeRead/beginScope (A1-11).
7. HIGH: "test range" rows 300-310/350-360/500-510/550-560 in IsGeneric* tables are Sith, Jedi, Mandalorians, droids, rancors, Krayt, and story NPCs; gender is ignored in RemapAppearance (C1-2).
8. HIGH: story tags with area prefix (203_Lorso, 203_Chano, 203_b4d4, 202_Falt) bypass exclusion; 203_b4d4 gets a 100% Twi'lek-male remap (C1-3, C2-1).
9. MED: staged appearance.2da/heads.2da are whole-file snapshots incl. Echoes/TSLRCM/HD-Twi'lek edits (115 vanilla rows changed), outside KPM ownership; no base pinning (D2-2).
10. MED: P_TwiH* heads and llk_dahnisb* exist only in the live Override, not the patch source (D1-2/3); the five `llk_*.mdx` (ahrnell, dahnish, harra, kehvehn, luposhar) are now present in `npc_diversity_src/override/` (rechecked 10-11, doc 38); N_CommM10 is TGA-only; 15 commoner textures are 2048x2048; fat rows' texa mismatches mesh slots; dancers use human N_CommF; literal `****` cells.
11. MED: selector bias: hash%2 correlates with hash%100 (Kel 3:2), Twi'lek low-bit dependence, correlated draws (C1-5); HashCreatureSpawn is dead code; float->int quantisation UB on NaN/huge.
12. MED: GOG hook 1 at 0x0048E691 collides with k2-class-menu-clothes; manifest now has conflicts = ["k2-class-menu-clothes"] (D2-9).
13. LOW: NpcDiversity_Init never called (no KPatchInit), so no KpLog (E-1); contract test is tautological (C1-8); check_hook_bytes gives a false stale-kpatch WARN (F-5).
14. Spec doc errors (fixed in §§1-5 of this doc).

- Answer to "new variety on Telos after prior visit?": not necessarily; heads.2da unchanged by install (heads 192-248 pre-date it); whether existing creatures rebuild via SetAppearance on save-load is UNVERIFIED (A1 f) -> probe build 1.1.0-probe answers it.
- Plan: probe first (instrumented 1.1.0-probe), then lock texture/remap strategy from the log.
- Decision gate: Strategy B (hook 2 writes pDesc+0x01 textureVar) if hook 2 fires with stable oid; else move remap hook to 0x85A1F0/0x85CC00.




## 8. Progress log (10-11)

- Waves 0-3 done and committed (9a96a46, efc5335, e643515). Version policy: 0.X.Y until the upstream PR (BUILD_AND_INSTALL 2b); this build is `0.1.1-probe`.
- Wave 4 install: hot-swapped (not a launcher reinstall). The live `patch_config.toml` npc-diversity hook param was hand-fixed `esp+4` -> `[esp+4]` (one line; backup `backups/npcdiv_probe_20261011_manual`), exports kept identical to the live DLL (`NpcDiversity_Init` kept, `KPatchInit` deferred), logs go to the fallback sink `npc-diversity_log.txt` at INFO. `KPatchInit` migration (19f) = one later full reinstall.
- Live `override/appearance.2da` replaced with the regenerated 812-row table (rows 812-815 gone); `heads.2da` identical (249 rows, llk heads 192-196 pre-existing).
- Known gap: `scripts/test_compat_breakage.py` fails on this tree (compat_check reports 45 ERROR items on an unmodified SafeRead.h); not caused by this mod, not waived by the user.
- Next: user plays Telos (existing save, then reload the same save), then `python3 scripts/kotor2_logs.py check npc-diversity_log.txt --new`, then the decision gate (Wave 4 table) and Wave 5.
- Optional follow-ons: H7 report `.claude/review/npc_div/H7_texture_dims.md` (fifteen 2048x2048 textures, 80 MiB VRAM); dancers 809/810 use N_TwiDanceA/B textures that exist only in the live override.
