# Recon MG4 (equip3d) + MG5 (chareffects), doc 32. Read-only, 2026-10-05

Paths: E = patch_manager_mods/equipment_3d_character_src/, C = patch_manager_mods/active_effects_panel_src/.
Equip line numbers = HEAD EquipCharPreview.cpp (1001 lines; working tree 1016, +9 shift from ~line 193 on).
K = ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches.

## 0. Dirty tree + mirror state
- Dirty (other session) = vertical-drag PAN feature only. Equip3dLogic.h: Cfg panPerPx/panMin/panMax/lockPx, 4 parseIniLine keys (pan_per_px, pan_min, pan_max, drag_lock_px), clampCfg, panDrag, dragAxis. EquipCharPreview.cpp hunks: Slot +panZ/appliedPanZ/startX/startY/axis (@194), kTips[1] text (@381), tickButtons reset (@474), applyView CrSetPos (@732), buildPreview (@764/773), tickInput drag + MMB (@843-846). equip3d_logic_test.cpp +31.
- It does NOT touch logging, loadIni, slots, wheel, bindCover, DllMain. Overlap = tickInput only (HEAD 827-842). Sequence MG4 after that WIP commits.
- Mirror: E HEAD == K/EquipCharPreview/{EquipCharPreview.cpp,Equip3dLogic.h}; working tree differs (pan). Last KPM commit on EquipCharPreview: 48cc872 (SceneView sync, = C3b ba361f8); prior c9dbe25 (C3a).
- C: all 5 files identical to K/CharEffectsPanel. Last KPM commit: cb19d4e (TextFit, = C4c 8e23979); prior d6eb583 (GuiKit swaps, C4b).
- 8aed2f5 (MG3 recipe) touched: Mod .cpp; Inv3dLogic.h (`#include "IniKit.h"` + `using inikit::trim/parseFloats/splitIniLine/IniLine*`); common_contract_test.cpp (+125: IniKit splitIniLine, GuiKit pickScreen/syncScreen, WheelHook sections); exports.def (+KPatchInit); manifest version bump; test.toml (-I); tests/static/subclassers.toml (entry removed).

## 1. Logging
| item | E (HEAD) | C |
|---|---|---|
| helpers | `logf` 91-100 (stamps HH:MM:SS.mmm), `dlogf` 101-108 (no stamp) | `vlog` 96-101, `logf` 102, `dlogf` 103, `whyLog` 106-113 (10 s same-code throttle; writes via vlog, bumps g_logLines itself) |
| file | fopen("equip3d_log.txt","a") per line | fopen("chareffects_log.txt","a") per line |
| caps | MaxLogLines 15000, MaxDebugLines 8000 | 20000 / 10000 |
| debug toggle | `equip3d_debug.txt` (loadIni 135, maybeReloadIni 171) | `chareffects_debug.txt` (138, 203); DllMain also `saferead::enableRing(g_debug)` |
| load line | DllMain 999 `logf("equip3d 0.1.0-probe loaded")`; loadIni(); `g_rng ^= GetTickCount()` | DllMain 1004 `logf("chareffects 0.1.0-probe build 4c loaded")`; loadIni(); enableRing |
| manifest | id `equipment-3d-character`, 0.1.0-probe | id `character-effects-panel`, 0.1.0-probe |
| KPatchInit | not exported (BindEquipView, EquipTick, EquipDestroy) | not exported (BindCharEffects, CharEffectsTick, CharEffectsDestroy) |
| KpLog | not included | not included |
- Recipe (from MG3): `#include "KpLog.h"`; `KP_LOG_PATCH("<manifest id>");` `KP_CHANNEL(chMain,"main"); KP_CHANNEL(chDbg,"debug");` `const bool g_fallbackSet = (KpLog_SetFallbackFile("equip3d_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);` (C: "chareffects_log.txt"). logf/dlogf keep their caps, format into `char buf[600]` via vsnprintf, then `KP_LOGV(chMain|chDbg, KPLOG_INFO, "%s", buf)` (drop the SYSTEMTIME prefix; KpLog stamps). `char g_loadLine[160]` formatted in DllMain; `ensureInit()` (g_inited InterlockedExchange; logf(g_loadLine); loadIni()) called from `extern "C" void __cdecl KPatchInit(const KPatchApi* api){ KpLog_Init(api); ensureInit(); }` and as first statement of each hook; add `KPatchInit` to exports.def; bump manifest version; remove loadIni() from DllMain ATTACH.
- E hooks: BindEquipView 848, EquipTick 914, EquipDestroy 989 (each starts `saferead::beginScope()`; ensureInit goes before it). C hooks: Bind 886, Tick 935, Destroy 994 (Destroy has NO beginScope).
- C: whyLog must keep its throttle and route to KP_LOGV (its own counter bump 110-111). `enableRing(g_debug)` at DllMain runs BEFORE loadIni sets g_debug (always false there): move into ensureInit after loadIni.

## 2. INI
| item | E (HEAD) | C |
|---|---|---|
| loader | `loadIni` 134-163, fgets 143, `char line[600]` | `loadIni` 137-193, fgets 155, `char line[300]` |
| long line | `eq3d::drainLongLine` 145 -> "INI line %d rejected (longer than 599 chars)" | none |
| trim | `using eq3d::trim` 132 (Equip3dLogic.h 36) | local `trim` 130-134 |
| split | `eq3d::splitIniLine` (h 81-91) | inlined 156-163 |
| numbers | `eq3d::parseFloats` (h 61-73) + `toIntSat` (h 53-58) | `parseNum` 135 (strtof, `e!=v && !*e && isfinite`), `(int)a` |
| reload | `maybeReloadIni` 165-173, 1000 ms, mtime + debug-file flag, "INI changed on disk, reloading" | 197-205 same |
| reject logs to keep | no '=' 150, unknown key 152, bad value 154, keep_obj10 DEPRECATED 153, clamp 159-160, "INI loaded..." 162 | no '=' 159, unknown 171, bad 172, "INI %s = %s" 173 |
- Diff vs IniKit.h: E `trim` IDENTICAL; E `splitIniLine` IDENTICAL; C `trim` IDENTICAL; C inline split logic IDENTICAL to `inikit::splitIniLine`.
- E `parseFloats` NOT identical: E rejects hex-float tokens ("0x"/"0X" after optional ws and sign; F19 hardening, h 63-65). `inikit::parseFloats` accepts them (strtof). Do not alias blindly: keep E-local parseFloats (or add the guard to IniKit + inikit_test).
- E `drainLongLine`, `toIntSat` are not in IniKit: stay local. Keep the fgets loop in both (forEachLine would lose the per-line reject logs and the long-line log).
- C `parseNum` equals `inikit::parseFloats(v,&a,1)` for an already-trimmed v (both accept hex/exp via strtof, both reject nan/inf/empty).
- Tests: E equip3d_logic_test.cpp includes Equip3dLogic.h; if that header adds `#include "IniKit.h"` test.toml default flags need `-I../_shared_src` (8aed2f5 did this for inv3d). C chareff_decode_test already has -I../_shared_src.

## 3. Slot tables
E: `struct Slot` 176-204 (int panel; unsigned char* block; bool usable; unsigned seq; ...), `MaxSlots=4; Slot g_slots[4]; unsigned g_seq` 212-215.
```
Slot* findSlot(int panel) { for (auto& s : g_slots) if (s.panel == panel && s.block) return &s; return nullptr; }
Slot* claimSlot(int panel) {
    if (Slot* e = findSlot(panel)) { destroyCreature(e, "re-bind"); return e; }   // existing: fields NOT reset
    Slot* oldest = &g_slots[0];
    for (auto& s : g_slots) { if (!s.block) { oldest = &s; break; } if (s.seq < oldest->seq) oldest = &s; }
    if (oldest->block) { logf("slot: evicting panel=%08x seq=%u", oldest->panel, oldest->seq); destroyCreature(oldest, "evict"); }
    return oldest; }   // reused entry keeps stale fields; caller sets some
```
- Bind 871-873: `Slot* s = claimSlot(panel); s->panel=panel; s->block=block; s->seq=++g_seq; s->bindTick=GetTickCount(); s->zoom=1;` then 912 `logf("H1: slot %d ready (panel=%08x seq=%u)", (int)(s - g_slots), ...)`.
- H4 989-995: findSlot; `destroyCreature(s,"panel destroyed"); s->usable=false; s->block=nullptr; s->panel=0;`. `destroyCreature` 565-575 (detachWeapons/Helmet, dtor via vtable, resets creature/built/real/...).
- PanelSlots<Slot,4> mapping: key = `(uintptr_t)(unsigned)panel`. `bool fresh; Slot* s = g_slots.claim(key, ++g_seq, &fresh); if(!fresh) destroyCreature(s,"re-bind");` `g_slots.onEvict = [](Slot& s, uintptr_t panel, unsigned seq){ logf("slot: evicting panel=%08x seq=%u",(unsigned)panel,seq); destroyCreature(&s,"evict"); };` (plain fn pointer; set once in ensureInit; PanelSlots calls it BEFORE `slot = Slot()`, so the creature is still live in the callback). Fresh/evicted entries are value-reset (stale-field reuse disappears; Slot has default member inits so copy-assign works). H4: destroyCreature then `g_slots.release(key)`; `find()` no longer needs the `s.block` test. Early-out binds (H1 returns at ~875/880/888 with usable=false) stay findable with usable=false, same as before.
- Gotchas: (a) after release() the used entries are not an array prefix, so inv3d's `slotIndex` forEach trick misreports the index in the "slot %d ready" log (912): log seq instead. (b) hit claim refreshes seq to the passed value (E did `++g_seq` after claim anyway).

C: `struct Slot` 316-337 (int panel; bool bound; unsigned seq; DWORD bindTick; ...), `MaxSlots=4; g_slots; g_seq` 338-340.
```
Slot* findSlot(int panel) { for (auto& s : g_slots) if (s.panel == panel && s.bound) return &s; return nullptr; }
Slot* claimSlot(int panel) { if (Slot* e = findSlot(panel)) return e; /* first !bound else lowest seq */
    if (oldest->bound) logf("slot: evicting panel=%08x seq=%u", oldest->panel, oldest->seq); return oldest; }   // no destroy: controls are engine-owned
```
- Bind 893-896: `Slot* s = claimSlot(panel); if (s->mqActive && s->panel == panel) logf("MARQUEE: reset (rebind)"); *s = Slot(); s->panel=panel; s->seq=++g_seq; s->bindTick=...;` ; H1 failure 911 sets `s->panel=panel; s->bound=false`; success `s->bound=true` 931; 933 `logf("H1: slot %d ready", (int)(s - g_slots))`. Destroy 994-1000: `g_panelRect[2]=0; s->bound=false; s->panel=0;`.
- PanelSlots: `claim(key, ++g_seq, &fresh)`; keep the MARQUEE log when `!fresh && s->mqActive`; KEEP explicit `*s = Slot()` for the hit case (claim does not reset a hit; set panel/seq after, or reassign `s->seq` is already set). `bound` stays in the Slot payload; after the 911 failure path the entry is findable with bound=false: tick/destroy callers must test `s && s->bound`, or `release(key)` there. onEvict: log only (no destroy). Destroy -> `release(key)`.
- Slot is ~30 KB (ent[96], text[82][100], tipText[1024]) x4: fine.

## 4. GUI
Wheel (both already `kp::WindowHook<Tag>` after MG6; direct install/uninstall present):
- E 250-267: `struct Equip3dWheelTag{}; using WheelHook = kp::WindowHook<Equip3dWheelTag>; volatile LONG g_wheelAccum; volatile DWORD g_lastTickMs; volatile LONG g_viewRect[4]; int g_wheelLogs`.
```
if (WheelHook::active() && m == WM_MOUSEWHEEL && g_cfg.wheelZoom && GetTickCount() - g_lastTickMs < 300 && g_viewRect[2] > 0) {
    POINT pt = {LOWORD(l),HIWORD(l)}; ...ScreenToClient/GetClientRect; syncScreen(); eq3d::clientToEngine(...);
    inside = eq3d::rectContains(x, y, g_viewRect[0..3]);
    if (g_wheelLogs < 20) logf("WHEEL: delta=%d cursor=(%ld,%ld) inside=%d", ...);
    if (inside) { InterlockedExchangeAdd(&g_wheelAccum, (short)HIWORD(w)); return 0; } }
return WheelHook::forward(h, m, w, l);
```
  No saferead::beginScope in the proc (`scoped` stays false). Install in gameWindow 279-282 (`g_hwnd && g_cfg.wheelZoom && !WheelHook::old`), log "WHEEL: subclassed window %p, old proc %p (err=%lu)". DllMain DETACH 998 `WheelHook::uninstall(&wheelProc)`. Producers: `g_lastTickMs = GetTickCount()` EquipTick 922; rect `g_viewRect[0..3]=ext[0],(LONG)top,ext[2],ext[3]` 827 (tickInput; stale after destroy, gated by 300 ms). Consumer 839 `InterlockedExchange(&g_wheelAccum,0)`, log "WHEEL: applied %ld -> zoom=%.2f" 842.
- C 224-240: same shape (`WheelTag`, `g_panelRect[4]` int engine px, `[2]=0` while the list does not overflow; written 733-734, zeroed 608/623/716/998), gate `g_cfg.wheelScroll`; `inside` is a plain `x>=rx && x<rx+rw && y>=ry && y<ry+rh`; install 252 uses `!WheelHook::active()`; DllMain 1003. Extra reader 951 `bool wheelPending = g_wheelAccum != 0;` -> `guikit::g_wheel.accum != 0` (public volatile). Consumer 758 `InterlockedExchange(&g_wheelAccum,0)`. `g_lastTickMs` set 943.
- MG3 recipe: `guikit::g_wheel.enabled = (cfg.wheelZoom|wheelScroll)!=0` refreshed after every loadIni (the copies read g_cfg live); `g_wheel.onEvent = wheelEvent` (logs the same WHEEL line); `scoped=false`; `g_wheel.iniW/iniH = cfg.screenW/H` when screenExplicit; `guikit::wheelInstall()` in gameWindow (`!guikit::g_wheelHwnd`); `guikit::wheelUninstall()` in DllMain DETACH; `g_wheel.touch(GetTickCount())`; `g_wheel.setRect(0,x,y,w,h)` (w<=0 inactive; C's `g_panelRect[2]=0` sites become `setRect(0,0,0,0,0)`); `g_wheel.take()`.
- tests/static/subclassers.toml: DELETE the E entry (L21-27, hints 295/1017) and C entry (L29-35, hints 256/1030) once `WheelHook::install/uninstall` text is gone. Keep ActionHotbar's.
Screen sync:
- E `syncScreen` 245-249: `if(g_cfg.screenExplicit) return; readAt<int>(0x009F42A4/A8); if(eq3d::screenValid(w,h)) {g_cfg.screenW=w; ...}` -> inv3d form `int w=(int)g_cfg.screenW,h=(int)g_cfg.screenH; if (guikit::syncScreen(explicit?w:0, explicit?h:0, &w,&h)) {assign}`. C `syncScreen` 219-223 same with inline range test; C loadIni 191 resets screen to 3840x1600/explicit=false when <100.
Cursor:
- E `cursorEngine` 285-291 / C 258-264: gameWindow(); GetCursorPos+ScreenToClient+GetClientRect(>=1); `syncScreen()`; map with cfg screenW/H (E via eq3d::clientToEngine, C inline). Return false when no window.
- -> `if(!gameWindow()) return false; syncScreen(); return guikit::cursorEngineF(x,y,(int)g_cfg.screenW,(int)g_cfg.screenH);` (inv3d form). Pass cfg values (>0 -> SS_Ini branch) so the no-readable-globals case still maps with the cfg default 3840x1600; passing 0,0 would fall back to raw client px (doc 32 sec. 5 warning).
Bind-by-tag:
- E `bindCover(panel, tag, expectId)` 294-314 returns unsigned char*; callers 899 (toolbar, expectId ExpectedControlId+3+i), 902 (menu labels), 909 (tipBg/tip). C `bindLabel(panel, tag, expectId)` 267-289, caller loop 904 over NumTags (82), expectId FirstId+t. Both: alloc+zero+LabelCtor+vtable check+InitControl(addToList=1)+readback; flags: id mismatch -> clear bit1; ALWAYS set 0x20; return ctl iff id==expectId. E calls `saferead::beginScope()` after the ctor (302 area).
- -> `guikit::bindByTag(panel, tag, expectId, 0, &info)` (identical flag rule via bindFlags; labelSize 0 = table LabelSize 0x148). Rebuild the log line from `info` (ctl,id,flagsBefore,flagsAfter,ext). bindByTag does NOT call saferead::beginScope (only the wheel proc does when `scoped`): add `saferead::beginScope()` after each bind.
- Local engine consts/types that go dead after: E 31-36 FnInitControl/FnExoCtor/FnExoDtor/FnNew/FnLabelCtor/LabelSize/LabelVftable and aliases InitCtl/CExoCtor/CExoDtor/NewFn (E still needs FnLoadImage/FnTextSet/BorderVtable/TextVtable/Label*Off while its raw text/fill code remains). C 45-48 same group.
Setters / flags:
- E `setCtlVisible` 367-371 and C `setCtlVisible` 337-340: read-modify-write of bit1 ONLY (`fl|2` / `fl&~2`). They never touch 0x20. `guikit::setVisible` (GuiKit.h 234-239) sets bit1 and CLEARS 0x20 when shown / SETS it when hidden: swapping would make the click-through cover labels (toolbar, tooltip, effect rows; all polled, not clickable) swallow clicks. Keep local (still reads via readAt, writes raw) or add a bit1-only helper to GuiKit with a contract test. Do NOT swap to guikit::setVisible.
- E `setCtlRect` 372-376 (vtable slot 1 via probeRead) -> `guikit::setRect`; E `ctlRect` 352-357 -> `guikit::getExtent`; E `setLabelText` 331-342 (text vtable gate + ExoCtor/TextSet/ExoDtor, own cache, failure logf, dlogf) -> `guikit::setText` (C did this; keep cache + logs); E `btnImage` 318-330 (pool 8, logf "btn: image '%s' -> %p") -> `guikit::imageFor(resref,&loaded)` (C pattern); E `btnSetFill` 344-351 (BorderVtable gate, raw write of fill at border+0x74) -> `guikit::setFill(ctl,im)`. C already uses guikit for setText/setFill/setTextColor/getExtent/setRect/imageFor (C4b); C leftovers: bindLabel, setCtlVisible, syncScreen, wheel, cursorEngine.

## 5. MG4: IsBad* / SafeRead (E)
- IsBadReadPtr/IsBadWritePtr sites at HEAD: 0 (`grep -c IsBad` EquipCharPreview.cpp = 0, Equip3dLogic.h = 0). C3a already converted all 14. SafeRead.h included at line 24 (SceneView.h 25); 9 `saferead::probeRead(SiteEquip,...)` calls remain; `readAt` wrapper 121-124 (site SiteEquip). Nothing left to do for the SafeRead part of MG4. Remaining raw writes: `*reinterpret_cast<int*>(ctl+CtlFlagsOff)=` (313, 370), `*reinterpret_cast<void**>(border+BorderFillOff)=` (350).
- C: also 0 sites (the word only in the line-38 comment); local `readAt` uses saferead site 0 (107).

## 6. MG5 only (C)
- `leaderClient()` 382-387: `readAt AppGlobal 0x00a1b4a4 -> +4 -> +4 -> +0x270 = party; n = party[0] in 1..64; return ((int(__thiscall*)(int,int))FnPartyMember=0x007e5da0)(party, 0)`.
- Common equivalent: `party::table()` = same chain (each link nonzero); `party::member(table, 0)` = `gameaddr::PartyGetAt` 0x007E5DA0 = SAME engine fn (needs 0 < count). Use `party::member(party::table(), 0)`. Do NOT use `party::controlledCreature()`: it calls GetControlledCreature 0x007E5D80 (different fn; may differ from member 0) and would change which creature the page shows (SetStats uses member 0).
- Server creature: C 55 / 965 `FnClientToServer 0x0077d800` thiscall(client) == `party::serverCreature(member)` (MemberServerCreature 0x0077D800, Party.h 89). Keep `saferead::beginScope()` after the engine call.
- `clientValid(c)` 388 (vtable == ClientCreatureVftable) has no Common equivalent (`k2fx::plausibleCreature` is effect-list based): keep.
- Time/item: line 457 already `k2fx::currentTime(&day,&ms)`; 506 `k2fx::isItemAbility(r.spell)`. No local copies left.
- Includes (doc 32 sec. 4 finding 3): CharEffectsPanel.cpp lines 33 `"../_shared/EffectIcons.h"`, 35 `"../_shared/GameAddr.h"`, 36 `"../_shared/GuiKit.h"`, 37 `"../_shared/WindowHook.h"`, 38 `"../_shared/SafeRead.h"` -> plain `"EffectIcons.h"` etc. (ChareffDecode.h already uses plain "TextFit.h"). Add KpLog.h, IniKit.h, PanelSlots.h, Party.h. The WindowHook.h include is redundant once GuiKit's wheel is used (GuiKit.h includes it). The KPM mirror holds the same `../_shared/` lines, so verify the KPM build layout with the kotor2-kpm-install skill before dropping.
- `bound` flag: Slot field (317); `findSlot` requires it (340); claim prefers `!bound`; Bind sets false at 911, true at 931; Destroy 999. Stays in the payload (doc 32 row 49).

## 7. Existing common_contract_test.cpp
- E (282 lines): includes check.h, fake_wndproc.h, WindowHook.h, GameAddr.h, SafeRead.h, K2Addr.h, SceneView.h. main(): saferead_contract (58), windowhook_contract (96, `kp::WindowHook<EquipTestTag>`), sceneview_contract (191). Nothing for GuiKit/IniKit/PanelSlots/KpLog.
- C (285 lines): includes SafeRead.h, GameAddr.h, K2Addr.h, EffectIcons.h, GuiKit.h, WindowHook.h. main(): saferead_contract (60), effecticons_contract (87: k2fx::iconForSpell, isItemAbility, currentTime), guikit_contract (setRect/getExtent/imageFor/textObjOk/setText/setTextColor/setFill), windowhook_contract (WheelTag). Nothing for Party/IniKit/PanelSlots/KpLog/wheel g_wheel.
- To add (mirror 8aed2f5 sections 5-7): MG4 = IniKit splitIniLine/trim (+ hex decision), GuiKit pickScreen/syncScreen, g_wheel (touch/setRect/take/onEvent/enabled), bindByTag/bindFlags, PanelSlots (fresh flag, onEvict-before-reset, release), KpLog compile prelude. MG5 = party::table/member(0)/serverCreature (see _shared_src/party_test.cpp fakes) + the same GuiKit/IniKit/PanelSlots items.
- test.toml: both: default `cxxflags=["-Werror"]`; `[common_contract_test] cxxflags=["-I../_shared_src","-I../../research/companion_levelup/tools/parity/stub","-DGUIKIT_HOST","-DWINDOWHOOK_HOST","-Werror"]`; C also `[chareff_decode_test] -I../_shared_src -Werror -Wno-format-truncation`. ADD to common_contract_test: `-I$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common` (KpLog.h; as 8aed2f5). E default needs `-I../_shared_src` only if Equip3dLogic.h gains the IniKit include.

## 8. Registries (tests/static)
- subclassers.toml: E L21-27 and C L29-35 (above): remove both in the same commit as the wheel move (check_subclassers FAILs a listed file lacking install/restore patterns).
- raw_addresses.toml: E L59 `[files."patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp"]` addresses 0x00462320 0x0073F530 0x007645F0 0x0076D200 0x0077EC20 0x0085D430 0x008AA16A 0x008AA6F0 0x008ABA50 0x009A4AB0 (none belong to the bindCover/screen/Fn* group; list unchanged by MG4 unless the checker flags removed literals). C L32 addresses 0x0084D871 0x0084DEF0 0x0084FBB0 (unchanged by MG5; 0x007e5da0/0x0077d800/0x00a1b4a4 consts are not listed).
- gui_ids.toml: E L51-62 source "EquipCharPreview.cpp:49-50,59-60" + source_regex (`constexpr int NumButtons = 5;`, `constexpr int MaxMenu = 20;`, `constexpr int ExpectedControlId = 49;`, `TipBgId = ExpectedControlId \+ 3 \+ NumButtons \+ MaxMenu, TipId = TipBgId \+ 1;`) -- keep those lines verbatim (HEAD has them at 47, 48, 52, 53; the `source` line hint already differs, regexes are what must match). C L84-90 "CharEffectsPanel.cpp:64-66", regexes `constexpr int FirstId = 69, MaxRows = 14;` and `static_assert\(NumTags == 82,` -- keep.
- span_exceptions.toml: no entries for these two files.

## 9. Risk list
1. E parseFloats hex-guard differs from IniKit (do not alias blindly).
2. Both setCtlVisible are bit1-only; guikit::setVisible toggles 0x20 (breaks click-through labels).
3. PanelSlots hit-claim does not reset fields (E re-bind: destroyCreature + keep fields; C: explicit `*s=Slot()`); release() breaks prefix-order slot-index logging.
4. C leader = PartyGetAt(table,0), not GetControlledCreature.
5. cursorEngineF fallback: pass cfg screen size.
6. bindByTag does not beginScope: add after each bind.
7. E dirty pan WIP overlaps tickInput; start MG4 after it commits.
8. C `../_shared/` includes must be fixed in the same commit that adds KpLog/IniKit/PanelSlots/Party includes.
