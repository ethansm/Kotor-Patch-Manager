// K1 Spawn Probe 0.1.0-probe (Steam Aspyr build 6A522E71...). Diagnostics only: no gameplay change, spawns nothing.
// Stage 1 (observe only) of the K1 -> K2 spawn work (doc 38, T23).
//   Hook (see hooks.toml) = CSWSObject-derived creature AddToArea (thiscall, ret 0x18; stack args area*, x, y, z, int, int).
//   ECX = creature. [esp+4] = first stack arg, delivered as a VALUE (KPM reads a bracketed source as the value at that slot):
//   the area pointer. The detour runs before the original bytes, so creature +0x94..+0x9c are the entry values
//   (AddToArea writes them itself later) and the area may not yet hold the creature.
// Log: k1-spawn-probe_log.txt in the game's working directory (appended; the banner line marks each process start).
// Every memory read goes through SafeRead (no faults, no unvalidated derefs). Other stack args are never read.
// Writes are capped at kMaxLogLines per process; further lines are counted in memory only (g_dropped).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "../_shared/SafeRead.h"
#include "SpawnLogic.h"

namespace {

using saferead::readAt;

constexpr const char* kVersion = "0.2.0-probe";
constexpr const char* kLogName = "k1-spawn-probe_log.txt";
constexpr int kMaxLogLines = 3000;
constexpr int kEventLines = 25;         // one dump event: header + 16 creature rows + 4 DUMP24 rows + 4 DUMPST rows
constexpr int kCreatureDwords = 0x100;  // creature object dump (dwords)
constexpr int kPtrDwords = 0x40;        // DUMP24 / DUMPST dumps (dwords)
constexpr int kAnyDumpMax = 3;          // DUMP-ANY: first distinct creature ids, any tag
constexpr const char* kNpcTag = "k1p_probe_npc";
constexpr const char* kIniName = ".\\k1_spawn_probe.ini";  // [spawn] enabled=1 (default on when the file or key is missing)
constexpr int kMaxSeenTags = 64;
constexpr unsigned kCreatureStructSize = 0x1220;
constexpr unsigned kPlaceholderId = 0x7f000000u;

// Engine functions of the CreateObject creature branch (Steam exe 7751F7D8..., doc 38 sec 13/14). Calling conventions per REA;
// the operator new cdecl is UNVERIFIED (MSVC operator new is always cdecl).
constexpr uintptr_t kAddrOperatorNew = 0x00919723;  // cdecl (size)
constexpr uintptr_t kAddrCreatureCtor = 0x00561F30;  // thiscall (p, id, 0), retn 8
constexpr uintptr_t kAddrResrefInit = 0x00710840;    // thiscall (CResRef*, const char*), retn 4
constexpr uintptr_t kAddrLoadTemplate = 0x00572A70;  // thiscall (p, CResRef*, 0), retn 8
constexpr uintptr_t kAddrSetOrient = 0x00543F10;     // thiscall (p, float vec3*), retn 4
constexpr uintptr_t kAddrAddToArea = 0x00568BD0;     // thiscall (p, area*, x, y, z, 1, 1), retn 0x18

using NewFn = void*(__cdecl*)(unsigned);
using CtorFn = void*(__thiscall*)(void*, unsigned, int);
using ResrefFn = void*(__thiscall*)(void*, const char*);
using LoadFn = int(__thiscall*)(void*, void*, int);
using OrientFn = void(__thiscall*)(void*, const float*);
using AddFn = void(__thiscall*)(void*, void*, float, float, float, int, int);
using DtorFn = void*(__thiscall*)(void*, int);

// Stage 2 roster (constant; Stage 3 moves it to a file). z = 0.0: every 204tel floor point in the GIT is z 0.0.
const k1spawn::RosterRow kRoster[] = {
    {"c_drdmouse01", "MouseDroid", "204tel", -39.0f, -63.1f, 0.0f, 0.0f},      // pet: vanilla mouse droid at WP_com_ith_01
    {"k1p_native_npc", "k1p_native_npc", "204tel", -21.1f, -31.1f, 0.0f, 0.0f},  // K1 NPC in the plant room
};

// SafeRead site ids (labels only; the failure ring stays disabled).
enum : unsigned {
    kSiteCreature = 1, kSiteArea, kSiteTagPtr, kSiteTagStr, kSiteResref, kSiteFloat, kSiteDump, kSiteStPtr,
};

// ---- state (all hooks run on the main thread) -------------------------------------------------------------
bool g_inited = false;       // banner written (first hook call)
int g_logLines = 0;          // lines written to the log by this process
long g_dropped = 0;          // lines refused by the cap (in memory only)
long g_seq = 0;              // ADDTOAREA calls since attach (counts calls after the cap too)
bool g_npcDumped = false;    // the k1p_probe_npc dump event has been taken (first match only)
int g_anyDumps = 0;          // DUMP-ANY events taken
unsigned g_anyIds[kAnyDumpMax];  // ids of the DUMP-ANY events
// Stage 2 spawn state (reset by the load hook).
bool g_enabled = true;               // [spawn] enabled in k1_spawn_probe.ini
k1spawn::SpawnState g_state;         // stable-tick gate for the armed area
char g_armedArea[17] = "";           // area resref the state was armed with (from the first AddToArea of this load)
void* g_area = nullptr;              // area pointer seen at AddToArea during this load (valid only for this load)
long g_addsSinceTick = 0;            // AddToArea calls since the last main-loop tick
bool g_spawning = false;             // re-entrancy guard: our own AddToArea call
char g_seenTags[kMaxSeenTags][33];   // tags observed at AddToArea since the last load (existence guard)
int g_seenCount = 0;
long g_loadSeq = 0;

// ---- logging ----------------------------------------------------------------------------------------------
// One appended line per call. Refused and counted once the cap is reached.
void logf(const char* fmt, ...) {
    if (g_logLines >= kMaxLogLines) { ++g_dropped; return; }
    FILE* f = fopen(kLogName, "a");
    if (!f) return;
    ++g_logLines;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

// Appends `dwords` dwords of the object at base as rows of 16: "<prefix> off=<hex>: v v v ...". ???????? = unreadable.
// The caller has reserved the lines against the cap.
void dumpBlock(const char* prefix, uintptr_t base, int dwords) {
    const int rows = (dwords + 15) / 16;
    FILE* f = fopen(kLogName, "a");
    if (!f) return;
    g_logLines += rows;
    for (int row = 0; row < rows; ++row) {
        fprintf(f, "%s off=%03x:", prefix, static_cast<unsigned>(row * 16 * 4));
        for (int i = row * 16; i < row * 16 + 16 && i < dwords; ++i) {
            unsigned v = 0;
            if (readAt(kSiteDump, base, i * 4, &v)) fprintf(f, " %08x", v);
            else fputs(" ????????", f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

// ---- formatting -------------------------------------------------------------------------------------------
// "??" when unreadable, else the value as %08x.
void fmtHex(char* out, size_t n, bool ok, unsigned v) {
    if (ok) snprintf(out, n, "%08x", v);
    else snprintf(out, n, "??");
}

void fmtDec(char* out, size_t n, bool ok, unsigned v) {
    if (ok) snprintf(out, n, "%u", v);
    else snprintf(out, n, "??");
}

void fmtFloat(char* out, size_t n, bool ok, float v) {
    if (ok) snprintf(out, n, "%.3f", v);
    else snprintf(out, n, "??");
}

// 16 bytes at p: "text" when the bytes up to the first NUL are printable, else hex:xx xx ... . "??" when unreadable.
// out needs 64 bytes.
void fmtRef16(char* out, size_t n, unsigned site, uintptr_t p) {
    unsigned char rf[16];
    if (p == 0 || !saferead::probeRead(site, reinterpret_cast<const void*>(p), sizeof rf)) {
        snprintf(out, n, "??");
        return;
    }
    memcpy(rf, reinterpret_cast<const void*>(p), sizeof rf);
    int len = 0;
    while (len < 16 && rf[len]) ++len;
    bool printable = len > 0;
    for (int i = 0; i < len && printable; ++i) {
        if (rf[i] < 0x20 || rf[i] > 0x7e) printable = false;
    }
    if (printable) {
        snprintf(out, n, "\"%.*s\"", len, reinterpret_cast<const char*>(rf));
        return;
    }
    static const char kHex[] = "0123456789abcdef";
    char* o = out;
    memcpy(o, "hex:", 4);
    o += 4;
    for (int i = 0; i < 16; ++i) {
        *o++ = kHex[rf[i] >> 4];
        *o++ = kHex[rf[i] & 0x0f];
        *o++ = (i < 15) ? ' ' : '\0';
    }
}

// Copies the NUL-terminated string at p into out (at most cap-1 bytes, each byte probed). Non-printable bytes become '.'.
// Returns false with out = "" when p is 0 or its first byte is unreadable. A string cut short by an unreadable byte is kept.
bool readStr(unsigned site, uintptr_t p, char* out, int cap) {
    out[0] = 0;
    if (p == 0 || cap < 2) return false;
    if (!saferead::probeRead(site, reinterpret_cast<const void*>(p), 1)) return false;
    int n = 0;
    while (n < cap - 1) {
        if (!saferead::probeRead(site, reinterpret_cast<const void*>(p + n), 1)) break;
        unsigned char ch = *reinterpret_cast<const unsigned char*>(p + n);
        if (ch == 0) break;
        out[n++] = (ch >= 0x20 && ch < 0x7f) ? static_cast<char>(ch) : '.';
    }
    out[n] = 0;
    return true;
}

// ASCII case-insensitive equality of two NUL-terminated strings.
bool ciEquals(const char* a, const char* b) {
    for (;; ++a, ++b) {
        int ca = static_cast<unsigned char>(*a);
        int cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}

// Fallback when the tag path (c+0x24 -> +0x18 -> char*) is wrong: scans `dwords` dwords at base for a pointer to the probe tag.
// Cheap first-byte filter, then a bounded string read. Returns the byte offset of the first hit in *offOut.
bool scanForTag(uintptr_t base, int dwords, unsigned* offOut) {
    for (int i = 0; i < dwords; ++i) {
        unsigned v = 0;
        if (!readAt(kSiteDump, base, i * 4, &v) || v < 0x10000u) continue;
        unsigned char b0 = 0;
        if (!readAt(kSiteTagStr, v, 0, &b0) || (b0 != 'k' && b0 != 'K')) continue;
        char s[33];
        if (readStr(kSiteTagStr, v, s, sizeof s) && ciEquals(s, kNpcTag)) { *offOut = static_cast<unsigned>(i * 4); return true; }
    }
    return false;
}

// ---- dump event (creature object, *(c+0x24) as DUMP24, *(c+0x1198) as DUMPST) -------------------------------
// Reserved against the cap as one block (kEventLines). Unreadable targets are reported on one line and skipped.
void dumpEvent(bool any, long seq, uintptr_t c, bool idOk, unsigned id, const char* tag,
               bool tagPtrOk, unsigned tagPtr) {
    if (g_logLines + kEventLines > kMaxLogLines) { g_dropped += kEventLines; return; }
    const char* sfx = any ? "-ANY" : "";
    char idStr[16];
    fmtHex(idStr, sizeof idStr, idOk, id);
    logf("DUMP-HDR trigger=%s n=%ld c=%08lx id=%s tag=\"%s\"", any ? "ANY" : "NPC", seq,
         static_cast<unsigned long>(c), idStr, tag);

    char pfx[64];
    unsigned first = 0;
    snprintf(pfx, sizeof pfx, "DUMP%s c=%08lx", sfx, static_cast<unsigned long>(c));
    if (readAt(kSiteDump, c, 0, &first)) dumpBlock(pfx, c, kCreatureDwords);
    else logf("%s skipped (unreadable)", pfx);

    char p24[32];
    fmtHex(p24, sizeof p24, tagPtrOk, tagPtr);
    snprintf(pfx, sizeof pfx, "DUMP24%s p=%s", sfx, p24);
    if (tagPtrOk && tagPtr && saferead::probeRead(kSiteDump, reinterpret_cast<const void*>(static_cast<uintptr_t>(tagPtr)), 4))
        dumpBlock(pfx, tagPtr, kPtrDwords);
    else logf("%s skipped (unreadable)", pfx);

    unsigned st = 0;
    const bool stOk = readAt(kSiteStPtr, c, 0x1198, &st);
    char pst[32];
    fmtHex(pst, sizeof pst, stOk, st);
    snprintf(pfx, sizeof pfx, "DUMPST%s p=%s", sfx, pst);
    if (stOk && st && saferead::probeRead(kSiteDump, reinterpret_cast<const void*>(static_cast<uintptr_t>(st)), 4))
        dumpBlock(pfx, st, kPtrDwords);
    else logf("%s skipped (unreadable)", pfx);
}

// ---- Stage 2: native spawn ------------------------------------------------------------------------------------
void ensureInit() {
    if (g_inited) return;
    g_inited = true;
    g_enabled = GetPrivateProfileIntA("spawn", "enabled", 1, kIniName) != 0;
    logf("k1-spawn-probe %s loaded (spawn %s)", kVersion, g_enabled ? "enabled" : "disabled by ini");
}

bool seenTag(const char* tag) {
    return k1spawn::ExistsByTag(g_seenCount, [](int i) { return static_cast<const char*>(g_seenTags[i]); }, tag);
}

void rememberTag(const char* tag) {
    if (!tag[0] || g_seenCount >= kMaxSeenTags || seenTag(tag)) return;
    strncpy(g_seenTags[g_seenCount], tag, 32);
    g_seenTags[g_seenCount][32] = 0;
    ++g_seenCount;
}

// The CreateObject creature branch, minus the optional walkmesh snap and appear message (doc 38 sec 14). Every step is logged
// BEFORE it runs, so a crash leaves the last step in the log. Returns true when the creature was added to the area.
bool nativeSpawn(const k1spawn::RosterRow& row, void* area) {
    logf("SPAWN begin resref=%s tag=%s area=%08lx at %.1f,%.1f,%.1f", row.resref, row.tag, static_cast<unsigned long>(reinterpret_cast<uintptr_t>(area)),
         row.x, row.y, row.z);
    logf("SPAWN step new");
    void* p = reinterpret_cast<NewFn>(kAddrOperatorNew)(kCreatureStructSize);
    if (!p) { logf("SPAWN fail new returned null"); return false; }
    logf("SPAWN step ctor p=%08lx", static_cast<unsigned long>(reinterpret_cast<uintptr_t>(p)));
    reinterpret_cast<CtorFn>(kAddrCreatureCtor)(p, kPlaceholderId, 0);
    alignas(8) char resref[32];
    memset(resref, 0, sizeof resref);
    logf("SPAWN step resref");
    reinterpret_cast<ResrefFn>(kAddrResrefInit)(resref, row.resref);
    logf("SPAWN step load");
    const int loaded = reinterpret_cast<LoadFn>(kAddrLoadTemplate)(p, resref, 0) & 0xFF;
    if (!loaded) {
        logf("SPAWN fail load template %s; destroying", row.resref);
        void** vtbl = *reinterpret_cast<void***>(p);
        reinterpret_cast<DtorFn>(vtbl[0])(p, 1);
        return false;
    }
    const k1spawn::Vec3 o = k1spawn::OrientationFromFacing(row.facing);
    const float vec[3] = {o.x, o.y, o.z};
    logf("SPAWN step orient");
    reinterpret_cast<OrientFn>(kAddrSetOrient)(p, vec);
    logf("SPAWN step add");
    g_spawning = true;
    reinterpret_cast<AddFn>(kAddrAddToArea)(p, area, row.x, row.y, row.z, 1, 1);
    g_spawning = false;
    unsigned id = 0;
    const bool idOk = readAt(kSiteCreature, reinterpret_cast<uintptr_t>(p), 4, &id);
    logf("SPAWN ok resref=%s id=%s%08x", row.resref, idOk ? "" : "?", id);
    return true;
}

// Called from the main-loop hook once the armed area has been quiet long enough.
void spawnRoster(const char* areaResref) {
    for (const k1spawn::RosterRow& row : kRoster) {
        if (!k1spawn::ResrefEquals(row.area, areaResref)) continue;
        if (seenTag(row.tag)) { logf("SPAWN skip resref=%s tag=%s already present (seen at AddToArea)", row.resref, row.tag); continue; }
        nativeSpawn(row, g_area);
    }
}

}  // namespace

extern "C" {

// AddToArea entry hook (thiscall: ECX = creature; [esp+4] = area pointer value). One line per call; dumps on triggers.
void __cdecl K1SpawnOnAddToArea(void* creature, void* areaArg) {
    saferead::beginScope();
    ensureInit();
    if (g_spawning) { logf("ADDTOAREA reentry from native spawn (ignored)"); return; }
    ++g_seq;
    ++g_addsSinceTick;
    const uintptr_t c = reinterpret_cast<uintptr_t>(creature);
    const uintptr_t a = reinterpret_cast<uintptr_t>(areaArg);

    unsigned id = 0, areaId = 0, f11ac = 0, f11fc = 0, tagPtr = 0, area2 = 0;
    const bool idOk = readAt(kSiteCreature, c, 4, &id);
    unsigned char type = 0;
    const bool typeOk = readAt(kSiteCreature, c, 8, &type);
    const bool area2Ok = readAt(kSiteArea, a, 0, &area2);
    const bool areaIdOk = readAt(kSiteCreature, c, 0x90, &areaId);
    float px = 0.0f, py = 0.0f, pz = 0.0f;
    const bool pxOk = readAt(kSiteFloat, c, 0x94, &px);
    const bool pyOk = readAt(kSiteFloat, c, 0x98, &py);
    const bool pzOk = readAt(kSiteFloat, c, 0x9c, &pz);
    const bool f11acOk = readAt(kSiteCreature, c, 0x11ac, &f11ac);
    const bool f11fcOk = readAt(kSiteCreature, c, 0x11fc, &f11fc);
    const bool tagPtrOk = readAt(kSiteTagPtr, c, 0x24, &tagPtr);

    // tagstr: the char* at ([c+0x24]+0x18), read as up to 32 chars. Empty when that is not a readable char*.
    char tagStr[33];
    tagStr[0] = 0;
    unsigned charPtr = 0;
    if (tagPtrOk && readAt(kSiteTagPtr, tagPtr, 0x18, &charPtr)) readStr(kSiteTagStr, charPtr, tagStr, sizeof tagStr);

    char sId[16], sType[16], sArea2[16], sAreaId[16], sX[16], sY[16], sZ[16], sF1[16], sF2[16], sTagPtr[16];
    char ar1[64], ar2[64];
    fmtHex(sId, sizeof sId, idOk, id);
    fmtDec(sType, sizeof sType, typeOk, type);
    fmtHex(sArea2, sizeof sArea2, area2Ok, area2);
    fmtHex(sAreaId, sizeof sAreaId, areaIdOk, areaId);
    fmtFloat(sX, sizeof sX, pxOk, px);
    fmtFloat(sY, sizeof sY, pyOk, py);
    fmtFloat(sZ, sizeof sZ, pzOk, pz);
    fmtHex(sF1, sizeof sF1, f11acOk, f11ac);
    fmtHex(sF2, sizeof sF2, f11fcOk, f11fc);
    fmtHex(sTagPtr, sizeof sTagPtr, tagPtrOk, tagPtr);
    fmtRef16(ar1, sizeof ar1, kSiteResref, a ? a + 0x110 : 0);
    fmtRef16(ar2, sizeof ar2, kSiteResref, (area2Ok && area2) ? static_cast<uintptr_t>(area2) + 0x110 : 0);

    logf("ADDTOAREA n=%ld c=%08lx id=%s type=%s area=%08lx area2=%s areaId=%s pos=%s,%s,%s f11ac=%s f11fc=%s "
         "tagptr=%s tagstr=\"%s\" ar1=%s ar2=%s",
         g_seq, static_cast<unsigned long>(c), sId, sType, static_cast<unsigned long>(a), sArea2, sAreaId,
         sX, sY, sZ, sF1, sF2, sTagPtr, tagStr, ar1, ar2);

    // Stage 2 tracking: arm the gate on the first add of an area, remember the area pointer and every tag seen.
    char areaRes[17];
    areaRes[0] = 0;
    if (a != 0) {
        unsigned char rb[16];
        if (saferead::probeRead(kSiteResref, reinterpret_cast<const void*>(a + 0x110), sizeof rb)) {
            memcpy(rb, reinterpret_cast<const void*>(a + 0x110), sizeof rb);
            int n = 0;
            while (n < 16 && rb[n] >= 0x20 && rb[n] < 0x7f) { areaRes[n] = static_cast<char>(rb[n]); ++n; }
            areaRes[n] = 0;
        }
    }
    if (areaRes[0]) {
        g_area = areaArg;
        if (!k1spawn::ResrefEquals(g_armedArea, areaRes)) {
            memcpy(g_armedArea, areaRes, sizeof g_armedArea);
            g_state.Arm(areaRes);
            logf("SPAWN armed area=%s", areaRes);
        }
    }
    if (tagStr[0]) rememberTag(tagStr);
    // Dump triggers. The k1p_probe_npc event fires once; DUMP-ANY fires once per distinct id, for the first kAnyDumpMax ids.
    bool npcHit = !g_npcDumped && tagStr[0] && ciEquals(tagStr, kNpcTag);
    if (!g_npcDumped && !npcHit) {
        unsigned off = 0;
        if (scanForTag(c, kCreatureDwords, &off)) { logf("TAGSCAN hit creature c=%08lx at +0x%x", static_cast<unsigned long>(c), off); npcHit = true; }
        else if (tagPtrOk && tagPtr && scanForTag(tagPtr, kPtrDwords, &off)) { logf("TAGSCAN hit [c+0x24]=%08x at +0x%x", tagPtr, off); npcHit = true; }
    }
    if (npcHit) {
        g_npcDumped = true;
        dumpEvent(false, g_seq, c, idOk, id, tagStr, tagPtrOk, tagPtr);
    }
    if (idOk && g_anyDumps < kAnyDumpMax) {
        bool fresh = true;
        for (int i = 0; i < g_anyDumps; ++i) {
            if (g_anyIds[i] == id) { fresh = false; break; }
        }
        if (fresh) {
            g_anyIds[g_anyDumps++] = id;
            dumpEvent(true, g_seq, c, idOk, id, tagStr, tagPtrOk, tagPtr);
        }
    }
}

// Hook LoadModuleInProgress entry (ECX = module): a new area load starts. Resets the Stage 2 per-load state.
void __cdecl K1SpawnOnLoad(void* module) {
    saferead::beginScope();
    ensureInit();
    ++g_loadSeq;
    g_state.Arm("");
    g_armedArea[0] = 0;
    g_area = nullptr;
    g_addsSinceTick = 0;
    g_seenCount = 0;
    g_spawning = false;
    logf("LOAD n=%ld module=%08lx", g_loadSeq, static_cast<unsigned long>(reinterpret_cast<uintptr_t>(module)));
}

// MainLoop entry hook (ECX = server internal), every tick. Spawns the roster once per load after the armed area has
// seen kStableTicks ticks without an AddToArea call.
void __cdecl K1SpawnOnTick(void* server) {
    (void)server;
    if (!g_inited || !g_enabled || g_spawning) return;
    g_state.Tick(g_addsSinceTick > 0 ? 1 : 0);
    g_addsSinceTick = 0;
    if (!g_area || !g_armedArea[0] || !g_state.ShouldSpawn(g_armedArea)) return;
    saferead::beginScope();
    g_state.MarkSpawned();
    logf("SPAWN stable area=%s after %d quiet ticks (load %ld, %d tags seen)", g_armedArea, g_state.StableTicks(), g_loadSeq, g_seenCount);
    spawnRoster(g_armedArea);
}

// No-op export: lets KPM and the loader confirm the DLL is present.
__declspec(dllexport) void K1Spawn_Init() {}

}  // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);
    return TRUE;
}
