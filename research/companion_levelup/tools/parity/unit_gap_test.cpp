// Unit tests added by the A3 mutation gate (mutate.py): pure logic whose edge cases no table-driven parity run reaches (synthetic presets / items,
// the log cap, the HOLD log rate limit, the options and wizard state machines). Run via parity.py.
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <string>
#include <unistd.h>

#include "TestHarness.h"
#include "LuoLogHost.h"
#define CHECK(c, ...) do { \
    char _detail[512]; \
    std::snprintf(_detail, sizeof(_detail), __VA_ARGS__); \
    ::kt::Check(__FILE__ ":" KT_STR(__LINE__) ": " #c, static_cast<bool>(c), _detail); \
} while (0)

static std::string readLog() {
    std::string s; FILE* f = fopen(LogName, "rb"); if (!f) return s;
    char b[4096]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
    fclose(f); return s;
}
// KpLog keeps the fallback file open across remove(): reopen it through the public API so the next write lands in a fresh file.
static void resetLog() { luoHostLogReset(LogName); }
static int countStr(const std::string& s, const char* sub) { int n = 0; for (size_t p = 0; (p = s.find(sub, p)) != std::string::npos; ++p) ++n; return n; }

static wiz::Item mk(const char* id, const char* convert, const char* role, const char* style) {
    wiz::Item it{};
    it.id = id; it.shortName = id;
    it.ans[wiz::Role] = role; it.ans[wiz::Style] = style; it.ans[wiz::Alignment] = "light"; it.ans[wiz::Convert] = convert; it.ans[wiz::Skills] = "combat";
    return it;
}

int main() {
    char dir[] = "/tmp/luo_gap_XXXXXX";
    if (!mkdtemp(dir) || chdir(dir)) return 2;
    luoHostLogInit();

    // ---- levelNow: a record with an unknown class level (clvl 0) never matches and never counts towards "records used up" ----
    {
        static Preset p; memset(&p, 0, sizeof p);
        p.holdClass = 5; p.convertAt = 9;
        p.lv[3].present = true; p.lv[3].cls = 2; p.lv[3].clvl = 0;
        const uint8_t ids[1] = {2}, lv[1] = {3};
        CHECK(levelNow(ids, lv, 1, &p, 4, true, 0, nullptr, nullptr) == Decision::Vanilla, "clvl-0 record: no match, not 'used up' -> VANILLA");
        p.lv[4].present = true; p.lv[4].cls = 2; p.lv[4].clvl = 4;
        const Rec* rec = nullptr; int cl = -1;
        CHECK(levelNow(ids, lv, 1, &p, 4, true, 0, &rec, &cl) == Decision::Path && cl == 4 && rec == &p.lv[4], "known record still matches");
    }

    // ---- log cap (F1): the shared sink (KpLog.h, Common) stops at KP_LOG_FALLBACK_MAX_LINES, keeps ONE raw cap marker, then goes silent ----
    {
        static const char kCapMarker[] = "--- log cap reached ---\n";
        const size_t markerLen = sizeof kCapMarker - 1;
        resetLog(); ::kplog::detail::gFallbackLines.store(0);
        for (uint32_t i = 0; i < KP_LOG_FALLBACK_MAX_LINES + 50u; ++i) KP_LOG(chGate, "cap filler %u", (unsigned)i);
        std::string l = readLog();
        CHECK(countStr(l, "--- log cap reached ---") == 1, "one cap marker");
        CHECK(countStr(l, "cap filler") == (int)KP_LOG_FALLBACK_MAX_LINES && countStr(l, "\n") == (int)KP_LOG_FALLBACK_MAX_LINES + 1, "MAX lines then the marker: %d fillers, %d lines", countStr(l, "cap filler"), countStr(l, "\n"));
        CHECK(l.size() >= markerLen && l.compare(l.size() - markerLen, markerLen, kCapMarker) == 0, "marker is the last line, nothing after");
        KP_LOG(chGate, "cap late line");
        CHECK(readLog() == l, "writes past the cap emit nothing");
        resetLog(); ::kplog::detail::gFallbackLines.store(0);   // the counter is process-global: later blocks need a live sink
    }

    // ---- HOLD/UNHOLD log rate limit (F14): a withheld transition is counted, printed on the next logged line, then reset ----
    {
        resetLog(); g_nHoldState = 0;
        g_stubTick = 1000; noteHold("t", 5, true, 0x1, true);
        g_stubTick = 1100; noteHold("t", 6, true, 0x1, true);      // new key, inside the gap -> suppressed (1)
        g_stubTick = 4000; noteHold("t", 7, true, 0x1, true);      // logged with suppressed=1, counter reset
        g_stubTick = 4100; noteHold("t", 8, true, 0x1, true);      // suppressed again (1)
        g_stubTick = 7000; noteHold("t", 0, false, 0x1, true);     // UNHOLD logged with suppressed=1 (not 2)
        std::string l = readLog();
        CHECK(countStr(l, "HOLD tag=t key=5 site=0x1\n") == 1, "first HOLD logged:\n%s", l.c_str());
        CHECK(l.find("key=6") == std::string::npos && l.find("key=8") == std::string::npos, "withheld keys not logged");
        CHECK(l.find("HOLD tag=t key=7 site=0x1 suppressed=1\n") != std::string::npos, "key 7 line carries suppressed=1:\n%s", l.c_str());
        CHECK(l.find("UNHOLD tag=t suppressed=1\n") != std::string::npos, "UNHOLD carries suppressed=1 (counter was reset):\n%s", l.c_str());
        // same state again: nothing; HOLD on a NEW key after the gap is logged (the key is part of the state)
        resetLog(); g_nHoldState = 0;
        g_stubTick = 20000; noteHold("u", 5, true, 0x2, false);
        g_stubTick = 30000; noteHold("u", 5, true, 0x2, false);
        g_stubTick = 40000; noteHold("u", 6, true, 0x2, false);
        l = readLog();
        CHECK(countStr(l, "WOULD HOLD tag=u key=5") == 1 && countStr(l, "WOULD HOLD tag=u key=6") == 1, "same key once, new key logged:\n%s", l.c_str());
    }

    // ---- HOLD log: a withheld flap that ended back on the logged state is flushed (" suppressed=N") by the first call after the gap, even with no new change ----
    {
        resetLog(); g_nHoldState = 0;
        g_stubTick = 1000; noteHold("f", 5, true, 0x1, true);      // HOLD logged
        g_stubTick = 1100; noteHold("f", 0, false, 0x1, true);     // UNHOLD withheld (suppressed 1)
        g_stubTick = 1200; noteHold("f", 5, true, 0x1, true);      // back on the logged state inside the gap: nothing
        g_stubTick = 1300; noteHold("f", 5, true, 0x1, true);      // still inside the gap
        std::string l = readLog();
        CHECK(countStr(l, "tag=f") == 1, "inside the gap only the first HOLD is logged:\n%s", l.c_str());
        g_stubTick = 4000; noteHold("f", 5, true, 0x1, true);      // gap elapsed, state unchanged since: the pending count is flushed
        l = readLog();
        CHECK(countStr(l, "HOLD tag=f key=5 site=0x1 suppressed=1\n") == 1, "flush line carries suppressed=1:\n%s", l.c_str());
        g_stubTick = 4100; noteHold("f", 5, true, 0x1, true);      // flushed: counter reset, same state -> silent again
        g_stubTick = 9000; noteHold("f", 5, true, 0x1, true);
        CHECK(countStr(readLog(), "tag=f") == 2, "no repeat after the flush:\n%s", readLog().c_str());
        // the unheld direction: UNHOLD logged, a HOLD flap withheld and undone -> flushed as UNHOLD suppressed=1
        resetLog(); g_nHoldState = 0;
        g_stubTick = 20000; noteHold("g", 5, true, 0x1, true);
        g_stubTick = 23000; noteHold("g", 0, false, 0x1, true);    // UNHOLD logged
        g_stubTick = 23100; noteHold("g", 7, true, 0x1, true);     // HOLD withheld (1)
        g_stubTick = 23200; noteHold("g", 0, false, 0x1, true);    // back to unheld inside the gap
        g_stubTick = 26000; noteHold("g", 0, false, 0x1, true);
        l = readLog();
        CHECK(l.find("UNHOLD tag=g suppressed=1\n") != std::string::npos && l.find("key=7") == std::string::npos, "unheld flush:\n%s", l.c_str());
    }

    // ---- table parse: a bad [featrules]/[powerrules] row disables carry-over only (presets, holds, records keep loading; one log line), not the whole table ----
    {
        auto put = [](const char* name, const char* rules) {
            FILE* f = fopen(name, "wb");
            fprintf(f, "[classskills]\ncs2=11111111\n[presets]\nt__x=r|s|a|c|k|8|5|synthetic\n%s[t__x]\nname=S\nL4=2|6|||||4\n", rules);
            fclose(f);
        };
        struct C { const char* what; const char* rules; int line; };
        const C bad[] = {
            {"malformed feat row", "[featrules]\nf3=0|||2\nf4=bad\nf5=0|||2\n", 7},
            {"duplicate feat row", "[featrules]\nf3=0|||2\nf3=0|||2\n", 7},
            {"over-cap req ids", "[featrules]\nf3=0|1,2,3||2\n", 6},
            {"feat class id 16", "[featrules]\nf3=0|||16\n", 6},
            {"power row without colon", "[featrules]\nf3=0|||2\n[powerrules]\np10=|56\np11=|2:0\n", 8},
            {"duplicate power row", "[powerrules]\np10=|2:0\np10=|2:0\n", 7},
            {"power id over cap", "[powerrules]\np320=|2:0\n", 6},
            {"rule row without =", "[powerrules]\np10=|2:0\ngarbage\n", 7},
        };
        for (const C& c : bad) {
            put("t.ini", c.rules);
            resetLog(); g_nHoldState = 0;
            snprintf(g_cfg.table, sizeof g_cfg.table, "t.ini"); g_tablePoll = {}; g_tableStamp = {}; g_tableOk = false; g_cfg.nTags = 0;
            reloadTable(true);
            const std::string l = readLog();
            char want[64]; snprintf(want, sizeof want, "CARRY disabled: malformed rule line %d\n", c.line);
            CHECK(g_tableOk && g_nPresets == 1 && g_presets[0].holdClass == 5 && g_presets[0].convertAt == 8, "%s: presets/index kept (ok %d n %d)", c.what, g_tableOk, g_nPresets);
            CHECK(g_presets[0].lv[4].present && g_cs[2][0], "%s: records and classskills after/before the rules kept", c.what);
            CHECK(!g_rules && !g_fr[3].present && !g_pr[10].present && !g_pr[11].present && g_ruleBadLine == c.line, "%s: carry disabled, rule tables cleared (rules %d line %d)", c.what, g_rules, g_ruleBadLine);
            CHECK(countStr(l, want) == 1 && countStr(l, "CARRY disabled") == 1 && l.find("TABLE loaded") != std::string::npos && l.find("TABLE ERROR") == std::string::npos, "%s: log:\n%s", c.what, l.c_str());
        }
        put("t.ini", "[featrules]\nf3=0|||2\n[powerrules]\np10=|2:0\n");   // control: a good table keeps carry on, no CARRY disabled line
        resetLog(); g_tablePoll = {}; g_tableStamp = {};
        reloadTable(true);
        CHECK(g_tableOk && g_rules && g_fr[3].present && g_pr[10].present && !g_ruleBadLine && countStr(readLog(), "CARRY disabled") == 0, "good rules: carry stays on");
        put("t.ini", "");   // old table (no rule sections): the existing line
        resetLog(); g_tablePoll = {}; g_tableStamp = {};
        reloadTable(true);
        CHECK(g_tableOk && !g_rules && countStr(readLog(), "CARRY disabled: table has no [featrules]/[powerrules] sections") == 1 && countStr(readLog(), "malformed rule line") == 0, "old table line unchanged");
    }

    // ---- OptionsLogic ----
    {
        // a row differing only for item 1 is shown (rowShown must compare item 1 against item 0)
        wiz::Item a[2] = {mk("a", "never", "melee", "single"), mk("b", "early", "melee", "single")};
        opt::State s; opt::init(s, a, 2, 0);
        CHECK(opt::shown(s, 0) && !opt::shown(s, 1), "2 items differing on Convert only: Convert shown, Role hidden");
        // tie-break: cycling Convert prefers the candidate that keeps the next row (Role)
        wiz::Item b[3] = {mk("b0", "never", "melee", "single"), mk("b1", "early", "ranged", "single"), mk("b2", "early", "melee", "single")};
        opt::init(s, b, 3, 0);
        CHECK(opt::cycle(s, 0, +1) && s.cur == 2, "cycle keeps Role: cur %d", s.cur);
        // Variant cycle honours the direction
        wiz::Item c[3] = {mk("c0", "early", "melee", "single"), mk("c1", "early", "melee", "single"), mk("c2", "early", "melee", "single")};
        opt::init(s, c, 3, 1);
        CHECK(opt::shown(s, opt::VariantRow), "variant shown for 3 identical presets");
        CHECK(opt::cycle(s, opt::VariantRow, -1) && s.cur == 0, "variant -1: cur %d", s.cur);
        CHECK(opt::cycle(s, opt::VariantRow, +1) && s.cur == 1, "variant +1: cur %d", s.cur);
        CHECK(opt::cycle(s, opt::VariantRow, -1) && opt::cycle(s, opt::VariantRow, -1) && s.cur == 2, "variant wraps backwards: cur %d", s.cur);
        // two presets, one answer set: Variant hidden (needs > 1 identical, shown only with >= 2)
        wiz::Item d2[2] = {mk("d0", "early", "melee", "single"), mk("d1", "never", "melee", "single")};
        opt::init(s, d2, 2, 0);
        CHECK(!opt::shown(s, opt::VariantRow), "variant hidden when no identical presets");
        // a row cycle with direction -1 goes to the previous value
        wiz::Item e[3] = {mk("e0", "early", "melee", "single"), mk("e1", "late", "melee", "single"), mk("e2", "never", "melee", "single")};
        opt::init(s, e, 3, 1);
        CHECK(opt::cycle(s, 0, -1) && s.cur == 0, "row -1: cur %d", s.cur);
        opt::init(s, e, 3, 1);
        CHECK(opt::cycle(s, 0, +1) && s.cur == 2, "row +1: cur %d", s.cur);
    }

    // ---- WizardLogic ----
    {
        wiz::State w; wiz::reset(w, nullptr, 0);
        wiz::View v = wiz::view(w);
        CHECK(!v.result && v.page == -1 && v.nOptions == 0, "no candidates: neither a result nor a page (page %d)", v.page);
        wiz::Item it[2] = {mk("w0", "early", "melee", "single"), mk("w1", "never", "melee", "single")};
        wiz::reset(w, it, 2);
        CHECK(!wiz::back(w) && w.depth == 0, "back() at depth 0 is a no-op (depth %d)", w.depth);
        // a Pick answer pins the exact item
        wiz::Item same[2] = {mk("p0", "early", "melee", "single"), mk("p1", "early", "melee", "single")};
        wiz::reset(w, same, 2);
        v = wiz::view(w);
        CHECK(v.page == wiz::Pick && v.nOptions == 2, "identical presets -> Pick page");
        CHECK(wiz::choose(w, 1), "choose on the Pick page");
        v = wiz::view(w);
        CHECK(v.result && v.resultItem == 1, "Pick answer selects exactly that item (%d)", v.resultItem);
    }

    return kt::Report("unit_gap_test");
}
