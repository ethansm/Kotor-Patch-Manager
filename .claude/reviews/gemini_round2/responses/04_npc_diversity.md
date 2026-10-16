REVIEW npc-diversity | files read: 28 | findings: 14

### R2-ND-01 - Tag exclusion is never called and keep-default relies solely on area-agnostic live (x, y) coordinates
- Class: reachable
- Severity: medium
- Confidence: 95
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:476-514 :: RemapNpcAppearance
- Anchor: `    ExcludeKind kind = ExcludeKind::None;`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:448`) -> hardcodes `const char tag[1] = {};` (`NpcDiversityDispatcher.cpp:476`) and `ExcludeKind kind = ExcludeKind::None;` (`NpcDiversityDispatcher.cpp:501`) without calling `ClassifyExclusion` -> calls `IsKeepDefault(tag, x, y, xyOk)` (`NpcDiversityDispatcher.cpp:511`) -> `IsKeepDefaultTag("")` returns `false` (`patch_manager_mods/npc_diversity_src/DiversityKeep.h:36`) and `IsKeepDefaultPos(x, y)` checks live coordinates against `roles::kKeepPositions` (`DiversityKeep.h:56-70`).
- Impact: `DiversityExclusion.h` (`ClassifyExclusion`) and `DiversityRoles.h` `kKeepTags[112]` are completely inert at runtime. Because `self + 0x24` / `self + 0x28` holds the creature's live world position rather than its spawn position, any keep-default NPC on a generic commoner row (112 tags including `203_batono`, `npc_ralon`, `lootra`, `aaida`, `nadaa`, `kiph`, `odis`, `waitress`, `bartender`, `mechanic`, `czerka_employee`) that walks or moves more than 0.3 m (`kKeepPosTol10 = 3`) before `SetAppearance` runs (e.g. on save/load or appearance refresh) loses its `IsKeepDefaultPos` protection and gets diversified. Conversely, because `roles::kKeepPositions[158]` merges `(x, y)` coordinates from all modules across the game without an area ResRef or Z coordinate, any generic commoner in any module whose `(x, y)` falls within +/-0.3 m of any of those 158 points from another module is falsely skipped as `why=keep`.
- Fix: Resolve and read the creature tag (or pair with the server-side tag table in `npc-world`), pass `tag` to `ClassifyExclusion(tag)` to populate `kind`, and scope position fallback entries by area ResRef (or initial spawn position) rather than a global 2D `(x, y)` list.

### R2-ND-02 - RemapNpcAppearance remaps pDesc->appearance on armored descriptors (subType >= 3) while withholding textureVar and clobbering why="tvarmor"
- Class: reachable
- Severity: medium
- Confidence: 92
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:525-559 :: RemapNpcAppearance
- Anchor: `        if ((why[0] == 'n' || why[0] == 't') && (newApp != d.appearance || newTv != d.textureVar)) why = "ok";`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:448`) with a generic commoner row (`skip == SkipReason::None`), `ShouldVary` true, and `d.subType >= 3` (armor body path) -> `RemapAppearance` (`NpcDiversityDispatcher.cpp:527`) overwrites `pDesc->appearance = cand` (`NpcDiversityDispatcher.cpp:530`, e.g. `802` `APP_COMMONER_FAT_MALE` or `803` `APP_COMMONER_FAT_FEMALE`) -> `static_cast<int>(d.subType) > layout::kDescSubTypeMaxCommoner` (`NpcDiversityDispatcher.cpp:546`) withholds writing `pDesc->textureVar = want` (`34`/`35`) and sets `why = "tvarmor"` (`NpcDiversityDispatcher.cpp:548`) -> line 559 matches `why[0] == 't'` and `newApp != d.appearance` and overwrites `why` with `"ok"`.
- Impact: An armored NPC (`subType >= 3`) on a generic commoner row still has its `pDesc->appearance` remapped to unarmored fat/dirty/Czerka/dancer rows (`802..817`) while `pDesc->textureVar` is left at the armor's variant index instead of the required mesh slot (`34` for fat male `802`, `35` for fat female `803`), and the `"tvarmor"` log reason is overwritten with `"ok"`.
- Fix: Skip both row remapping and `textureVar` mutation when `static_cast<int>(d.subType) > layout::kDescSubTypeMaxCommoner` (before `ShouldVary` / `RemapAppearance`).
- Diff:
```diff
--- a/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
+++ b/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
@@ -515,6 +515,9 @@ __declspec(dllexport) void __cdecl RemapNpcAppearance(void* pCreature, layout::A
     } else if (!okOid || oid == 0) {
         // The seed is the creature id; without it nothing is written (the creature stays vanilla).
         why = "unread:oid";
+    } else if (static_cast<int>(d.subType) > layout::kDescSubTypeMaxCommoner) {
+        ++g_c.remTvArmor;
+        why = "tvarmor";
     } else if (!ShouldVary(oid, d.appearance, ActiveVisitSalt(g_visitVariety, g_visitSalt))) {
         // Only kVaryPct of generic commoners change; the roll is a pure function of (oid, original row).
         ++g_c.remNoVary;
```

### R2-ND-03 - Non-commoner rows (SkipReason::NotCommonerRow) are not gated before IsKeepDefault, ShouldVary, and subType checks
- Class: reachable
- Severity: low
- Confidence: 98
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:500-548 :: RemapNpcAppearance
- Anchor: `    } else if (!ShouldVary(oid, d.appearance, ActiveVisitSalt(g_visitVariety, g_visitSalt))) {`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:448`) for any non-commoner creature (companions, bosses, aliens, droids, soldiers) with `d.gender <= 1` and `d.appearance < APP_FIRST_APPENDED` not in `DiversityTables.h` -> `ShouldSkipDescriptor` (`NpcDiversityDispatcher.cpp:500`, `patch_manager_mods/npc_diversity_src/HookDecision.h:25`) returns `SkipReason::NotCommonerRow` -> bypasses `skip == SkipReason::Gender` (`NpcDiversityDispatcher.cpp:508`) and runs `IsKeepDefault` (`NpcDiversityDispatcher.cpp:511`), `ShouldVary` (`NpcDiversityDispatcher.cpp:518`), `++g_c.remVary` (`NpcDiversityDispatcher.cpp:523`), and `d.subType > layout::kDescSubTypeMaxCommoner` (`NpcDiversityDispatcher.cpp:546`).
- Impact: Every non-commoner creature with `gender <= 1` pollutes `g_c.remKeep`, `g_c.remNoVary` (~65% of non-commoners), `g_c.remVary` (~35% of non-commoners), and `g_c.remTvArmor` (even when `want == 0`). `g_c.remSkipRow` only counts the ~35% of non-commoners that happen to pass `ShouldVary`, and `why = "skip:row"` (documented in `NpcDivLog.h:283`) is never emitted anywhere.
- Fix: Gate `skip == SkipReason::NotCommonerRow` (when the row is not an already-remapped appended row `APP_FIRST_APPENDED..APP_COUNT-1`) immediately after `skip == SkipReason::Gender`, incrementing `g_c.remSkipRow` and setting `why = "skip:row"`.
- Diff:
```diff
--- a/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
+++ b/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
@@ -508,6 +508,9 @@ __declspec(dllexport) void __cdecl RemapNpcAppearance(void* pCreature, layout::A
     } else if (skip == SkipReason::Gender) {
         ++g_c.remSkipGender;
         why = "skip:gender";
+    } else if (skip == SkipReason::NotCommonerRow && (d.appearance < APP_FIRST_APPENDED || d.appearance >= APP_COUNT)) {
+        ++g_c.remSkipRow;
+        why = "skip:row";
     } else if (IsKeepDefault(tag, x, y, xyOk)) {
         // Named / merchant / uniformed / plot NPCs (position list, plus the tag list once the tag is readable).
         ++g_c.remKeep;
```

### R2-ND-04 - OverrideNpcTexture (3bba848) mislabels B-type non-commoner/non-armor textures as non-B model types and checks empty texa at an unreachable site
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:401-432 :: OverrideNpcTexture
- Anchor: `        } else if (!isArmorBodyRes(cur)) {`
- Trigger: `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` detour (`0x0085E601` / `0x0048E691`) -> `OverrideNpcTexture` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:372`) on any B-type appearance row whose texture ResRef is neither `N_Comm*` (`IsCommonerTexture`, line 412) nor `pmb*`/`pfb*` (`isArmorBodyRes`, line 422), such as `N_CzerkaOff01` (row 804), `N_TwiDanceA01` (row 809), `N_CzerkaOffC01` (row 814), `N_RepOff01`, or `P_AttonB01` -> increments `++g_texNonB` (`NpcDiversityDispatcher.cpp:424`) and logs `STAT texa emptyCell=%u nonB=%u` (`NpcDiversityDispatcher.cpp:279`).
- Impact: `GetBTypeModelAndTextureResRef` is only called for `modeltype == B` rows; true non-B model types (`F`, `S`, `L`) never reach `0x0085E601`. Instead, `g_texNonB` counts B-type unique/uniform/companion clothing textures (including this mod's own rows `804, 809, 810, 814`). Likewise, as noted in `patch_manager_mods/34_npc_diversity_dispatcher.md:252` (`0x85E543 -> 0x85E60F`), the engine jumps over `0x0085E601` when `texa` is empty, so `if (cur[0] == '\0') { ++g_texEmptyCell; return; }` at line 403 cannot observe empty `texa` cells that take the `0x85E543` branch.
- Fix: Rename the counter/comment from `nonB` ("Non-B model types") to reflect B-type non-commoner/non-armor textures (e.g. `otherB`), and note that observing empty `texa` cells or non-B model types requires hooking `CreateBodyAppearance` (`0x0085A1F0`) or before `0x0085E543`.

### R2-ND-05 - logBanner passes APP_TWILEK_FEMALE_UNDERWEAR (811) instead of APP_COUNT - 1 (817) as the last appended row
- Class: reachable
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:245-247 :: logBanner
- Anchor: `    nl::FormatBannerRows(b, sizeof b, APP_FIRST_APPENDED, APP_TWILEK_FEMALE_UNDERWEAR, APP_COUNT - APP_FIRST_APPENDED,`
- Trigger: First call to `OverrideNpcTexture` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:374`) or `RemapNpcAppearance` (`NpcDiversityDispatcher.cpp:450`) -> `ensureInit` (`NpcDiversityDispatcher.cpp:164`) -> `logBanner` (`NpcDiversityDispatcher.cpp:245`) -> `nl::FormatBannerRows` (`patch_manager_mods/npc_diversity_src/NpcDivLog.h:244`).
- Impact: `DiversityRows.h:9-35` defines 16 appended rows `802..817` (`APP_COUNT = 818`, ending at `APP_TWILEK_FEMALE_TONE_C = 817`), but `logBanner()` passes `APP_TWILEK_FEMALE_UNDERWEAR` (`811`) as `appHi` alongside `APP_COUNT - APP_FIRST_APPENDED` (`16`) and `APP_COUNT` (`818`), logging the inconsistent line `BANNER rows app=802-811 n=16 head=192-196 n=5 appcount=818 headcount=249`.
- Fix: Pass `APP_COUNT - 1` (or `APP_TWILEK_FEMALE_TONE_C`) as `appHi` in `logBanner()`.
- Diff:
```diff
--- a/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
+++ b/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
@@ -242,7 +242,7 @@ void logBanner() {
             KP_LOGV(::kplog::detail::gMain, KPLOG_WARN, "%s", b);
         }
     }
-    nl::FormatBannerRows(b, sizeof b, APP_FIRST_APPENDED, APP_TWILEK_FEMALE_UNDERWEAR, APP_COUNT - APP_FIRST_APPENDED,
+    nl::FormatBannerRows(b, sizeof b, APP_FIRST_APPENDED, APP_COUNT - 1, APP_COUNT - APP_FIRST_APPENDED,
                          HEAD_LLK_AHRNELL, HEAD_LLK_DAHNISH, HEAD_LLK_DAHNISH - HEAD_LLK_AHRNELL + 1, APP_COUNT,
                          HEAD_COUNT);
     logLine(chInit, b);
```

### R2-ND-06 - PickTextureVar is called with currentTv = 0, making current-variant collision avoidance dead code in production
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:545 :: RemapNpcAppearance
- Anchor: `        const uint8_t want = PickTextureVar(oid, newApp, 0, ActiveVisitSalt(g_visitVariety, g_visitSalt));`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:448`) -> calls `PickTextureVar(oid, newApp, 0, ...)` (`NpcDiversityDispatcher.cpp:545` -> `patch_manager_mods/npc_diversity_src/DiversityLogic.h:180-187`) when a creature passes `ShouldVary` (35%), keeps its vanilla appearance row, and `allow[idx]` equals its existing `d.textureVar` (e.g. `1`).
- Impact: Because `DiversitySlots.h` contains only nonzero slots (`1..17, 34, 35`) and `RemapNpcAppearance` passes `currentTv = 0`, `if (allow[idx] == currentTv && n > 1)` in `DiversityLogic.h:186` never triggers in production. Roughly 1/16 (6.25%) of varied male commoners and 1/17 (5.88%) of varied female commoners that stay on their base row draw their existing vanilla `textureVar` (`want == d.textureVar`), resulting in no texture change (`why=nochg`) despite passing `ShouldVary`.
- Fix: Either track the creature's initial `textureVar` per `(oid)` so `currentTv` can be passed idempotently, or document that `currentTv` is unused by the dispatcher and remove the dead parameter.

### R2-ND-07 - oneShotCreDumps and onePtrDumps latch their completion bitmasks before verifying readable/non-null memory
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:315-341 :: oneShotCreDumps
- Anchor: `        g_creDumpDone |= 1u << i;`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:465-466`) -> `oneShotCreDumps` (`NpcDiversityDispatcher.cpp:317-321`) sets `g_cre0Done = true` and `g_creDumpDone |= 1u << i` before `oneShotCreN` (`NpcDiversityDispatcher.cpp:307`) checks `saferead::probeRead`, and `onePtrDumps` (`NpcDiversityDispatcher.cpp:332-335`) sets `g_ptrDumpDone |= 1u << i` before checking `p != 0 && saferead::probeRead(...)`.
- Impact: Contrary to the comment at line 314 (`from the first readable creature`) and unlike `oneShotDsc0` (lines 296-298), if `probeRead` fails on the first creature in `oneShotCreN`, that window is never dumped for the rest of the session; in `onePtrDumps`, if the pointer at `self + off` is `0` or unreadable on the first creature, it logs 64 zero bytes (`p=00000000`) and never probes subsequent creatures.
- Fix:Have `oneShotCreN` return `bool` and set `g_cre0Done` / `g_creDumpDone` only when `probeRead` succeeds, and in `onePtrDumps` set `g_ptrDumpDone |= 1u << i` only when `p != 0` and `probeRead` succeeds.

### R2-ND-08 - STAT read never increments g_c.fail for sites 2, 3, 6, 7, 8, 10, 11 and misattributes S_FRAME_TEX (site 2) to S_FRAME_DESC (site 1)
- Class: reachable
- Severity: low
- Confidence: 98
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:381-385 :: OverrideNpcTexture
- Anchor: `        ++g_c.texUnread; logRead(S_FRAME_DESC, fr, 0x18); return;`
- Trigger: `OverrideNpcTexture` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:381-385`) or `RemapNpcAppearance` (`NpcDiversityDispatcher.cpp:465-475`) encounters a failed `saferead` probe at `S_FRAME_TEX` (line 382), `S_FRAME_RET` (line 385), `S_CRE_PTR` (line 331), `S_CRE_OID` (line 473), `S_CRE_XY` (lines 474-475), `S_DUMP` (lines 213-215, 296, 307, 335), or `S_HOOKBYTES` (line 194) -> `dumpStats` (`NpcDiversityDispatcher.cpp:280`) -> `nl::FormatStatRead` (`patch_manager_mods/npc_diversity_src/NpcDivLog.h:436`).
- Impact: `g_c.fail[site]` is only incremented inside `logRead()`, which is only ever called with `S_FRAME_DESC` (1), `S_DESC` (4), and `S_TEXRES` (5). A failure at `S_FRAME_TEX` (2) is logged and counted under `S_FRAME_DESC` (1) at line 383, and failures at sites `3, 6, 7, 8, 10, 11` are never counted in `g_c.fail`, so `STAT read 1:%u ... 10:%u` permanently reports `0` for sites `2, 3, 6, 7, 8, 9, 10`.
- Fix: Separate the `S_FRAME_DESC` and `S_FRAME_TEX` checks so `S_FRAME_TEX` calls `logRead(S_FRAME_TEX, ...)`, and call `logRead` (or increment `g_c.fail[site]`) when `S_FRAME_RET`, `S_CRE_PTR`, `S_CRE_OID`, `S_CRE_XY`, or `S_DUMP` fails.

### R2-ND-09 - dumpStats emits multi-line saferead::dump output into single-line KP_LOG, and FormatRmpExt formats unclamped floats when !xyOk
- Class: reachable
- Severity: low
- Confidence: 95
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:283-285 :: dumpStats
- Anchor: `    KP_LOG(chStat, "STAT ring %s", ring);`
- Trigger: Hook call 64, 256, or `1024 * k` -> `maybeDump` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:290`) -> `dumpStats` (`NpcDiversityDispatcher.cpp:283-285`) -> `saferead::dump(ring, sizeof ring)` (`patch_manager_mods/_shared_src/SafeRead.h:211-215`) writes multi-line text with embedded `\n`s (up to 479 bytes) into `KP_LOG(chStat, "STAT ring %s", ring)`; and in `RemapNpcAppearance` (`NpcDiversityDispatcher.cpp:481, 564`), when `!xyOk` (`!plausibleCoord(x)`), raw `x, y` are still formatted with `%.1f` in `nl::FormatRmpExt` (`patch_manager_mods/npc_diversity_src/NpcDivLog.h:296`).
- Impact: `STAT ring` injects trailing and multi-line newlines into `npc-diversity_log.txt` and exceeds the 232-byte single-line grammar cap. In `FormatRmpExt`, an unplausible float (e.g. `1e38f`) expands to ~40 digits in `xy=%.1f,%.1f`, truncating `app=`, `tv=`, `h=`, `why=`, and `stype=` off the 231-char `RMP` line.
- Fix: Pass `xyOk ? x : 0.0f` and `xyOk ? y : 0.0f` to `FormatRmpExt` (keeping raw bits in `xyb=`), and log `saferead` ring entries one single-line record at a time without trailing newlines.

### R2-ND-10 - In-place mutation of pDesc->appearance to appended rows (802..817) alters HookDecision on re-entry and prevents visit_variety re-rolls
- Class: latent
- Severity: medium
- Confidence: 90
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:500-558 :: RemapNpcAppearance
- Anchor: `        if (skip == SkipReason::None) {`
- Trigger: `CSWCCreature::SetAppearance` detour -> `RemapNpcAppearance` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:448`) overwrites `pDesc->appearance = cand` (`802..817`, line 530) -> on a subsequent `RemapNpcAppearance` call with that mutated descriptor, `ShouldSkipDescriptor` (`patch_manager_mods/npc_diversity_src/HookDecision.h:25`) returns `SkipReason::NotCommonerRow` instead of `SkipReason::None`, `ShouldVary` (`patch_manager_mods/npc_diversity_src/DiversityLogic.h:172`) forces `true`, line 541 increments `g_c.remSkipRow`, and `RemapAppearance` is skipped; additionally, when `cand` is a single-texture row (`804, 809, 810, 812..817`), `PickTextureVar` returns `0` (`NpcDiversityDispatcher.cpp:545`) and line 549 (`else if (want != 0 && want != d.textureVar)`) leaves any prior `pDesc->textureVar` untouched.
- Impact: If the engine re-invokes `SetAppearance` with the mutated descriptor (or across area visits once `visit_variety` supplies a new `g_visitSalt`), creatures remapped to `802..817` are permanently locked into their appended row and cannot re-roll `ShouldVary` or `RemapAppearance`, while creatures that stayed on their vanilla row re-roll on the next visit.
- Fix: Record a small `oid -> originalAppearance` map on first sight of each creature so subsequent `RemapNpcAppearance` calls key `ShouldSkipDescriptor`, `ShouldVary`, and `RemapAppearance` on the creature's original row, and reset `pDesc->textureVar = 1` when remapping to a single-texture appended row (`804, 809, 810, 812..817`).

### R2-ND-11 - Off-by-one bounds check (site <= nl::kSiteCount) in logRead allows out-of-bounds write to g_c.fail[12]
- Class: latent
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:171-173 :: logRead
- Anchor: `    if (site <= nl::kSiteCount) ++g_c.fail[site];`
- Trigger: `logRead(site, addr, len)` (`patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:171`) called with `site == nl::kSiteCount` (`12`, `patch_manager_mods/npc_diversity_src/NpcDivLog.h:25`) -> indexes `g_c.fail[12]` on `uint32_t fail[kSiteCount]` (`NpcDivLog.h:150`).
- Impact: `g_readBudget` at line 117 has size `nl::kSiteCount + 1` (`13`), but `Counts::fail` in `NpcDivLog.h:150` has size `nl::kSiteCount` (`12`, valid indices `0..11`). Passing `site == 12` writes past `g_c.fail` into `g_c.slot[0][0]`.
- Fix: Change `site <= nl::kSiteCount` to `site < nl::kSiteCount` (and `site >= nl::kSiteCount` on the next line), or size `Counts::fail` as `kSiteCount + 1`.
- Diff:
```diff
--- a/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
+++ b/patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp
@@ -169,8 +169,8 @@ uint32_t charVal(char c) { return static_cast<uint32_t>(static_cast<unsigned cha
 void logLine(kplog::Channel& ch, const char* line) { KP_LOG(ch, "%s", line); }
 
 void logRead(unsigned site, uintptr_t addr, unsigned len) {
-    if (site <= nl::kSiteCount) ++g_c.fail[site];
-    if (site > nl::kSiteCount || ++g_readBudget[site] > 3) return;
+    if (site < nl::kSiteCount) ++g_c.fail[site];
+    if (site >= nl::kSiteCount || ++g_readBudget[site] > 3) return;
     char b[nl::kLineMax];
     nl::FormatRead(b, sizeof b, site, static_cast<uint32_t>(addr), len, 0, g_readBudget[site]);
     KP_LOGV(chRead, KPLOG_WARN, "%s", b);
```

### R2-ND-12 - DiversityHeadItems.h is never included or tested in C++, and findAppearance misses 57 of 65 generic commoner rows and all 16 appended rows
- Class: test-gap
- Severity: medium
- Confidence: 98
- Location: patch_manager_mods/npc_diversity_src/DiversityHeadItems.h:135-147 :: findAppearance
- Anchor: `inline constexpr const HeadAppearance* findAppearance(uint16_t row) {`
- Trigger: Calling `diversity_head_items::findAppearance(row)` (`patch_manager_mods/npc_diversity_src/DiversityHeadItems.h:135`) on generic commoner rows (`14, 185..194, 280, 339..349, 416, 417, 445..450` in `patch_manager_mods/npc_diversity_src/DiversityTables.h:15-65`) or appended rows (`802..817` in `patch_manager_mods/npc_diversity_src/DiversityRows.h:9-24`).
- Impact: `DiversityHeadItems.h` is not `#include`d in `NpcDiversityDispatcher.cpp`, any header, or any `*_test.cpp`. Because `kHeadAppearances[54]` only lists rows observed wearing headgear in vanilla UTCs (`flags & 0x04 != 0`), `findAppearance(row)` returns `nullptr` for 57 of the 65 diversifiable rows in `DiversityTables.h` and all 16 appended rows `802..817`. Meanwhile, 8 rows in `DiversityTables.h` (`17, 591, 592, 595, 596, 606, 612, 614`) are recorded in `kHeadAppearances` as wearing head items in vanilla (and row `606` lacks `GoggleHook`, `flags = 0x05`), yet `RemapNpcAppearance` never consults `DiversityHeadItems.h` or `d.headOverride`.
- Fix: Add a C++ unit test covering `DiversityHeadItems.h` (`findAppearance`, `itemAllowed`, row sort order, and bitset bounds), and either wire head-item / `GoggleHook` eligibility into `RemapNpcAppearance` or remove the unused header.

### R2-ND-13 - diversity_logic_test tests RemapAppearance distributions with dead HashCreaturePosition instead of live HashCreatureId, and common_contract_test / diversity_tables_test omit key checks
- Class: test-gap
- Severity: medium
- Confidence: 100
- Location: patch_manager_mods/npc_diversity_src/diversity_logic_test.cpp:16-19 :: DrawHash
- Anchor: `    return NpcDiversity::HashCreaturePosition(static_cast<float>(i) * 0.37f,`
- Trigger: `testMaleCommonerDistribution` (`patch_manager_mods/npc_diversity_src/diversity_logic_test.cpp:320`), `testFemaleCommonerDistribution` (`diversity_logic_test.cpp:345`), `testTwilekMaleDistribution` (`diversity_logic_test.cpp:369`), and `testTwilekFemaleDistribution` (`diversity_logic_test.cpp:405`) call `DrawHash(i, row)` (`diversity_logic_test.cpp:16-19`), which invokes `HashCreaturePosition` (`patch_manager_mods/npc_diversity_src/DiversityLogic.h:240`).
- Impact:
  1. `NpcDiversityDispatcher.cpp:526` hashes creatures with `HashCreatureId(oid, d.appearance, ...)`, never `HashCreaturePosition` (which is dead code in `DiversityLogic.h`, along with `QuantizeCoord`, `ShouldDiversifyTexture`, `HashDescriptor`, `HashSeedFallback`, and `HookDecision.h`'s `ResolveTextureSeed` and `IsPlausibleResRef`). A defect or bias in `HashCreatureId` feeding `RemapAppearance` will not fail any of the four distribution tests in `diversity_logic_test.cpp`.
  2. In `patch_manager_mods/npc_diversity_src/common_contract_test.cpp:32-41`, `testDescriptorOffsets()` compares `offsetof(AppearanceDesc, field)` to `L::kDescOffField` (already `static_assert`ed in `NpcDivLayout.h:36-42`) rather than literal hex offsets (`0x01, 0x14, 0x18, 0x1A, 0x1B, 0x1C`), and never includes or tests `_shared_src/SafeRead.h` or `KpLog.h` (flagged as `C1-8` in `34_npc_diversity_dispatcher.md:260` and still unfixed).
  3. In `patch_manager_mods/npc_diversity_src/diversity_tables_test.cpp:59-68`, only 14 of the 65 rows in `DiversityTables.h` are checked; in particular, `417, 449, 450, 619` in `IsGenericTwilekMale` and `447, 448, 621, 622` in `IsGenericTwilekFemale` are never tested anywhere in the C++ test suite.
- Fix: Change `DrawHash` in `diversity_logic_test.cpp` to call `NpcDiversity::HashCreatureId(i + 1u, row)`, pin literal hex offsets and `SafeRead.h` contracts in `common_contract_test.cpp`, and test all 65 rows (plus a full `0..65535` count/disjointness sweep across all 4 tables) in `diversity_tables_test.cpp`.

### R2-ND-14 - Design doc 34 and README.md misstate the active hash inputs, Hook 1 behaviour, exclusion status, and appended row count
- Class: doc
- Severity: low
- Confidence: 100
- Location: patch_manager_mods/34_npc_diversity_dispatcher.md:7-8 :: Executive Summary
- Anchor: `2. **The KPM Runtime Dispatcher Detour**: A deterministic C++ engine hook in `swkotor2.exe` that dynamically varies NPC body shapes, head assignments, and clothing texture slots (01–35+) based on creature coordinates, area ResRef, and tag hash.`
- Trigger: Comparing `patch_manager_mods/34_npc_diversity_dispatcher.md:7-8, 136-156, 172-176, 274` and `patch_manager_mods/npc_diversity_src/README.md:1, 9-12` against `patch_manager_mods/npc_diversity_src/NpcDiversityDispatcher.cpp:369-560` and `patch_manager_mods/npc_diversity_src/DiversityRows.h:9-35`.
- Impact: Both docs state that (1) the runtime hash uses spawn coordinates `(X, Y)`, area ResRef, and creature tag (the code uses `HashCreatureId(oid, appearance, visitSalt)` and never reads area ResRef or tag), (2) `GetBTypeModelAndTextureResRef` (`OverrideNpcTexture`) rotates textures across `01..35` (Hook 1 is log-only; `RemapNpcAppearance` writes `pDesc->textureVar` from the 16/17-slot allowlists in `DiversitySlots.h`), (3) the tag exclusion guard protects companions and named story NPCs at runtime (`ClassifyExclusion` is never called in `NpcDiversityDispatcher.cpp`), and (4) `appearance.2da` appends rows `802-815` or is an `812-row table (rows 812-815 gone)` (`DiversityRows.h` appends 16 rows `802..817` with `APP_COUNT = 818`).
- Fix: Update `34_npc_diversity_dispatcher.md` and `npc_diversity_src/README.md` to match the 0.1.6-probe implementation (`HashCreatureId`, log-only Hook 1, position-only `IsKeepDefault` until tag reading is wired, and appended rows `802..817`).

## Retracted
- `saferead::regionWritable` integer wrap at `NpcDiversityDispatcher.cpp:529, 550`: `regionWritable` lacks `probeWrite`'s `a < 0x10000 || a + len < a` overflow check, but `saferead::probeRead(S_DESC, pDesc, sizeof d)` at line 459 already guarantees `pd >= 0x10000` and `pd + sizeof(d) >= pd` before lines 529 and 550 execute.
- Signed `char` `d.subType` comparison `static_cast<int>(d.subType) > layout::kDescSubTypeMaxCommoner` at `NpcDiversityDispatcher.cpp:546`: negative `char` values would pass `<= 2`, which matches `GetBTypeModelAndTextureResRef` clamping `subType` to `1..14` (so `<= 0` clamps to `1`, the commoner `modela`/`texa` slot).
- `cur[7]` indexing and `char` cast in `OverrideNpcTexture` census hash at `NpcDiversityDispatcher.cpp:405`: `cur` is a zero-initialized `char[17]` populated by `saferead::readResRef16`, which restricts non-NUL bytes to `0x20..0x7E`, so `cur[7]` is always in-bounds and non-negative.

## Not assessed
- `patch_manager_mods/npc_diversity_src/override/*`, `rejected_pool/*`, `*.json`: skipped per task scope instructions.
- `KpLog.h`: located outside `patch_manager_mods/_shared_src/` in the KPM Common include path; assessed only via its call sites in `NpcDiversityDispatcher.cpp`.
- Engine behavior of `CSWCObject::m_nObjectID` (`+0x04`) across save/load and module reload, and exact offset of the creature tag string on client `CSWCCreature`: needs reverse engineering / live probe verification (`40_npc_behaviour_variety.md` §3, §13.6).
