// common_contract_test.cpp -- Patches/Common CONTRACT test for Quick Loot (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the headers' own behaviour. Every message starts with "quick_loot relies on:" so a failure names the consumer and the
// reason. Built against ../_shared_src and the KPM checkout's Patches/Common (see test.toml), so ANY edit to SafeRead.h / KpLog.h / GameAddr.h
// re-runs this test. Areas: (1) saferead as rd<T> uses it in QuickLoot.cpp (u8/u16/int/u32 fields, absolute globals, failures),
// (2) the GameAddr.h constants the hooks call, (3) KpLog fallback sink before init, (4) KpLog_Init rejections, (5) KpLog_Init success.
// Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "GameAddr.h"
#include "KpLog.h"
#include "QuickLootLogic.h"
#include "check.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define HCHECK(why, expr) ::kt::Check("quick_loot relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("quick_loot relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("quick-loot");   // QuickLoot.cpp (compile-time id validity)
KP_CHANNEL(chMain, "main");

// rd<T> exactly as QuickLoot.cpp (site 0x7100)
template <typename T> static bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(0x7100, base, off, out); }

// ---- fake kpatch.log service
struct Sub { int ch, lvl; std::string text; };
static std::vector<Sub> g_subs;
static std::vector<std::string> g_regs, g_chans;
static int g_enabledCalls;
static int32_t KPATCH_CALL fReg(const char* id) { g_regs.push_back(id); return 5; }
static int32_t KPATCH_CALL fChan(int32_t patch, const char* name) { g_chans.push_back(std::string(std::to_string(patch)) + ":" + name); return 100 + (int)g_chans.size(); }
static int32_t KPATCH_CALL fEn(int32_t ch, int32_t lvl) { ++g_enabledCalls; if (g_chans.size() >= 2 && ch == 100 + 2) return 0; return lvl <= KPLOG_INFO; }   // 2nd channel resolved ("raw") is off
static void KPATCH_CALL fSub(int32_t ch, int32_t lvl, const char* line, uint32_t len) { g_subs.push_back({ ch, lvl, std::string(line, len) }); }
static void KPATCH_CALL fMark(const char*) {}
static void KPATCH_CALL fTick(int32_t) {}
static uint32_t KPATCH_CALL fFrame() { return 0; }
static uint64_t KPATCH_CALL fNow() { return 0; }
static KPatchLogApi g_log = { sizeof(KPatchLogApi), fReg, fChan, fEn, fSub, fMark, fTick, fFrame, fNow };
static const void* g_service = &g_log;
static int g_requireCalls; static std::string g_reqName; static uint32_t g_reqVer;
static const void* KPATCH_CALL fRequire(const char* n, uint32_t v) { ++g_requireCalls; g_reqName = n; g_reqVer = v; return g_service; }
static KPatchApi g_api = { sizeof(KPatchApi), 1, nullptr, fRequire };

// The mod's KPatchInit (QuickLoot.cpp) is `KpLog_Init(api)`; this is that call.
static int modKPatchInit(const KPatchApi* api) { return KpLog_Init(api); }

static std::string slurp(const std::string& p) {
    std::string s; FILE* f = fopen(p.c_str(), "rb"); if (!f) return s;
    char b[512]; size_t n; while ((n = fread(b, 1, sizeof b, f)) > 0) s.append(b, n);
    fclose(f); return s;
}
static int countLines(const std::string& s) { int n = 0; for (char c : s) n += c == '\n'; return n; }

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_32BIT
    flags |= MAP_32BIT;
#endif
    constexpr size_t ArenaSize = 1u << 16;
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, flags, -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    if (static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(m)) + ArenaSize > 0xFFFFFFFFull) { printf("arena above 4GB\n"); return 5; }
    memset(m, 0, ArenaSize);
    const uintptr_t A = reinterpret_cast<uintptr_t>(m);
    g_stubVirtualQueryAll = true;

    // ================= 1. saferead via rd<T>: the container read chain of QlShowContainer / QlCloseInventory
    {
        saferead::beginScope();   // enter()
        const uintptr_t app = A + 0x100, srv = A + 0x200, plc = A + 0x400, repo = A + 0x800, arr = A + 0xC00, item = A + 0x1000;
        *reinterpret_cast<uint32_t*>(A + 0x40) = static_cast<uint32_t>(app);          // *AppGlobal
        *reinterpret_cast<uint32_t*>(app + 8) = static_cast<uint32_t>(srv);           // serverApp = *(app+8)
        uint32_t gotApp = 0, gotSrv = 0;
        HCHECK("rd(<absolute global>, 0) reads a 32-bit pointer into a uint32_t (AppGlobal)", rd<uint32_t>(A + 0x40, 0, &gotApp));
        HEQ("app pointer exact", (unsigned long long)gotApp, (unsigned long long)app);
        HCHECK("rd(app, 8) reads serverApp", rd<uint32_t>(gotApp, 8, &gotSrv));
        HEQ("serverApp exact", (unsigned long long)gotSrv, (unsigned long long)srv);

        *reinterpret_cast<uint32_t*>(plc + 0x3C4) = static_cast<uint32_t>(repo);      // Plc_Repo
        *reinterpret_cast<uint8_t*>(plc + 0x390) = 1;                                  // Plc_Open (byte)
        *reinterpret_cast<uint8_t*>(plc + 0x391) = 0xEE;                               // neighbour byte must not leak into the open flag
        *reinterpret_cast<int32_t*>(repo + 0x10) = 3;                                  // Repo_Count
        *reinterpret_cast<uint32_t*>(repo + 0x0C) = static_cast<uint32_t>(arr);        // Repo_Array
        for (int i = 0; i < 3; ++i) *reinterpret_cast<uint32_t*>(arr + 4 * i) = 0x7F000010u + i;
        *reinterpret_cast<int32_t*>(item + 0x2CC) = 17;                                // Item_Stack
        uint32_t r = 0, a = 0, id = 0; int n = -1, stack = -1; uint8_t open = 9;
        HCHECK("rd<uint32_t>(plc, 0x3C4) repository pointer", rd<uint32_t>(plc, 0x3C4, &r));
        HEQ("repository pointer exact", (unsigned long long)r, (unsigned long long)repo);
        HCHECK("rd<int>(repo, 0x10) item count", rd<int>(repo, 0x10, &n));
        HEQ("count value", n, 3);
        HCHECK("rd<uint8_t>(plc, 0x390) open flag reads exactly one byte", rd<uint8_t>(plc, 0x390, &open));
        HEQ("open flag is the byte, not the neighbour", (int)open, 1);
        HCHECK("rd<uint32_t>(repo, 0x0C) id array pointer", rd<uint32_t>(repo, 0x0C, &a));
        HCHECK("rd<uint32_t>(arr, i*4) per-item object id", rd<uint32_t>(a, 8, &id));
        HEQ("third object id", (unsigned long long)id, 0x7F000012ULL);
        HCHECK("rd<int>(item, 0x2CC) stack size", rd<int>(item, 0x2CC, &stack));
        HEQ("stack value", stack, 17);

        // the event record: *(uint16_t*)a3 (QlPlaceableEvent), a3 may be junk
        const uintptr_t rec = A + 0x1400;
        *reinterpret_cast<uint32_t*>(rec) = 0xFFFF0016u;
        uint16_t id16 = 0;
        HCHECK("rd<uint16_t>(rec, 0) reads the low half-word", rd<uint16_t>(rec, 0, &id16));
        HEQ("event id from the low 16 bits", (int)id16, 0x16);
        HEQ("short cast of the id, as the handler does", (int)static_cast<short>(id16), 0x16);

        // failures leave *out untouched and never throw
        int keep = 55;
        HCHECK("rd with base 0 fails (a3 == 0, plc == 0)", !rd(0, 0x3C4, &keep));
        HEQ("failed rd leaves *out untouched", keep, 55);
        HCHECK("rd with a tiny pointer (<0x10000) fails (junk a3)", !rd(0x20, 0, &keep) && keep == 55);
        g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("rd on memory the OS reports unreadable fails", !rd(repo, 0x10, &keep));
        g_stubVirtualQueryAll = true; saferead::beginScope();
        HCHECK("rd works again after beginScope() (enter() resets the cache)", rd(repo, 0x10, &keep) && keep == 3);
        LONG c0 = saferead::g_ringCount;
        rd(0x20, 0, &keep);
        HEQ("ring is off by default: a failed rd records nothing (mod never calls enableRing)", (long long)saferead::g_ringCount, (long long)c0);
    }

    // ================= 2. GameAddr.h constants the hooks call
    HEQ("gameaddr::AppGlobal is 0x00A1B4A4", (unsigned long long)gameaddr::AppGlobal, 0x00A1B4A4ULL);
    HEQ("gameaddr::ClientToServerObjectId is 0x0051C8B0", (unsigned long long)gameaddr::ClientToServerObjectId, 0x0051C8B0ULL);

    // ================= 3. KpLog fallback sink (before any successful init)
    HCHECK("KpLog_HasCore() is false before KPatchInit: enter() skips the START line", !KpLog_HasCore());
    const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
    char path[300]; snprintf(path, sizeof path, "%s/quickloot_contract_%d.txt", td, (int)getpid());
    remove(path);
    KpLog_SetFallbackFile(path);
    HCHECK("KpLog_SetFallbackFile only stores the path, no I/O", access(path, F_OK) != 0);
    HCHECK("fallback default level is WARN: INFO probe lines are dropped without a kplog.ini section", !KP_ENABLED(chMain, KPLOG_INFO));
    KP_LOG(chMain, "%s", "SHOW dropped");
    HCHECK("an INFO KP_LOG at the default WARN level writes no file", access(path, F_OK) != 0);
    KpLog_SetFallbackLevel(KPLOG_INFO);
    HCHECK("after SetFallbackLevel(INFO) KP_ENABLED is true", KP_ENABLED(chMain, KPLOG_INFO));
    {
        char b[160];
        qlogic::fmtClose(b, sizeof b, 1, 1800, 4, 0);
        KP_LOG(chMain, "%s", b);   // logLine()
    }
    {
        std::string s = slurp(path);
        HEQ("the %s-forwarded probe line is written: one line", countLines(s), 1);
        HCHECK("fallback line has the patch id and channel \"quick-loot/main\"", s.find(" quick-loot/main ") != std::string::npos);
        HCHECK("the formatted CLOSE text arrives unchanged", s.find("CLOSE flag=1 dt=1800ms n=4 open=0\n") != std::string::npos);
    }
    KpLog_SetFallbackFile(nullptr);
    remove(path);

    // ================= 4. KpLog_Init rejections
    HEQ("KpLog_Init(nullptr) returns 0 (old core: KPatchInit gets no api)", KpLog_Init(nullptr), 0);
    { KPatchApi noReq = { sizeof(KPatchApi), 1, nullptr, nullptr };
      HEQ("KpLog_Init with a null require returns 0", KpLog_Init(&noReq), 0); }
    g_service = nullptr;
    HEQ("KpLog_Init with the log service missing returns 0", modKPatchInit(&g_api), 0);
    HCHECK("it asked for the \"kpatch.log\" interface at version 1", g_reqName == "kpatch.log" && g_reqVer == 1u);
    g_service = &g_log;
    { KPatchLogApi nf = g_log; nf.Submit = nullptr; g_service = &nf;
      HEQ("a service with a null function pointer is rejected", KpLog_Init(&g_api), 0); }
    g_service = &g_log;
    HCHECK("no failed init flipped HasCore()", !KpLog_HasCore());
    g_regs.clear();

    // ================= 5. KpLog_Init success against the fake core
    HEQ("KpLog_Init(api) with a complete kpatch.log service returns 1", modKPatchInit(&g_api), 1);
    HCHECK("HasCore() is true after init: enter() logs the START line", KpLog_HasCore());
    HCHECK("registered once under the manifest id \"quick-loot\" (KP_LOG_PATCH)", g_regs.size() == 1 && g_regs[0] == "quick-loot");
    g_subs.clear();
    KP_LOG(chMain, "START quick-loot %s", "0.0.1-probe");
    HEQ("the START line is submitted", (long long)g_subs.size(), 1);
    if (!g_subs.empty()) {
        HEQ("submitted at INFO", g_subs[0].lvl, KPLOG_INFO);
        HCHECK("submitted text is exactly the formatted message", g_subs[0].text == "START quick-loot 0.0.1-probe");
    }
    HCHECK("the \"main\" channel was resolved for patch handle 5", !g_chans.empty() && g_chans[0] == "5:main");

    return kt::Report("common_contract_test (quick_loot)");
}
