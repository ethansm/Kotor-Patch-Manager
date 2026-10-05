// common_contract_test.cpp -- Patches/Common CONTRACT test for Texture Memory Release (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on in CodePatch.h (committed, putRel32, write), not the header's whole behaviour (that is _shared_src/codepatch_test.cpp). Every message starts
// with "texture_memory_release relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// CodePatch.h re-runs this test. Host-only: codepatch::ops() is replaced with fakes that record every Win32-level call; "code" is a
// host buffer, rel32 targets are fake 32-bit addresses (never dereferenced).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <initializer_list>
#include "CodePatch.h"
#include "IniKit.h"
#include "check.h"
#include "KpLog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

#define CCHECK(why, expr) ::kt::Check("texture_memory_release relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define CEQ(why, a, b) ::kt::CheckEq("texture_memory_release relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("k2-texture-memory-release");
KP_CHANNEL(chMain, "main");

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
        const uint8_t repl[7] = { 0xE9, 1, 2, 3, 4, 0x90, 0x90 };
        reset();
        int r = codepatch::write(A(code + 1), repl, 7);
        CEQ("write returns 1 on success", r, 1);
        CCHECK("write copies the bytes at the site and nowhere else", memcmp(code + 1, repl, 7) == 0 && code[0] == 1 && code[1 + 7] == (uint8_t)(2 + 7));
        checkWriteProtocol(code + 1, 7, true, 2);
        uint8_t keep[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
        reset(); g_protFail = true;
        CEQ("write returns -2 when VirtualProtect fails", codepatch::write(A(keep), repl, 7), -2);
        const uint8_t same[8] = { 9, 9, 9, 9, 9, 9, 9, 9 };
        CCHECK("write leaves the bytes untouched when protect fails", memcmp(keep, same, 8) == 0);
        CCHECK("write makes no restore call and no flush when protect fails", g_protN == 1 && g_flushN == 0);
    }

    // ---- putRel32 as the cave builder uses it: field at `at` inside the cave, displacement relative to that same address
    {
        uint8_t cave[16]; memset(cave, 0xCC, sizeof cave);
        // The mod passes at == reinterpret_cast<uintptr_t>(at); on the host that is a 64-bit pointer, so pin the identity via the
        // fieldSite argument: the displacement depends ONLY on fieldSite and target, and is stored little-endian at `field`.
        codepatch::putRel32(cave + 4, 0x2000, 0x4261D6);
        CEQ("putRel32 = target - (fieldSite + 4)", rd32(cave + 4), (int32_t)(0x4261D6u - 0x2004u));
        CCHECK("putRel32 writes exactly the 4 bytes at `field`", cave[3] == 0xCC && cave[8] == 0xCC);
        // cave jump layout: E9 at c[23], field at c+24: a jmp from cave to TxFree/TxSkip is relative to the end of the field
        codepatch::putRel32(cave, 0x1000 + 24, 0x1000 + 24 + 4 + 7);
        CEQ("putRel32 forward jump of 7 bytes past the field end encodes +7", rd32(cave), 7);
        codepatch::putRel32(cave, 0x1000 + 24, 0x1000 + 24 + 4);
        CEQ("putRel32 target == end of field encodes 0", rd32(cave), 0);
    }

    // ---- inikit::enabledFlag / enabledFile: the texmem.txt kill switch (key "enabled"; missing key/file = on; last matching line wins)
    {
        CCHECK("enabledFlag(\"enabled=0\") is off", !inikit::enabledFlag("enabled=0"));
        CCHECK("enabledFlag with a missing key is on", inikit::enabledFlag("other=1"));
        CCHECK("enabledFlag: the last matching line wins", inikit::enabledFlag("enabled=0\nenabled=1"));
        CCHECK("enabledFlag skips leading tab, spaces and '=' before the value", !inikit::enabledFlag("\tenabled = 0"));
        CCHECK("enabledFile on a missing file is on", inikit::enabledFile("/nonexistent/texmem_missing.txt"));
    }

    // ---- jmpRel / allocCave as the site jump and cave allocation use them
    {
        uint8_t out[7]; memset(out, 0xCC, sizeof out);
        codepatch::jmpRel(0x1000, 0x2000, out, 7);
        CCHECK("jmpRel starts with E9", out[0] == 0xE9);
        CEQ("jmpRel rel32 = target - (at + 5), little-endian", rd32(out + 1), (int32_t)(0x2000 - 0x1005));
        CCHECK("jmpRel pads the 7-byte site with two 0x90", out[5] == 0x90 && out[6] == 0x90);
        uint8_t* cv = codepatch::allocCave(64);
        CCHECK("allocCave(64) returns non-null", cv != nullptr);
        bool zero = cv != nullptr; for (int i = 0; cv && i < 64; ++i) if (cv[i] != 0) zero = false;
        CCHECK("allocCave(64) is zero-filled", zero);
        free(cv);   // host build: calloc-backed
    }

    // ---- KpLog fallback sink (before any KpLog_Init; the mod sets it from a static initialiser): an INFO line lands in the
    // fallback file with the "<ms> <id>/main " prefix, and the file is only created by the first line (SetFallbackFile does no I/O).
    {
        const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
        char path[300]; snprintf(path, sizeof path, "%s/texmem_contract_%d.txt", td, (int)getpid());
        remove(path);
        CCHECK("KpLog_HasCore() is false before KPatchInit (old-core path: emit from the first hook call / helper tick)", !KpLog_HasCore());
        KpLog_SetFallbackFile(path); KpLog_SetFallbackLevel(KPLOG_INFO);
        CCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)", access(path, F_OK) != 0);
        CCHECK("fallback level INFO enables INFO on the main channel (the load line is INFO)", KP_ENABLED(chMain, KPLOG_INFO));
        KP_LOGV(chMain, KPLOG_INFO, "%s", "k2-texture-memory-release 1.1.0 loaded: probe tick=1");
        std::string t; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) t.append(b, n); fclose(f); }
        size_t sp = t.find(' ');
        CCHECK("fallback line starts with a decimal tick count", sp != std::string::npos && sp > 0 && t.find_first_not_of("0123456789") == sp);
        CCHECK("fallback line has the prefix \"<ms> k2-texture-memory-release/main \" then the text unchanged", t.substr(sp) == " k2-texture-memory-release/main k2-texture-memory-release 1.1.0 loaded: probe tick=1\n");
        KP_LOGV(chMain, KPLOG_WARN, "%s", "REFUSED probe");
        std::string u; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) u.append(b, n); fclose(f); }
        CCHECK("a WARN line carries the WARN tag (REFUSED load lines)", u.find(" k2-texture-memory-release/main WARN REFUSED probe\n") != std::string::npos);
        KpLog_SetFallbackFile(nullptr);
        remove(path);
    }

    return kt::Report("common_contract_test (texture_memory_release)");
}
