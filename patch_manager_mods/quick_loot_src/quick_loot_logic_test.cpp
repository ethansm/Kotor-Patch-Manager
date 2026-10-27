// quick_loot_logic_test.cpp -- host test of QuickLootLogic.h (doc 52 QL0/QL1a): event filter, wrap-safe ms, item cap, tracker, take-all decisions, formatters.
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

    // ---- QL1a: consume matrix. Base case consumes; each single failing condition yields exactly its reason.
    {
        kt::Check("all conditions met consumes", shouldConsume(true, true, 1, false, true, false) == Reason::Consume);
        kt::Check("disabled", shouldConsume(false, true, 1, false, true, false) == Reason::Disabled);
        kt::Check("no hud", shouldConsume(true, false, 1, false, true, false) == Reason::NoHud);
        kt::Check("flag 0", shouldConsume(true, true, 0, false, true, false) == Reason::Flag);
        kt::Check("flag 2", shouldConsume(true, true, 2, false, true, false) == Reason::Flag);
        kt::Check("flag -1", shouldConsume(true, true, -1, false, true, false) == Reason::Flag);
        kt::Check("shift held", shouldConsume(true, true, 1, true, true, false) == Reason::Shift);
        kt::Check("no placeable", shouldConsume(true, true, 1, false, false, false) == Reason::NoPlc);
        kt::Check("busy", shouldConsume(true, true, 1, false, true, true) == Reason::Busy);
        // priority: disabled > nohud > flag > shift > noplc > busy
        kt::Check("priority disabled over everything", shouldConsume(false, false, 0, true, false, true) == Reason::Disabled);
        kt::Check("priority nohud over flag", shouldConsume(true, false, 0, true, false, true) == Reason::NoHud);
        kt::Check("priority flag over shift", shouldConsume(true, true, 0, true, false, true) == Reason::Flag);
        kt::Check("priority shift over noplc", shouldConsume(true, true, 1, true, false, true) == Reason::Shift);
        kt::Check("priority noplc over busy", shouldConsume(true, true, 1, false, false, true) == Reason::NoPlc);
        // exhaustive: only the one combination consumes
        int consumes = 0;
        for (int m = 0; m < 32; ++m)
            for (int flag = 0; flag <= 2; ++flag)
                if (shouldConsume(m & 1, m & 2, flag, m & 4, m & 8, m & 16) == Reason::Consume) ++consumes;
        kt::CheckEq("exactly one input combination consumes", consumes, 1);
        kt::Check("reason names", strcmp(reasonName(Reason::Shift), "shift") == 0 && strcmp(reasonName(Reason::Flag), "flag") == 0 &&
                  strcmp(reasonName(Reason::NoHud), "nohud") == 0 && strcmp(reasonName(Reason::Busy), "busy") == 0 &&
                  strcmp(reasonName(Reason::Disabled), "disabled") == 0 && strcmp(reasonName(Reason::NoPlc), "noplc") == 0);
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

    return kt::Report("quick_loot_logic_test");
}
