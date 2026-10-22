// slow_holster_page_test.cpp -- host test of HolsterSlowPage.h: the shipped schema parses to one slider stepper row bound to the flat holster.ini
// key slow_holster_x (no section) with min 1 / max 8 / step 1 / default 1, every schema string fits the reader's caps, the start value and
// change paths read and write that key as a clamped integer, a build against a fake k2.menuui writes nothing, and the 64 KB reader finds a key
// that sits past the stock reader's 16 KB cut. Fake ini read / write stand in for the file (the host test never touches the real holster.ini).
#define INIW_FILE_API 1   // mcdecl::fileRead (the stock 16 KB reader) is compared against hsp::readIni below
#define MCDECL_FILE_API 1
#include "HolsterSlowPage.h"
#include "ModConfigDecl.h"
#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace {

// ---- fake holster.ini: one flat key, the value a read returns, and the writes
bool g_iniHas = false;
char g_iniVal[16] = "";
struct Write { char path[32], key[48], val[16]; bool flat; };
Write g_writes[8];
int g_nWrites = 0;

void copyTo(char* dst, size_t cap, const char* src) {
    const size_t n = strlen(src) < cap ? strlen(src) : cap - 1;
    memcpy(dst, src, n);
    dst[n] = 0;
}

bool fakeRead(const char* path, const char* section, const char* key, char* out, int cap) {
    if (!g_iniHas || section != nullptr || strcmp(path, hsp::kIniFile) != 0 || strcmp(key, hsp::kIniKey) != 0) return false;
    copyTo(out, static_cast<size_t>(cap), g_iniVal);
    return true;
}

bool fakeWrite(const char* path, const char* section, const char* key, const char* val) {
    if (g_nWrites >= 8) return false;
    Write& w = g_writes[g_nWrites++];
    copyTo(w.path, sizeof w.path, path);
    copyTo(w.key, sizeof w.key, key);
    copyTo(w.val, sizeof w.val, val);
    w.flat = section == nullptr;
    return true;
}

// ---- fake k2.menuui (only what Page::build and the change path touch)
struct FakeRow {
    K2MuiRowSpec spec;
    char label[64], fmt[48];
};
FakeRow g_rows[4];
int g_nRows = 0;
K2MuiChangeFn g_fn = nullptr;
void* g_fnUser = nullptr;
K2MenuUiApi g_table;
k2mc::MenuClient g_client;

uint32_t KPATCH_CALL fNewPage(void*, const char*) { return 9; }
int32_t KPATCH_CALL fAddRow(uint32_t, const K2MuiRowSpec* sp) {
    if (g_nRows >= 4) return -1;
    FakeRow& r = g_rows[g_nRows];
    r.spec = *sp;
    snprintf(r.label, sizeof r.label, "%s", sp->label ? sp->label : "");
    snprintf(r.fmt, sizeof r.fmt, "%s", sp->fmt ? sp->fmt : "");
    r.spec.label = r.label;
    r.spec.fmt = r.fmt;
    return g_nRows++;
}
void KPATCH_CALL fSetHandler(uint32_t, K2MuiChangeFn fn, void* u) { g_fn = fn; g_fnUser = u; }
int32_t KPATCH_CALL fGet(uint32_t, int32_t) { return 0; }
void KPATCH_CALL fSetV(uint32_t, int32_t, int32_t) {}
void KPATCH_CALL fSetT(uint32_t, int32_t, const char*) {}
void KPATCH_CALL fSetS(uint32_t, int32_t, int32_t) {}
void KPATCH_CALL fSetR(uint32_t, int32_t, int32_t, int32_t) {}

void connectFake() {
    memset(&g_table, 0, sizeof g_table);
    g_table.struct_size = sizeof g_table;
    g_table.NewPage = fNewPage;
    g_table.AddRow = fAddRow;
    g_table.SetChangeHandler = fSetHandler;
    g_table.GetValue = fGet;
    g_table.SetValue = fSetV;
    g_table.SetRowText = fSetT;
    g_table.SetRowState = fSetS;
    g_table.SetRowRange = fSetR;
    g_client = k2mc::MenuClient();
    g_client.p = &g_table;
    g_client.connected = true;
    g_nRows = 0;
    g_fn = nullptr;
    g_fnUser = nullptr;
}

// Page as HolsterEditor.cpp KPatchInit leaves it: schema loaded, fake io.
void setupPage(mcdecl::Page& pg) {
    char err[96];
    pg = mcdecl::Page();
    KT_CHECK(pg.load(hsp::kPageSchema, strlen(hsp::kPageSchema), err, sizeof err));
    pg.io.read = fakeRead;
    pg.io.write = fakeWrite;
    g_nWrites = 0;
    g_iniHas = false;
}

static void test_schema_parses_one_slider_row() {
    static mcdecl::PageDecl pd;
    char err[96];
    KT_CHECK(mcdecl::parseSchema(hsp::kPageSchema, strlen(hsp::kPageSchema), pd, err, sizeof err));
    KT_CHECK_EQ(pd.nRows, 1);
    KT_CHECK(strcmp(pd.name, "Slow holster") == 0);
    KT_CHECK(strcmp(pd.title, "Slow holster") == 0);
    KT_CHECK(strlen(pd.desc) > 0 && strlen(pd.desc) < 191);      // the provider keeps 192 bytes of description
    const mcdecl::RowDecl& r = pd.rows[0];
    KT_CHECK(strcmp(r.id, hsp::kRowId) == 0);
    KT_CHECK_EQ(r.kind, (int)mcdecl::K_STEPPER);
    KT_CHECK(r.slider);                                          // style = "slider"
    KT_CHECK(strcmp(r.label, "Slow holster") == 0);
    KT_CHECK(strlen(r.desc) > 0 && strlen(r.desc) < 191);
    KT_CHECK(strcmp(r.fmt, "Slow holster: %dx") == 0);
    KT_CHECK_EQ(r.restart ? 1 : 0, 0);                           // apply = "live": holster polls holster.ini, no " (restart)" caption
    KT_CHECK_EQ(r.wrap ? 1 : 0, 0);
    KT_CHECK_EQ(r.scale, 0);                                     // raw integer written, no scaling
}

// Every string value in the schema fits the TOML reader's cap (95 characters) and a bad schema is refused (so a long edit fails here, not in game).
static void test_schema_string_caps() {
    const char* p = hsp::kPageSchema;
    int checked = 0;
    while ((p = strchr(p, '"')) != nullptr) {
        const char* q = strchr(p + 1, '"');
        KT_CHECK(q != nullptr);
        if (!q) break;
        KT_CHECK(static_cast<size_t>(q - p - 1) <= 95);
        ++checked;
        p = q + 1;
    }
    KT_CHECK(checked >= 10);
    static mcdecl::PageDecl pd;
    char err[96];
    const char* noRows = "[page]\nname = \"x\"\n";
    KT_CHECK(!mcdecl::parseSchema(noRows, strlen(noRows), pd, err, sizeof err));
}

static void test_binding_is_flat_slow_holster_x() {
    static mcdecl::PageDecl pd;
    char err[96];
    KT_CHECK(mcdecl::parseSchema(hsp::kPageSchema, strlen(hsp::kPageSchema), pd, err, sizeof err));
    const mcdecl::RowDecl& r = pd.rows[0];
    KT_CHECK(strcmp(r.ini, "holster.ini") == 0);
    KT_CHECK(strcmp(r.key, "slow_holster_x") == 0);
    KT_CHECK(r.section[0] == 0);                                 // flat key: holster.ini has no [section] headers
    KT_CHECK(strcmp(r.key, hsp::kIniKey) == 0 && strcmp(r.ini, hsp::kIniFile) == 0);
    KT_CHECK_EQ(r.minV, hsp::kMin);
    KT_CHECK_EQ(r.maxV, hsp::kMax);
    KT_CHECK_EQ(r.def, hsp::kDefault);
    KT_CHECK_EQ(hsp::kMin, 1);                                   // 1 = normal speed, the floor
    KT_CHECK_EQ(hsp::kMax, 8);
    KT_CHECK_EQ(r.step, 1);
}

// Distinct id: the Holstered Weapons entry ("holster-editor") keeps its own id, so the provider never overwrites one entry with the other.
static void test_entry_is_a_distinct_registration() {
    static mcdecl::Page pg;
    setupPage(pg);
    const K2ModConfigEntry e = pg.entry(hsp::kEntryId);
    KT_CHECK(strcmp(e.id, "holster-slow") == 0);
    KT_CHECK(strcmp(e.id, "holster-editor") != 0);
    KT_CHECK(strlen(e.id) < 32);
    KT_CHECK(strcmp(e.name, "Slow holster") == 0);
    KT_CHECK_EQ(e.struct_size, sizeof e);
    KT_CHECK(e.user == &pg && e.build != nullptr && e.tick != nullptr && e.destroy != nullptr);
}

// startValue (the build-time read): the flat key as a clamped int; absent key or junk = the schema default (normal speed).
static void test_start_value_reads_clamped_flat_key() {
    static mcdecl::Page pg;
    setupPage(pg);
    const struct { const char* text; int want; } cases[] = {{"3", 3}, {"1", 1}, {"8", 8}, {"99", 8}, {"0", 1}, {"-4", 1}, {"junk", 1}};
    for (const auto& c : cases) {
        g_iniHas = true;
        copyTo(g_iniVal, sizeof g_iniVal, c.text);
        KT_CHECK_EQ(pg.startValue(0), c.want);
    }
    g_iniHas = false;
    KT_CHECK_EQ(pg.startValue(0), hsp::kDefault);                // no holster.ini key yet: normal speed
}

// Robustness: build against a fake menu writes NOTHING (a default write would add an unasked-for line to holster.ini); only a changed value writes.
static void test_build_writes_nothing_and_change_writes_once() {
    static mcdecl::Page pg;
    setupPage(pg);
    connectFake();
    pg.menu = &g_client;
    pg.build(reinterpret_cast<void*>(0x1234));
    KT_CHECK(pg.built && pg.rowId[0] == 0 && g_nRows == 1);
    if (!pg.built || g_nRows != 1 || !g_fn) { KT_CHECK(false); return; }
    const K2MuiRowSpec& sp = g_rows[0].spec;
    KT_CHECK_EQ(sp.kind, MUI_ROW_SLIDER);
    KT_CHECK(sp.min == 1 && sp.max == 8 && sp.step == 1 && sp.value == 1);
    KT_CHECK((sp.flags & MUI_RF_RESTART) == 0);                  // live
    KT_CHECK(strcmp(g_rows[0].fmt, "Slow holster: %dx") == 0);
    KT_CHECK_EQ(g_nWrites, 0);                                   // build (and the start read) never write
    KT_CHECK_EQ(pg.get("slow"), 1);

    KT_CHECK_EQ(g_fn(9, 0, 1, 0, g_fnUser), 1);                  // re-set of the start value: accepted, not written
    KT_CHECK_EQ(g_nWrites, 0);
    KT_CHECK_EQ(g_fn(9, 0, 4, 1, g_fnUser), 4);
    KT_CHECK_EQ(g_nWrites, 1);
    KT_CHECK(strcmp(g_writes[0].path, "holster.ini") == 0 && strcmp(g_writes[0].key, "slow_holster_x") == 0);
    KT_CHECK(strcmp(g_writes[0].val, "4") == 0 && g_writes[0].flat);   // plain integer, flat key
    KT_CHECK_EQ(g_fn(9, 0, 4, 0, g_fnUser), 4);                  // same value again: no second write
    KT_CHECK_EQ(g_nWrites, 1);
    KT_CHECK_EQ(g_fn(9, 0, 99, 1, g_fnUser), 8);                 // above the range: clamped
    KT_CHECK(strcmp(g_writes[1].val, "8") == 0);
    KT_CHECK_EQ(g_fn(9, 0, -3, -1, g_fnUser), 1);                // below the range: clamped to 1 (normal)
    KT_CHECK(strcmp(g_writes[2].val, "1") == 0);
    KT_CHECK_EQ(g_nWrites, 3);
}

// The stock mcdecl::fileRead keeps 16 KB; hsp::readIni keeps 64 KB. A large live holster.ini with the key near its end must still read back.
static void test_large_ini_read_past_16kb() {
    std::string text;
    for (int i = 0; text.size() < 20000; ++i) text += "place_off_pad" + std::to_string(i) + "=0.0 0.0 0.0\n";   // flat filler, ~20 KB
    text += "slow_holster_x=5\n";
    KT_CHECK(text.size() > 16384 + 1000);
    const char* tmp = getenv("TMPDIR");
    std::string path = std::string(tmp && *tmp ? tmp : "/tmp") + "/slow_holster_page_test_" + std::to_string(static_cast<long>(rand())) + ".ini";
    FILE* f = fopen(path.c_str(), "wb");
    KT_CHECK(f != nullptr);
    if (!f) return;
    fwrite(text.data(), 1, text.size(), f);
    fclose(f);
    char out[32] = "";
    KT_CHECK(hsp::readIni(path.c_str(), nullptr, hsp::kIniKey, out, sizeof out));
    KT_CHECK(strcmp(out, "5") == 0);
    char missing[32] = "";
    KT_CHECK(!hsp::readIni(path.c_str(), nullptr, "no_such_key", missing, sizeof missing));
    KT_CHECK(!mcdecl::fileRead(path.c_str(), nullptr, hsp::kIniKey, out, sizeof out));   // documents why readIni exists: the stock reader misses it
    remove(path.c_str());
    KT_CHECK(!hsp::readIni(path.c_str(), nullptr, hsp::kIniKey, out, sizeof out));       // no file: absent
    KT_CHECK(!hsp::readIni(nullptr, nullptr, hsp::kIniKey, out, sizeof out));
}

}  // namespace

int main() {
    test_schema_parses_one_slider_row();
    test_schema_string_caps();
    test_binding_is_flat_slow_holster_x();
    test_entry_is_a_distinct_registration();
    test_start_value_reads_clamped_flat_key();
    test_build_writes_nothing_and_change_writes_once();
    test_large_ini_read_past_16kb();
    return kt::Report("slow_holster_page");
}
