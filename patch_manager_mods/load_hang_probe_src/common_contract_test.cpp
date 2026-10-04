// common_contract_test.cpp -- Patches/Common CONTRACT test for Load Hang Probe (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "load_hang_probe relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY
// edit to SafeRead.h re-runs this test: a Common change that would break the probe fails here first.
//
// Areas: (1) readAt as the snapshot code uses it (LoadHangProbe.cpp:148-175), (2) the failure ring: off until enableRing(true) (:734),
// g_ringCount (:610), dump() (:497, :725). Host-only, deterministic.
// (3) CodePatch.h codepatch::patchIat (the five file-I/O IAT hooks), via injected codepatch::ops() fakes and a hand-built PE32 image. Failures are made with the stub's VirtualQuery switched off.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "CodePatch.h"
#include "check.h"
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

#define HCHECK(why, expr) ::kt::Check("load_hang_probe relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("load_hang_probe relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

using saferead::readAt;   // LoadHangProbe.cpp:41

static int g_sink = 0;
static uint8_t* g_arena = nullptr;
constexpr size_t ArenaSize = 1u << 16;

// A readAt that fails because the OS says the page is unreadable (not because of the address), at an arena address.
static bool failingRead(unsigned site, uintptr_t base, int off) {
    int v = 0;
    g_stubVirtualQueryAll = false; saferead::beginScope();
    bool r = readAt(site, base, off, &v);
    g_stubVirtualQueryAll = true; saferead::beginScope();
    return r;
}

// ---- codepatch (CodePatch.h) fakes: every Win32 call goes through codepatch::ops(), replaced here by recorders ------------------
namespace cpfake {
struct ProtCall { void* p; SIZE_T n; DWORD newProt; uint8_t firstByte; int seq; };
struct FlushCall { const void* p; SIZE_T n; int seq; };
static ProtCall prot[8]; static int protN = 0;
static FlushCall flushed[8]; static int flushN = 0;
static int seq = 0;
static bool protFail = false;
static DWORD oldProt = PAGE_EXECUTE_READ;   // what the fake reports as the previous protection
static BOOL protect(LPVOID p, SIZE_T n, DWORD np, PDWORD old) {
    if (protN < 8) prot[protN] = { p, n, np, *static_cast<uint8_t*>(p), seq };
    ++protN; ++seq;
    if (protFail) return 0;
    *old = oldProt;
    return 1;
}
static void flush(const void* p, SIZE_T n) { if (flushN < 8) flushed[flushN] = { p, n, seq }; ++flushN; ++seq; }
static SIZE_T query(LPCVOID, MEMORY_BASIC_INFORMATION*, SIZE_T) { return 0; }
static void reset() { protN = flushN = seq = 0; protFail = false; oldProt = PAGE_EXECUTE_READ; }
}

// ---- hand-built PE32 image for patchIat (offsets by hand, independent of CodePatch.h's structs) -----------------------------
// desc0 = decoy dll importing the same function names (slot values 0x5555000i); desc1 = the mod's dll: slot 0 is an ordinal import,
// slot i+1 is fn[i] (value 0x2222000i). The two descriptors share the name strings, so a wrong-dll match would swap the decoy.
static uint8_t g_img[0x1000] __attribute__((aligned(16)));
static void put32(uint32_t off, uint32_t v) { memcpy(g_img + off, &v, 4); }
static void putStr(uint32_t off, const char* s) { memcpy(g_img + off, s, strlen(s) + 1); }
static uint32_t get32(uint32_t off) { uint32_t v; memcpy(&v, g_img + off, 4); return v; }
constexpr uint32_t E_LFANEW = 0x80, DIR_IMPORT = E_LFANEW + 24 + 96 + 8;      // PE sig(4)+file hdr(20); data dirs at opt+96; dir 1 = import
constexpr uint32_t DESC_D = 0x200, DESC_G = DESC_D + 20;
constexpr uint32_t OFT_D = 0x400, OFT_G = 0x440, FT_D = 0x500, FT_G = 0x540, NAMES = 0x600;
constexpr uint32_t N_FN = 3;
static const char* const FN[N_FN] = { "CreateFileA", "ReadFile", "CloseHandle" };
static void buildImage(const char* dll, const char* decoy, bool withImportDir) {
    memset(g_img, 0, sizeof g_img);
    put32(0x3C, E_LFANEW);
    memcpy(g_img + E_LFANEW, "PE\0\0", 4);
    if (withImportDir) { put32(DIR_IMPORT, DESC_D); put32(DIR_IMPORT + 4, 60); }
    putStr(0x300, decoy); putStr(0x320, dll);
    for (uint32_t i = 0; i < N_FN; ++i) putStr(NAMES + i * 0x20 + 2, FN[i]);                   // hint(2) + name
    put32(DESC_D + 0, OFT_D); put32(DESC_D + 12, 0x300); put32(DESC_D + 16, FT_D);
    put32(DESC_G + 0, OFT_G); put32(DESC_G + 12, 0x320); put32(DESC_G + 16, FT_G);
    put32(OFT_G, 0x80000000u | 16); put32(FT_G, 0x11110000u);
    for (uint32_t i = 0; i < N_FN; ++i) {
        put32(OFT_D + i * 4, NAMES + i * 0x20); put32(FT_D + i * 4, 0x55550000u + i);
        put32(OFT_G + (i + 1) * 4, NAMES + i * 0x20); put32(FT_G + (i + 1) * 4, 0x22220000u + i);
    }                                                                                           // terminators stay zero
}
static uint32_t slotG(uint32_t i) { return FT_G + (i + 1) * 4; }
static uint32_t slotD(uint32_t i) { return FT_D + i * 4; }

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    flags |= MAP_32BIT;
#endif
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    if (static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(m)) + ArenaSize > 0xFFFFFFFFull) { printf("arena above 4GB\n"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    memset(g_arena, 0, ArenaSize);
    g_stubVirtualQueryAll = true;
    const uintptr_t A = reinterpret_cast<uintptr_t>(g_arena);

    // ---- readAt: the snapshot walk app -> client -> internal -> fields (LoadHangProbe.cpp:151-175)
    // fake app global at A: *(A)=app; app+4=client; client+4=internal; internal+0x288 = int
    *reinterpret_cast<uint32_t*>(A + 0x00) = static_cast<uint32_t>(A + 0x100);        // AppGlobal -> app
    *reinterpret_cast<uint32_t*>(A + 0x104) = static_cast<uint32_t>(A + 0x200);       // app+4 -> client
    *reinterpret_cast<uint32_t*>(A + 0x204) = static_cast<uint32_t>(A + 0x300);       // client+4 -> internal
    *reinterpret_cast<int32_t*>(A + 0x300 + 0x288) = 7;
    *reinterpret_cast<int32_t*>(A + 0x300 + 0x18) = -3;
    {
        saferead::beginScope();
        // The mod's uintptr_t is 4 bytes on the 32-bit target; uint32_t models it here (a host uintptr_t would be an 8-byte misaligned load).
        uint32_t app = 0, client = 0, internal = 0; int v = 99;
        HCHECK("readAt(1, AppGlobal, 0, &app) reads an absolute global (base = address, off 0) (LoadHangProbe.cpp:151)", readAt(1, A, 0, &app));
        HEQ("the global's value arrives unchanged in a pointer-sized (32-bit target) out (:151)", (unsigned long long)app, (unsigned long long)(A + 0x100));
        HCHECK("chained readAt(2, app, 4, &client) follows the pointer read before (:152)", readAt(2, app, 4, &client));
        HCHECK("chained readAt(3, client, 4, &internal) (:152)", readAt(3, client, 4, &internal));
        HEQ("internal resolves through two hops (:152)", (unsigned long long)internal, (unsigned long long)(A + 0x300));
        HCHECK("readAt int at a field offset (In_AreaReady-style, :165)", readAt(22, internal, 0x288, &v));
        HEQ("int field value read back exactly (:165)", v, 7);
        HCHECK("readAt int with a small offset (:172 +0x18)", readAt(42, internal, 0x18, &v));
        HEQ("negative int is preserved, not zero-extended (:172-173)", v, -3);
        uint32_t u = 0;
        HCHECK("readAt<uint32_t> works too", readAt(1, A, 0, &u));
        HEQ("uint32_t read matches the stored word", (unsigned long long)u, (unsigned long long)(A + 0x100));
    }
    {   // failure leaves the caller's pre-set value (the code initialises locals to 0/-1 and relies on that, :169-170)
        int v = 1234;
        bool r = readAt(1, 0, 0, &v);
        HCHECK("readAt with base 0 returns false (null app/client/internal chain stops, :152)", !r);
        HEQ("a failed readAt leaves *out untouched (locals are pre-initialised, :169)", v, 1234);
        r = readAt(1, 0x20, 0, &v);
        HCHECK("readAt below 0x10000 is refused (garbage pointer in a chain)", !r);
        HEQ("*out untouched after the low-address refusal", v, 1234);
        HCHECK("readAt with an unreadable region returns false", !failingRead(5, A, 0));
        int w = 77;
        HCHECK("probe succeeds again after the region is readable again (the scope cache was reset)", readAt(5, A, 0, &w));
    }

    // ---- ring: off until enableRing(true) (LoadHangProbe.cpp:734)
    {
        LONG before = saferead::g_ringCount;
        failingRead(50, A, 0);
        readAt(51, 0x20, 0, &g_sink);
        HEQ("failures are NOT recorded before enableRing(true) (:734): g_ringCount stays put", (long long)saferead::g_ringCount, (long long)before);
        char t[256];
        saferead::dump(t, sizeof t);
        HCHECK("dump with the ring off still reports a header (the diag export works before init, :725)", strncmp(t, "saferead: 0 failed probes", 25) == 0);
        saferead::enableRing(true);
    }
    // ---- ring on: counts and dump text
    {
        LONG c0 = saferead::g_ringCount;
        int dummy = 0;
        readAt(33, A + ArenaSize - 0x10, 0, &dummy);   // readable: no record
        HEQ("a successful readAt adds nothing to g_ringCount (:610 failedProbes)", (long long)saferead::g_ringCount, (long long)c0);
        readAt(26, 0, 0, &dummy);
        HEQ("base==0 short-circuits before the probe, so it is not counted as a failed probe", (long long)saferead::g_ringCount, (long long)c0);
        readAt(27, 0x20, 0, &dummy);
        HEQ("a failed readAt adds exactly 1 to g_ringCount (:610)", (long long)saferead::g_ringCount, (long long)c0 + 1);
        failingRead(28, A + 0x40, 4);
        HEQ("a second failed readAt (unreadable region) adds 1 more (:610)", (long long)saferead::g_ringCount, (long long)c0 + 2);
        static char ring[8192];   // as LoadHangProbe.cpp:497
        int n = saferead::dump(ring, sizeof(ring));
        char hdr[64]; snprintf(hdr, sizeof hdr, "saferead: %ld failed probes", (long)saferead::g_ringCount);
        HCHECK("dump output starts with \"saferead: <n> failed probes\" (log analyser greps it, :497)", strncmp(ring, hdr, strlen(hdr)) == 0);
        HEQ("dump returns the string length (NUL-terminated)", n, (int)strlen(ring));
        HCHECK("dump lists the site id of a failed probe (site=27, :497)", strstr(ring, "site=27 ") != nullptr);
        HCHECK("dump lists the address of that probe (addr=00000020)", strstr(ring, "addr=00000020") != nullptr);
        char sa[40]; snprintf(sa, sizeof sa, "site=28 addr=%08x", (unsigned)(A + 0x44));
        HCHECK("dump lists base+off as the failed address (site=28 addr=base+4)", strstr(ring, sa) != nullptr);
        HCHECK("dump shows entries oldest first (27 before 28)", strstr(ring, "site=27 ") < strstr(ring, "site=28 "));
        HCHECK("dump does not list the successful or base==0 probes", strstr(ring, "site=33 ") == nullptr && strstr(ring, "site=26 ") == nullptr);
    }
    // ---- HangProbeDumpDiag shape (:725): null buf / cap <= 0 -> 0; tiny caps stay NUL-terminated and in bounds
    {
        auto diag = [](char* buf, int cap) { return buf && cap > 0 ? saferead::dump(buf, cap) : 0; };   // the export's body, :725
        char g[64]; memset(g, 'G', sizeof g);
        HEQ("diag export: null buf returns 0 (:725)", saferead::dump(nullptr, 100), 0);
        HEQ("dump(null, cap) returns 0 without crashing", diag(nullptr, 100), 0);
        HEQ("dump with cap 0 returns 0 and writes nothing", saferead::dump(g, 0), 0);
        HCHECK("cap 0 left the buffer untouched", g[0] == 'G');
        HEQ("dump with negative cap returns 0", saferead::dump(g, -4), 0);
        HCHECK("negative cap left the buffer untouched", g[0] == 'G');
        memset(g, 'G', sizeof g);
        int n = saferead::dump(g, 16);
        HEQ("a tiny cap (16) is clamped to cap-1 chars (:725 caller buffer)", n, 15);
        HEQ("tiny cap output is NUL-terminated at cap-1", g[15], 0);
        HEQ("tiny cap output does not write past cap", g[16], 'G');
        HCHECK("tiny cap output is a prefix of the header", strncmp(g, "saferead: ", 10) == 0);
        memset(g, 'G', sizeof g);
        n = saferead::dump(g, 1);
        HEQ("cap 1 yields the empty string", n, 0);
        HEQ("cap 1: NUL at [0], nothing past it", g[0] == 0 && g[1] == 'G', 1);
    }
    // ---- wrap: after more than RingSize failures only the last 64 are listed
    {
        HEQ("RingSize is 64 (the 8192-byte buffer at :497 is sized for it)", saferead::RingSize, 64);
        for (unsigned i = 0; i < 100; ++i) readAt(1000 + i, 0x20, 0, &g_sink);
        static char big[8192];
        int n = saferead::dump(big, sizeof big);
        int lines = 0; for (int i = 0; i < n; ++i) lines += big[i] == '\n';
        HEQ("after >64 failures dump has header + exactly 64 entry lines", lines, 65);
        HCHECK("header says showing last 64", strstr(big, "(showing last 64)") != nullptr);
        HCHECK("newest failure (site 1099) is kept", strstr(big, "site=1099 ") != nullptr);
        HCHECK("oldest kept is site 1036", strstr(big, "site=1036 ") != nullptr);
        HCHECK("site 1035 aged out of the ring", strstr(big, "site=1035 ") == nullptr);
        HCHECK("the early site=27 probe aged out", strstr(big, "site=27 ") == nullptr);
        HCHECK("the full 8192 buffer holds the 64-entry dump without truncation (:497)", n < 8191);
        HCHECK("failedProbes keeps counting past the ring size (:610 prints the total)", saferead::g_ringCount > 100);
        char hdr[64]; snprintf(hdr, sizeof hdr, "saferead: %ld failed probes", (long)saferead::g_ringCount);
        HCHECK("header total equals g_ringCount, not the ring size", strncmp(big, hdr, strlen(hdr)) == 0);
        char sm[300]; int ns = saferead::dump(sm, 300);
        HEQ("small cap after wrap still returns strlen", ns, (int)strlen(sm));
        HCHECK("small cap after wrap stays in bounds", ns <= 299);
    }
    // ---- codepatch::patchIat: the five file-I/O IAT hooks (LoadHangProbe.cpp CreateFileA/W, ReadFile, SetFilePointer, CloseHandle)
    {
        codepatch::Ops savedOps = codepatch::ops();
        codepatch::ops() = { cpfake::query, cpfake::protect, cpfake::flush };
        buildImage("KERNEL32.dll", "USER32.dll", true);
        HMODULE mod = g_img;
        void* orig = nullptr;
        void* const hook = reinterpret_cast<void*>(static_cast<uintptr_t>(0x12345678));
        cpfake::reset();
        HCHECK("patchIat swaps the named IAT slot, matching the dll name case-insensitively (\"kernel32.DLL\" vs \"KERNEL32.dll\")", codepatch::patchIat(mod, "kernel32.DLL", "ReadFile", hook, &orig));
        HEQ("patchIat writes the hook into the named slot", get32(slotG(1)), 0x12345678u);
        HEQ("patchIat returns the old slot value in *orig (rReadFile is called through it)", (unsigned long long)reinterpret_cast<uintptr_t>(orig), 0x22220001ull);
        HCHECK("patchIat leaves neighbouring slots (other hooked imports) and the ordinal entry alone", get32(slotG(0)) == 0x22220000u && get32(slotG(2)) == 0x22220002u && get32(FT_G) == 0x11110000u);
        HEQ("patchIat does not touch the other dll's same-named slot", get32(slotD(1)), 0x55550001u);
        HCHECK("patchIat opens just the 4-byte slot RW and restores the previous protection", cpfake::protN == 2 && cpfake::prot[0].n == 4 && cpfake::prot[0].newProt == PAGE_READWRITE && cpfake::prot[1].newProt == PAGE_EXECUTE_READ);
        void* o2 = nullptr;
        HCHECK("a second patchIat on another import of the same dll works (the mod hooks five in a row)", codepatch::patchIat(mod, "KERNEL32.dll", FN[2], hook, &o2));
        HEQ("the second hook's *orig is the second slot's old value", (unsigned long long)reinterpret_cast<uintptr_t>(o2), 0x22220002ull);
        HEQ("the first hooked slot is unchanged by the second call", get32(slotG(1)), 0x12345678u);
        void* o3 = reinterpret_cast<void*>(0x1);
        HCHECK("patchIat is false for a function the dll does not import, *orig untouched (the mod then keeps its r* pointer null)", !codepatch::patchIat(mod, "KERNEL32.dll", "NotImported", hook, &o3) && o3 == reinterpret_cast<void*>(0x1));
        HCHECK("patchIat is false for a dll the exe does not import", !codepatch::patchIat(mod, "OLE32.dll", "ReadFile", hook, &o3));
        buildImage("KERNEL32.dll", "USER32.dll", true);
        cpfake::reset(); cpfake::protFail = true;
        HCHECK("patchIat is false when protect fails (that hook is simply not installed), *orig untouched", !codepatch::patchIat(mod, "KERNEL32.dll", "ReadFile", hook, &o3) && o3 == reinterpret_cast<void*>(0x1));
        HEQ("patchIat leaves the slot untouched when protect fails", get32(slotG(1)), 0x22220001u);
        cpfake::protFail = false;
        buildImage("KERNEL32.dll", "USER32.dll", false);
        HCHECK("patchIat is false for an image without an import directory", !codepatch::patchIat(mod, "KERNEL32.dll", "ReadFile", hook, &o3));
        codepatch::ops() = savedOps;
    }
    return kt::Report("common_contract_test (load_hang_probe)");
}
