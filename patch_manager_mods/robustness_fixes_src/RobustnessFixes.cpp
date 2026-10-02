// k2-robustness-fixes 1.1.0 (Steam Aspyr build 6A522E71...). Engine robustness fixes from the
// doc 10 Phase 6E research (batches W4-W7, Opus-verified; section "Robustness set").
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
//
// Config: $G/robustnessfixes.txt, one `key=0|1` per line (enabled=0 disables all; missing keys default to 1).
// Log: $G/robustnessfixes_log.txt (one `loaded` line per launch with the per-group state; a helper thread polls
// every 5 s and appends a COUNTERS line (all 11 values) when any changed, a one-shot `FIRED <group> ...` line the
// first time each fix's counter moves (sndri, vmid, guiid, netovf, abbound, kt), and reports a non-zero byte in
// the KT dummy entry).
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <initializer_list>

namespace {

struct Site { uintptr_t at; uint8_t n; uint8_t orig[16]; };

enum { G_KT, G_VMID, G_SNDRI, G_NETOVF, G_GUIID, G_ABBOUND, G_COUNT };
const char* const GroupKey[G_COUNT] = { "kt", "vmid", "sndri", "netovf", "guiid", "abbound" };

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

constexpr uintptr_t KtAResume = 0x00726531, KtBResume = 0x00726CDD;
constexpr uintptr_t NetStrDtor = 0x00733780, NetEpilogue = 0x0070446D;
constexpr uintptr_t AbBody = 0x0058C622, AbExit = 0x0058C63E;
constexpr uintptr_t SndriResume = 0x007054DA, VmidResume = 0x00669017, GuiAccept = 0x0040F978, GuiReject = 0x0040F9E4;
constexpr uintptr_t AilQuickShutdownIat = 0x0098657C;

alignas(16) uint8_t g_ktDummy[0x24];   // stand-in key entry for orphaned CRes (+0x10 is written 0 by ReleaseResObject)
volatile LONG g_ktDestroy = 0, g_ktUpdate = 0, g_abClamp = 0;
volatile LONG g_sndri = 0, g_vmReject = 0, g_vmNeg = 0, g_vmLastId = 0, g_guiReject = 0, g_guiLastId = 0, g_netDrop = 0, g_netLastLen = 0;

CRITICAL_SECTION g_logLock;
int g_state[G_COUNT];      // 1 patched, 0 disabled, -1 refused (bytes), -2 refused (protect), -3 refused (unmapped), -4 no cave
uintptr_t g_badAt[G_COUNT]; uint8_t g_seen[G_COUNT][16]; uint8_t g_seenN[G_COUNT];
uint8_t* g_cave = nullptr; size_t g_caveUsed = 0;

void logf(const char* fmt, ...) {
    char line[600];
    va_list ap; va_start(ap, fmt); vsnprintf(line, sizeof(line), fmt, ap); va_end(ap);
    EnterCriticalSection(&g_logLock);
    if (FILE* f = fopen("robustnessfixes_log.txt", "a")) {
        SYSTEMTIME t; GetLocalTime(&t);
        fprintf(f, "[%02u:%02u:%02u.%03u %lu] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, GetTickCount(), line);
        fclose(f);
    }
    LeaveCriticalSection(&g_logLock);
}

void readConfig(bool on[G_COUNT]) {
    for (int g = 0; g < G_COUNT; ++g) on[g] = true;
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
        for (int g = 0; g < G_COUNT; ++g) if (kl == strlen(GroupKey[g]) && !strncmp(p, GroupKey[g], kl)) on[g] = val;
    }
    fclose(f);
    if (!all) for (int g = 0; g < G_COUNT; ++g) on[g] = false;
}

bool committed(uintptr_t at, size_t n) {
    MEMORY_BASIC_INFORMATION mb;
    return VirtualQuery(reinterpret_cast<void*>(at), &mb, sizeof(mb)) && mb.State == MEM_COMMIT &&
           at + n <= reinterpret_cast<uintptr_t>(mb.BaseAddress) + mb.RegionSize;
}
int writeCode(uintptr_t at, const uint8_t* repl, int n) {
    auto p = reinterpret_cast<uint8_t*>(at);
    DWORD old;
    if (!VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &old)) return -2;
    memcpy(p, repl, n);
    VirtualProtect(p, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, n);
    return 1;
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
void jmpTo(uintptr_t at, uintptr_t target, uint8_t* out, int n) {   // E9 rel32 + NOP pad to n bytes
    out[0] = 0xE9; int32_t r = static_cast<int32_t>(target - (at + 5)); memcpy(out + 1, &r, 4);
    for (int i = 5; i < n; ++i) out[i] = 0x90;
}

bool verifySite(int g, const Site& s) {
    if (!committed(s.at, s.n)) { g_state[g] = -3; g_badAt[g] = s.at; return false; }
    memcpy(g_seen[g], reinterpret_cast<void*>(s.at), s.n); g_seenN[g] = s.n;
    if (memcmp(g_seen[g], s.orig, s.n) != 0) { g_state[g] = -1; g_badAt[g] = s.at; return false; }
    return true;
}
bool write(int g, uintptr_t at, const uint8_t* bytes, int n) {
    int r = writeCode(at, bytes, n);
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
    jmpTo(KtA.at, reinterpret_cast<uintptr_t>(ca), ra, 10);
    jmpTo(KtB.at, reinterpret_cast<uintptr_t>(cb), rb, 5);
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
    uint8_t r2[5]; jmpTo(VmidB.at, reinterpret_cast<uintptr_t>(cv), r2, 5);
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
    uint8_t r[12]; jmpTo(Sndri.at, reinterpret_cast<uintptr_t>(cv), r, 12);
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
    uint8_t ra[15]; jmpTo(NetA.at, reinterpret_cast<uintptr_t>(cv), ra, 15);
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
    uint8_t r[6]; jmpTo(Guiid.at, reinterpret_cast<uintptr_t>(cv), r, 6);
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
    uint8_t r[8]; jmpTo(AbB.at, reinterpret_cast<uintptr_t>(cv), r, 8);
    if (write(g, AbB.at, r, 8)) g_state[g] = 1;
}

const char* stateName(int s) {
    switch (s) { case 1: return "ON"; case 0: return "off"; case -1: return "REFUSED(bytes)"; case -2: return "REFUSED(protect)";
                 case -3: return "REFUSED(unmapped)"; default: return "REFUSED(cave)"; }
}

DWORD WINAPI helperThread(LPVOID) {
    LONG last[11]; for (LONG& v : last) v = -1;
    bool dummyReported = false, firedSndri = false, firedVm = false, firedGui = false, firedNet = false, firedAb = false, firedKt = false;
    for (;;) {
        Sleep(5000);
        LONG now[11] = { g_ktDestroy, g_ktUpdate, g_abClamp, g_sndri, g_vmReject, g_vmNeg, g_vmLastId, g_guiReject, g_guiLastId, g_netDrop, g_netLastLen };
        if (memcmp(now, last, sizeof now)) {
            logf("COUNTERS kt_destroy_orphan=%ld kt_update_orphan=%ld ab_shrink_clamp=%ld sndri_restart=%ld vm_reject=%ld vm_neg=%ld vm_last_id=%ld "
                 "gui_reject=%ld gui_last_id=%ld net_drop=%ld net_last_len=%ld",
                 now[0], now[1], now[2], now[3], now[4], now[5], now[6], now[7], now[8], now[9], now[10]);
            memcpy(last, now, sizeof now);
        }
        if (!firedSndri && now[3]) { firedSndri = true; logf("FIRED sndri restart=%ld", now[3]); }
        if (!firedVm && now[5]) { firedVm = true; logf("FIRED vmid neg=%ld last_id=%ld", now[5], now[6]); }
        if (!firedGui && now[7]) { firedGui = true; logf("FIRED guiid reject=%ld last_id=%ld", now[7], now[8]); }
        if (!firedNet && now[9]) { firedNet = true; logf("FIRED netovf drop=%ld last_len=%ld", now[9], now[10]); }
        if (!firedAb && now[2]) { firedAb = true; logf("FIRED abbound clamp=%ld", now[2]); }
        if (!firedKt && (now[0] || now[1])) { firedKt = true; logf("FIRED kt destroy=%ld update=%ld", now[0], now[1]); }
        if (!dummyReported) {
            for (int i = 0; i < 0x24; ++i) if (g_ktDummy[i]) {
                logf("KT dummy entry byte +0x%02x = %02x (unexpected writer; left as is)", i, g_ktDummy[i]);
                dummyReported = true; break;
            }
        }
    }
}

}  // namespace

// Harness-only exports (never called by the game).
extern "C" int __cdecl RobFixTestState(int g) { return g >= 0 && g < G_COUNT ? g_state[g] : -99; }
// out11 order: kt_destroy_orphan kt_update_orphan ab_shrink_clamp sndri_restart vm_reject vm_neg vm_last_id gui_reject gui_last_id net_drop net_last_len
extern "C" void __cdecl RobFixTestCounters(long* out11) {
    const long v[11] = { g_ktDestroy, g_ktUpdate, g_abClamp, g_sndri, g_vmReject, g_vmNeg, g_vmLastId, g_guiReject, g_guiLastId, g_netDrop, g_netLastLen };
    for (int i = 0; i < 11; ++i) out11[i] = v[i];
}
extern "C" void* __cdecl RobFixTestDummy() { return g_ktDummy; }

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason != DLL_PROCESS_ATTACH) return TRUE;
    InitializeCriticalSection(&g_logLock);
    bool on[G_COUNT]; readConfig(on);
    g_cave = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    void (*const apply[G_COUNT])() = { applyKt, applyVmid, applySndri, applyNetovf, applyGuiid, applyAbBound };
    for (int g = 0; g < G_COUNT; ++g) if (on[g]) apply[g]();
    char buf[400]; int k = 0; int patched = 0;
    for (int g = 0; g < G_COUNT; ++g) {
        k += snprintf(buf + k, sizeof(buf) - k, " %s=%s", GroupKey[g], stateName(g_state[g]));
        if (g_state[g] == 1) ++patched;
    }
    logf("k2-robustness-fixes 1.1.0 loaded: %d/%d groups patched:%s cave=%p pid=%lu", patched, G_COUNT, buf, (void*)g_cave, GetCurrentProcessId());
    for (int g = 0; g < G_COUNT; ++g) if (g_state[g] == -1) {
        char hex[64]; int h = 0; for (int i = 0; i < g_seenN[g]; ++i) h += snprintf(hex + h, sizeof(hex) - h, "%02x ", g_seen[g][i]);
        logf("  %s REFUSED: bytes at 0x%08x are %s(group untouched)", GroupKey[g], (unsigned)g_badAt[g], hex);
    } else if (g_state[g] < -1) logf("  %s REFUSED at 0x%08x%s", GroupKey[g], (unsigned)g_badAt[g], g_state[g] == -2 ? " (VirtualProtect failed; group may be PARTIAL)" : "");
    bool any = false; for (int g = 0; g < G_COUNT; ++g) if (g_state[g] == 1) any = true;
    if (any) CloseHandle(CreateThread(nullptr, 0, helperThread, nullptr, 0, nullptr));
    return TRUE;
}
