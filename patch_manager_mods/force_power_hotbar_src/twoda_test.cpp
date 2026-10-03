// twoda_test: hb::TwoDA against testdata/spells_fixture.2da (default; cwd = this dir), or a real
// spells.2da given as argv[1] (real-data mode: fixture-specific row/col counts are not asserted).
// Optional argv[2] = dialog.tlk enables the TLK step (skipped otherwise).
// Category is stored as a hex string ("0x1808") in the real file; TwoDA returns cells verbatim.
#include "TwoDA.h"
#include "Tlk.h"
#include "check.h"
#include <cstdlib>
#include <cstring>
struct Expect { int row; const char* name; const char* fp; const char* range; const char* category; const char* icon; };
int main(int argc, char** argv) {
    const bool real = argc > 1;
    hb::TwoDA t;
    if (!kt::Check("Load", t.Load(real ? argv[1] : "testdata/spells_fixture.2da"), t.error.c_str())) return kt::Report("twoda_test");
    printf("cols=%d rows=%d\n", (int)t.cols.size(), (int)t.rows.size());
    KT_CHECK_EQ(t.rows.size(), t.labels.size());
    if (!real) {
        KT_CHECK_EQ(t.rows.size(), 43);
        KT_CHECK_EQ(t.cols.size(), 7);
        KT_CHECK_EQ(t.cols[0].c_str(), "label");
        KT_CHECK_EQ(t.cols[6].c_str(), "spelldesc");
    }
    KT_CHECK(t.rows.size() > 42);
    const Expect exp[] = {
        {8,  "267", "25", "P", "0x1808", "ip_speedburst"},
        {10, "287", "30", "T", "0x2441", "ip_cure"},
        {22, "281", "25", "P", "0x1808", "ip_mind"},
        {42, "301", "20", "P", "0x1808", "ip_resistenergy1"},
    };
    for (const Expect& e : exp) {
        printf("row %d name=%s forcepoints=%s range=%s category=%s icon=%s\n", e.row, t.Get(e.row, "name").c_str(), t.Get(e.row, "forcepoints").c_str(), t.Get(e.row, "range").c_str(), t.Get(e.row, "category").c_str(), t.Get(e.row, "iconresref").c_str());
        if (e.row >= (int)t.rows.size()) { KT_CHECK(e.row < (int)t.rows.size()); continue; }
        KT_CHECK_EQ(t.labels[e.row].c_str(), std::to_string(e.row).c_str());   // row label is the row index
        KT_CHECK_EQ(t.Get(e.row, "name").c_str(), e.name);
        KT_CHECK_EQ(t.Get(e.row, "forcepoints").c_str(), e.fp);
        KT_CHECK_EQ(t.Get(e.row, "range").c_str(), e.range);
        KT_CHECK_EQ(t.Get(e.row, "category").c_str(), e.category);
        KT_CHECK_EQ(t.Get(e.row, "iconresref").c_str(), e.icon);
    }
    // Column lookup and the "label" column (spell constant name).
    KT_CHECK_EQ(t.Col("label"), 0);
    KT_CHECK(t.Col("iconresref") > 0);
    KT_CHECK_EQ(t.Get(8, "label").c_str(), "FORCE_POWER_SPEED_BURST");
    KT_CHECK_EQ(t.Get(10, "label").c_str(), "FORCE_POWER_CURE");
    // Not-found contract (TwoDA::Get): unknown column, or row out of range / negative -> "" (Col -> -1).
    KT_CHECK_EQ(t.Col("no_such_column"), -1);
    KT_CHECK_EQ(t.Get(8, "no_such_column").c_str(), "");
    KT_CHECK_EQ(t.Get(-1, "name").c_str(), "");
    KT_CHECK_EQ(t.Get((int)t.rows.size(), "name").c_str(), "");
    KT_CHECK_EQ(t.Get(100000, "name").c_str(), "");
    hb::Tlk k;
    if (argc > 2 && k.Open(argv[2])) {
        for (int r : {8, 10}) { int n = atoi(t.Get(r, "name").c_str()), d = atoi(t.Get(r, "spelldesc").c_str()); printf("tlk name[%d]='%s'\n desc[%d]='%s'\n", n, k.Get(n).c_str(), d, k.Get(d).c_str()); }
    } else printf("tlk skipped: %s\n", k.error.c_str());
    return kt::Report("twoda_test");
}
