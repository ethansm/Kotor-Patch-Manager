// Host test of the 0.3.0-a gate handlers (hold gate, instant gate, GUI guards, Auto OK backstop) on fake engine structs in a MAP_32BIT arena.
// The engine functions the call sites used to call (CanLevelUp, options accessor, 0x740FC0, 0x73FB90, 0x7E5DA0, 0x77D800) are fakes;
// the config is a temp levelup_override.ini in a temp cwd (table = argv[1], the real runtime table). Run via parity.py.
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <sys/mman.h>
#include <unistd.h>
#include <map>
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
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fail; fprintf(stderr, "FAIL line %d: ", __LINE__); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

// ---- fake engine ----
static int g_canRet = 1, g_ccRet = 0;
static void* g_optPtr = nullptr;
static void* g_creatureRet = nullptr;
static void* g_lastTableEcx = nullptr;
static int g_fakeCalls = 0;
static int fakeCanLevelUp(void*) { ++g_fakeCalls; return g_canRet; }
static void* fakeAccessor(void*) { ++g_fakeCalls; return g_optPtr; }
static int fakeCharChange(void*) { ++g_fakeCalls; return g_ccRet; }
static void* fakeTableGet(void* ecx) { ++g_fakeCalls; g_lastTableEcx = ecx; return reinterpret_cast<void*>(0x1000F000); }
static void* fakePartyGet(void* party, int idx) { ++g_fakeCalls; return party == reinterpret_cast<void*>(0x1000F000) && idx == 0 ? reinterpret_cast<void*>(0x1000F100) : nullptr; }
static void* fakeGetCreature(void*) { ++g_fakeCalls; return g_creatureRet; }
static uint32_t g_ctlObj[4] = {0, 0x00001234u, 0, 0};
static void* g_ctlEcx = nullptr;
static void* fakeControlled(void* ecx) { ++g_fakeCalls; g_ctlEcx = ecx; return g_ctlObj; }

// ---- fake globals table (M3): values keyed by (table, upper-case-agnostic name) ----
static std::map<std::pair<void*, std::string>, int> g_vars;
static int g_ctors = 0, g_dtors = 0, g_addDup = 0, g_setMissing = 0, g_adds = 0;
static uintptr_t g_nameSlots[64];
static void* g_serverInitEcx = nullptr;
static std::string nameOf(void* n) { return reinterpret_cast<const char*>(static_cast<uintptr_t>(*reinterpret_cast<uint32_t*>(n))); }
static void* fakeStrCtor(void* buf, const char* s) {
    uintptr_t slot = g_nameSlots[g_ctors++ & 63];
    strcpy(reinterpret_cast<char*>(slot), s);
    reinterpret_cast<uint32_t*>(buf)[0] = (uint32_t)slot;
    reinterpret_cast<uint32_t*>(buf)[1] = (uint32_t)strlen(s) + 1;
    return buf;
}
static void fakeStrDtor(void*) { ++g_dtors; }
static int fakeGFind(void* t, void* n) { return g_vars.count({t, nameOf(n)}) ? 0 : -1; }
static int fakeGAdd(void* t, void* n) { ++g_adds; if (g_vars.count({t, nameOf(n)})) ++g_addDup; g_vars[{t, nameOf(n)}] = 0; return 1; }
static void fakeGSet(void* t, void* n, uint32_t v) { if (!g_vars.count({t, nameOf(n)})) { ++g_setMissing; return; } g_vars[{t, nameOf(n)}] = (int)v; }
static void fakeGGet(void* t, void* n, int* out) { auto it = g_vars.find({t, nameOf(n)}); *out = it == g_vars.end() ? 0 : it->second; }
static int fakeServerInit(void* app) { g_serverInitEcx = app; return 0x77; }
// GUID currently stored in the fake table `t` (bits of the LUO_G names that exist) and how many names exist.
static uint32_t fakeGuid(void* t, int* found) {
    uint32_t g = 0; *found = 0;
    for (int i = 0; i < 32; ++i) { char nm[8]; pt::bitName(i, nm); auto it = g_vars.find({t, nm}); if (it == g_vars.end()) continue; ++*found; if (it->second) g |= 1u << i; }
    return g;
}

// ---- fake creature / stats ----
struct Fake { uintptr_t stats, creature; };
static Fake makeCreature(const char* tag, std::vector<std::pair<int, int>> classes) {
    Fake f;
    f.stats = arenaAlloc(0x400);
    f.creature = arenaAlloc(0x1200);
    uint8_t* st = reinterpret_cast<uint8_t*>(f.stats);
    st[St_ClassCount] = (uint8_t)classes.size();
    for (size_t i = 0; i < classes.size(); ++i) { st[St_ClassId + i * St_ClassStride] = (uint8_t)classes[i].first; st[St_ClassLevel + i * St_ClassStride] = (uint8_t)classes[i].second; }
    *reinterpret_cast<uint32_t*>(st + St_Creature) = (uint32_t)f.creature;
    uint8_t* cr = reinterpret_cast<uint8_t*>(f.creature);
    uintptr_t str = arenaAlloc(64);
    memcpy(reinterpret_cast<void*>(str), tag, strlen(tag));
    *reinterpret_cast<uint32_t*>(cr + Cr_Tag) = (uint32_t)str;
    *reinterpret_cast<uint32_t*>(cr + Cr_Tag + 4) = (uint32_t)strlen(tag);
    *reinterpret_cast<uint32_t*>(cr + Cr_Stats) = (uint32_t)f.stats;
    return f;
}
static void* P(uintptr_t a) { return reinterpret_cast<void*>(a); }

// ---- config / log helpers ----
static void setIni(const char* body, const char* table) {
    FILE* f = fopen("levelup_override.ini", "w");
    fprintf(f, "%s\ntable=%s\n", body, table);
    fclose(f);
    g_stubTick += 5000;   // past the 1000 ms reload throttle; the next gate call re-reads the ini
}
static std::string readLog() {
    std::string s; if (FILE* f = fopen("levelup_override_log.txt", "r")) { char b[512]; while (fgets(b, sizeof b, f)) s += b; fclose(f); }
    return s;
}
static void clearLog() { remove("levelup_override_log.txt"); }
static bool logHas(const char* x) { return readLog().find(x) != std::string::npos; }

int main(int argc, char** argv) {
    if (argc < 2) return 2;
    const char* table = argv[1];
    char tmpl[] = "/tmp/gate_test_XXXXXX";
    if (!mkdtemp(tmpl) || chdir(tmpl)) { perror("mkdtemp"); return 5; }
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    g_eng.checkCode = false;
    g_eng.canLevelUp = reinterpret_cast<uintptr_t>(&fakeCanLevelUp);
    g_eng.optAccessor = reinterpret_cast<uintptr_t>(&fakeAccessor);
    g_eng.controlled = reinterpret_cast<uintptr_t>(&fakeControlled);
    g_eng.charChange = reinterpret_cast<uintptr_t>(&fakeCharChange);
    g_eng.tableGet = reinterpret_cast<uintptr_t>(&fakeTableGet);
    g_eng.partyGet = reinterpret_cast<uintptr_t>(&fakePartyGet);
    g_eng.getCreature = reinterpret_cast<uintptr_t>(&fakeGetCreature);
    // app global chain: [appGlobal] = app, [app+4] = client
    uintptr_t cell = arenaAlloc(16), app = arenaAlloc(32);
    *reinterpret_cast<uint32_t*>(cell) = (uint32_t)app;
    *reinterpret_cast<uint32_t*>(app + 4) = 0x10001234;
    g_eng.appGlobal = cell;
    g_eng.rulesGlobal = arenaAlloc(16);   // zero cell: the banked-level count reads no rules object (R11 is covered by ui_test.cpp)
    for (auto& sl : g_nameSlots) sl = arenaAlloc(32);
    g_eng.strCtor = reinterpret_cast<uintptr_t>(&fakeStrCtor);
    g_eng.strDtor = reinterpret_cast<uintptr_t>(&fakeStrDtor);
    g_eng.guidFind = reinterpret_cast<uintptr_t>(&fakeGFind);
    g_eng.guidAdd = reinterpret_cast<uintptr_t>(&fakeGAdd);
    g_eng.guidSet = reinterpret_cast<uintptr_t>(&fakeGSet);
    g_eng.guidGet = reinterpret_cast<uintptr_t>(&fakeGGet);
    g_eng.serverInit = reinterpret_cast<uintptr_t>(&fakeServerInit);
    // live globals chain: [app+8] = srv, [srv+4] = intl, table = intl + 0x100FC
    uintptr_t srv = arenaAlloc(32), intl = arenaAlloc(0x10200);
    const uintptr_t liveTbl = intl + 0x100FC;
    *reinterpret_cast<uint32_t*>(app + 8) = (uint32_t)srv;
    *reinterpret_cast<uint32_t*>(srv + 4) = (uint32_t)intl;

    const char* PRESET = "atton__melee_dual_light_early";
    Fake heldAtton = makeCreature("Atton", {{2, 7}});
    Fake sentinel = makeCreature("atton", {{2, 7}, {5, 0}});
    Fake onPath = makeCreature("atton", {{2, 5}});
    Fake pc = makeCreature("", {{0, 7}});
    Fake other = makeCreature("hk47", {{6, 7}});
    // the preset must hold [(2,7)] and not hold [(2,7),(5,0)] / [(2,5)] (levelNow twin of the python rule)
    char ini[512];

    // 1. mode=rewrite, no tag configured -> pass-through
    setIni("mode=rewrite", table);
    g_canRet = 1;
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "unconfigured must pass through");
    CHECK(LuoHold_84F221(P(other.stats), nullptr) == 1, "unconfigured other tag must pass through");

    // 2. held Atton: rewrite -> 0; real 0 stays 0; log mode -> r + WOULD HOLD
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=0", PRESET);
    setIni(ini, table); clearLog();
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "held atton must be gated in rewrite mode");
    for (int i = 0; i < 5; ++i) LuoHold_5FD7CE(P(heldAtton.stats));   // per-frame calls: no more log lines
    CHECK(logHas("HOLD tag=Atton key=8 site=0x74F3C7"), "HOLD log line missing:\n%s", readLog().c_str());
    { std::string l = readLog(); size_t n = 0, p = 0; while ((p = l.find("HOLD tag=", p)) != std::string::npos) { ++n; ++p; } CHECK(n == 1, "HOLD logged %zu times (want 1)", n); }
    g_canRet = 0;
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "real 0 must stay 0");
    g_canRet = 1;
    CHECK(LuoHold_757808(P(onPath.stats)) == 1, "on-path atton must pass through");
    CHECK(LuoHold_75FCB8(P(pc.stats)) == 1, "PC (empty tag) must pass through");
    CHECK(LuoHold_75218B(P(other.stats)) == 1, "unconfigured tag must pass through");
    CHECK(LuoHold_7C9F3B(P(0x20)) == 1, "unreadable stats must pass through");
    // 3. Sentinel added -> pass-through + UNHOLD logged
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "held again");
    clearLog();
    CHECK(LuoHold_74F3C7(P(sentinel.stats)) == 1, "atton with Sentinel must pass through");
    CHECK(logHas("UNHOLD tag=atton"), "UNHOLD log line missing:\n%s", readLog().c_str());
    // 4. mode=log -> r + WOULD HOLD
    snprintf(ini, sizeof ini, "mode=log\npreset.atton=%s\nhold.atton=1", PRESET);
    setIni(ini, table); clearLog();
    CHECK(LuoHold_66C95C(P(heldAtton.stats)) == 1, "log mode must not veto");
    CHECK(logHas("WOULD HOLD tag=Atton key=8"), "WOULD HOLD missing:\n%s", readLog().c_str());
    // 5. hold=0 and legacy tags= -> pass-through
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=0", PRESET);
    setIni(ini, table);
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "hold=0 must pass through");
    setIni("mode=rewrite\ntags=atton", table);
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "legacy tags= (hold 0) must pass through");
    CHECK(tagConfig("ATTON") && tagConfig("ATTON")->preset >= 0 && !strcmp(g_presets[tagConfig("atton")->preset].id, "atton__live") && !tagConfig("atton")->instant && !tagConfig("atton")->hold, "legacy tag config");
    // 6. mode=off
    snprintf(ini, sizeof ini, "mode=off\npreset.atton=%s", PRESET);
    setIni(ini, table);
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "mode=off must pass through");
    // reload throttle: a config change inside 1000 ms is not seen by the gates
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s", PRESET);
    setIni(ini, table);
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "rewrite after reload");
    { FILE* f = fopen("levelup_override.ini", "w"); fprintf(f, "mode=off\ntable=%s\n", table); fclose(f); }
    g_stubTick += 10;
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "config change inside the throttle window must not be seen");
    g_stubTick += 2000;
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "config change after the throttle window must be seen");

    // 7. instant gate
    uint8_t opts[16], optsBit[16];
    for (int i = 0; i < 16; ++i) opts[i] = optsBit[i] = (uint8_t)(0x40 + i);
    opts[8] = 0x40; optsBit[8] = 0x41;
    uintptr_t optA = arenaAlloc(16), optB = arenaAlloc(16);
    memcpy(P(optA), opts, 16); memcpy(P(optB), optsBit, 16);
    uintptr_t frame = arenaAlloc(0x200), ebp = frame + 0x100;
    struct { void* (__cdecl* fn)(void*, void*); int off; } sites[4] = {{LuoInstant_74F3E9, -0x38}, {LuoInstant_5FD7A0, -0x14}, {LuoInstant_5FE281, -0x18}, {LuoInstant_66C93A, -8}};
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=1", PRESET);
    setIni(ini, table); clearLog();
    for (auto& s : sites) {
        memset(P(frame), 0, 0x200);
        *reinterpret_cast<uint32_t*>(ebp + s.off) = (uint32_t)onPath.creature;
        g_optPtr = P(optA);
        void* r = s.fn(P(0x1234), P(ebp));
        CHECK(r != g_optPtr && r, "instant: non-held must return a copy (off %d)", s.off);
        if (r != g_optPtr && r) {
            const uint8_t* rb = static_cast<const uint8_t*>(r);
            CHECK((rb[8] & 1) == 1, "instant bit not set");
            bool same = true; for (int i = 0; i < 16; ++i) if (i != 8 && rb[i] != opts[i]) same = false;
            CHECK(same && (rb[8] & ~1) == (opts[8] & ~1), "instant: other bytes changed");
            CHECK(!(static_cast<const uint8_t*>(g_optPtr)[8] & 1), "instant: original struct was modified");
        }
        g_optPtr = P(optB);
        CHECK(s.fn(P(0x1234), P(ebp)) == g_optPtr, "instant: real bit already set -> original pointer");
        *reinterpret_cast<uint32_t*>(ebp + s.off) = (uint32_t)heldAtton.creature;
        g_optPtr = P(optA);
        CHECK(s.fn(P(0x1234), P(ebp)) == g_optPtr, "instant: held -> original pointer");
        *reinterpret_cast<uint32_t*>(ebp + s.off) = (uint32_t)pc.creature;
        CHECK(s.fn(P(0x1234), P(ebp)) == g_optPtr, "instant: PC -> original pointer");
        *reinterpret_cast<uint32_t*>(ebp + s.off) = 0x20;
        CHECK(s.fn(P(0x1234), P(ebp)) == g_optPtr, "instant: unreadable creature -> original pointer");
    }
    { std::string l = readLog(); size_t n = 0, p = 0; while ((p = l.find("INSTANT tag=atton key=6", p)) != std::string::npos) { ++n; ++p; } CHECK(n == 1, "INSTANT logged %zu times (want once per tag/key)", n); }
    snprintf(ini, sizeof ini, "mode=log\npreset.atton=%s\ninstant.atton=1", PRESET);
    setIni(ini, table);
    memset(P(frame), 0, 0x200); *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)onPath.creature; g_optPtr = P(optA);
    CHECK(LuoInstant_74F3E9(P(1), P(ebp)) == g_optPtr, "instant: log mode -> original pointer");
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\ninstant.atton=0", PRESET);
    setIni(ini, table);
    CHECK(LuoInstant_74F3E9(P(1), P(ebp)) == g_optPtr, "instant=0 -> original pointer");

    // 7b. leader instant 0x74F423: dummy controlled object only for a configured instant, not-held creature in rewrite mode
    {
        void* const real = g_ctlObj;
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=1", PRESET);
        setIni(ini, table); clearLog();
        memset(P(frame), 0, 0x200);
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)onPath.creature;
        void* r = LuoLeaderInstant_74F423(P(0x5678), P(ebp));
        CHECK(r && r != real && static_cast<const uint32_t*>(r)[1] == 0x7F000000u && g_ctlEcx == P(0x5678), "leader instant: dummy object with a non-matching id, real call made with ecx");
        LuoLeaderInstant_74F423(P(0x5678), P(ebp));
        { std::string l = readLog(); size_t n = 0, q = 0; while ((q = l.find("INSTANT-LEADER tag=atton key=6", q)) != std::string::npos) { ++n; ++q; } CHECK(n == 1, "INSTANT-LEADER logged %zu times (want once)", n); }
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)heldAtton.creature;
        CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: held -> real object");
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)pc.creature;
        CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: PC -> real object");
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = 0x20;
        CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: unreadable creature -> real object");
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)onPath.creature;
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\ninstant.atton=0", PRESET);
        setIni(ini, table);
        CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: instant=0 -> real object");
        snprintf(ini, sizeof ini, "mode=log\npreset.atton=%s\ninstant.atton=1", PRESET);
        setIni(ini, table);
        CHECK(LuoLeaderInstant_74F423(P(1), P(ebp)) == real, "leader instant: log mode -> real object");
    }

    // 8. GUI guards + Auto OK backstop
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1", PRESET);
    setIni(ini, table); clearLog();
    g_ccRet = 0; g_creatureRet = P(heldAtton.creature);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 1, "gui guard: held leader -> 1");
    CHECK(g_lastTableEcx == P(0x10001234), "table-get ECX must be [[appGlobal]+4], got %p", g_lastTableEcx);
    CHECK(LuoGuardAuto_84F9FE(P(7)) == 1, "auto guard: held leader -> 1");
    CHECK(logHas("GUARD gui tag=Atton") && logHas("GUARD auto tag=Atton") && logHas("GUARD-PROBE gui") && logHas("tagCreature=Atton"), "guard log lines:\n%s", readLog().c_str());
    g_creatureRet = P(sentinel.creature);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 0, "gui guard: unheld leader -> 0");
    g_creatureRet = P(pc.creature);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 0, "gui guard: PC leader -> 0");
    g_creatureRet = nullptr;
    CHECK(LuoGuardGui_84FD2F(P(7)) == 0, "gui guard: null leader -> 0");
    g_creatureRet = P(0x20);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 0, "gui guard: unreadable leader -> 0");
    g_creatureRet = P(heldAtton.creature); g_ccRet = 5;
    CHECK(LuoGuardGui_84FD2F(P(7)) == 5 && LuoGuardAuto_84F9FE(P(7)) == 5, "gui guard: real nonzero passes through");
    g_ccRet = 0;
    CHECK(LuoAutoOk_850062(P(9)) == nullptr, "auto OK backstop: held -> 0");
    CHECK(logHas("GUARD autook tag=Atton"), "autook log missing");
    g_creatureRet = P(sentinel.creature);
    CHECK(LuoAutoOk_850062(P(9)) == g_creatureRet, "auto OK backstop: unheld -> creature");
    g_creatureRet = P(pc.creature);
    CHECK(LuoAutoOk_850062(P(9)) == g_creatureRet, "auto OK backstop: PC -> creature");
    g_creatureRet = nullptr;
    CHECK(LuoAutoOk_850062(P(9)) == nullptr, "auto OK backstop: real 0 stays 0");
    snprintf(ini, sizeof ini, "mode=log\npreset.atton=%s\nhold.atton=1", PRESET);
    setIni(ini, table);
    g_creatureRet = P(heldAtton.creature);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 0 && LuoAutoOk_850062(P(9)) == g_creatureRet, "guards in log mode pass through");


    // 9. M3 save binding (fake globals table, fake CExoString, fake 0x51C450)
    {
        void* tabB = P(arenaAlloc(64)); void* tabC = P(arenaAlloc(64));
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1", PRESET);
        remove("levelup_playthroughs.ini");
        setIni(ini, table); clearLog();
        g_canRet = 1;
        CHECK(ptGuid() == 0, "guid starts at 0");
        // save with no GUID writes nothing
        LuoGuidSave(tabB);
        CHECK(g_vars.empty() && logHas("GUID save none"), "save with g_guid=0 must write nothing:\n%s", readLog().c_str());
        CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "dev key applies without a GUID");
        // a pre-existing hand-written playthrough file: unknown lines and other sections must survive the rewrite
        { FILE* f = fopen("levelup_playthroughs.ini", "w"); fprintf(f, "; my notes\n[g11111111]\nkreia=kreia__live|instant=0|hold=1\n[misc]\nx=y\n"); fclose(f); }
        // ptApply with no live server (chain broken) -> refused
        *reinterpret_cast<uint32_t*>(app + 8) = 0;
        CHECK(!ptApply("hk47", PRESET, false, true) && ptGuid() == 0 && g_vars.empty(), "ptApply without a live table must fail cleanly");
        *reinterpret_cast<uint32_t*>(app + 8) = (uint32_t)srv;
        // ptApply creates the GUID, writes 32 booleans to the live table and the ini
        CHECK(ptApply("hk47", PRESET, true, false), "ptApply failed:\n%s", readLog().c_str());
        const uint32_t guid = ptGuid();
        int found = 0;
        CHECK(guid != 0 && fakeGuid(P(liveTbl), &found) == guid && found == 32, "live table bits %08x found %d vs guid %08x", fakeGuid(P(liveTbl), &found), found, guid);
        CHECK(g_addDup == 0 && g_setMissing == 0 && g_adds == 32, "adds %d dup %d setMissing %d", g_adds, g_addDup, g_setMissing);
        CHECK(g_ctors == g_dtors, "ctor/dtor balance %d/%d", g_ctors, g_dtors);
        char sec[24]; snprintf(sec, sizeof sec, "[g%08x]", (unsigned)guid);
        std::string iniText;
        { FILE* f = fopen("levelup_playthroughs.ini", "r"); char b[512]; while (f && fgets(b, sizeof b, f)) iniText += b; if (f) fclose(f); }
        CHECK(iniText.find(sec) != std::string::npos && iniText.find("hk47=") != std::string::npos && iniText.find("|instant=1|hold=0") != std::string::npos, "ini content:\n%s", iniText.c_str());
        CHECK(iniText.find("; my notes\n[g11111111]\nkreia=kreia__live|instant=0|hold=1\n[misc]\nx=y\n") == 0, "hand-written content preserved:\n%s", iniText.c_str());
        CHECK(access("levelup_playthroughs.ini.tmp", F_OK) != 0, "tmp file left behind");
        CHECK(logHas("GUID new g") && logHas("live-write ok=1") && logHas("PLAYTHROUGH write g") && logHas("tag=hk47 ok=1") && logHas("CONFIG source=playthrough g"), "ptApply log:\n%s", readLog().c_str());
        // playthrough section wins outright: the dev-only tag (atton) is unconfigured; the section's tag is configured
        CHECK(tagConfig("atton") == nullptr, "dev-only tag must be unconfigured while a playthrough section exists");
        CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "dev-only atton must pass through under a playthrough section");
        { const TagCfg* h = tagConfig("HK47"); CHECK(h && h->preset >= 0 && h->instant && !h->hold && !strcmp(g_presets[h->preset].id, PRESET), "hk47 playthrough entry (presets resolve against the table)"); }
        // adding atton to the playthrough re-enables the hold gate through the playthrough source
        CHECK(ptApply("atton", PRESET, false, true) && ptGuid() == guid, "second ptApply keeps the GUID");
        CHECK(g_adds == 32, "second ptApply must not re-add names (adds %d)", g_adds);
        CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "atton held through its playthrough entry");
        CHECK(ptRemove("atton") && tagConfig("atton") == nullptr && LuoHold_74F3C7(P(heldAtton.stats)) == 1, "ptRemove -> unconfigured although the dev key names atton");
        CHECK(!ptRemove("atton"), "ptRemove of an absent tag");
        { FILE* f = fopen("levelup_playthroughs.ini", "r"); iniText.clear(); char b[512]; while (f && fgets(b, sizeof b, f)) iniText += b; if (f) fclose(f); }
        CHECK(iniText.find(sec) != std::string::npos && iniText.find("atton=") == std::string::npos && iniText.find("hk47=") != std::string::npos, "ini after remove:\n%s", iniText.c_str());
        CHECK(ptRemove("hk47") && tagConfig("hk47") == nullptr && tagConfig("atton") == nullptr, "removing the last tag leaves the section: dev keys stay off");
        CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "empty playthrough section keeps the dev keys off");
        // save writes the same bits into another table
        LuoGuidSave(tabB);
        CHECK(fakeGuid(tabB, &found) == guid && found == 32, "save bits %08x found %d", fakeGuid(tabB, &found), found);
        { char want[64]; snprintf(want, sizeof want, "GUID save g%08x table=", (unsigned)guid); CHECK(logHas(want) && logHas(" ok=1"), "save log:\n%s", readLog().c_str()); }
        CHECK(g_ctors == g_dtors, "ctor/dtor balance after save %d/%d", g_ctors, g_dtors);
        // new server clears the cache, returns 0x51C450's EAX, passes ecx
        CHECK(LuoGuidNewServer(P(0x4321)) == 0x77 && g_serverInitEcx == P(0x4321), "new-server must call 0x51C450(ecx) and return its EAX");
        CHECK(ptGuid() == 0 && logHas("GUID new-server (cache cleared, was g") && logHas("CONFIG source=dev"), "new-server clears the GUID:\n%s", readLog().c_str());
        CHECK(tagConfig("atton") != nullptr && LuoHold_74F3C7(P(heldAtton.stats)) == 0, "dev keys apply again after new-server");
        // load restores the GUID from the saved table and switches to the playthrough section
        LuoGuidLoad(tabB, 1);
        CHECK(ptGuid() == guid, "load restored %08x want %08x", ptGuid(), guid);
        { char want[96]; snprintf(want, sizeof want, "GUID load g%08x found=32/32 result=1", (unsigned)guid); CHECK(logHas(want), "load log:\n%s", readLog().c_str()); }
        CHECK(tagConfig("atton") == nullptr && LuoHold_74F3C7(P(heldAtton.stats)) == 1, "after load the playthrough section applies");
        // load of a table without the names (old save / new game) -> GUID 0, dev keys
        LuoGuidLoad(tabC, 1);
        CHECK(ptGuid() == 0 && logHas("found=0/32") && tagConfig("atton") != nullptr, "load without a saved GUID -> 0 / dev");
        LuoGuidLoad(tabB, 1);
        CHECK(ptGuid() == guid, "reload guid");
        // mode=off still tracks the GUID
        setIni("mode=off", table);
        LuoGuidNewServer(P(1));
        CHECK(ptGuid() == 0, "mode=off: new-server clears");
        LuoGuidLoad(tabB, 1);
        CHECK(ptGuid() == guid, "mode=off: load still tracks the GUID");
        // engine mismatch: save binding disabled, logged once; handlers do not touch the table
        const size_t before = g_vars.size();
        uintptr_t junk = arenaAlloc(64);
        g_eng.checkCode = true; g_guidEngState = 0;
        g_eng.guidFind = g_eng.guidAdd = g_eng.guidSet = g_eng.guidGet = g_eng.strCtor = g_eng.strDtor = junk;
        clearLog();
        void* tabD = P(arenaAlloc(64));
        LuoGuidSave(tabD);
        LuoGuidLoad(tabD, 1);
        CHECK(logHas("GUID ENGINE mismatch") && g_vars.size() == before && ptGuid() == 0, "engine mismatch must disable the binding:\n%s", readLog().c_str());
        // matching bytes -> enabled again
        g_guidEngState = 0;
        auto put = [&](uintptr_t& fn, const uint8_t* c, size_t n) { fn = arenaAlloc(32); memcpy(P(fn), c, n); };
        put(g_eng.guidFind, kGuidFindCode, sizeof kGuidFindCode); put(g_eng.guidAdd, kGuidAddCode, sizeof kGuidAddCode);
        put(g_eng.guidSet, kGuidSetCode, sizeof kGuidSetCode); put(g_eng.guidGet, kGuidGetCode, sizeof kGuidGetCode);
        put(g_eng.strCtor, kStrCtorCode, sizeof kStrCtorCode); put(g_eng.strDtor, kStrDtorCode, sizeof kStrDtorCode);
        put(g_eng.serverInit, kServerInitCode, sizeof kServerInitCode);
        CHECK(guidEngineOk() && g_guidEngState == 1, "matching prologues must verify");
        g_eng.checkCode = false;
    }

    printf("gate_test checks %d failures %d (engine calls %d)\n", g_checks, g_fail, g_fakeCalls);
    return g_fail ? 1 : 0;
}
