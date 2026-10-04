// r10_probe_test.cpp -- host test of R10Probe.h (R10 PC level-up filter, log-only probe state machine; doc 25 section 8).
#include "R10Probe.h"
#include "check.h"
#include <string.h>
#include <string>
#include <vector>

static std::vector<std::string> g_out;
static void sink(void*, const char* l) { g_out.push_back(l); }
static r10::State mk(bool on, bool verbose = false) { g_out.clear(); r10::State s; s.on = on; s.verbose = verbose; s.emit = sink; return s; }
static r10::PanelInfo info(uint32_t panel) { r10::PanelInfo i; i.panel = panel; i.bonus = 1; i.client = 0x1111; i.server = 0x2222; i.tag = "atton"; i.ispc = 0; i.stats6c = 1; i.lvl = 5; i.picks = 2; return i; }
static bool has(const char* s) { for (auto& l : g_out) if (l == s) return true; return false; }

int main() {
    using namespace r10;
    { // gate off: nothing, no state
        State s = mk(false, true);
        onRow(s, Feats, 1); onChoosable(s, Feats, 0x1000, 1, nullptr); onRow(s, Powers, 2); flush(s);
        KT_CHECK(g_out.empty());
        KT_CHECK(!s.active && s.lines == 0);
        KT_CHECK(!wantInfo(s, Feats, 0x1000));
    }
    { // first call logs the panel line once; counters; flush on close
        State s = mk(true);
        PanelInfo i = info(0xABC);
        KT_CHECK(wantInfo(s, Feats, 0xABC));
        onChoosable(s, Feats, 0xABC, 10, &i);
        KT_CHECK(!wantInfo(s, Feats, 0xABC));
        KT_CHECK(wantInfo(s, Feats, 0xABD));
        KT_CHECK(wantInfo(s, Powers, 0xABC));
        onChoosable(s, Feats, 0xABC, 11, nullptr); onChoosable(s, Feats, 0xABC, 12, nullptr);
        KT_CHECK_EQ((int)g_out.size(), 1);
        KT_CHECK(g_out[0] == "R10 panel kind=feats panel=00000abc bonus=1 client=00001111 server=00002222 tag=\"atton\" ispc=0 stats6c=1 lvl=5 picks=2");
        flush(s);
        KT_CHECK_EQ((int)g_out.size(), 2);
        KT_CHECK(g_out[1] == "R10 done kind=feats panel=00000abc rows=0 choosable_seen=3");
        flush(s);
        KT_CHECK_EQ((int)g_out.size(), 2);   // second flush is a no-op
    }
    { // F1/P1 before any panel: rows are adopted by the first choosable call of the same kind
        State s = mk(true);
        for (int k = 0; k < 5; ++k) onRow(s, Feats, k);
        KT_CHECK(g_out.empty());
        PanelInfo i = info(0x500);
        KT_CHECK(wantInfo(s, Feats, 0x500));
        onChoosable(s, Feats, 0x500, 1, &i); onChoosable(s, Feats, 0x500, 2, nullptr);
        flush(s);
        KT_CHECK_EQ((int)g_out.size(), 2);
        KT_CHECK(g_out[1] == "R10 done kind=feats panel=00000500 rows=5 choosable_seen=2");
    }
    { // rows only (no choosable ever): done line carries panel=0
        State s = mk(true);
        onRow(s, Powers, 7); onRow(s, Powers, 8); flush(s);
        KT_CHECK_EQ((int)g_out.size(), 1);
        KT_CHECK(g_out[0] == "R10 done kind=powers panel=00000000 rows=2 choosable_seen=0");
    }
    { // flush on a new panel pointer; grid rows of the second panel start a fresh session
        State s = mk(true);
        PanelInfo a = info(0xA0), b = info(0xB0);
        onRow(s, Feats, 1); onChoosable(s, Feats, 0xA0, 1, &a); onChoosable(s, Feats, 0xA0, 2, nullptr);
        onRow(s, Feats, 1); onRow(s, Feats, 2);   // second panel's grid (same kind, previous session had choosables)
        KT_CHECK(has("R10 done kind=feats panel=000000a0 rows=1 choosable_seen=2"));
        onChoosable(s, Feats, 0xB0, 1, &b);
        flush(s);
        KT_CHECK(has("R10 done kind=feats panel=000000b0 rows=2 choosable_seen=1"));
        // different pointer with no rows in between also flushes
        State t = mk(true);
        onChoosable(t, Feats, 0xA0, 1, &a); onChoosable(t, Feats, 0xB0, 1, &b);
        KT_CHECK(has("R10 done kind=feats panel=000000a0 rows=0 choosable_seen=1"));
        KT_CHECK(t.active && t.panel == 0xB0 && t.seen == 1);
    }
    { // kind switch flushes (grid of powers arrives while a feats session is open)
        State s = mk(true);
        PanelInfo a = info(0xA0), p = info(0xC0);
        onRow(s, Feats, 1); onChoosable(s, Feats, 0xA0, 1, &a);
        onRow(s, Powers, 5);
        KT_CHECK(has("R10 done kind=feats panel=000000a0 rows=1 choosable_seen=1"));
        onChoosable(s, Powers, 0xC0, 5, &p);
        KT_CHECK(has("R10 panel kind=powers panel=000000c0 bonus=1 client=00001111 server=00002222 tag=\"atton\" ispc=0 stats6c=1 lvl=5 picks=2"));
        onChoosable(s, Feats, 0xA0, 1, &a);   // kind switch from the choosable side
        KT_CHECK(has("R10 done kind=powers panel=000000c0 rows=1 choosable_seen=1"));
    }
    { // unreadable info: -1 values, tag quotes sanitised, long tag truncated
        State s = mk(true);
        PanelInfo i; i.panel = 0x77; i.tag = "a\"b";
        onChoosable(s, Powers, 0x77, 1, &i);
        KT_CHECK(g_out[0] == "R10 panel kind=powers panel=00000077 bonus=0 client=00000000 server=00000000 tag=\"a'b\" ispc=-1 stats6c=-1 lvl=-1 picks=-1");
        State n = mk(true);
        onChoosable(n, Feats, 0x78, 1, nullptr);   // null info tolerated
        KT_CHECK_EQ((int)g_out.size(), 1);
    }
    { // verbose per-id lines only when verbose
        State q = mk(true, false);
        PanelInfo i = info(0x10);
        onRow(q, Feats, 3); onChoosable(q, Feats, 0x10, 4, &i);
        KT_CHECK_EQ((int)g_out.size(), 1);   // panel line only
        State v = mk(true, true);
        onRow(v, Feats, 3); onRow(v, Powers, 9); onChoosable(v, Powers, 0x20, 8, &i); onChoosable(v, Feats, 0x30, 4, &i);
        KT_CHECK(has("R10 id kind=feats site=F1 id=3"));
        KT_CHECK(has("R10 id kind=powers site=P1 id=9"));
        KT_CHECK(has("R10 id kind=powers site=P2 id=8"));
        KT_CHECK(has("R10 id kind=feats site=F2 id=4"));
    }
    { // hard cap: 400 lines per session, id lines stop early so summaries still fit
        State s = mk(true, true);
        PanelInfo i = info(0x40);
        onChoosable(s, Feats, 0x40, 0, &i);
        for (uint32_t k = 0; k < 2000; ++k) onChoosable(s, Feats, 0x40, k, nullptr);
        KT_CHECK_EQ(s.lines, IdLineLimit);
        flush(s);
        KT_CHECK(g_out.back() == "R10 done kind=feats panel=00000040 rows=0 choosable_seen=2001");
        for (int k = 0; k < 2000; ++k) { onChoosable(s, Powers, 0x50 + k, 1, &i); flush(s); }
        KT_CHECK_EQ((int)g_out.size(), MaxLines);
        KT_CHECK(g_out.back() == "R10 line cap reached");
        KT_CHECK_EQ(s.lines, MaxLines);
    }
    return kt::Report("r10_probe_test");
}
