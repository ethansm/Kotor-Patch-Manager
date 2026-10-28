// QuickLootMachine.h -- the decision part of quick-loot's peek / take state machine (doc 52 QL1a/QL1b/QL2p), extracted from QuickLoot.cpp so a
// host test can drive it frame by frame (quick_loot_sim_test.cpp). No game memory, no Win32. Every engine read and every engine / UI side
// effect goes through qlogic::Env, which QuickLoot.cpp implements over the real engine and the sim test over a fake world. Actions are
// executed immediately (not queued): OpenInventory may trigger a synchronous ShowContainerGui and the code that follows must see it.
// Log line texts and behaviour are unchanged from the pre-extraction QuickLoot.cpp.
#pragma once
#include "QuickLootLogic.h"

namespace qlogic {

constexpr int NameCap = 48;   // captured name buffer (sanitizeName cuts at ToastNameMax = 40)

struct Captured {
    unsigned count = 0;   // names captured (<= ToastMaxItems)
    unsigned total = 0;   // real repository count
    uint32_t ids[ToastMaxItems];
    int stacks[ToastMaxItems];
    char names[ToastMaxItems][NameCap];
};

// Leader / area snapshot. ok = leader object and its area resolved; haveSrv = the server object table was reachable.
struct World {
    bool ok = false;
    bool haveSrv = false;
    uint32_t ctrl = 0;           // leader server object id (the OpenInventory opener)
    uintptr_t area = 0;
    float x = 0, y = 0;
};

struct Config {
    bool enabled = true, proxOn = true;
    float openM = DefaultOpenM, closeM = DefaultCloseM;
    int takeKey = DefaultTakeKey;
    uint32_t scanMs = DefaultScanMs;
};

// Engine / UI boundary. Reads first, then actions.
struct Env {
    virtual ~Env() {}
    virtual void enter() = 0;                                                  // saferead scope + START log (called wherever the engine is read)
    // reads
    virtual bool readWorld(World& w) = 0;
    virtual Sup suppress(bool worldOk) = 0;
    virtual bool keyDown() = 0;                                                // take key held and the game window has focus
    virtual bool hudOk() = 0;                                                  // polls the hud-host, then: connected
    virtual bool shiftDown() = 0;
    virtual uintptr_t resolvePlc(uint32_t sid) = 0;                            // placeable for an area-list object id, 0 = gone (only with World::haveSrv)
    virtual bool plcPos(uintptr_t plc, float* x, float* y) = 0;
    virtual int itemCount(uintptr_t plc) = 0;                                  // -1 unreadable
    virtual int openFlag(uintptr_t plc) = 0;                                   // -1 unreadable
    virtual uintptr_t msgObject() = 0;                                         // client message object, 0 = not available
    virtual void capture(uintptr_t plc, Captured& c) = 0;                      // item names / stacks
    virtual bool areaList(const World& w, int* count) = 0;                     // false = list unreadable (no scan)
    virtual bool areaRawCount(const World& w, int* count) = 0;                 // AREA log: raw count
    virtual bool areaObject(const World& w, int i, uint32_t* id, float* x, float* y) = 0;   // false = skip (not a placeable / unreadable)
    virtual bool candFlags(uint32_t id, uintptr_t* plc, CandFlags* f) = 0;     // false = plc gone or unreadable
    // actions
    virtual void openInventory(uintptr_t plc, uint32_t opener) = 0;            // engine OpenInventory, playAnim = 1
    virtual void sendClose(uintptr_t msg, uint32_t obj, int flag) = 0;         // client OK-button Close message (flag 1 = take all)
    virtual void serverClose(uintptr_t plc, uint32_t opener) = 0;              // CloseInventory(plc, opener, 0)
    virtual void labelShow(const char* text) = 0;                              // peek label text (changed)
    virtual void labelHide() = 0;
    virtual void toast(const char* text) = 0;                                  // "Took: ..." toast
    virtual void log(const char* line) = 0;
};

struct Machine {
    Config cfg;
    Take take;
    Peek peek;
    Cooldown cool;
    SupTracker sup;
    uint32_t lastScan = 0;
    bool keyPrev = false;
    uintptr_t lastArea = 0;
    uint32_t nearId = 0;   // SCAN change detector: nearest in-range placeable id (0 = none) and its ok flag
    int nearOk = -1;
    uint32_t frame = 0;    // last HUD frame counter seen by onFrame
    Captured cap;          // scratch (shared: uses never overlap)

    // ---- QL1b / QL2p helpers
    void reportTook(Env& e, const Captured& c) {
        char b[160];
        const char* np[ToastMaxItems];
        for (unsigned i = 0; i < c.count; ++i) {
            fmtTookItem(b, sizeof b, i, c.ids[i], c.names[i], c.stacks[i]);
            e.log(b);
            np[i] = c.names[i];
        }
        char text[96];
        if (fmtTook(text, sizeof text, np, c.stacks, c.count, c.total > c.count ? c.total - c.count : 0)) {
            fmtToast(b, sizeof b, text);
            e.log(b);
            e.toast(text);
        }
    }

    // Ends the peek (any outcome): the container goes into the cooldown set. addCooldown false after an area change: proxTick has just
    // cleared the set for the new area, and an old-area id added back would be a stale entry (sim scenario f).
    void peekEnd(Env& e, bool addCooldown = true) {
        char b[160];
        const uint32_t sid = peek.serverId;
        if (addCooldown) {
            cool.add(sid);
            fmtCooldown(b, sizeof b, "add", sid, -1.0f, cool.n);
            e.log(b);
        }
        if (peek.text[0]) e.labelHide();
        peek.clear();
    }

    // Sends the peek's Close(flag) once. Returns false when the message object is not available yet (retry on the next tick, until GiveUpMs).
    // plcResolves false: the container is gone, nothing to close (sent stays 0).
    bool peekSendClose(Env& e, int flag, bool plcResolves, uint32_t now, int* sent) {
        *sent = 0;
        if (!plcResolves) return true;
        if (!peek.closeStartMs) peek.closeStartMs = now ? now : 1;
        const uintptr_t msg = e.msgObject();
        if (!msg) {
            char b[160];
            if (!peek.closeTried) { peek.closeTried = true; fmtErr(b, sizeof b, "peek-msgobj", msSince(now, peek.sinceMs)); e.log(b); }
            return giveUp(now, peek.closeStartMs);   // true = give up, the peek ends without a Close
        }
        e.sendClose(msg, peek.obj, flag);
        *sent = 1;
        return true;
    }

    // Re-reads the peeked container and rebuilds the label text (change-only, logged).
    void peekRefresh(Env& e, int n) {
        e.capture(peek.plc, cap);
        char key[8];
        keyLabel(cfg.takeKey, key, sizeof key);
        const char* np[ToastMaxItems];
        for (unsigned i = 0; i < cap.count; ++i) np[i] = cap.names[i];
        char text[sizeof peek.text];
        fmtPeek(text, sizeof text, key, n, np, cap.stacks, cap.count);
        if (strcmp(text, peek.text) != 0) {
            strncpy(peek.text, text, sizeof peek.text - 1);
            peek.text[sizeof peek.text - 1] = 0;
            char b[200];
            fmtPeekLabel(b, sizeof b, text);
            e.log(b);
            if (peek.text[0]) e.labelShow(peek.text); else e.labelHide();
        }
    }

    // Rising edge of TakeKey while peeking: take everything (Close flag 1), toast, cooldown.
    void peekTake(Env& e, uint32_t now) {
        const int n = e.itemCount(peek.plc);
        if (n <= 0) return;   // nothing listed: the key does nothing
        const uintptr_t msg = e.msgObject();
        char b[160];
        if (!msg) { fmtErr(b, sizeof b, "peek-take-msgobj", msSince(now, peek.sinceMs)); e.log(b); return; }
        e.capture(peek.plc, cap);   // names BEFORE the Close
        const uint32_t obj = peek.obj;
        const uint32_t dt = msSince(now, peek.sinceMs);
        e.sendClose(msg, obj, closeFlagFor(n));
        fmtPeekTake(b, sizeof b, obj, dt, n);
        e.log(b);
        reportTook(e, cap);
        peekEnd(e);
    }

    // Active peek, every ScanMs: leave checks, empty check, label refresh.
    void peekPoll(Env& e, const World& w, Sup s, uint32_t now) {
        char b[200];
        const uint32_t sid = peek.serverId;
        const uintptr_t cur = w.haveSrv ? e.resolvePlc(sid) : 0;
        const bool resolves = cur != 0 && cur == peek.plc;
        float dist = 1.0e9f;
        float px = 0, py = 0;
        if (resolves && w.ok && e.plcPos(cur, &px, &py)) dist = dist2d(w.x, w.y, px, py);
        const bool newArea = areaChanged(peek.area, w.area);
        const Leave lv = leaveReason(resolves, s != Sup::None, newArea, dist, cfg.closeM);
        if (lv != Leave::None) {
            const int n = resolves ? e.itemCount(cur) : -1;
            int sent = 0;
            if (!peekSendClose(e, 0, resolves, now, &sent)) return;
            fmtPeekClose(b, sizeof b, peek.obj, lv == Leave::Suppress ? supName(s) : leaveName(lv), sent, dist, n);
            e.log(b);
            peekEnd(e, !newArea);
            return;
        }
        const int n = cur ? e.itemCount(cur) : -1;
        if (peek.emptyNow(n, frame, now)) {   // OnOpen settled and still nothing in it: close quietly
            int sent = 0;
            if (!peekSendClose(e, 0, true, now, &sent)) return;
            fmtPeekEmpty(b, sizeof b, peek.obj, msSince(now, peek.sinceMs), e.openFlag(cur));
            e.log(b);
            peekEnd(e);
            return;
        }
        peekRefresh(e, n);
    }

    // Expecting phase: OpenInventory was called, its ShowContainerGui has not come. Gives up after ExpectTimeoutMs.
    void peekExpectCheck(Env& e, uint32_t now) {
        if (!peek.expired(now)) return;
        char b[160];
        const int open = e.openFlag(peek.plc);
        fmtPeekNoShow(b, sizeof b, peek.serverId, msSince(now, peek.expectMs), open);
        e.log(b);
        // Opened but no client Show came: no client object id to send the Close message with. Close it server-side (flag 0, items stay),
        // else the open flag (+0x390) stays set and OpenInventory refuses this container for the rest of the session.
        if (open == 1 && peek.opener) {
            e.serverClose(peek.plc, peek.opener);
            e.log("PEEK-NOSHOW server close sent");
        }
        peekEnd(e);
    }

    // Opens the chosen container with the engine's OpenInventory (lid animation on). The ShowContainerGui it triggers arrives in onShow.
    void peekOpen(Env& e, const World& w, uint32_t sid, uintptr_t plc, float dist, uint32_t now) {
        char b[160];
        peek.begin(plc, sid, w.area, now);   // before the call: a synchronous Show must already find the expectation
        fmtPeekOpen(b, sizeof b, sid, plc, dist, e.itemCount(plc));
        e.log(b);
        peek.opener = w.ctrl;
        e.openInventory(plc, w.ctrl);
        const int open = e.openFlag(plc);
        if (open == 0 && !peek.active) {   // refused (OpenInventory gates failed): do not retry until the leader has left
            fmtPeekRefused(b, sizeof b, sid, open);
            e.log(b);
            peekEnd(e);
        }
    }

    // Scan the leader's area for the nearest openable placeable; open it. Cooldown ids are released here once the leader is > CloseM away.
    void proxScan(Env& e, const World& w, uint32_t now) {
        char b[200];
        int cnt = 0;
        if (!e.areaList(w, &cnt)) return;
        if (cnt < 0) cnt = 0;
        if (cnt > static_cast<int>(MaxScanObjects)) cnt = static_cast<int>(MaxScanObjects);
        uint32_t bestId = 0, curNear = 0;
        uintptr_t bestPlc = 0;
        float bestD = 0, nearD = 0;
        CandFlags nearF;
        const char* nearWhy = "";
        bool curNearOk = false;
        for (int i = 0; i < cnt; ++i) {
            uint32_t id = 0;
            float px = 0, py = 0;
            if (!e.areaObject(w, i, &id, &px, &py)) continue;
            const float d = dist2d(w.x, w.y, px, py);
            bool cd = cool.contains(id);
            if (cd && cool.release(id, d, cfg.closeM)) {
                fmtCooldown(b, sizeof b, "release", id, d, cool.n);
                e.log(b);
                cd = false;
            }
            if (d > cfg.closeM) continue;
            uintptr_t plc = 0;
            CandFlags f;
            if (!e.candFlags(id, &plc, &f)) continue;
            f.cooldown = cd;
            const char* why = candidateWhy(f, d, cfg.openM);
            const bool ok = strcmp(why, "ok") == 0;
            if (!curNear || d < nearD) { curNear = id; nearD = d; nearF = f; nearWhy = why; curNearOk = ok; }
            if (ok && (!bestId || d < bestD)) { bestId = id; bestD = d; bestPlc = plc; }
        }
        // SCAN line only when the nearest in-range placeable (or its verdict) changes -- never per scan
        const int okI = curNearOk ? 1 : 0;
        if (curNear != nearId || (curNear && okI != nearOk)) {
            if (curNear) fmtScan(b, sizeof b, curNear, nearD, nearF, nearWhy);
            else fmtScanNone(b, sizeof b);
            e.log(b);
        }
        nearId = curNear; nearOk = curNear ? okI : -1;
        if (bestId) peekOpen(e, w, bestId, bestPlc, bestD, now);
    }

    // Proximity peek tick (K2HUD_FRAME). Key edge every frame; the scan / poll every ScanMs.
    void proxTick(Env& e, uint32_t now) {
        if (!cfg.proxOn || !cfg.enabled) return;
        const bool edge = risingEdge(keyPrev, e.keyDown());   // updated every frame: a key held while the peek opens never fires
        if (peek.active && edge) { e.enter(); peekTake(e, now); return; }
        if (msSince(now, lastScan) < cfg.scanMs) return;
        lastScan = now;
        e.enter();
        World w;
        const bool wok = e.readWorld(w);
        if (wok && areaChanged(lastArea, w.area)) {
            char b[160];
            cool.clear();   // a new area: the old ids mean nothing
            nearId = 0; nearOk = -1;
            int cnt = -1;
            e.areaRawCount(w, &cnt);
            fmtArea(b, sizeof b, w.area, cnt);
            e.log(b);
        }
        if (wok) lastArea = w.area;
        const Sup s = e.suppress(wok);
        const Sup prevSup = sup.last;
        if (sup.update(s)) {   // change-only
            char b[96];
            fmtSuppress(b, sizeof b, supName(prevSup), supName(s));
            e.log(b);
        }
        if (peek.active) { peekPoll(e, w, s, now); return; }
        if (peek.expect) { peekExpectCheck(e, now); return; }
        if (take.active || s != Sup::None || !wok) return;
        proxScan(e, w, now);
    }

    // Sends the pending take-all Close (click take).
    void takeTick(Env& e, uint32_t now) {
        if (!readyToSend(take.openSeen, frame, take.openFrame, now, take.showMs)) return;
        e.enter();
        char b[160];
        const uintptr_t msg = e.msgObject();
        if (!msg) {
            const uint32_t dt = msSince(now, take.showMs);
            if (!take.errLogged) { take.errLogged = true; fmtErr(b, sizeof b, "msgobj", dt); e.log(b); }
            if (giveUp(now, take.showMs)) { fmtErr(b, sizeof b, "giveup", dt); e.log(b); take.clear(); }
            return;
        }
        const int n = e.itemCount(take.plc);
        const int closeFlag = closeFlagFor(n);   // items at send time, not the Show flag (OnOpen loot arrives after Show)
        const uint32_t obj = take.obj;
        const int openSeen = take.openSeen ? 1 : 0;
        const uint32_t dt = msSince(now, take.showMs);
        cap.count = 0; cap.total = 0;   // QL1b: names / stacks BEFORE the Close (the repository then includes the OnOpen loot)
        if (closeFlag) e.capture(take.plc, cap);
        take.clear();   // before the call: a re-entrant Show must not see a stale pending take
        e.sendClose(msg, obj, closeFlag);
        fmtTakeSend(b, sizeof b, obj, dt, openSeen, n);
        e.log(b);
        if (!closeFlag) {   // empty: closed with flag 0, no toast
            fmtTakeEmpty(b, sizeof b, obj, dt, openSeen);
            e.log(b);
            return;
        }
        reportTook(e, cap);
    }

    // K2HUD_FRAME: the pending click-take Close, then the proximity peek. `now` = ms clock, `fr` = HUD frame counter.
    void onFrame(Env& e, uint32_t fr, uint32_t now) {
        frame = fr;
        if (take.active) takeTick(e, now);
        proxTick(e, now);
    }

    // ShowContainerGui(objId, flag) for plc (0 = unresolved). n = item count read by the caller. Returns 1 = consumed (peek or armed take), 0 = vanilla.
    int onShow(Env& e, uint32_t objId, uintptr_t plc, int flag, int n, uint32_t now) {
        char b[160];
        if (plc && peek.onShow(objId, plc, now, frame)) {   // QL2p: the ShowContainerGui of our OpenInventory = a peek, not a take (any flag)
            fmtPeekShow(b, sizeof b, objId, plc, flag, n);
            e.log(b);
            return 1;
        }
        const bool hud = e.hudOk();   // polls the hud-host before the decision (as before)
        const bool shift = e.shiftDown();
        const Reason why = shouldConsume(cfg.enabled, hud, shift, plc != 0, take.active);
        if (why != Reason::Consume) {
            fmtVanilla(b, sizeof b, why);
            e.log(b);
            return 0;
        }
        if (peek.active && plc == peek.plc) {   // a click Show for the container this peek already holds open: the take-all wins, one Close in total
            e.log("PEEK-SUPERSEDED by click take");
            peekEnd(e);
        }
        take.arm(objId, plc, now, frame);
        fmtTakeArm(b, sizeof b, objId, plc, n);
        e.log(b);
        return 1;
    }

    // Placeable event filter (hot path: call before reading the event record).
    bool wantsEvent(uintptr_t plc) const { return (take.active && plc == take.plc) || (plc && plc == peek.plc && peek.busy()); }
    void onEvent(uintptr_t plc, int tid) {
        take.onEvent(plc, tid, frame);
        peek.onEvent(plc, tid, frame);
    }
};

}   // namespace qlogic
