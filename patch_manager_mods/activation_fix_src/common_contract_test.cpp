// common_contract_test.cpp -- Patches/Common CONTRACT test for Activation Fix (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on in CodePatch.h (committed, writeChecked, putRel32), not the header's whole behaviour (that is _shared_src/codepatch_test.cpp). Every message starts
// with "activation_fix relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml), so ANY edit to
// CodePatch.h re-runs this test. Host-only: codepatch::ops() is replaced with fakes that record every Win32-level call; "code" is a
// host buffer, rel32 targets are fake 32-bit addresses (never dereferenced).
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <initializer_list>
#include "CodePatch.h"
#include "check.h"
#include "KpLog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>

#define CCHECK(why, expr) ::kt::Check("activation_fix relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define CEQ(why, a, b) ::kt::CheckEq("activation_fix relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("k2-activation-fix");
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

    // ---- ActivationFix.cpp writeSite == `if (!committed) return -3; return writeChecked(...)`; pin the pieces and the composition
    {
        static uint8_t code[16] = { 0xFF, 0x15, 0xAA, 0xBB, 0xCC, 0xDD, 0x90, 0x90 };
        const uint8_t orig[6] = { 0xFF, 0x15, 0xAA, 0xBB, 0xCC, 0xDD };
        const uint8_t repl[6] = { 0xE8, 1, 2, 3, 4, 0x90 };
        uint8_t seen[6];
        auto writeSite = [&](uintptr_t at) {   // the mod's wrapper, verbatim
            if (!codepatch::committed(at, 6)) return -3;
            return codepatch::writeChecked(at, orig, repl, 6, seen);
        };
        g_vqBase = A(code); g_vqSize = sizeof code; reset();
        memset(seen, 0xEE, sizeof seen);
        CEQ("writeChecked returns 1 when the bytes match", writeSite(A(code)), 1);
        CCHECK("writeChecked fills `seen` with the bytes found before writing", memcmp(seen, orig, 6) == 0);
        CCHECK("writeChecked writes repl over the site", memcmp(code, repl, 6) == 0 && code[6] == 0x90);
        checkWriteProtocol(code, 6, true, 0xFF);

        // second attempt: the site now holds repl, so expect no longer matches -> -1, `seen` = what is there, nothing written, no protect
        uint8_t before[16]; memcpy(before, code, sizeof code);
        reset(); memset(seen, 0xEE, sizeof seen);
        CEQ("writeChecked returns -1 on a byte mismatch (perf-lab already patched the site)", writeSite(A(code)), -1);
        CCHECK("writeChecked fills `seen` even on mismatch (the REFUSED log prints it)", memcmp(seen, repl, 6) == 0);
        CCHECK("writeChecked does not touch protection or bytes on mismatch", g_protN == 0 && g_flushN == 0 && memcmp(code, before, sizeof code) == 0);

        // protect failure after a match: -2 with `seen` already filled
        memcpy(code, orig, 6); reset(); g_protFail = true; memset(seen, 0xEE, sizeof seen);
        CEQ("writeChecked returns -2 when VirtualProtect fails", writeSite(A(code)), -2);
        CCHECK("writeChecked leaves the bytes untouched when protect fails", memcmp(code, orig, 6) == 0 && g_flushN == 0);
        CCHECK("writeChecked has filled `seen` before the protect failure", memcmp(seen, orig, 6) == 0);

        // wrapper: -3 comes from committed() BEFORE the site is read or protected
        reset(); g_vqMode = 2; memset(seen, 0xEE, sizeof seen);
        CEQ("the committed()-first wrapper returns -3 on a non-committed page", writeSite(A(code)), -3);
        CCHECK("the -3 path reads nothing, protects nothing, writes nothing", seen[0] == 0xEE && g_protN == 0 && g_flushN == 0 && memcmp(code, orig, 6) == 0);
        reset();
        g_vqSize = 3;   // region ends inside the 6-byte span: stricter than the old state-only check, still -3 (real sites are far inside .text)
        CEQ("a span running past the region end is -3, not a partial write", writeSite(A(code)), -3);
        g_vqSize = sizeof code;
    }

    // ---- putRel32: displacement is computed for the FIELD SITE in the game, not for the staging buffer it is built in
    {
        uint8_t repl[6] = { 0xE8, 0, 0, 0, 0, 0x90 };
        codepatch::putRel32(repl + 1, 0x40D69F, 0x10002000);   // SitePost + 1 (ActivationFix.cpp)
        CEQ("putRel32 = target - (fieldSite + 4), independent of the staging address", rd32(repl + 1), (int32_t)(0x10002000u - 0x40D6A3u));
        CCHECK("putRel32 writes exactly the 4 field bytes", repl[0] == 0xE8 && repl[5] == 0x90);
        uint8_t other[4];
        codepatch::putRel32(other, 0x40D69F, 0x10002000);
        CCHECK("putRel32 gives the same field for a different staging buffer", memcmp(other, repl + 1, 4) == 0);
        codepatch::putRel32(other, 0x40D69F, 0x40D000);
        CEQ("putRel32 backward target is negative", rd32(other), 0x40D000 - 0x40D6A3);
    }

    // ---- KpLog fallback sink (before any KpLog_Init; the mod sets it from a static initialiser): an INFO line lands in the
    // fallback file with the "<ms> <id>/main " prefix, and the file is only created by the first line (SetFallbackFile does no I/O).
    {
        const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
        char path[300]; snprintf(path, sizeof path, "%s/activationfix_contract_%d.txt", td, (int)getpid());
        remove(path);
        CCHECK("KpLog_HasCore() is false before KPatchInit (old-core path: emit from the first hook call / helper tick)", !KpLog_HasCore());
        KpLog_SetFallbackFile(path); KpLog_SetFallbackLevel(KPLOG_INFO);
        CCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)", access(path, F_OK) != 0);
        CCHECK("fallback level INFO enables INFO on the main channel (the load line is INFO)", KP_ENABLED(chMain, KPLOG_INFO));
        KP_LOGV(chMain, KPLOG_INFO, "%s", "k2-activation-fix 1.1.0 loaded: probe tick=1");
        std::string t; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) t.append(b, n); fclose(f); }
        size_t sp = t.find(' ');
        CCHECK("fallback line starts with a decimal tick count", sp != std::string::npos && sp > 0 && t.find_first_not_of("0123456789") == sp);
        CCHECK("fallback line has the prefix \"<ms> k2-activation-fix/main \" then the text unchanged", t.substr(sp) == " k2-activation-fix/main k2-activation-fix 1.1.0 loaded: probe tick=1\n");
        KP_LOGV(chMain, KPLOG_WARN, "%s", "REFUSED probe");
        std::string u; if (FILE* f = fopen(path, "r")) { char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) u.append(b, n); fclose(f); }
        CCHECK("a WARN line carries the WARN tag (REFUSED load lines)", u.find(" k2-activation-fix/main WARN REFUSED probe\n") != std::string::npos);
        KpLog_SetFallbackFile(nullptr);
        remove(path);
    }

    return kt::Report("common_contract_test (activation_fix)");
}
