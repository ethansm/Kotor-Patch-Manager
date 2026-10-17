// Host test of CombatTrace.h (ctrace): CSTATE/ctx goldens, bounded truncation, step truth table, change-gated capped Tracker.
#include "CombatTrace.h"
#include "check.h"
#include <string.h>
#include <string>
#include <vector>

namespace {

std::vector<std::string> g_lines;
void sink(const char* line) { g_lines.push_back(line); }

ctrace::CombatState S(int c, int p, int m) { ctrace::CombatState s; s.combat = (int8_t)c; s.paused = (int8_t)p; s.menu = (int8_t)m; return s; }

std::string fmtS(const ctrace::CombatState& a, const ctrace::CombatState& b) { char buf[128]; ctrace::fmtCState(buf, sizeof buf, a, b); return buf; }

}  // namespace

int main() {
    using namespace ctrace;

    // fmtCState goldens.
    KT_CHECK_EQ(fmtS(S(-1, -1, -1), S(0, 0, 0)).c_str(), "CSTATE combat=?>0 paused=?>0 menu=?>0");
    KT_CHECK_EQ(fmtS(S(0, 0, 0), S(1, 0, 0)).c_str(), "CSTATE combat=0>1 paused=0>0 menu=0>0");
    KT_CHECK_EQ(fmtS(S(1, 1, 0), S(1, 0, 1)).c_str(), "CSTATE combat=1>1 paused=1>0 menu=0>1");
    KT_CHECK_EQ(fmtS(S(-1, -1, -1), S(-1, -1, -1)).c_str(), "CSTATE combat=?>? paused=?>? menu=?>?");
    KT_CHECK_EQ(fmtS(S(5, 2, -7), S(0, 1, 9)).c_str(), "CSTATE combat=?>0 paused=?>1 menu=?>?");   // out-of-range -> '?'

    // fmtCtx goldens.
    { char b[64]; int n = fmtCtx(b, sizeof b, S(1, 0, -1)); KT_CHECK_EQ(b, " [c=1 p=0 m=?]"); KT_CHECK_EQ(n, 14); }
    { char b[64]; fmtCtx(b, sizeof b, S(-1, -1, -1)); KT_CHECK_EQ(b, " [c=? p=? m=?]"); }

    // Truncation: always NUL-terminated, length = strlen, cap<=0 -> 0 and untouched.
    {
        char b[16]; memset(b, 'Z', sizeof b);
        KT_CHECK_EQ(fmtCState(b, 1, S(0, 0, 0), S(1, 1, 1)), 0);
        KT_CHECK_EQ((int)b[0], 0);
        KT_CHECK_EQ(b[1], 'Z');
        memset(b, 'Z', sizeof b);
        int n = fmtCState(b, 8, S(0, 0, 0), S(1, 1, 1));
        KT_CHECK_EQ(n, 7);
        KT_CHECK_EQ(b, "CSTATE ");
        KT_CHECK_EQ(b[8], 'Z');
        memset(b, 'Z', sizeof b);
        KT_CHECK_EQ(fmtCState(b, 0, S(0, 0, 0), S(1, 1, 1)), 0);
        KT_CHECK_EQ(fmtCState(b, -3, S(0, 0, 0), S(1, 1, 1)), 0);
        KT_CHECK_EQ(b[0], 'Z');
        memset(b, 'Z', sizeof b);
        KT_CHECK_EQ(fmtCtx(b, 1, S(0, 0, 0)), 0);
        KT_CHECK_EQ((int)b[0], 0);
        memset(b, 'Z', sizeof b);
        n = fmtCtx(b, 8, S(0, 0, 0));
        KT_CHECK_EQ(n, 7);
        KT_CHECK_EQ(b, " [c=0 p");
        KT_CHECK_EQ(b[8], 'Z');
        KT_CHECK_EQ(fmtCtx(b, 0, S(0, 0, 0)), 0);
    }

    // combatStateStep truth table.
    {
        CombatState prev = S(0, 0, 0);
        KT_CHECK(!combatStateStep(prev, S(0, 0, 0)));
        KT_CHECK(combatStateStep(prev, S(1, 0, 0)));
        KT_CHECK_EQ((int)prev.combat, 1);
        KT_CHECK(!combatStateStep(prev, S(1, 0, 0)));
        KT_CHECK(combatStateStep(prev, S(1, 1, 0)));
        KT_CHECK_EQ((int)prev.paused, 1);
        KT_CHECK(combatStateStep(prev, S(1, 1, 1)));
        KT_CHECK_EQ((int)prev.menu, 1);
        KT_CHECK(combatStateStep(prev, S(-1, 1, 1)));   // known -> unknown is a change
        KT_CHECK_EQ((int)prev.combat, -1);
        KT_CHECK(!combatStateStep(prev, S(-1, 1, 1)));
        CombatState u;   // default is all unknown
        KT_CHECK(!combatStateStep(u, S(-1, -1, -1)));
    }

    // Tracker: first tick emits, identical silent, flip emits.
    {
        Tracker t; g_lines.clear();
        tick(t, S(0, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 1);
        KT_CHECK_EQ(g_lines[0].c_str(), "CSTATE combat=?>0 paused=?>0 menu=?>0");
        tick(t, S(0, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 1);
        tick(t, S(1, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 2);
        KT_CHECK_EQ(g_lines[1].c_str(), "CSTATE combat=0>1 paused=0>0 menu=0>0");
        tick(t, S(1, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 2);
        KT_CHECK_EQ(t.lines, 2);
    }
    // Tracker: all-unknown first tick still emits once.
    {
        Tracker t; g_lines.clear();
        tick(t, S(-1, -1, -1), sink);
        tick(t, S(-1, -1, -1), sink);
        KT_CHECK_EQ((int)g_lines.size(), 1);
        KT_CHECK_EQ(g_lines[0].c_str(), "CSTATE combat=?>? paused=?>? menu=?>?");
    }
    // Tracker cap=2: 2 lines, exactly one cap marker, then silence; prev keeps updating.
    {
        Tracker t; t.cap = 2; g_lines.clear();
        tick(t, S(0, 0, 0), sink);
        tick(t, S(1, 0, 0), sink);
        tick(t, S(0, 0, 0), sink);
        tick(t, S(1, 1, 1), sink);
        tick(t, S(0, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 3);
        KT_CHECK_EQ(g_lines[2].c_str(), "CSTATE cap reached");
        KT_CHECK_EQ((int)t.prev.combat, 0);
        tick(t, S(1, 0, 0), sink);
        KT_CHECK_EQ((int)g_lines.size(), 3);
        KT_CHECK_EQ((int)t.prev.combat, 1);
    }
    // Null sink is safe.
    { Tracker t; tick(t, S(0, 0, 0), nullptr); KT_CHECK(t.primed); }

    return kt::Report("combat_trace_test");
}
