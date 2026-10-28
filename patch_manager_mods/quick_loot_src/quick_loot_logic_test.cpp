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
    kt::CheckEq("Took duration: <= 3 names, no more = 3000", toastDurationMs(3, 0), 3000u);
    kt::CheckEq("Took duration: 1 name = 3000", toastDurationMs(1, 0), 3000u);
    kt::CheckEq("Took duration: 4 names = 3500", toastDurationMs(4, 0), 3500u);
    kt::CheckEq("Took duration: +K more = 3500", toastDurationMs(2, 1), 3500u);
    kt::CheckEq("Took duration: long line with +K more stays 3500", toastDurationMs(9, 20), 3500u);
    kt::Check("Took duration never above the 4 s cap", toastDurationMs(16, 4000) <= ToastCapMs && ToastLongMs <= ToastCapMs && ToastCapMs == 4000);
    kt::Check("toast visible for the longer duration", toastVisible(4499, 1000, 3500) && !toastVisible(4500, 1000, 3500));
    {
        Rect r = toastRect(100, 50, 20, 10, 200, 24);
        kt::CheckEq("toast x = name centre - W/2", r.x, 100 + 20 + 100 - 600);
        kt::CheckEq("toast y = name bottom + 20", r.y, 50 + 10 + 24 + 20);
        kt::Check("toast size 1200x20, the same as the peek", r.w == 1200 && r.h == 20 && r.w == PeekW && ToastW == LabelW);
        kt::Check("rect equality", r == toastRect(100, 50, 20, 10, 200, 24) && r != toastRect(101, 50, 20, 10, 200, 24));
    }
    {   // clampLabelX: x in [24, screenW - 24 - w]
        Rect r = { -100, 500, 1200, 20 };
        kt::CheckEq("clamp left edge to 24", clampLabelX(r, 0, 3840).x, 24);
        r.x = 3000;
        kt::CheckEq("clamp right edge to screenW - 24 - w", clampLabelX(r, 0, 3840).x, 3840 - 24 - 1200);
        r.x = 1000;
        kt::CheckEq("inside the range is untouched", clampLabelX(r, 0, 3840).x, 1000);
        r.x = 24;
        kt::CheckEq("exactly 24 is kept", clampLabelX(r, 0, 3840).x, 24);
        r.x = 3840 - 24 - 1200;
        kt::CheckEq("exactly screenW-24-w is kept", clampLabelX(r, 0, 3840).x, 3840 - 24 - 1200);
        r.x = 5;
        kt::CheckEq("panel origin offsets the range", clampLabelX(r, 100, 3840).x, 124);
        kt::CheckEq("unknown screen width = no clamp", clampLabelX(r, 0, 0).x, 5);
        kt::CheckEq("a label wider than the panel is centred", clampLabelX(r, 0, 1000).x, (1000 - 1200) / 2);
        kt::Check("clamp keeps y/w/h", clampLabelX(r, 0, 3840).y == 500 && clampLabelX(r, 0, 3840).w == 1200 && clampLabelX(r, 0, 3840).h == 20);
    }
    {   // backing panel rect
        Rect lab = { 1000, 600, 1200, 20 };
        Rect full = backingRect(lab, 200);   // a very long text: the full label width + padding
        kt::Check("backing at most label + 2 pad wide", full.x == 1000 - 10 && full.w == 1220 && full.y == 600 - 4 && full.h == 28);
        Rect small = backingRect(lab, 28);
        kt::CheckEq("backing width follows the text", small.w, 28 * 24 + 20);
        kt::CheckEq("backing is centred on the label", small.x + small.w / 2, 1000 + 600);
        kt::Check("black, alpha 0.55", BackAlpha == 0.55f && BackPadX == 10 && BackPadY == 4);
    }
    {
        char t[200];
        const char* n1[] = { "Medpac" };
        kt::CheckEq("count 0 gives no toast", fmtTook(t, sizeof t, n1, nullptr, 0), 0u);
        kt::Check("count 0 leaves an empty string", t[0] == 0);
        kt::CheckEq("one item", fmtTook(t, sizeof t, n1, nullptr, 1), 16u);
        kt::Check("one item text (no count)", strcmp(t, "Acquired: Medpac") == 0);
        const char* n3[] = { "Medpac", "Blaster Pistol", "Vibroblade" };
        int s3[] = { 2, 1, 999 };
        fmtTook(t, sizeof t, n3, s3, 3);
        kt::Check("count in brackets, stack suffix for 2 and 999, none for 1", strcmp(t, "Acquired (3): Medpac x2, Blaster Pistol, Vibroblade x999") == 0);
        int s4[] = { 0, -1, 1000 };
        fmtTook(t, sizeof t, n3, s4, 3);
        kt::Check("no suffix for 0, -1, 1000", strcmp(t, "Acquired (3): Medpac, Blaster Pistol, Vibroblade") == 0);
        kt::Check("null stacks array is fine", fmtTook(t, sizeof t, n3, nullptr, 3) > 0 && strstr(t, " x") == nullptr);
        {   // the review example: 4 items, 2 listed names + "+2 more"
            const char* n2[] = { "Medpac", "Blaster Pistol" };
            int s2[] = { 2, 1 };
            fmtTook(t, sizeof t, n2, s2, 2, 2);
            kt::Check("Acquired (4): Medpac x2, Blaster Pistol, +2 more", strcmp(t, "Acquired (4): Medpac x2, Blaster Pistol, +2 more") == 0 && strlen(t) == 48);
            fmtTook(t, sizeof t, n1, nullptr, 1, 3);
            kt::Check("one captured name of four: count is count + extra", strcmp(t, "Acquired (4): Medpac, +3 more") == 0);
            ListInfo li;
            fmtTook(t, sizeof t, n2, s2, 2, 2, &li);
            kt::Check("ListInfo: 2 shown, 2 more", li.shown == 2 && li.more == 2);
            fmtTook(t, sizeof t, n3, s3, 3, 0, &li);
            kt::Check("ListInfo: 3 shown, none more", li.shown == 3 && li.more == 0);
            fmtTook(t, sizeof t, n1, nullptr, 0, 0, &li);
            kt::Check("ListInfo: no line", li.shown == 0 && li.more == 0);
        }
        // names: garbage / NUL / null / empty / spaces / too long
        const char* ng[] = { "Med\x01pac\xFF", "", nullptr, "   ", "Name\0Hidden" };
        fmtTook(t, sizeof t, ng, nullptr, 5);
        kt::Check("garbage bytes become ?, empty/null/blank become item, NUL ends the name",
                  strcmp(t, "Acquired (5): Med?pac?, item, item, item, Name") == 0);
        std::string longName(100, 'Z');
        const char* nl[] = { longName.c_str() };
        fmtTook(t, sizeof t, nl, nullptr, 1);
        kt::Check("a long name is cut to 17 chars + ...", strcmp(t, ("Acquired: " + std::string(17, 'Z') + "...").c_str()) == 0 && strlen(t) == 30);
        // per-entry cut: 20 chars stay, 21+ become 17 + "..."; trailing space / punctuation trimmed before the dots; " xN" after the cut
        {
            char en[64];
            kt::CheckEq("entryName: 20 chars untouched", entryName(en, sizeof en, std::string(20, 'Q').c_str()), 20u);
            kt::Check("entryName: 20 chars text", strcmp(en, std::string(20, 'Q').c_str()) == 0);
            entryName(en, sizeof en, std::string(21, 'Q').c_str());
            kt::Check("entryName: 21 chars -> 17 + ...", strcmp(en, (std::string(17, 'Q') + "...").c_str()) == 0);
            entryName(en, sizeof en, "Lightsaber Crystal: Opila");
            kt::Check("entryName: Lightsaber Crystal: Opila", strcmp(en, "Lightsaber Crysta...") == 0);
            entryName(en, sizeof en, "Abcdefghijklmn: --xyzw");
            kt::Check("entryName: trailing space/punctuation trimmed before ...", strcmp(en, "Abcdefghijklmn...") == 0);
            entryName(en, sizeof en, "Medpac");
            kt::Check("entryName: short name untouched", strcmp(en, "Medpac") == 0);
            entryName(en, sizeof en, nullptr);
            kt::Check("entryName: null -> item", strcmp(en, "item") == 0);
            const char* nx[] = { "Lightsaber Crystal: Opila" };
            int sx[] = { 3 };
            fmtTook(t, sizeof t, nx, sx, 1);
            kt::Check("suffix after the cut: Lightsaber Crysta... x3", strcmp(t, "Acquired: Lightsaber Crysta... x3") == 0);
            char raw[64];
            sanitizeName(raw, sizeof raw, std::string(100, 'Z').c_str());
            kt::CheckEq("sanitizeName (log lines) still cuts at 40", strlen(raw), 40u);
        }
        // overflow and the 56-char boundary: "Acquired (2): " is 14 chars; entry 1 = 20 'A' + " x99" (24)
        std::string a(20, 'A'), b13(13, 'B'), b14(14, 'B');
        const char* e1[] = { a.c_str(), b13.c_str() };
        int se1[] = { 99, 9 };   // 24 and 16 chars (13 + " x9"): 14 + 24 + 2 + 16 = 56
        kt::CheckEq("exactly 56 chars fits whole", fmtTook(t, sizeof t, e1, se1, 2), 56u);
        kt::Check("exactly 56 chars has no +K more", strstr(t, "more") == nullptr && strlen(t) == 56);
        const char* e2[] = { a.c_str(), b14.c_str() };
        unsigned l = fmtTook(t, sizeof t, e2, se1, 2);   // 57 chars
        kt::Check("57 chars overflows to +1 more", strcmp(t, ("Acquired (2): " + a + " x99, +1 more").c_str()) == 0 && l == strlen(t));
        kt::Check("overflow text never passes 56 chars", l <= ToastMaxChars && ToastMaxChars == 56);
        // all fit only without a suffix: three items, exactly 56 ("Acquired (3): " 14 + 13 + 2 + 13 + 2 + 12)
        std::string c14(13, 'C'), d14(13, 'D'), f14(12, 'F');
        const char* e3[] = { c14.c_str(), d14.c_str(), f14.c_str() };
        l = fmtTook(t, sizeof t, e3, nullptr, 3);
        kt::Check("three items totalling exactly 56 all show", l == 56 && strstr(t, "more") == nullptr);
        // many items: 16 captured + extra beyond the capture
        std::string nm10(10, 'M');
        const char* many[16];
        for (int i = 0; i < 16; ++i) many[i] = nm10.c_str();
        l = fmtTook(t, sizeof t, many, nullptr, 16, 5);
        kt::Check("16 items + extra 5 stay within 56 and count all the rest", l <= 56 && strstr(t, "more") != nullptr && strncmp(t, "Acquired (21): ", 15) == 0);
        unsigned k = 0; sscanf(strstr(t, "+") + 1, "%u", &k);
        unsigned shownN = 0; for (const char* p = t + 15; (p = strstr(p, nm10.c_str())) != nullptr; p += 10) ++shownN;
        kt::CheckEq("shown + K = 21", shownN + k, 21u);
        l = fmtTook(t, sizeof t, many, nullptr, 40);
        kt::Check("count above the capture cap folds into +K more", l <= 56 && strstr(t, "more") != nullptr && strncmp(t, "Acquired (40): ", 15) == 0);
        // worst case from the review: 25 items, 20-char names with the longest stack suffix still show one name and +K more within 56
        std::string n20(20, 'W');
        const char* wc[16];
        int swc[16];
        for (int i = 0; i < 16; ++i) { wc[i] = n20.c_str(); swc[i] = 999; }
        l = fmtTook(t, sizeof t, wc, swc, 16, 9);
        kt::Check("worst case: at least one name shows, within 56", l <= 56 && strstr(t, n20.c_str()) != nullptr && strstr(t, "more") != nullptr);
        // small caller buffer
        char small[10];
        l = fmtTook(small, sizeof small, n3, s3, 3);
        kt::Check("small buffer is cut and terminated", l == 9 && strlen(small) == 9 && strncmp(small, "Acquired (3):", 9) == 0);
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
        fmtToast(tl, sizeof tl, "Acquired: Medpac");
        kt::Check("TOAST line", strcmp(tl, "TOAST Acquired: Medpac") == 0);
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
                  strcmp(t, "[F] Get Items (3): Medpac x2, Blaster Pistol, Vibroblade") == 0);
        const char* n1[] = { "Medpac" };
        fmtPeek(t, sizeof t, "F", 1, n1, nullptr, 1);
        kt::Check("one item: [F] Get Item: Medpac (no count)", strcmp(t, "[F] Get Item: Medpac") == 0);
        fmtPeek(t, sizeof t, "F", 6, n3, s3, 3);
        kt::Check("items beyond the captured ones fold into +K more", strcmp(t, "[F] Get Items (6): Medpac x2, Blaster Pistol, +4 more") == 0);
        {
            const char* n2[] = { "Medpac", "Blaster Pistol" };
            int s2[] = { 2, 1 };
            kt::CheckEq("review example 1 length", fmtPeek(t, sizeof t, "F", 4, n2, s2, 2), 53u);
            kt::Check("[F] Get Items (4): Medpac x2, Blaster Pistol, +2 more", strcmp(t, "[F] Get Items (4): Medpac x2, Blaster Pistol, +2 more") == 0);
            const char* real[16] = { "Medpac", "Blaster Pistol", "Adhesive Grenade", "Lightsaber Crystal: Opila", "Frag Grenade", "Credit Chip", "Vibroblade",
                                     "Combat Suit", "Stimulant", "Glow Rod", "Repair Kit", "Security Spike", "Power Cell", "Ion Blaster", "Datapad", "Cure Pack" };
            int sr[16] = { 2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
            ListInfo li;
            kt::CheckEq("review example 2 length", fmtPeek(t, sizeof t, "F", 25, real, sr, 16, &li), 55u);
            kt::Check("[F] Get Items (25): Medpac x2, Blaster Pistol, +23 more", strcmp(t, "[F] Get Items (25): Medpac x2, Blaster Pistol, +23 more") == 0);
            kt::Check("ListInfo of the peek line", li.shown == 2 && li.more == 23);
            kt::Check("a rebound key shows its name", (fmtPeek(t, sizeof t, "G", 1, n1, nullptr, 1), strcmp(t, "[G] Get Item: Medpac") == 0));
            kt::Check("an unnamed key falls back to 'key'", (fmtPeek(t, sizeof t, "key", 2, n2, s2, 2), strcmp(t, "[key] Get Items (2): Medpac x2, Blaster Pistol") == 0));
        }
        kt::CheckEq("empty container: no label", fmtPeek(t, sizeof t, "F", 0, n3, s3, 0), 0u);
        kt::Check("no label leaves an empty string (never '0 items')", t[0] == 0);
        kt::CheckEq("total but nothing captured: no label", fmtPeek(t, sizeof t, "F", 4, n3, s3, 0), 0u);
        // 56-char cap with +K more
        std::string nm(20, 'N');
        const char* many[16];
        for (int i = 0; i < 16; ++i) many[i] = nm.c_str();
        unsigned l = fmtPeek(t, sizeof t, "F", 25, many, nullptr, 16);
        kt::Check("long list stays within 56 chars", l <= PeekMaxChars && PeekMaxChars == 56 && l == strlen(t));
        kt::Check("long list ends in +K more", strstr(t, "more") != nullptr);
        unsigned k = 0; sscanf(strstr(t, "+") + 1, "%u", &k);
        unsigned shownN = 0; for (const char* p = t; (p = strstr(p, nm.c_str())) != nullptr; p += 20) ++shownN;
        kt::CheckEq("shown + K = 25", shownN + k, 25u);
        // exact 56 boundary: prefix "[F] Get Items (2): " is 19 chars; 19 + 20 + 2 + 15 = 56
        std::string a(20, 'A'), b(15, 'B'), c(16, 'B');
        const char* e1[] = { a.c_str(), b.c_str() };
        l = fmtPeek(t, sizeof t, "F", 2, e1, nullptr, 2);
        kt::Check("exactly 56 chars fits whole", l == 56 && strstr(t, "more") == nullptr);
        const char* e2[] = { a.c_str(), c.c_str() };
        l = fmtPeek(t, sizeof t, "F", 2, e2, nullptr, 2);
        kt::Check("57 chars overflows to +1 more", l <= 56 && strstr(t, ", +1 more") != nullptr);
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
    fmtPeekLabel(b, sizeof b, "[F] Get Item: Medpac");
    kt::Check("PEEK-LABEL line", strcmp(b, "PEEK-LABEL [F] Get Item: Medpac") == 0);
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

    // ---- 0.3.1-dev: placement, latch, Took state, dwell, new log lines
    {
        Geometry g;
        g.haveName = true; g.name = { 1700, 522, 533, 32 };
        g.havePanel = true; g.panel = { 0, 0, 3840, 1600 };
        Rect r; Pos p = Pos::None;
        kt::Check("no menu shown, panel known: fixed spot B", choosePlacement(g, &r, &p) && p == Pos::B);
        kt::Check("B is centred at 62% of the panel height", r.x == 1920 - 600 && r.y == 992 && r.w == 1200 && r.h == 20);
        g.menuShown = true;
        kt::Check("menu shown: A under the name", choosePlacement(g, &r, &p) && p == Pos::A);
        kt::Check("A = name centre - 600, name bottom + 20", r.x == 1700 + 266 - 600 && r.y == 522 + 32 + 20 && r.w == 1200);
        g.name = { 100, 522, 533, 32 };   // a target at the left edge: the 1200 px line would start at x < 0
        kt::Check("A is clamped to x >= 24", choosePlacement(g, &r, &p) && r.x == 24);
        g.name = { 3500, 522, 300, 32 };
        kt::Check("A is clamped to x <= screenW - 24 - w", choosePlacement(g, &r, &p) && r.x == 3840 - 24 - 1200);
        g.menuShown = false; g.havePanel = false;
        kt::Check("no panel, no menu, a name seen: A at the last name rect (no clamp)", choosePlacement(g, &r, &p) && p == Pos::A && r.x == 3650 - 600);
        g.haveName = false;
        kt::Check("nothing to anchor to", !choosePlacement(g, &r, &p));
        g.havePanel = true; g.panel = { 0, 0, 0, 0 };
        kt::Check("a panel with zero size is unknown", !choosePlacement(g, &r, &p));
        kt::Check("posName", strcmp(posName(Pos::A), "A") == 0 && strcmp(posName(Pos::B), "B") == 0 && strcmp(posName(Pos::None), "-") == 0);
    }
    {
        Latch l;
        kt::Check("latch starts free", !l.held && l.pos == Pos::None);
        l.set(Pos::A, { 1, 2, 3, 4 });
        kt::Check("latch holds position and rect", l.held && l.pos == Pos::A && l.rect == Rect{ 1, 2, 3, 4 });
        l.clear();
        kt::Check("latch clear", !l.held && l.pos == Pos::None && l.rect == Rect{ 0, 0, 0, 0 });
    }
    {
        ToastState t;
        kt::Check("toast idle", !t.active && !t.visible(1000));
        t.start("Acquired: Medpac", 1000, 3500);
        kt::Check("toast visible 0..3499 ms", t.visible(1000) && t.visible(4499) && !t.visible(4500));
        kt::Check("toast text kept", strcmp(t.text, "Acquired: Medpac") == 0);
        t.start("Acquired (2): A, B", 2000, ToastMs);
        kt::Check("a new toast replaces the old one at once", t.visible(2000) && t.visible(4999) && !t.visible(5000) && strcmp(t.text, "Acquired (2): A, B") == 0);
        t.start("Acquired: X", 0xFFFFFF9Cu, 3000);
        kt::Check("toast across the 32-bit wrap", t.visible(100u) && !t.visible(3000u));
        std::string longText(200, 'x');
        t.start(longText.c_str(), 0, 3000);
        kt::Check("overlong text is cut and terminated", strlen(t.text) == sizeof t.text - 1);
    }
    {
        Dwell d;
        kt::Check("no candidate: no open, state clear", !d.update(0, 1000, 400) && d.id == 0);
        kt::Check("first sight does not open", !d.update(7, 1000, 400));
        kt::Check("250 ms later: not yet", !d.update(7, 1250, 400));
        kt::Check("399 ms: not yet", !d.update(7, 1399, 400));
        kt::Check("400 ms: opens", d.update(7, 1400, 400));
        kt::Check("a different candidate restarts the dwell", !d.update(8, 1500, 400) && !d.update(8, 1800, 400) && d.update(8, 1900, 400));
        d.update(0, 2000, 400);
        kt::Check("losing the candidate resets", !d.update(8, 2100, 400));
        kt::Check("dwell 0 opens on first sight", d.update(9, 3000, 0));
        Dwell w;
        w.update(5, 0xFFFFFF9Cu, 400);
        kt::Check("dwell across the 32-bit wrap", !w.update(5, 100u, 400) && w.update(5, 300u, 400));
        d.reset();
        kt::Check("reset", d.id == 0);
        kt::Check("dwell constant", DwellMs == 400);
    }
    {
        char b[160];
        fmtLatch(b, sizeof b, Pos::A, { 676, 574, 1200, 20 });
        kt::Check("LABEL-LATCH line", strcmp(b, "LABEL-LATCH pos=A rect=676,574,1200,20") == 0);
        fmtLabelExtent(b, sizeof b, 52, 676, 574, 1200, 20);
        kt::Check("LABEL-EXTENT line", strcmp(b, "LABEL-EXTENT chars=52 extent=676,574,1200,20") == 0);
        fmtGiveUpServerClose(b, sizeof b, 0x1234ABCDu, 1);
        kt::Check("GIVEUP-SERVERCLOSE line", strcmp(b, "GIVEUP-SERVERCLOSE plc=1234abcd flag=1") == 0);
    }

    // ---- 0.4.0-dev: multi-row list
    {
        char b[96];
        // row names: 40-char cut (first 37 + "..."), stacks, trailing space trimmed, printable-only
        kt::CheckEq("ListRows clamp: 0 -> 1", clampListRows(0), 1u);
        kt::CheckEq("ListRows clamp: -3 -> 1", clampListRows(-3), 1u);
        kt::CheckEq("ListRows clamp: 8 stays", clampListRows(8), 8u);
        kt::CheckEq("ListRows clamp: 12 stays", clampListRows(12), 12u);
        kt::CheckEq("ListRows clamp: 40 -> 12", clampListRows(40), 12u);
        kt::Check("ListRows default 8", ListRowsDefault == 8 && ListRowsMin == 1 && ListRowsMax == 12);
        rowName(b, sizeof b, "Medpac");
        kt::Check("rowName short", strcmp(b, "Medpac") == 0);
        rowName(b, sizeof b, "Lightsaber Crystal: Opila");
        kt::Check("rowName 25 chars is not cut (the one-line label would cut at 20)", strcmp(b, "Lightsaber Crystal: Opila") == 0);
        const char* n40 = "0123456789012345678901234567890123456789";   // exactly 40
        rowName(b, sizeof b, n40);
        kt::Check("rowName 40 chars kept", strcmp(b, n40) == 0);
        const char* n41 = "0123456789012345678901234567890123456789X";
        rowName(b, sizeof b, n41);
        kt::Check("rowName 41 chars -> first 37 + ...", strcmp(b, "0123456789012345678901234567890123456...") == 0 && strlen(b) == 40);
        rowName(b, sizeof b, "Mandalorian Heavy Armor Plating Mk IV Upgrade Kit");   // 49 > NameCap-1: read cap, then cut
        kt::Check("rowName long name", strcmp(b, "Mandalorian Heavy Armor Plating Mk IV U...") != 0 && strlen(b) <= 40 && strcmp(b + strlen(b) - 3, "...") == 0);
        rowName(b, sizeof b, "Mandalorian Heavy Armor Plating Mk IV Ugly Name Here Too");
        kt::Check("rowName: trailing punctuation before the dots is trimmed", strlen(b) <= 40);
        rowName(b, sizeof b, "Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, , , , bbb");   // cut lands on punctuation
        kt::Check("rowName cut trims trailing non-alnum", strstr(b, ", ...") == nullptr && strcmp(b + strlen(b) - 3, "...") == 0);
        rowName(b, sizeof b, "Name   ");
        kt::Check("rowName trims trailing spaces", strcmp(b, "Name") == 0);
        rowName(b, sizeof b, "");
        kt::Check("rowName empty -> item", strcmp(b, "item") == 0);
        rowName(b, sizeof b, nullptr);
        kt::Check("rowName null -> item", strcmp(b, "item") == 0);
        rowName(b, sizeof b, "Caf\xC3\xA9");
        kt::Check("rowName non-printable -> ?", strcmp(b, "Caf??") == 0);
        rowText(b, sizeof b, "Medpac", 2);
        kt::Check("rowText stack 2", strcmp(b, "Medpac x2") == 0);
        rowText(b, sizeof b, "Medpac", 1);
        kt::Check("rowText stack 1 has no suffix", strcmp(b, "Medpac") == 0);
        rowText(b, sizeof b, "Medpac", 999);
        kt::Check("rowText stack 999", strcmp(b, "Medpac x999") == 0);
        rowText(b, sizeof b, "Medpac", 1000);
        kt::Check("rowText stack 1000 has no suffix", strcmp(b, "Medpac") == 0);
        rowText(b, sizeof b, "Medpac", -1);
        kt::Check("rowText unknown stack has no suffix", strcmp(b, "Medpac") == 0);
        rowText(b, sizeof b, n41, 3);   // 37 wide digits + "... x3" is wider than the block: cut further, stack kept
        kt::Check("rowText cut name + stack", strncmp(b, "0123456789", 10) == 0 && strcmp(b + strlen(b) - 6, "... x3") == 0 && textPx10(b) <= RowTextMaxPx && strlen(b) < 43);
        rowText(b, sizeof b, "Mandalorian Heavy Armor Plating Mk IV Upgrade Kit", 3);
        kt::Check("rowText 40-char cut + stack", strcmp(b, "Mandalorian Heavy Armor Plating Mk IV... x3") == 0 || strcmp(b + strlen(b) - 6, "... x3") == 0);
        // a row never wider than the budget: wide letters cut further
        rowText(b, sizeof b, "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", 999);
        kt::Check("rowText fits RowTextMaxPx", textPx10(b) <= RowTextMaxPx && strstr(b, "... x999") != nullptr);
        rowText(b, sizeof b, "WWWWWWWWWWWWWWWWWWWW", 1, 200);
        kt::Check("rowText honours a small budget", textPx10(b) <= 200 && strcmp(b + strlen(b) - 3, "...") == 0);
        kt::Check("textPx10 empty = 0, grows with text", textPx10("") == 0 && textPx10("ab") > textPx10("a"));
        kt::Check("textPx10 Medpac x2 = the mockup's 200.5 px", textPx10("Medpac x2") == 201);
    }
    {
        // ListData: rows, "+K more" past the capture
        const char* names[3] = { "Medpac", "Blaster Pistol", "Adhesive Grenade" };
        const int st[3] = { 2, 1, 3 };
        ListData d;
        d.set(names, st, 3, 3);
        kt::Check("3 items, no more", d.items == 3 && d.more == 0 && d.rowsTotal() == 3 && d.totalItems() == 3);
        char b[64];
        d.rowAt(0, b, sizeof b); kt::Check("row 0", strcmp(b, "Medpac x2") == 0);
        d.rowAt(2, b, sizeof b); kt::Check("row 2", strcmp(b, "Adhesive Grenade x3") == 0);
        d.rowAt(3, b, sizeof b); kt::Check("row past the end is empty", b[0] == 0);
        d.set(names, st, 3, 70);
        kt::Check("70 items, 3 captured: +67 more row", d.more == 67 && d.rowsTotal() == 4 && d.totalItems() == 70);
        d.rowAt(3, b, sizeof b); kt::Check("last row is the more row", strcmp(b, "+67 more") == 0);
        kt::Check("maxRowPx covers the widest row", d.maxRowPx >= textPx10("Adhesive Grenade x3"));
        d.set(names, st, 3, 1);
        kt::Check("total below count is treated as count", d.more == 0 && d.items == 3);
        d.set(names, st, 0, 5);
        kt::Check("no names: empty, no more row", d.empty() && d.more == 0 && d.rowsTotal() == 0);
        const char* many[100];
        for (int i = 0; i < 100; ++i) many[i] = "X";
        d.set(many, nullptr, 100, 100);
        kt::Check("capture cap 64, 36 more", d.items == ListMaxItems && ListMaxItems == 64 && d.more == 36);
    }
    {
        // scroll view
        ListView v; v.reset(25, 8);
        kt::Check("25 rows / 8 visible: max offset 17", v.maxOff() == 17 && v.scrollable());
        kt::Check("wheel back at the top does nothing", !v.scroll(120) && v.off == 0);
        kt::Check("one notch down = 1 row", v.scroll(-120) && v.off == 1);
        kt::Check("one notch up = back", v.scroll(120) && v.off == 0);
        kt::Check("3 notches in one delta", v.scroll(-360) && v.off == 3);
        kt::Check("half notch accumulates", !v.scroll(-60) && v.off == 3 && v.scroll(-60) && v.off == 4);
        kt::Check("opposite half notches cancel", !v.scroll(-60) && !v.scroll(60) && v.off == 4);
        kt::Check("big delta clamps at maxOff", v.scroll(-120 * 99) && v.off == 17);
        kt::Check("at the bottom: no change", !v.scroll(-120) && v.off == 17);
        kt::Check("clamp drops the remainder (no wind-up)", v.scroll(120) && v.off == 16);
        kt::Check("big positive delta clamps at 0", v.scroll(120 * 99) && v.off == 0);
        v.scroll(-120 * 5);
        v.setTotal(10);
        kt::Check("setTotal shrinks the offset into range", v.off == 2 && v.maxOff() == 2);
        v.setTotal(6);
        kt::Check("list fits: offset 0, not scrollable", v.off == 0 && !v.scrollable() && !v.scroll(-120));
        v.reset(25, 8);
        kt::Check("reset -> top", v.off == 0 && v.total == 25 && v.rows == 8 && v.acc == 0);
        v.reset(8, 8);
        kt::Check("exactly rows: not scrollable", !v.scrollable() && v.maxOff() == 0);
    }
    {
        // headers, indicator, block build
        char b[64];
        fmtHeader(b, sizeof b, false, "F", 25);
        kt::Check("peek header", strcmp(b, "[F] Get Items (25)") == 0);
        fmtHeader(b, sizeof b, true, "F", 4);
        kt::Check("Took header", strcmp(b, "Acquired Items (4)") == 0);
        fmtHeader(b, sizeof b, false, nullptr, 2);
        kt::Check("null key -> key", strcmp(b, "[key] Get Items (2)") == 0);
        fmtRange(b, sizeof b, 1, 8, 25);
        kt::Check("range", strcmp(b, "1-8 of 25") == 0);

        const char* names[25];
        std::string store[25];
        for (int i = 0; i < 25; ++i) { store[i] = "Item " + std::to_string(i + 1); names[i] = store[i].c_str(); }
        ListData d; d.set(names, nullptr, 25, 25);
        ListView v; v.reset(d.rowsTotal(), 8);
        ListDraw o;
        kt::Check("builds", buildListDraw(d, v, false, "F", &o));
        kt::Check("8 visible, scrollable, indicator", o.vis == 8 && o.scrollable && strcmp(o.indicator, "1-8 of 25") == 0 && o.off == 0);
        kt::Check("first / last row", strcmp(o.rows[0], "Item 1") == 0 && strcmp(o.rows[7], "Item 8") == 0);
        kt::CheckEq("height = pads + header + gap + 8 rows", o.h, blockHeightPx(8));
        kt::Check("width within [min, max]", o.w >= BlockMinW && o.w <= BlockMaxW);
        v.off = 10;
        ListDraw o2;
        buildListDraw(d, v, false, "F", &o2);
        kt::Check("scrolled: rows and indicator follow, size does not", strcmp(o2.rows[0], "Item 11") == 0 && strcmp(o2.indicator, "11-18 of 25") == 0 && o2.w == o.w && o2.h == o.h);
        v.off = 17;
        buildListDraw(d, v, false, "F", &o2);
        kt::Check("bottom", strcmp(o2.rows[7], "Item 25") == 0 && strcmp(o2.indicator, "18-25 of 25") == 0);
        v.off = 99;
        buildListDraw(d, v, false, "F", &o2);
        kt::Check("offset past the end is clamped", o2.off == 17);
        // fewer items than rows: short block, no indicator
        ListData d4; d4.set(names, nullptr, 4, 4);
        ListView v4; v4.reset(4, 8);
        ListDraw o4;
        buildListDraw(d4, v4, true, "F", &o4);
        kt::Check("4 items: 4 rows, no indicator", o4.vis == 4 && !o4.scrollable && o4.indicator[0] == 0 && strcmp(o4.header, "Acquired Items (4)") == 0);
        kt::CheckEq("4 rows height", o4.h, blockHeightPx(4));
        // the Took header "Acquired Items (4)" is wider than the 300 px floor: the block grows to header + pads (headers are never cut)
        kt::CheckEq("Took header wider than the floor widens the block", o4.w, o4.headerPx + 2 * BlockPadX);
        {   // single short item: the narrowest real header ("[F] Get Items (1)", 303 px) already exceeds the 300 px floor, so the floor never binds now
            ListData d1; d1.set(names, nullptr, 1, 1);
            ListView v1b; v1b.reset(1, 8);
            ListDraw o1;
            buildListDraw(d1, v1b, false, "F", &o1);
            kt::CheckEq("single item: width = header + pads", o1.w, o1.headerPx + 2 * BlockPadX);
            kt::Check("single item: never below the floor", o1.w >= BlockMinW && o4.w >= BlockMinW);
        }
        // the more row
        ListData d5; d5.set(names, nullptr, 20, 25);
        ListView v5; v5.reset(d5.rowsTotal(), 8);
        v5.off = v5.maxOff();
        ListDraw o5;
        buildListDraw(d5, v5, false, "F", &o5);
        kt::Check("more row last, indicator ends at the item total", strcmp(o5.rows[7], "+5 more") == 0 && strcmp(o5.indicator, "14-25 of 25") == 0);
        // width follows the widest of all rows and is capped
        const char* wide[3] = { "Medpac", "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW", "Credits" };
        ListData d6; d6.set(wide, nullptr, 3, 3);
        ListView v6; v6.reset(3, 8);
        ListDraw o6;
        buildListDraw(d6, v6, false, "F", &o6);
        kt::Check("very wide name: block at (nearly) the max width", o6.w > BlockMaxW - 80 && o6.w <= BlockMaxW);
        kt::Check("wide row still fits its budget", textPx10(o6.rows[1]) <= o6.w - 2 * BlockPadX);
        ListData e; ListDraw o7;
        kt::Check("empty list: nothing to draw", !buildListDraw(e, v6, false, "F", &o7));
        ListView v1; v1.reset(25, 1);
        ListDraw o8;
        buildListDraw(d, v1, false, "F", &o8);
        kt::Check("ListRows 1", o8.vis == 1 && strcmp(o8.indicator, "1-1 of 25") == 0);
        ListView v12; v12.reset(25, 12);
        buildListDraw(d, v12, false, "F", &o8);
        kt::CheckEq("ListRows 12 height", o8.h, blockHeightPx(12));
        ListView vbig; vbig.reset(25, 40);
        buildListDraw(d, vbig, false, "F", &o8);
        kt::Check("rows above 12 clamp to 12", o8.vis == 12);
    }
    {
        // Took duration per list row
        kt::CheckEq("Took 1 row: 3 s", tookDurationMs(1), 3000u);
        kt::CheckEq("Took 3 rows: 3 s", tookDurationMs(3), 3000u);
        kt::CheckEq("Took 4 rows: 3.5 s", tookDurationMs(4), 3500u);
        kt::CheckEq("Took 8 rows: 5.5 s", tookDurationMs(8), 5500u);
        kt::CheckEq("Took 9 rows: 6 s", tookDurationMs(9), 6000u);
        kt::CheckEq("Took 65 rows: capped at 6 s", tookDurationMs(65), 6000u);
        // hover hold
        ToastState t;
        t.start("x", 1000, 3000);
        t.tick(2000, false);
        kt::Check("no hover: timer runs", t.visible(3999) && !t.visible(4000));
        t.tick(2500, true);
        kt::Check("hover: shownAt moves with the clock", t.shownAt == 1500 && t.visible(4499) && !t.visible(4500));
        for (uint32_t now = 2516; now <= 12500; now += 16) t.tick(now, true);   // ~10 s of 16 ms frames hovered
        kt::Check("held through a long hover", t.visible(12500) && t.shownAt == 11500);
        const uint32_t held = t.shownAt;
        t.tick(12516, false);
        kt::Check("released: runs from where it was", t.visible(held + 2999) && !t.visible(held + 3000));
        ToastState w;
        w.start("x", 0xFFFFFF9Cu, 3000);
        w.tick(100u, true);
        kt::Check("hold across the 32-bit wrap", w.shownAt == 100u && w.visible(100u + 2999u) && !w.visible(100u + 3000u));
        ToastState idle;
        idle.tick(500, true);
        kt::Check("inactive toast: hold does nothing", !idle.active && idle.shownAt == 0);
    }
    {
        // block placement: A / B, clamps, bottom flip
        Geometry g;
        g.menuShown = true; g.haveName = true; g.name = { 1700, 520, 533, 32 };
        g.havePanel = true; g.panel = { 0, 0, 3840, 1600 };
        Anchor a; Pos p = Pos::None;
        kt::Check("anchor A", chooseAnchor(g, &a, &p) && p == Pos::A && a.cx == 1966 && a.top == 572 && a.nameTop == 520 && a.hasName);
        Rect r = blockRect(p, a, 400, 334, g.panel);
        kt::Check("A: centred on the name, under it", r == Rect{ 1766, 572, 400, 334 });
        g.menuShown = false;
        kt::Check("anchor still A when only the name rect is known and no panel", (g.havePanel = false, chooseAnchor(g, &a, &p) && p == Pos::A));
        g.havePanel = true;
        kt::Check("anchor B at the panel", chooseAnchor(g, &a, &p) && p == Pos::B && a.cx == 1920 && a.top == 992 && !a.hasName);
        r = blockRect(p, a, 400, 334, g.panel);
        kt::Check("B: centred, 62 %", r == Rect{ 1720, 992, 400, 334 });
        Geometry none;
        kt::Check("nothing to anchor to", !chooseAnchor(none, &a, &p));
        // x clamp
        a = Anchor(); a.cx = 50; a.top = 100;
        r = blockRect(Pos::A, a, 400, 300, { 0, 0, 3840, 1600 });
        kt::Check("x clamped to 24", r.x == 24);
        a.cx = 3830;
        r = blockRect(Pos::A, a, 400, 300, { 0, 0, 3840, 1600 });
        kt::Check("x clamped to the right margin", r.x == 3840 - 24 - 400);
        r = blockRect(Pos::A, a, 400, 300, { 0, 0, 0, 0 });
        kt::Check("unknown panel: no clamp", r.x == 3830 - 200 && r.y == 100);
        // bottom clamp: A flips above the name, B moves up
        a = Anchor(); a.cx = 1966; a.top = 1352; a.nameTop = 1300; a.hasName = true;
        r = blockRect(Pos::A, a, 400, 334, { 0, 0, 3840, 1600 });
        kt::Check("A at the bottom: flipped above the name", r.y == 1300 - 20 - 334 && r.y + r.h <= 1576);
        a.nameTop = 200;   // flipping would leave the panel at the top: slide up to the bottom limit instead
        a.top = 1500;
        r = blockRect(Pos::A, a, 400, 334, { 0, 0, 3840, 400 });
        kt::Check("A, no room above: bottom limit", r.y == 376 - 334 || r.y == 24);
        a = Anchor(); a.cx = 1920; a.top = 992;
        r = blockRect(Pos::B, a, 400, 334, { 0, 0, 3840, 600 });
        kt::Check("B on a short panel: moved up to the limit", r.y == 600 - 24 - 334);
        r = blockRect(Pos::B, a, 400, 334, { 0, 0, 3840, 1600 });
        kt::Check("B fits: unchanged", r.y == 992);
        r = blockRect(Pos::B, a, 400, 900, { 0, 0, 3840, 600 });
        kt::Check("block taller than the panel: top limit", r.y == 24);
        // child rects and hit test
        kt::Check("header rect", headerRect(400) == Rect{ BlockPadX, BlockPadY, 400 - 2 * BlockPadX + LabelSlackPx, HeaderH });
        kt::Check("row 0 under the header gap", rowRect(0, 400).y == BlockPadY + HeaderH + HeaderGapPx && rowRect(3, 400).y == rowRect(0, 400).y + 3 * (RowH + RowGapPx));
        kt::Check("indicator ends on the right padding line", indicatorRect(500, 120).x + 120 == 500 - BlockPadX);
        const Rect blk = { 1000, 500, 400, 334 };
        const float rowsTop = static_cast<float>(500 + BlockPadY + HeaderH + HeaderGapPx);
        kt::CheckEq("hover row 0", rowAtPoint(blk, 8, 1100.0f, rowsTop + 1.0f), 0);
        kt::CheckEq("hover row 2", rowAtPoint(blk, 8, 1100.0f, rowsTop + 2 * (RowH + RowGapPx) + 5.0f), 2);
        kt::CheckEq("hover the header: none", rowAtPoint(blk, 8, 1100.0f, rowsTop - 10.0f), -1);
        kt::CheckEq("hover left of the text area: none", rowAtPoint(blk, 8, 1000.0f + BlockPadX - 1, rowsTop + 5.0f), -1);
        kt::CheckEq("hover below the last visible row: none", rowAtPoint(blk, 3, 1100.0f, rowsTop + 3 * (RowH + RowGapPx) + 1.0f), -1);
        kt::Check("point in rect", pointInRect(blk, 1000.0f, 500.0f) && pointInRect(blk, 1399.9f, 833.9f) && !pointInRect(blk, 1400.0f, 600.0f) && !pointInRect(blk, 1100.0f, 499.0f));
        // latch keeps the anchor and clears it
        Latch l;
        l.anchor.cx = 7;
        l.set(Pos::B, { 1, 2, 3, 4 });
        l.clear();
        kt::Check("latch clear drops the anchor", l.anchor.cx == 0 && !l.held);
        // log lines
        char b[96];
        fmtListLog(b, sizeof b, 8, 25, 0, 640, 334);
        kt::Check("LIST line", strcmp(b, "LIST rows=8 items=25 off=0 w=640 h=334") == 0);
        fmtScroll(b, sizeof b, 3, 17);
        kt::Check("SCROLL line", strcmp(b, "SCROLL off=3/17") == 0);
        fmtWheelSub(b, sizeof b, 2, 5);
        kt::Check("WHEEL-SUB line", strcmp(b, "WHEEL-SUB token=2 after=5") == 0);
    }

    {
        // ---- 0.5.0-dev (QL2V): row / icon / text geometry, RowH variants, icon resref, scrollbar
        kt::CheckEq("RowH is 64 (was 32)", RowH, 64);
        kt::Check("alternatives 56 / 72", RowHAlt1 == 56 && RowHAlt2 == 72);
        kt::CheckEq("icon = RowH - 2 pad", iconPx(), 56);
        kt::CheckEq("icon at 56 / 72", iconPx(56) * 1000 + iconPx(72), 48 * 1000 + 64);
        // rows: the gap, the scrollbar reserve, the icon square, the vertically centred text
        const Rect r0 = rowRect(0, 800), r1 = rowRect(1, 800), r0s = rowRect(0, 800, true);
        kt::Check("row 0 / 1 pitch = RowH + gap", r1.y - r0.y == RowH + RowGapPx && r0.h == RowH && r0.x == BlockPadX);
        kt::CheckEq("row width without the scrollbar", r0.w, 800 - 2 * BlockPadX);
        kt::CheckEq("row width with the scrollbar", r0s.w, 800 - 2 * BlockPadX - SbReserve);
        const Rect ic = rowIconRect(r0), tx = rowTextRect(r0);
        kt::Check("icon square inside the row", ic.w == ic.h && ic.x == r0.x + RowIconPad && ic.y == r0.y + RowIconPad && ic.y + ic.h == r0.y + r0.h - RowIconPad);
        kt::Check("text after the icon, centred on the glyph box", tx.x == ic.x + ic.w + IconGapPx && tx.y == r0.y + (RowH - TextGlyphH) / 2 && tx.h == TextBoxH && tx.x + tx.w - LabelSlackPx == r0.x + r0.w - RowTextPadR);
        kt::Check("text box stays inside the row", tx.y >= r0.y && tx.y + tx.h <= r0.y + r0.h);
        // RowH variants fit the panel (ListRows 8 default and 12 max) and the scrollbar
        const int rhs[3] = { RowHAlt1, RowH, RowHAlt2 };
        for (int k = 0; k < 3; ++k) {
            const int rh = rhs[k];
            const std::string n = std::to_string(rh);
            kt::Check(("RowH " + n + ": 12 rows fit 1600 px with the margins").c_str(), blockHeightPx(12, rh) + 2 * ScreenMarginPx <= 1600);
            kt::Check(("RowH " + n + ": 8 rows block = pads + header + gap + rows + gaps").c_str(), blockHeightPx(8, rh) == 2 * BlockPadY + HeaderH + HeaderGapPx + 8 * rh + 7 * RowGapPx);
            kt::Check(("RowH " + n + ": text box fits the row").c_str(), (rh - TextGlyphH) / 2 >= 0 && (rh - TextGlyphH) / 2 + TextBoxH <= rh + 4 && iconPx(rh) >= 32);
            kt::Check(("RowH " + n + ": 2 rows already fit a scrollbar, 1 row does not").c_str(), sbFits(2, rh) && !sbFits(1, rh));
        }
        // buildListDraw carries rowH, the scrollbar flag and the width reserve
        const char* names[25];
        std::string store[25];
        const char* icons[25];
        for (int i = 0; i < 25; ++i) { store[i] = "Item " + std::to_string(i + 1); names[i] = store[i].c_str(); icons[i] = "iw_blstrpstl_001"; }
        ListData d; d.set(names, nullptr, 25, 25, icons);
        ListData d6; d6.set(names, nullptr, 6, 6, icons);
        ListView v25; v25.reset(d.rowsTotal(), 8), v25.off = 3;
        ListView v6; v6.reset(d6.rowsTotal(), 8);
        ListDraw o, o6;
        buildListDraw(d, v25, false, "F", &o);
        buildListDraw(d6, v6, false, "F", &o6);
        kt::Check("25 items: scrollbar, RowH, icons follow the offset", o.sb && o.rowH == RowH && strcmp(o.icons[0], "iw_blstrpstl_001") == 0 && strcmp(o.rows[0], "Item 4") == 0);
        kt::Check("6 items: no scrollbar", !o6.sb && !o6.scrollable);
        ListDraw ow;
        buildListDraw(d, v25, false, "F", &ow, RowHAlt2);
        kt::Check("RowH 72 build: taller block, rowH carried", ow.rowH == 72 && ow.h == blockHeightPx(8, 72) && ow.h > o.h);
        // block width: rows + icon chrome + scrollbar
        kt::Check("block width holds the widest row + icon chrome (+ scrollbar when shown)", o.w >= d.maxRowPx + rowChromePx() + SbReserve + 2 * BlockPadX || o.w == BlockMinW);
        kt::Check("scrollbar column sits inside the block", sbLayout(o.w, o.vis, o.rowsTotal, o.off).col.x + SbW + BlockPadX == o.w);
        // a "+K more" row has no icon; icons past the capture are empty
        ListData dm; dm.set(names, nullptr, 3, 10, icons);
        kt::Check("iconAt: items have one, the more row and past-the-end none", strcmp(dm.iconAt(0), "iw_blstrpstl_001") == 0 && dm.iconAt(3)[0] == 0 && dm.iconAt(99)[0] == 0);
        // the widest worst-case row text still fits the max block
        kt::Check("RowTextMaxPx leaves the max block room for the worst chrome", RowTextMaxPx + rowChromePx(RowHAlt2) + SbReserve + 2 * BlockPadX <= BlockMaxW);
    }
    {
        // ---- icon resref: copy of the engine's 16-byte, non-terminated CResRef
        char o[ResrefCap];
        char raw[16];
        memset(raw, 'a', sizeof raw);   // 16 chars, no NUL
        kt::CheckEq("16 chars, not NUL-terminated: copied whole and terminated", static_cast<int>(sanitizeResref(o, raw)), 16);
        kt::Check("... terminated at 16", o[16] == 0 && strncmp(o, "aaaaaaaaaaaaaaaa", 16) == 0);
        memset(raw, 0, sizeof raw); memcpy(raw, "iw_blstrpstl_001", 16);
        kt::Check("real resref", sanitizeResref(o, raw) == 16 && strcmp(o, "iw_blstrpstl_001") == 0);
        memset(raw, 0, sizeof raw); memcpy(raw, "ii_medeqpmnt_1\0\0", 16);
        kt::Check("stops at the first NUL", sanitizeResref(o, raw) == 14 && strcmp(o, "ii_medeqpmnt_1") == 0);
        memset(raw, 0, sizeof raw); memcpy(raw, "ia\x01_c\xFFlass", 10);
        sanitizeResref(o, raw);
        kt::Check("bytes outside 0x20..0x7E become '?'", strcmp(o, "ia?_c?lass") == 0);
        memset(raw, 0, sizeof raw); memcpy(raw, "abc   ", 6);
        kt::Check("trailing spaces trimmed", sanitizeResref(o, raw) == 3 && strcmp(o, "abc") == 0);
        memset(raw, 0, sizeof raw);
        kt::Check("all NUL (engine empty result): empty", sanitizeResref(o, raw) == 0 && o[0] == 0);
        kt::Check("null raw: empty", sanitizeResref(o, nullptr) == 0 && o[0] == 0);
        kt::Check("base item index plausibility", baseItemIdxOk(0) && baseItemIdxOk(55) && baseItemIdxOk(MaxBaseItemIdx - 1) && !baseItemIdxOk(-1) && !baseItemIdxOk(MaxBaseItemIdx) && !baseItemIdxOk(0x7FFFFFFF));
        // set() sanitizes the icon too, a too-long name never touches it
        ListData d; const char* nm[2] = { "A", "B" }; const char* ic[2] = { "ok_icon", nullptr };
        d.set(nm, nullptr, 2, 2, ic);
        kt::Check("ListData icons: kept / null -> empty", strcmp(d.iconAt(0), "ok_icon") == 0 && d.iconAt(1)[0] == 0);
        d.set(nm, nullptr, 2, 2);
        kt::Check("ListData without icons: all empty", d.iconAt(0)[0] == 0 && d.iconAt(1)[0] == 0);
    }
    {
        // ---- scrollbar: thumb size / position, arrows, track, hit test, click stepping
        const int w = 800;
        const Sb s0 = sbLayout(w, 8, 25, 0), sm = sbLayout(w, 8, 25, 8), se = sbLayout(w, 8, 25, 17);
        kt::Check("shown for an overflowing list", s0.shown && sm.shown && se.shown);
        kt::Check("hidden when the list fits / exactly the rows", !sbLayout(w, 6, 6, 0).shown && !sbLayout(w, 8, 8, 0).shown && !sbLayout(w, 0, 0, 0).shown);
        kt::Check("hidden when the rows cannot hold a scrollbar (ListRows 1)", !sbLayout(w, 1, 25, 0).shown);
        kt::CheckEq("column height = the list height", s0.col.h, listHeightPx(8));
        kt::Check("column on the right padding line, at the first row", s0.col.x == w - BlockPadX - SbW && s0.col.w == SbW && s0.col.y == BlockPadY + HeaderH + HeaderGapPx);
        kt::Check("arrows: square, at the column ends", s0.up == Rect{ s0.col.x, s0.col.y, SbW, SbW } && s0.down == Rect{ s0.col.x, s0.col.y + s0.col.h - SbW, SbW, SbW });
        kt::Check("track between the arrows", s0.track.y == s0.up.y + SbW && s0.track.y + s0.track.h == s0.down.y);
        const int th = s0.track.h * 8 / 25;
        kt::CheckEq("thumb size = track x visible / total", s0.thumb.h, th);
        kt::Check("thumb at the top of the track at off 0", s0.thumb.y == s0.track.y && s0.thumb.h == sm.thumb.h && s0.thumb.h == se.thumb.h);
        kt::CheckEq("thumb at the bottom of the track at the last offset", se.thumb.y + se.thumb.h, s0.track.y + s0.track.h);
        kt::CheckEq("thumb at the middle: (track - thumb) x off / maxOff", sm.thumb.y, s0.track.y + (s0.track.h - th) * 8 / 17);
        kt::Check("thumb stays inside the track, moves down with the offset", sm.thumb.y > s0.thumb.y && sm.thumb.y < se.thumb.y && sm.thumb.x >= sm.track.x && sm.thumb.x + sm.thumb.w <= sm.track.x + sm.track.w);
        kt::Check("offset past the end clamps like the last one", sbLayout(w, 8, 25, 99).thumb == se.thumb);
        const Sb sbig = sbLayout(w, 8, 4000, 0);
        kt::CheckEq("huge list: thumb at its minimum", sbig.thumb.h, SbThumbMin);
        const Sb s2 = sbLayout(w, 2, 3, 1);
        kt::Check("2 visible of 3: thumb inside the (short) track", s2.shown && s2.thumb.h <= s2.track.h && s2.thumb.y + s2.thumb.h <= s2.track.y + s2.track.h);
        // hit test (block-relative)
        const float cx = static_cast<float>(sm.col.x + SbW / 2);
        kt::Check("up arrow", sbHitAt(sm, cx, static_cast<float>(sm.up.y + 5)) == SbPart::Up);
        kt::Check("down arrow", sbHitAt(sm, cx, static_cast<float>(sm.down.y + SbW - 1)) == SbPart::Down);
        kt::Check("track above the thumb = page up", sbHitAt(sm, cx, static_cast<float>(sm.thumb.y - 2)) == SbPart::PageUp);
        kt::Check("track below the thumb = page down", sbHitAt(sm, cx, static_cast<float>(sm.thumb.y + sm.thumb.h + 2)) == SbPart::PageDown);
        kt::Check("on the thumb = thumb", sbHitAt(sm, cx, static_cast<float>(sm.thumb.y + sm.thumb.h / 2)) == SbPart::Thumb);
        kt::Check("outside the column (the rows) = none", sbHitAt(sm, static_cast<float>(sm.col.x - 1), static_cast<float>(sm.track.y + 5)) == SbPart::None && sbHitAt(sm, static_cast<float>(sm.col.x + SbW), static_cast<float>(sm.track.y + 5)) == SbPart::None);
        kt::Check("above / below the column = none", sbHitAt(sm, cx, static_cast<float>(sm.col.y - 1)) == SbPart::None && sbHitAt(sm, cx, static_cast<float>(sm.col.y + sm.col.h)) == SbPart::None);
        kt::Check("hidden scrollbar: never a hit", sbHitAt(sbLayout(w, 6, 6, 0), cx, 100.0f) == SbPart::None);
        // click stepping: arrows one row, track one page (the visible rows), clamped at both ends
        kt::Check("steps: up -1, down +1, pages +-rows, thumb / none 0", sbStepFor(SbPart::Up, 8) == -1 && sbStepFor(SbPart::Down, 8) == 1 && sbStepFor(SbPart::PageUp, 8) == -8 &&
                  sbStepFor(SbPart::PageDown, 8) == 8 && sbStepFor(SbPart::Thumb, 8) == 0 && sbStepFor(SbPart::None, 8) == 0);
        ListView v; v.reset(25, 8);
        kt::Check("down arrow: one row", v.step(sbStepFor(SbPart::Down, v.rows)) && v.off == 1);
        kt::Check("page down: 8 rows", v.step(sbStepFor(SbPart::PageDown, v.rows)) && v.off == 9);
        kt::Check("page down clamps at 17", v.step(sbStepFor(SbPart::PageDown, v.rows)) && v.off == 17 && !v.step(sbStepFor(SbPart::PageDown, v.rows)) && v.off == 17);
        kt::Check("down arrow at the end: no change", !v.step(1) && v.off == 17);
        kt::Check("page up: 8 rows", v.step(sbStepFor(SbPart::PageUp, v.rows)) && v.off == 9);
        kt::Check("page up clamps at 0", v.step(-8) && v.off == 1 && v.step(-8) && v.off == 0 && !v.step(-8));
        kt::Check("up arrow at the top: no change", !v.step(sbStepFor(SbPart::Up, v.rows)) && v.off == 0);
        v.acc = 77; v.step(1);
        kt::Check("a click drops the half-notch wheel remainder", v.acc == 0);
        ListView fit; fit.reset(5, 8);
        kt::Check("a list that fits ignores steps", !fit.step(3) && fit.off == 0);
        // hover rows leave the scrollbar column and the row gaps out
        const Rect blk = { 100, 100, 900, 700 };
        const float y0 = static_cast<float>(100 + rowsTopPx());
        kt::CheckEq("hover: row 0", rowAtPoint(blk, 8, 300.0f, y0 + 10.0f, true), 0);
        kt::CheckEq("hover: the gap between rows is no row", rowAtPoint(blk, 8, 300.0f, y0 + RowH + 1.0f, true), -1);
        kt::CheckEq("hover: row 1 after the gap", rowAtPoint(blk, 8, 300.0f, y0 + RowH + RowGapPx + 1.0f, true), 1);
        kt::CheckEq("hover: the scrollbar column is no row", rowAtPoint(blk, 8, static_cast<float>(100 + 900 - BlockPadX - 5), y0 + 10.0f, true), -1);
        kt::CheckEq("hover: the same x is row 0 without a scrollbar", rowAtPoint(blk, 8, static_cast<float>(100 + 900 - BlockPadX - 5), y0 + 10.0f, false), 0);
        char b[96];
        fmtSbClick(b, sizeof b, SbPart::PageDown, 9, 17);
        kt::Check("SBCLICK line", strcmp(b, "SBCLICK part=pagedown off=9/17") == 0);
    }

    return kt::Report("quick_loot_logic_test");
}
