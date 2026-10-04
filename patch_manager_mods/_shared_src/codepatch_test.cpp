// Host test of codepatch (CodePatch.h): write / writeChecked / committed / rel32 builders / patchIat, with injected fake ops
// recording every call and a real host buffer standing in for code. Windows is the stub windows.h (-DGUIKIT_HOST).
#include <windows.h>
#include <initializer_list>
#include "CodePatch.h"
#include "check.h"

// ---- fake ops ---------------------------------------------------------------------------------------------------------
struct ProtCall { void* p; SIZE_T n; DWORD newProt; uint8_t firstByte; int seq; };
struct FlushCall { const void* p; SIZE_T n; int seq; };
static ProtCall g_prot[8]; static int g_protN = 0;
static FlushCall g_flush[8]; static int g_flushN = 0;
static int g_seq = 0;
static bool g_protFail = false;
static DWORD g_oldProt = PAGE_EXECUTE_READ;     // what the fake reports as the previous protection

static BOOL fakeProtect(LPVOID p, SIZE_T n, DWORD np, PDWORD old) {
    if (g_protN < 8) g_prot[g_protN] = { p, n, np, *static_cast<uint8_t*>(p), g_seq };
    ++g_protN; ++g_seq;
    if (g_protFail) return 0;
    *old = g_oldProt;
    return 1;
}
static void fakeFlush(const void* p, SIZE_T n) { if (g_flushN < 8) g_flush[g_flushN] = { p, n, g_seq }; ++g_flushN; ++g_seq; }

static int g_vqMode = 0;                         // 0 fail, 1 committed, 2 reserved (non-commit)
static uintptr_t g_vqBase = 0; static SIZE_T g_vqSize = 0;
static SIZE_T fakeQuery(LPCVOID, MEMORY_BASIC_INFORMATION* m, SIZE_T) {
    if (g_vqMode == 0) return 0;
    m->BaseAddress = reinterpret_cast<void*>(g_vqBase); m->RegionSize = g_vqSize;
    m->State = g_vqMode == 1 ? MEM_COMMIT : 0x2000; m->Protect = PAGE_READWRITE;
    return sizeof *m;
}
static void reset() { g_protN = g_flushN = g_seq = 0; g_protFail = false; g_oldProt = PAGE_EXECUTE_READ; }

// ---- in-memory PE32 image (offsets written by hand, independent of CodePatch.h's structs) -------------------------------
static uint8_t g_img[0x1000] __attribute__((aligned(16)));
static void put32(uint32_t off, uint32_t v) { std::memcpy(g_img + off, &v, 4); }
static void putStr(uint32_t off, const char* s) { std::memcpy(g_img + off, s, std::strlen(s) + 1); }
static uint32_t get32(uint32_t off) { uint32_t v; std::memcpy(&v, g_img + off, 4); return v; }

constexpr uint32_t E_LFANEW = 0x80, DIR_IMPORT = E_LFANEW + 24 + 96 + 8;     // PE sig(4)+file hdr(20)+opt hdr data dirs at +96; dir 1 = import
constexpr uint32_t DESC0 = 0x200, DESC1 = DESC0 + 20;                         // desc0 = USER32.dll (decoy), desc1 = kernel32.dll
constexpr uint32_t OFT_K = 0x400, OFT_U = 0x420, FT_K = 0x500, FT_U = 0x520;
constexpr uint32_t N_SLEEP = 0x340, N_CLOSE = 0x360, N_ONLYUSER = 0x380, N_CLOSE_U = 0x3A0;
constexpr uint32_t ORDINAL_FLAG = 0x80000000u;
constexpr uint32_t V_ORD = 0x11111111, V_SLEEP = 0x22222222, V_CLOSE = 0x33333333, V_UCLOSE = 0x55555555, V_ONLYUSER = 0x44444444;

// zeroOftK: kernel32's OriginalFirstThunk is 0, so the loader (and patchIat) reads names through FirstThunk.
static void buildImage(bool zeroOftK, bool noImportDir = false) {
    std::memset(g_img, 0, sizeof g_img);
    put32(0x3C, E_LFANEW);
    std::memcpy(g_img + E_LFANEW, "PE\0\0", 4);
    if (!noImportDir) { put32(DIR_IMPORT, DESC0); put32(DIR_IMPORT + 4, 80); }
    // names: hint(2) + string
    putStr(N_SLEEP + 2, "Sleep"); putStr(N_CLOSE + 2, "CloseHandle"); putStr(N_ONLYUSER + 2, "OnlyUser"); putStr(N_CLOSE_U + 2, "CloseHandle");
    putStr(0x300, "USER32.dll"); putStr(0x320, "kernel32.dll");
    // desc0: USER32.dll { OnlyUser, CloseHandle }
    put32(DESC0 + 0, OFT_U); put32(DESC0 + 12, 0x300); put32(DESC0 + 16, FT_U);
    put32(OFT_U + 0, N_ONLYUSER); put32(OFT_U + 4, N_CLOSE_U); put32(OFT_U + 8, 0);
    put32(FT_U + 0, V_ONLYUSER); put32(FT_U + 4, V_UCLOSE); put32(FT_U + 8, 0);
    // desc1: kernel32.dll { ordinal 16, Sleep, CloseHandle }
    put32(DESC1 + 0, zeroOftK ? 0 : OFT_K); put32(DESC1 + 12, 0x320); put32(DESC1 + 16, FT_K);
    if (!zeroOftK) {
        put32(OFT_K + 0, ORDINAL_FLAG | 16); put32(OFT_K + 4, N_SLEEP); put32(OFT_K + 8, N_CLOSE); put32(OFT_K + 12, 0);
        put32(FT_K + 0, V_ORD); put32(FT_K + 4, V_SLEEP); put32(FT_K + 8, V_CLOSE); put32(FT_K + 12, 0);
    } else {   // unbound image: the IAT still holds the name RVAs
        put32(FT_K + 0, ORDINAL_FLAG | 16); put32(FT_K + 4, N_SLEEP); put32(FT_K + 8, N_CLOSE); put32(FT_K + 12, 0);
    }
    // desc2 stays all-zero: the terminator
}

int main() {
    codepatch::Ops saved = codepatch::ops();
    codepatch::ops() = { fakeQuery, fakeProtect, fakeFlush };

    // ---- write ------------------------------------------------------------------------------------------------------
    {
        uint8_t code[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        const uint8_t repl[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
        reset();
        int r = codepatch::write(reinterpret_cast<uintptr_t>(code + 2), repl, 4);
        KT_CHECK_EQ(r, 1);
        const uint8_t want[8] = { 1, 2, 0xAA, 0xBB, 0xCC, 0xDD, 7, 8 };
        KT_CHECK(std::memcmp(code, want, 8) == 0);
        KT_CHECK_EQ(g_protN, 2);
        KT_CHECK(g_prot[0].p == code + 2 && g_prot[0].n == 4 && g_prot[0].newProt == PAGE_EXECUTE_READWRITE);
        KT_CHECK(g_prot[1].p == code + 2 && g_prot[1].n == 4);
        KT_CHECK_EQ(g_prot[1].newProt, static_cast<DWORD>(PAGE_EXECUTE_READ));      // restores the first call's old value
        KT_CHECK_EQ(g_prot[0].firstByte, 3);                                        // not yet written at the first protect
        KT_CHECK_EQ(g_prot[1].firstByte, 0xAA);                                     // copied before the restore
        KT_CHECK_EQ(g_flushN, 1);
        KT_CHECK(g_flush[0].p == code + 2 && g_flush[0].n == 4);
        KT_CHECK(g_flush[0].seq > g_prot[1].seq);                                   // flush last
        // a different old protection is restored verbatim
        reset(); g_oldProt = PAGE_READONLY;
        KT_CHECK_EQ(codepatch::write(reinterpret_cast<uintptr_t>(code), repl, 1), 1);
        KT_CHECK_EQ(g_prot[1].newProt, static_cast<DWORD>(PAGE_READONLY));
        // protect failure: -2, bytes untouched, no restore call, no flush
        uint8_t code2[4] = { 9, 9, 9, 9 };
        reset(); g_protFail = true;
        KT_CHECK_EQ(codepatch::write(reinterpret_cast<uintptr_t>(code2), repl, 4), -2);
        const uint8_t same[4] = { 9, 9, 9, 9 };
        KT_CHECK(std::memcmp(code2, same, 4) == 0);
        KT_CHECK_EQ(g_protN, 1); KT_CHECK_EQ(g_flushN, 0);
    }

    // ---- writeChecked -----------------------------------------------------------------------------------------------
    {
        uint8_t code[8] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10, 0x90 };
        const uint8_t orig[7] = { 0x0F, 0xB6, 0xC8, 0x85, 0xC9, 0x75, 0x10 };
        const uint8_t repl[7] = { 0xE9, 1, 2, 3, 4, 0x90, 0x90 };
        uint8_t seen[7];
        reset(); std::memset(seen, 0xEE, sizeof seen);
        KT_CHECK_EQ(codepatch::writeChecked(reinterpret_cast<uintptr_t>(code), orig, repl, 7, seen), 1);
        KT_CHECK(std::memcmp(seen, orig, 7) == 0);                                   // seen = bytes before the write
        KT_CHECK(std::memcmp(code, repl, 7) == 0); KT_CHECK_EQ(code[7], 0x90);
        KT_CHECK_EQ(g_protN, 2); KT_CHECK_EQ(g_flushN, 1);
        KT_CHECK(g_prot[0].newProt == PAGE_EXECUTE_READWRITE);
        KT_CHECK_EQ(g_prot[1].newProt, static_cast<DWORD>(PAGE_EXECUTE_READ));
        KT_CHECK(g_flush[0].p == code && g_flush[0].n == 7);
        // mismatch: now `code` holds repl, expect orig again -> -1
        uint8_t before[8]; std::memcpy(before, code, 8);
        reset(); std::memset(seen, 0xEE, sizeof seen);
        KT_CHECK_EQ(codepatch::writeChecked(reinterpret_cast<uintptr_t>(code), orig, repl, 7, seen), -1);
        KT_CHECK(std::memcmp(seen, before, 7) == 0);                                 // seen == the original (found) bytes
        KT_CHECK(std::memcmp(code, before, 8) == 0);                                 // untouched
        KT_CHECK_EQ(g_protN, 0); KT_CHECK_EQ(g_flushN, 0);                           // protect never called
        // a one-byte difference at the very end is still a mismatch
        uint8_t c2[7]; std::memcpy(c2, orig, 7); c2[6] ^= 1;
        reset();
        KT_CHECK_EQ(codepatch::writeChecked(reinterpret_cast<uintptr_t>(c2), orig, repl, 7, seen), -1);
        KT_CHECK_EQ(g_protN, 0);
        // protect failure on a matching site: -2, seen filled, untouched
        uint8_t c3[7]; std::memcpy(c3, orig, 7);
        reset(); g_protFail = true; std::memset(seen, 0xEE, sizeof seen);
        KT_CHECK_EQ(codepatch::writeChecked(reinterpret_cast<uintptr_t>(c3), orig, repl, 7, seen), -2);
        KT_CHECK(std::memcmp(seen, orig, 7) == 0);
        KT_CHECK(std::memcmp(c3, orig, 7) == 0);
        KT_CHECK_EQ(g_protN, 1); KT_CHECK_EQ(g_flushN, 0);
    }

    // ---- committed --------------------------------------------------------------------------------------------------
    {
        g_vqBase = 0x400000; g_vqSize = 0x1000;
        g_vqMode = 0;
        KT_CHECK(!codepatch::committed(0x400100, 4));                                // query fails
        g_vqMode = 2;
        KT_CHECK(!codepatch::committed(0x400100, 4));                                // reserved, not committed
        g_vqMode = 1;
        KT_CHECK(codepatch::committed(0x400100, 4));
        KT_CHECK(codepatch::committed(0x400000, 0x1000));                            // exact fit, whole region
        KT_CHECK(codepatch::committed(0x400FFC, 4));                                 // exact fit at the end
        KT_CHECK(!codepatch::committed(0x400FFD, 4));                                // straddles the region end by one byte
        KT_CHECK(!codepatch::committed(0x400FFC, 5));
        KT_CHECK(!codepatch::committed(0x400000, 0x1001));
        KT_CHECK(codepatch::committed(0x400FFF, 1));
        KT_CHECK(codepatch::committed(0x400100, 0));                                 // zero length inside
    }

    // ---- putRel32 / jmpRel / callRel --------------------------------------------------------------------------------
    {
        uint8_t f[4];
        codepatch::putRel32(f, 0x1000, 0x1100);                                      // forward: 0x1100 - 0x1004
        KT_CHECK_EQ(f[0], 0xFC); KT_CHECK_EQ(f[1], 0x00); KT_CHECK_EQ(f[2], 0); KT_CHECK_EQ(f[3], 0);
        codepatch::putRel32(f, 0x2000, 0x1000);                                      // backward: 0x1000 - 0x2004 = -0x1004
        KT_CHECK_EQ(f[0], 0xFC); KT_CHECK_EQ(f[1], 0xEF); KT_CHECK_EQ(f[2], 0xFF); KT_CHECK_EQ(f[3], 0xFF);
        codepatch::putRel32(f, 0x1000, 0x1004);                                      // zero displacement
        KT_CHECK_EQ(f[0], 0); KT_CHECK_EQ(f[1], 0); KT_CHECK_EQ(f[2], 0); KT_CHECK_EQ(f[3], 0);
        // the staging buffer's own address is irrelevant: only fieldSite counts
        uint8_t g[8] = { 0 };
        codepatch::putRel32(g + 3, 0x4261D7, 0x4261E6);                              // 0x4261E6 - 0x4261DB = 0xB
        KT_CHECK_EQ(g[3], 0x0B); KT_CHECK_EQ(g[2], 0); KT_CHECK_EQ(g[7], 0);

        uint8_t j[7]; std::memset(j, 0xCC, sizeof j);
        codepatch::jmpRel(0x4261CF, 0x426300, j, 7);                                 // forward, padded: 0x426300 - 0x4261D4 = 0x12C
        KT_CHECK_EQ(j[0], 0xE9); KT_CHECK_EQ(j[1], 0x2C); KT_CHECK_EQ(j[2], 0x01); KT_CHECK_EQ(j[3], 0); KT_CHECK_EQ(j[4], 0);
        KT_CHECK_EQ(j[5], 0x90); KT_CHECK_EQ(j[6], 0x90);
        uint8_t j5[6]; std::memset(j5, 0xCC, sizeof j5);
        codepatch::jmpRel(0x5000, 0x4000, j5, 5);                                    // backward, n == 5: no pad written
        KT_CHECK_EQ(j5[0], 0xE9); KT_CHECK_EQ(j5[1], 0xFB); KT_CHECK_EQ(j5[2], 0xEF); KT_CHECK_EQ(j5[3], 0xFF); KT_CHECK_EQ(j5[4], 0xFF);
        KT_CHECK_EQ(j5[5], 0xCC);
        uint8_t j9[9]; std::memset(j9, 0xCC, sizeof j9);
        codepatch::jmpRel(0x1000, 0x1005, j9, 9);                                    // lands right after itself: rel 0, pad 4
        KT_CHECK_EQ(j9[1], 0); KT_CHECK_EQ(j9[4], 0); KT_CHECK_EQ(j9[5], 0x90); KT_CHECK_EQ(j9[8], 0x90);

        uint8_t c[5]; std::memset(c, 0xCC, sizeof c);
        codepatch::callRel(0x1000, 0x2000, c);                                       // forward: 0x2000 - 0x1005 = 0xFFB
        KT_CHECK_EQ(c[0], 0xE8); KT_CHECK_EQ(c[1], 0xFB); KT_CHECK_EQ(c[2], 0x0F); KT_CHECK_EQ(c[3], 0); KT_CHECK_EQ(c[4], 0);
        codepatch::callRel(0x2000, 0x1000, c);                                       // backward: 0x1000 - 0x2005 = -0x1005
        KT_CHECK_EQ(c[0], 0xE8); KT_CHECK_EQ(c[1], 0xFB); KT_CHECK_EQ(c[2], 0xEF); KT_CHECK_EQ(c[3], 0xFF); KT_CHECK_EQ(c[4], 0xFF);
    }

    // ---- patchIat ---------------------------------------------------------------------------------------------------
    {
        void* const hook = reinterpret_cast<void*>(static_cast<uintptr_t>(0x12345678));
        void* orig = nullptr;
        HMODULE mod = g_img;

        // hit by name; dll name matched case-insensitively ("KERNEL32.dll" vs "kernel32.dll"); ordinal entry skipped
        buildImage(false); reset();
        KT_CHECK(codepatch::patchIat(mod, "KERNEL32.dll", "CloseHandle", hook, &orig));
        KT_CHECK_EQ(get32(FT_K + 8), 0x12345678u);
        KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(V_CLOSE));
        KT_CHECK_EQ(get32(FT_K + 0), V_ORD); KT_CHECK_EQ(get32(FT_K + 4), V_SLEEP);   // neighbours untouched
        KT_CHECK_EQ(get32(FT_U + 4), V_UCLOSE);                                       // same function name in the other dll untouched
        KT_CHECK_EQ(g_protN, 2);
        KT_CHECK(g_prot[0].p == g_img + FT_K + 8 && g_prot[0].n == 4 && g_prot[0].newProt == PAGE_READWRITE);
        KT_CHECK(g_prot[1].p == g_img + FT_K + 8 && g_prot[1].newProt == PAGE_EXECUTE_READ);   // restored to the old protection
        KT_CHECK_EQ(g_prot[0].firstByte, 0x33);                                       // swapped after the first protect, not before
        KT_CHECK_EQ(g_flushN, 0);                                                     // the IAT slot is data: no icache flush

        // the lower-case spelling and the first by-name entry (index 1, behind the ordinal)
        buildImage(false); orig = nullptr;
        KT_CHECK(codepatch::patchIat(mod, "kernel32.dll", "Sleep", hook, &orig));
        KT_CHECK_EQ(get32(FT_K + 4), 0x12345678u); KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(V_SLEEP));
        KT_CHECK_EQ(get32(FT_K + 8), V_CLOSE);

        // second descriptor walk: dll in the first descriptor
        buildImage(false); orig = nullptr;
        KT_CHECK(codepatch::patchIat(mod, "user32.DLL", "CloseHandle", hook, &orig));
        KT_CHECK_EQ(get32(FT_U + 4), 0x12345678u); KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(V_UCLOSE));
        KT_CHECK_EQ(get32(FT_K + 8), V_CLOSE);

        // OriginalFirstThunk == 0: names are read through FirstThunk (unbound image)
        buildImage(true); orig = nullptr;
        KT_CHECK(codepatch::patchIat(mod, "KERNEL32.dll", "Sleep", hook, &orig));
        KT_CHECK_EQ(get32(FT_K + 4), 0x12345678u); KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(N_SLEEP));

        // misses leave everything alone
        buildImage(false); orig = reinterpret_cast<void*>(static_cast<uintptr_t>(0xABCD)); reset();
        KT_CHECK(!codepatch::patchIat(mod, "KERNEL32.dll", "NoSuchFn", hook, &orig));       // missing function
        KT_CHECK(!codepatch::patchIat(mod, "KERNEL32.dll", "OnlyUser", hook, &orig));       // exists, but only in another dll
        KT_CHECK(!codepatch::patchIat(mod, "KERNEL32.dll", "closehandle", hook, &orig));    // function names are case-sensitive
        KT_CHECK(!codepatch::patchIat(mod, "kernel32", "CloseHandle", hook, &orig));        // dll name must match whole
        KT_CHECK(!codepatch::patchIat(mod, "ADVAPI32.dll", "CloseHandle", hook, &orig));    // missing dll
        KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(0xABCD));
        KT_CHECK_EQ(g_protN, 0);
        KT_CHECK_EQ(get32(FT_K + 8), V_CLOSE); KT_CHECK_EQ(get32(FT_U + 0), V_ONLYUSER);
        KT_CHECK(codepatch::patchIat(mod, "USER32.dll", "OnlyUser", hook, &orig));          // ... and this one is a hit
        KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(V_ONLYUSER));

        // protect failure: false, slot and *orig untouched
        buildImage(false); orig = reinterpret_cast<void*>(static_cast<uintptr_t>(0xABCD)); reset(); g_protFail = true;
        KT_CHECK(!codepatch::patchIat(mod, "KERNEL32.dll", "CloseHandle", hook, &orig));
        KT_CHECK_EQ(get32(FT_K + 8), V_CLOSE); KT_CHECK_EQ(reinterpret_cast<uintptr_t>(orig), static_cast<uintptr_t>(0xABCD));
        KT_CHECK_EQ(g_protN, 1);

        // no import directory
        buildImage(false, true); reset();
        KT_CHECK(!codepatch::patchIat(mod, "KERNEL32.dll", "CloseHandle", hook, &orig));
        KT_CHECK_EQ(g_protN, 0);
    }

    // ---- the default (host) ops work end to end against the stub's VirtualQuery ----------------------------------------------
    {
        codepatch::ops() = saved;
        g_stubVirtualQueryAll = true;
        uint8_t code[6] = { 1, 2, 3, 4, 5, 6 };
        const uint8_t repl[2] = { 0xEE, 0xFF };
        KT_CHECK(codepatch::committed(reinterpret_cast<uintptr_t>(code), sizeof code));
        KT_CHECK_EQ(codepatch::write(reinterpret_cast<uintptr_t>(code + 4), repl, 2), 1);
        KT_CHECK_EQ(code[4], 0xEE); KT_CHECK_EQ(code[5], 0xFF); KT_CHECK_EQ(code[3], 4);
        g_stubVirtualQueryAll = false;
        KT_CHECK(!codepatch::committed(reinterpret_cast<uintptr_t>(code), sizeof code));
    }

    return kt::Report("codepatch_test");
}
