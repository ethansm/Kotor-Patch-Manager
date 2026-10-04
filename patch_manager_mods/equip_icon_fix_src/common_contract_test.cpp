// common_contract_test.cpp -- Patches/Common CONTRACT test for Equip Icon Fix (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on in CodePatch.h (committed, callRel, write), not the header's whole behaviour (that is _shared_src/codepatch_test.cpp). Every message starts
// with "equip_icon_fix relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// CodePatch.h re-runs this test. Host-only: codepatch::ops() is replaced with fakes that record every Win32-level call; "code" is a
// host buffer, rel32 targets are fake 32-bit addresses (never dereferenced).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <initializer_list>
#include "CodePatch.h"
#include "check.h"

#define CCHECK(why, expr) ::kt::Check("equip_icon_fix relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define CEQ(why, a, b) ::kt::CheckEq("equip_icon_fix relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

namespace {
struct ProtCall { void* p; SIZE_T n; DWORD newProt; uint8_t firstByte; int seq; };
ProtCall g_prot[8]; int g_protN = 0;
struct FlushCall { const void* p; SIZE_T n; int seq; };
FlushCall g_flush[8]; int g_flushN = 0;
int g_seq = 0; bool g_protFail = false;
DWORD g_oldProt = PAGE_EXECUTE_READ;
int g_vqMode = 1;   // 0 query fails, 1 committed, 2 reserved
uintptr_t g_vqBase = 0; SIZE_T g_vqSize = 0;

BOOL fakeProtect(LPVOID p, SIZE_T n, DWORD np, PDWORD old) {
    if (g_protN < 8) g_prot[g_protN] = { p, n, np, *static_cast<uint8_t*>(p), g_seq };
    ++g_protN; ++g_seq;
    if (g_protFail) return 0;
    *old = g_oldProt;
    return 1;
}
void fakeFlush(const void* p, SIZE_T n) { if (g_flushN < 8) g_flush[g_flushN] = { p, n, g_seq }; ++g_flushN; ++g_seq; }
SIZE_T fakeQuery(LPCVOID, MEMORY_BASIC_INFORMATION* m, SIZE_T) {
    if (g_vqMode == 0) return 0;
    m->BaseAddress = reinterpret_cast<void*>(g_vqBase); m->RegionSize = g_vqSize;
    m->State = g_vqMode == 1 ? MEM_COMMIT : 0x2000; m->Protect = PAGE_READWRITE;
    return sizeof *m;
}
void reset() { g_protN = g_flushN = g_seq = 0; g_protFail = false; g_oldProt = PAGE_EXECUTE_READ; g_vqMode = 1; }
uintptr_t A(const void* p) { return reinterpret_cast<uintptr_t>(p); }
int32_t rd32(const uint8_t* p) { int32_t v; memcpy(&v, p, 4); return v; }

// The write protocol every mod here depends on: protect(RWX) -> copy -> restore the OLD protection -> flush exactly the patched range.
void checkWriteProtocol(const uint8_t* at, int n, bool isFirstByteOld, uint8_t oldFirst) {
    CEQ("write protects the range exactly twice", g_protN, 2);
    CCHECK("write makes the range PAGE_EXECUTE_READWRITE first", g_prot[0].p == at && g_prot[0].n == (SIZE_T)n && g_prot[0].newProt == PAGE_EXECUTE_READWRITE);
    CEQ("write restores the previous protection verbatim", g_prot[1].newProt, (DWORD)PAGE_EXECUTE_READ);
    CCHECK("write has not copied yet at the first protect", !isFirstByteOld || g_prot[0].firstByte == oldFirst);
    CEQ("write flushes once", g_flushN, 1);
    CCHECK("write flushes exactly the patched range, after the restore", g_flush[0].p == at && g_flush[0].n == (SIZE_T)n && g_flush[0].seq > g_prot[1].seq);
}
}  // namespace

int main() {
    codepatch::ops() = { fakeQuery, fakeProtect, fakeFlush };

    // ---- committed: the pre-write page probe (a site is only touched when its WHOLE span is inside one committed region)
    {
        static uint8_t buf[64];
        g_vqBase = A(buf); g_vqSize = sizeof buf; reset();
        CCHECK("committed accepts a span inside a committed region", codepatch::committed(A(buf), sizeof buf));
        CCHECK("committed accepts the last byte of the region", codepatch::committed(A(buf) + sizeof buf - 1, 1));
        CCHECK("committed rejects a span that runs past the region end", !codepatch::committed(A(buf) + sizeof buf - 4, 5));
        g_vqMode = 2;
        CCHECK("committed rejects a reserved (non-MEM_COMMIT) region", !codepatch::committed(A(buf), 4));
        g_vqMode = 0;
        CCHECK("committed rejects an address VirtualQuery fails on", !codepatch::committed(A(buf), 4));
        g_vqMode = 1;
    }

    // ---- write: protect(RWX), copy, restore old protection, flush; -2 leaves the bytes untouched
    {
        uint8_t code[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
        const uint8_t repl[5] = { 0xE8, 1, 2, 3, 4 };
        reset();
        int r = codepatch::write(A(code + 1), repl, 5);
        CEQ("write returns 1 on success", r, 1);
        CCHECK("write copies the bytes at the site and nowhere else", memcmp(code + 1, repl, 5) == 0 && code[0] == 1 && code[1 + 5] == (uint8_t)(2 + 5));
        checkWriteProtocol(code + 1, 5, true, 2);
        uint8_t keep[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
        reset(); g_protFail = true;
        CEQ("write returns -2 when VirtualProtect fails", codepatch::write(A(keep), repl, 5), -2);
        const uint8_t same[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
        CCHECK("write leaves the bytes untouched when protect fails", memcmp(keep, same, 8) == 0);
        CCHECK("write makes no restore call and no flush when protect fails", g_protN == 1 && g_flushN == 0);
    }

    // ---- callRel: E8 rel32 against the SITE address; DllMain compares it with the bytes found, so the encoding must be exact
    {
        uint8_t out[6]; memset(out, 0xCC, sizeof out);
        codepatch::callRel(0x8ABA8C, 0x10001000, out);
        CEQ("callRel opcode is E8", out[0], 0xE8);
        CEQ("callRel rel32 = target - (at + 5)", rd32(out + 1), (int32_t)(0x10001000u - 0x8ABA91u));
        CEQ("callRel writes exactly 5 bytes", out[5], 0xCC);
        uint8_t back[5];
        codepatch::callRel(0x8AE793, 0x8AD930, back);
        CEQ("callRel rel32 is negative for a backward target", rd32(back + 1), 0x8AD930 - 0x8AE798);
        // The "already patched?" compare in DllMain: a call assembled for the original target equals the same bytes re-assembled.
        uint8_t again[5];
        codepatch::callRel(0x8AE793, 0x8AD930, again);
        CCHECK("callRel is deterministic (DllMain compares assembled call bytes with the bytes found)", memcmp(back, again, 5) == 0);
    }

    return kt::Report("common_contract_test (equip_icon_fix)");
}
