// K1 Spawn Probe 0.3.0-probe (Steam Aspyr build 6A522E71...): a DLL-only client of the npc-world library (k2.npcworld@1, NpcWorld.h).
// Doc 38 stage 2 (native spawn of K1 content into K2) on top of the shared library. It owns NO hook: the three world detours
// (LoadModuleInProgress, creature AddToArea, MainLoop), the POP-STABLE latch, the seen-tag list and the native CreateObject sequence
// all live in npc-world, which must load first (manifest requires = ["npc-world"]; a DLL-only patch has no hook to retry connect() from).
// What this client keeps from 0.2.0: the roster, the forensic dumps (ADDTOAREA line with +0x11AC/+0x11FC, NPC dump, DUMP-ANY, TAGSCAN)
// and the log format, so the harvest keys do not change. New fields are appended at the end of the ADDTOAREA line.
// Log: k1-spawn-probe_log.txt in the game's working directory (appended; the banner line marks each process start).
// Every memory read goes through SafeRead (no faults, no unvalidated derefs). Creature and area pointers from an event are valid only
// inside the callback, so all reads happen there. Lines are budgeted by k1spawn::LogBudget (ClientCore.h): 3000 per process, 2500 of them
// ADDTOAREA. The first logf/ADDTOAREA line refused at the total cap writes one "CAP reached" line (SPAWN and dump refusals do not); further lines are counted in memory only.
// Switches: npc_world.ini [world] enabled=0 silences ALL events, including this client's observation (unlike 0.2.0).
// Client ini ".\k1_spawn_probe.ini" [spawn] enabled=0 only stops this client's own spawns (observation stays on).
// The provider's 5 ms WARN on the spawning STABLE callback is expected: the spawns run inside that callback.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "kpatch_api.h"
#include "SafeRead.h"
#include "NpcWorld.h"
#include "ClientCore.h"

namespace {

using saferead::readAt;

constexpr const char* kVersion = "0.3.0-probe";
constexpr const char* kLogName = "k1-spawn-probe_log.txt";
constexpr int kEventLines = 25;         // one dump event: header + 16 creature rows + 4 DUMP24 rows + 4 DUMPST rows
constexpr int kCreatureDwords = 0x100;  // creature object dump (dwords)
constexpr int kPtrDwords = 0x40;        // DUMP24 / DUMPST dumps (dwords)
constexpr const char* kIniName = ".\\k1_spawn_probe.ini";  // [spawn] enabled=1 (default on when the file or key is missing)

// SafeRead site ids (labels only; the failure ring stays disabled).
enum : unsigned {
    kSiteCreature = 1, kSiteArea, kSiteTagPtr, kSiteTagStr, kSiteResref, kSiteFloat, kSiteDump, kSiteStPtr,
};

// ---- state (all callbacks run on the engine's main thread) ------------------------------------------------------
k2nw::Client g_nw;
bool g_enabled = true;       // [spawn] enabled in k1_spawn_probe.ini
long g_seq = 0;              // ADDTOAREA events since attach (counts events after the cap too)
long g_loadSeq = 0;
char g_armedArea[17] = "";   // area resref "SPAWN armed" was last logged for (reset by AREA_LOAD)
FILE* g_event = nullptr;     // log file kept open for the duration of one event callback (callbacks over 5 ms are WARNed)
k1spawn::LogBudget g_budget;   // every line and dump row is charged here (3000 total, 2500 ADDTOAREA)
k1spawn::EventScope g_scope;   // nesting of onEvent callbacks: the outermost one owns g_event
k1spawn::DumpGate g_gate;      // NPC dump once; DUMP-ANY for the first distinct creature ids
k1spawn::ScanBudget g_scan;    // fallback TAGSCAN, at most 64 per process

// ---- logging ----------------------------------------------------------------------------------------------------
// Writes one line the budget has already charged. Uses the event's open handle; outside an event it opens and closes the log.
// A line is flushed at the end of every event (flushEvent) and when the log closes, so a crash keeps the last flushed line.
void writeLine(const char* text) {
    FILE* f = g_event ? g_event : fopen(kLogName, "a");
    if (!f) return;
    fputs(text, f);
    fputc('\n', f);
    if (!g_event) fclose(f);
}

void flushEvent() {
    if (g_event) fflush(g_event);
}

// Charges one line against the budget. A refused line is counted as dropped; the first refusal at the total cap writes the
// "CAP reached" marker (the marker is not charged). Returns true when the line may be written.
bool chargeLine(k1spawn::LogKind kind) {
    if (g_budget.reserve(1, kind)) return true;
    char marker[48];
    if (g_budget.takeCapMarker(g_seq, marker, sizeof marker)) writeLine(marker);
    return false;
}

// Formats one line, charges it, and writes it only when the budget allows.
void vlogKind(k1spawn::LogKind kind, const char* fmt, va_list ap) {
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    if (chargeLine(kind)) writeLine(buf);
}

void logf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlogKind(k1spawn::LogKind::Other, fmt, ap);
    va_end(ap);
}

void logfKind(k1spawn::LogKind kind, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlogKind(kind, fmt, ap);
    va_end(ap);
}

// The DLL side of the core's Env: writes lines the budget already charged, opens the log for one event, calls npc-world.
struct DllEnv : k1spawn::Env {
    void line(const char* text) override { writeLine(text); }
    void openLog() override { g_event = fopen(kLogName, "a"); }
    void closeLog() override {
        if (g_event) fclose(g_event);
        g_event = nullptr;
    }
    uint32_t spawn(const char* resref, const char* tag, float x, float y, float z, float facing) override {
        return g_nw.spawn(resref, tag, x, y, z, facing);
    }
    bool tagPresent(const char* tag) override { return g_nw.tagPresent(tag); }
    void flush() override { flushEvent(); }
};
DllEnv g_env;

// Appends `dwords` dwords of the object at base as rows of 16: "<prefix> off=<hex>: v v v ...". ???????? = unreadable.
// The rows are charged as one reservation; a refused block writes nothing.
void dumpBlock(const char* prefix, uintptr_t base, int dwords) {
    const int rows = (dwords + 15) / 16;
    if (!g_budget.reserve(rows, k1spawn::LogKind::Other)) return;
    FILE* f = g_event ? g_event : fopen(kLogName, "a");
    if (!f) return;
    for (int row = 0; row < rows; ++row) {
        fprintf(f, "%s off=%03x:", prefix, static_cast<unsigned>(row * 16 * 4));
        for (int i = row * 16; i < row * 16 + 16 && i < dwords; ++i) {
            unsigned v = 0;
            if (readAt(kSiteDump, base, i * 4, &v)) fprintf(f, " %08x", v);
            else fputs(" ????????", f);
        }
        fputc('\n', f);
    }
    if (!g_event) fclose(f);
}

// ---- formatting -------------------------------------------------------------------------------------------------
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

// Fallback when the tag path (c+0x24 -> +0x18 -> char*) is wrong: scans `dwords` dwords at base for a pointer to a harvest tag.
// Cheap first-byte filter, then a bounded string read. Returns the byte offset of the first hit in *offOut.
bool scanForTag(uintptr_t base, int dwords, unsigned* offOut) {
    for (int i = 0; i < dwords; ++i) {
        unsigned v = 0;
        if (!readAt(kSiteDump, base, i * 4, &v) || v < 0x10000u) continue;
        unsigned char b0 = 0;
        if (!readAt(kSiteTagStr, v, 0, &b0) || (b0 != 'k' && b0 != 'K' && b0 != 'm' && b0 != 'M')) continue;
        char s[33];
        if (readStr(kSiteTagStr, v, s, sizeof s) && k1spawn::IsHarvestTag(s)) { *offOut = static_cast<unsigned>(i * 4); return true; }
    }
    return false;
}

// ---- dump event (creature object, *(c+0x24) as DUMP24, *(c+0x1198) as DUMPST) -------------------------------------
// Needs kEventLines of room in the budget, or writes nothing and returns false so the caller's gate stays open.
// Unreadable targets are reported on one line and skipped.
bool dumpEvent(bool any, long seq, uintptr_t c, bool idOk, unsigned id, const char* tag,
               bool tagPtrOk, unsigned tagPtr) {
    if (!g_budget.fits(kEventLines)) return false;
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
    return true;
}

// ---- event handlers ---------------------------------------------------------------------------------------------
void onLoad(const K2NwEvent* ev) {
    g_armedArea[0] = 0;
    g_loadSeq = ev->load_seq;
    logf("LOAD n=%ld module=n/a salt=%08x", g_loadSeq, ev->visit_salt);  // the library does not pass the module pointer
}

// One creature is being added to an area (library H3, before the engine's own AddToArea code runs): the 0.2.0 ADDTOAREA line from the
// event fields, plus the creature-memory extras the library does not expose. Library spawns (origin LIB) are logged but never dumped.
void onCreatureAdded(const K2NwEvent* ev) {
    saferead::beginScope();
    ++g_seq;
    const uintptr_t c = reinterpret_cast<uintptr_t>(ev->creature);
    const uintptr_t a = reinterpret_cast<uintptr_t>(ev->area_ptr);
    const bool fromLib = K2NW_EVENT_HAS(ev, origin) && ev->origin == K2NW_ORIGIN_LIB;
    const char* libTag = (K2NW_EVENT_HAS(ev, tag) && ev->tag) ? ev->tag : "";
    const char* libArea = (K2NW_EVENT_HAS(ev, area) && ev->area) ? ev->area : "";

    // Total cap reached: no line, dump or scan can be written, so no memory is read. The refused line sends the CAP marker out once.
    if (g_budget.capped()) {
        chargeLine(k1spawn::LogKind::AddToArea);
        return;
    }

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

    // tagstr: the char* at ([c+0x24]+0x18), read as up to 32 chars. Empty when that is not a readable char*. The dump trigger needs it
    // even when the ADDTOAREA line is refused. The library's own tag read is logged as libtag so the two paths can be compared in the harvest.
    char tagStr[33];
    tagStr[0] = 0;
    unsigned charPtr = 0;
    if (tagPtrOk && readAt(kSiteTagPtr, tagPtr, 0x18, &charPtr)) readStr(kSiteTagStr, charPtr, tagStr, sizeof tagStr);

    if (g_budget.addToAreaAllowed()) {
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
        logfKind(k1spawn::LogKind::AddToArea,
                 "ADDTOAREA n=%ld c=%08lx id=%s type=%s area=%08lx area2=%s areaId=%s pos=%s,%s,%s f11ac=%s f11fc=%s "
                 "tagptr=%s tagstr=\"%s\" ar1=%s ar2=%s origin=%s oid=%08x libtag=\"%s\" libarea=\"%s\" libpos=%.3f,%.3f,%.3f",
                 g_seq, static_cast<unsigned long>(c), sId, sType, static_cast<unsigned long>(a), sArea2, sAreaId,
                 sX, sY, sZ, sF1, sF2, sTagPtr, tagStr, ar1, ar2, fromLib ? "LIB" : "ENGINE", ev->oid, libTag, libArea,
                 ev->x, ev->y, ev->z);
    } else {
        chargeLine(k1spawn::LogKind::AddToArea);  // ADDTOAREA sub-cap reached: refused and counted, nothing formatted
    }
    if (fromLib) return;

    // The first engine add of a load names the area (what 0.2.0 logged when it armed its own gate).
    if (libArea[0] && !k1spawn::ResrefEquals(g_armedArea, libArea)) {
        strncpy(g_armedArea, libArea, sizeof g_armedArea - 1);
        g_armedArea[sizeof g_armedArea - 1] = 0;
        logf("SPAWN armed area=%s", g_armedArea);
    }

    // Dump triggers. The NPC trigger is a harvest tag in either tag path. While the NPC dump is still wanted, the fallback scan runs
    // only when both tag strings came back empty (ScanBudget). DUMP-ANY fires once per distinct id, for the first three ids.
    const char* dumpTag = tagStr[0] ? tagStr : libTag;
    bool npcHit = k1spawn::IsHarvestTag(tagStr) || k1spawn::IsHarvestTag(libTag);
    if (!npcHit && g_gate.wantNpc(true) && g_scan.allow(libTag[0] == 0, tagStr[0] == 0, g_budget.capped())) {
        unsigned off = 0;
        if (scanForTag(c, kCreatureDwords, &off)) { logf("TAGSCAN hit creature c=%08lx at +0x%x", static_cast<unsigned long>(c), off); npcHit = true; }
        else if (tagPtrOk && tagPtr && scanForTag(tagPtr, kPtrDwords, &off)) { logf("TAGSCAN hit [c+0x24]=%08x at +0x%x", tagPtr, off); npcHit = true; }
    }
    if (g_gate.wantNpc(npcHit)) {
        if (dumpEvent(false, g_seq, c, idOk, id, dumpTag, tagPtrOk, tagPtr)) g_gate.commitNpc();
    }
    if (idOk && g_gate.wantAny(id)) {
        if (dumpEvent(true, g_seq, c, idOk, id, dumpTag, tagPtrOk, tagPtr)) g_gate.commitAny(id);
    }
}

// The library's POP-STABLE latch fired for this visit: RunStable (ClientCore.h) decides and spawns the roster rows of the current area
// through the library (one decision line per row). A refusal reason is in npc-world_log.txt (Disabled, NotStable, TagPresent, CapReached, EngineFailed ...).
void onStable(const K2NwEvent* ev) {
    saferead::beginScope();
    const char* area = (K2NW_EVENT_HAS(ev, area) && ev->area) ? ev->area : "";
    k1spawn::RunStable(g_env, area, ev->visit_seq, ev->visit_salt, g_enabled, g_budget);
}

void KPATCH_CALL onEvent(const K2NwEvent* ev, void*) {
    if (!ev) return;
    k1spawn::ScopedEvent scope(g_scope, g_env);  // opens the log for the outermost callback; closes it on every return path
    switch (ev->kind) {
        case K2NW_AREA_LOAD:      onLoad(ev); break;
        case K2NW_CREATURE_ADDED: onCreatureAdded(ev); break;
        case K2NW_AREA_STABLE:    onStable(ev); break;
        default: break;
    }
}

void onConnect(bool ok, const char* why) {
    if (g_nw.connected && !ok) logf("CONNECT partial %s", why ? why : "?");
    else logf("CONNECT %s %s", ok ? "ok" : "fail", why ? why : "?");
}

}  // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    g_enabled = GetPrivateProfileIntA("spawn", "enabled", 1, kIniName) != 0;
    logf("k1-spawn-probe %s loaded (client of npc-world, spawn %s)", kVersion, g_enabled ? "enabled" : "disabled by ini");
    g_nw.init(api, "k1-spawn-probe");
    g_nw.onConnect = onConnect;
    // add() must come before connect(): registration happens inside connect() and later add() calls never register.
    g_nw.add(K2NW_AREA_LOAD | K2NW_CREATURE_ADDED | K2NW_AREA_STABLE, onEvent);
    g_nw.connect();  // one shot; on failure the client stays inert (no callbacks, spawn() returns 0)
}

}  // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinst);
    return TRUE;
}
