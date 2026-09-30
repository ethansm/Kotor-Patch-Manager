#include "TwoDA.h"
#include "Tlk.h"
#include <cstdlib>
int main(int argc, char** argv) {
    hb::TwoDA t; if (!t.Load(argc > 1 ? argv[1] : "spells.2da")) { printf("FAIL %s\n", t.error.c_str()); return 1; }
    printf("cols=%d rows=%d\n", (int)t.cols.size(), (int)t.rows.size());
    for (int r : {8, 10, 22, 42}) printf("row %d label=%s name=%s forcepoints=%s range=%s category=%s icon=%s\n", r, t.labels[r].c_str(), t.Get(r, "name").c_str(), t.Get(r, "forcepoints").c_str(), t.Get(r, "range").c_str(), t.Get(r, "category").c_str(), t.Get(r, "iconresref").c_str());
    hb::Tlk k; if (argc > 2 && k.Open(argv[2])) {
        for (int r : {8, 10}) { int n = atoi(t.Get(r, "name").c_str()), d = atoi(t.Get(r, "spelldesc").c_str()); printf("tlk name[%d]='%s'\n desc[%d]='%s'\n", n, k.Get(n).c_str(), d, k.Get(d).c_str()); }
    } else printf("tlk skipped: %s\n", k.error.c_str());
    return t.rows.size() > 42 ? 0 : 1;
}
