# RV5 spec: action-hotbar fixes (RH1-RH5, RL23-RL31)
Mod + version bump: action-hotbar 0.1.5-dev -> 0.1.6-dev (`patch_manager_mods/action_hotbar_src/manifest.toml:4`, `ActionHotbar.cpp:44` `Version`; the load banner `action-hotbar %s loaded` at :1891 is the version log string). Add one bullet block at the top of `CHANGELOG.md` (new `## Unreleased` section under the title) naming RH1-RH4, RL23-RL31 in player words.
All paths below are relative to `patch_manager_mods/action_hotbar_src/`. Line numbers are from the tree at spec time; re-anchor by the quoted text.
Apply order: A) HotbarLogic.h pure helpers + hotbar_logic_test.cpp (RH1, RH2, RH4, RL23, RL24, RL26, RL29, RL30) -> B) HotbarConfig.h + config_roundtrip_test.cpp (RL25 readFile) -> C) ActionHotbar.cpp glue top to bottom -> D) manifest/CHANGELOG/PLAY rows. All edits in one commit (`git commit -- <paths>`).

## Changes

### RH1: Save with no snapshot -> empty bar (persave contract)
- Files/anchors: `HotbarLogic.h:595-608` (SnapSrc, snapStatus), `ActionHotbar.cpp:263` (`g_pendingSaveRetries`), `:1807-1827` (snapshotToSave), `:1855-1875` (restoreFromSave), `:2103-2125` (HUD-tick pending-snapshot block), `hotbar_logic_test.cpp:754`.
- Before (HotbarLogic.h):
```
inline const char* snapStatus(SnapSrc src, bool loaded) {
    if (src == SS_Absent) return "absent (live hotbar kept)";
```
- After (HotbarLogic.h; add right after snapStatus):
```
inline const char* snapStatus(SnapSrc src, bool loaded) {
    if (src == SS_Absent) return "absent (bar emptied)";
    ...unchanged...
}
// What a load does with the pick: absent -> empty profiles (documented contract, README:86); unreadable -> keep the live bar; else replace.
enum SnapAction { Snap_Keep, Snap_Empty, Snap_Replace };
inline SnapAction snapAction(SnapSrc src, bool readable) { return src == SS_Absent ? Snap_Empty : (readable ? Snap_Replace : Snap_Keep); }
```
- Before (ActionHotbar.cpp:1865-1869):
```
    if (pick.src == hb::SS_Absent) {
        KP_LOGV(chCfg, KPLOG_INFO, "%s", hb::fmtPersaveSource(why, hb::snapStatus(pick.src, false), pick.path).c_str());
        return;
    }
```
- After:
```
    if (hb::snapAction(pick.src, false) == hb::Snap_Empty) {   // no copy in the save folder: empty profiles, [General] kept (same path as a new game)
        replaceLiveProfiles(nullptr, hb::snapStatus(pick.src, false), pick.path, why);
        return;
    }
```
  (the `snap.Load` failure branch below stays: UNREADABLE keeps the live bar.)
- Pending-snapshot hardening (R4; without it a slow/overwritten deferral makes a real save load as an empty bar, and mergeSnapshot then drops ALL profiles):
  1. Time-based give-up. Before (ActionHotbar.cpp:263): `int g_pendingSaveRetries = 0;` After: `DWORD g_pendingSaveSince = 0;   // GetTickCount() when the deferral was armed; the HUD tick gives up after 10 s (was 60 HUD ticks = 0.2-1 s)`. Remove both `g_pendingSaveRetries = 0;` lines in snapshotToSave (1816, 1824) by replacing the two arming blocks with one helper defined directly above snapshotToSave:
```
// RH1: arm the deferred snapshot (save folder empty or not yet a directory). WARN when a still-pending one is replaced: that save never gets a copy.
static void armPendingSave(const char* dir, const char* why) {
    if (g_pendingSaveSnapshot) KP_LOGV(chCfg, KPLOG_WARN, "persave: %s: previous pending snapshot (%s) replaced", why ? why : "save written", g_pendingSaveDir.empty() ? "unresolved folder" : g_pendingSaveDir.c_str());
    g_pendingSaveDir = dir ? dir : ""; g_pendingSaveWhy = why ? why : "save written"; g_pendingSaveSnapshot = true; g_pendingSaveSince = GetTickCount();
}
```
  In snapshotToSave, Before (first arming block): `g_pendingSaveDir.clear(); g_pendingSaveWhy = why ? why : "save written"; g_pendingSaveSnapshot = true; g_pendingSaveRetries = 0; return;` (one statement per line in the file) After: `armPendingSave(nullptr, why); return;`. Before (second block): `g_pendingSaveDir = dir; g_pendingSaveWhy = why ? why : "save written"; g_pendingSaveSnapshot = true; g_pendingSaveRetries = 0; return;` After: `armPendingSave(dir, why); return;`. (The HUD tick clears `g_pendingSaveSnapshot` before it re-calls snapshotToSave, so a tick-driven re-arm never WARNs.)
  2. HUD tick (the two give-up tests): Before `} else if (++g_pendingSaveRetries > 60) {` (inside `if (!dir.empty())`) and `} else if (++g_pendingSaveRetries > 60) {` (the `else` of `if (!dir.empty())`). After both: `} else if (GetTickCount() - g_pendingSaveSince > 10000) {`.
  3. Flush before load. In restoreFromSave, directly after the `if (!dir || !*dir) {...return;}` folder check and before `hb::SnapPick pick = ...`:
```
    if (g_pendingSaveSnapshot) {   // RH1: a save made moments ago may still be waiting for its folder; write it before looking for a snapshot
        std::string pd = g_pendingSaveDir;
        if (pd.empty() && g_se.currentSaveFolder(cur, sizeof cur) > 0 && cur[0] && strcmp(cur, dir) != 0) pd = cur;   // never flush into the save being loaded (it would overwrite its snapshot)
        DWORD pat = pd.empty() ? INVALID_FILE_ATTRIBUTES : GetFileAttributesA(pd.c_str());
        if (pat != INVALID_FILE_ATTRIBUTES && (pat & FILE_ATTRIBUTE_DIRECTORY)) { g_pendingSaveSnapshot = false; g_pendingSaveDir.clear(); snapshotToSave(pd.c_str(), g_pendingSaveWhy.c_str()); }
    }
```
  (`cur` is the local `char cur[260]` already declared at the top of restoreFromSave; `<cstring>` strcmp is available in this TU, `rg -n "strcmp" ActionHotbar.cpp | head -2` to confirm, else use `std::string(cur) != dir`.)
  Logging: WARN `persave: <why>: previous pending snapshot (<dir>) replaced` (only on overwrite); the flush logs the normal `persave: <why>: snapshot ...` lines.
- Edge cases: "folder not recognised" (1859-1862) still keeps the live bar. New-game path (K2SE_NEW_GAME) already calls replaceLiveProfiles(nullptr,...), so mergeSnapshot(general, nullptr) is proven. A save whose snapshot CopyFileA failed at save time now loads empty (that is the documented contract); a deferral that outlives 10 s logs `persave: ... no snapshot` and loads empty too. `.load.bak` still receives the previous INI (RL26 rotates it), so a mistaken empty is recoverable. README:86 and CHANGELOG:50 stay as they are.
- Logging: unchanged lines from replaceLiveProfiles (`persave: <why>: replaced ok=1 profiles=0 ...`, `persave: <why>: absent (bar emptied): source=<path>`), both INFO.

### RH2: makeScale always sets sx/sy
- Files/anchors: `HotbarLogic.h:285-289` (makeScale); `hotbar_logic_test.cpp:417-419`; caller `ActionHotbar.cpp:1551` (no change).
- Before:
```
    Scale k; double c = aspectCorrection(w, h, rw, rh);
    if (c != 1.0) { k.sx = (double)w / rw; k.sy = (double)h / rh; k.c = c; }
    return k;
```
- After (also fix the comment above it to "Scale for the client: sx/sy = px per layout unit; c is the x squeeze (1.0 for 4:3); the identity only when a size is unknown."):
```
    Scale k;
    if (w > 0 && h > 0 && rw > 0 && rh > 0) { k.sx = (double)w / rw; k.sy = (double)h / rh; k.c = aspectCorrection(w, h, rw, rh); }
    return k;
```
- Edge cases: 800x600 -> sx=sy=1.0 (unchanged); unknown/negative sizes -> identity (test `iv` stays); ultrawide unchanged (same formula). Executor: `rg -n "makeScale|g_scale" ActionHotbar.cpp` and confirm nothing else assumes sx==1 for 4:3 (only unitRect consumers).
- Logging: unchanged (`H3: window ... client=1024x768 ... x correction 1.0000`).

### RH3: assignSlot rolls back a failed Save
- Files/anchors: `ActionHotbar.cpp:1296-1308` (assignSlot).
- Before:
```
    hb::Profile& d = g_cfg.profile(g_charKey);   // per-character profile; ...
    int frozen = hb::freezeEffective(d.slots[0], g_eff);
    ...
    bool ok = g_cfg.Save(IniPath);
    ...
    if (!ok) { ed.msg = "Saving the config failed (see log)"; return false; }
```
- After (backup BEFORE freezeEffective, which also mutates d):
```
    hb::Profile& d = g_cfg.profile(g_charKey);
    const hb::Profile backup = d;   // RH3: a failed Save must not leave the edit (or the duplicate it blanked) in g_cfg
    int frozen = hb::freezeEffective(d.slots[0], g_eff);
    ...
    if (!ok) { d = backup; ed.msg = "Saving the config failed (see log)"; return false; }
```
  (the early `return false` REFUSED paths above `profile()` need no change; none after it exits before Save.)
- Edge cases: `g_cfg.profile()` may have created an empty profile for a new character; restoring leaves that empty profile (harmless, serialises as an empty section; do not erase). No recomputeBound/refreshEffective on the failed path (already so). Copy of Profile is a plain value copy (strings, vector).
- Logging: unchanged (`edit: assign slot=.. save=FAILED ...` WARN + `edit: assign ... ok=0` value line).

### RH4: members with a live pending @lowhp heal are skipped (no duplicate slots)
- Files/anchors: `HotbarLogic.h` (after chooseTarget, :37), `ActionHotbar.cpp:604-617` (useOnMember), `:628-660` (targetedUse), new globals near `g_pendingReload` (:259). No heal-size guess and no new chooser: a member that already has a heal in flight is simply ineligible for the next press.
- Pure helper (HotbarLogic.h, after chooseTarget; `<cstdint>` is already included):
```
// RH4: a heal the hotbar sent that queue-hud may still hold. Live while younger than 5 s (wrap-safe) and the member's HP has not risen since the press (a rise = it resolved).
constexpr uint32_t PendingHealMs = 5000;
inline bool pendingHealLive(uint32_t now, uint32_t since, int hpNow, int hpAtPress) { return now - since < PendingHealMs && hpNow <= hpAtPress; }
```
- ActionHotbar.cpp globals (next to `g_pendingReload`):
```
struct PendHeal { uint32_t id = 0; DWORD since = 0; int hp = 0; };   // RH4: heals this mod sent, keyed by member object id (leader switches reorder party slots)
PendHeal g_pendHeal[3];
void notePendingHeal(uint32_t id, int hp) {
    int pick = -1;
    for (int i = 0; i < 3 && pick < 0; ++i) if (g_pendHeal[i].id == id) pick = i;   // pass 1: same member, update in place (no duplicate)
    for (int i = 0; i < 3 && pick < 0; ++i) if (!g_pendHeal[i].id) pick = i;        // pass 2: a free slot
    if (pick < 0) { pick = 0; for (int i = 1; i < 3; ++i) if ((int)(g_pendHeal[pick].since - g_pendHeal[i].since) > 0) pick = i; }   // pass 3: the oldest (wrap-safe)
    g_pendHeal[pick].id = id; g_pendHeal[pick].since = GetTickCount(); g_pendHeal[pick].hp = hp;
}
```
- useOnMember: add a final param `uint32_t* calledId = nullptr` (signature now `bool useOnMember(int idx, const party::Vitals* v, uint32_t clientItem, uintptr_t itemPtr, const char* why, uint32_t* calledId = nullptr)`); after the closing `KP_LOG(chTarget, "target: PerformUseItem(item %08x -> member %d id %08x) returned", ...)` line add `if (calledId) *calledId = memberId;`. The early `return true` refusal branches and `TARGET_FAIL` leave it 0. targetedUse's last line:
  Before: `    return useOnMember(idx, v, clientItem, itemPtr, "lowhp");`
  After:
```
    uint32_t calledId = 0; bool handled = useOnMember(idx, v, clientItem, itemPtr, "lowhp", &calledId);
    if (calledId) { notePendingHeal(calledId, v[idx].cur); KP_LOGV(chTarget, KPLOG_DEBUG, "target: lowhp pending heal noted member %d id %08x hp=%d", idx, (unsigned)calledId, v[idx].cur); }
    return handled;
```
- targetedUse trace loop, keep the member ids: Before
```
    for (int i = 0; i < n; ++i) {
        uint32_t mid = 0;
        if (v[i].client) saferead::readAt<uint32_t>(S_Target, v[i].client, Client_ObjectId, &mid);
```
  After:
```
    uint32_t mids[3] = {};
    for (int i = 0; i < n; ++i) {
        uint32_t& mid = mids[i];
        if (v[i].client) saferead::readAt<uint32_t>(S_Target, v[i].client, Client_ObjectId, &mid);
```
  (the following `KP_LOGV(... "target:   [%d] client=%08x id=%08x ...", ..., mid, ...)` line is unchanged; `n <= 3` because `party::members(v, 3)`.)
- targetedUse choose step. Before: `    int idx = hb::chooseTarget(c, n, 0, thr, fbSelf);` After:
```
    DWORD nowT = GetTickCount();
    for (int i = 0; i < n; ++i)
        for (PendHeal& p : g_pendHeal) if (p.id && p.id == mids[i]) {
            if (hb::pendingHealLive(nowT, p.since, v[i].cur, p.hp)) { c[i].eligible = false; KP_LOG(chTarget, "target: member %d id %08x has a pending heal (age %lu ms), skipped", i, (unsigned)p.id, (unsigned long)(nowT - p.since)); }
            else { KP_LOGV(chTarget, KPLOG_DEBUG, "target: pending heal cleared member id %08x", (unsigned)p.id); p.id = 0; }
        }
    int idx = hb::chooseTarget(c, n, 0, thr, fbSelf);
```
  `chooseTarget` and `slotEnabled` already honour `eligible`; fallbackSelf behaviour is unchanged (self is used only if self is still eligible). Keep the existing `lowest-HP below %d%%` log line (it reflects the adjusted choice).
- Edge cases: member Down/left -> `c[i].ok/down/eligible` already skip it; its stale entry expires in 5 s. mids[i]==0 (unreadable) -> no match. Tick wrap handled by unsigned subtraction. Save/load or area change: entries expire (<=5 s) and the id rarely matches; acceptable. Paused buffering longer than 5 s is NOT covered (the 5 s cap is the Opus decision; note it in one README target-modes sentence). Every wounded member pending -> idx -1 -> with fbSelf the heal falls back to self, else "nobody to use it on, skipped"; accepted. Queue-hud dispatch itself is untouched (no cross-mod API).
- Logging: `target: member N id X has a pending heal (age A ms), skipped` (default level, only when a pending entry is live); DEBUG `pending heal noted` / `cleared`.

### RH5: gate DEBUG line naming the raw top panel vtable (no behavior change)
- Files/anchors: `ActionHotbar.cpp:296` (GateInfo), `:312` (gateLog), `:324-334` (gateEval panel loop).
- GateInfo: add `uintptr_t rawTop = 0; uint32_t rawTopVt = 0, rawTopF48 = 0;`.
- gateEval, inside `for (int i = panelCount - 1; ...)` right after the `readAt ... bad panel ptr` line, before the `if (p == hud && ...)` line:
```
        if (i == panelCount - 1) { g.rawTop = p; g.rawTopF48 = (uint32_t)rdInt(S_Hud, p, Panel_Flags); uintptr_t vt = 0; saferead::readAt(S_Hud, p, 0, &vt); g.rawTopVt = (uint32_t)vt; }   // RH5: topmost panel even when its ignore bits hide it from the gate
```
- gateLog, as its first statement (independent of `same`/cap; logs only when the raw top changes):
```
    static uint32_t lastRawVt = 0xFFFFFFFFu; static uintptr_t lastRaw = 0;
    if (g.rawTop && (g.rawTopVt != lastRawVt || g.rawTop != lastRaw)) { lastRawVt = g.rawTopVt; lastRaw = g.rawTop; KP_LOGV(chGate, KPLOG_DEBUG, "GATE rawtop=%08x vt=%08x f48=%08x ignored=%d (gate open=%d why=%s)", (unsigned)g.rawTop, (unsigned)g.rawTopVt, (unsigned)g.rawTopF48, (int)((g.rawTopF48 & PanelIgnoredByInput) != 0), (int)g.open, g.why); }
```
- Code fix: NONE (do NOT swallow whenever g_hudGateOk, V4-13). Only if the PLAY row below shows `SWALLOW` in a dialogue add `inConversation` to KeyEvent in a follow-up.
- Logging: DEBUG only; needs `kplog.ini [action-hotbar] level=debug` for the PLAY row.

### RL23: Invoke=0 passes bound digits
- Files/anchors: `HotbarLogic.h:299` (KeyEvent), `:313-320` (decideKey), `ActionHotbar.cpp:1988` (KeyEvent fill), `HotbarConfig.h:250` comment, README Invoke line.
- KeyEvent: add `invokeOn = true` at the end of the bool list. decideKey, after the `pass(unbound)` branch:
```
            else if (!k.slotBound) a.decision = "pass(unbound)";
            else if (!k.invokeOn) a.decision = "pass(invoke off)";   // dry run: the key keeps its game meaning (party switch on 1-3)
```
- cpp (after `ke.slotBound = ...`): `ke.invokeOn = g_cfg.general.invoke != 0;`. HotbarConfig.h:250 comment -> `0 = log only (dry run); bound digits then pass through to the game`.
- Logging: decision string `pass(invoke off)` (DEBUG cap line).

### RL24: @self direct route requires EntryUsable; visible refusal
- Files/anchors: `HotbarLogic.h` (after useRoute, :162), `ActionHotbar.cpp:604-608` (useOnMember refusal), `:689-692` (drainRing), `updateHover` end (:1057), consts near :109.
- Pure helper (HotbarLogic.h; EntryUsable is in HotbarConfig.h, already included):
```
// RL24: item @self takes the direct route only when the HUD entry's bit0 (usable) is set; else the vanilla invoke reports why not.
// @lowhp is NOT gated: bit0 describes the controlled character, and useOnMember already asks the engine (canUseItem) about the TARGET member.
inline bool directRouteAllowed(UseRoute r, int entryFlags) { return r != UR_SelfDirect || (entryFlags & EntryUsable) != 0; }
```
- drainRing, Before:
```
        hb::UseRoute route = hb::useRoute(a, probe, g_cfg.general.invoke != 0);
        if (route == hb::UR_SelfDirect && selfUse(e, a)) continue;
        if (route == hb::UR_LowHpDirect && targetedUse(e, a, probe)) continue;
```
  After:
```
        hb::UseRoute route = hb::useRoute(a, probe, g_cfg.general.invoke != 0);
        if (!hb::directRouteAllowed(route, e.flags)) { KP_LOG(chTarget, "target: %s: @self entry not usable (flags %x), vanilla path reports why", sk.c_str(), (unsigned)e.flags); route = hb::UR_Invoke; }
        g_useRefused = false;
        if (route == hb::UR_SelfDirect && selfUse(e, a)) { if (g_useRefused) showRefusal(r.slot); continue; }
        if (route == hb::UR_LowHpDirect && targetedUse(e, a, probe)) { if (g_useRefused) showRefusal(r.slot); continue; }
```
  (UR_Invoke + Invoke=0 falls to the existing `if (g_cfg.general.invoke) invokeEntry` = nothing, plus the log line above.) Do NOT make useOnMember return false on a CanUseItem refusal (would arm portrait-pick for @self with 2+ members).
- Refusal feedback (new, file scope before drainRing; widgets are touched on the HUD tick, drainRing is called from there at :2166):
```
constexpr int RefuseTipId = 230;   // owner id of the transient "not usable" tooltip (>= CellTipBase, so updateHover's "no hover" hide leaves it)
bool g_useRefused = false; DWORD g_refuseUntil = 0;
void showRefusal(int slot) {
    if (slot < 0 || slot >= ButtonCount) return;
    showTipText(g_bar.b[slot].bg, "Not usable right now", RefuseTipId); g_refuseUntil = GetTickCount() + 1500;
    KP_LOGV(chTip, KPLOG_DEBUG, "tip: refusal shown slot %d", slot);
}
```
  useOnMember refusal branch: `KP_LOG(...CanUseItem refused...); g_useRefused = true; return true;` (declare g_useRefused ABOVE useOnMember: put the three globals + RefuseTipId next to `g_pendHeal`; showRefusal must come after showTipText, so define it right before drainRing and keep only the bool/DWORD globals early). updateHover, last line after the `no hover` hide: `if (g_bar.tipSlot == RefuseTipId && (int)(GetTickCount() - g_refuseUntil) >= 0) hideTip("refusal expired");`
- Edge cases: slot widgets absent -> showTipText already guards unbound tip labels. Hovering a slot replaces the tip after 450 ms (accepted). Click source and key source behave the same.
- Logging: `target: <slot key>: @self entry not usable (flags N), vanilla path reports why` (INFO); `tip: refusal shown slot N` (DEBUG).

### RL25: checkConfig reload failure on an existing file retries; stale flag; readFile checks ferror
- Files/anchors: `ActionHotbar.cpp:445-459` (checkConfig), `:1281` (assignSlot, after checkConfig(true)), `:1840-1848` (replaceLiveProfiles), `HotbarConfig.h:320-328` (readFile).
- checkConfig Before: `if (!filewatch::changed(g_cfgPoll, IniPath, GetTickCount(), force, &st)) return;` ... `if (!c.Load(IniPath)) { KP_LOGV(chCfg, KPLOG_WARN, "cfg: cannot read %s (mtime=%lld, keeping previous)", IniPath, mt); return; }`
  After:
```
    const filewatch::Stamp prevStamp = g_cfgPoll.last; const bool prevPrimed = g_cfgPoll.primed;   // before changed() consumes it
    if (!filewatch::changed(...)) return;
    ...
    if (!c.Load(IniPath)) {
        if (!st.exists) { KP_LOGV(chCfg, KPLOG_WARN, "cfg: cannot read %s (mtime=%lld, keeping previous)", IniPath, mt); return; }   // R7: a missing file is harmless (g_cfg stays, assignSlot's Save recreates it): no stale flag, no retry
        if (!g_cfgStale) KP_LOGV(chCfg, KPLOG_WARN, "cfg: cannot read %s (mtime=%lld, keeping previous, retrying)", IniPath, mt);
        else KP_LOG_EVERY(chCfg, 30000, "cfg: still cannot read %s (retrying every poll)", IniPath);
        g_cfgStale = true; g_cfgPoll.last = prevStamp; g_cfgPoll.primed = prevPrimed;   // un-consume: the next poll sees the change again and retries
        return;
    }
    g_cfgStale = false;
```
  Global `bool g_cfgStale = false;` beside `g_cfgPoll`. The stale flag and the stamp restore apply only when the file EXISTS but cannot be read (locked/permissions); a deleted INI keeps the old behaviour (WARN, keep `g_cfg`). Restoring the stamp (not `primed=false`) avoids the first-poll `writeTemplateIni()` firing.
- assignSlot: right after `checkConfig(true);` add
```
    if (g_cfgStale) { KP_LOGV(chEdit, KPLOG_WARN, "edit: assign REFUSED: %s unreadable (stale config), saving would overwrite it", IniPath); ed.msg = "Config file unreadable - not saving (see log)"; return false; }
```
- replaceLiveProfiles: in `if (ok) { g_cfg = n; ... }` add `g_cfgStale = false;`. snapshotToSave unchanged.
- readFile After: `bool bad = ferror(f) != 0; fclose(f); if (bad) { out->clear(); return false; } return true;`
- Edge cases: user deletes the INI: editing keeps working (stale stays false; the next `assignSlot` Save recreates the file). Locked/unreadable existing INI: editing refuses (message tells why) until a poll reads it again or a persave load rewrites it via replaceLiveProfiles. Locked-then-free file loads on the next 1 s poll. Tick wrap handled by filewatch.
- Logging: WARN once on the transition (existing file only), then INFO every 30 s.

### RL26: rotate `.load.bak` (keep 3)
- Files/anchors: `HotbarLogic.h` (small helper), `ActionHotbar.cpp:258` (paths), `:1840` (`CopyFileA(IniPath, LoadBakPath, FALSE)`).
- HotbarLogic.h: `inline std::string loadBakName(const char* base, int gen) { return gen <= 0 ? std::string(base) : std::string(base) + "." + std::to_string(gen); }`
- ActionHotbar.cpp before replaceLiveProfiles:
```
// RL26: keep the last 3 load backups (.load.bak newest, .load.bak.1, .load.bak.2): two quick loads no longer lose an unsaved bar edit.
void rotateLoadBak() {
    std::string b1 = hb::loadBakName(LoadBakPath, 1), b2 = hb::loadBakName(LoadBakPath, 2);
    DeleteFileA(b2.c_str()); MoveFileA(b1.c_str(), b2.c_str()); MoveFileA(LoadBakPath, b1.c_str());   // a missing source just fails
}
```
  Call `rotateLoadBak();` immediately before `BOOL bak = CopyFileA(IniPath, LoadBakPath, FALSE);`. Add one README sentence next to :86: "Bar edits only persist if you save the game afterwards; loading backs the old file up (last three kept)."
- Logging: none new (existing `persave: replaced ... bak=` flag).

### RL27: hide stale tooltips on scroll / mode change
- Files/anchors: `ActionHotbar.cpp:1313-1319` (scrollPicker), `:1509` (tab click), `:1511` (modeClick call).
- scrollPicker, after `ed.off = no;`: `if (g_bar.tipSlot >= CellTipBase && g_bar.tipSlot < ToggleTipId) hideTip("scrolled");`
- tab click, after `ed.tab = i;`: same hide (`"tab changed"`). Mode button: `{ if (g_bar.tipSlot == ModeTipId) hideTip("mode clicked"); modeClick(hud, ed.slot); return; }`.
- Toggle tip: already hidden by setMode (`:1263`, hides any tip >= CellTipBase when m != M_Pick); no change.
- Logging: `tip: hide slot N (scrolled|tab changed|mode clicked)` DEBUG (existing hideTip).

### RL28: picker snapshot throttle
- Files/anchors: `ActionHotbar.cpp:1420-1438` (edPicker); const near :123.
- Add `constexpr int PickerRefreshFrames = 20;   // picker list rebuild interval (stack counts / usable flags change slowly)`.
- Before: block starts `std::vector<Entry> all; GroupSnapshot snap(hud); for (...) {...} unsigned h = ...; if (h != ed.sig || g_frame - ed.lastRefresh >= 60) { ... }`
- After: wrap the build in `const bool due = ed.sig == 0 || g_frame - ed.lastRefresh >= PickerRefreshFrames;  if (due) { <same build loop and signature>; if (h != ed.sig) KP_LOG(...unchanged log...); ed.sig = h; ed.lastRefresh = g_frame; ed.list = all; }` (the `|| >= 60` copy test is replaced by the outer throttle; the copy always happens on a due frame). `setMode(M_Pick)` and the tab click already reset `ed.sig = 0` (and setMode `lastRefresh = -1000`), so opening and tab changes rebuild at once. Everything from `maxOff = hb::pickerMaxOff(...)` down is outside the `if (due)`.
- Logging: unchanged.

### RL29: modifier resync assigns; clearDown on pass
- Files/anchors: `ActionHotbar.cpp:1975-1979` (resync block), `HotbarLogic.h:310-322` (decideKey).
- cpp Before: `if ((g_ctrl && !c) || (g_alt && !a) || (g_shift && !sh)) { KP_LOG(... "cap: stuck modifier cleared (ctrl %d->%d ...", ...); g_ctrl = g_ctrl && c; g_alt = g_alt && a; g_shift = g_shift && sh; }`
  After: `if (g_ctrl != c || g_alt != a || g_shift != sh) { KP_LOG(chInput, "cap: modifier resync (ctrl %d->%d alt %d->%d shift %d->%d)", (int)g_ctrl, (int)c, (int)g_alt, (int)a, (int)g_shift, (int)sh); g_ctrl = c; g_alt = a; g_shift = sh; }` and fix the comment to "resync modifiers whose key event was missed (Alt-Tab, modal)". Risk: relies on GetAsyncKeyState being right right after focus return (assumed, V4-12); the new log line makes a wrong "set" visible.
- decideKey digit branch: in the `down` path set `a.clearDown = true;` first (a key-down that passes drops a stale "down was swallowed" flag; the cpp applies clearDown before markDown, so the swallow branch still ends marked), and in the swallow branch add `a.clearDown = false;`. Also in the `else a.decision = "pass(shift)";` arm set `a.clearDown = true`. Key-up behaviour unchanged.
- Test impact: existing key-down pass vectors assert swallow/enqueue/markDown only; key-up pass (`:448`) still has `!clearDown`.

### RL30: comments / WARN reword only
- `HotbarLogic.h:318` comment -> `// a bound key is swallowed while the HUD tick is fresh (hudActive): passing it on would switch party members (keys 1-3)`.
- `ActionHotbar.cpp:1986` trailing comment on `ke.ringFull = ...` -> same wording (`bound digits are swallowed while the HUD tick is fresh`).
- `ActionHotbar.cpp:2002-2008`: comment `the digit is not invoked` -> `the digit passes to the game`; WARN text `"gate: HUD tick stale %lu ms (limit %lu) with gate open, digit %02x not invoked"` -> `"... digit %02x passed to the game"`.
- No test pins these strings (`rg -n "not invoked" *.cpp` to confirm).

### RL31: wheel subscribe retry throttle; stance WARN reset
- Files/anchors: `ActionHotbar.cpp:1205-1219` (ensureWheelHook), `:2152-2155` (stance warned).
- ensureWheelHook Before: `if (!subscribed) { int32_t tok = g_guiInput.wheel_subscribe(hotbarWheelCb, nullptr); if (tok > 0) {...} } else {...}`
  After:
```
        static long nextTry = 0; static int fails = 0;
        if (!subscribed) {
            if (g_frame >= nextTry) {   // RL31: not every frame (the provider WARNs per attempt)
                int32_t tok = g_guiInput.wheel_subscribe(hotbarWheelCb, nullptr);
                if (tok > 0) { subscribed = true; KP_LOG(chInput, "wheel: subscribed to k2.guiinput (token %d, after %d failed attempts)", tok, fails); }
                else { ++fails; nextTry = g_frame + 60; if (fails == 1) KP_LOGV(chInput, KPLOG_WARN, "wheel: k2.guiinput wheel_subscribe returned %d, retrying every 60 frames (picker wheel scrolling off until it succeeds)", (int)tok); }
            }
        } else { ...unchanged... }
```
  Fallback to the local WheelHook after N failures is NOT added (the connected branch returning early is deliberate; double subclassing risk). Confirm `g_frame` is `long`-compatible (`rg -n "g_frame" ActionHotbar.cpp | head -3`).
- Stance: Before `static bool warned = false; if (!ok && !warned) { warned = true; KP_LOGV(... WARN ...); }` After: `if (ok) { if (warned) { warned = false; KP_LOGV(chSlots, KPLOG_DEBUG, "actionbar: stance/form read recovered"); } } else if (!warned) { warned = true; KP_LOGV(...WARN unchanged...); }`.

## Tests
- Unit (`hotbar_logic_test.cpp`, all inside `main` before `return kt::Report`; scoped run `python3 scripts/run_tests.py hotbar_logic_test`):
  - RH1: replace `:754` with `KT_CHECK_EQ(snapStatus(SS_Absent, false), "absent (bar emptied)");` add `KT_CHECK(snapAction(SS_Absent,false)==Snap_Empty && snapAction(SS_Absent,true)==Snap_Empty && snapAction(SS_New,false)==Snap_Keep && snapAction(SS_Legacy,false)==Snap_Keep && snapAction(SS_New,true)==Snap_Replace);`
  - RH2: line 419 -> `Scale i2 = makeScale(1600,1200,800,600); KT_CHECK(i2.sx==2.0 && i2.sy==2.0 && i2.c==1.0);` plus `unitRect(4,478,20,35,i2,r)` == {8,956,40,70}; `Scale i3 = makeScale(1024,768,800,600)`: sx==1.28 && sy==1.28 && c==1.0, unitRect(4,478,20,35) == {5,611,25,44}; 800x600 and `makeScale(-5,...)` identity cases stay.
  - RH3: `Config c; ActionRef p8 = mk(K_Spell,8,"P"), q9 = mk(K_Spell,9,"Q"); c.SetSlot("hero",0,2,p8,"o"); c.SetSlot("hero",0,4,q9,"o"); Profile bk = c.profile("hero"); moveDuplicates(c.profile("hero"),0,5,p8); c.SetSlot("hero",0,5,p8,"n");` expect slot 2 blank and slot 5 == p8; then `c.profile("hero") = bk;` expect `Same(slots[0][2],p8)`, `!slots[0][2].blank`, `slots[0][5].empty()`, `Same(slots[0][4],q9)`.
  - RH4: `Candidate m[3]` = {20/100,30/100,80/100 all ok,eligible}, thr 50, fallbackSelf false, selfIdx 0: all eligible -> 0; `m[0].eligible=false` -> 1; `m[0]` and `m[1]` ineligible -> -1; same with fallbackSelf true and self (index 0) ineligible -> -1, with self eligible and nobody wounded below thr -> 0. `pendingHealLive(10999,6000,50,50)` true; `(11000,6000,50,50)` false; `(10000,6000,51,50)` false; wrap `(0x100,0xFFFFFF00,50,50)` true.
  - RL23: `K(0x02,true)` hudActive+slotBound, `invokeOn=false` -> decision "pass(invoke off)", !swallow, !enqueue, !markDown; `invokeOn=true` -> "SWALLOW" unchanged. Order: `invokeOn=false` with slotBound=false -> "pass(unbound)".
  - RL24: `directRouteAllowed(UR_SelfDirect,0)==false`, `(UR_SelfDirect,1)==true`, `(UR_LowHpDirect,0)==true`, `(UR_LowHpDirect,1)==true`, `(UR_Invoke,0)==true`, `(UR_LogOnly,0)==true`.
  - RL26: `loadBakName("a.bak",0)=="a.bak"`, `(…,1)=="a.bak.1"`, `(…,2)=="a.bak.2"`.
  - RL29: `K(0x03,true)` with `downSwallowed=true`, hudActive, slotBound=false -> clearDown true, swallow false; same with ctrl=true -> clearDown true; bound swallow case -> clearDown false, markDown true; shift -> clearDown true.
- Unit (`config_roundtrip_test.cpp`, near `:381`): `{ std::string g3 = "x"; KT_CHECK(!Config::readFile(".", &g3)); }` (fopen on a directory succeeds on POSIX, fread sets ferror). Run `python3 scripts/run_tests.py config_roundtrip_test`.
- Other suites to re-run: `hotbar_log_test` (UNREADABLE string unchanged), `textures_test`, `common_contract_test` (all via `python3 scripts/run_tests.py hotbar`).
- PLAY rows (append to `patch_manager_mods/PLAY_TEST_QUEUE.md`, columns `id | source | mod@version | install | steps | pass signal | status | added`; install `level=debug` for AHR5; status `pending`; added 2026-10-23; source `doc 47 RV5`; mod `action-hotbar 0.1.6-dev`):
  - AHR1 (RH1): bind slot 1, save as A; load a save made before the mod (or delete `action_hotbar.ini` from a save folder), load A then B. Pass: `persave: load: absent (bar emptied)` + `replaced ok=1 profiles=0`, bar empty after B, A still shows its bar. Also: save then load the same slot within 1 s -> the bar is kept (no `previous pending snapshot ... replaced`, or if logged, no empty bar).
  - AHR2 (RH2): windowed 1024x768 (and 1600x1200), load a save. Pass: `H3: ... client=1024x768 ... x correction 1.0000`, bar at the bottom-left, frames square; editor/tooltip placed correctly.
  - AHR3 (RH3): make `action_hotbar.ini` read-only, bind slot 3 (`edit: assign ... save=FAILED`), clear read-only, bind slot 7. Pass: slot 3 still empty in the INI.
  - AHR4 (RH4): queue-hud on, two members hurt, @lowhp item slot, press twice within a second. Pass: the two `target: PerformUseItem(... member N ...)` lines name different members, with `has a pending heal` between them.
  - AHR5 (RH5): bind 1-3, talk to an NPC and pick replies with 1/2/3 in a voiced and a cutscene dialogue. Pass: reply selected, no `cap: ... SWALLOW` in the dialogue; `GATE rawtop=... vt=...` names the dialogue panel. FAIL (-> follow-up `inConversation` fix) if any SWALLOW.
  - AHR6 (RL23-RL31 batch): `Invoke=0` + bound slot 1 -> party switch still works (`pass(invoke off)`); @self item dimmed by HUD -> vanilla message or "Not usable right now" tip; @lowhp item still heals a wounded companion while the leader is at full HP; make the INI unreadable (lock or permissions) then bind (refusal message + `cfg: cannot read`, then recovers once readable); delete the INI in game then bind (still works, file recreated); edit, load A, load B -> `.load.bak` and `.load.bak.1` differ; picker wheel scroll with a still cursor hides the tip; log has no repeating `wheel_subscribe` lines.

## Tooling
- `python3 scripts/run_tests.py hotbar` (scoped; or the individual names above), then `scripts/pre_install_gate.sh --mod action_hotbar_src` (expect `GATE PASS`). No `_shared_src` file is touched, so `compat_check.py` is not needed (run `python3 scripts/compat_check.py action_hotbar_src/HotbarLogic.h` only if a shared header gets edited by mistake).
- Install is RV7 (needs user GO); do not hot-swap here.

## Risks / ordering
- RV5 has no dependency on RV1-RV4 and touches only action_hotbar_src; it shares no function with queue-hud/selfheal edits. RH4 relies on HP only (rise = resolved, 5 s cap); no cross-mod API.
- Same-function overlaps inside RV5: `useOnMember` (RH4 calledId param + RL24 `g_useRefused`) and `drainRing`/`targetedUse` (RH4, RL24): apply RH4 first, then RL24. `gateEval`/`gateLog` (RH5 only). `replaceLiveProfiles` (RH1 caller path, RL25 stale reset, RL26 rotate): one edit pass. `decideKey` (RL23, RL29, RL30): one edit pass.
- Deviations: (1) RL24 refusal feedback uses the existing tooltip widgets with a 1.5 s expiry instead of a new message call (no new engine address). (2) RL25 restores the previous poll stamp instead of `primed=false` (the latter would re-run the first-poll template write on a deleted INI). (3) RL31 does not add the local-WheelHook fallback. (4) RH4 uses "skip members with a live pending heal" instead of a heal-size guess; item heal amounts are not read. (5) RL24 gates only @self; @lowhp relies on `canUseItem` for the target member.
- Open: dialogue panel ignore bits are unknown until AHR5 runs (Open engine question 4).

## Review corrections applied
- R4: RH1 gets a 10 s time-based pending-snapshot give-up (`g_pendingSaveSince` replaces the tick counter), a WARN when a pending snapshot is overwritten (`armPendingSave`), a flush before `pickSnapshot` in restoreFromSave (never into the save being loaded), and the PLAY row "save then load within 1 s".
- R5: RH4 drops PendingHealPct/chooseTargetPending: a member with a live pending heal gets `eligible=false`; notePendingHeal is now a two-pass (same id, free slot, oldest) so no duplicates; RH4 unit vectors rewritten.
- R6: RL24 gates EntryUsable for @self only (`directRouteAllowed`), @lowhp left to canUseItem; unit vectors and log text updated; showRefusal kept for the CanUseItem branch.
- R7: RL25 sets `g_cfgStale` and restores the poll stamp only when the INI exists but is unreadable; a missing file keeps the old WARN and editing works; AHR6 wording uses an unreadable INI.
