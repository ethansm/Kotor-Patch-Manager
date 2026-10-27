// quick_loot_logic_test.cpp -- host test of QuickLootLogic.h (doc 52 QL0): event filter, wrap-safe ms, item cap, tracker, formatters.
#include "QuickLootLogic.h"
#include "check.h"
#include <string.h>

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
    char tiny[8];
    fmtShow(tiny, sizeof tiny, 1, 1, 1, 1, 1);
    kt::Check("formatter truncates safely into a small buffer", strlen(tiny) == 7);

    return kt::Report("quick_loot_logic_test");
}
