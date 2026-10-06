// k2-robustness-fixes 1.3.0 (Steam Aspyr build 6A522E71...). Engine robustness fixes from the
// doc 10 Phase 6E research (batches W4-W8, Opus-verified; section "Robustness set").
//
// DLL-only. Each fix is an independent group: DllMain verifies every byte of the group's sites, then writes the group;
// a mismatch refuses that group only (logged), the other groups still apply. Caves live in one VirtualAlloc'd RWX
// block; every rel32 is computed from the FINAL site/cave address (lesson 242).
//
//   kt       KT: CExoKeyTable::DestroyTable 0x726350 / UpdateTable 0x726A50 keep CRes+0x14 pointing into the key
//            entry array they are about to free when the resource is still demanded ("still in demand" branch);
//            ReleaseResObject 0x713010 later writes [[CRes+0x14]+0x10]=0 into freed heap (0x71306C). Caves at the
//            flag stores 0x726527 / 0x726C9C (EAX = CRes) also set CRes+0x14 = &g_ktDummy, a zeroed 0x24-byte key
//            entry (NOT 0: GetResRef 0x711270 and the BIF service log path 0x713FB0 read +0x14 unguarded).
//            The in-place site 0x726505 is not used: perf-lab 0.3.0 hooks 0x72650B inside it.
//   vmid     VMID: NWScript command dispatch 0x668FD9 CMP [EBP+8],0x36D / JGE -> JAE at 0x668FE0, so sign-extended
//            ids 0x8000-0xFFFF (negative) take the existing reject path (-2002) instead of indexing before the table.
//            Counters (1.1.0): the reject tail 0x669012 (MOV EAX,-2002) is a cave that counts every reject
//            (vm_reject), counts negative ids separately (vm_neg, i.e. the ones the JAE change newly rejects) and
//            records the last negative id (vm_last_id). Both sites are verified before either is written.
//   sndri    SNDRI: Options > Sound apply re-initialises Miles via 0x7051C0; the teardown branch clears the driver
//            handle (+0xD4) and jumps past the AIL_quick_startup block, so the tail shuts Miles down and returns 0
//            (sound dead until a second apply). 0x7054CE: cave = lock inc g_sndri, CALL [AIL_quick_shutdown], then
//            fall into the startup block 0x7054DA (sndri_restart counts how often the bug path fired).
//   netovf   NETOVF: CNetLayer send 0x704060 only logs "Message Buffer Overflow" and still memcpys. B (0x7040EF)
//            turns the lap test into the true heap bound (end > 0x20000), A (0x704406) is a cave that makes that
//            branch drop the message (count net_drop, record the length [ebp+0x10] in net_last_len, destroy the
//            local string, return 0, epilogue 0x70446D). Lap case unchanged.
//   guiid    GUIID: CSWGuiPanel::InitControl 0x40F620 pads the control list up to the control ID; ids >= 4096 are
//            rejected like negative ones (vanilla max id 143 over 159 GUIs). 0x40F972 is a cave: negative ids go to
//            the engine's own reject (0x40F9E4) uncounted, ids >= 4096 are counted (gui_reject, gui_last_id in ECX
//            only) and rejected the same way, everything else resumes at 0x40F978.
//   abbound  AB: CExoArrayList<0x3C entry>::SetAllocated 0x58C540 copies the OLD count into the new buffer; a shrink
//            (reachable via 0x8427A0) overruns it. Cave at 0x58C61A bounds the copy by min(old count, new capacity).
//   1.2.0 (W8, engine_research_findings/W8.md):
//   fs       FS: CExoResMan::Malloc 0x712F30 is entered for a CRes whose failed sync Service left +0x10 != 0 with flags 4/0x10
//            clear; the new buffer overwrites the old pointer (leak). Cave at 0x712F55 counts that state (fs_stale,
//            fs_stale_bytes) and replays. Key fs=1 (DEFAULT) = count only; fs=2 also frees like FreeChunk 0x711FE0 (push
//            size, vtbl+0xC, call 0x7120C0, restore CRes+0x18 which 0x7120C0 zeroes and Malloc reads next). One cave, the
//            free is gated by the runtime byte g_fsFree.
//   ssb      SS-b: CaptureScreenToTGA 0x4289D0 dereferences a NULL "camera" object (0x428B31 JZ -> 0x428B44 arm). Cave at
//            0x428B2A: camera NULL -> count (ssb_camnull) and skip the render block (jmp 0x428B88).
//   ssc      SS-c: 0x428AE8 JLE (padded width <= viewport height) skips the only writers of the output dims [EBP-0xC]/[EBP-8].
//            Cave at 0x428AE5 counts that case (ssc_dims) and continues at 0x428AEA either way (= NOP-ing the JLE).
//   td       TD: C2DA::NextToken 0x71C180 copies a token into a 256-byte stack buffer with no bound (cookie directly above).
//            Two 37-byte in-place blocks (0x71C243 quoted loop, 0x71C2C9 bare loop) clamp the destination at 255 chars
//            (cmp [ebp-8],0xFF / adc [ebp+0x10],0); counter cave at the shared loop exit 0x71C2F0 counts td_long (len > 255).
//            All three sites are one group.
//   vmlk     VMLK: ActionDoCommand 0x6699D0 drops the popped script situation (code copy + stack, ~45 KB) when OBJECT_SELF
//            is missing (JNZ 0x669A30 -> 0x669A3E). Cave at 0x669A27 frees it with 0x7037C0 (situation delete: dtor 0x6FF6E0 +
//            operator delete, ret 4, ignores ECX) and
//            counts vmlk_free. The success exit jumps to 0x669A32 (perf-lab PerfLabAdcHandoff detour, adjacent, no overlap).
//            1.2.1 (W9 item 6.3): 1.2.0 called 0x6FDBB0, a thiscall wrapper that loads ECX=[ECX+0x1C] before calling 0x7037C0;
//            on this branch ECX = the byte at 0x99447F (0/1), so the first vmlk fire would have faulted. Never fired in the field.
//   movspin  F1: PlayMovies 0x798820 spins with no yield; cave at the pump branch 0x798A47 does one Sleep(1) via the IAT slot
//            0x9860D4 per message pump (once per 501 spins) and counts f1_sleeps. DEFAULT OFF (movspin=1 enables): may shift
//            the load-hang (H1) race, test in a dedicated soak.
//
// Config: $G/robustnessfixes.txt, one `key=value` per line (enabled=0 disables all; missing keys default to 1, except
// movspin = 0; key=0 disables a group; fs also takes 2 = count + free).
// Log (1.3.0): shared KpLog.h, channel main (core file k2-robustness-fixes_log.txt; old core: fallback robustnessfixes_log.txt at INFO).
//      One `loaded` line per launch with the per-group state; a helper thread polls
// every 5 s and appends a COUNTERS line (all 18 values) when any changed, a one-shot `FIRED <group> ...` line the
// first time each fix's counter moves (sndri, vmid, guiid, netovf, abbound, kt, fs, ssb, ssc, td, vmlk, movspin), and reports a non-zero byte in
// the KT dummy entry). 1.2.0: 12 groups, 18 counter values, FIRED lines for fs/ssb/ssc/td/vmlk/movspin too.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <initializer_list>
#include "CodePatch.h"
#include "KpLog.h"

KP_LOG_PATCH("k2-robustness-fixes");

namespace {

struct Site { uintptr_t at; uint8_t n; uint8_t orig[40]; };

enum { G_KT, G_VMID, G_SNDRI, G_NETOVF, G_GUIID, G_ABBOUND, G_FS, G_SSB, G_SSC, G_TD, G_VMLK, G_MOVSPIN, G_COUNT };
const char* const GroupKey[G_COUNT] = { "kt", "vmid", "sndri", "netovf", "guiid", "abbound", "fs", "ssb", "ssc", "td", "vmlk", "movspin" };
constexpr int NCOUNTERS = 18;

// Sites per group (original bytes from the dump, checked against the Steam Aspyr exe).
const Site KtA   = { 0x00726527, 10, { 0x89, 0x50, 0x0C, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF } };
const Site KtB   = { 0x00726C9C, 5,  { 0x89, 0x50, 0x0C, 0xEB, 0x3C } };
const Site Vmid  = { 0x00668FE0, 2,  { 0x7D, 0x30 } };
const Site VmidB = { 0x00669012, 5,  { 0xB8, 0x2E, 0xF8, 0xFF, 0xFF } };
const Site Sndri = { 0x007054CE, 12, { 0xC7, 0x82, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x52 } };
const Site NetA  = { 0x00704406, 15, { 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x8D, 0x4D, 0xE4, 0xE8, 0x6B, 0xF3, 0x02, 0x00 } };
const Site NetB  = { 0x007040EF, 5,  { 0x05, 0x00, 0x00, 0x01, 0x00 } };
const Site Guiid = { 0x0040F972, 6,  { 0x83, 0x7D, 0xB0, 0x00, 0x7C, 0x6C } };
const Site AbB   = { 0x0058C61A, 8,  { 0x8B, 0x55, 0xEC, 0x3B, 0x51, 0x04, 0x7D, 0x1C } };
// 1.2.0 (W8; original bytes read from the vanilla exe 6A522E)
const Site FsA   = { 0x00712F55, 6,  { 0x8B, 0x45, 0xF0, 0x8B, 0x4D, 0x08 } };
const Site SsbA  = { 0x00428B2A, 9,  { 0x83, 0xBD, 0x5C, 0xFF, 0xFF, 0xFF, 0x00, 0x74, 0x11 } };
const Site SscA  = { 0x00428AE5, 5,  { 0x3B, 0x55, 0xF0, 0x7E, 0x2D } };
const Site TdA   = { 0x0071C243, 37, { 0x8B, 0x55, 0x10, 0x8B, 0x45, 0xFC, 0x8A, 0x08, 0x88, 0x0A, 0x8B, 0x55, 0x10, 0x83, 0xC2, 0x01, 0x89, 0x55, 0x10,
                                       0x8B, 0x45, 0xFC, 0x83, 0xC0, 0x01, 0x89, 0x45, 0xFC, 0x8B, 0x4D, 0xF8, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xF8 } };
const Site TdB   = { 0x0071C2C9, 37, { 0x8B, 0x45, 0x10, 0x8B, 0x4D, 0xFC, 0x8A, 0x11, 0x88, 0x10, 0x8B, 0x45, 0x10, 0x83, 0xC0, 0x01, 0x89, 0x45, 0x10,
                                       0x8B, 0x4D, 0xFC, 0x83, 0xC1, 0x01, 0x89, 0x4D, 0xFC, 0x8B, 0x55, 0xF8, 0x83, 0xC2, 0x01, 0x89, 0x55, 0xF8 } };
const Site TdC   = { 0x0071C2F0, 6,  { 0x8B, 0x45, 0x08, 0x8B, 0x4D, 0xFC } };
const Site VmlkA = { 0x00669A27, 11, { 0x0F, 0xB6, 0x0D, 0x7F, 0x44, 0x99, 0x00, 0x3B, 0xC1, 0x75, 0x0C } };
const Site MovA  = { 0x00798A47, 9,  { 0x8B, 0x45, 0x84, 0x8B, 0x88, 0x54, 0x02, 0x00, 0x00 } };

constexpr uintptr_t KtAResume = 0x00726531, KtBResume = 0x00726CDD;
constexpr uintptr_t NetStrDtor = 0x00733780, NetEpilogue = 0x0070446D;
constexpr uintptr_t AbBody = 0x0058C622, AbExit = 0x0058C63E;
constexpr uintptr_t SndriResume = 0x007054DA, VmidResume = 0x00669017, GuiAccept = 0x0040F978, GuiReject = 0x0040F9E4;
constexpr uintptr_t AilQuickShutdownIat = 0x0098657C;
constexpr uintptr_t FsResume = 0x00712F5B, FsFreeResData = 0x007120C0;
constexpr uintptr_t SsbSkip = 0x00428B88, SsbPresent = 0x00428B33, SscResume = 0x00428AEA, TdResume = 0x0071C2F6;
constexpr uintptr_t VmlkSuccess = 0x00669A32, VmlkDone = 0x00669A3E, VmlkDeleteSituation = 0x007037C0;   // 1.2.1: was 0x6FDBB0 (needs ECX = owner; see header)
constexpr uintptr_t MovResume = 0x00798A50, SleepIat = 0x009860D4;

alignas(16) uint8_t g_ktDummy[0x24];   // stand-in key entry for orphaned CRes (+0x10 is written 0 by ReleaseResObject)
volatile LONG g_ktDestroy = 0, g_ktUpdate = 0, g_abClamp = 0;
volatile LONG g_sndri = 0, g_vmReject = 0, g_vmNeg = 0, g_vmLastId = 0, g_guiReject = 0, g_guiLastId = 0, g_netDrop = 0, g_netLastLen = 0;
volatile LONG g_fsStale = 0, g_fsStaleBytes = 0, g_ssbCamnull = 0, g_sscDims = 0, g_tdLong = 0, g_vmlkFree = 0, g_f1Sleeps = 0;
volatile uint8_t g_fsFree = 0;   // read by the FS cave: 0 = count only (fs=1), 1 = also free the stale buffer (fs=2)

int g_state[G_COUNT];      // 1 patched, 0 disabled, -1 refused (bytes), -2 refused (protect), -3 refused (unmapped), -4 no cave
uintptr_t g_badAt[G_COUNT]; uint8_t g_seen[G_COUNT][40]; uint8_t g_seenN[G_COUNT];
uint8_t* g_cave = nullptr; size_t g_caveUsed = 0;

// ---- log ----
// 1.3.0: shared KpLog.h (channel main) replaces the private fopen/append logger (KpLog serialises writes, so no lock). Lines keep their text;
// KpLog adds the prefix (core file k2-robustness-fixes_log.txt; old core: fallback robustnessfixes_log.txt at INFO, `<ms> k2-robustness-fixes/main ...`).
KP_CHANNEL(chMain, "main");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("robustnessfixes_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// DllMain formats the load line and the REFUSED continuation lines (setLoad / g_refLine) but they are emitted once the logger is known:
// by KPatchInit, else by the helper-thread tick (old core), or directly from DllMain when no group is patched (no helper thread).
char g_loadLine[700]; int g_loadLevel = KPLOG_INFO;
char g_refLine[G_COUNT][200];
volatile LONG g_loadEmitted = 0;
void setLoad(int level, const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vsnprintf(g_loadLine, sizeof(g_loadLine), fmt, ap); va_end(ap);
    size_t n = strlen(g_loadLine);
    snprintf(g_loadLine + n, sizeof(g_loadLine) - n, " tick=%lu", GetTickCount());
    g_loadLevel = level;
}
void emitLoadLine() {
    if (g_loadEmitted) return;                         // cheap fast path
    if (InterlockedExchange(&g_loadEmitted, 1)) return;
    KP_LOGV(chMain, g_loadLevel, "%s", g_loadLine);
    for (int g = 0; g < G_COUNT; ++g) if (g_refLine[g][0]) KP_LOGV(chMain, KPLOG_WARN, "%s", g_refLine[g]);
}
void logf(const char* fmt, ...) {
    char line[600];
    va_list ap; va_start(ap, fmt); vsnprintf(line, sizeof(line), fmt, ap); va_end(ap);
    KP_LOGV(chMain, KPLOG_INFO, "%s", line);
}

void readConfig(int lvl[G_COUNT]) {   // lvl: 0 off, 1 on (fs: 1 = count only, 2 = count + free)
    for (int g = 0; g < G_COUNT; ++g) lvl[g] = 1;
    lvl[G_MOVSPIN] = 0;   // default OFF
    FILE* f = fopen("robustnessfixes.txt", "r");
    if (!f) return;
    char line[128]; bool all = true;
    while (fgets(line, sizeof(line), f)) {
        char* p = line; while (*p == ' ' || *p == '\t') ++p;
        char* eq = strchr(p, '='); if (!eq) continue;
        char* e = eq; while (e > p && (e[-1] == ' ' || e[-1] == '\t')) --e;
        size_t kl = e - p; const char* v = eq + 1; while (*v == ' ') ++v;
        bool val = *v != '0';
        if (kl == 7 && !strncmp(p, "enabled", 7)) all = val;
        for (int g = 0; g < G_COUNT; ++g) if (kl == strlen(GroupKey[g]) && !strncmp(p, GroupKey[g], kl))
            lvl[g] = !val ? 0 : (g == G_FS && *v >= '2' && *v <= '9') ? 2 : 1;
    }
    fclose(f);
    if (!all) for (int g = 0; g < G_COUNT; ++g) lvl[g] = 0;
}

// Cave builder: appends to the RWX block; rel32 fields are resolved against the final cave address.
struct Emit {
    uint8_t* base; size_t n;
    uintptr_t here() const { return reinterpret_cast<uintptr_t>(base + n); }
    void b(std::initializer_list<int> v) { for (int x : v) base[n++] = static_cast<uint8_t>(x); }
    void d(uint32_t v) { memcpy(base + n, &v, 4); n += 4; }
    void rel(uint8_t op, uintptr_t target) { base[n++] = op; int32_t r = static_cast<int32_t>(target - (here() + 4)); memcpy(base + n, &r, 4); n += 4; }
    void lockInc(volatile LONG* p) { b({ 0xF0, 0xFF, 0x05 }); d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p))); }
    // rel8 jump with a label fixup: emits opcode + placeholder, returns the placeholder's offset for fix8().
    size_t j8(uint8_t op) { base[n++] = op; base[n++] = 0; return n - 1; }
    void fix8(size_t at, uintptr_t target) {
        intptr_t r = static_cast<intptr_t>(target) - static_cast<intptr_t>(reinterpret_cast<uintptr_t>(base + at) + 1);
        if (r < -128 || r > 127) { ++bad8; return; }
        base[at] = static_cast<uint8_t>(static_cast<int8_t>(r));
    }
    int bad8 = 0;   // number of rel8 fixups that did not fit (checked by the caller before writing)
};
uint8_t* caveAlloc(size_t want) {
    if (!g_cave) return nullptr;
    if (g_caveUsed + want > 4096) return nullptr;
    uint8_t* p = g_cave + g_caveUsed; g_caveUsed += (want + 15) & ~size_t(15);
    return p;
}
bool verifySite(int g, const Site& s) {
    if (!codepatch::committed(s.at, s.n)) { g_state[g] = -3; g_badAt[g] = s.at; return false; }
    memcpy(g_seen[g], reinterpret_cast<void*>(s.at), s.n); g_seenN[g] = s.n;
    if (memcmp(g_seen[g], s.orig, s.n) != 0) { g_state[g] = -1; g_badAt[g] = s.at; return false; }
    return true;
}
bool write(int g, uintptr_t at, const uint8_t* bytes, int n) {
    int r = codepatch::write(at, bytes, n);
    if (r != 1) { g_state[g] = r; g_badAt[g] = at; return false; }
    return true;
}

void applyKt() {
    const int g = G_KT;
    if (!verifySite(g, KtA) || !verifySite(g, KtB)) return;
    uint8_t* ca = caveAlloc(40); uint8_t* cb = caveAlloc(40);
    if (!ca || !cb) { g_state[g] = -4; return; }
    const uint32_t dummy = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_ktDummy));
    Emit a{ ca, 0 };
    a.b({ 0x89, 0x50, 0x0C });                 // mov [eax+0xC],edx      (replayed flag store)
    a.b({ 0xC7, 0x40, 0x14 }); a.d(dummy);     // mov dword [eax+0x14],&g_ktDummy
    a.lockInc(&g_ktDestroy);                   // lock inc [g_ktDestroy]  (flags dead: LEA + CALL 0x733780 follow)
    a.b({ 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF });   // mov dword [ebp-4],-1 (replayed)
    a.rel(0xE9, KtAResume);
    Emit b{ cb, 0 };
    b.b({ 0x89, 0x50, 0x0C });
    b.b({ 0xC7, 0x40, 0x14 }); b.d(dummy);
    b.lockInc(&g_ktUpdate);                    // flags dead: 0x726CDD JMP 0x726D57 JMP 0x726B61
    b.rel(0xE9, KtBResume);                    // replays EB 3C (JMP 0x726CDD)
    uint8_t ra[10], rb[5];
    codepatch::jmpRel(KtA.at, reinterpret_cast<uintptr_t>(ca), ra, 10);
    codepatch::jmpRel(KtB.at, reinterpret_cast<uintptr_t>(cb), rb, 5);
    if (write(g, KtA.at, ra, 10) && write(g, KtB.at, rb, 5)) g_state[g] = 1;
}

void applyVmid() {
    const int g = G_VMID;
    if (!verifySite(g, Vmid) || !verifySite(g, VmidB)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.lockInc(&g_vmReject);                    // every reject (positive >= 0x36D and negative)
    e.b({ 0x83, 0x7D, 0x08, 0x00 });           // cmp dword [ebp+8],0
    size_t jDone = e.j8(0x7D);                 // jge done
    e.lockInc(&g_vmNeg);                       // negative id: the case the JAE change newly rejects
    e.b({ 0x8B, 0x45, 0x08 });                 // mov eax,[ebp+8]
    e.b({ 0xA3 }); e.d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_vmLastId)));   // mov [g_vmLastId],eax
    uintptr_t done = e.here();
    e.b({ 0xB8, 0x2E, 0xF8, 0xFF, 0xFF });     // mov eax,-2002 (replayed)
    e.rel(0xE9, VmidResume);                   // jmp 0x669017
    e.fix8(jDone, done);
    if (e.bad8) { g_state[g] = -4; return; }
    const uint8_t r1[2] = { 0x73, 0x30 };      // JAE 0x669012 (same rel8)
    uint8_t r2[5]; codepatch::jmpRel(VmidB.at, reinterpret_cast<uintptr_t>(cv), r2, 5);
    if (write(g, Vmid.at, r1, 2) && write(g, VmidB.at, r2, 5)) g_state[g] = 1;
}

void applySndri() {
    const int g = G_SNDRI;
    if (!verifySite(g, Sndri)) return;
    uint8_t* cv = caveAlloc(32);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.lockInc(&g_sndri);                       // teardown-branch hit: sound was about to stay dead
    e.b({ 0xFF, 0x15 }); e.d(AilQuickShutdownIat);   // call dword [0x98657C] (_AIL_quick_shutdown@0)
    e.rel(0xE9, SndriResume);                  // jmp 0x7054DA (AIL_quick_startup block)
    uint8_t r[12]; codepatch::jmpRel(Sndri.at, reinterpret_cast<uintptr_t>(cv), r, 12);
    if (write(g, Sndri.at, r, 12)) g_state[g] = 1;
}

void applyNetovf() {
    const int g = G_NETOVF;
    if (!verifySite(g, NetA) || !verifySite(g, NetB)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.lockInc(&g_netDrop);
    e.b({ 0x8B, 0x45, 0x10 });                 // mov eax,[ebp+0x10]     message length
    e.b({ 0xA3 }); e.d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_netLastLen)));   // mov [g_netLastLen],eax
    e.b({ 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF });   // mov dword [ebp-4],-1 (replayed)
    e.b({ 0x8D, 0x4D, 0xE4 });                 // lea ecx,[ebp-0x1C]     (replayed)
    e.rel(0xE8, NetStrDtor);                   // call 0x733780          (replayed, rel32 from the cave)
    e.b({ 0x33, 0xC0 });                       // xor eax,eax (return 0 = not sent)
    e.rel(0xE9, NetEpilogue);                  // jmp 0x70446D
    uint8_t ra[15]; codepatch::jmpRel(NetA.at, reinterpret_cast<uintptr_t>(cv), ra, 15);
    const uint8_t rb[5] = { 0xB8, 0x00, 0x00, 0x02, 0x00 };   // mov eax,0x20000 (end > ring size = heap overflow)
    if (write(g, NetA.at, ra, 15) && write(g, NetB.at, rb, 5)) g_state[g] = 1;
}

void applyGuiid() {
    const int g = G_GUIID;
    if (!verifySite(g, Guiid)) return;
    uint8_t* cv = caveAlloc(64);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x83, 0x7D, 0xB0, 0x00 });           // cmp dword [ebp-0x50],0   (replayed)
    size_t jNeg = e.j8(0x7C);                  // jl engineReject          (negative: engine path, not counted)
    e.b({ 0x81, 0x7D, 0xB0, 0x00, 0x10, 0x00, 0x00 });   // cmp dword [ebp-0x50],0x1000
    size_t jOk = e.j8(0x7C);                   // jl accept
    e.lockInc(&g_guiReject);
    e.b({ 0x8B, 0x4D, 0xB0 });                 // mov ecx,[ebp-0x50]       (ECX only, never EAX)
    e.b({ 0x89, 0x0D }); e.d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_guiLastId)));   // mov [g_guiLastId],ecx
    uintptr_t engineReject = e.here(); e.rel(0xE9, GuiReject);   // jmp 0x40F9E4
    uintptr_t accept = e.here();       e.rel(0xE9, GuiAccept);   // jmp 0x40F978
    e.fix8(jNeg, engineReject); e.fix8(jOk, accept);
    if (e.bad8) { g_state[g] = -4; return; }
    uint8_t r[6]; codepatch::jmpRel(Guiid.at, reinterpret_cast<uintptr_t>(cv), r, 6);
    if (write(g, Guiid.at, r, 6)) g_state[g] = 1;
}

void applyAbBound() {
    const int g = G_ABBOUND;
    if (!verifySite(g, AbB)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x8B, 0x55, 0xEC });                 // mov edx,[ebp-0x14]     j
    e.b({ 0x3B, 0x51, 0x04 });                 // cmp edx,[ecx+4]        old count
    e.b({ 0x0F, 0x8D }); size_t jExit = e.n; e.d(0);   // jge exit
    e.b({ 0x3B, 0x51, 0x08 });                 // cmp edx,[ecx+8]        new capacity (stored at 0x58C576)
    e.b({ 0x0F, 0x8D }); size_t jClamp = e.n; e.d(0);  // jge clamp
    e.rel(0xE9, AbBody);                       // copy element j
    uintptr_t clamp = e.here(); e.lockInc(&g_abClamp);
    uintptr_t exitAt = e.here(); e.rel(0xE9, AbExit);
    auto fix = [&](size_t at, uintptr_t target) { int32_t r = static_cast<int32_t>(target - (reinterpret_cast<uintptr_t>(cv) + at + 4)); memcpy(cv + at, &r, 4); };
    fix(jExit, exitAt); fix(jClamp, clamp);
    uint8_t r[8]; codepatch::jmpRel(AbB.at, reinterpret_cast<uintptr_t>(cv), r, 8);
    if (write(g, AbB.at, r, 8)) g_state[g] = 1;
}

void applyFs() {
    const int g = G_FS;
    if (!verifySite(g, FsA)) return;
    uint8_t* cv = caveAlloc(96);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x8B, 0x4D, 0x08 });                 // mov ecx,[ebp+8]              CRes
    e.b({ 0x83, 0x79, 0x10, 0x00 });           // cmp dword [ecx+0x10],0
    size_t jNone = e.j8(0x74);                 // je done                      nothing attached
    e.b({ 0xF6, 0x41, 0x0C, 0x14 });           // test byte [ecx+0xC],0x14     4 = loaded, 0x10 = async in flight
    size_t jBusy = e.j8(0x75);                 // jnz done
    e.b({ 0x8B, 0x51, 0x18 });                 // mov edx,[ecx+0x18]           NEW size (what Malloc is about to use)
    e.lockInc(&g_fsStale);                     // lock inc [fs_stale]
    e.b({ 0xF0, 0x01, 0x15 }); e.d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_fsStaleBytes)));   // lock add [fs_stale_bytes],edx
    e.b({ 0x80, 0x3D }); e.d(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_fsFree))); e.b({ 0x00 });   // cmp byte [g_fsFree],0
    size_t jCount = e.j8(0x74);                // je done                      fs=1: count only, keep the buffer
    e.b({ 0x52 });                             // push edx                     save the size across the free
    e.b({ 0x8B, 0x01 });                       // mov eax,[ecx]
    e.b({ 0xFF, 0x50, 0x0C });                 // call dword [eax+0xC]         vtbl+0xC OnResourceFreed (as FreeChunk 0x712038)
    e.b({ 0x8B, 0x4D, 0x08 });                 // mov ecx,[ebp+8]
    e.b({ 0x51 });                             // push ecx                     arg (callee ret 4)
    e.b({ 0x8B, 0x4D, 0xF0 });                 // mov ecx,[ebp-0x10]           this = resman
    e.rel(0xE8, FsFreeResData);                // call 0x007120C0              FreeResourceData
    e.b({ 0x8B, 0x4D, 0x08 });                 // mov ecx,[ebp+8]
    e.b({ 0x58 });                             // pop eax                      saved size
    e.b({ 0x89, 0x41, 0x18 });                 // mov [ecx+0x18],eax           restore +0x18 (0x7120C0 zeroes it, Malloc reads it)
    uintptr_t done = e.here();
    e.b({ 0x8B, 0x45, 0xF0 });                 // mov eax,[ebp-0x10]           replay 1
    e.b({ 0x8B, 0x4D, 0x08 });                 // mov ecx,[ebp+8]              replay 2
    e.rel(0xE9, FsResume);                     // jmp 0x00712F5B
    e.fix8(jNone, done); e.fix8(jBusy, done); e.fix8(jCount, done);
    if (e.bad8) { g_state[g] = -4; return; }
    uint8_t r[6]; codepatch::jmpRel(FsA.at, reinterpret_cast<uintptr_t>(cv), r, 6);
    if (write(g, FsA.at, r, 6)) g_state[g] = 1;
}

void applySsb() {
    const int g = G_SSB;
    if (!verifySite(g, SsbA)) return;
    uint8_t* cv = caveAlloc(32);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x83, 0xBD, 0x5C, 0xFF, 0xFF, 0xFF, 0x00 });   // cmp dword [ebp-0xA4],0   (replayed)
    size_t jPresent = e.j8(0x75);              // jnz present
    e.lockInc(&g_ssbCamnull);                  // camera NULL: the render block would dereference it
    e.rel(0xE9, SsbSkip);                      // jmp 0x00428B88                skip the render block
    uintptr_t present = e.here(); e.rel(0xE9, SsbPresent);   // jmp 0x00428B33   camera present: normal path
    e.fix8(jPresent, present);
    if (e.bad8) { g_state[g] = -4; return; }
    uint8_t r[9]; codepatch::jmpRel(SsbA.at, reinterpret_cast<uintptr_t>(cv), r, 9);
    if (write(g, SsbA.at, r, 9)) g_state[g] = 1;
}

void applySsc() {
    const int g = G_SSC;
    if (!verifySite(g, SscA)) return;
    uint8_t* cv = caveAlloc(32);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x3B, 0x55, 0xF0 });                 // cmp edx,[ebp-0x10]       (replayed)
    size_t jOk = e.j8(0x7F);                   // jg cont                  width > height: original path
    e.lockInc(&g_sscDims);                     // w <= h: the original would have skipped the dims writers
    uintptr_t cont = e.here(); e.rel(0xE9, SscResume);   // jmp 0x00428AEA   both outcomes continue (JLE dropped)
    e.fix8(jOk, cont);
    if (e.bad8) { g_state[g] = -4; return; }
    uint8_t r[5]; codepatch::jmpRel(SscA.at, reinterpret_cast<uintptr_t>(cv), r, 5);
    if (write(g, SscA.at, r, 5)) g_state[g] = 1;
}

void applyTd() {
    const int g = G_TD;
    if (!verifySite(g, TdA) || !verifySite(g, TdB) || !verifySite(g, TdC)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x81, 0x7D, 0xF8, 0xFF, 0x00, 0x00, 0x00 });   // cmp dword [ebp-8],0xFF   token length
    size_t jFits = e.j8(0x76);                 // jbe replay               len <= 255 fits
    e.lockInc(&g_tdLong);                      // len >= 256: the token would have smashed the cookie
    uintptr_t replay = e.here();
    e.b({ 0x8B, 0x45, 0x08 });                 // mov eax,[ebp+8]          (replayed)
    e.b({ 0x8B, 0x4D, 0xFC });                 // mov ecx,[ebp-4]          (replayed)
    e.rel(0xE9, TdResume);                     // jmp 0x0071C2F6
    e.fix8(jFits, replay);
    if (e.bad8) { g_state[g] = -4; return; }
    // 27-byte clamped copy + 10 NOP (replaces both 37-byte store/advance blocks; the loop heads reload every register)
    static const uint8_t blk[37] = {
        0x8B, 0x55, 0xFC,                      // mov edx,[ebp-4]          source cursor
        0x8A, 0x0A,                            // mov cl,[edx]
        0x8B, 0x45, 0x10,                      // mov eax,[ebp+0x10]       dst
        0x88, 0x08,                            // mov [eax],cl
        0x81, 0x7D, 0xF8, 0xFF, 0x00, 0x00, 0x00,   // cmp dword [ebp-8],0xFF   CF = (len < 255)
        0x83, 0x55, 0x10, 0x00,                // adc dword [ebp+0x10],0   dst += CF (stays on index 255 afterwards)
        0xFF, 0x45, 0xFC,                      // inc dword [ebp-4]
        0xFF, 0x45, 0xF8,                      // inc dword [ebp-8]
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    uint8_t rc[6]; codepatch::jmpRel(TdC.at, reinterpret_cast<uintptr_t>(cv), rc, 6);
    if (write(g, TdA.at, blk, 37) && write(g, TdB.at, blk, 37) && write(g, TdC.at, rc, 6)) g_state[g] = 1;
}

void applyVmlk() {
    const int g = G_VMLK;
    if (!verifySite(g, VmlkA)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.b({ 0x0F, 0xB6, 0x0D, 0x7F, 0x44, 0x99, 0x00 });   // movzx ecx,byte [0x0099447F]   (replayed)
    e.b({ 0x3B, 0xC1 });                       // cmp eax,ecx                   (replayed)
    size_t jMissing = e.j8(0x75);              // jnz missing
    e.rel(0xE9, VmlkSuccess);                  // jmp 0x00669A32                object found: hand-over (perf-lab detour if live)
    uintptr_t missing = e.here();
    e.lockInc(&g_vmlkFree);                    // object missing: the popped situation would leak
    e.b({ 0xFF, 0x75, 0xF4 });                 // push dword [ebp-0xC]          situation
    e.rel(0xE8, VmlkDeleteSituation);          // call 0x007037C0               delete situation (ret 4, ECX unused)
    e.rel(0xE9, VmlkDone);                     // jmp 0x00669A3E                xor eax,eax / epilogue, as before
    e.fix8(jMissing, missing);
    if (e.bad8) { g_state[g] = -4; return; }
    uint8_t r[11]; codepatch::jmpRel(VmlkA.at, reinterpret_cast<uintptr_t>(cv), r, 11);
    if (write(g, VmlkA.at, r, 11)) g_state[g] = 1;
}

void applyMovspin() {
    const int g = G_MOVSPIN;
    if (!verifySite(g, MovA)) return;
    uint8_t* cv = caveAlloc(48);
    if (!cv) { g_state[g] = -4; return; }
    Emit e{ cv, 0 };
    e.lockInc(&g_f1Sleeps);                    // one per message pump (= one per 501 spins)
    e.b({ 0x6A, 0x01 });                       // push 1
    e.b({ 0xFF, 0x15 }); e.d(SleepIat);        // call dword [0x009860D4]       Sleep(1) (stdcall; clobbers EAX/ECX/EDX only)
    e.b({ 0x8B, 0x45, 0x84 });                 // mov eax,[ebp-0x7C]            (replayed)
    e.b({ 0x8B, 0x88, 0x54, 0x02, 0x00, 0x00 });   // mov ecx,[eax+0x254]       (replayed)
    e.rel(0xE9, MovResume);                    // jmp 0x00798A50
    uint8_t r[9]; codepatch::jmpRel(MovA.at, reinterpret_cast<uintptr_t>(cv), r, 9);
    if (write(g, MovA.at, r, 9)) g_state[g] = 1;
}

const char* stateName(int s) {
    switch (s) { case 1: return "ON"; case 0: return "off"; case -1: return "REFUSED(bytes)"; case -2: return "REFUSED(protect)";
                 case -3: return "REFUSED(unmapped)"; default: return "REFUSED(cave)"; }
}

DWORD WINAPI helperThread(LPVOID) {
    LONG last[NCOUNTERS]; for (LONG& v : last) v = -1;
    bool dummyReported = false, firedSndri = false, firedVm = false, firedGui = false, firedNet = false, firedAb = false, firedKt = false;
    bool firedFs = false, firedSsb = false, firedSsc = false, firedTd = false, firedVmlk = false, firedMov = false;
    for (;;) {
        Sleep(5000);
        emitLoadLine();
        LONG now[NCOUNTERS] = { g_ktDestroy, g_ktUpdate, g_abClamp, g_sndri, g_vmReject, g_vmNeg, g_vmLastId, g_guiReject, g_guiLastId, g_netDrop, g_netLastLen,
                                g_fsStale, g_fsStaleBytes, g_ssbCamnull, g_sscDims, g_tdLong, g_vmlkFree, g_f1Sleeps };
        if (memcmp(now, last, sizeof now)) {
            // Two lines (the KPM core truncates a line's text at 232 bytes); worst case 223 / 202 bytes with 10-digit values.
            logf("COUNTERS kt_destroy_orphan=%ld kt_update_orphan=%ld ab_shrink_clamp=%ld sndri_restart=%ld vm_reject=%ld vm_neg=%ld vm_last_id=%ld "
                 "gui_reject=%ld gui_last_id=%ld",
                 now[0], now[1], now[2], now[3], now[4], now[5], now[6], now[7], now[8]);
            logf("COUNTERS net_drop=%ld net_last_len=%ld fs_stale=%ld fs_stale_bytes=%ld ssb_camnull=%ld ssc_dims=%ld td_long=%ld vmlk_free=%ld f1_sleeps=%ld",
                 now[9], now[10], now[11], now[12], now[13], now[14], now[15], now[16], now[17]);
            memcpy(last, now, sizeof now);
        }
        if (!firedSndri && now[3]) { firedSndri = true; logf("FIRED sndri restart=%ld", now[3]); }
        if (!firedVm && now[5]) { firedVm = true; logf("FIRED vmid neg=%ld last_id=%ld", now[5], now[6]); }
        if (!firedGui && now[7]) { firedGui = true; logf("FIRED guiid reject=%ld last_id=%ld", now[7], now[8]); }
        if (!firedNet && now[9]) { firedNet = true; logf("FIRED netovf drop=%ld last_len=%ld", now[9], now[10]); }
        if (!firedAb && now[2]) { firedAb = true; logf("FIRED abbound clamp=%ld", now[2]); }
        if (!firedKt && (now[0] || now[1])) { firedKt = true; logf("FIRED kt destroy=%ld update=%ld", now[0], now[1]); }
        if (!firedFs && now[11]) { firedFs = true; logf("FIRED fs stale=%ld bytes=%ld mode=%s", now[11], now[12], g_fsFree ? "free" : "count"); }
        if (!firedSsb && now[13]) { firedSsb = true; logf("FIRED ssb camnull=%ld", now[13]); }
        if (!firedSsc && now[14]) { firedSsc = true; logf("FIRED ssc dims=%ld", now[14]); }
        if (!firedTd && now[15]) { firedTd = true; logf("FIRED td long=%ld", now[15]); }
        if (!firedVmlk && now[16]) { firedVmlk = true; logf("FIRED vmlk free=%ld", now[16]); }
        if (!firedMov && now[17]) { firedMov = true; logf("FIRED movspin sleeps=%ld", now[17]); }
        if (!dummyReported) {
            for (int i = 0; i < 0x24; ++i) if (g_ktDummy[i]) {
                KP_LOGV(chMain, KPLOG_WARN, "KT dummy entry byte +0x%02x = %02x (unexpected writer; left as is)", i, g_ktDummy[i]);
                dummyReported = true; break;
            }
        }
    }
}

}  // namespace

// Harness-only exports (never called by the game).
extern "C" int __cdecl RobFixTestState(int g) { return g >= 0 && g < G_COUNT ? g_state[g] : -99; }
// out18 order: kt_destroy_orphan kt_update_orphan ab_shrink_clamp sndri_restart vm_reject vm_neg vm_last_id gui_reject gui_last_id net_drop net_last_len
//              fs_stale fs_stale_bytes ssb_camnull ssc_dims td_long vmlk_free f1_sleeps
extern "C" void __cdecl RobFixTestCounters(long* out18) {
    const long v[NCOUNTERS] = { g_ktDestroy, g_ktUpdate, g_abClamp, g_sndri, g_vmReject, g_vmNeg, g_vmLastId, g_guiReject, g_guiLastId, g_netDrop, g_netLastLen,
                                g_fsStale, g_fsStaleBytes, g_ssbCamnull, g_sscDims, g_tdLong, g_vmlkFree, g_f1Sleeps };
    for (int i = 0; i < NCOUNTERS; ++i) out18[i] = v[i];
}
extern "C" void* __cdecl RobFixTestDummy() { return g_ktDummy; }

// KpLog_Init may fail on an old core; the load line is emitted either way (KpLog then falls back to robustnessfixes_log.txt).
extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); emitLoadLine(); }

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    int lvl[G_COUNT]; readConfig(lvl);
    g_fsFree = lvl[G_FS] == 2 ? 1 : 0;
    g_cave = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    void (*const apply[G_COUNT])() = { applyKt, applyVmid, applySndri, applyNetovf, applyGuiid, applyAbBound,
                                       applyFs, applySsb, applySsc, applyTd, applyVmlk, applyMovspin };
    for (int g = 0; g < G_COUNT; ++g) if (lvl[g]) apply[g]();
    char buf[500]; int k = 0; int patched = 0;
    for (int g = 0; g < G_COUNT; ++g) {
        if (g == G_FS && g_state[g] == 1) k += snprintf(buf + k, sizeof(buf) - k, " fs=ON(%s)", g_fsFree ? "free" : "count");
        else k += snprintf(buf + k, sizeof(buf) - k, " %s=%s", GroupKey[g], stateName(g_state[g]));
        if (g_state[g] == 1) ++patched;
    }
    bool refused = false; for (int g = 0; g < G_COUNT; ++g) if (g_state[g] < 0) refused = true;
    setLoad(refused ? KPLOG_WARN : KPLOG_INFO, "k2-robustness-fixes 1.3.0 loaded: %d/%d groups patched:%s cave=%p pid=%lu", patched, G_COUNT, buf, (void*)g_cave, GetCurrentProcessId());
    for (int g = 0; g < G_COUNT; ++g) if (g_state[g] == -1) {
        char hex[100]; int h = 0; for (int i = 0; i < g_seenN[g] && i < 24; ++i) h += snprintf(hex + h, sizeof(hex) - h, "%02x ", g_seen[g][i]);
        snprintf(g_refLine[g], sizeof g_refLine[g], "  %s REFUSED: bytes at 0x%08x are %s(group untouched)", GroupKey[g], (unsigned)g_badAt[g], hex);
    } else if (g_state[g] < -1) snprintf(g_refLine[g], sizeof g_refLine[g], "  %s REFUSED at 0x%08x%s", GroupKey[g], (unsigned)g_badAt[g], g_state[g] == -2 ? " (VirtualProtect failed; group may be PARTIAL)" : "");
    bool any = false; for (int g = 0; g < G_COUNT; ++g) if (g_state[g] == 1) any = true;
    if (any) CloseHandle(CreateThread(nullptr, 0, helperThread, nullptr, 0, nullptr));
    else emitLoadLine();   // no helper thread to emit it later
    return TRUE;
}
