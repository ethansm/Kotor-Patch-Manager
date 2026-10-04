// common_contract_test.cpp -- Patches/Common CONTRACT test for Mouse Probe (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the headers' own behaviour (that is _shared_src/saferead_test.cpp and KPM's KpLog tests). Every message starts with
// "mouse_probe relies on:" so a failure names the consumer and the reason. Built against ../_shared_src and the KPM checkout's
// Patches/Common (see test.toml), so ANY edit to SafeRead.h / KpLog.h re-runs this test: a Common change that would break the probe
// fails here first.
//
// 0.3.0 adds area (5): the pure click heuristics in ClickLogic.h and the hook-side pointer hops that ProbeMouseMsg / ProbeClickInWorld / ProbeDefaultAction make.
// Areas: (1) saferead as rd<T> uses it (MouseProbe.cpp:55,74), (2) KpLog fallback sink BEFORE any successful KpLog_Init (init cannot be
// undone): file path, level gate, old-core path, (3) KpLog_Init rejections, (4) KpLog_Init success against a fake kpatch.log service.
// Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include "SafeRead.h"
#include "KpLog.h"
#include "ClickLogic.h"
#include "check.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#define HCHECK(why, expr) ::kt::Check("mouse_probe relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("mouse_probe relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("mouse-probe");   // MouseProbe.cpp:19 (compile-time id validity)
KP_CHANNEL(chMain, "main");    // :28
KP_CHANNEL(chRaw, "raw");      // :29
KP_CHANNEL(chDrag, "drag");    // :32
KP_CHANNEL(chClick, "click");  // 0.3.0 click channel

// ---- rd<T> exactly as MouseProbe.cpp:74
template <typename T> static bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(1, base, off, out); }

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

// The mod's KPatchInit (MouseProbe.cpp:21) is `KpLog_Init(api)`; this is that call.
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

    // ================= 1. saferead via rd<T> (MouseProbe.cpp:74; hook bodies :97-:160)
    {
        saferead::beginScope();   // enter() :55
        const uintptr_t ebp = A + 0x400;   // a hook's frame pointer; the probe reads at negative offsets
        *reinterpret_cast<uint32_t*>(ebp - 8) = 0x00008000u;       // rgbButtons
        *reinterpret_cast<int32_t*>(ebp - 0x14) = -12;             // lX
        *reinterpret_cast<int32_t*>(ebp - 0x10) = 34;              // lY
        *reinterpret_cast<int32_t*>(ebp - 0xC) = 0;                // lZ
        unsigned btn = 0; int lx = 0, ly = 0, lz = 9;
        HCHECK("rd(ebp, -8, &btn) reads at a NEGATIVE offset from the frame pointer (:97)", rd(ebp, -8, &btn));
        HEQ("unsigned button mask arrives intact (:98 shifts it)", btn, 0x8000u);
        HCHECK("three DIMOUSESTATE lX/lY/lZ reads at -0x14/-0x10/-0xC (:101)", rd(ebp, -0x14, &lx) && rd(ebp, -0x10, &ly) && rd(ebp, -0xC, &lz));
        HEQ("lX negative value preserved (:101)", lx, -12);
        HEQ("lY value (:101)", ly, 34);
        HEQ("lZ zero read as 0, not as failure (:101)", lz, 0);

        // DragFlagAddr-style absolute read: base = address, off 0 (:137)
        const uintptr_t dragFlag = A + 0x800; *reinterpret_cast<int32_t*>(dragFlag) = 1;
        int drag = 0;
        HCHECK("rd(DragFlagAddr, 0, &drag) reads an absolute global (:137)", rd(dragFlag, 0, &drag));
        HEQ("drag flag value (:137)", drag, 1);

        // float and pointer fields: self+0x408/0x40C floats (:122), ctl+0x94/0x98 floats, uintptr_t self (:152)
        const uintptr_t self = A + 0x1000;
        *reinterpret_cast<float*>(self + 0x408) = 0.5f; *reinterpret_cast<float*>(self + 0x40C) = -0.25f;
        *reinterpret_cast<uint32_t*>(ebp - 0x28) = static_cast<uint32_t>(self);
        uintptr_t gotSelf = 0; float nx = 0, ny = 0;
        HCHECK("rd(ebp, -0x28, &self) reads a 32-bit pointer into a uintptr_t (:122)", rd(ebp, -0x28, &gotSelf));
        HEQ("pointer value zero-extended exactly (:122)", (unsigned long long)gotSelf, (unsigned long long)self);
        HCHECK("rd float self+0x408 / +0x40C (:122)", rd(self, 0x408, &nx) && rd(self, 0x40C, &ny));
        HCHECK("float values bit-exact (:122-125)", nx == 0.5f && ny == -0.25f);
        int mode = -1;
        *reinterpret_cast<int32_t*>(self + 0xC) = 3;
        HCHECK("rd int at ctl+0xC (:153)", rd(self, 0xC, &mode));
        HEQ("mode value (:153)", mode, 3);

        // failures: the hooks `return` / fall to "unreadable" paths; *out must stay at its initial value
        int keep = 55;
        HCHECK("rd with base 0 fails (self==0 -> early return, :122)", !rd(0, 0x408, &keep));
        HEQ("failed rd leaves *out untouched (locals initialised to 0/-1, :134 :152)", keep, 55);
        HCHECK("rd with a tiny pointer (<0x10000) fails (garbage ctl, :153)", !rd(0x20, 0x94, &keep));
        HEQ("*out untouched after the low-pointer refusal", keep, 55);
        g_stubVirtualQueryAll = false; saferead::beginScope();
        HCHECK("rd on memory the OS reports unreadable fails (:152 ctl=unreadable path)", !rd(self, 0x94, &keep));
        g_stubVirtualQueryAll = true; saferead::beginScope();
        HCHECK("rd works again after beginScope() with readable memory (enter() resets the cache, :55)", rd(self, 0xC, &keep) && keep == 3);
        // the mod never calls enableRing, so failures must not pile up (no ring cost on the hot mouse path)
        LONG c0 = saferead::g_ringCount;
        rd(0x20, 0, &keep);
        HEQ("ring is off by default: a failed rd records nothing (mod never calls enableRing)", (long long)saferead::g_ringCount, (long long)c0);
    }

    // ================= 5. click channel (0.3.0): ClickLogic.h + the hook-side reads
    {
        using namespace clicklogic;
        // lParam unpacking: x = LOWORD, y = HIWORD, signed (multi-monitor negatives)
        HEQ("unpackX takes LOWORD", unpackX(0x00640032u), 0x32);
        HEQ("unpackY takes HIWORD", unpackY(0x00640032u), 0x64);
        HEQ("unpackX is signed (0xFFF6 = -10)", unpackX(0x0010FFF6u), -10);
        HEQ("unpackY is signed (0xFFF0 = -16)", unpackY(0xFFF00005u), -16);
        HCHECK("0x201..0x206 are the click messages, 0x200 (move) and 0x207 are not",
               isMouseButtonMsg(0x201) && isMouseButtonMsg(0x206) && !isMouseButtonMsg(0x200) && !isMouseButtonMsg(0x207));
        HCHECK("only 0x201..0x203 are LMB", isLmbMsg(0x201) && isLmbMsg(0x203) && !isLmbMsg(0x204));

        // Branch A/B derivation (HandleMouseClickInWorld)
        const uint32_t H = 0x1234, INV = 0x7F000000u;
        HCHECK("branch B: last==sel==hov, valid", isBranchB(H, H, H));
        HCHECK("branch A: last invalid", !isBranchB(H, H, INV));
        HCHECK("branch A: sel != hov", !isBranchB(H, 0x55, 0x55));
        HCHECK("branch A: last != sel", !isBranchB(H, H, 0x55));
        HCHECK("ground hover (hov invalid, sel==last==invalid) is A, never B", !isBranchB(INV, INV, INV));

        // MISSEDDBL: two 0x201 within 500 ms and <=4 px, no 0x203 between
        { ClickState c;
          HCHECK("first press is no verdict and has no delta", c.step(0x201, 100, 100, 0, 1000) == Verdict::None && !c.haveDelta);
          HCHECK("second 0x201 at 300 ms / 3 px -> MissedDbl", c.step(0x201, 102, 102, 0, 1300) == Verdict::MissedDbl);
          HEQ("dt reported", (long long)c.dtMs, 300LL);
          HCHECK("dpos reported (sqrt(8))", fabs(c.dpos - sqrt(8.0)) < 1e-9); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000);
          HCHECK("501 ms apart -> no verdict", c.step(0x201, 100, 100, 0, 1501) == Verdict::None); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000);
          HCHECK("exactly 500 ms apart still counts", c.step(0x201, 100, 100, 0, 1500) == Verdict::MissedDbl); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000);
          HCHECK("5 px apart -> no verdict", c.step(0x201, 105, 100, 0, 1200) == Verdict::None); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000);
          HCHECK("exactly 4 px apart still counts", c.step(0x201, 104, 100, 0, 1200) == Verdict::MissedDbl); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000); c.step(0x202, 100, 100, 0, 1050);
          HCHECK("a release between the presses does not suppress MissedDbl", c.step(0x201, 100, 100, 0, 1200) == Verdict::MissedDbl); }
        { ClickState c; c.step(0x201, 100, 100, 0, 1000); c.step(0x202, 100, 100, 0, 1040);
          HCHECK("0x203 between -> a proper double click: no DblGuard at guard 0", c.step(0x203, 100, 100, 0, 1200) == Verdict::None);
          c.step(0x202, 100, 100, 0, 1240);
          HCHECK("the next 0x201 after a real double click is not MissedDbl", c.step(0x201, 100, 100, 0, 1300) == Verdict::None); }
        { ClickState c;
          HCHECK("0x203 with a set guard -> DblGuard", c.step(0x203, 5, 5, 1, 50) == Verdict::DblGuard);
          HCHECK("0x201 with a set guard is not DblGuard", c.step(0x201, 5, 5, 1, 90) == Verdict::None);
          HCHECK("0x203 with guard 0 is not DblGuard", c.step(0x203, 5, 5, 0, 120) == Verdict::None); }
        { ClickState c;
          HCHECK("right-button messages never produce a verdict", c.step(0x204, 1, 1, 1, 10) == Verdict::None && c.step(0x204, 1, 1, 0, 20) == Verdict::None);
          HCHECK("an RMB press does not count as an LMB press", !c.haveLmb && !c.havePress); }
        // recent-LMB window for MOVEPT
        { ClickState c;
          HCHECK("no LMB seen -> not recent", !c.recentLmb(1000));
          c.step(0x202, 1, 1, 0, 1000);
          HCHECK("0x202 counts as an LMB event", c.recentLmb(3500) && c.recentLmb(4000));
          HCHECK("just past 3000 ms is no longer recent", !c.recentLmb(4001)); }

        // Hook-side reads: ProbeMouseMsg (ebp+8 -> msg, msg+4 id, msg+0xC lParam, ebp-0x1C -> this, this+0x3EC guard)
        saferead::beginScope();
        const uintptr_t ebp = A + 0x2400, msg = A + 0x2800, self = A + 0x3000;
        *reinterpret_cast<uint32_t*>(ebp + 8) = static_cast<uint32_t>(msg);
        *reinterpret_cast<int32_t*>(msg + 4) = 0x203;
        *reinterpret_cast<uint32_t*>(msg + 0xC) = 0x012C00C8u;
        *reinterpret_cast<uint32_t*>(ebp - 0x1C) = static_cast<uint32_t>(self);
        *reinterpret_cast<int32_t*>(self + 0x3EC) = 1;
        uintptr_t gm = 0, gs = 0; int gid = 0, gg = 0; uint32_t lp = 0;
        HCHECK("rd(ebp, 8, &msgp) reads a POSITIVE offset (ProbeMouseMsg)", rd(ebp, 8, &gm));
        HEQ("msg pointer exact", (unsigned long long)gm, (unsigned long long)msg);
        HCHECK("msg id at +4 and lParam at +0xC", rd(gm, 4, &gid) && rd(gm, 0xC, &lp));
        HEQ("msg id", gid, 0x203);
        HEQ("lParam x", unpackX(lp), 200);
        HEQ("lParam y", unpackY(lp), 300);
        HCHECK("this at ebp-0x1C then guard at this+0x3EC", rd(ebp, -0x1C, &gs) && rd(gs, 0x3EC, &gg));
        HEQ("guard value", gg, 1);
        // ProbeDefaultAction: this=[ebp-0x70], entry=[ebp-4] (+8 arg, +0xC fn, +0x1C type), actor=[ebp-8]
        const uintptr_t entry = A + 0x3800;
        *reinterpret_cast<uint32_t*>(ebp - 4) = static_cast<uint32_t>(entry);
        *reinterpret_cast<uint32_t*>(entry + 0xC) = 0x0077CF70u; *reinterpret_cast<uint32_t*>(entry + 8) = 7u; *reinterpret_cast<uint32_t*>(entry + 0x1C) = 5u;
        uintptr_t ge = 0; uint32_t fn = 0, arg = 0, ty = 0;
        HCHECK("entry ptr and fn/arg/type reads", rd(ebp, -4, &ge) && rd(ge, 0xC, &fn) && rd(ge, 8, &arg) && rd(ge, 0x1C, &ty));
        HCHECK("entry fields exact", fn == 0x0077CF70u && arg == 7u && ty == 5u);
        // ProbeLButtonDown / ProbeClickInWorld: hov +0x4B0, sel +0x2B4, last +0x2B8, vec +0x4B4.., nact +0x4D0
        *reinterpret_cast<uint32_t*>(self + 0x4B0) = 0x11; *reinterpret_cast<uint32_t*>(self + 0x2B4) = 0x11; *reinterpret_cast<uint32_t*>(self + 0x2B8) = 0x11;
        *reinterpret_cast<float*>(self + 0x4B4) = 1.5f; *reinterpret_cast<int32_t*>(self + 0x4D0) = 2;
        uint32_t hov = 0, sel = 0, last = 0; float vx = 0; int nact = 0;
        HCHECK("this+0x4B0/0x2B4/0x2B8/0x4B4/0x4D0 reads", rd(self, 0x4B0, &hov) && rd(self, 0x2B4, &sel) && rd(self, 0x2B8, &last) && rd(self, 0x4B4, &vx) && rd(self, 0x4D0, &nact));
        HCHECK("values exact and derive branch B", hov == 0x11 && vx == 1.5f && nact == 2 && isBranchB(hov, sel, last));
        // an unreadable message pointer fails soft (ProbeMouseMsg returns without logging)
        int keepId = 77;
        HCHECK("rd on a null msg pointer fails and leaves *out untouched", !rd(0, 4, &keepId) && keepId == 77);

    }

    // ================= 2. KpLog fallback sink (before any successful init)
    HCHECK("KpLog_HasCore() is false before KPatchInit: the old-core path in enter() (:56)", !KpLog_HasCore());
    const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
    char path[300]; snprintf(path, sizeof path, "%s/mouseprobe_contract_%d.txt", td, (int)getpid());
    remove(path);
    KpLog_SetFallbackFile(path);   // the mod passes "mouseprobe_log.txt" from a static initialiser (:37)
    HCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser, :37)", access(path, F_OK) != 0);
    HCHECK("fallback default level is WARN: KP_ENABLED(INFO) is false (mod starts quiet until the flag file, :62-65)", !KP_ENABLED(chMain, KPLOG_INFO));
    HCHECK("fallback default level WARN lets WARN through (STUCKDRAG?, :144)", KP_ENABLED(chDrag, KPLOG_WARN));
    HCHECK("click channel INFO is off at the default WARN fallback level (CLICK/DBLACT/... stay quiet)", !KP_ENABLED(chClick));
    HCHECK("click channel WARN is on at the default fallback level (MISSEDDBL / DBLGUARD)", KP_ENABLED(chClick, KPLOG_WARN));
    HCHECK("channel macros accept the one-argument KP_ENABLED(chRaw) form the hooks use (:99)", !KP_ENABLED(chRaw));
    KP_LOG(chMain, "START mouseprobe %s", "0.3.0");
    HCHECK("at the default WARN level an INFO KP_LOG is dropped and writes no file (:57/:66)", access(path, F_OK) != 0);
    KP_LOGV(chDrag, KPLOG_WARN, "STUCKDRAG? flag set %.1fs with no RMB in raw mask", 2.5);
    {
        std::string s = slurp(path);
        HEQ("at WARN a KP_LOGV(WARN) is written (:144): exactly one line", countLines(s), 1);
        HCHECK("fallback line has the patch id and channel: \"mouse-probe/drag\"", s.find(" mouse-probe/drag ") != std::string::npos);
        HCHECK("fallback line carries the WARN tag", s.find(" WARN STUCKDRAG? flag set 2.5s with no RMB in raw mask\n") != std::string::npos);
    }
    KpLog_SetFallbackLevel(KPLOG_INFO);   // :65 flag file appeared
    HCHECK("after SetFallbackLevel(INFO) KP_ENABLED(INFO) is true (flag-file session, :65)", KP_ENABLED(chMain, KPLOG_INFO));
    KP_LOG(chMain, "START mouseprobe 0.2.0");
    KP_LOGV(chDrag, KPLOG_WARN, "warn at info level");
    {
        std::string s = slurp(path);
        HEQ("at INFO both the INFO START line and the WARN line are written (3 lines total)", countLines(s), 3);
        HCHECK("the INFO line is written without a level tag", s.find(" mouse-probe/main START mouseprobe 0.2.0\n") != std::string::npos);
        HCHECK("the WARN line is tagged", s.find(" mouse-probe/drag WARN warn at info level\n") != std::string::npos);
    }
    KpLog_SetFallbackLevel(KPLOG_WARN);   // :69 flag file gone
    KP_LOG(chMain, "STOP (flag file gone)");
    KP_LOGV(chDrag, KPLOG_WARN, "still warn");
    {
        std::string s = slurp(path);
        HEQ("back at WARN: INFO dropped, WARN written (4 lines total, :69)", countLines(s), 4);
        HCHECK("the dropped INFO text is absent", s.find("STOP (flag file gone)") == std::string::npos);
    }
    KpLog_SetFallbackFile(nullptr);
    remove(path);

    // ================= 3. KpLog_Init rejections (each leaves the fallback in place)
    HEQ("KpLog_Init(nullptr) returns 0 (old core: KPatchInit gets no api, :21)", KpLog_Init(nullptr), 0);
    { KPatchApi noReq = { sizeof(KPatchApi), 1, nullptr, nullptr };
      HEQ("KpLog_Init with a null require returns 0", KpLog_Init(&noReq), 0); }
    g_service = nullptr;
    HEQ("KpLog_Init with the log service missing returns 0 (old core without kpatch.log)", modKPatchInit(&g_api), 0);
    HEQ("it asked for the \"kpatch.log\" interface", g_reqName == "kpatch.log", 1);
    HEQ("it asked for version 1", (unsigned long long)g_reqVer, 1ULL);
    g_service = &g_log;
    { KPatchLogApi sh = g_log; sh.struct_size = 8; g_service = &sh;
      HEQ("a short struct_size service is rejected", KpLog_Init(&g_api), 0); }
    { KPatchLogApi nf = g_log; nf.Submit = nullptr; g_service = &nf;
      HEQ("a service with a null function pointer is rejected", KpLog_Init(&g_api), 0); }
    { KPatchLogApi bad = g_log; bad.Register = [](const char*) -> int32_t { return -1; }; g_service = &bad;
      HEQ("a service that rejects the patch id (Register < 0) is rejected", KpLog_Init(&g_api), 0); }
    g_service = &g_log;
    HCHECK("no failed init flipped HasCore() (enter() keeps the old-core path, :56)", !KpLog_HasCore());
    HCHECK("no failed init registered anything with the service", g_regs.empty() || g_regs[0] == "mouse-probe");
    g_regs.clear();

    // ================= 4. KpLog_Init success against the fake core
    HEQ("KpLog_Init(api) with a complete kpatch.log service returns 1 (:21)", modKPatchInit(&g_api), 1);
    HCHECK("HasCore() is true after init: enter() takes the new-core START path (:56)", KpLog_HasCore());
    HEQ("the patch registered exactly once", (long long)g_regs.size(), 1);
    HCHECK("registered under the manifest id \"mouse-probe\" (KP_LOG_PATCH, :19)", !g_regs.empty() && g_regs[0] == "mouse-probe");
    g_subs.clear();
    KP_LOG(chMain, "START mouseprobe 0.2.0");   // :57
    HEQ("new core: the START line is submitted (not written to a file)", (long long)g_subs.size(), 1);
    if (!g_subs.empty()) {
        HEQ("submitted at INFO", g_subs[0].lvl, KPLOG_INFO);
        HCHECK("submitted text is exactly the formatted message, no prefix or newline", g_subs[0].text == "START mouseprobe 0.2.0");
    }
    HCHECK("the \"main\" channel was resolved lazily for patch handle 5 (Channel(patch,\"main\"))", !g_chans.empty() && g_chans[0] == "5:main");
    HCHECK("KP_ENABLED(chRaw) consults the service (raw channel off -> false, :99)", !KP_ENABLED(chRaw));
    HCHECK("the raw channel was resolved under the same patch handle", g_chans.size() >= 2 && g_chans[1] == "5:raw");
    size_t nsub = g_subs.size();
    KP_LOG(chRaw, "RAW %d %d %d %08x", 1, 2, 3, 4u);
    HEQ("a disabled channel submits nothing (Enabled gates before formatting, :108)", (long long)g_subs.size(), (long long)nsub);
    KP_LOGV(chDrag, KPLOG_WARN, "STUCKDRAG? flag set %.1fs with no RMB in raw mask", 3.0);
    HEQ("KP_LOGV(WARN) on an enabled channel is submitted (:144)", (long long)g_subs.size(), (long long)nsub + 1);
    if (g_subs.size() > nsub) {
        HEQ("submitted at WARN", g_subs.back().lvl, KPLOG_WARN);
        HCHECK("WARN text matches", g_subs.back().text == "STUCKDRAG? flag set 3.0s with no RMB in raw mask");
        HCHECK("drag channel got its own handle", g_subs.back().ch != g_subs[0].ch);
    }
    KP_LOGV(chDrag, KPLOG_DEBUG, "debug detail");
    HEQ("a level the service reports disabled (DEBUG) is not submitted", (long long)g_subs.size(), (long long)nsub + 1);
    return kt::Report("common_contract_test (mouse_probe)");
}
