# RV6 spec: queue-hud low items (RL4, RL7, RL9, RL10, RL13-RL16, RL20-RL22) + docs batch (V1-14, V2-14, V3-13)
Mod(s) + version bump: queue-hud 0.4.8-dev -> 0.4.9-dev (patch_manager_mods/queue_hud_src/manifest.toml:4, queue_hud.ini:1, QueueHud.cpp:1 header, QueueHud.cpp KPatchInit log string `"0.4.5-dev: kit=%d ..."`, doc 30 title).
Precondition: `rg -n '^version' patch_manager_mods/queue_hud_src/manifest.toml` must print 0.4.8-dev (RV1-RV3 applied). Otherwise stop. All anchors below are by function/text (line numbers shift after RV1-RV3).
Files: queue_hud_src/{QueueHudLogic.h, QueueHud.cpp, queue_hud_logic_test.cpp, common_contract_test.cpp, queue_hud.ini, manifest.toml}, _shared_src/GameAddr.h, companion_selfheal_src/common_contract_test.cpp, scripts/queue_hud_log_check.py (+test), docs 29/30, PLAY_TEST_QUEUE.md.
Apply order: A pure logic + tests, B .cpp glue, C ini/docs/version, D GameAddr alias. Tests `-Werror`: every new enumerator needs its `case` in `whyName(DropWhy)`.

## Changes

### RL15: parseIni BOM skip, inline `;` comment, rejected count (QueueHudLogic.h parseIniLine/parseIni; QueueHud.cpp loadIni/KPatchInit)
- Before (parseIniLine): `const char* vb = eq + 1; const char* ve = e; detail::trim(vb, ve);`
  After: `const char* vb = eq + 1; const char* ve = detail::findCh(vb, e, ';'); detail::trim(vb, ve);   // RL15: "key = v ; note"`
- Before (parseIni): `inline void parseIni(Config& c, const char* text) { if (!text) return; char buf[256]; const char* p = text; while (*p) {` ... `if (len < sizeof buf) {... parseIniLine(c, buf); }`
  After (complete):
```
// Returns the number of rejected lines (unknown key, bad value, no '=', or over 255 bytes); *firstBadLine = 1-based line of the first one (0 = none). A UTF-8 BOM at the start is skipped.
inline int parseIni(Config& c, const char* text, int* firstBadLine = nullptr) {
    if (firstBadLine) *firstBadLine = 0;
    if (!text) return 0;
    const unsigned char* u = reinterpret_cast<const unsigned char*>(text);
    if (u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) text += 3;   // short-circuit: u[1]/u[2] are read only after a matching u[0]/u[1]
    char buf[256];
    const char* p = text;
    int line = 0, bad = 0;
    while (*p) {
        const char* q = p;
        while (*q && *q != '\n') ++q;
        ++line;
        const unsigned len = static_cast<unsigned>(q - p);
        bool ok = false;
        if (len < sizeof buf) { memcpy(buf, p, len); buf[len] = 0; ok = parseIniLine(c, buf); }
        if (!ok) { ++bad; if (firstBadLine && *firstBadLine == 0) *firstBadLine = line; }
        p = *q ? q + 1 : q;
    }
    return bad;
}
```
- Update the comment above parseIniLine: `// ... inline "; note" after a value is ignored (RL15)` and replace its key list with the full 37 keys of the table in the "Docs" section (V1-14).
- QueueHud.cpp: before `void loadIni()` add `int g_iniRejected = 0, g_iniFirstBad = 0;` ; in loadIni replace `qh::parseIni(g_cfg, buf);` by `g_iniRejected = qh::parseIni(g_cfg, buf, &g_iniFirstBad);`.
  In KPatchInit right after the `KP_LOG(chMain, "0.4.9-dev: kit=%d ...` line add:
  `if (g_iniRejected > 0) QLOGV(chMain, KPLOG_WARN, "queue_hud.ini: %d line(s) not applied (unknown key, bad value or over 255 bytes), first at line %d; those keys keep their defaults", g_iniRejected, g_iniFirstBad);`
- Edge cases: CRLF (trim strips \r); no trailing newline; BOM + comment line; `log=` (empty) stays rejected; a `;` can never be part of a valid value (colours/numbers/words).
- Logging: one default-level WARN at startup, only when > 0.

### RL14: QJ signature covers all entry types (QueueHudLogic.h QueueSig/makeQueueSig/sigEqual; QueueHud.cpp journalTick)
- Keep `types[8]`/`nTypes` (fmtQj still prints 8 + `+N`, goldens unchanged). Add `uint32_t restHash = 0;   // RL14: FNV-1a of entry types 8..15 (0 when ne <= 8)` to QueueSig.
- makeQueueSig after the types loop (note `q[i]` for i < min(ne,kMaxEntries) must be readable):
```
    if (q && s.ne > 8) { uint32_t h = 2166136261u; const int cap = s.ne < kMaxEntries ? s.ne : kMaxEntries;
        for (int i = 8; i < cap; ++i) h = (h ^ static_cast<uint32_t>(q[i].type)) * 16777619u;
        s.restHash = h; }
```
- sigEqual: add `|| a.restHash != b.restHash` to the first condition. Update the comment above it.
- journalTick: `qh::QEntry q[8] = {}; for (... i < 8 ...)` -> `qh::QEntry q[qh::kMaxEntries] = {}; for (int i = 0; i < a.ne && i < qh::kMaxEntries; ++i) {...}`.
- Logging: no format change.

### RL13: combat -1 ignored in modeChanged/modeWhy (QueueHudLogic.h ModeState helpers; QueueHud.cpp modeEmit)
- Semantics: `now.combat < 0` (unreadable HUD flags) is "no information": never a change, never a reason. prev -1 -> now known is a change but not "edge"/"ended".
- Before: `return a.combat != b.combat || a.soft != b.soft ...` (modeChanged(prev,now)); `if (prev.combat != now.combat) return now.combat == 1 ? "edge" : "ended";`
  After: `return (b.combat >= 0 && a.combat != b.combat) || a.soft != b.soft ...` ; `if (prev.combat >= 0 && now.combat >= 0 && prev.combat != now.combat) return now.combat == 1 ? "edge" : "ended";`
- New pure helper (after modeWhy) so a line written during an unreadable frame does not erase the last known combat:
```
// The state modeEmit stores as "previous": a MODE line written while combat is unreadable keeps the last known combat value (RL13).
inline ModeState modeStored(const ModeState& prev, bool primed, const ModeState& now) {
    ModeState s = now;
    if (primed && now.combat < 0) s.combat = prev.combat;
    return s;
}
```
- modeEmit: replace `g_modePrev[k] = n; g_modePrimed[k] = true;` by `g_modePrev[k] = qh::modeStored(g_modePrev[k], g_modePrimed[k], n); g_modePrimed[k] = true;`.
- Result: sequence combat 1, -1, 1 writes zero MODE lines (was 2: why=ended, why=edge).

### RL20: hidden count in MODE; stale cur in engineHoldsAction (QueueHudLogic.h; QueueHud.cpp)
(a) hidden +N. buildSlots refactor: rename the body to `inline int buildSlotsN(const QEntry* e, int ne, const RNode* rn, int nrn, const RNode* cur, Slot* out, int cap)`, replace every `kMaxSlots` in the body by `cap`, change `detail::push(Slot out[kMaxSlots], ...)` to `Slot* out`, and keep the public wrapper:
```
inline int buildSlots(const QEntry* e, int ne, const RNode* rn, int nrn, const RNode* cur, Slot out[kMaxSlots]) { return buildSlotsN(e, ne, rn, nrn, cur, out, kMaxSlots); }
constexpr int kMaxSlotsAll = kMaxEntries + kMaxRound;   // every entry gives <= 1 slot, the first 0x3F up to kMaxRound more
// Slots the panel cannot draw (total qualifying slots beyond kMaxSlots); Cancel still clears them (RL20).
inline int hiddenSlots(const QEntry* e, int ne, const RNode* rn, int nrn, const RNode* cur) {
    Slot all[kMaxSlotsAll];
    const int n = buildSlotsN(e, ne, rn, nrn, cur, all, kMaxSlotsAll);
    return n > kMaxSlots ? n - kMaxSlots : 0;
}
```
- ModeState: add `int8_t hidden = 0;` (end of struct). modeChanged: `|| a.hidden != b.hidden`. fmtMode: after the `why=` write add `if (s.hidden > 0) { w.s(" hidden="); w.i(s.hidden); }` (hidden=0 lines are byte-identical to today).
- QueueHud.cpp: new global `int g_modeHidden[3];` next to `g_modeStrip`; reset `g_modeHidden[0] = g_modeHidden[1] = g_modeHidden[2] = 0;` in onHudDraw beside the `g_modeStrip[...] = "hidden:empty"` resets; drawLeader after `buildSlots(...)`: `g_modeHidden[0] = qh::hiddenSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr);` ; drawStrips after its `buildSlots(...)` (inside the per-k loop): `g_modeHidden[k] = qh::hiddenSlots(...)` same args; modeEmit after `n.banner = banner;`: `n.hidden = static_cast<int8_t>(g_modeHidden[k] > 99 ? 99 : g_modeHidden[k]);`.
- scripts/queue_hud_log_check.py: add `"hidden": r"\d+",` to FIELD_RE (MODE REQUIRED unchanged; extra keys are already accepted). scripts/test_queue_hud_log_check.py: add one golden `MODE m=0 combat=0 soft=- menu=0 paused=0 hostile=? party=? strip=shown cancel=shown banner=vanilla why=ui hidden=2` to ModeParseTests (parses, `rec["f"]["hidden"] == "2"`).
(b) stale cur. New pure fn (QueueHudLogic.h, after curActionPresent):
```
// The engine holds an action for this creature (RL20: a stale flagless cur node does not count): an executing use-item / cast-spell entry, a pending round node of
// sub-type 9/10, or a REAL (curReal) cur node of sub-type 9/10. A bare 0x3F is not proof.
inline bool holdsAction(const QEntry* e, int ne, const RNode* rn, int nrn, bool haveCur, int curSub, int aa8, int ab8, bool paused, uint32_t staleSince, uint32_t now) {
    for (int i = 0; e && i < ne && i < kMaxEntries; ++i) if (e[i].type == T_USE_ITEM || e[i].type == T_CAST_SPELL) return true;
    for (int i = 0; rn && i < nrn && i < kMaxRound; ++i) if (rn[i].subType == 9 || rn[i].subType == 10) return true;
    return curActionPresent(curReal(haveCur, curSub, aa8, ab8, paused, staleSince, now), curSub);
}
```
- QueueHud.cpp: Snapshot gets `uint32_t staleSince = 0;`. In gatherFor, in the memberK block before `activeNeedsSlot`: `s.staleSince = g_combat == 0 ? g_itemQueue[memberK].staleCurSince : 0;` (0 = old behaviour; in combat staleCurSince is frozen, so it is not used there). engineHoldsAction body becomes:
```
    return qh::holdsAction(s.e, s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries, s.rn, s.nrn, s.haveCur, s.haveCur ? (int)s.cur.subType : -1,
                           s.aa8, s.ab8, s.paused, s.staleSince, GetTickCount());
```
- Edge cases: paused (flags ignored, node counts as before), leader switch (staleSince of the owning buffer), combat (staleSince 0).
(c) Down companion strip hidden, (d) detached icons in hard combat (RL22): docs only, see Docs.

### RL7: present test matches the node to the active action once it ran (QueueHudLogic.h; QueueHud.cpp tickItemQueues)
- Deviation note: effect is bookkeeping/label only. After the action ran, the dispatch gates (gateNe, nrn, creal) still hold the next buffered command behind a foreign node (doc 30 QH4, by design); RL7 only lets `buf.active` resolve EXECUTED instead of sitting in AV_PRESENT.
- New pure helpers (after holdsAction):
```
namespace detail {
inline bool idMatches(const QueuedAction& a, bool spell, uint32_t id) {   // item ids: low 24 bits (client id = server id | high bit, GameAddr ServerIdMask)
    return spell ? (a.kind == ActionKind::Spell && id == a.spellId) : (a.kind == ActionKind::Item && (id & 0xFFFFFFu) == (a.clientItem & 0xFFFFFFu));
}
inline bool nodeIsAction(const QueuedAction& a, const RNode& n) {
    return (n.subType == 9 && idMatches(a, true, n.spellId)) || (n.subType == 10 && idMatches(a, false, n.itemId));
}
}  // namespace detail
// RL7 (V2-11): is the engine holding OUR active action? matchOnly=false (the pre-RL7 test): any use/cast entry, any pending 9/10 node, a real cur 9/10.
// matchOnly=true (buf.sawExecuting: it already ran): only the entry/node carrying the action's own spell id / item id counts, so a foreign
// selfheal/AI node queued behind the finished cast stops keeping the verdict AV_PRESENT (BG23).
inline bool actionPresentIn(const QueuedAction& act, bool matchOnly, const QEntry* e, int ne, const RNode* rn, int nrn, bool creal, const RNode* cur) {
    for (int i = 0; e && i < ne && i < kMaxEntries; ++i)
        if ((e[i].type == T_USE_ITEM || e[i].type == T_CAST_SPELL) && (!matchOnly || detail::idMatches(act, e[i].type == T_CAST_SPELL, e[i].param0))) return true;
    if (cur && creal && (cur->subType == 9 || cur->subType == 10) && (!matchOnly || detail::nodeIsAction(act, *cur))) return true;
    for (int i = 0; rn && i < nrn && i < kMaxRound; ++i)
        if ((rn[i].subType == 9 || rn[i].subType == 10) && (!matchOnly || detail::nodeIsAction(act, rn[i]))) return true;
    return false;
}
```
- QueueHud.cpp tickItemQueues: replace the block from `bool actionInQueue = false;` through the `for (int i = 0; i < s.nrn; ++i) { if (s.rn[i].subType == 9 || ... actionInQueue = true; }` loop by (keep the `hasFollowLeader` scan of 0x3D):
```
        bool hasFollowLeader = false;
        for (int i = 0; i < s.engineNe; ++i) if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader
        const int engNe = s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries;
        const bool actionInQueue = qh::actionPresentIn(buf.activeAction, buf.active && buf.sawExecuting, s.e, engNe, s.rn, s.nrn, creal, s.haveCur ? &s.cur : nullptr);
```
  If RV2 already extracted this block into a helper (RQ4 orphan / RQ7), keep its structure and only thread the `matchOnly` argument through it. Apply RL7 AFTER RV2's edits to this block.
- When buf is idle (`!buf.active`) matchOnly is false, so idle/inactive behaviour is unchanged.
- Logging: none new (VANISH kind=executed already logs).

### RL4: party generation change -> drop-all (QueueHudLogic.h; QueueHud.cpp tickItemQueues)
- Logic: add `DR_PARTY_CHANGED` and `DR_MEMBER_DOWN` after the LAST `DropWhy` enumerator present when you apply this (RV1/RV2 may have appended DR_REFUSED/DR_PRESENT_TIMEOUT); `whyName(DropWhy)`: `"party_changed"`, `"member_down"`; extend the fmtDrop comment list.
```
// RL4 (V1-9): was the whole party re-created (save load / area change)? prev/now = the three members' SERVER creature pointers (0 = empty slot).
// True only when both sides hold a pointer and the non-zero sets are disjoint; a rotation, join/leave (sets overlap) or an unreadable table (now all 0) is false.
inline bool partyGenerationChanged(const uintptr_t prev[3], const uintptr_t now[3]) {
    bool anyPrev = false, anyNow = false;
    for (int i = 0; i < 3; ++i) {
        if (now[i]) anyNow = true;
        if (!prev[i]) continue;
        anyPrev = true;
        for (int j = 0; j < 3; ++j) if (now[j] == prev[i]) return false;
    }
    return anyPrev && anyNow;
}
```
- QueueHud.cpp: globals next to `g_identLostMs`: `uintptr_t g_prevSrv[3] = {0, 0, 0};`. In tickItemQueues, right after `if (!tbl) return;` and BEFORE the "Identity pre-pass":
```
    {   // RL4: party re-created (load / area): every buffered pointer is dead; drop now instead of the 20 s identity-lost wait
        uintptr_t nowSrv[3] = {0, 0, 0};
        for (int k = 0; k < 3; ++k) { const uintptr_t c = party::member(tbl, k); nowSrv[k] = c ? party::serverCreature(c) : 0; }
        if (qh::partyGenerationChanged(g_prevSrv, nowSrv)) {
            int dropped = 0;
            for (int k = 0; k < 3; ++k) {
                if (g_handoff[k].pending) endHandoff(k, qh::DR_PARTY_CHANGED);
                if (bufBusy(g_itemQueue[k])) { ++dropped; traceDropAll(k, qh::DR_PARTY_CHANGED); g_itemQueue[k].clear(); }
                discardDetached(k, "party_changed", true);   // RV3 signature (default arg on the forward declaration); true = default-level line, not just a trace
                g_flagGate[k].reset(); g_kickLast[k] = 0; g_downSince[k] = 0; g_identLostMs[k] = 0; g_identLastTick[k] = 0; g_prevRiWhy[k] = g_prevDsWhy[k] = -1;
                g_roundOrder[k] = RoundOrder(); g_cpend[k] = CPending(); g_handoffHold[k] = false; g_heldStrip[k] = HeldStrip();   // R7: stale pointers / icons from the old party
            }
            g_heldLeader = Held();   // R7: dead creature pointer in Held.creature, stale leader icons for 400 ms
            if (g_logOn && g_cfg.traceQueue) QTRACE("PARTY_GEN old=%08x,%08x,%08x new=%08x,%08x,%08x dropped=%d", (unsigned)g_prevSrv[0], (unsigned)g_prevSrv[1], (unsigned)g_prevSrv[2], (unsigned)nowSrv[0], (unsigned)nowSrv[1], (unsigned)nowSrv[2], dropped);
            if (dropped) QLOG(chMain, "party changed (load / area): dropped %d buffered member queue(s)", dropped);
        }
        if (nowSrv[0] || nowSrv[1] || nowSrv[2]) for (int k = 0; k < 3; ++k) g_prevSrv[k] = nowSrv[k];   // a transient all-zero read keeps the baseline
    }
```
- R7 analysis: `partyGenerationChanged` is false for leader switch (rotation), join/leave and a transient partial read; true only for a full disjoint swap (load / area re-creation), which scripts do not do while the PC persists. All the globals reset above are declared before `tickItemQueues` (RoundOrder :1224, CPending, HeldStrip/Held, g_handoffHold); if RV3 reordered declarations, forward-declare nothing: they are plain globals above the function.
- Not tracked during combat/menu (tick returns earlier): a load from a menu frame is caught on the first normal tick after it. A PC that survives a transition (sets overlap) falls back to the existing 20 s identity-lost drop (OPEN 1).
- Deviation: `WAIT why=dialogue` NOT implemented. No verified conversation flag exists in the repo (only npc_diversity_src/DiversityAnim.h:122 `server+0x54 dialog partner oid`, marked "unverified"). Left as PLAY row QH-RL4b; if the row shows a dispatch during dialogue, open a WI to log `server+0x54` at dispatch (do not gate on it unverified).

### RL9: hold serverCast while the member is Down (QueueHud.cpp mayDispatch + tickItemQueues; QueueHudLogic.h)
- Before (mayDispatch): `return qh::ownerMayDispatch(slotOfCreature(tbl, a.creature), g_cfg.companionServerCast && qh::serverRouteEligible(a));`
  After:
```
    const int slot = slotOfCreature(tbl, a.creature);
    if (!qh::ownerMayDispatch(slot, g_cfg.companionServerCast && qh::serverRouteEligible(a))) return false;
    if (slot <= 0) return true;   // the leader's client route is unchanged
    const uintptr_t cli = party::member(tbl, slot);
    return !memberDown(slot, cli ? party::serverCreature(cli) : 0, GetTickCount());   // RL9: a Down companion neither casts nor re-injects (RI_NOT_CTRL / DS_NOT_CTRL); revival releases the hold
```
  (memberDown is defined earlier in the file: no forward declaration needed.)
- Logic: `constexpr uint32_t kDownHoldMs = 20000;` and `inline bool downHoldExpired(uint32_t since, uint32_t now) { return since != 0 && now - since >= kDownHoldMs; }` (wrap-safe unsigned subtraction).
- QueueHud.cpp: global `DWORD g_downSince[3] = {0, 0, 0};` next to g_identLostMs. In the per-k loop, after `if (skip[k]) continue;` and before `const bool ownerCtrl`:
```
        if (k > 0 && bufBusy(buf) && memberDown(k, srv, now)) {   // RL9: bounded hold (an invisible stale queue must not fire minutes later)
            if (g_downSince[k] == 0) { g_downSince[k] = now; if (g_logOn && g_cfg.traceQueue) QTRACE("DOWN_HOLD m=%d cmd=%u", k, (unsigned)bufHead(buf).cmdId); }
            else if (qh::downHoldExpired(g_downSince[k], now)) { traceDropAll(k, qh::DR_MEMBER_DOWN); buf.clear(); g_downSince[k] = 0; continue; }
        } else g_downSince[k] = 0;
```
- Deviation/addition vs doc 47: the 20 s wall-clock ceiling (paused time counts) is new; without it a Down companion's invisible buffer (strip hidden by design, 0.4.1) could fire at an arbitrary later time.
- Engine unknown (OPEN 2): value of CrCommandable while Down; stale-target / used-up-item revalidation (rest of V2-15) is NOT done (PLAY only).

### RL10: unreadable engine queue at click (QueueHud.cpp, both hooks)
- Finding: `fmtCmd` already prints `engq=unreadable` when `engq < 0`, and the hooks pass -1, so the cause is in the CMD line today; what is missing is a default-level record. Deviation from "buffer when empty": rejected. With an empty idle buffer, DISPATCH_NOW hands the click to the engine through the client route, which works without our read; buffering would park the command behind a creature the tick cannot gather (it would sit until identity-lost / null-creature drop).
- New static helper before `QueueHudPerformUseItem`:
```
// RL10: the engine queue could not be read at click time (server creature unresolved: load / area change). The decision stays decideAction's (empty buffer = dispatch_now); leave a default-level record.
static void noteUnreadableClick(int k, uint32_t cmd, qh::ActionDecision dec) {
    QLOGV(chMain, KPLOG_WARN, "click with unreadable engine queue m=%d cmd=%u dec=%s", k, (unsigned)cmd, dec == qh::DECIDE_DISPATCH_NOW ? "dispatch_now" : dec == qh::DECIDE_BUFFER ? "buffer" : "drop_full");
}
```
  In both hooks directly after `const qh::ActionDecision dec = qh::decideAction(...);` add `if (gateQ < 0) noteUnreadableClick(memberK, cmd, dec);`.
- Tests: none pure (hook glue); PLAY check only.

### RL16: drop_full cue (QueueHud.cpp, both hooks)
- A helper exists: `guikit::playHoverSound()` (_shared_src/GuiKit.h, pinned by queue_hud_src/common_contract_test.cpp "keep their signatures"): idx 1 mouse-enter tick, false when the sound manager is null. It is the only sound helper (no dedicated "denied" sound); used as the cue.
- Before (item hook): `QLOGV(chMain, KPLOG_WARN, "item buffer full for member %d, dropped item %08x cmd=%u", memberK, itemId, (unsigned)cmd);`
  After: `const bool snd = guikit::playHoverSound(); QLOGV(chMain, KPLOG_WARN, "item buffer full for member %d, dropped item %08x cmd=%u sound=%d", memberK, itemId, (unsigned)cmd, snd ? 1 : 0);`
  Spell hook: same, text `"action buffer full for member %d, dropped spell %08x cmd=%u sound=%d"`.
- Edge: human click rate only; the click is still consumed (return 1). Not played in combat (hooks pass through first).

### RL21: FlagGate comment (QueueHudLogic.h kFlagBusyCapMs block)
- Replace the comment `// ---- QL6b flag gate: after an item/spell the engine keeps aa8/ab8 set for up to ~0.7 s and a dispatch in that window is wiped. ----` with:
  `// ---- QL6b flag gate: aa8 (busy window) clears within ~1.5 s, ab8 (round lock) is cleared only by EndCombatRound, up to ~3 s of UNPAUSED time (doc 29 D1/D2); a dispatch while either is set is wiped. The 2 s cap can therefore release while ab8 is still set (hold=flag_cap); the other gates (cur, nrn) still hold. Real ab8 lifetime out of combat is unmeasured (QH-RL21 row). ----`
- QueueHud.cpp dispatch log: `QLOG(chMain, "dispatch hold=flag_cap member=%d cmd=%u aa8=%d ab8=%d", ...)` -> append ` held=%u` with `(unsigned)(now - g_flagGate[k].holdSince)` (the gate resets AFTER this line; if RV2/RL3 renamed `holdSince`, use its replacement or drop the field). No cap change.

### RL22 + RL20(c): docs only (doc 30)
- QH4e checklist item 4 gets: `Exception (intended, QL11): with handoff_detach_held=1 the detached round-node icons are drawn in hard combat (drawDetachedSlots, leader only); expect "ROUND_DETACH ... act=detach" and the icon, not a hidden panel.` Add the same one-line comment above the `drawDetachedSlots(hud, s, n, now);` call in drawLeader.
- Doc 30 design note: a Down (HP<=0) companion keeps its buffer but its strip and Cancel are hidden by design (0.4.1); since RL9 the buffer is held and dropped after 20 s (`DROP why=member_down`).

## Docs batch (V1-14, V2-14, V3-13)
- QueueHudLogic.h comments: (1) slotStyle (`// Slot 0 while waiting (and waitingDim) is dimmed: icon, background ...`) -> `icon alpha scales only when waiting_icon_dim=1; background, border and frame always scale`. (2) stripHeadSize comment -> `... never taller than the vit bar, except that it never drops below the strip icon size when the vit bar is shorter`. (3) Config: `stripIconPx // 0 = auto (60% of the vit-rect height, min 12)` -> `// 0 = auto: left = 60% of the vit height clamped 48..72, above/below = 60%, min 12`; `#include "SelfHealUi.h"` comment -> `// shui::stripReserve / buttonRect: fallback footprint of the neighbouring selfheal button (left-strip reserve)`. (4) parseIniLine comment: full key list below.
- QueueHud.cpp:1 header: `// Queue HUD 0.4.9-dev (green out-of-combat action queue ...` (keep the rest of line 1); replace the `hook H8 0x0074C168 ...` line by two lines: `hook H9 0x00775603 PerformUseItem / H10 0x00775703 PerformCastSpell (hooks.toml): out-of-combat use/cast on a party member: buffer (return 1) or track (dispatch_now)` and `HUD capture (hud pointer + combat bit): hud-host K2HUD_FRAME (old hook H8 0x74C168 migrated, HL4)`. Replace `The only engine write is the Cancel call.` by `Engine calls: ClearAllActions (Cancel, hand-off clear-first), the replayed PerformUseItem/PerformCastSpell, AddUseTalentOnObjectActions (QL20 server route), the follower-starvation rotate (ActListRemoveHead/AddTail), round-node detach/reattach.` Stale comment before `const bool keepNow` in handoffTick (`Combat start wipes the engine queue; the first send clears this member's creature ...`) -> `Combat start wipes the engine queue. Default (handoff_keep_new=1, handoff_keep_unpaused=1): no clear, the player's pending round nodes are remembered (RoundOrder). The clear-first send (ServerClearAllActions) only runs with keep_new=0, or with keep_unpaused=0 when combat starts unpaused.` (do this one last; RV3 edits the same block).
- V2-14 naming (_shared_src/GameAddr.h): replace the `UseItemRouteA` line with
  `constexpr uintptr_t AddUseTalentOnObject = 0x0057D8B0;   // CSWSCreature::AddUseTalentOnObjectActions thiscall(server creature; talentType, talentId, targetServerId, byte, itemServerId (0x7F000000 none), propIdx (-1 none), byte, byte) RET 0x20, 0 = refused (commandable gate 0057d8e3); companion-selfheal route A (item: 0,0,self,0,item,0,0,0) and queue-hud QL20 serverCast (spell)`
  `constexpr uintptr_t UseItemRouteA = AddUseTalentOnObject; // legacy alias (companion-selfheal, perf_lab smoke_selfheal, selfheal contract test)`
  QueueHud.cpp: `AddrUseTalentOnObject = gameaddr::AddUseTalentOnObject`. Keep CompanionSelfHeal.cpp / smoke_selfheal.cpp untouched (alias). Pins: queue_hud_src/common_contract_test.cpp add `QEQ("gameaddr::AddUseTalentOnObject is AddUseTalentOnObjectActions 0x0057D8B0 (doc 27 R1 route A, QL20)", (unsigned long long)gameaddr::AddUseTalentOnObject, 0x0057D8B0ULL);` next to the ServerClearAllActions pin; companion_selfheal_src/common_contract_test.cpp:123 keep the UseItemRouteA pin and add `gameaddr::UseItemRouteA == gameaddr::AddUseTalentOnObject`.
- RV2 premise note (one line, doc 29/30 and the RV2 ORPHAN text): `CServerExoApp::ClearAllActions 0x51CB10` (queue-hud's Cancel, `GameAddr::ServerClearAllActions`) DOES empty the round list (RemoveAllActions 0x590430 at 0x51CB47); only the object method `CSWSObject::ClearAllActions 0x541080` does not, so an orphan round node is expected after a keyboard Esc / quickbar press or a script clear, NOT after queue-hud's Cancel. Wherever doc 29/30 (or the RV2 orphan comment) says "ClearAllActions never empties the round list", add "(object method 0x541080 only)".
- V2-14 ClearAllActions disagreement (resolved from 30_queue_hud_evidence/RE.md:86): object method `CSWSObject::ClearAllActions 0x541080` never empties the round list (doc 29 D6, PQ2 4.4); the app-level `CServerExoApp::ClearAllActions 0x51CB10` (what Cancel and GameAddr::ServerClearAllActions call) does (it calls RemoveAllActions 0x590430 at 0x51CB47). Edit doc 29 D6 text `ClearAllActions never empties the round list` -> `CSWSObject::ClearAllActions 0x541080 never empties the round list (the app-level 0x51CB10 does)`; doc 30 QL2 D2 add `(app-level 0x51CB10; the object method 0x541080 does not, doc 29 D6)`.
- V3-13 queue_hud.ini: line 1-2 header -> `; queue_hud.ini -- Queue HUD 0.4.9-dev. Optional: copy next to swkotor2.exe. Every key is optional. Commented lines (;key = v) show the built-in default; uncommented lines are active and repeat it EXCEPT waiting_icon_dim = 0 (built-in 1) and strip_head_pct = 150 (built-in 100).` + `; Colours are "r,g,b" floats 0..1, alphas 0..1, border_px a single digit 1..6. A bad value, unknown key or line over 255 bytes is skipped (default stays) and counted in one startup WARN; a trailing "; note" after a value is allowed.` ; strip_icon_px comment -> `; Icon edge in px. 0 = auto: left side = 60% of the health-bar height clamped to 48..72 (about 58 px at 3840x1600); above / below = 60% of the health-bar height; never below 12. Otherwise 8..96.` ; the `pause_reinject` / `pause_eaten_wait` comments are correct only once RV1 RQ5 is in (verify `reInjectWhy` honours both; RV1 also rewrites the `spell_replay_pos` comment, do not touch).
- V3-13 manifest.toml: version 0.4.9-dev; description sentence `One capture hook on the HUD update plus portrait-slots HUD_DRAW / FRAME_BEGIN callbacks; the only engine call is the Cancel click.` -> `Two hooks (PerformUseItem 0x775603, PerformCastSpell 0x775703) buffer out-of-combat use/cast commands; HUD capture comes from hud-host K2HUD_FRAME, drawing from portrait-slots HUD_DRAW / FRAME_BEGIN. Engine calls: ClearAllActions (Cancel, hand-off), the replayed Perform* calls, AddUseTalentOnObjectActions (companion Force powers) and the queue rotate behind a follower's 0x3D.`; replace the trailing key list `(enabled, kit, ... strip_max)` by `(see doc 30 key table)`.
- Doc 30: title -> `(queue-hud 0.4.9-dev)`; add a RESUME HERE bullet first: `0.4.9-dev (RV1-RV6, doc 47 review fixes): built, install pending (RV7).` Append section `## 2026-10-23 QL10-QL20 summary (0.4.x)` with one bullet each, taken from doc 33 rows QL9-QL20 (do not re-research): QL9 spell replay pos (fixed by RV1 pos/flag), QL10 keep-new / detach_held / RoundOrder (combat click order), QL11 detached icons (drawDetachedSlots), QL12-QL14 MODE trace + analyzer flags L/M/N, QL16 CSTATE (cross-mod), QL17 B1 owner hold (DS_NOT_CTRL) + strip head (strip_head_pct) + waiting_icon_dim, QL18 whole-ini reader, QL19 Cancel button tint, QL20 companion serverCast (`SRV_CAST`) + `kickStarved` (`SRV_KICK`), then `Review 47 (RV1-RV6): DROP reasons party_changed / member_down, lines PARTY_GEN / DOWN_HOLD, MODE hidden=N.` Then `### Ini keys (generated from parseIniLine + Config, 0.4.9-dev)` as a table `Key | Default | Values`, from this list (37 keys; shipped-ini overrides in brackets):
  - bool, default 1: enabled, kit, cancel, waiting_dim, [waiting_icon_dim: built-in 1, shipped 0], log, trace_queue, trace_clicks, trace_mode, companions, pause_reinject, pause_eaten_wait, dispatch_wait_flags, handoff_on_combat, handoff_clear_first, handoff_keep_new, handoff_keep_unpaused, handoff_detach_held, handoff_resend_held, companion_server_cast, hide_banner_soft, hide_redbar_soft, show_in_combat, spell_replay_pos (obsolete after RV1: accepted, ignored). Bool values 0/1/true/false/on/off/yes/no.
  - float 0..1: frame_alpha 0.9, bg_alpha 0.45, border_alpha 0.9, waiting_alpha 0.6. rgb "r,g,b" each 0..1: bg_rgb 0.05,0.25,0.28; border_rgb 0.10,0.58,0.62; waiting_dim_rgb 0.03,0.15,0.18.
  - int: border_px 2 (single digit 1..6); strip_icon_px 0 (0 = auto, else 8..96); strip_side left (left|above|below); strip_max 4 (single digit 1..4); strip_head_pct 100 [shipped 150] (100..200, clamped, capped at vit height); mode_cap 2000 (1..100000).
- Doc 29: line 3 `Status 2026-10-04: static RE done ... Probe cmdq-probe DESIGNED, not built.` -> `Status 2026-10-23: the indicator and queue fixes shipped as queue-hud (doc 30); static RE (PQ1/PQ2) still valid; cmdq-probe BUILT (HK1), not installed.` ; heading `## Follow-up (not started)` -> `## Follow-up (DONE: queue-hud, doc 30)`.
- Not done (OPEN): V3-13 "open checklists have no results" (move to PLAY rows) is left to the owner; doc 30 body sections 36-71 still describe 0.2.0 behaviour.

## Tests
Unit (queue_hud_logic_test.cpp; register each new function in main after `testServerRouteQL20();`; `python3 scripts/run_tests.py queue_hud_logic_test`):
- testRL15Ini: `parseIniLine(c,"log=0 ; quiet")` true and `!c.log`; `"bg_alpha = 0.5 ;"` true/0.5; `"border_px=3 ; x"` -> 3; `"log=;"` false; `"; only"` true. `Config a; parseIni(a,"\xEF\xBB\xBF" "enabled=0\r\nlog=0 ; x\r\n")` returns 0, `!a.enabled && !a.log`. `int fb; parseIni(d,"garbage line\nunknown=5\nborder_alpha=0.1\nborder_px=99\nborder_px=6\nwaiting_dim=0",&fb)` == 3, fb == 1, borderAlpha 0.1, borderPx 6 (existing vector). Overlong: `std::string(300,'a') + "\nlog=0"` -> returns 1, fb 1, `!log`. `"\xEF\xBB"` alone -> returns 1. nullptr -> 0. Existing `parseIni` calls (void use) keep compiling.
- testShippedIni: `fopen("queue_hud.ini","rb")` (cwd = test dir), read whole file; `Config c; KT_CHECK_EQ(parseIni(c,txt,&fb),0)`; `Config ref; ref.waitingIconDim=false; ref.stripHeadPct=150; KT_CHECK(configEqual(c, ref))` (pins "uncommented keys = defaults except the two overrides"; keep in step if RV1 removed spellReplayPos from Config). KT_CHECK(file opened) so a missing file fails.
- testRL14SigRest: update testQueueSig lines `t[9].type = 0x99; // past the cap: not compared` + the following `KT_CHECK(sigEqual(...))` -> now `KT_CHECK(!sigEqual(big, makeQueueSig(t,10,...)))` (restHash differs); identical arrays equal; `big.restHash != 0`; ne=8 sig `restHash == 0`; ne=16 (QEntry w[16]) change at index 15 -> unequal; `big.nTypes == 8` and the s10 fmtQj golden `types=[c,d,e,f,10,11,12,13,+2]` unchanged.
- testRL13Mode: base{combat=1,...strings}; `!modeChanged(base, {combat=-1})`; `modeChanged({combat=-1}, {combat=1})`; `modeWhy({0},{1})=="edge"`; `modeWhy({combat=1},{combat=-1,menu=1})=="menu"`; `modeWhy({-1},{1,menu=0})=="ui"`; modeStored(prev{1}, true, now{-1,paused=1}).combat == 1; modeStored(prev, false, now{-1}).combat == -1. Sequence 1,-1,1 through modeChanged/modeStored: zero changes. Existing testModeChangedWhy still passes (combat 0->1 edge, 1->0 ended).
- testRL20Hidden: six `E(0x0F, id)` entries, no nodes -> `buildSlots`==4, `hiddenSlots`==2; `{E(0x3F), E(0x0F,1), E(0x0F,2), E(0x0F,3)}` + rn `{N(9,1,300), N(10,2,0,77)}` -> 5 slots total, hidden 1; empty/null -> 0; 4 slots -> 0. fmtMode with `hidden=2` ends `why=ui hidden=2`; hidden=0 golden (goldB) unchanged; `modeChanged` true when only hidden differs.
- testRL20Holds: `holdsAction`: entry 0x2E -> true; rn sub 9 -> true; bare 0x3F -> false; cur sub 9 aa8=0 ab8=0 unpaused staleSince=1000 now=2600 -> false (now=2400 -> true); same stale cur but paused -> true; aa8=1 -> true.
- testRL7Match: act = makeSpellAction(cr, 300, ...). matchOnly=false + entry `E(0x0F,999)` -> true; matchOnly=true + same -> false; `E(0x0F,300)` -> true; rn `N(9,1,301)` false, `N(9,1,300)` true, `N(10,1,0,300)` false; cur `N(9,1,300)` with creal true -> true, creal false -> false. Item act = makeItemAction(cr, 0x80000123,...): entry `E(0x2E,0x123)` true, `E(0x2E,0x124)` false; rn `N(10,1,0,0x7F000123)` true (low 24 bits). Verdict: ActionBuffer b{active,activeAction=spell 300,sawExecuting=true,activeStartTick=1000}; present=actionPresentIn(...rn N(9,1,301)...)=false; `b.noteView(false,2000)`; `activeVerdict(b, EngineView{}, 2200)==AV_EXECUTED`; with sawExecuting=false the same node gives present=true -> `AV_PRESENT`.
- testRL4Generation: prev {A,B,C} now {D,E,F} true; {B,A,C} false (rotation); {A,B,C} vs {A,E,F} false; {A,0,0} vs {B,0,0} true; now {0,0,0} false; prev {0,0,0} false; {A,B,0} vs {C,0,0} true.
- testRL9DownHold: `downHoldExpired(0, 99999)` false; (1000, 20999) false; (1000, 21000) true; wrap `since=0xFFFFFF00u, now=0x4E00u` -> 0x4F00=20224 -> true; `since=0xFFFFFF00u, now=0x100u` false.
- testWhyNames: add `whyName(DR_PARTY_CHANGED)=="party_changed"`, `whyName(DR_MEMBER_DOWN)=="member_down"`, and a fmtDrop golden `DROP cmd=14 m=1 why=member_down n=1`.
- common_contract_test (queue_hud_src + companion_selfheal_src): the two address pins above (`python3 scripts/run_tests.py cpp:patch_manager_mods/queue_hud_src/common_contract_test.cpp` and the selfheal one).
- Python: `python3 scripts/run_tests.py scripts/test_queue_hud_log_check.py` (hidden= golden).
PLAY rows (append to patch_manager_mods/PLAY_TEST_QUEUE.md; source `doc 47 RV6`, mod `queue-hud 0.4.9-dev`, install `RV7 hot-swap`, status `pending`):
- QH-RL4a: pause, buffer 2 commands on the leader, quickload (or leave the area), unpause. Pass: `DROP ... why=party_changed` and `PARTY_GEN` within ~1 s of the first tick after load, no stale icons. If instead `DROP why=identity_lost` appears after ~20 s, the Exile's creature survived (record it: OPEN 1).
- QH-RL4b: pause, buffer a power, start a companion conversation (or trigger a cutscene), unpause during it. Pass (information): note whether a `dispatching buffered` line lands while the dialogue is open; if yes, open a WI (dialogue gate; candidate server+0x54, unverified).
- QH-RL9: Tab to a companion, queue a Force power on it via the server route, let it drop to 0 HP. Pass: `DOWN_HOLD m=` once, no `SRV_CAST` while down; revived within 20 s -> the cast happens, otherwise `DROP why=member_down`.
- QH-RL15/16: put `cancel = fasle` and `log=1 ; x` in queue_hud.ini, start the game: one WARN `1 line(s) not applied ... first at line N` (the `log=1 ; x` line is accepted). Then pause, fill the leader (1 active + 3 buffered), click a 5th power: `dec=drop_full`, WARN `... sound=1`, an audible tick.
- QH-RL20/21: pause, buffer 3 powers while the leader has a 0x3F with 2 round nodes (dispatch one, queue more fast): a `MODE ... hidden=N` line appears when icons exceed 4; unpaused, queue 2 spells back to back and read `dispatch hold=flag_cap ... held=` (the ab8 lifetime measurement; closes OPEN 3).

## Tooling
- Build/test: `python3 scripts/run_tests.py queue_hud_logic_test` ; `python3 scripts/run_tests.py patch_manager_mods/queue_hud_src/common_contract_test.cpp` ; `python3 scripts/run_tests.py patch_manager_mods/companion_selfheal_src/common_contract_test.cpp` ; `python3 scripts/run_tests.py scripts/test_queue_hud_log_check.py` (use `--list | rg -i queue` if a name differs; never bare run_tests.py). After editing, re-run each failing suite immediately (Verify-After-Fix).
- `python3 scripts/compat_check.py patch_manager_mods/_shared_src/GameAddr.h` (constants only; expect every including mod listed, no ABI change). Also `python3 scripts/run_tests.py smoke_selfheal` (the perf_lab smoke pins the `UseItemRouteA` name by string, R8).
- `scripts/pre_install_gate.sh --mod queue_hud_src` must print GATE PASS (also `--mod companion_selfheal_src` because of the alias and its contract pin). `python3 scripts/check_raw_addresses.py` stays clean (no raw hex added). Commit with explicit paths: `git commit -- <the files above>`; install is RV7 (user GO).

## Risks / ordering
- Apply after RV1 (hook typedef/replay, ini `spell_replay_pos`, dispatchAction bool), RV2 (the `actionInQueue` block, FlagGate fields, g_flagGate/g_kickLast resets in the swap block) and RV3 (discardDetached signature, handoffTick comment block, cwHideAll/doCancel). Conflict hot spots: RL7 (RV2 block), RL21 `holdSince` (RV2 RL3), RL4 discardDetached call (RV3), the handoffTick comment (RV3).
- The `DropWhy` enum is touched by RV1/RV2/RV3 too: append, never renumber; `whyName` has no default case, so a missing case fails `-Werror`.
- RL4's disjoint-set rule means a surviving PC keeps today's 20 s path; RL9's wall-clock ceiling also counts paused time (accepted).
- Open engine questions for the PLAY rows: (1) do the PC/companion server creatures get new addresses on load/area change; (2) CrCommandable value while Down; (3) real ab8 lifetime after an out-of-combat cast.

## Review corrections applied
- R7 (MINOR): RL4 drop-all also resets `g_roundOrder[k]`, `g_cpend[k]`, `g_handoffHold[k]`, `g_heldStrip[k]` per slot and `g_heldLeader` once; `discardDetached(k, "party_changed", true)` for a default-level line; generation analysis recorded under RL4.
- R8 (MINOR): alias `UseItemRouteA = AddUseTalentOnObject` verified (new line precedes the alias); tooling now also runs `smoke_selfheal`; RV2 ORPHAN premise note added to the docs batch (0x51CB10 empties the round list, only object 0x541080 does not).
- R9: no change needed (verified). RV3-dependent: `discardDetached` third arg relies on RV3 RQ9.
