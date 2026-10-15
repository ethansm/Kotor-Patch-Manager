// twoda_test: hb::TwoDA against testdata/spells_fixture.2da (default; cwd = this dir), or a real
// spells.2da given as argv[1] (real-data mode: fixture-specific row/col counts are not asserted).
// Optional argv[2] = dialog.tlk runs the TLK step on it; without argv[2] the step runs on a synthetic dialog.tlk in TMPDIR.
// Category is stored as a hex string ("0x1808") in the real file; TwoDA returns cells verbatim.
#include "TwoDA.h"
#include "Tlk.h"
#include "check.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>

// ---- in-memory binary 2DA builder for malformed-file tests (Parse takes a string, no files needed).
static void le32(std::string& s, uint32_t v) { for (int i = 0; i < 4; ++i) s += (char)((v >> (8 * i)) & 0xFF); }
static void le16(std::string& s, uint32_t v) { s += (char)(v & 0xFF); s += (char)((v >> 8) & 0xFF); }
// cells[r][c] strings are laid out back to back in the data blob. Overrides let a test lie in the header.
static std::string Build2da(const std::vector<std::string>& cols, const std::vector<std::vector<std::string>>& cells,
                            long nrowsOverride = -1, bool colTerm = true, long offOverride = -1) {
    std::string s = "2DA V2.b\n";
    for (const std::string& c : cols) { s += c; s += '\t'; }
    if (colTerm) s += '\0';
    le32(s, nrowsOverride >= 0 ? (uint32_t)nrowsOverride : (uint32_t)cells.size());
    for (size_t r = 0; r < cells.size(); ++r) { s += std::to_string(r); s += '\t'; }
    std::string blob; std::string offs;
    for (const auto& row : cells) for (const std::string& c : row) { le16(offs, offOverride >= 0 ? (uint32_t)offOverride : (uint32_t)blob.size()); blob += c; blob += '\0'; }
    s += offs; le16(s, (uint32_t)blob.size()); s += blob;
    return s;
}
// Synthetic dialog.tlk (V3.0, Tlk.h layout): strref i holds "S<i>". Header 20 bytes, 40-byte entries (text offset at +28, size at +32), then the text.
static std::string BuildTlk(uint32_t count) {
    std::string s = "TLK V3.0"; le32(s, 0); le32(s, count);
    std::string ents, data;
    for (uint32_t i = 0; i < count; ++i) {
        const std::string txt = "S" + std::to_string(i);
        le32(ents, 0); ents += std::string(16, '\0'); le32(ents, 0); le32(ents, 0);   // flags, sound, volVar, pitchVar
        le32(ents, (uint32_t)data.size()); le32(ents, (uint32_t)txt.size()); le32(ents, 0);   // offset, size, soundLen
        data += txt;
    }
    le32(s, 20 + (uint32_t)ents.size());   // stringsOffset: text starts right after the entries
    return s + ents + data;
}
struct Expect { int row; const char* name; const char* fp; const char* range; const char* category; const char* icon; };
int main(int argc, char** argv) {
    const bool real = argc > 1;
    hb::TwoDA t;
    if (!kt::Check("Load", t.Load(real ? argv[1] : "testdata/spells_fixture.2da"), t.error.c_str())) return kt::Report("twoda_test");
    printf("cols=%d rows=%d\n", (int)t.cols.size(), (int)t.rows.size());
    KT_CHECK_EQ(t.rows.size(), t.labels.size());
    if (!real) {
        KT_CHECK_EQ(t.rows.size(), 43);
        KT_CHECK_EQ(t.cols.size(), 9);   // + usertype, hostilesetting (HB09)
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

    // ================= malformed / edge-case Parse tests =================
    {
        const std::vector<std::string> C = {"Label", "Name"};
        const std::vector<std::vector<std::string>> R = {{"a", "1"}, {"b", "2"}};
        const std::string good = Build2da(C, R);
        hb::TwoDA m;
        KT_CHECK(m.Parse(good));
        KT_CHECK(m.labels[0] == "0" && m.labels[1] == "1");                      // the first label must not swallow the row-count's top byte
        for (const auto& row : m.rows) KT_CHECK_EQ(row.size(), m.cols.size());   // exactly one cell per column
        KT_CHECK_EQ(m.rows.size(), 2); KT_CHECK_EQ(m.Get(1, "Name").c_str(), "2"); KT_CHECK_EQ(m.labels[1].c_str(), "1");
        // < 16 bytes, bad magic.
        KT_CHECK(!m.Parse(std::string())); KT_CHECK_EQ(m.error.c_str(), "not a binary 2DA V2.b");
        KT_CHECK(!m.Parse(good.substr(0, 15))); KT_CHECK_EQ(m.error.c_str(), "not a binary 2DA V2.b");   // 15 bytes is still "too short" (not a later parse error)
        KT_CHECK(!m.Parse(std::string("2DA V2.b\n") + "abc"));                 // 12 bytes
        { std::string b = good; b[0] = 'X'; KT_CHECK(!m.Parse(b)); }
        { std::string b = good; b.replace(0, 8, "2DA V2.0"); KT_CHECK(!m.Parse(b)); }   // ASCII 2DA unsupported
        { std::string b = good; b.replace(0, 8, "2da v2.b"); KT_CHECK(!m.Parse(b)); }
        // Header without \t terminator / without the \0 list terminator.
        { std::string b = "2DA V2.b\nLabel Name"; b.append(8, 'x'); KT_CHECK(!m.Parse(b)); KT_CHECK_EQ(m.error.c_str(), "bad column header"); }
        KT_CHECK(!m.Parse(Build2da(C, R, -1, false)));                           // no \0: counts read as column text, no \t found
        { std::string b = "2DA V2.b\nLabel\tName\t"; KT_CHECK(!m.Parse(b + std::string(1, '\0'))); }   // ends right after terminator (truncated)
        // Truncated counts/offsets.
        { std::string b = good.substr(0, 9 + 12 + 1 + 2); KT_CHECK(!m.Parse(b)); }    // cut inside the row-count field
        KT_CHECK_EQ(m.error.c_str(), "truncated");
        { size_t labelEnd = 9 + 12 + 1 + 4 + 4; KT_CHECK(!m.Parse(good.substr(0, labelEnd + 3))); KT_CHECK_EQ(m.error.c_str(), "truncated offsets"); }
        KT_CHECK(m.Parse(good.substr(0, good.size() - 3)));                     // data blob cut short: unreachable cells -> ""
        KT_CHECK_EQ(m.Get(1, "Label").c_str(), "b"); KT_CHECK_EQ(m.Get(1, "Name").c_str(), "");
        { std::string b = good.substr(0, 9 + 12 + 1 + 4 + 2); KT_CHECK(!m.Parse(b)); }   // cut inside row labels
        KT_CHECK_EQ(m.error.c_str(), "bad row label");
        // nrows limits.
        KT_CHECK(!m.Parse(Build2da(C, R, 100001))); KT_CHECK_EQ(m.error.c_str(), "implausible row count");
        KT_CHECK(!m.Parse(Build2da(C, R, 0xFFFFFFFFl)));
        KT_CHECK(!m.Parse(Build2da(C, R, 100000))); KT_CHECK_EQ(m.error.c_str(), "bad row label");   // allowed count (limit is > 100000), data far too short
        KT_CHECK(!m.Parse(Build2da(C, R, 65536))); KT_CHECK_EQ(m.error.c_str(), "bad row label");      // byte 2 of the count counts (not implausible, just truncated)
        KT_CHECK(!m.Parse(Build2da(C, R, 0x01000000l))); KT_CHECK_EQ(m.error.c_str(), "implausible row count");   // byte 3 of the count counts
        KT_CHECK(!m.Parse(Build2da(C, R, 0x80000000l))); KT_CHECK_EQ(m.error.c_str(), "implausible row count");
        KT_CHECK(!m.Parse(Build2da(C, R, 3)));                                   // header claims one row more than exists
        // Exact size boundaries: header + labels + offsets + the 2-byte data size, no blob, is the smallest valid file.
        { const size_t minSize = good.size() - 8;                                 // blob "a\0" "1\0" "b\0" "2\0" = 8 bytes
          KT_CHECK(m.Parse(good.substr(0, minSize))); KT_CHECK_EQ(m.rows.size(), 2); KT_CHECK_EQ(m.Get(0, "Label").c_str(), "");
          KT_CHECK(!m.Parse(good.substr(0, minSize - 1))); KT_CHECK_EQ(m.error.c_str(), "truncated offsets"); }
        // Row count field exactly at the end of the file: not "truncated" (the 4 bytes are there), but there is no room for the offsets.
        { std::string b = "2DA V2.b\nA\t"; b += '\0'; le32(b, 0); KT_CHECK_EQ(b.size(), 16);
          KT_CHECK(!m.Parse(b)); KT_CHECK_EQ(m.error.c_str(), "truncated offsets"); }
        // Cell offset past end -> "" (not a crash).
        KT_CHECK(m.Parse(Build2da(C, R, -1, true, 0xFFFF)));
        KT_CHECK_EQ(m.Get(0, "Name").c_str(), ""); KT_CHECK_EQ(m.Get(1, "Label").c_str(), "");
        // Cell offset exactly at the end of the data blob (== d.size()) -> "".
        KT_CHECK(m.Parse(Build2da({"A"}, {{"x"}}, -1, true, 2))); KT_CHECK_EQ(m.Get(0, "A").c_str(), "");
        // 0 rows with columns; 0 cols with rows; both zero.
        KT_CHECK(m.Parse(Build2da(C, {})));
        KT_CHECK_EQ(m.cols.size(), 2); KT_CHECK_EQ(m.rows.size(), 0); KT_CHECK_EQ(m.labels.size(), 0);
        KT_CHECK_EQ(m.Get(0, "Name").c_str(), "");
        KT_CHECK(m.Parse(Build2da({}, {{}, {}}, 2)));                            // 0 columns, 2 rows
        KT_CHECK_EQ(m.cols.size(), 0); KT_CHECK_EQ(m.rows.size(), 2); KT_CHECK_EQ(m.labels.size(), 2);
        KT_CHECK_EQ(m.Get(0, "Name").c_str(), ""); KT_CHECK_EQ(m.Col("x"), -1);
        KT_CHECK(m.Parse(Build2da({}, {})));
        KT_CHECK_EQ(m.rows.size(), 0);
        // Load: missing file.
        KT_CHECK(!m.Load("testdata/no_such_file.2da")); KT_CHECK_EQ(m.error.c_str(), "cannot open");
        KT_CHECK_EQ(m.cols.size(), 0);
    }

    // ================= HB-1d-01: mid-blob cell with no NUL reads to the next NUL (TwoDA.h:88) =================
    {
        // Blob "abcd\0": cell A (offset 0) is "ab" with no terminator; cell B (offset 2) is "cd\0". Current merge: A = "abcd".
        std::string b = "2DA V2.b\nA\tB\t"; b += '\0';
        le32(b, 1); b += "0\t";                                  // one row, label "0"
        le16(b, 0); le16(b, 2); le16(b, 5);                      // cell offsets 0 and 2, data size 5
        b += "abcd"; b += '\0';
        hb::TwoDA m; KT_CHECK(m.Parse(b));
        KT_CHECK_EQ(m.rows.size(), 1);
        KT_CHECK_EQ(m.Get(0, "A").c_str(), "abcd");   // a reader capped at the next offset would give "ab"
        KT_CHECK_EQ(m.Get(0, "B").c_str(), "cd");
    }
    // ================= HB-1d-02: 43000 columns x 100000 rows: the cells-size bound (TwoDA.h:80) must fire =================
    {
        const std::vector<std::string> wide(43000, "c");
        const std::vector<std::vector<std::string>> rows(100000);   // all 100000 labels present, no cell data ("bad row label" cannot fire first)
        hb::TwoDA m;
        KT_CHECK(!m.Parse(Build2da(wide, rows)));
        KT_CHECK_EQ(m.error.c_str(), "truncated offsets");          // 43000 * 100000 cells cannot fit in the file
    }
    // ================= Parse is self-contained: a second Parse replaces, not appends =================
    {
        const std::string a = Build2da({"Label", "Name"}, {{"a", "1"}, {"b", "2"}});
        const std::string b = Build2da({"X"}, {{"z"}});
        hb::TwoDA m;
        KT_CHECK(m.Parse(a)); KT_CHECK(m.Parse(b));
        KT_CHECK_EQ(m.cols.size(), 1); KT_CHECK_EQ(m.rows.size(), 1); KT_CHECK_EQ(m.labels.size(), 1);
        KT_CHECK_EQ(m.Get(0, "X").c_str(), "z");
        KT_CHECK_EQ(m.Col("Name"), -1);
        KT_CHECK(m.Parse(b));                                                    // same content twice: still 1 row
        KT_CHECK_EQ(m.rows.size(), 1); KT_CHECK_EQ(m.cols.size(), 1);
        // Stale error cleared by a good Parse.
        KT_CHECK(!m.Parse("junk")); KT_CHECK(m.Parse(a)); KT_CHECK_EQ(m.error.c_str(), "");
        // A failed Parse leaves no partial state behind.
        KT_CHECK(m.Parse(a)); KT_CHECK(!m.Parse(Build2da({"Q"}, {{"1"}}, 100001)));
        KT_CHECK_EQ(m.cols.size(), 0); KT_CHECK_EQ(m.rows.size(), 0); KT_CHECK_EQ(m.labels.size(), 0);
        // Load twice on the fixture: no accumulation either.
        hb::TwoDA f; KT_CHECK(f.Load("testdata/spells_fixture.2da")); KT_CHECK(f.Load("testdata/spells_fixture.2da"));
        KT_CHECK_EQ(f.rows.size(), 43); KT_CHECK_EQ(f.cols.size(), 9);
    }

    // ================= Col is case-insensitive (engine 2DA column names are), first match wins =================
    {
        hb::TwoDA m; KT_CHECK(m.Parse(Build2da({"Label", "ForcePoints", "SPELLDESC", "dup", "DUP"}, {{"l", "25", "d", "first", "second"}})));
        KT_CHECK_EQ(m.Col("Label"), 0); KT_CHECK_EQ(m.Col("label"), 0); KT_CHECK_EQ(m.Col("LABEL"), 0);
        KT_CHECK_EQ(m.Col("forcepoints"), 1); KT_CHECK_EQ(m.Col("FORCEPOINTS"), 1);
        KT_CHECK_EQ(m.Col("spelldesc"), 2); KT_CHECK_EQ(m.Col("SpellDesc"), 2);
        KT_CHECK_EQ(m.Col("dup"), 3); KT_CHECK_EQ(m.Col("Dup"), 3);              // first of duplicates
        KT_CHECK_EQ(m.Col("labe"), -1); KT_CHECK_EQ(m.Col("labels"), -1); KT_CHECK_EQ(m.Col(""), -1);
        KT_CHECK_EQ(m.Col(nullptr), -1);
        KT_CHECK_EQ(m.Get(0, "forcePOINTS").c_str(), "25");
        KT_CHECK_EQ(m.Get(0, "SpellDesc").c_str(), "d");
        { hb::TwoDA z; KT_CHECK(z.Parse(Build2da({"Z", "a"}, {{"1", "2"}})));      // 'Z' is the last upper-case letter folded
          KT_CHECK_EQ(z.Col("z"), 0); KT_CHECK_EQ(z.Col("Z"), 0); KT_CHECK_EQ(z.Col("A"), 1); }
        // The real fixture, with the exact names ActionHotbar.cpp uses, resolves to the same columns either way.
        for (const char* n : {"name", "forcepoints", "range", "category", "iconresref", "spelldesc", "label"}) {
            std::string up = n; for (char& ch : up) ch = (char)toupper((unsigned char)ch);
            KT_CHECK_EQ(t.Col(up.c_str()), t.Col(n)); KT_CHECK(t.Col(n) >= 0);
        }
    }
    // ================= Load reads the whole file, including a final 1-byte chunk (file size = 64 KiB + 1) =================
    {
        std::string probe = Build2da({"A"}, {{"Q"}}); probe.pop_back();             // no trailing NUL: the last cell ends at EOF
        const size_t pad = 65537 - probe.size();
        std::string big = Build2da({"A"}, {{std::string(pad, 'p') + "Q"}}); big.pop_back();
        KT_CHECK_EQ(big.size(), 65537);
        const char* td = getenv("TMPDIR"); std::string path = std::string(td && *td ? td : "/tmp") + "/twoda_big_" + std::to_string((long)getpid()) + ".2da";
        FILE* f = fopen(path.c_str(), "wb"); KT_CHECK(f != nullptr);
        if (f) { fwrite(big.data(), 1, big.size(), f); fclose(f); }
        hb::TwoDA m; KT_CHECK(m.Load(path.c_str()));
        KT_CHECK(m.Get(0, "A") == std::string(pad, 'p') + "Q");
        remove(path.c_str());
    }
    // TLK step. argv[2] = real dialog.tlk: must open, and the spell name/desc strrefs must resolve to non-empty text.
    if (argc > 2) {
        hb::Tlk k;
        if (!kt::Check("tlk Open", k.Open(argv[2]), k.error.c_str())) return kt::Report("twoda_test");
        for (int r : {8, 10}) { int n = atoi(t.Get(r, "name").c_str()), d = atoi(t.Get(r, "spelldesc").c_str()); printf("tlk name[%d]='%s'\n desc[%d]='%s'\n", n, k.Get(n).c_str(), d, k.Get(d).c_str()); KT_CHECK(!k.Get(n).empty()); KT_CHECK(!k.Get(d).empty()); }
    } else {
        // Default gate (no argv): synthetic TLK sized to cover every strref the two spell rows use, so this step always runs.
        uint32_t count = 1;
        for (int r : {8, 10}) for (const char* col : {"name", "spelldesc"}) { const uint32_t ref = (uint32_t)atoi(t.Get(r, col).c_str()); if (ref + 1 > count) count = ref + 1; }
        const char* td = getenv("TMPDIR"); std::string path = std::string(td && *td ? td : "/tmp") + "/twoda_tlk_" + std::to_string((long)getpid()) + ".tlk";
        const std::string tlk = BuildTlk(count);
        FILE* f = fopen(path.c_str(), "wb"); KT_CHECK(f != nullptr);
        if (f) { fwrite(tlk.data(), 1, tlk.size(), f); fclose(f); }
        hb::Tlk k;
        if (kt::Check("synthetic tlk Open", k.Open(path.c_str()), k.error.c_str())) {
            for (int r : {8, 10}) for (const char* col : {"name", "spelldesc"}) {
                const uint32_t ref = (uint32_t)atoi(t.Get(r, col).c_str());
                KT_CHECK(k.Get(ref) == "S" + std::to_string(ref));   // exact text: an off-by-one in entry or text offset fails here
            }
        }
        remove(path.c_str());
    }
    return kt::Report("twoda_test");
}
