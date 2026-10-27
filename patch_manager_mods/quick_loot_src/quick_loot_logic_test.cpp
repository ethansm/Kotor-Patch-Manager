// quick_loot_logic_test.cpp -- host test of QuickLootLogic.h (doc 52 QL0/QL1a/QL2p): event filter, wrap-safe ms, item cap, tracker, take-all decisions, formatters.
#include "QuickLootLogic.h"
#include "check.h"
#include <string.h>
#include <string>

using namespace qlogic;

int main() {
    // ---- event filter: 0x16 / 0x17 / 0x1B only; heartbeat (0) and neighbours are not watched
    kt::Check("0x16 watched", isWatchedEvent(0x16));
    kt::Check("0x17 watched", isWatchedEvent(0x17));
    kt::Check("0x1B watched", isWatchedEvent(0x1B));
    kt::Check("0x00 heartbeat not watched", !isWatchedEvent(0x00));
    kt::Check("0x15 not watched", !isWatchedEvent(0x15));
    kt::Check("0x18 not watched", !isWatchedEvent(0x18));
    kt::Check("0x1C not watched", !isWatchedEvent(0x1C));
    kt::Check("negative id not watched", !isWatchedEvent(-1));

    // ---- wrap-safe ms
    kt::CheckEq("msSince plain", msSince(1500u, 1000u), 500u);
    kt::CheckEq("msSince zero", msSince(7u, 7u), 0u);
    kt::CheckEq("msSince across the 32-bit wrap", msSince(100u, 0xFFFFFF9Cu), 200u);

    // ---- item cap
    kt::CheckEq("clamp 0", clampItems(0), 0u);
    kt::CheckEq("clamp negative", clampItems(-5), 0u);
    kt::CheckEq("clamp small", clampItems(3), 3u);
    kt::CheckEq("clamp at cap", clampItems(64), 64u);
    kt::CheckEq("clamp above cap", clampItems(65), 64u);
    kt::CheckEq("clamp large but sane", clampItems(4096), 64u);
    kt::CheckEq("clamp garbage count logs nothing", clampItems(4097), 0u);
    kt::CheckEq("clamp INT_MAX logs nothing", clampItems(0x7FFFFFFF), 0u);

    // ---- tracker
    {
        Tracker t;
        kt::Check("nothing pending: no match", !t.matches(0x1000, 5));
        kt::Check("null plc never matches", !t.matches(0, 5));
        t.onShow(0x1000, 100);
        kt::Check("pending matches", t.matches(0x1000, 150));
        kt::Check("other placeable does not match", !t.matches(0x2000, 150));
        kt::Check("close of another placeable is ignored", !t.onClose(0x2000, 200) && !t.closed);
        kt::Check("close of the pending placeable", t.onClose(0x1000, 300) && t.closed);
        kt::Check("inside the tail window still matches", t.matches(0x1000, 300 + TailMs));
        kt::Check("past the tail window stops matching", !t.matches(0x1000, 300 + TailMs + 1));
        kt::Check("expiry cleared the pending pointer", t.pending == 0 && !t.closed);
        kt::Check("and stays unmatched", !t.matches(0x1000, 300 + TailMs + 2));
        t.onShow(0x3000, 10000);
        kt::Check("a new Show re-arms", t.matches(0x3000, 10001) && !t.matches(0x1000, 10001));
        Tracker w; w.onShow(0x1, 0xFFFFFF00u); w.onClose(0x1, 0xFFFFFF10u);
        kt::Check("tail window is wrap-safe", w.matches(0x1, 0x00000010u) && !w.matches(0x1, 0x00001000u));
        Tracker r; r.onShow(0x1, 0);
        kt::Check("raw log budget is 3", r.wantRaw() && r.wantRaw() && r.wantRaw() && !r.wantRaw());
        r.onShow(0x1, 1);
        kt::Check("a new Show resets the raw budget", r.wantRaw());
    }

    // ---- QL1a: consume matrix. Base case consumes; each single failing condition yields exactly its reason. The Show flag is NOT a condition.
    {
        kt::Check("all conditions met consumes", shouldConsume(true, true, false, true, false) == Reason::Consume);
        kt::Check("disabled", shouldConsume(false, true, false, true, false) == Reason::Disabled);
        kt::Check("no hud", shouldConsume(true, false, false, true, false) == Reason::NoHud);
        kt::Check("shift held", shouldConsume(true, true, true, true, false) == Reason::Shift);
        kt::Check("no placeable", shouldConsume(true, true, false, false, false) == Reason::NoPlc);
        kt::Check("busy", shouldConsume(true, true, false, true, true) == Reason::Busy);
        // priority: disabled > nohud > shift > noplc > busy
        kt::Check("priority disabled over everything", shouldConsume(false, false, true, false, true) == Reason::Disabled);
        kt::Check("priority nohud over shift", shouldConsume(true, false, true, false, true) == Reason::NoHud);
        kt::Check("priority shift over noplc", shouldConsume(true, true, true, false, true) == Reason::Shift);
        kt::Check("priority noplc over busy", shouldConsume(true, true, false, false, true) == Reason::NoPlc);
        // exhaustive: only the one combination consumes
        int consumes = 0;
        for (int m = 0; m < 16; ++m)
            if (shouldConsume(m & 1, m & 2, m & 4, m & 8, m & 16) == Reason::Consume) ++consumes;
        kt::CheckEq("exactly one input combination consumes", consumes, 1);
        kt::Check("reason names", strcmp(reasonName(Reason::Shift), "shift") == 0 && strcmp(reasonName(Reason::NoHud), "nohud") == 0 &&
                  strcmp(reasonName(Reason::Busy), "busy") == 0 && strcmp(reasonName(Reason::Disabled), "disabled") == 0 &&
                  strcmp(reasonName(Reason::NoPlc), "noplc") == 0 && strcmp(reasonName(Reason::Consume), "consume") == 0);
        // QL1a fix: close flag follows the items present at send time
        kt::CheckEq("close flag: 3 items -> 1", closeFlagFor(3), 1);
        kt::CheckEq("close flag: 1 item -> 1", closeFlagFor(1), 1);
        kt::CheckEq("close flag: 0 items -> 0", closeFlagFor(0), 0);
        kt::CheckEq("close flag: unreadable -> 0", closeFlagFor(-1), 0);
    }

    // ---- QL1a: readyToSend (2-frame rule, 1000 ms timeout), giveUp (5000 ms), wrap
    {
        kt::Check("no OnOpen, fresh: not ready", !readyToSend(false, 10, 0, 1100, 1000));
        kt::Check("OnOpen same frame: not ready", !readyToSend(true, 10, 10, 1010, 1000));
        kt::Check("OnOpen 1 frame ago: not ready", !readyToSend(true, 11, 10, 1020, 1000));
        kt::Check("OnOpen 2 frames ago: ready", readyToSend(true, 12, 10, 1030, 1000));
        kt::Check("OnOpen 5 frames ago: ready", readyToSend(true, 15, 10, 1030, 1000));
        kt::Check("frame difference ignored without OnOpen", !readyToSend(false, 500, 10, 1030, 1000));
        kt::Check("timeout 999 ms: not ready", !readyToSend(false, 0, 0, 1999, 1000));
        kt::Check("timeout 1000 ms: ready", readyToSend(false, 0, 0, 2000, 1000));
        kt::Check("timeout also covers OnOpen seen but frames stalled", readyToSend(true, 10, 10, 2500, 1000));
        kt::Check("frame counter wrap: 0xFFFFFFFF -> 1 is 2 frames", readyToSend(true, 1, 0xFFFFFFFFu, 1010, 1000));
        kt::Check("frame counter wrap: 0xFFFFFFFF -> 0 is 1 frame", !readyToSend(true, 0, 0xFFFFFFFFu, 1010, 1000));
        kt::Check("ms wrap: timeout across the 32-bit wrap", readyToSend(false, 0, 0, 100, 0xFFFFFC18u) && !readyToSend(false, 0, 0, 0xFFFFFE00u, 0xFFFFFC18u));
        kt::Check("giveUp 4999 ms: no", !giveUp(5999, 1000));
        kt::Check("giveUp 5000 ms: yes", giveUp(6000, 1000));
        kt::Check("giveUp wrap-safe", giveUp(4000, 0xFFFFF000u) && !giveUp(500, 0xFFFFF000u));
    }

    // ---- QL1a: pending take
    {
        Take k;
        kt::Check("idle take ignores events", !k.onEvent(0x1000, EvtOpen, 5));
        k.arm(0x7F000001u, 0x1000, 100, 7);
        kt::Check("armed", k.active && k.obj == 0x7F000001u && k.plc == 0x1000 && k.showMs == 100 && k.showFrame == 7 && !k.openSeen);
        kt::Check("other placeable's OnOpen ignored", !k.onEvent(0x2000, EvtOpen, 8) && !k.openSeen);
        kt::Check("other event id ignored", !k.onEvent(0x1000, EvtClosed, 8) && !k.onEvent(0x1000, 0, 8) && !k.openSeen);
        kt::Check("null plc ignored", !k.onEvent(0, EvtOpen, 8));
        kt::Check("OnOpen marks and records the frame", k.onEvent(0x1000, EvtOpen, 9) && k.openSeen && k.openFrame == 9);
        kt::Check("second OnOpen does not move the frame", !k.onEvent(0x1000, EvtOpen, 20) && k.openFrame == 9);
        k.errLogged = true;
        k.clear();
        kt::Check("clear", !k.active && k.plc == 0 && !k.openSeen);
        kt::Check("cleared take ignores events", !k.onEvent(0x1000, EvtOpen, 30));
        k.arm(0x7F000002u, 0x3000, 500, 40);
        kt::Check("re-arm resets openSeen and errLogged", k.active && !k.openSeen && !k.errLogged && k.plc == 0x3000);
    }

    // ---- formatters
    char b[160];
    fmtShow(b, sizeof b, 0x7F0000AAu, 0, 0x1234ABCDu, 1, 3);
    kt::Check("SHOW line", strcmp(b, "SHOW obj=7f0000aa flag=0 plc=1234abcd open=1 n=3") == 0);
    fmtItem(b, sizeof b, 2, 0x7F000011u, 5);
    kt::Check("ITEM line", strcmp(b, "ITEM 2 id=7f000011 stack=5") == 0);
    fmtEvt(b, sizeof b, 0x1B, 250, 0);
    kt::Check("EVT line", strcmp(b, "EVT id=1b dt=250ms n=0") == 0);
    fmtClose(b, sizeof b, 1, 1800, 4, 0);
    kt::Check("CLOSE line", strcmp(b, "CLOSE flag=1 dt=1800ms n=4 open=0") == 0);
    fmtEvtRaw(b, sizeof b, 0x1000u, 5, 6, 0x2000u, 0x16, 5);
    kt::Check("EVTRAW line", strcmp(b, "EVTRAW plc=00001000 a1=5 a2=6 a3=00002000 u16=0016 ref=5") == 0);
    fmtEvtRaw(b, sizeof b, 0x1000u, 5, 6, 0x2000u, -1, 5);
    kt::Check("EVTRAW unreadable id prints ffff", strstr(b, "u16=ffff") != nullptr);
    fmtTakeArm(b, sizeof b, 0x7F0000AAu, 0x1234ABCDu, 3);
    kt::Check("TAKE-ARM line", strcmp(b, "TAKE-ARM obj=7f0000aa plc=1234abcd n=3") == 0);
    fmtVanilla(b, sizeof b, Reason::Shift);
    kt::Check("VANILLA line", strcmp(b, "VANILLA reason=shift") == 0);
    fmtTakeSend(b, sizeof b, 0x7F0000AAu, 66, 1, 3);
    kt::Check("TAKE-SEND line", strcmp(b, "TAKE-SEND obj=7f0000aa dt=66ms open=1 n=3") == 0);
    fmtErr(b, sizeof b, "msgobj", 120);
    kt::Check("ERR line", strcmp(b, "ERR msgobj dt=120ms") == 0);
    char tiny[8];
    fmtShow(tiny, sizeof tiny, 1, 1, 1, 1, 1);
    kt::Check("formatter truncates safely into a small buffer", strlen(tiny) == 7);

    // ---- QL1b: toast timing (wrap-safe), placement, text
    kt::Check("toast visible at 0 ms", toastVisible(1000, 1000, ToastMs));
    kt::Check("toast visible at 2999 ms", toastVisible(3999, 1000, ToastMs));
    kt::Check("toast gone at 3000 ms", !toastVisible(4000, 1000, ToastMs));
    kt::Check("toast gone long after", !toastVisible(900000, 1000, ToastMs));
    kt::Check("toast visible across the 32-bit wrap", toastVisible(100u, 0xFFFFFF9Cu, ToastMs));
    kt::Check("toast gone across the 32-bit wrap", !toastVisible(3000u, 0xFFFFFF9Cu, ToastMs));
    kt::Check("toast with the clock behind shownAt is not shown", !toastVisible(999u, 1000u, ToastMs));
    {
        Rect r = toastRect(100, 50, 20, 10, 200, 24);
        kt::CheckEq("toast x = name centre - W/2", r.x, 100 + 20 + 100 - 210);
        kt::CheckEq("toast y = name bottom + 20", r.y, 50 + 10 + 24 + 20);
        kt::Check("toast size 420x20", r.w == 420 && r.h == 20);
        kt::Check("rect equality", r == toastRect(100, 50, 20, 10, 200, 24) && r != toastRect(101, 50, 20, 10, 200, 24));
    }
    {
        char t[200];
        const char* n1[] = { "Medpac" };
        kt::CheckEq("count 0 gives no toast", fmtTook(t, sizeof t, n1, nullptr, 0), 0u);
        kt::Check("count 0 leaves an empty string", t[0] == 0);
        kt::CheckEq("one item", fmtTook(t, sizeof t, n1, nullptr, 1), 12u);
        kt::Check("one item text", strcmp(t, "Took: Medpac") == 0);
        const char* n3[] = { "Medpac", "Blaster Pistol", "Vibroblade" };
        int s3[] = { 2, 1, 999 };
        fmtTook(t, sizeof t, n3, s3, 3);
        kt::Check("stack suffix for 2 and 999, none for 1", strcmp(t, "Took: Medpac x2, Blaster Pistol, Vibroblade x999") == 0);
        int s4[] = { 0, -1, 1000 };
        fmtTook(t, sizeof t, n3, s4, 3);
        kt::Check("no suffix for 0, -1, 1000", strcmp(t, "Took: Medpac, Blaster Pistol, Vibroblade") == 0);
        kt::Check("null stacks array is fine", fmtTook(t, sizeof t, n3, nullptr, 3) > 0 && strstr(t, " x") == nullptr);
        // names: garbage / NUL / null / empty / spaces / too long
        const char* ng[] = { "Med\x01pac\xFF", "", nullptr, "   ", "Name\0Hidden" };
        fmtTook(t, sizeof t, ng, nullptr, 5);
        kt::Check("garbage bytes become ?, empty/null/blank become item, NUL ends the name",
                  strcmp(t, "Took: Med?pac?, item, item, item, Name") == 0);
        std::string longName(100, 'Z');
        const char* nl[] = { longName.c_str() };
        fmtTook(t, sizeof t, nl, nullptr, 1);
        kt::CheckEq("a long name is cut at 40 chars", strlen(t), 46u);
        // overflow and the 80-char boundary
        std::string a(40, 'A'), b32(32, 'B'), b33(33, 'B');
        const char* e1[] = { a.c_str(), b32.c_str() };
        kt::CheckEq("exactly 80 chars fits whole", fmtTook(t, sizeof t, e1, nullptr, 2), 80u);
        kt::Check("exactly 80 chars has no +K more", strstr(t, "more") == nullptr && strlen(t) == 80);
        const char* e2[] = { a.c_str(), b33.c_str() };
        unsigned l = fmtTook(t, sizeof t, e2, nullptr, 2);
        kt::Check("81 chars overflows to +1 more", strcmp(t, ("Took: " + a + ", +1 more").c_str()) == 0 && l == strlen(t));
        kt::Check("overflow text never passes 80 chars", l <= ToastMaxChars);
        // all fit only without a suffix: three items, exactly 80
        std::string c26(26, 'C'), d26(26, 'D');   // 6 + 26 + 2 + 26 + 2 + 18 = 80
        std::string f18(18, 'F');
        const char* e3[] = { c26.c_str(), d26.c_str(), f18.c_str() };
        l = fmtTook(t, sizeof t, e3, nullptr, 3);
        kt::Check("three items totalling exactly 80 all show", l == 80 && strstr(t, "more") == nullptr);
        // many items: 16 captured + extra beyond the capture
        std::string nm10(10, 'M');
        const char* many[16];
        for (int i = 0; i < 16; ++i) many[i] = nm10.c_str();
        l = fmtTook(t, sizeof t, many, nullptr, 16, 5);
        kt::Check("16 items + extra 5 stay within 80 and count all the rest", l <= 80 && strstr(t, "more") != nullptr);
        unsigned k = 0; sscanf(strstr(t, "+") + 1, "%u", &k);
        unsigned shownN = 0; for (const char* p = t; (p = strstr(p, nm10.c_str())) != nullptr; p += 10) ++shownN;
        kt::CheckEq("shown + K = 21", shownN + k, 21u);
        l = fmtTook(t, sizeof t, many, nullptr, 40);
        kt::Check("count above the capture cap folds into +K more", l <= 80 && strstr(t, "more") != nullptr);
        // small caller buffer
        char small[10];
        l = fmtTook(small, sizeof small, n3, s3, 3);
        kt::Check("small buffer is cut and terminated", l == 9 && strlen(small) == 9 && strncmp(small, "Took: Med", 9) == 0);
        kt::CheckEq("null out is harmless", fmtTook(nullptr, 5, n3, s3, 3), 0u);
        // sanitizeName direct
        char sn[8];
        sanitizeName(sn, sizeof sn, "ABCDEFGHIJ");
        kt::Check("sanitizeName honours a small cap", strcmp(sn, "ABCDEFG") == 0);
        char tl[200];
        fmtTookItem(tl, sizeof tl, 1, 0x7F000011u, "Med\x02pac", 3);
        kt::Check("TOOK line", strcmp(tl, "TOOK 1 id=7f000011 name=Med?pac stack=3") == 0);
        fmtTookItem(tl, sizeof tl, 0, 0x7F000012u, nullptr, -1);
        kt::Check("TOOK line falls back to item", strcmp(tl, "TOOK 0 id=7f000012 name=item stack=-1") == 0);
        fmtToast(tl, sizeof tl, "Took: Medpac");
        kt::Check("TOAST line", strcmp(tl, "TOAST Took: Medpac") == 0);
    }

    // ---- QL2p: config clamps
    {
        kt::Check("openM default band", clampOpenM(3.0f) == 3.0f && clampOpenM(0.5f) == 0.5f);
        kt::Check("openM nonsense falls back", clampOpenM(0.0f) == DefaultOpenM && clampOpenM(-2.0f) == DefaultOpenM && clampOpenM(500.0f) == DefaultOpenM);
        kt::Check("closeM kept when >= openM + 1", clampCloseM(3.0f, 5.0f) == 5.0f && clampCloseM(3.0f, 4.0f) == 4.0f);
        kt::Check("closeM clamped up to openM + 1", clampCloseM(3.0f, 3.5f) == 4.0f && clampCloseM(3.0f, 0.0f) == 4.0f && clampCloseM(2.0f, 2.99f) == 3.0f);
        kt::Check("closeM NaN falls to the minimum", clampCloseM(3.0f, NAN) == 4.0f);
        kt::CheckEq("scanMs clamps low", clampScanMs(1), 50u);
        kt::CheckEq("scanMs clamps negative", clampScanMs(-5), 50u);
        kt::CheckEq("scanMs clamps high", clampScanMs(999999), 5000u);
        kt::CheckEq("scanMs passes 250", clampScanMs(250), 250u);
        kt::CheckEq("takeKey ok", clampTakeKey(70), 70);
        kt::CheckEq("takeKey 0 -> F", clampTakeKey(0), 0x46);
        kt::CheckEq("takeKey 300 -> F", clampTakeKey(300), 0x46);
        kt::Check("2D distance ignores z (3-4-5)", dist2d(0, 0, 3, 4) == 5.0f && dist2d(1, 1, 1, 1) == 0.0f && dist2d(-3, 0, 0, 4) == 5.0f);
    }

    // ---- QL2p: candidate filter, each flag
    {
        CandFlags ok; ok.hasInv = true; ok.useable = true;
        kt::Check("baseline candidate ok at 2.9", candidateOk(ok, 2.9f, 3.0f) && strcmp(candidateWhy(ok, 2.9f, 3.0f), "ok") == 0);
        kt::Check("exactly OpenM is in range", candidateOk(ok, 3.0f, 3.0f));
        kt::Check("beyond OpenM is far", !candidateOk(ok, 3.01f, 3.0f) && strcmp(candidateWhy(ok, 3.01f, 3.0f), "far") == 0);
        kt::Check("NaN distance is not in range", !candidateOk(ok, NAN, 3.0f));
        CandFlags f = ok; f.hasInv = false;
        kt::Check("no inventory", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "noinv") == 0);
        f = ok; f.locked = true;
        kt::Check("locked", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "locked") == 0);
        f = ok; f.trap = true;
        kt::Check("trap armed", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "trap") == 0);
        f = ok; f.useable = false;
        kt::Check("not useable", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "unuse") == 0);
        f = ok; f.open = true;
        kt::Check("already open", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "open") == 0);
        f = ok; f.cooldown = true;
        kt::Check("in cooldown", !candidateOk(f, 1, 3) && strcmp(candidateWhy(f, 1, 3), "cooldown") == 0);
        f = CandFlags();
        kt::Check("all-false flags: first reason is noinv", strcmp(candidateWhy(f, 1, 3), "noinv") == 0);
        // exhaustive: only hasInv && useable && !locked && !trap && !open && !cooldown passes
        int passes = 0;
        for (int m = 0; m < 64; ++m) {
            CandFlags g; g.hasInv = m & 1; g.locked = m & 2; g.trap = m & 4; g.useable = m & 8; g.open = m & 16; g.cooldown = m & 32;
            if (candidateOk(g, 1.0f, 3.0f)) ++passes;
        }
        kt::CheckEq("exactly one flag combination is a candidate", passes, 1);
    }

    // ---- QL2p: nearest pick
    {
        float d[4] = {2.5f, 1.0f, 1.0f, 0.5f};
        bool ok[4] = {true, true, true, false};
        kt::CheckEq("nearest ok wins, not-ok nearer is ignored", pickNearest(d, ok, 4), 1);
        bool none[4] = {false, false, false, false};
        kt::CheckEq("none ok -> -1", pickNearest(d, none, 4), -1);
        kt::CheckEq("empty list -> -1", pickNearest(d, ok, 0), -1);
        bool only3[4] = {false, false, false, true};
        kt::CheckEq("single ok", pickNearest(d, only3, 4), 3);
        kt::CheckEq("tie goes to the lowest index", pickNearest(d, ok, 3), 1);
    }

    // ---- QL2p: cooldown / hysteresis (enter at OpenM, leave at CloseM, no re-open until left)
    {
        Cooldown c;
        kt::Check("empty set contains nothing", !c.contains(5) && c.n == 0);
        c.add(5); c.add(5); c.add(6);
        kt::Check("add dedupes", c.n == 2 && c.contains(5) && c.contains(6));
        kt::Check("inside the band (3.5 m) the id stays", !c.release(5, 3.5f, 5.0f) && c.contains(5));
        kt::Check("exactly CloseM does not release", !c.release(5, 5.0f, 5.0f) && c.contains(5));
        kt::Check("just past CloseM releases", c.release(5, 5.01f, 5.0f) && !c.contains(5) && c.contains(6) && c.n == 1);
        kt::Check("releasing an unknown id is a no-op", !c.release(99, 50.0f, 5.0f) && c.n == 1);
        // hysteresis scenario: open at 2.9, take/close (cooldown), wander at 3.5 and 4.9 (no re-open), then 5.1 releases, 2.9 re-opens
        Cooldown h; h.add(7);
        CandFlags f; f.hasInv = true; f.useable = true;
        f.cooldown = h.contains(7);
        kt::Check("in cooldown at 2.9 m: no reopen", !candidateOk(f, 2.9f, 3.0f));
        h.release(7, 3.5f, 5.0f); h.release(7, 4.9f, 5.0f);
        f.cooldown = h.contains(7);
        kt::Check("still no reopen at 4.9 m", !candidateOk(f, 2.9f, 3.0f) && h.contains(7));
        h.release(7, 5.1f, 5.0f);
        f.cooldown = h.contains(7);
        kt::Check("after walking past CloseM it can reopen", candidateOk(f, 2.9f, 3.0f));
        // cap: 64 ids, the oldest is dropped
        Cooldown full;
        for (uint32_t i = 1; i <= CooldownCap; ++i) full.add(i);
        kt::Check("full at the cap", full.n == CooldownCap && full.contains(1) && full.contains(CooldownCap));
        full.add(1000);
        kt::Check("overflow drops the oldest, keeps the newest", full.n == CooldownCap && !full.contains(1) && full.contains(2) && full.contains(1000));
        full.clear();
        kt::Check("clear (area change) empties it", full.n == 0 && !full.contains(2));
        kt::Check("area change detector", areaChanged(0x10, 0x20) && !areaChanged(0x10, 0x10) && !areaChanged(0, 0x20) && !areaChanged(0x10, 0));
    }

    // ---- QL2p: leave decision
    {
        kt::Check("in range, all fine: stay", leaveReason(true, false, false, 4.9f, 5.0f) == Leave::None);
        kt::Check("exactly CloseM: stay", leaveReason(true, false, false, 5.0f, 5.0f) == Leave::None);
        kt::Check("past CloseM: far", leaveReason(true, false, false, 5.1f, 5.0f) == Leave::Far);
        kt::Check("suppress starts: leave", leaveReason(true, true, false, 1.0f, 5.0f) == Leave::Suppress);
        kt::Check("area change: leave", leaveReason(true, false, true, 1.0f, 5.0f) == Leave::Area);
        kt::Check("plc gone: leave", leaveReason(false, false, false, 1.0f, 5.0f) == Leave::Gone);
        kt::Check("priority gone > suppress > area > far", leaveReason(false, true, true, 9, 5) == Leave::Gone && leaveReason(true, true, true, 9, 5) == Leave::Suppress &&
                  leaveReason(true, false, true, 9, 5) == Leave::Area);
        kt::Check("leave names", strcmp(leaveName(Leave::Far), "far") == 0 && strcmp(leaveName(Leave::Gone), "gone") == 0 &&
                  strcmp(leaveName(Leave::Area), "area") == 0 && strcmp(leaveName(Leave::Suppress), "suppress") == 0);
    }

    // ---- QL2p: suppress transitions
    {
        kt::Check("all clear", suppressReason(true, false, false, false, 0) == Sup::None);
        kt::Check("no world", suppressReason(false, false, false, false, 0) == Sup::NoWorld);
        kt::Check("menu", suppressReason(true, true, false, false, 0) == Sup::Menu);
        kt::Check("dialog", suppressReason(true, false, true, false, 0) == Sup::Dialog);
        kt::Check("paused", suppressReason(true, false, false, true, 0) == Sup::Paused);
        kt::Check("combat", suppressReason(true, false, false, false, 1) == Sup::Combat);
        kt::Check("combat unreadable counts as combat", suppressReason(true, false, false, false, -1) == Sup::CombatUnk);
        kt::Check("priority noworld > menu > dialog > paused > combat",
                  suppressReason(false, true, true, true, 1) == Sup::NoWorld && suppressReason(true, true, true, true, 1) == Sup::Menu &&
                  suppressReason(true, false, true, true, 1) == Sup::Dialog && suppressReason(true, false, false, true, 1) == Sup::Paused);
        kt::Check("sup names", strcmp(supName(Sup::None), "none") == 0 && strcmp(supName(Sup::CombatUnk), "combat?") == 0 && strcmp(supName(Sup::NoWorld), "noworld") == 0);
        SupTracker t;
        kt::Check("start: none -> none is no change", !t.update(Sup::None));
        kt::Check("none -> combat changes", t.update(Sup::Combat) && t.last == Sup::Combat);
        kt::Check("combat -> combat does not log again", !t.update(Sup::Combat));
        kt::Check("combat -> paused changes", t.update(Sup::Paused));
        kt::Check("paused -> none changes", t.update(Sup::None));
        kt::Check("and stays quiet", !t.update(Sup::None));
    }

    // ---- QL2p: key rising edge + label
    {
        bool prev = false;
        kt::Check("up: no edge", !risingEdge(prev, false));
        kt::Check("press: edge", risingEdge(prev, true));
        kt::Check("held: no edge", !risingEdge(prev, true) && !risingEdge(prev, true));
        kt::Check("release: no edge", !risingEdge(prev, false));
        kt::Check("press again: edge", risingEdge(prev, true));
        bool held = false;
        risingEdge(held, true);   // key already down while the peek opens (prev tracked every frame)
        kt::Check("key held when the peek opens never fires", !risingEdge(held, true));
        char kb[8];
        kt::Check("key label F", strcmp(keyLabel(0x46, kb, sizeof kb), "F") == 0);
        kt::Check("key label 7", strcmp(keyLabel(0x37, kb, sizeof kb), "7") == 0);
        kt::Check("key label other", strcmp(keyLabel(0x70, kb, sizeof kb), "key") == 0);
    }
    {
        char t[200];
        const char* n3[] = { "Medpac", "Blaster Pistol", "Vibroblade" };
        int s3[] = { 2, 1, 1 };
        kt::Check("peek label, all shown", fmtPeek(t, sizeof t, "F", 3, n3, s3, 3) > 0 &&
                  strcmp(t, "3 items  [F] take all: Medpac x2, Blaster Pistol, Vibroblade") == 0);
        const char* n1[] = { "Medpac" };
        fmtPeek(t, sizeof t, "F", 1, n1, nullptr, 1);
        kt::Check("one item is singular", strcmp(t, "1 item  [F] take all: Medpac") == 0);
        fmtPeek(t, sizeof t, "F", 6, n3, s3, 3);
        kt::Check("items beyond the captured ones fold into +K more", strcmp(t, "6 items  [F] take all: Medpac x2, Blaster Pistol, Vibroblade, +3 more") == 0);
        kt::CheckEq("empty container: no label", fmtPeek(t, sizeof t, "F", 0, n3, s3, 0), 0u);
        kt::Check("no label leaves an empty string", t[0] == 0);
        kt::CheckEq("total but nothing captured: no label", fmtPeek(t, sizeof t, "F", 4, n3, s3, 0), 0u);
        // 100-char cap with +K more
        std::string nm(20, 'N');
        const char* many[16];
        for (int i = 0; i < 16; ++i) many[i] = nm.c_str();
        unsigned l = fmtPeek(t, sizeof t, "F", 25, many, nullptr, 16);
        kt::Check("long list stays within 100 chars", l <= PeekMaxChars && l == strlen(t));
        kt::Check("long list ends in +K more", strstr(t, "more") != nullptr);
        unsigned k = 0; sscanf(strstr(t, "+") + 1, "%u", &k);
        unsigned shownN = 0; for (const char* p = t; (p = strstr(p, nm.c_str())) != nullptr; p += 20) ++shownN;
        kt::CheckEq("shown + K = 25", shownN + k, 25u);
        // exact 100 boundary: prefix "2 items  [F] take all: " is 23 chars; 23 + 38 + 2 + 37 = 100
        std::string a(38, 'A'), b(37, 'B'), c(38, 'B');
        const char* e1[] = { a.c_str(), b.c_str() };
        l = fmtPeek(t, sizeof t, "F", 2, e1, nullptr, 2);
        kt::Check("exactly 100 chars fits whole", l == 100 && strstr(t, "more") == nullptr);
        const char* e2[] = { a.c_str(), c.c_str() };
        l = fmtPeek(t, sizeof t, "F", 2, e2, nullptr, 2);
        kt::Check("101 chars overflows to +1 more", l <= 100 && strstr(t, ", +1 more") != nullptr);
        char small[12];
        l = fmtPeek(small, sizeof small, "F", 3, n3, s3, 3);
        kt::Check("small buffer is cut and terminated", l == 11 && strlen(small) == 11);
        kt::CheckEq("null out is harmless", fmtPeek(nullptr, 5, "F", 3, n3, s3, 3), 0u);
        // label rects
        Rect r = peekRectName(100, 50, 200, 24);
        kt::Check("peek rect under the name, centred", r.x == 100 + 100 - PeekW / 2 && r.y == 50 + 24 + ToastGapBelowName && r.w == PeekW && r.h == ToastH);
        Rect fx = peekRectFixed(0, 0, 1920, 1080);
        kt::Check("fixed rect centred at 62% of the panel height", fx.x == 960 - PeekW / 2 && fx.y == 669 && fx.w == PeekW && fx.h == ToastH);
        Rect fo = peekRectFixed(100, 40, 800, 600);
        kt::Check("fixed rect follows the panel origin", fo.x == 100 + 400 - PeekW / 2 && fo.y == 40 + 372);
    }

    // ---- QL2p: the peek state machine
    {
        Peek p;
        kt::Check("idle is not busy", !p.busy() && !p.active);
        kt::Check("a Show with nothing expected is not a peek", !p.onShow(0x7F000001u, 0x1000, 100, 5));
        p.begin(0x1000, 0x7F000009u, 0xA0, 1000);
        kt::Check("begin: expecting, busy, not active", p.busy() && !p.active && p.expect == 0x1000 && p.serverId == 0x7F000009u && p.area == 0xA0);
        kt::Check("expiry: not yet at 1499 ms", !p.expired(2499));
        kt::Check("expiry at 1500 ms", p.expired(2500));
        kt::Check("expiry wrap-safe", p.expired(1400u - 1000u + 1000u + 1500u) && !p.expired(1400));
        kt::Check("another placeable's Show is not ours", !p.onShow(0x7F000002u, 0x2000, 1010, 6) && !p.active);
        kt::Check("OnOpen before the Show is remembered", p.onEvent(0x1000, EvtOpen, 6) && p.openSeen && p.openFrame == 6);
        kt::Check("other plc / event ignored", !p.onEvent(0x2000, EvtOpen, 7) && !p.onEvent(0x1000, EvtClosed, 7) && !p.onEvent(0x1000, EvtOpen, 8));
        kt::Check("our Show is consumed as the peek", p.onShow(0x7F0000AAu, 0x1000, 1020, 7) && p.active && p.expect == 0 && p.obj == 0x7F0000AAu && p.sinceMs == 1020);
        kt::Check("a second Show of the same plc is not a peek", !p.onShow(0x7F0000AAu, 0x1000, 1030, 8));
        kt::Check("active never expires", !p.expired(99999));
        kt::Check("not empty while items present", !p.emptyNow(2, 100, 5000));
        kt::Check("empty but OnOpen not settled (frame 6 -> 7)", !p.emptyNow(0, 7, 1030));
        kt::Check("empty and OnOpen settled 2 frames ago", p.emptyNow(0, 8, 1030));
        kt::Check("unreadable count is not empty", !p.emptyNow(-1, 100, 5000));
        p.clear();
        kt::Check("clear -> idle", !p.busy() && p.plc == 0 && p.text[0] == 0 && !p.openSeen);
        p.begin(0x3000, 5, 0xB0, 10);
        p.onShow(0x7F000003u, 0x3000, 20, 1);
        kt::Check("no OnOpen: empty only after the 1000 ms timeout", !p.emptyNow(0, 2, 500) && p.emptyNow(0, 2, 1020));
        kt::Check("events before begin are ignored", [] { Peek q; return !q.onEvent(0x1000, EvtOpen, 1); }());
    }

    // ---- QL2p: formatters
    fmtPeekOpen(b, sizeof b, 0x7F000009u, 0x1234ABCDu, 2.5f, 0);
    kt::Check("PEEK-OPEN line", strcmp(b, "PEEK-OPEN obj=7f000009 plc=1234abcd dist=2.50 n=0") == 0);
    fmtPeekShow(b, sizeof b, 0x7F0000AAu, 0x1234ABCDu, 0, 0);
    kt::Check("PEEK-SHOW line", strcmp(b, "PEEK-SHOW obj=7f0000aa plc=1234abcd flag=0 n=0") == 0);
    fmtPeekLabel(b, sizeof b, "1 item  [F] take all: Medpac");
    kt::Check("PEEK-LABEL line", strcmp(b, "PEEK-LABEL 1 item  [F] take all: Medpac") == 0);
    fmtPeekEmpty(b, sizeof b, 0x7F0000AAu, 66, 1);
    kt::Check("PEEK-EMPTY line", strcmp(b, "PEEK-EMPTY obj=7f0000aa dt=66ms open=1") == 0);
    fmtPeekTake(b, sizeof b, 0x7F0000AAu, 1200, 3);
    kt::Check("PEEK-TAKE line", strcmp(b, "PEEK-TAKE obj=7f0000aa dt=1200ms n=3") == 0);
    fmtPeekClose(b, sizeof b, 0x7F0000AAu, "far", 1, 5.25f, 3);
    kt::Check("PEEK-CLOSE line", strcmp(b, "PEEK-CLOSE obj=7f0000aa reason=far sent=1 dist=5.25 n=3") == 0);
    fmtPeekNoShow(b, sizeof b, 0x7F000009u, 1500, 1);
    kt::Check("PEEK-NOSHOW line", strcmp(b, "PEEK-NOSHOW obj=7f000009 dt=1500ms open=1") == 0);
    fmtPeekRefused(b, sizeof b, 0x7F000009u, 0);
    kt::Check("PEEK-REFUSED line", strcmp(b, "PEEK-REFUSED obj=7f000009 open=0") == 0);
    fmtTakeEmpty(b, sizeof b, 0x7F0000AAu, 70, 1);
    kt::Check("TAKE-EMPTY line", strcmp(b, "TAKE-EMPTY obj=7f0000aa dt=70ms open=1") == 0);
    fmtSuppress(b, sizeof b, "none", "combat");
    kt::Check("SUPPRESS line", strcmp(b, "SUPPRESS none>combat") == 0);
    {
        CandFlags f; f.hasInv = true; f.useable = true; f.locked = true;
        fmtScan(b, sizeof b, 0x7F000009u, 1.5f, f, "locked");
        kt::Check("SCAN line", strcmp(b, "SCAN nearest obj=7f000009 dist=1.50 inv=1 lock=1 trap=0 use=1 open=0 cd=0 why=locked") == 0);
        fmtScanNone(b, sizeof b);
        kt::Check("SCAN none line", strcmp(b, "SCAN nearest none") == 0);
    }
    fmtArea(b, sizeof b, 0xA0A0A0A0u, 321);
    kt::Check("AREA line", strcmp(b, "AREA area=a0a0a0a0 objs=321") == 0);
    fmtCooldown(b, sizeof b, "release", 0x7F000009u, 5.5f, 2);
    kt::Check("COOLDOWN line", strcmp(b, "COOLDOWN release obj=7f000009 dist=5.50 size=2") == 0);

    return kt::Report("quick_loot_logic_test");
}
