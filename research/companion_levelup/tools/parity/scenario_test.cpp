// T2 scenario runner (Phase 6b M3): drives the REAL LevelUpOverride.cpp on fake engine structs (MAP_32BIT arena) from the scenario data that
// scenarios.py derives from tests/luo_scenarios/saves.toml. One process per scenario (the mod keeps process-global state).
//   scenario_test <levelup_paths.ini> <scenarios.dat> <id>   run one scenario: action script, then [expect] events / absent / decisions
//   scenario_test <levelup_paths.ini> --unit                 onLevelUp T2 (decideAct truth table, DECIDE/DRY/SKIP/APPLIED on fake call chains)
//                                                            and LUO_G removal safety (no LUO_G -> unconfigured, no writes until Apply)
// Data file (line based, produced by scenarios.py): SCENARIO id | GLOBALS hex8 count | CREATURE tag classes xp inparty leader |
//   INI key=value | PTGUID hex8|none | COMPANION tag spec | ACT words... | EVENT regex | ABSENT regex | DECISION key value | END
// Regexes are ECMAScript, matched per log line after the `[tick] ` prefix and leading spaces (saves.toml contract). Exit 1 on any failure.
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <sys/mman.h>
#include <unistd.h>
#include <deque>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

static uint8_t* g_arena = nullptr;
static size_t g_used = 0;
constexpr size_t ArenaSize = 16u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { fprintf(stderr, "arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used);
    memset(g_arena + g_used, 0, n);
    g_used += n;
    return a;
}
static int g_fail = 0, g_checks = 0;
static std::string g_ctx = "unit";
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fail; fprintf(stderr, "FAIL [%s] line %d: ", g_ctx.c_str(), __LINE__); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }
static void* P(uintptr_t a) { return reinterpret_cast<void*>(a); }

// ---- the XP table (rules+0x38): tbl[i] = XP to reach level i+1, from testdata/exptable.tsv (real exptable.2da values) ----
static uint32_t g_xpTbl[51];

// ---- fake creatures ----
struct Cr { std::string tag, memTag; uintptr_t stats = 0, creature = 0; bool inParty = true, leader = false; };
static std::deque<Cr> g_cr;   // deque: addCreature hands out references
static Cr* crByStats(void* s) { for (auto& c : g_cr) if (P(c.stats) == s) return &c; return nullptr; }
static Cr* crByTag(const std::string& t) { for (auto& c : g_cr) if (c.tag == t) return &c; return nullptr; }
static Cr& addCreature(const std::string& tag, const std::vector<std::pair<int, int>>& classes, uint32_t xp, bool inParty, bool leader) {
    g_cr.emplace_back();
    Cr& c = g_cr.back();
    c.tag = tag; c.memTag = tag == "pc" ? "" : tag; c.inParty = inParty; c.leader = leader;   // the engine's PC has an empty tag
    c.stats = arenaAlloc(0x400); c.creature = arenaAlloc(0x1200);
    at<uint8_t>(c.stats, St_ClassCount) = (uint8_t)classes.size();
    for (size_t i = 0; i < classes.size(); ++i) { at<uint8_t>(c.stats, St_ClassId + i * St_ClassStride) = (uint8_t)classes[i].first; at<uint8_t>(c.stats, St_ClassLevel + i * St_ClassStride) = (uint8_t)classes[i].second; }
    at<uint32_t>(c.stats, St_Creature) = (uint32_t)c.creature;
    at<uint32_t>(c.stats, St_XP) = xp;
    uintptr_t str = arenaAlloc(64);
    memcpy(P(str), c.memTag.data(), c.memTag.size());
    at<uint32_t>(c.creature, Cr_Tag) = (uint32_t)str; at<uint32_t>(c.creature, Cr_Tag + 4) = (uint32_t)c.memTag.size();
    at<uint32_t>(c.creature, Cr_Stats) = (uint32_t)c.stats;
    return c;
}
static int totalLevel(const Cr& c) { int t = 0; for (int i = 0; i < at<uint8_t>(c.stats, St_ClassCount); ++i) t += at<uint8_t>(c.stats, St_ClassLevel + i * St_ClassStride); return t; }
static int newestClass(const Cr& c) { int n = at<uint8_t>(c.stats, St_ClassCount); return n ? at<uint8_t>(c.stats, St_ClassId + (n - 1) * St_ClassStride) : 0; }
// Real CanLevelUp 0x6B9790: can take level total+1 when the XP table says so.
static int realCan(const Cr& c) { int t = totalLevel(c); return t >= 1 && t < 50 && g_xpTbl[t] <= at<uint32_t>(c.stats, St_XP) ? 1 : 0; }

// ---- fake engine ----
static int g_ccRet = 0;
static Cr* g_leader = nullptr;
static int fakeCanLevelUp(void* s) { Cr* c = crByStats(s); return c ? realCan(*c) : 0; }
static void* fakeAccessor(void*) { return nullptr; }
static int fakeCharChange(void*) { return g_ccRet; }
static void* fakeTableGet(void*) { return P(0x1000F000); }
static void* fakePartyGet(void* party, int idx) { return party == P(0x1000F000) && idx == 0 ? P(0x1000F100) : nullptr; }
static void* fakeGetCreature(void*) { return g_leader ? P(g_leader->creature) : nullptr; }
static uint32_t g_ctlObj[4] = {0, 0x1234, 0, 0};
static void* fakeControlled(void*) { return g_ctlObj; }

static std::map<std::pair<void*, std::string>, int> g_vars;
static int g_adds = 0, g_sets = 0, g_ctors = 0;
static uintptr_t g_nameSlots[64];
static std::string nameOf(void* n) { return reinterpret_cast<const char*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(n))); }
static void* fakeStrCtor(void* buf, const char* s) {
    uintptr_t slot = g_nameSlots[g_ctors++ & 63];
    strncpy(reinterpret_cast<char*>(slot), s, 255);
    reinterpret_cast<uint32_t*>(buf)[0] = (uint32_t)slot;
    reinterpret_cast<uint32_t*>(buf)[1] = (uint32_t)strlen(s) + 1;
    return buf;
}
static void fakeStrDtor(void*) {}
static int fakeGFind(void* t, void* n) { return g_vars.count({t, nameOf(n)}) ? 0 : -1; }
static int fakeGAdd(void* t, void* n) { ++g_adds; g_vars[{t, nameOf(n)}] = 0; return 1; }
static void fakeGSet(void* t, void* n, uint32_t v) { ++g_sets; if (g_vars.count({t, nameOf(n)})) g_vars[{t, nameOf(n)}] = (int)v; }
static void fakeGGet(void* t, void* n, int* out) { auto it = g_vars.find({t, nameOf(n)}); *out = it == g_vars.end() ? 0 : it->second; }
static int fakeServerInit(void*) { return 0x77; }
static std::vector<std::string> g_banked;
static void fakeBankedAdd(void*, void* exo, int, uint8_t) { g_banked.push_back(nameOf(exo)); }
// LUO_G names present in table t / the GUID they spell.
static int globalsIn(void* t, uint32_t* guid = nullptr) {
    int found = 0; uint32_t g = 0;
    for (int i = 0; i < 32; ++i) { char nm[8]; pt::bitName(i, nm); auto it = g_vars.find({t, nm}); if (it == g_vars.end()) continue; ++found; if (it->second) g |= 1u << i; }
    if (guid) *guid = g;
    return found;
}
static void preloadGlobals(void* t, uint32_t guid, int count) { for (int i = 0; i < count && i < 32; ++i) { char nm[8]; pt::bitName(i, nm); g_vars[{t, nm}] = (guid >> i) & 1; } }

// pending: fake AddFeat / SetSkillDelta / push (as apply_test.cpp)
static uint32_t* hdr(void* arr) { return static_cast<uint32_t*>(arr); }
static void grow(void* arr, int esz) {
    uint32_t* h = hdr(arr);
    if (h[1] != h[2]) return;
    uint32_t cap = h[2] ? h[2] * 2 : 16;
    uintptr_t n = arenaAlloc(size_t(cap) * esz);
    if (h[1]) memcpy(P(n), P(uintptr_t(h[0])), size_t(h[1]) * esz);
    h[0] = (uint32_t)n; h[2] = cap;
}
static void fakeAddFeat(void* pending, uint16_t feat) {
    void* arr = static_cast<uint8_t*>(pending) + 0x18; uint32_t* h = hdr(arr);
    for (uint32_t i = 0; i < h[1]; ++i) if (reinterpret_cast<uint16_t*>(uintptr_t(h[0]))[i] == feat) return;
    grow(arr, 2); reinterpret_cast<uint16_t*>(uintptr_t(h[0]))[h[1]++] = feat;
}
static void fakeSetSkillDelta(void* pending, uint16_t idx, uint8_t v) {
    if (idx < NumSkills) reinterpret_cast<uint8_t*>(uintptr_t(*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pending) + 0x24)))[idx] = v;
}
static void fakePushU32(void* arr, uint32_t v) { grow(arr, 4); uint32_t* h = hdr(arr); reinterpret_cast<uint32_t*>(uintptr_t(h[0]))[h[1]++] = v; }
// A vanilla-like pending for a level taken as class `cls`: no feats/powers picked yet, zero skill deltas, 3 skill points unspent.
static uintptr_t makePending(int cls) {
    uintptr_t pd = arenaAlloc(Pd_Size);
    uintptr_t d = arenaAlloc(NumSkills);
    at<uint32_t>(pd, Pd_Deltas) = (uint32_t)d; at<uint16_t>(pd, Pd_Unspent) = 3;
    at<uint8_t>(pd, Pd_Class) = (uint8_t)cls;
    return pd;
}
// EBP chain for LevelUpOverrideEntry: [ebp] = next frame, [ebp+4] = that frame's return address. AutoLevelup (0x6BEDF0..+5244) on or off the chain.
// The mod walks the chain with uintptr_t reads, which are 8 bytes on a 64-bit host (only frame 0 is usable there); the 32-bit build (-m32, = the
// engine's real layout) walks all frames, so AutoLevelup sits one frame deeper there.
static uintptr_t makeChain(bool viaAuto) {
    uintptr_t b = arenaAlloc(0x200);
    const bool deep = sizeof(void*) == 4;
    const uint32_t autoRa[3] = {deep ? 0x00850086u : 0x006C00AFu, 0x006C00AFu, 0x0051C442u}, manRa[3] = {0x00661F26u, 0x0065FC91u, 0x0051C442u};
    const uint32_t* ra = viaAuto ? autoRa : manRa;
    for (int i = 0; i < 3; ++i) { at<uint32_t>(b + i * 0x40, 0) = i < 2 ? (uint32_t)(b + (i + 1) * 0x40) : 0; at<uint32_t>(b + i * 0x40, 4) = ra[i]; }
    return b;
}

// ---- config / log helpers ----
static void writeIni(const std::vector<std::string>& lines, const char* table) {
    FILE* f = fopen("levelup_override.ini", "w");
    for (auto& l : lines) fprintf(f, "%s\n", l.c_str());
    fprintf(f, "table=%s\n", table);
    fclose(f);
    g_stubTick += 5000;   // past the 1000 ms reload throttle and the 5 s banked-message gap
}
static std::string fileText(const char* name) { std::string s; if (FILE* f = fopen(name, "r")) { char b[512]; while (fgets(b, sizeof b, f)) s += b; fclose(f); } return s; }
static std::string readLog() { return fileText("levelup_override_log.txt"); }
static void clearLog() { remove("levelup_override_log.txt"); }
static bool logHas(const char* x) { return readLog().find(x) != std::string::npos; }
// Per log line after the `[tick] ` prefix and leading spaces (the contract in saves.toml; anchor with ^), case-sensitive ECMAScript.
static bool logRe(const std::string& re) {
    std::regex rx;
    try { rx = std::regex(re, std::regex::ECMAScript); } catch (std::regex_error&) { CHECK(false, "bad regex %s", re.c_str()); return false; }
    std::istringstream is(readLog()); std::string l;
    while (std::getline(is, l)) {
        size_t p = 0;
        if (!l.empty() && l[0] == '[') { size_t q = l.find("] "); if (q != std::string::npos) p = q + 2; }
        while (p < l.size() && l[p] == ' ') ++p;
        if (std::regex_search(l.substr(p), rx)) return true;
    }
    return false;
}

// ---- GuiKit / character-screen fakes (as ui_test.cpp) for `ui` / `click` actions ----
using namespace guikit;
static void* fAlloc(unsigned n) { return reinterpret_cast<void*>(arenaAlloc(n)); }
static void fCtor(void* t) {
    uintptr_t c = reinterpret_cast<uintptr_t>(t);
    at<uint32_t>(c, 0) = LabelVtable; at<uint32_t>(c, Label_Border) = BorderVtable; at<uint32_t>(c, Label_Text) = TextVtable;
    at<uint32_t>(c, Label_Text + Text_Renderer) = 0x1000; at<int>(c, Ctl_Flags) = 0x2;
}
static uintptr_t g_exoSlots[64]; static int g_exoNext = 0;
static void* fExoCtor(void* e, const char* s) { uintptr_t sl = g_exoSlots[g_exoNext++ & 63]; strncpy(reinterpret_cast<char*>(sl), s, 255); memset(e, 0, 16); *reinterpret_cast<uint32_t*>(e) = (uint32_t)sl; return e; }
static const char* exoStr(void* e) { return reinterpret_cast<const char*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(e))); }
static void fExoDtor(void*) {}
static void fInit(void*, void* ctl, void* exo, int, int) {
    uintptr_t c = reinterpret_cast<uintptr_t>(ctl);
    const bool plain = !strcmp(exoStr(exo), "LBL_EXPERIENCE_STAT");
    at<int>(c, Ctl_Id) = -1;
    const int u[4] = {plain ? 60 : 54, plain ? 449 : 431, plain ? 108 : 223, plain ? 18 : 39};
    for (int k = 0; k < 4; ++k) at<int>(c, Ctl_X + 4 * k) = u[k] * 2;
}
static std::map<uintptr_t, std::string> g_text;
static void fTextSet(void* sub, void* exo) { g_text[reinterpret_cast<uintptr_t>(sub) - (Label_Text + Text_StringSub)] = exoStr(exo); }
static void fFont(void*, const char*) {}
static void fAlign(void*, unsigned) {}
static void fColor(void*, float*) {}
static void fList(void* list, void* v) { uintptr_t l = reinterpret_cast<uintptr_t>(list); uint32_t arr = at<uint32_t>(l, 0); int cnt = at<int>(l, 4); at<uint32_t>(arr, cnt * 4) = (uint32_t)reinterpret_cast<uintptr_t>(v); at<int>(l, 4) = cnt + 1; }
static void* fLoadImage(const char*) { return reinterpret_cast<void*>(0x5000); }
static void fSound(void*, char) {}
static void fSetRect(uint32_t, uintptr_t ctl, int* r) { for (int k = 0; k < 4; ++k) at<int>(ctl, Ctl_X + 4 * k) = r[k]; }
static int curX = 0, curY = 0; static bool lmb = false;
static bool fCursor(int* x, int* y, int* w, int* h) { *x = curX; *y = curY; *w = 1920; *h = 800; return true; }
static bool fFg() { return true; }
static bool fLmb() { return lmb; }
static uintptr_t g_panel = 0, g_panelEbp = 0;
static void ensurePanel() {
    if (g_panel) return;
    g_panel = arenaAlloc(0x200);
    uintptr_t arr = arenaAlloc(512 * 4);
    at<uint32_t>(g_panel, Panel_CtlArray) = (uint32_t)arr; at<int>(g_panel, Panel_CtlCount) = 70;
    uintptr_t btn = arenaAlloc(0x150); at<uint32_t>(btn, 0) = LabelVtable;
    at<int>(btn, Ctl_X) = 724; at<int>(btn, Ctl_Y) = 650; at<int>(btn, Ctl_W) = 188; at<int>(btn, Ctl_H) = 80;
    at<uint32_t>(arr, 66 * 4) = (uint32_t)btn;
    uintptr_t frame = arenaAlloc(0x400); g_panelEbp = frame + 0x300;
    at<uint32_t>(g_panelEbp, -0x288) = (uint32_t)g_panel;
    LuoUiPanelCtor((int)g_panel);
}
static int uiTickAs(Cr& c) { ensurePanel(); return LuoHold_84F221(P(c.stats), P(g_panelEbp)); }

// ---- the engine world ----
static uintptr_t g_app = 0, g_srv = 0, g_intl = 0;
static void* liveTbl() { return P(g_intl + 0x100FC); }
static void initWorld(const char* tmplName) {
    char tmpl[64]; snprintf(tmpl, sizeof tmpl, "/tmp/%s_XXXXXX", tmplName);
    if (!mkdtemp(tmpl) || chdir(tmpl)) { perror("mkdtemp"); exit(5); }
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); exit(5); }
    g_arena = static_cast<uint8_t*>(m);
    if (mmap(P(0x00987000), 0x10000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0) == MAP_FAILED) { perror("mmap 0x987000"); exit(5); }
    if (mmap(P(0x00A1B000), 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0) == MAP_FAILED) { perror("mmap 0xA1B000"); exit(5); }
    at<uint32_t>(LabelVtable, 4) = 0x1234;
    g_stubVirtualQueryAll = true;
    g_eng.checkCode = false;
    g_eng.addFeat = reinterpret_cast<uintptr_t>(&fakeAddFeat); g_eng.setSkillDelta = reinterpret_cast<uintptr_t>(&fakeSetSkillDelta); g_eng.pushU32 = reinterpret_cast<uintptr_t>(&fakePushU32);
    g_eng.canLevelUp = reinterpret_cast<uintptr_t>(&fakeCanLevelUp); g_eng.optAccessor = reinterpret_cast<uintptr_t>(&fakeAccessor);
    g_eng.controlled = reinterpret_cast<uintptr_t>(&fakeControlled); g_eng.charChange = reinterpret_cast<uintptr_t>(&fakeCharChange);
    g_eng.tableGet = reinterpret_cast<uintptr_t>(&fakeTableGet); g_eng.partyGet = reinterpret_cast<uintptr_t>(&fakePartyGet); g_eng.getCreature = reinterpret_cast<uintptr_t>(&fakeGetCreature);
    g_eng.strCtor = reinterpret_cast<uintptr_t>(&fakeStrCtor); g_eng.strDtor = reinterpret_cast<uintptr_t>(&fakeStrDtor);
    g_eng.guidFind = reinterpret_cast<uintptr_t>(&fakeGFind); g_eng.guidAdd = reinterpret_cast<uintptr_t>(&fakeGAdd);
    g_eng.guidSet = reinterpret_cast<uintptr_t>(&fakeGSet); g_eng.guidGet = reinterpret_cast<uintptr_t>(&fakeGGet);
    g_eng.serverInit = reinterpret_cast<uintptr_t>(&fakeServerInit); g_eng.bankedAdd = reinterpret_cast<uintptr_t>(&fakeBankedAdd);
    for (auto& sl : g_nameSlots) sl = arenaAlloc(256);
    for (auto& sl : g_exoSlots) sl = arenaAlloc(256);
    // app chain: [appGlobal] = app; [app+4] = client; [client+4] = clientInternal; [ci+0x40] = GuiInGame; [app+8] = srv; [srv+4] = intl
    uintptr_t cell = arenaAlloc(16), client = arenaAlloc(32), ci = arenaAlloc(0x100), gui = arenaAlloc(32);
    g_app = arenaAlloc(32); g_srv = arenaAlloc(32); g_intl = arenaAlloc(0x10200);
    at<uint32_t>(cell, 0) = (uint32_t)g_app; at<uint32_t>(g_app, 4) = (uint32_t)client; at<uint32_t>(client, 4) = (uint32_t)ci; at<uint32_t>(ci, 0x40) = (uint32_t)gui;
    at<uint32_t>(g_app, 8) = (uint32_t)g_srv; at<uint32_t>(g_srv, 4) = (uint32_t)g_intl;
    g_eng.appGlobal = cell;
    // rules object at the REAL global 0xA1B4D0 (onLevelUp reads it raw): skill count +0x123, XP table +0x38
    uintptr_t rules = arenaAlloc(0x200);
    at<uint32_t>(0x00A1B4D0, 0) = (uint32_t)rules; at<uint8_t>(rules, Rules_SkillCount) = NumSkills;
    for (int i = 0; i < 51; ++i) at<uint32_t>(rules, 0x38 + 4 * i) = g_xpTbl[i];
    g_eng.rulesGlobal = 0x00A1B4D0;
    uintptr_t screen = arenaAlloc(16), snd = arenaAlloc(16);
    at<uint32_t>(snd, 0) = (uint32_t)arenaAlloc(16);
    guikit::Engine& e = g_gk;
    e.alloc = fAlloc; e.labelCtor = fCtor; e.initControl = fInit; e.exoCtor = fExoCtor; e.exoDtor = fExoDtor; e.textSet = fTextSet; e.setFont = fFont; e.setAlign = fAlign;
    e.textColor = fColor; e.listAdd = fList; e.loadImage = fLoadImage; e.playSound = fSound; e.callSetRect = fSetRect;
    e.screenWAddr = screen; e.screenHAddr = screen + 4; e.guiSoundMgrAddr = snd;
    e.cursorClient = fCursor; e.gameForeground = fFg; e.lbuttonDown = fLmb;
}
static bool loadXp(const std::string& path) {
    std::ifstream f(path); std::string l; int n = 0;
    while (std::getline(f, l)) { if (l.empty() || l[0] == '#') continue; unsigned lv; unsigned long x; if (sscanf(l.c_str(), "%u %lu", &lv, &x) == 2 && lv >= 1 && lv <= 51) { g_xpTbl[lv - 1] = (uint32_t)x; ++n; } }
    return n == 51;
}

// ---- scenario data ----
struct Scn {
    std::string id; uint32_t globals = 0; int nGlobals = 0; bool ptSet = false; uint32_t ptGuid = 0;
    struct CrSpec { std::string tag; std::vector<std::pair<int, int>> classes; uint32_t xp; bool inParty, leader; };
    std::vector<CrSpec> cr; std::vector<std::string> ini, companions, acts, events, absent; std::vector<std::pair<std::string, std::string>> decisions;
};
static bool readScn(const char* path, const std::string& id, Scn& out) {
    std::ifstream f(path); std::string line; bool in = false;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line); std::string k; ss >> k;
        std::string rest; std::getline(ss, rest); if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
        if (k == "SCENARIO") { if (in) return true; in = rest == id; if (in) out.id = id; continue; }
        if (!in) continue;
        std::istringstream rs(rest);
        if (k == "GLOBALS") rs >> std::hex >> out.globals >> std::dec >> out.nGlobals;
        else if (k == "CREATURE") {
            Scn::CrSpec c; std::string cl; int ip, ld; rs >> c.tag >> cl >> c.xp >> ip >> ld; c.inParty = ip; c.leader = ld;
            std::istringstream cs(cl); std::string it;
            while (std::getline(cs, it, ',')) { int a, b; if (sscanf(it.c_str(), "%d:%d", &a, &b) == 2) c.classes.push_back({a, b}); }
            out.cr.push_back(c);
        }
        else if (k == "INI") out.ini.push_back(rest);
        else if (k == "PTGUID") { if (rest != "none") { out.ptSet = true; rs >> std::hex >> out.ptGuid; } }
        else if (k == "COMPANION") out.companions.push_back(rest);
        else if (k == "ACT") out.acts.push_back(rest);
        else if (k == "EVENT") out.events.push_back(rest);
        else if (k == "ABSENT") out.absent.push_back(rest);
        else if (k == "DECISION") { std::string dk, dv; rs >> dk; std::getline(rs, dv); if (!dv.empty() && dv[0] == ' ') dv.erase(0, 1); out.decisions.push_back({dk, dv}); }
        else if (k == "END") return true;
    }
    return in;
}

static std::map<std::string, std::string> g_dec;
static const char* g_preset = "atton__melee_dual_light_early";

// One action of the script. Returns false on an unknown action.
static bool act(const std::string& line, void* tabLoad, void* tabSave) {
    std::istringstream ss(line); std::string a, arg; ss >> a >> arg;
    if (a == "load") {
        LuoGuidLoad(tabLoad, 1);
        char b[16]; snprintf(b, sizeof b, "g%08x", (unsigned)ptGuid()); g_dec["guid"] = b;
        g_dec["load.found"] = std::to_string(globalsIn(tabLoad));
    } else if (a == "save") {
        LuoGuidSave(tabSave);
        g_dec["save.globals"] = std::to_string(globalsIn(tabSave));
    } else if (a == "newserver") {
        LuoGuidNewServer(P(g_app));
        char b[16]; snprintf(b, sizeof b, "g%08x", (unsigned)ptGuid()); g_dec["guid"] = b;
    } else if (a == "tick") {   // the hold gate at 0x74F3C7: HOLD (real can, gated 0) / PASS / NONE (real cannot level)
        Cr* c = crByTag(arg); if (!c) return false;
        const int real = realCan(*c), r = LuoHold_74F3C7(P(c->stats));
        g_dec[arg + ".canLevelUp"] = !real ? "NONE" : r ? "PASS" : "HOLD";
    } else if (a == "auto") {   // an auto level-up (AutoLevelup on the chain) if the gate lets it through
        Cr* c = crByTag(arg); if (!c) return false;
        const int real = realCan(*c), r = LuoHold_74F3C7(P(c->stats));
        if (!real || !r) { g_dec[arg + ".autoLevel"] = "BLOCKED"; return true; }
        const size_t before = readLog().size();
        uintptr_t pend = makePending(newestClass(*c));
        LevelUpOverrideEntry(P(c->stats), P(0x006B9770), P(pend), 1, P(makeChain(true)));
        const std::string seg = readLog().substr(before);
        g_dec[arg + ".autoLevel"] = seg.find("APPLIED tag=") != std::string::npos ? "APPLIED" : seg.find("WOULD REWRITE") != std::string::npos ? "WOULD_REWRITE" :
            seg.find("ALREADY") != std::string::npos ? "ALREADY" : seg.find("SKIP") != std::string::npos ? "SKIP" : seg.find(" DRY ") != std::string::npos ? "DRY" : "NONE";
    } else if (a == "guard") {   // GUI level-up button guard with `arg` as the party leader
        Cr* c = crByTag(arg); if (!c) return false;
        g_leader = c; g_ccRet = 0;
        g_dec[arg + ".guard"] = LuoGuardGui_84FD2F(P(7)) ? "HOLD" : "PASS";
    } else if (a == "ui") {
        Cr* c = crByTag(arg); if (!c) return false;
        uiTickAs(*c); uiTickAs(*c);
    } else if (a == "click") {   // arg = control name; the UI tick uses the creature of the previous `ui` (or the first one)
        Cr* c = crByTag(g_cr.empty() ? "" : g_cr[0].tag); if (!c) return false;
        int idx = -1; for (int i = 0; i < C_N; ++i) if (arg == kUiName[i]) idx = i;
        if (idx < 0) return false;
        ensurePanel(); UiPanel* r = findUi(g_panel);
        int x, y, w, h;
        if (!r || !r->c[idx] || !getExtent(r->c[idx], &x, &y, &w, &h)) { CHECK(false, "click %s: no control extent", arg.c_str()); return true; }
        curX = x + w / 2; curY = y + h / 2;
        lmb = false; uiTickAs(*c); lmb = true; uiTickAs(*c); lmb = false; uiTickAs(*c);
        g_dec["ui.open"] = r->open ? "1" : "0";
    } else return false;
    return true;
}

static int runScenario(const char* datPath, const std::string& id) {
    Scn s;
    if (!readScn(datPath, id, s)) { fprintf(stderr, "scenario %s not found in %s\n", id.c_str(), datPath); return 2; }
    g_ctx = id;
    const char* table = getenv("LUO_TABLE");
    initWorld("luo_scn");
    void* tabLoad = P(arenaAlloc(64)); void* tabSave = P(arenaAlloc(64));
    preloadGlobals(tabLoad, s.globals, s.nGlobals);
    for (auto& c : s.cr) addCreature(c.tag, c.classes, c.xp, c.inParty, c.leader);
    for (auto& c : g_cr) if (c.leader) g_leader = &c;
    if (!g_leader) for (auto& c : g_cr) if (c.inParty) { g_leader = &c; break; }
    std::vector<std::string> ini = s.ini;
    remove("levelup_playthroughs.ini");
    if (s.ptSet) { FILE* f = fopen("levelup_playthroughs.ini", "w"); fprintf(f, "[g%08x]\n", (unsigned)s.ptGuid); for (auto& c : s.companions) { size_t sp = c.find(' '); fprintf(f, "%s=%s\n", c.substr(0, sp).c_str(), c.substr(sp + 1).c_str()); } fclose(f); }
    writeIni(ini, table);
    clearLog();
    for (auto& a : s.acts) { if (!act(a, tabLoad, tabSave)) CHECK(false, "unknown or failed action '%s'", a.c_str()); }
    // banked count: the last BANKED line
    { std::string l = readLog(); size_t p = l.rfind("BANKED tag="); if (p != std::string::npos) { size_t q = l.find(" n=", p); if (q != std::string::npos) g_dec["banked"] = std::to_string(atoi(l.c_str() + q + 3)); } }
    for (auto& e : s.events) CHECK(logRe(e), "expected event /%s/ not in log:\n%s", e.c_str(), readLog().c_str());
    for (auto& e : s.absent) CHECK(!logRe(e), "forbidden event /%s/ in log:\n%s", e.c_str(), readLog().c_str());
    for (auto& d : s.decisions) { auto it = g_dec.find(d.first); CHECK(it != g_dec.end() && it->second == d.second, "decision %s = '%s' (want '%s')", d.first.c_str(), it == g_dec.end() ? "<unset>" : it->second.c_str(), d.second.c_str()); }
    printf("scenario %s: checks %d failures %d", id.c_str(), g_checks, g_fail);
    for (auto& d : g_dec) printf(" %s=%s", d.first.c_str(), d.second.c_str());
    printf("\n");
    return g_fail ? 1 : 0;
}

// ---- unit mode: onLevelUp T2 + LUO_G removal safety ----
static int runUnit(const char* table) {
    initWorld("luo_unit");
    const std::string P_ = g_preset;
    // 1. decideAct truth table
    {
        g_ctx = "decideAct";
        for (int auto_ = 0; auto_ < 2; ++auto_)
            for (const char* st : {"rewrite", "already", "no-record", "class-mismatch", "negative-unspent"})
                for (int ab = 0; ab < 2; ++ab)
                    for (int ca = 0; ca < 2; ++ca) {
                        LuAct want = !auto_ ? LuAct::SkipManual : !strcmp(st, "already") ? LuAct::Already : strcmp(st, "rewrite") ? LuAct::SkipStatus : (!ab || !ca) ? LuAct::WouldRewrite : LuAct::Apply;
                        CHECK(decideAct(auto_, st, ab, ca) == want, "decideAct(auto=%d,%s,build=%d,cfg=%d)", auto_, st, ab, ca);
                    }
    }
    // 2. onLevelUp through LevelUpOverrideEntry with a fake caller chain
    g_ctx = "onLevelUp";
    Cr& rebase = addCreature("atton", {{2, 3}, {5, 1}}, 28000, true, false);   // Scoundrel 3 + Sentinel 1 -> key 5, a record exists (REBASE/PATH)
    Cr& held = addCreature("atton2", {{2, 7}}, 28000, true, false);             // unconfigured tag
    (void)held;
    auto devIni = [&](const char* mode, int apply) {
        writeIni({std::string("mode=") + mode, "verbose=1", "apply=" + std::to_string(apply), "preset.atton=" + P_, "hold.atton=1", "instant.atton=0"}, table);
        clearLog();
    };
    auto fire = [&](Cr& c, bool viaAuto, uintptr_t* pdOut = nullptr) {
        uintptr_t pd = makePending(newestClass(c));
        if (pdOut) *pdOut = pd;
        LevelUpOverrideEntry(P(c.stats), P(0x006B9770), P(pd), 1, P(makeChain(viaAuto)));
    };
    uintptr_t pd = 0;
    // DECIDE + DRY, mode=log: the decision is logged, nothing is rewritten, no SKIP/APPLIED
    devIni("log", 1);
    fire(rebase, true, &pd);
    CHECK(logHas("DECIDE tag=atton preset=" "atton__melee_dual_light_early key=5 decision=") && logHas(" DRY ") && !logHas("APPLIED") && !logHas("SKIP") && !logHas("WOULD REWRITE"), "mode=log (auto):\n%s", readLog().c_str());
    CHECK(logHas(sizeof(void*) == 4 ? "flow=auto-button" : "flow=auto-other") &&
          logHas(sizeof(void*) == 4 ? "ret=006b9770(ValidateLevelUp) <- 00850086(DoAutoLevelup) <- 006c00af(AutoLevelup)" : "ret=006b9770(ValidateLevelUp) <- 006c00af(AutoLevelup)"), "chain text:\n%s", readLog().c_str());
    { PendDump d; readPend(pd, NumSkills, &d); CHECK(d.ok && d.pd.nFeats == 0 && d.pd.unspent == 3, "mode=log must not touch the pending"); }
    // SKIP manual: rewrite mode, AutoLevelup NOT on the chain -> the player's picks kept
    devIni("rewrite", 1);
    fire(rebase, false, &pd);
    CHECK(logHas("DECIDE tag=atton") && logHas("SKIP manual " "atton__melee_dual_light_early key=5 flow=manual-gui") && !logHas("APPLIED"), "manual:\n%s", readLog().c_str());
    { PendDump d; readPend(pd, NumSkills, &d); CHECK(d.ok && d.pd.nFeats == 0 && d.pd.unspent == 3, "manual flow must not touch the pending"); }
    // WOULD REWRITE: apply=0 on the auto chain
    devIni("rewrite", 0);
    fire(rebase, true, &pd);
    CHECK(logHas("WOULD REWRITE atton__melee_dual_light_early key=5 (apply=0)") && !logHas("APPLIED"), "apply=0:\n%s", readLog().c_str());
    { PendDump d; readPend(pd, NumSkills, &d); CHECK(d.ok && d.pd.nFeats == 0, "apply=0 must not touch the pending"); }
    // APPLIED: rewrite + apply=1 on the auto chain; the pending now equals computeRewrite; a replay is ALREADY
    devIni("rewrite", 1);
    fire(rebase, true, &pd);
    CHECK(logHas("APPLIED tag=atton__melee_dual_light_early key=5 ok=1"), "apply:\n%s", readLog().c_str());
    {
        PendDump d; readPend(pd, NumSkills, &d);
        const Preset* pr = findPreset(P_.c_str()); const Rec* rec = nullptr; int rl = -1;
        uint8_t ids[2] = {2, 5}, lv[2] = {3, 1};
        levelNow(ids, lv, 2, pr, 5, true, 0, &rec, &rl);
        Pend base{}; base.cls = 5; base.unspent = 3;
        Result r = computeRewrite(rec, base, 0);
        CHECK(d.ok && !strcmp(r.status, "rewrite") && samePend(d.pd, r.p), "pending after APPLIED != computeRewrite (status %s)", r.status);
        clearLog();
        LevelUpOverrideEntry(P(rebase.stats), P(0x006B9770), P(pd), 1, P(makeChain(true)));
        CHECK(logHas("ALREADY (replay)") && !logHas("APPLIED"), "replay:\n%s", readLog().c_str());
    }
    // AutoLevelup found as the direct return address (not only on the EBP chain)
    devIni("rewrite", 1);
    { uintptr_t p2 = makePending(5); LevelUpOverrideEntry(P(rebase.stats), P(0x006BEDF0 + 0x100), P(p2), 1, P(makeChain(false))); CHECK(logHas("APPLIED tag="), "AutoLevelup as ret:\n%s", readLog().c_str()); }
    // unconfigured tag: no DECIDE; mode=off: nothing logged; HOLD mid-loop never rewrites
    devIni("rewrite", 1);
    fire(*crByTag("atton2"), true);
    CHECK(logHas("tag=atton2") && !logHas("DECIDE") && !logHas("APPLIED"), "unconfigured tag:\n%s", readLog().c_str());
    devIni("off", 1);
    fire(rebase, true, &pd);
    CHECK(!logHas("DECIDE") && !logHas("APPLIED"), "mode=off:\n%s", readLog().c_str());
    devIni("rewrite", 1);
    { Cr& h7 = addCreature("atton", {{2, 7}}, 28000, true, false); fire(h7, true, &pd);
      CHECK(logHas("decision=HOLD_MIDLOOP") && !logHas("APPLIED"), "HOLD mid-loop is vanilla:\n%s", readLog().c_str()); }

    // 3. LUO_G removal safety
    g_ctx = "luog";
    {
        remove("levelup_playthroughs.ini");
        writeIni({"mode=rewrite", "preset.atton=" + P_, "hold.atton=1"}, table);
        clearLog(); g_adds = g_sets = 0;
        void* t0 = P(arenaAlloc(64)); void* t1 = P(arenaAlloc(64)); void* t2 = P(arenaAlloc(64));
        LuoGuidLoad(t0, 1);   // a save with no LUO_G at all (pre-mod / mod removed)
        CHECK(ptGuid() == 0 && logHas("GUID load g00000000 found=0/32"), "no LUO_G -> unconfigured, logged:\n%s", readLog().c_str());
        for (int i = 0; i < 3; ++i) LuoHold_74F3C7(P(rebase.stats));
        LuoGuidSave(t1);
        CHECK(g_adds == 0 && g_sets == 0 && g_vars.empty() && logHas("GUID save none"), "no writes before Apply (adds %d sets %d)", g_adds, g_sets);
        // Apply: the first ptApply creates a GUID and writes 32 booleans to the LIVE table
        CHECK(ptApply("atton", P_.c_str(), false, true, 0) && ptGuid() != 0, "ptApply");
        const uint32_t guid = ptGuid(); uint32_t back = 0;
        CHECK(g_adds == 32 && globalsIn(liveTbl(), &back) == 32 && back == guid, "Apply writes 32 globals (adds %d)", g_adds);
        // save adds the 32 globals to the saved table
        LuoGuidSave(t2);
        uint32_t sg = 0;
        CHECK(globalsIn(t2, &sg) == 32 && sg == guid, "save -> 32 globals with the GUID");
        // load of the full set restores the GUID; a partial set (16 of 32) is treated as none and logged
        LuoGuidNewServer(P(g_app));
        LuoGuidLoad(t2, 1);
        CHECK(ptGuid() == guid, "load restores");
        void* tp = P(arenaAlloc(64)); preloadGlobals(tp, guid, 16);
        const int adds = g_adds, sets = g_sets;
        LuoGuidLoad(tp, 1);
        CHECK(ptGuid() == 0 && logHas("found=16/32") && g_adds == adds && g_sets == sets, "partial set -> GUID 0, logged, no writes");
        // and a clean load afterwards (no LUO_G) writes nothing either
        LuoGuidLoad(t0, 1);
        CHECK(ptGuid() == 0 && g_adds == adds && g_sets == sets, "load without globals writes nothing");
    }
    printf("scenario_test --unit checks %d failures %d\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "usage: scenario_test <levelup_paths.ini> --unit | <scenarios.dat> <id>\n"); return 2; }
    const char* table = argv[1];
    setenv("LUO_TABLE", table, 1);
    const char* xp = getenv("LUO_EXPTABLE");
    if (!xp || !loadXp(xp)) { fprintf(stderr, "LUO_EXPTABLE (testdata/exptable.tsv) missing or incomplete\n"); return 2; }
    if (!strcmp(argv[2], "--unit")) return runUnit(table);
    if (argc < 4) return 2;
    return runScenario(argv[2], argv[3]);
}
