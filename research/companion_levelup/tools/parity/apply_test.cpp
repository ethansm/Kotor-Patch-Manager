// Host test of applyRewrite (LevelUpOverride.cpp 0.2.0): builds fake engine pending structs in a below-4GB arena (engine pointers are
// 32-bit fields; MAP_32BIT keeps every address representable), swaps g_eng to fakes of AddFeat (dedup + push on the +0x18 array),
// SetSkillDelta (idx < 8) and the uint32 push (grow 16 then x2), then for every "rewrite" case from stdin (same line format as
// parity_main.cpp): apply, re-read with readPend, assert == computeRewrite out and untouched bytes unchanged; replay -> "already".
// Variants per case: arrays with cap == count (forces engine growth), cap 16, and empty {0,0,0} arrays. Also checks the abort path
// (count changed since read -> no write at all). Run via parity.py.
#include "../../../../patch_manager_mods/level_up_override_src/LevelUpOverride.cpp"
#include <sys/mman.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static uint8_t* g_arena = nullptr;
static size_t g_used = 0;
constexpr size_t ArenaSize = 256u << 20;
static uintptr_t arenaAlloc(size_t n) {
    n = (n + 15) & ~size_t(15);
    if (g_used + n > ArenaSize) { fprintf(stderr, "arena full\n"); exit(4); }
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena + g_used);
    memset(g_arena + g_used, 0xCD, n);
    g_used += n;
    return a;
}
static uint32_t* hdr(void* arr) { return static_cast<uint32_t*>(arr); }   // {ptr, count, cap}
static void grow(void* arr, int esz) {
    uint32_t* h = hdr(arr);
    if (h[1] != h[2]) return;
    uint32_t cap = h[2] ? h[2] * 2 : 16;
    uintptr_t n = arenaAlloc(size_t(cap) * esz);
    if (h[1]) memcpy(reinterpret_cast<void*>(n), reinterpret_cast<void*>(uintptr_t(h[0])), size_t(h[1]) * esz);
    h[0] = static_cast<uint32_t>(n); h[2] = cap;
}
static int g_engCalls = 0;
static void fakeAddFeat(void* pending, uint16_t feat) {
    ++g_engCalls;
    void* arr = static_cast<uint8_t*>(pending) + 0x18;
    uint32_t* h = hdr(arr);
    for (uint32_t i = 0; i < h[1]; ++i) if (reinterpret_cast<uint16_t*>(uintptr_t(h[0]))[i] == feat) return;
    grow(arr, 2);
    reinterpret_cast<uint16_t*>(uintptr_t(h[0]))[h[1]++] = feat;
}
static void fakeSetSkillDelta(void* pending, uint16_t idx, uint8_t v) {
    ++g_engCalls;
    if (idx >= NumSkills) return;
    reinterpret_cast<uint8_t*>(uintptr_t(*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pending) + 0x24)))[idx] = v;
}
static void fakePushU32(void* arr, uint32_t v) {
    ++g_engCalls;
    grow(arr, 4);
    uint32_t* h = hdr(arr);
    reinterpret_cast<uint32_t*>(uintptr_t(h[0]))[h[1]++] = v;
}

enum CapMode { CapExact, Cap16 };
static void fillArr(uintptr_t pending, int off, int esz, const void* src, int n, CapMode cm) {
    uint32_t* h = reinterpret_cast<uint32_t*>(pending + off);
    if (n == 0) { h[0] = h[1] = h[2] = 0; return; }
    uint32_t cap = cm == CapExact ? uint32_t(n) : (n > 16 ? uint32_t(n) : 16u);
    uintptr_t a = arenaAlloc(size_t(cap) * esz);
    for (int i = 0; i < n; ++i) {
        if (esz == 2) reinterpret_cast<uint16_t*>(a)[i] = static_cast<const uint16_t*>(src)[i];
        else reinterpret_cast<uint32_t*>(a)[i] = static_cast<const uint32_t*>(src)[i];
    }
    h[0] = static_cast<uint32_t>(a); h[1] = uint32_t(n); h[2] = cap;
}
static uintptr_t makePending(const Pend& p, CapMode cm) {
    uintptr_t pd = arenaAlloc(Pd_Size);
    memset(reinterpret_cast<void*>(pd), 0, Pd_Size);
    fillArr(pd, Pd_SpellsAdd, 4, p.spells, p.nSpells, cm);
    const uint32_t rem[2] = {0x55, 0x66};
    fillArr(pd, Pd_SpellsRem, 4, rem, 2, cm);
    fillArr(pd, Pd_Feats, 2, p.feats, p.nFeats, cm);
    uintptr_t d = arenaAlloc(NumSkills);
    for (int i = 0; i < NumSkills; ++i) reinterpret_cast<uint8_t*>(d)[i] = static_cast<uint8_t>(p.deltas[i]);
    uint8_t* b = reinterpret_cast<uint8_t*>(pd);
    *reinterpret_cast<uint32_t*>(b + Pd_Deltas) = static_cast<uint32_t>(d);
    *reinterpret_cast<uint16_t*>(b + Pd_Unspent) = static_cast<uint16_t>(p.unspent);
    b[Pd_Ability] = static_cast<uint8_t>(p.ability);
    b[Pd_HP] = 7; b[Pd_Class] = static_cast<uint8_t>(p.cls); b[Pd_Force] = 3; b[0x2E] = 0xAA; b[0x2F] = 0xBB;
    return pd;
}

static void ids(const std::string& s, std::vector<uint32_t>& out) { std::stringstream ss(s); std::string t; while (std::getline(ss, t, ',')) if (!t.empty()) out.push_back(std::stoul(t)); }

static int g_fail = 0;
#define CHECK(c, ...) do { if (!(c)) { ++g_fail; fprintf(stderr, "FAIL %s:%d: ", tag, key); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); } } while (0)

int main(int argc, char** argv) {
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (m == MAP_FAILED) { perror("mmap MAP_32BIT"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    g_eng = {reinterpret_cast<uintptr_t>(&fakeAddFeat), reinterpret_cast<uintptr_t>(&fakeSetSkillDelta), reinterpret_cast<uintptr_t>(&fakePushU32), false};
    // 0.2.1 auto-only gate, on the live run 1 call chains (doc 08): Auto button passes AutoLevelup, manual GUI does not.
    {
        const char* tag = "gate"; int key = 0;
        const uintptr_t autoFr[4] = {0x006C00AF, 0x00850086, 0x0075BA5A, 0x0075CE6B}, manFr[4] = {0x00661F26, 0x0065FC91, 0x00531585, 0x0051C442};
        CHECK(viaAutoLevelup(0x006B9770, autoFr, 4), "auto-button chain not seen as AutoLevelup");
        CHECK(!viaAutoLevelup(0x006B9770, manFr, 4), "manual-gui chain seen as AutoLevelup");
        CHECK(!viaAutoLevelup(0x006B9770, autoFr, 0), "empty chain seen as AutoLevelup");
    }
    if (argc < 2 || !parseTable(argv[1])) { fprintf(stderr, "table: %s\n", g_tableErr); return 2; }
    g_tableOk = true;
    int applied = 0, replays = 0, aborts = 0, skipped = 0;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::stringstream ls(line); std::string head, f, s, d;
        std::getline(ls, head, '|'); std::getline(ls, f, '|'); std::getline(ls, s, '|'); std::getline(ls, d, '|');
        char tag[32]; int key, cls, ab, un;
        if (sscanf(head.c_str(), "%31s %d %d %d %d", tag, &key, &cls, &ab, &un) != 5) return 3;
        Pend base{}; base.cls = cls; base.ability = ab; base.unspent = un;
        std::vector<uint32_t> fv, sv, dv; ids(f, fv); ids(s, sv); ids(d, dv);
        for (uint32_t x : fv) base.feats[base.nFeats++] = (uint16_t)x;
        for (uint32_t x : sv) base.spells[base.nSpells++] = x;
        for (int i = 0; i < NumSkills; ++i) base.deltas[i] = (int)dv.at(i);
        Pend empty = base; empty.nFeats = 0; empty.nSpells = 0;
        const Rec* rec = lookupRecord(tag, key);
        for (int variant = 0; variant < 3; ++variant) {
            const Pend& in = variant == 2 ? empty : base;
            Result r = computeRewrite(rec, in);
            if (strcmp(r.status, "rewrite")) { ++skipped; continue; }
            uintptr_t pd = makePending(in, variant == 1 ? Cap16 : CapExact);
            PendDump before; readPend(pd, NumSkills, &before);
            CHECK(before.ok && before.featsOk && before.spellsOk && before.deltasOk && samePend(before.pd, in), "fake pending re-read != input");
            uint8_t raw0[Pd_Size]; memcpy(raw0, reinterpret_cast<void*>(pd), Pd_Size);
            g_engCalls = 0;
            CHECK(applyRewrite(pd, before.pd, r, NumSkills), "applyRewrite returned false");
            PendDump after; readPend(pd, NumSkills, &after);
            CHECK(after.ok && after.featsOk && after.spellsOk && after.deltasOk, "after unreadable");
            CHECK(samePend(after.pd, r.p), "after != computeRewrite out");
            CHECK(g_engCalls == NumSkills + (r.p.nFeats - r.nKeptFeats) + (r.p.nSpells - r.nKeptSpells), "engine call count %d", g_engCalls);
            const uint8_t* raw1 = reinterpret_cast<uint8_t*>(pd);
            CHECK(!memcmp(raw0 + Pd_SpellsRem, raw1 + Pd_SpellsRem, 12), "spells-remove header changed");
            CHECK(!memcmp(raw0 + Pd_Deltas, raw1 + Pd_Deltas, 4), "deltas pointer changed");
            CHECK(!memcmp(raw0 + Pd_HP, raw1 + Pd_HP, Pd_Size - Pd_HP), "hp/class/force/padding changed");
            CHECK(after.nRem == 2 && after.rem[0] == 0x55 && after.rem[1] == 0x66, "spells-remove contents changed");
            ++applied;
            Result again = computeRewrite(rec, after.pd);
            CHECK(!strcmp(again.status, "already"), "replay status %s", again.status);
            ++replays;
            // abort path: count no longer matches what was read -> nothing written
            uintptr_t pd2 = makePending(in, CapExact);
            PendDump in2; readPend(pd2, NumSkills, &in2);
            uint32_t* fc = reinterpret_cast<uint32_t*>(pd2 + Pd_Feats + 4);
            if (*fc) {
                --*fc;
                uint8_t snap[Pd_Size]; memcpy(snap, reinterpret_cast<void*>(pd2), Pd_Size);
                uintptr_t dp2 = *reinterpret_cast<uint32_t*>(pd2 + Pd_Deltas); uint8_t dsnap[NumSkills]; memcpy(dsnap, reinterpret_cast<void*>(dp2), NumSkills);
                g_engCalls = 0;
                CHECK(!applyRewrite(pd2, in2.pd, r, NumSkills), "abort path applied");
                CHECK(g_engCalls == 0 && !memcmp(snap, reinterpret_cast<void*>(pd2), Pd_Size) && !memcmp(dsnap, reinterpret_cast<void*>(dp2), NumSkills),
                      "abort path wrote");
                ++aborts;
            }
        }
    }
    printf("apply_test applied %d replays-already %d aborts %d skipped(non-rewrite) %d failures %d\n", applied, replays, aborts, skipped, g_fail);
    return g_fail ? 1 : 0;
}
