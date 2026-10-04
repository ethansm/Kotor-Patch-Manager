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
// apply-now: fake CSWSCreatureStats::AutoLevelup(stats, notify): counts, raises class slot 0 by one level, and the (fake) real CanLevelUp then answers 0
static int g_autoCalls = 0, g_autoNotify = -1;
static void* g_autoEcx = nullptr;
static void fakeAutoLevelup(void* st, int notify) { ++g_autoCalls; g_autoEcx = st; g_autoNotify = notify; ++reinterpret_cast<uint8_t*>(st)[St_ClassLevel]; g_canRet = 0; }
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
    f.creature = arenaAlloc(0x1210);   // +0x1200 = PlayerCreated flag (apply-now)
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
    fprintf(f, "dev=1\n%s\ntable=%s\n", body, table);   // dev=1: these tests use the dev keys (1.0.0: ignored without it)
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
    g_eng.autoLevelup = reinterpret_cast<uintptr_t>(&fakeAutoLevelup);
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

    // 0. 1.0.0 banner: first hook call logs the module file's crc32 (zlib crc32 of "123456789" = cbf43926); no ini yet -> defaults mode=rewrite
    { FILE* mf = fopen("luo_module.bin", "wb"); fputs("123456789", mf); fclose(mf); }
    g_stubModulePath = "luo_module.bin";
    remove("levelup_override.ini");
    g_stubTick += 5000;
    g_canRet = 1;
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1, "missing ini: nothing configured -> pass-through");
    CHECK(logHas("loaded; first hook call (build apply=1 crc32=cbf43926) ==") && logHas("CONFIG missing (defaults) mode=rewrite apply=1 verbose=0 ") && g_cfg.mode == Mode::Rewrite && !g_cfg.verbose,
          "banner crc32 / missing-ini defaults (rewrite, verbose=0):\n%s", readLog().c_str());
    // 0b. dev keys and legacy tags= are ignored without dev=1 (any position in the file); with dev=1 they apply
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ntags=kreia\npause.atton=6\ntable=%s\n", PRESET, table);
    { FILE* f = fopen("levelup_override.ini", "w"); fputs(ini, f); fclose(f); g_stubTick += 5000; }
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 1 && g_cfg.nTags == 0 && !g_cfg.dev && tagConfig("atton") == nullptr && tagConfig("kreia") == nullptr, "dev keys without dev=1 are ignored");
    { FILE* f = fopen("levelup_override.ini", "w"); fprintf(f, "%sdev=1\n", ini); fclose(f); g_stubTick += 5000; }
    CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0 && g_cfg.dev && tagConfig("atton") != nullptr && tagConfig("kreia") != nullptr && logHas("DEV keys enabled (dev=1)"), "dev=1 (last line of the file) enables the dev keys");
    g_nHoldState = 0; clearLog();   // forget the HOLD state / banner lines of the checks above

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
    g_stubTick += 3000;   // F14: HOLD/UNHOLD lines are rate limited per tag (2 s); the UNHOLD below is a deliberate, later transition
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
        const uintptr_t ctlEcxMem = arenaAlloc(32);   // F4: the 0x740FC0 guarded read needs a readable ecx
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=1", PRESET);
        setIni(ini, table); clearLog();
        memset(P(frame), 0, 0x200);
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)onPath.creature;
        void* r = LuoLeaderInstant_74F423(P(ctlEcxMem), P(ebp));
        CHECK(r && r != real && static_cast<const uint32_t*>(r)[1] == 0x7F000000u && g_ctlEcx == P(ctlEcxMem), "leader instant: dummy object with a non-matching id, real call made with ecx");
        LuoLeaderInstant_74F423(P(ctlEcxMem), P(ebp));
        { std::string l = readLog(); size_t n = 0, q = 0; while ((q = l.find("INSTANT-LEADER tag=atton key=6", q)) != std::string::npos) { ++n; ++q; } CHECK(n == 1, "INSTANT-LEADER logged %zu times (want once)", n); }
        *reinterpret_cast<uint32_t*>(ebp - 0x38) = (uint32_t)heldAtton.creature;
        CHECK(LuoLeaderInstant_74F423(P(ctlEcxMem), P(ebp)) == real, "leader instant: held -> real object (readable ecx: only the HOLD rule may refuse)");
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
        // F4: 0x740FC0 (character change in progress) nonzero -> the real object, no INSTANT-LEADER; zero again -> dummy
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=1", PRESET);
        setIni(ini, table); clearLog();
        g_ccRet = 3;
        CHECK(LuoLeaderInstant_74F423(P(ctlEcxMem), P(ebp)) == real && !logHas("INSTANT-LEADER"), "leader instant: character change in progress -> real object, not logged");
        g_ccRet = 0;
        { void* r2 = LuoLeaderInstant_74F423(P(ctlEcxMem), P(ebp)); CHECK(r2 && r2 != real, "leader instant: no change in progress -> dummy again"); }
    }

    // 8. GUI guards + Auto OK backstop
    snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1", PRESET);
    setIni(ini, table); clearLog();
    g_ccRet = 0; g_creatureRet = P(heldAtton.creature);
    CHECK(LuoGuardGui_84FD2F(P(7)) == 1, "gui guard: held leader -> 1");
    CHECK(g_lastTableEcx == P(0x10001234), "table-get ECX must be [[appGlobal]+4], got %p", g_lastTableEcx);
    CHECK(LuoGuardAuto_84F9FE(P(7)) == 1, "auto guard: held leader -> 1");
    CHECK(logHas("GUARD gui tag=Atton") && logHas("GUARD auto tag=Atton"), "guard log lines:\n%s", readLog().c_str());
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

    // 8b. 0.3.1 pause.atton=6: Scoundrel 6 (key 7 > 6) is held although the preset has a record for it; Scoundrel 5 (key 6 == pause) is not
    {
        Fake paused = makeCreature("atton", {{2, 6}});
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\ninstant.atton=1\npause.atton=6", PRESET);
        setIni(ini, table); clearLog();
        CHECK(LuoHold_74F3C7(P(paused.stats)) == 0 && LuoHold_757808(P(paused.stats)) == 0, "pause: Scoundrel 6 must be refused at the hold gates");
        CHECK(LuoHold_74F3C7(P(onPath.stats)) == 1, "pause: key == pauseAt must pass");
        CHECK(LuoHold_74F3C7(P(sentinel.stats)) == 1, "pause: creature with the hold class passes (REBASE)");
        CHECK(logHas("CONFIG ") && logHas("atton:" ) && logHas("/h1/p6"), "pause: CONFIG line shows /p6:\n%s", readLog().c_str());
        CHECK(logHas("HOLD tag=atton key=7 site=0x74F3C7"), "pause: HOLD log line:\n%s", readLog().c_str());
        for (auto& st : sites) {
            memset(P(frame), 0, 0x200);
            *reinterpret_cast<uint32_t*>(ebp + st.off) = (uint32_t)paused.creature;
            g_optPtr = P(optA);
            CHECK(st.fn(P(0x1234), P(ebp)) == g_optPtr, "pause: held-by-pause creature gets no instant (off %d)", st.off);
            *reinterpret_cast<uint32_t*>(ebp + st.off) = (uint32_t)onPath.creature;
            CHECK(st.fn(P(0x1234), P(ebp)) != g_optPtr, "pause: creature at key == pauseAt still gets instant (off %d)", st.off);
        }
        g_ccRet = 0; g_creatureRet = P(paused.creature);
        CHECK(LuoGuardGui_84FD2F(P(7)) == 1 && LuoGuardAuto_84F9FE(P(7)) == 1 && LuoAutoOk_850062(P(9)) == nullptr, "pause: GUI guards + Auto OK backstop hold the paused leader");
        // pause 0 / hold off / garbage: not held
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\npause.atton=0", PRESET);
        setIni(ini, table);
        CHECK(LuoHold_74F3C7(P(paused.stats)) == 1, "pause=0: Scoundrel 6 passes (preset default)");
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=0\npause.atton=6", PRESET);
        setIni(ini, table);
        CHECK(LuoHold_74F3C7(P(paused.stats)) == 1, "hold=0: pause has no effect");
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\npause.atton=abc", PRESET);
        setIni(ini, table);
        CHECK(LuoHold_74F3C7(P(paused.stats)) == 1 && tagConfig("atton") && tagConfig("atton")->pause == 0, "garbage pause = 0");
        snprintf(ini, sizeof ini, "mode=rewrite\npreset.atton=%s\nhold.atton=1\npause.atton=-3", PRESET);
        setIni(ini, table);
        CHECK(tagConfig("atton") && tagConfig("atton")->pause == 0, "negative pause = 0");
    }


    // 8c. apply-now: after "Apply" with instant=1 the next menu-draw gate call levels the creature once (the HUD refresh does not run in the menu)
    {
        const unsigned menuSites[4] = {0x756DD4, 0x7575B5, 0x757808, 0x75798C};
        auto gate = [&](unsigned site, const Fake& c) -> int {
            switch (site) { case 0x756DD4: return LuoHold_756DD4(P(c.stats)); case 0x7575B5: return LuoHold_7575B5(P(c.stats));
                            case 0x757808: return LuoHold_757808(P(c.stats)); case 0x75798C: return LuoHold_75798C(P(c.stats)); default: return LuoHold_74F3C7(P(c.stats)); } };
        auto cfgIni = [&](const char* mode, int instant) { snprintf(ini, sizeof ini, "mode=%s\npreset.atton=%s\nhold.atton=1\ninstant.atton=%d", mode, PRESET, instant); setIni(ini, table); };
        g_ccRet = 0;
        // (a) instant + rewrite: one AutoLevelup(stats, 1), the arrow-site CanLevelUp then returns 0; logged; one-shot (f)
        for (unsigned site : menuSites) {
            Fake a = makeCreature("atton", {{2, 5}});
            cfgIni("rewrite", 1); clearLog(); g_canRet = 1; g_autoCalls = 0; g_autoEcx = nullptr; g_autoNotify = -1;
            CHECK(gate(site, a) == 1 && g_autoCalls == 0, "apply-now: no request -> arrow stays, no AutoLevelup (site %x)", site);
            requestApplyNow("Atton");   // tag compare is case-insensitive
            CHECK(gate(0x74F3C7, a) == 1 && g_autoCalls == 0, "apply-now: only the menu-draw sites consume the request (site %x)", site);
            CHECK(gate(site, a) == 0 && g_autoCalls == 1 && g_autoEcx == P(a.stats) && g_autoNotify == 1, "apply-now: levelled once, arrow cleared (site %x calls %d)", site, g_autoCalls);
            CHECK(logHas("INSTANT apply-now tag=atton lvl 5->6"), "apply-now log line (site %x):\n%s", site, readLog().c_str());
            g_canRet = 1;
            CHECK(gate(site, a) == 1 && g_autoCalls == 1, "apply-now: one-shot, the second gate call does nothing (site %x)", site);
        }
        // a gate call for another creature leaves the request pending; the configured one then consumes it; the window expires
        { Fake a = makeCreature("atton", {{2, 5}}); cfgIni("rewrite", 1); g_canRet = 1; g_autoCalls = 0;
          requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(other.stats)) == 1 && g_autoCalls == 0 && g_applyNow.pending, "apply-now: other creature's gate call keeps the request");
          CHECK(LuoHold_756DD4(P(a.stats)) == 0 && g_autoCalls == 1 && !g_applyNow.pending, "apply-now: configured creature consumes it");
          g_canRet = 1; Fake a2 = makeCreature("atton", {{2, 5}}); requestApplyNow("atton"); g_stubTick += ApplyNowWindowMs + 10;
          CHECK(LuoHold_756DD4(P(a2.stats)) == 1 && g_autoCalls == 1 && !g_applyNow.pending, "apply-now: an expired request is dropped, no AutoLevelup"); }
        // (b) instant=0 -> no AutoLevelup (request cleared, arrow stays by design)
        { Fake b = makeCreature("atton", {{2, 5}}); cfgIni("rewrite", 0); clearLog(); g_canRet = 1; g_autoCalls = 0; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(b.stats)) == 1 && g_autoCalls == 0 && !g_applyNow.pending, "apply-now: instant=0 -> no AutoLevelup, request cleared"); }
        // (c) held creature (Scoundrel 7 with the hold class missing) -> none; the hold gate still hides the arrow
        { cfgIni("rewrite", 1); clearLog(); g_canRet = 1; g_autoCalls = 0; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(heldAtton.stats)) == 0 && g_autoCalls == 0, "apply-now: held creature -> no AutoLevelup (calls %d)", g_autoCalls);
          g_applyNow.pending = false; }
        // (d) PlayerCreated / PC / real CanLevelUp 0 / character change -> none
        { Fake d = makeCreature("atton", {{2, 5}}); cfgIni("rewrite", 1); clearLog(); g_canRet = 1; g_autoCalls = 0;
          *reinterpret_cast<uint32_t*>(d.creature + Cr_PlayerCreated) = 1; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(d.stats)) == 1 && g_autoCalls == 0 && logHas("INSTANT apply-now skip tag=atton key=6 reason=player-created"), "apply-now: PlayerCreated -> none:\n%s", readLog().c_str());
          requestApplyNow("atton"); CHECK(LuoHold_756DD4(P(pc.stats)) == 1 && g_autoCalls == 0 && g_applyNow.pending, "apply-now: PC (empty tag) never matches"); g_applyNow.pending = false;
          Fake d2 = makeCreature("atton", {{2, 5}}); g_canRet = 0; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(d2.stats)) == 0 && g_autoCalls == 0, "apply-now: real CanLevelUp 0 -> none"); g_applyNow.pending = false;
          g_canRet = 1; g_ccRet = 3; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(d2.stats)) == 1 && g_autoCalls == 0 && logHas("reason=character-change"), "apply-now: character change in progress -> none");
          g_ccRet = 0; }
        // (e) log mode -> WOULD line only
        { Fake e = makeCreature("atton", {{2, 5}}); cfgIni("log", 1); clearLog(); g_canRet = 1; g_autoCalls = 0; requestApplyNow("atton");
          CHECK(LuoHold_756DD4(P(e.stats)) == 1 && g_autoCalls == 0 && logHas("INSTANT apply-now WOULD tag=atton key=6") && !logHas("apply-now tag="), "apply-now: log mode -> WOULD only:\n%s", readLog().c_str()); }
        g_canRet = 1;
    }

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
        CHECK(!ptApply("hk47", PRESET, false, true, 0) && ptGuid() == 0 && g_vars.empty(), "ptApply without a live table must fail cleanly");
        *reinterpret_cast<uint32_t*>(app + 8) = (uint32_t)srv;
        // ptApply creates the GUID, writes 32 booleans to the live table and the ini
        CHECK(ptApply("hk47", PRESET, true, false, 0), "ptApply failed:\n%s", readLog().c_str());
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
        CHECK(ptApply("atton", PRESET, false, true, 0) && ptGuid() == guid, "second ptApply keeps the GUID");
        CHECK(g_adds == 32, "second ptApply must not re-add names (adds %d)", g_adds);
        CHECK(LuoHold_74F3C7(P(heldAtton.stats)) == 0, "atton held through its playthrough entry");
        // 0.3.1: pause through the playthrough section (copied into g_ptCfg, serialised as |pause=6)
        { Fake paused2 = makeCreature("atton", {{2, 6}});
          CHECK(LuoHold_74F3C7(P(paused2.stats)) == 1, "playthrough pause 0: Scoundrel 6 passes");
          CHECK(ptApply("atton", PRESET, false, true, 6) && tagConfig("atton") && tagConfig("atton")->pause == 6, "ptApply with pause");
          CHECK(LuoHold_74F3C7(P(paused2.stats)) == 0, "playthrough pause 6: Scoundrel 6 refused");
          std::string t2; FILE* f2 = fopen("levelup_playthroughs.ini", "r"); char b2[512]; while (f2 && fgets(b2, sizeof b2, f2)) t2 += b2; if (f2) fclose(f2);
          CHECK(t2.find("atton=" + std::string(PRESET) + "|instant=0|hold=1|pause=6\n") != std::string::npos && t2.find("hk47=" + std::string(PRESET) + "|instant=1|hold=0\n") != std::string::npos, "ini has |pause=6 only for atton:\n%s", t2.c_str());
          CHECK(ptApply("atton", PRESET, false, true, 0) && tagConfig("atton")->pause == 0 && LuoHold_74F3C7(P(paused2.stats)) == 1, "pause back to 0"); }
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

    // ---- readKnown sanity (carry-over power read): {ptr,count,cap} must satisfy count <= cap < 4096 (and count <= 100); a Jedi class at class level >= 2 with no
    // known power at all means the array is not what we think -> pkOk=false (powers stay plain, no owed powers). Feat lists (empty here) stay readable.
    {
        auto setPow = [&](const Fake& c, int cls, uint32_t count, uint32_t cap, uint32_t first) {
            uint8_t* st = reinterpret_cast<uint8_t*>(c.stats);
            uintptr_t arr = arenaAlloc(4 * 128 + 16);
            for (uint32_t i = 0; i < count && i < 128; ++i) reinterpret_cast<uint32_t*>(arr)[i] = first + i;
            uint8_t* h = st + 0x90 + cls * 0x28;   // literal layout on purpose: {uint32* data, int count, int cap}
            *reinterpret_cast<uint32_t*>(h) = (uint32_t)arr; *reinterpret_cast<uint32_t*>(h + 4) = count; *reinterpret_cast<uint32_t*>(h + 8) = cap;
        };
        auto rk = [&](const Fake& c, int nCls, int* np) { static KnownSet ks; bool ok = true; const bool r = readKnown(c.stats, nCls, &ks, &ok); *np = ks.np; return r ? ok : !ok; };
        int np = 0;
        Fake sol = makeCreature("rk1", {{0, 4}}); setPow(sol, 0, 3, 3, 10);
        CHECK(rk(sol, 1, &np) && np == 3, "count == cap ok (np %d)", np);
        Fake solNone = makeCreature("rk2", {{0, 4}});
        CHECK(rk(solNone, 1, &np) && np == 0, "non-Jedi with 0 powers is fine (np %d)", np);
        Fake over = makeCreature("rk3", {{0, 4}}); setPow(over, 0, 3, 2, 10);
        CHECK(!rk(over, 1, &np) && np == 0, "count > cap rejected (np %d)", np);
        Fake bigCap = makeCreature("rk4", {{0, 4}}); setPow(bigCap, 0, 3, 4096, 10);
        CHECK(!rk(bigCap, 1, &np), "cap 4096 rejected");
        Fake okCap = makeCreature("rk5", {{0, 4}}); setPow(okCap, 0, 3, 4095, 10);
        CHECK(rk(okCap, 1, &np) && np == 3, "cap 4095 accepted (np %d)", np);
        Fake big = makeCreature("rk6", {{0, 4}}); setPow(big, 0, 101, 128, 10);
        CHECK(!rk(big, 1, &np), "count 101 rejected");
        Fake atCap = makeCreature("rk7", {{0, 4}}); setPow(atCap, 0, 100, 100, 10);
        CHECK(rk(atCap, 1, &np) && np == 100, "count 100 accepted (np %d)", np);
        Fake jNone = makeCreature("rk8", {{5, 2}});
        CHECK(!rk(jNone, 1, &np) && np == 0, "Jedi (5) level 2 with 0 powers -> not ok (np %d)", np);
        for (int jc : {3, 4, 11, 12, 13}) { Fake j = makeCreature("rkj", {{jc, 3}}); CHECK(!rk(j, 1, &np), "Jedi class %d level 3, 0 powers -> not ok", jc); }
        Fake jLvl1 = makeCreature("rk9", {{5, 1}});
        CHECK(rk(jLvl1, 1, &np) && np == 0, "Jedi level 1 with 0 powers is fine (np %d)", np);
        Fake jOk = makeCreature("rk10", {{5, 2}}); setPow(jOk, 0, 1, 1, 20);
        CHECK(rk(jOk, 1, &np) && np == 1, "Jedi level 2 with a power ok (np %d)", np);
        Fake multi = makeCreature("rk11", {{0, 3}, {5, 4}}); setPow(multi, 0, 2, 2, 30);
        CHECK(rk(multi, 2, &np) && np == 2, "total over all classes: Jedi entry empty but another class has powers -> ok (np %d)", np);
        Fake multiNone = makeCreature("rk12", {{0, 3}, {5, 4}});
        CHECK(!rk(multiNone, 2, &np), "two classes, Jedi level 4, no powers anywhere -> not ok");
        Fake nonJ = makeCreature("rk13", {{6, 9}});
        CHECK(rk(nonJ, 1, &np) && np == 0, "droid class level 9 with 0 powers is fine");
    }

    { // R10 dim (batch 2a): plain detours LuoR10Dim_90C36A / LuoR10Dim_909B56 on a fake panel; tags = the real runtime/r10_tags.ini next to `table`
        const uintptr_t frame = arenaAlloc(0x200), ebp = frame + 0x100;
        const uintptr_t panel = arenaAlloc(0x1C00), client = arenaAlloc(0x400);
        *reinterpret_cast<uint32_t*>(panel + Pn_Client) = (uint32_t)client;
        *reinterpret_cast<uint32_t*>(ebp + F2_Panel) = (uint32_t)panel;
        *reinterpret_cast<uint32_t*>(ebp + P2_Panel) = (uint32_t)panel;
        const Fake pc = makeCreature("", {{1, 4}}), comp = makeCreature("Atton", {{1, 4}});
        const uintptr_t fArr = arenaAlloc(2 * 16), pArr = arenaAlloc(4 * 16);
        // feats (profile melee/single/light): 1 general, 3/9 melee, 7 general, 2 untagged = kept; 18/20/26 ranged = dropped
        const uint16_t fBase[10] = {1, 18, 3, 20, 7, 2, 26, 9, 18, 1}, fWant[6] = {1, 3, 7, 2, 9, 1};
        auto setF = [&](const uint16_t* ids, int n, int picks) {
            memcpy(P(fArr), ids, 2 * n);
            *reinterpret_cast<uint32_t*>(panel + PnF_Vec) = (uint32_t)fArr;
            *reinterpret_cast<int32_t*>(panel + PnF_Vec + 4) = n;
            *reinterpret_cast<int32_t*>(panel + PnF_Vec + 8) = 16;
            *reinterpret_cast<uint16_t*>(panel + PnF_Picks) = (uint16_t)picks;
        };
        auto fCount = [&] { return *reinterpret_cast<int32_t*>(panel + PnF_Vec + 4); };
        auto fSame = [&](const uint16_t* want, int n) { return fCount() == n && !memcmp(P(fArr), want, 2 * n); };
        const char* on = "r10=1\nr10.profile=melee/single/light\nr10.feats=build_dim\nr10.powers=build_dim";
        g_creatureRet = P(pc.creature);

        setIni("r10=0\nr10.profile=melee/single/light\nr10.feats=build_dim", table); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && !logHas("R10 dim"), "r10=0: vector untouched, no R10 line:\n%s", readLog().c_str());

        setIni(on, table); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fWant, 6), "r10=1 PC feats: compacted to 6 in order (count %d)", fCount());
        CHECK(reinterpret_cast<uint16_t*>(fArr)[6] == 26 && reinterpret_cast<uint16_t*>(fArr)[9] == 1, "slots past the new count are not rewritten");
        CHECK(logHas("R10 tags loaded") && logHas("feats=82 powers=73"), "tag table loaded:\n%s", readLog().c_str());
        { char want[160]; snprintf(want, sizeof want, "R10 dim kind=feats panel=%08x mode=build_dim before=10 after=6 picks=1 fallback=0 verdict=applied", (unsigned)panel);
          CHECK(logHas(want), "dim line '%s' missing:\n%s", want, readLog().c_str()); }
        LuoR10Dim_90C36A(P(ebp));   // a rebuild that kept the compacted vector: nothing left to drop, nothing written
        CHECK(fSame(fWant, 6) && logHas("before=6 after=6 picks=1 fallback=0 verdict=nochange"), "second build idempotent:\n%s", readLog().c_str());

        g_creatureRet = P(comp.creature); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && logHas("reason=not_pc") && !logHas("verdict="), "companion (tagged): untouched:\n%s", readLog().c_str());
        g_creatureRet = nullptr; clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && logHas("reason=unreadable"), "no server creature: untouched:\n%s", readLog().c_str());
        g_creatureRet = P(pc.creature);

        const uint16_t ranged[2] = {18, 20};
        clearLog(); setF(ranged, 2, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(ranged, 2) && logHas("fallback=1 verdict=fallback"), "nothing fits with a pick left: vanilla:\n%s", readLog().c_str());
        clearLog(); setF(ranged, 2, 0);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fCount() == 0 && logHas("before=2 after=0 picks=0"), "nothing fits, no picks left: emptied (count %d)", fCount());

        setIni("r10=1\nr10.profile=melee/single/light\nr10.feats=all\nr10.powers=build_dim", table); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && !logHas("R10 dim"), "r10.feats=all: untouched:\n%s", readLog().c_str());
        setIni("r10=1\nr10.profile=bogus\nr10.feats=build_hide", table); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && logHas("verdict=noprofile"), "invalid profile: untouched:\n%s", readLog().c_str());
        setIni("mode=off\nr10=1\nr10.profile=melee/single/light\nr10.feats=build_dim", table); clearLog(); setF(fBase, 10, 1);
        LuoR10Dim_90C36A(P(ebp));
        CHECK(fSame(fBase, 10) && !logHas("R10 dim"), "mode=off: untouched:\n%s", readLog().c_str());

        // powers: dword ids; every power carries all roles, so a light build drops only the dark powers 7 and 9
        setIni(on, table); clearLog();
        const uint32_t pBase[6] = {4, 7, 10, 9, 0x10007, 12}, pWant[4] = {4, 10, 0x10007, 12};
        memcpy(P(pArr), pBase, sizeof pBase);
        *reinterpret_cast<uint32_t*>(panel + PnP_Vec) = (uint32_t)pArr;
        *reinterpret_cast<int32_t*>(panel + PnP_Vec + 4) = 6;
        *reinterpret_cast<uint8_t*>(panel + PnP_Picks) = 2;
        LuoR10Dim_909B56(P(ebp));
        CHECK(*reinterpret_cast<int32_t*>(panel + PnP_Vec + 4) == 4 && !memcmp(P(pArr), pWant, sizeof pWant), "powers: dark 7, 9 dropped (count %d)",
              *reinterpret_cast<int32_t*>(panel + PnP_Vec + 4));
        CHECK(logHas("R10 dim kind=powers") && logHas("before=6 after=4 picks=2 fallback=0 verdict=applied"), "powers dim line:\n%s", readLog().c_str());
        remove("levelup_override.ini");
    }

    printf("gate_test checks %d failures %d (engine calls %d)\n", g_checks, g_fail, g_fakeCalls);
    return g_fail ? 1 : 0;
}
