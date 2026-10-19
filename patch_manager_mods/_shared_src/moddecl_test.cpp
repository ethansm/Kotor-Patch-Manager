// Host test of ModConfigDecl.h: schema parsing and validation, RowSpec mapping, start values from ini, the change path (hook final value,
// ini write-through, scaled captions), the dynamic-row pass-throughs, iniGet, formatScaled and the entry struct. A fake k2.menuui table
// stands in for mod-config.
#include "ModConfigDecl.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

using namespace mcdecl;

// ---------------------------------------------------------------- fake k2.menuui rows provider
struct FakeRow {
    K2MuiRowSpec spec;
    char label[64], fmt[48], zero[24];
    int32_t value, minV, maxV, flags;
    char text[96];
};
static FakeRow g_rows[40];
static int g_nRows;
static K2MuiChangeFn g_fn;
static void* g_fnUser;
static uint32_t g_page = 7;
static int g_newPageCalls, g_failAddAt, g_addCalls;
static char g_title[64];

static void fakeReset() {
    memset(g_rows, 0, sizeof g_rows);
    g_nRows = 0;
    g_fn = nullptr;
    g_fnUser = nullptr;
    g_newPageCalls = 0;
    g_failAddAt = -1;
    g_addCalls = 0;
    g_title[0] = 0;
}
static int32_t clampTo(const FakeRow& r, int32_t v) {
    if (r.spec.kind == MUI_ROW_TOGGLE) return v ? 1 : 0;
    return v < r.minV ? r.minV : (v > r.maxV ? r.maxV : v);
}
static uint32_t KPATCH_CALL fNewPage(void*, const char* title) {
    ++g_newPageCalls;
    snprintf(g_title, sizeof g_title, "%s", title ? title : "");
    return g_page;
}
static int32_t KPATCH_CALL fAddRow(uint32_t, const K2MuiRowSpec* sp) {
    if (g_addCalls++ == g_failAddAt || g_nRows >= 40) return -1;
    FakeRow& r = g_rows[g_nRows];
    r.spec = *sp;
    r.value = sp->value;
    r.minV = sp->min;
    r.maxV = sp->max;
    r.flags = sp->flags;
    r.value = clampTo(r, sp->value);
    return g_nRows++;
}
static void KPATCH_CALL fSetHandler(uint32_t, K2MuiChangeFn fn, void* u) { g_fn = fn; g_fnUser = u; }
static int32_t KPATCH_CALL fGet(uint32_t, int32_t row) { return row >= 0 && row < g_nRows ? g_rows[row].value : 0; }
static void KPATCH_CALL fSet(uint32_t, int32_t row, int32_t v) { if (row >= 0 && row < g_nRows) g_rows[row].value = clampTo(g_rows[row], v); }
static void KPATCH_CALL fSetText(uint32_t, int32_t row, const char* t) { if (row >= 0 && row < g_nRows) snprintf(g_rows[row].text, sizeof g_rows[row].text, "%s", t ? t : ""); }
static void KPATCH_CALL fSetState(uint32_t, int32_t row, int32_t f) { if (row >= 0 && row < g_nRows) g_rows[row].flags = f; }
static void KPATCH_CALL fSetRange(uint32_t, int32_t row, int32_t lo, int32_t hi) {
    if (row < 0 || row >= g_nRows) return;
    g_rows[row].minV = lo;
    g_rows[row].maxV = hi;
    g_rows[row].value = clampTo(g_rows[row], g_rows[row].value);
}
// Unused plain-v1 slots: never called by Page.
static int32_t KPATCH_CALL fNoopI(uint32_t, const char*, const char*) { return 0; }
static int32_t KPATCH_CALL fNoopL(uint32_t, const char*) { return 0; }
static int32_t KPATCH_CALL fNoopC(uint32_t, void*, int32_t) { return 0; }
static int32_t KPATCH_CALL fClicked(uint32_t, int32_t) { return 0; }
static void KPATCH_CALL fNoopT(uint32_t, int32_t, const char*) {}
static void KPATCH_CALL fNoopR(uint32_t, int32_t, const K2MuiRect*) {}
static void KPATCH_CALL fNoopD(uint32_t, const char*) {}
static int32_t KPATCH_CALL fPane(void*, int32_t, K2MuiRect*) { return 0; }
static void* KPATCH_CALL fCtl(uint32_t, int32_t) { return nullptr; }
static int32_t KPATCH_CALL fSplit(uint32_t, int32_t) { return 0; }

static K2MenuUiApi g_table = {sizeof(K2MenuUiApi), fNewPage, fNoopI, fNoopL, fNoopC, fClicked, fNoopT, fNoopR, fNoopD, fPane, fCtl, fSplit,
                              fAddRow, fSetHandler, fGet, fSet, fSetText, fSetState, fSetRange};

static k2mc::MenuClient g_client;
static void connectFake(const K2MenuUiApi* t) {
    g_client = k2mc::MenuClient();
    g_client.p = t;
    g_client.connected = true;
}

// ---------------------------------------------------------------- fake ini
static char g_iniText[1024];
struct Written { char path[64], section[48], key[48], val[16]; };
static Written g_wr[16];
static int g_nWr;
static bool fakeRead(const char* path, const char* section, const char* key, char* out, int cap) {
    if (strcmp(path, "enemy_strip.ini") != 0) return false;
    return iniGet(g_iniText, strlen(g_iniText), section, key, out, cap);
}
static bool fakeWrite(const char* path, const char* section, const char* key, const char* val) {
    if (g_nWr >= 16) return false;
    Written& w = g_wr[g_nWr++];
    snprintf(w.path, sizeof w.path, "%s", path);
    snprintf(w.section, sizeof w.section, "%s", section ? section : "");
    snprintf(w.key, sizeof w.key, "%s", key);
    snprintf(w.val, sizeof w.val, "%s", val);
    return true;
}

// ---------------------------------------------------------------- schemas
// The enemy-effects-strip page (8 rows, doc 46 sec 8) expressed declaratively: proves 1:1 coverage for MCF7.
static const char* kStripSchema = R"TOML(
[page]
name  = "Enemy Effects Strip"
title = "Enemy Effects Strip"
desc  = "Icon row above the target name."

[[row]]
id = "enabled"
kind = "toggle"
label = "Show icons"
desc = "Show or hide the icon row at once."
default = 1
ini = "enemy_strip.ini"
section = "Strip"
key = "Enabled"

[[row]]
id = "max_icons"
kind = "stepper"
label = "Max icons"
desc = "Most icons shown at once (1-8)."
default = 5
min = 1
max = 8
step = 1
apply = "restart"
fmt = "Max icons: %d"
ini = "enemy_strip.ini"
section = "Strip"
key = "MaxIcons"

[[row]]
id = "icon_size"
kind = "stepper"
label = "Icon size"
desc = "Icon edge in pixels. Auto uses the name label height."
default = 0
min = 0
max = 64
step = 2
zero_text = "Auto"
ini = "enemy_strip.ini"
section = "Strip"
key = "IconSize"

[[row]]
id = "name_gap"
kind = "stepper"
label = "Name gap"
desc = "Pixels between the icon row and the name label."
default = 4
min = 0
max = 24
step = 1
ini = "enemy_strip.ini"
section = "Strip"
key = "NameGap"

[[row]]
id = "icon_gap"
kind = "stepper"
label = "Icon gap"
desc = "Pixels between neighbouring icons."
default = 2
min = 0
max = 16
step = 1
ini = "enemy_strip.ini"
section = "Strip"
key = "IconGap"

[[row]]
id = "pill"
kind = "toggle"
label = "Backing pill"
desc = "Draw a pill behind the icon row."
default = 1
ini = "enemy_strip.ini"
section = "Strip"
key = "Pill"

[[row]]
id = "pill_alpha"
kind = "stepper"
label = "Pill opacity"
desc = "Opacity of the pill."
default = 60
min = 0
max = 100
step = 5
scale = 100
ini = "enemy_strip.ini"
section = "Strip"
key = "PillAlpha"

[[row]]
id = "drain_alpha"
kind = "stepper"
label = "Drain shading"
desc = "Opacity of the remaining-time drain shading. 0 = off."
default = 0
min = 0
max = 100
step = 5
scale = 100
zero_text = "Off"
ini = "enemy_strip.ini"
section = "Strip"
key = "DrainAlpha"
)TOML";

static const char* kHealSchema = R"TOML(
[page]
name = "Companion Self-Heal"
desc = "Per-character heal rules."
[[row]]
id = "who"
kind = "dynamic"
label = "Character"
min = 0
max = 2
[[row]]
id = "auto"
kind = "toggle"
label = "Auto-heal"
[[row]]
id = "pct"
kind = "stepper"
label = "Heal"
min = 5
max = 95
step = 5
default = 35
fmt = "Heal below %d%%"
[[row]]
id = "speed"
kind = "list"
label = "Speed"
choices = ["Off", "Slow", "Fast"]
default = 1
wrap = true
)TOML";

static bool parseStr(const char* t, PageDecl& pd, char* err, int cap) { return parseSchema(t, strlen(t), pd, err, cap); }

static void test_parse_strip_golden() {
    static PageDecl pd;
    char err[96];
    KT_CHECK(parseStr(kStripSchema, pd, err, sizeof err));
    KT_CHECK_EQ(pd.nRows, 8);
    KT_CHECK(strcmp(pd.name, "Enemy Effects Strip") == 0 && strcmp(pd.title, "Enemy Effects Strip") == 0);
    static const int kinds[8] = {K_TOGGLE, K_STEPPER, K_STEPPER, K_STEPPER, K_STEPPER, K_TOGGLE, K_STEPPER, K_STEPPER};
    for (int i = 0; i < 8; ++i) KT_CHECK_EQ(pd.rows[i].kind, kinds[i]);
    KT_CHECK(pd.rows[1].restart && !pd.rows[0].restart);
    KT_CHECK(strcmp(pd.rows[2].zero, "Auto") == 0 && pd.rows[2].step == 2);
    KT_CHECK(pd.rows[6].scale == 100 && pd.rows[7].scale == 100);
    KT_CHECK(strcmp(pd.rows[7].key, "DrainAlpha") == 0 && strcmp(pd.rows[0].section, "Strip") == 0);
    KT_CHECK_EQ(pd.find("icon_gap"), 4);
    KT_CHECK_EQ(pd.find("nope"), -1);
    KT_CHECK(parseStr(kHealSchema, pd, err, sizeof err));
    KT_CHECK_EQ(pd.nRows, 4);
    KT_CHECK(strcmp(pd.title, "Companion Self-Heal") == 0);          // title defaults to name
    KT_CHECK(pd.rows[0].kind == K_DYNAMIC && pd.rows[0].wrap);       // dynamic = wrap stepper
    KT_CHECK(strcmp(pd.rows[1].label, "Auto-heal") == 0);
    KT_CHECK(pd.rows[3].minV == 0 && pd.rows[3].maxV == 2 && pd.rows[3].def == 1);   // list range from choices
}

static void expectBad(const char* text, const char* want) {
    static PageDecl pd;
    char err[96];
    const bool ok = parseStr(text, pd, err, sizeof err);
    if (ok || !strstr(err, want)) {
        printf("expectBad: ok=%d err='%s' want '%s'\n", ok ? 1 : 0, err, want);
        KT_CHECK(false);
    }
    KT_CHECK_EQ(pd.nRows, 0);
}

static void test_parse_errors() {
    const char* pg = "[page]\nname = \"P\"\n";
    char b[1200];
    expectBad("[[row]]\nid=\"a\"\n", "[[row]] before [page]");
    expectBad("[page]\n[page]\n", "duplicate [page]");
    expectBad("x = 1\n", "key outside a table");
    expectBad("[other]\n", "unknown table");
    expectBad("[[page]]\n", "plain table");
    expectBad("[page]\n[row]\n", "rows are [[row]]");
    expectBad(pg, "no [[row]]");
    expectBad("[[row]]\n", "[[row]] before [page]");
    expectBad("", "missing [page]");
    expectBad("[page]\ndesc = \"d\"\n[[row]]\nid = \"a\"\nkind = \"toggle\"\n", "needs a name or title");
    snprintf(b, sizeof b, "%s[[row]]\nkind = \"toggle\"\n", pg);
    expectBad(b, "row 1: missing id");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\n", pg);
    expectBad(b, "row 1 (a): missing kind");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"slider\"\n", pg);
    expectBad(b, "line 5: kind must be");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\n[[row]]\nid = \"a\"\nkind = \"toggle\"\n", pg);
    expectBad(b, "row 2 (a): duplicate id");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\nbogus = 1\n", pg);
    expectBad(b, "unknown row key 'bogus'");
    snprintf(b, sizeof b, "%s[[row]]\nid = 3\n", pg);
    expectBad(b, "expected a string");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"stepper\"\nmin = 5\nmax = 1\n", pg);
    expectBad(b, "min is above max");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"stepper\"\nstep = 0\n", pg);
    expectBad(b, "step must be at least 1");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"list\"\nchoices = [\"x\"]\n", pg);
    expectBad(b, "at least 2 choices");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"stepper\"\nchoices = [\"x\",\"y\"]\n", pg);
    expectBad(b, "list rows only");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\nscale = 100\n", pg);
    expectBad(b, "stepper and dynamic");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"stepper\"\nscale = -1\n", pg);
    expectBad(b, "scale must be positive");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\nini = \"x.ini\"\n", pg);
    expectBad(b, "ini needs a key");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\nkey = \"K\"\n", pg);
    expectBad(b, "need an ini file");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"dynamic\"\nini = \"x.ini\"\nkey = \"K\"\n", pg);
    expectBad(b, "no ini binding");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\napply = \"later\"\n", pg);
    expectBad(b, "apply must be live or restart");
    // string longer than the field (label cap 63) and an overlong id
    char longLabel[100];
    memset(longLabel, 'L', 70); longLabel[70] = 0;
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"toggle\"\nlabel = \"%s\"\n", pg, longLabel);
    expectBad(b, "string too long for this field");
    char longId[40];
    memset(longId, 'i', 32); longId[32] = 0;
    snprintf(b, sizeof b, "%s[[row]]\nid = \"%s\"\nkind = \"toggle\"\n", pg, longId);
    expectBad(b, "string too long for this field");
    // too many choices (13) and a too-long choice
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"list\"\nchoices = [\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"10\",\"11\",\"12\",\"13\"]\n", pg);
    expectBad(b, "too many choices");
    snprintf(b, sizeof b, "%s[[row]]\nid = \"a\"\nkind = \"list\"\nchoices = [\"%s\",\"y\"]\n", pg, longLabel);
    expectBad(b, "choice too long");
}

static void test_row_cap() {
    static char big[40000];
    int o = snprintf(big, sizeof big, "[page]\nname = \"P\"\n");
    for (int i = 0; i < 33; ++i) {
        o += snprintf(big + o, sizeof big - o, "[[row]]\nid = \"r%d\"\nkind = \"toggle\"\n", i);
        if (i == 31) {
            static PageDecl pd;
            char err[96];
            KT_CHECK(parseStr(big, pd, err, sizeof err));     // 32 rows accepted
            KT_CHECK_EQ(pd.nRows, 32);
        }
    }
    expectBad(big, "too many rows");                           // the 33rd is refused
}

static void test_defaults_clamped() {
    static PageDecl pd;
    char err[96];
    const char* t = "[page]\nname=\"P\"\n[[row]]\nid=\"a\"\nkind=\"stepper\"\nmin=5\nmax=95\nstep=5\ndefault=500\n"
                    "[[row]]\nid=\"b\"\nkind=\"toggle\"\ndefault=7\n[[row]]\nid=\"c\"\nkind=\"list\"\nchoices=[\"x\",\"y\"]\ndefault=-3\n";
    KT_CHECK(parseStr(t, pd, err, sizeof err));
    KT_CHECK_EQ(pd.rows[0].def, 95);
    KT_CHECK_EQ(pd.rows[1].def, 1);
    KT_CHECK_EQ(pd.rows[2].def, 0);
}

// ---------------------------------------------------------------- iniGet / formatScaled
static void test_iniget() {
    char o[32];
    const char* t = "top=9\r\n; c=1\r\n[Strip]\r\nMaxIcons = 4  ; note\r\nmaxicons=6\r\nName=a;b\r\n;Skipped=1\r\n[Other]\r\nMaxIcons=99\r\n";
    KT_CHECK(iniGet(t, strlen(t), "Strip", "MaxIcons", o, sizeof o) && strcmp(o, "6") == 0);   // last duplicate wins, case-insensitive
    KT_CHECK(iniGet(t, strlen(t), "strip", "NAME", o, sizeof o) && strcmp(o, "a;b") == 0);     // glued ';' is part of the value
    KT_CHECK(!iniGet(t, strlen(t), "Strip", "Skipped", o, sizeof o));                           // commented default is absent
    KT_CHECK(iniGet(t, strlen(t), "Other", "MaxIcons", o, sizeof o) && strcmp(o, "99") == 0);
    KT_CHECK(iniGet(t, strlen(t), nullptr, "top", o, sizeof o) && strcmp(o, "9") == 0);         // flat file
    KT_CHECK(iniGet(t, strlen(t), "", "top", o, sizeof o));
    KT_CHECK(!iniGet(t, strlen(t), nullptr, "MaxIcons", o, sizeof o));                          // flat region ends at the first header
    KT_CHECK(!iniGet(t, strlen(t), "Nope", "x", o, sizeof o));
    KT_CHECK(!iniGet(nullptr, 0, "S", "k", o, sizeof o) && o[0] == 0);
    KT_CHECK(!iniGet(t, strlen(t), "Strip", "", o, sizeof o));
    char tiny[3];
    KT_CHECK(iniGet("[S]\nk=abcdef\n", 13, "S", "k", tiny, sizeof tiny) && strcmp(tiny, "ab") == 0);   // truncated, terminated
    KT_CHECK(iniGet("[S]\nk=\n", 7, "S", "k", o, sizeof o) && o[0] == 0);                                // empty value is present
}

static void test_format_scaled() {
    RowDecl r;
    r.reset();
    strcpy(r.label, "Opacity");
    r.scale = 100;
    char b[64];
    formatScaled(b, sizeof b, r, 75); KT_CHECK(strcmp(b, "Opacity: 0.75") == 0);
    formatScaled(b, sizeof b, r, 100); KT_CHECK(strcmp(b, "Opacity: 1.00") == 0);
    formatScaled(b, sizeof b, r, 5); KT_CHECK(strcmp(b, "Opacity: 0.05") == 0);
    formatScaled(b, sizeof b, r, 0); KT_CHECK(strcmp(b, "Opacity: 0.00") == 0);
    formatScaled(b, sizeof b, r, -25); KT_CHECK(strcmp(b, "Opacity: -0.25") == 0);
    formatScaled(b, sizeof b, r, 250); KT_CHECK(strcmp(b, "Opacity: 2.50") == 0);
    r.scale = 10; formatScaled(b, sizeof b, r, 15); KT_CHECK(strcmp(b, "Opacity: 1.50") == 0);
    r.scale = 3; formatScaled(b, sizeof b, r, 1); KT_CHECK(strcmp(b, "Opacity: 0.33") == 0);
    strcpy(r.zero, "Off");
    r.scale = 100; formatScaled(b, sizeof b, r, 0); KT_CHECK(strcmp(b, "Opacity: Off") == 0);
    r.restart = true; formatScaled(b, sizeof b, r, 50); KT_CHECK(strcmp(b, "Opacity: 0.50 (restart)") == 0);
    formatScaled(b, 8, r, 50); KT_CHECK(strcmp(b, "Opacity") == 0 || strlen(b) == 7);   // bounded
}

// ---------------------------------------------------------------- Page build + change path
static int g_hookCalls;
static char g_hookId[32];
static int32_t g_hookReq, g_hookDir, g_hookRet;
static bool g_hookSet;
static int g_enters;
static int32_t KPATCH_CALL dummyUnused() { return 0; }
static int32_t hookChange(void*, int row, const char* id, int32_t requested, int dir) {
    ++g_hookCalls;
    snprintf(g_hookId, sizeof g_hookId, "%s", id);
    g_hookReq = requested;
    g_hookDir = dir;
    (void)row;
    return g_hookSet ? g_hookRet : requested;
}
static void hookEnter(void*) { ++g_enters; }

static Page g_pg;   // ~40 KB

static void setupPage(const char* schema) {
    fakeReset();
    connectFake(&g_table);
    char err[96];
    g_pg = Page();
    KT_CHECK(g_pg.load(schema, strlen(schema), err, sizeof err));
    g_pg.menu = &g_client;
    g_pg.hooks.onChange = hookChange;
    g_pg.hooks.onEnter = hookEnter;
    g_pg.io.read = fakeRead;
    g_pg.io.write = fakeWrite;
    g_hookCalls = 0; g_hookSet = false; g_nWr = 0; g_enters = 0;
    (void)dummyUnused;
}

static void test_build_mapping() {
    snprintf(g_iniText, sizeof g_iniText, "[Strip]\nMaxIcons=7\nPillAlpha=45\nEnabled=0\nIconSize=999\n");
    setupPage(kStripSchema);
    K2ModConfigEntry e = g_pg.entry("enemy-effects-strip");
    e.build(reinterpret_cast<void*>(0x1234), e.user);
    KT_CHECK(g_pg.built);
    KT_CHECK_EQ(g_newPageCalls, 1);
    KT_CHECK(strcmp(g_title, "Enemy Effects Strip") == 0);
    KT_CHECK_EQ(g_nRows, 8);
    KT_CHECK(g_fn != nullptr && g_fnUser == &g_pg);
    KT_CHECK_EQ(g_rows[0].spec.kind, MUI_ROW_TOGGLE);
    KT_CHECK_EQ(g_rows[1].spec.kind, MUI_ROW_STEPPER);
    KT_CHECK(g_rows[1].spec.flags & MUI_RF_RESTART);
    KT_CHECK(g_rows[1].spec.fmt && strcmp(g_rows[1].spec.fmt, "Max icons: %d") == 0);
    KT_CHECK(g_rows[2].spec.zeroText && strcmp(g_rows[2].spec.zeroText, "Auto") == 0);
    KT_CHECK(g_rows[6].spec.fmt == nullptr && g_rows[6].spec.zeroText == nullptr);   // scaled rows get their caption through SetRowText
    KT_CHECK(strcmp(g_rows[0].spec.label, "Show icons") == 0 && strcmp(g_rows[0].spec.desc, "Show or hide the icon row at once.") == 0);
    KT_CHECK(g_rows[1].spec.min == 1 && g_rows[1].spec.max == 8 && g_rows[1].spec.step == 1);
    // start values: ini where present and valid, clamped; the default otherwise
    KT_CHECK_EQ(g_rows[0].value, 0);        // Enabled=0
    KT_CHECK_EQ(g_rows[1].value, 7);        // MaxIcons=7
    KT_CHECK_EQ(g_rows[2].value, 64);       // IconSize=999 clamped to max
    KT_CHECK_EQ(g_rows[3].value, 4);        // not in the ini: default
    KT_CHECK_EQ(g_rows[6].value, 45);
    KT_CHECK(strcmp(g_rows[6].text, "Pill opacity: 0.45") == 0);   // scaled caption pushed at build
    KT_CHECK(strcmp(g_rows[7].text, "Drain shading: Off") == 0);
    KT_CHECK_EQ(g_pg.get("max_icons"), 7);
    // enter / leave
    e.enter(g_page, e.user);
    KT_CHECK_EQ(g_enters, 1);
    // entry struct
    KT_CHECK(e.struct_size == sizeof(K2ModConfigEntry) && strcmp(e.id, "enemy-effects-strip") == 0 && strcmp(e.name, "Enemy Effects Strip") == 0);
    KT_CHECK(e.desc == g_pg.decl.desc && e.user == &g_pg && e.build && e.enter && e.leave && e.tick && e.destroy);
    e.destroy(nullptr, e.user);
    KT_CHECK(!g_pg.built);
    e.enter(g_page, e.user);
    KT_CHECK_EQ(g_enters, 1);               // an unbuilt page ignores callbacks
}

static void test_change_path() {
    snprintf(g_iniText, sizeof g_iniText, "[Strip]\nMaxIcons=4\nPillAlpha=60\n");
    setupPage(kStripSchema);
    K2ModConfigEntry e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    // accept: ini written with the final value
    KT_CHECK_EQ(g_fn(g_page, 1, 5, 1, g_fnUser), 5);
    KT_CHECK_EQ(g_nWr, 1);
    KT_CHECK(strcmp(g_wr[0].path, "enemy_strip.ini") == 0 && strcmp(g_wr[0].section, "Strip") == 0 && strcmp(g_wr[0].key, "MaxIcons") == 0 && strcmp(g_wr[0].val, "5") == 0);
    KT_CHECK(strcmp(g_hookId, "max_icons") == 0 && g_hookReq == 5 && g_hookDir == 1);
    KT_CHECK_EQ(g_pg.get("max_icons"), 5);
    // snap: the hook returns a different final value, that one is written
    g_hookSet = true; g_hookRet = 8;
    KT_CHECK_EQ(g_fn(g_page, 1, 6, 1, g_fnUser), 8);
    KT_CHECK(strcmp(g_wr[1].val, "8") == 0);
    // reject: the hook returns the old value -> no write
    g_hookRet = 8;
    const int before = g_nWr;
    KT_CHECK_EQ(g_fn(g_page, 1, 9, 1, g_fnUser), 8);
    KT_CHECK_EQ(g_nWr, before);
    // out-of-range final is clamped before it is written
    g_hookRet = 500;
    KT_CHECK_EQ(g_fn(g_page, 1, 2, -1, g_fnUser), 8);       // clamped to 8 == current: no write
    KT_CHECK_EQ(g_nWr, before);
    g_hookRet = -4;
    KT_CHECK_EQ(g_fn(g_page, 1, 2, -1, g_fnUser), 1);
    KT_CHECK(strcmp(g_wr[before].val, "1") == 0);
    // toggle: dir 0, writes 0/1
    g_hookSet = false;
    KT_CHECK_EQ(g_fn(g_page, 0, 0, 0, g_fnUser), 0);
    KT_CHECK(strcmp(g_wr[g_nWr - 1].key, "Enabled") == 0 && strcmp(g_wr[g_nWr - 1].val, "0") == 0 && g_hookDir == 0);
    // scaled row: caption refreshed through SetRowText with the final value
    KT_CHECK_EQ(g_fn(g_page, 6, 65, 1, g_fnUser), 65);
    KT_CHECK(strcmp(g_rows[6].text, "Pill opacity: 0.65") == 0);
    g_hookSet = true; g_hookRet = 70;
    KT_CHECK_EQ(g_fn(g_page, 6, 65, 1, g_fnUser), 70);
    KT_CHECK(strcmp(g_rows[6].text, "Pill opacity: 0.70") == 0 && strcmp(g_wr[g_nWr - 1].val, "70") == 0);
    // unknown provider row: accepted as requested, nothing written
    const int w = g_nWr;
    KT_CHECK_EQ(g_fn(g_page, 99, 3, 1, g_fnUser), 3);
    KT_CHECK_EQ(g_nWr, w);
    // a failed ini write does not change the returned value
    g_nWr = 16;
    g_hookSet = false;
    KT_CHECK_EQ(g_fn(g_page, 3, 9, 1, g_fnUser), 9);
}

static void test_no_ini_and_dynamic() {
    setupPage(kHealSchema);
    K2ModConfigEntry e = g_pg.entry("companion-selfheal");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(g_pg.built && g_nRows == 4);
    KT_CHECK(g_rows[0].spec.flags & MUI_RF_WRAP);                        // dynamic wraps
    KT_CHECK(g_rows[3].spec.choices != nullptr && g_rows[3].spec.nChoices == 3 && strcmp(g_rows[3].spec.choices[2], "Fast") == 0);
    KT_CHECK_EQ(g_rows[3].value, 1);
    // rows without ini leave persistence to the hook: nothing written
    KT_CHECK_EQ(g_fn(g_page, 2, 40, 1, g_fnUser), 40);
    KT_CHECK_EQ(g_nWr, 0);
    KT_CHECK_EQ(g_hookCalls, 1);
    // dynamic pass-throughs
    g_pg.setRange("who", 0, 5);
    KT_CHECK(g_rows[0].minV == 0 && g_rows[0].maxV == 5);
    g_pg.setValue("who", 4);
    KT_CHECK_EQ(g_rows[0].value, 4);
    KT_CHECK_EQ(g_pg.get("who"), 4);
    g_pg.setRange("who", 0, 2);                                          // value re-clamped by the provider, mirror follows
    KT_CHECK_EQ(g_pg.get("who"), 2);
    g_pg.setText("who", "Atton");
    KT_CHECK(strcmp(g_rows[0].text, "Atton") == 0);
    g_pg.setState("pct", MUI_RF_DISABLED);
    KT_CHECK_EQ(g_rows[2].flags, MUI_RF_DISABLED);
    g_pg.setValue("nope", 3);                                            // unknown id: no crash
    g_pg.setText("nope", "x");
    KT_CHECK_EQ(g_pg.get("nope"), 0);
    // a dynamic row's range is not clamped by the stale decl range
    g_pg.setRange("who", 0, 9);
    g_pg.setValue("who", 9);
    KT_CHECK_EQ(g_fn(g_page, 0, 8, 1, g_fnUser), 8);
}

static void test_provider_variants() {
    // A provider without rows: the page stays unbuilt, nothing is called.
    setupPage(kHealSchema);
    K2MenuUiApi shortTable = g_table;
    shortTable.struct_size = (uint32_t)offsetof(K2MenuUiApi, AddRow);
    connectFake(&shortTable);
    K2ModConfigEntry e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(!g_pg.built);
    KT_CHECK_EQ(g_newPageCalls, 0);
    g_pg.setValue("pct", 5);                                             // all helpers are safe on an unbuilt page
    g_pg.setText("pct", "x");
    g_pg.setRange("pct", 0, 1);
    g_pg.setState("pct", 0);
    // null panel
    setupPage(kHealSchema);
    e = g_pg.entry("x");
    e.build(nullptr, e.user);
    KT_CHECK(!g_pg.built);
    // unloaded page
    fakeReset();
    connectFake(&g_table);
    g_pg = Page();
    g_pg.menu = &g_client;
    e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(!g_pg.built && g_newPageCalls == 0);
    // NewPage refused
    setupPage(kHealSchema);
    K2MenuUiApi noPage = g_table;
    noPage.NewPage = [](void*, const char*) -> uint32_t { return 0; };
    connectFake(&noPage);
    e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(!g_pg.built);
    // one AddRow fails: the page is still built, the failed row is skipped everywhere and ids still map
    setupPage(kHealSchema);
    g_failAddAt = 1;
    e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(g_pg.built);
    KT_CHECK_EQ(g_nRows, 3);                                             // rows 0, 2, 3 -> provider ids 0, 1, 2 (id 1 failed)
    g_failAddAt = -1;
    g_pg.setValue("auto", 1);                                            // failed row: no provider call, no crash
    KT_CHECK_EQ(g_fn(g_page, 1, 40, 1, g_fnUser), 40);                   // provider row 1 is decl row 2 ("pct")
    KT_CHECK(strcmp(g_hookId, "pct") == 0);
    // every AddRow fails: not built
    setupPage(kHealSchema);
    K2MenuUiApi noRows = g_table;
    noRows.AddRow = [](uint32_t, const K2MuiRowSpec*) -> int32_t { return -1; };
    connectFake(&noRows);
    e = g_pg.entry("x");
    e.build(reinterpret_cast<void*>(1), e.user);
    KT_CHECK(!g_pg.built);
}

static void test_footprint() {
    KT_CHECK(sizeof(Page) < 64 * 1024);
    KT_CHECK(sizeof(PageDecl) < 48 * 1024);
}

int main() {
    test_parse_strip_golden();
    test_parse_errors();
    test_row_cap();
    test_defaults_clamped();
    test_iniget();
    test_format_scaled();
    test_build_mapping();
    test_change_path();
    test_no_ini_and_dynamic();
    test_provider_variants();
    test_footprint();
    return kt::Report("moddecl_test");
}
