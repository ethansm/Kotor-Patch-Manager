// NpcDiversityDispatcher.cpp -- Master NPC Diversity Dispatcher runtime detour hooks (0.1.1-probe).
//
// Hook 1 OverrideNpcTexture: inside CSWCCreatureAppearance::GetBTypeModelAndTextureResRef, after the
//   texture ResRef copy (Steam 0x85E601). Rewrites N_CommM/F<nn> to a per-creature slot.
// Hook 2 RemapNpcAppearance: CSWCCreature::SetAppearance entry (Steam 0x76D200), args ecx=this,
//   [esp+4]=pDesc. Remaps the appearance row (fat / Czerka / Twi'lek variants).
//
// Every engine read goes through SafeRead (no raw dereference). The probe build logs the layout facts
// that are still INFERRED (creature oid / position / tag offsets, DSC0 / CRE0 hex dumps) so the
// strategy can be locked from a play log. Log grammar: .claude/review/npc_div/E_logging.md 4.2.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "KpLog.h"
#include "../_shared/SafeRead.h"
#include "DiversityLogic.h"
#include "DiversityRows.h"
#include "HookDecision.h"
#include "NpcDivLayout.h"
#include "NpcDivLog.h"

KP_LOG_PATCH("npc-diversity");

extern "C" void __cdecl KPatchInit(const KPatchApi* api) { KpLog_Init(api); }

namespace {

using namespace NpcDiversity;
namespace nl = NpcDiversity::log;

constexpr const char* kVersion = "0.1.1-probe";
constexpr size_t kResRefLen = 16;

// SafeRead site ids (counted in Counts::fail, printed by STAT read).
enum : unsigned {
    S_FRAME_DESC = 1, S_FRAME_TEX = 2, S_FRAME_RET = 3, S_DESC = 4, S_TEXRES = 5,
    S_CRE_TAGPTR = 6, S_CRE_OID = 7, S_CRE_XY = 8, S_CRE_TAGSTR = 9, S_DUMP = 10, S_HOOKBYTES = 11,
};

KP_CHANNEL(chInit, "init");
KP_CHANNEL(chTex, "tex");
KP_CHANNEL(chRemap, "remap");
KP_CHANNEL(chStat, "stat");
KP_CHANNEL(chCensus, "census");
KP_CHANNEL(chProbe, "probe");
KP_CHANNEL(chRead, "read");

// Only stores the path (no I/O); old core: same file name the checker expects.
const bool g_fallbackSet = (KpLog_SetFallbackFile("npc-diversity_log.txt"), true);

nl::Counts g_c;
nl::U32Set<256> g_frSet, g_retSet, g_oidSet, g_cenTexSet, g_cenRemSet;
nl::Budget g_bTex, g_bRmp, g_bAbi;
uint32_t g_readBudget[nl::kSiteCount + 1] = {};
bool g_inited = false;
bool g_dsc0Done = false;
bool g_cre0Done = false;
uint32_t g_totalCalls = 0;

// Hooks run on the game thread; the stash is keyed by pDesc and also checked against the descriptor's
// current appearance, so a stale or cross-thread entry can only cause a miss, never a wrong join.
struct Stash {
    uint32_t pd;
    uint32_t hash;
    uint16_t appBefore;
    uint16_t appAfter;
    bool valid;
};
constexpr int kStashSize = 8;
Stash g_stash[kStashSize];
int g_stashNext = 0;

void logBanner();

void ensureInit() {
    if (g_inited) return;
    g_inited = true;
    g_c.reset();
    g_frSet.reset(); g_retSet.reset(); g_oidSet.reset(); g_cenTexSet.reset(); g_cenRemSet.reset();
    saferead::enableRing(true);
    logBanner();  // first hook call: never under the loader lock
}

uint32_t charVal(char c) { return static_cast<uint32_t>(static_cast<unsigned char>(c)); }

void logLine(kplog::Channel& ch, const char* line) { KP_LOG(ch, "%s", line); }

void logRead(unsigned site, uintptr_t addr, unsigned len) {
    if (site <= nl::kSiteCount) ++g_c.fail[site];
    if (site > nl::kSiteCount || ++g_readBudget[site] > 3) return;
    char b[nl::kLineMax];
    nl::FormatRead(b, sizeof b, site, static_cast<uint32_t>(addr), len, 0, g_readBudget[site]);
    KP_LOGV(chRead, KPLOG_WARN, "%s", b);
}

// ---- startup banner (lazy: first hook call, never under the loader lock) ----

struct HookAddr { const char* name; uint32_t addr; const unsigned char* want; size_t wantLen; const char* wantHex; };

const unsigned char kTexBytes[] = {0xC6, 0x45, 0xFC, 0x00, 0x8D, 0x4D, 0xB4};
const unsigned char kRemapBytes[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x74};

bool shaIs(const char* sha, const char* want) {
    return sha && std::strlen(sha) == std::strlen(want) && StartsWithCaseInsensitive(sha, want);
}

void logBanner() {
    const char* sha = std::getenv("KOTOR_VERSION_SHA");
    const bool steam = shaIs(sha, "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF");
    const bool gog = shaIs(sha, "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914");
    uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)));
    uint32_t ts = 0, lfanew = 0;
    if (saferead::readAt(S_DUMP, base, 0x3C, &lfanew)) saferead::readAt(S_DUMP, base, static_cast<int>(lfanew) + 8, &ts);

    char b[nl::kLineMax];
    nl::FormatBanner0(b, sizeof b, kVersion, KpLog_HasCore() ? 1 : 0, sha ? sha : "unknown", base, ts);
    logLine(chInit, b);

    const HookAddr hooks[2] = {
        {"tex", steam ? 0x0085E601u : (gog ? 0x0048E691u : 0u), kTexBytes, sizeof kTexBytes, "c645fc008d4db4"},
        {"remap", steam ? 0x0076D200u : (gog ? 0x00419070u : 0u), kRemapBytes, sizeof kRemapBytes, "558bec83ec74"},
    };
    for (const HookAddr& h : hooks) {
        const char* pre = "unread";
        char got[33] = {};
        if (h.addr) {
            unsigned char raw[8] = {};
            if (saferead::probeRead(S_HOOKBYTES, reinterpret_cast<const void*>(h.addr), h.wantLen)) {
                std::memcpy(raw, reinterpret_cast<const void*>(h.addr), h.wantLen);
                for (size_t i = 0; i < h.wantLen && i < 8; ++i) std::snprintf(got + i * 2, 3, "%02x", raw[i]);
                if (std::memcmp(raw, h.want, h.wantLen) == 0) pre = "ok";
                else if (raw[0] == 0xE9 || raw[0] == 0xE8 || raw[0] == 0xFF) pre = "jmp";  // detour already installed
                else pre = "bad";
            }
        } else {
            pre = "nosha";
        }
        nl::FormatBannerHook(b, sizeof b, h.name, h.addr, pre);
        logLine(chInit, b);
        if (std::strcmp(pre, "bad") == 0) {
            nl::FormatHookBytes(b, sizeof b, h.addr, h.wantHex, got);
            KP_LOGV(::kplog::detail::gMain, KPLOG_WARN, "%s", b);
        }
    }
    nl::FormatBannerRows(b, sizeof b, APP_FIRST_APPENDED, APP_TWILEK_FEMALE_UNDERWEAR, APP_COUNT - APP_FIRST_APPENDED,
                         HEAD_LLK_AHRNELL, HEAD_LLK_DAHNISH, HEAD_LLK_DAHNISH - HEAD_LLK_AHRNELL + 1, APP_COUNT,
                         HEAD_COUNT);
    logLine(chInit, b);
}

// ---- STAT dump (call-count throttled; no flag-file trigger) ----

void dumpStats() {
    if (!KP_ENABLED(chStat)) return;
    char b[nl::kLineMax];
    g_c.frDistinct = g_frSet.count;
    g_c.retDistinct = g_retSet.count;
    g_c.oidDistinct = g_oidSet.count;
    nl::FormatStatTex(b, sizeof b, g_c); logLine(chStat, b);
    nl::FormatStatRemap(b, sizeof b, g_c); logLine(chStat, b);
    nl::FormatStatAbi(b, sizeof b, g_c); logLine(chStat, b);
    nl::FormatStatXy(b, sizeof b, g_c); logLine(chStat, b);
    for (uint32_t g = 0; g < 2; ++g) {
        int next = 0;
        while (next >= 0) {
            const int cur = next;
            next = nl::FormatStatSlot(b, sizeof b, g_c, g, static_cast<uint32_t>(cur));
            if (b[0]) logLine(chStat, b);
            if (cur == 0 && next < 0) break;
        }
    }
    for (uint32_t kind = 0; kind < 2; ++kind) {
        int next = 0;
        while (next >= 0) {
            next = nl::FormatStatApp(b, sizeof b, g_c, kind, static_cast<uint32_t>(next));
            if (b[0]) logLine(chStat, b);
        }
    }
    nl::FormatStatRead(b, sizeof b, g_c); logLine(chStat, b);
    nl::FormatStatCensus(b, sizeof b, g_cenTexSet.count + g_cenRemSet.count, g_cenTexSet.ovf + g_cenRemSet.ovf);
    logLine(chStat, b);
    char ring[480];
    saferead::dump(ring, sizeof ring);
    KP_LOG(chStat, "STAT ring %s", ring);
}

void maybeDump() {
    ++g_totalCalls;
    if (g_totalCalls == 64 || g_totalCalls == 256 || (g_totalCalls % 1024) == 0) dumpStats();
}

void oneShotDsc0(uint32_t pd) {
    if (g_dsc0Done || !KP_ENABLED(chProbe)) return;
    uint8_t bytes[32] = {};
    if (!saferead::probeRead(S_DUMP, reinterpret_cast<const void*>(static_cast<uintptr_t>(pd)), sizeof bytes)) return;
    std::memcpy(bytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(pd)), sizeof bytes);
    g_dsc0Done = true;
    char b[nl::kLineMax];
    nl::FormatDsc0(b, sizeof b, pd, bytes);
    logLine(chProbe, b);
}

void oneShotCre0(uint32_t self) {
    if (g_cre0Done || !KP_ENABLED(chProbe)) return;
    uint8_t bytes[64] = {};
    if (!saferead::probeRead(S_DUMP, reinterpret_cast<const void*>(static_cast<uintptr_t>(self)), sizeof bytes)) return;
    std::memcpy(bytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(self)), sizeof bytes);
    g_cre0Done = true;
    char b[nl::kLineMax];
    nl::FormatCre0(b, sizeof b, self, bytes);
    logLine(chProbe, b);
}

uint32_t floatBits(float f) { uint32_t u; std::memcpy(&u, &f, 4); return u; }

bool plausibleCoord(float v) { return v == v && v > -1.0e5f && v < 1.0e5f; }

void abiCheck(const layout::AppearanceDesc& d, uint32_t pd, const char* where) {
    bool bad = false;
    if (d.appearance >= APP_COUNT + 64) { ++g_c.abiBadApp; bad = true; }
    if (d.gender > 4) { ++g_c.abiBadGender; bad = true; }
    if (d.descMode14 < 0 || d.descMode14 > 16) { ++g_c.abiBadDesc14; bad = true; }
    if (bad && g_bAbi.take(5, 0)) {
        char b[nl::kLineMax];
        nl::FormatAbi(b, sizeof b, d.appearance, d.gender, d.descMode14, d.textureVar, pd, where);
        KP_LOGV(::kplog::detail::gMain, KPLOG_WARN, "%s", b);
    }
}

void stashPut(uint32_t pd, uint32_t hash, uint16_t before, uint16_t after) {
    g_stash[g_stashNext] = {pd, hash, before, after, true};
    g_stashNext = (g_stashNext + 1) % kStashSize;
}

const Stash* stashFind(uint32_t pd, uint16_t currentApp) {
    for (int i = 0; i < kStashSize; ++i) {
        if (g_stash[i].valid && g_stash[i].pd == pd && g_stash[i].appAfter == currentApp) return &g_stash[i];
    }
    return nullptr;
}

} // namespace

extern "C" {

// Intercepts CSWCCreatureAppearance::GetBTypeModelAndTextureResRef after the vanilla texture lookup.
// `frame` is the resolver's EBP: [frame+4] = return address, [frame+8] = pDesc, [frame+0x10] = out ResRef*.
__declspec(dllexport) void __cdecl OverrideNpcTexture(void* frame) {
    saferead::beginScope();
    ensureInit();
    ++g_c.texCalls;
    maybeDump();
    if (!frame) { ++g_c.texNullGuard; return; }

    const uintptr_t fr = reinterpret_cast<uintptr_t>(frame);
    uint32_t pdPtr = 0, texPtr = 0, ret = 0;
    if (!saferead::readAt(S_FRAME_DESC, fr, layout::kFrameOffPDesc, &pdPtr) ||
        !saferead::readAt(S_FRAME_TEX, fr, layout::kFrameOffOutTexture, &texPtr)) {
        ++g_c.texUnread; logRead(S_FRAME_DESC, fr, 0x18); return;
    }
    saferead::readAt(S_FRAME_RET, fr, 4, &ret);
    g_frSet.insert(static_cast<uint32_t>(fr));
    g_retSet.insert(ret);
    if (!pdPtr || !texPtr) { ++g_c.texNullGuard; return; }

    layout::AppearanceDesc d{};
    if (!saferead::probeRead(S_DESC, reinterpret_cast<const void*>(static_cast<uintptr_t>(pdPtr)), sizeof d)) {
        ++g_c.texUnread; logRead(S_DESC, pdPtr, sizeof d); return;
    }
    std::memcpy(&d, reinterpret_cast<const void*>(static_cast<uintptr_t>(pdPtr)), sizeof d);
    abiCheck(d, pdPtr, "tex");

    char cur[17] = {};
    const bool haveTex = saferead::readResRef16(S_TEXRES, texPtr, cur, 0);
    if (!haveTex) { ++g_c.texUnread; logRead(S_TEXRES, texPtr, 16); return; }
    oneShotDsc0(pdPtr);

    if (KP_ENABLED(chCensus) && g_cenTexSet.insert(nl::detail::Fmix(d.appearance * 977u + d.gender * 31u + d.textureVar + (uint32_t)d.descMode14 * 7u + (uint32_t)cur[0] * 131u + (uint32_t)cur[7] * 17u))) {
        char b[nl::kLineMax];
        nl::FormatCenTex(b, sizeof b, d.appearance, d.gender, d.textureVar, d.descMode14, cur, std::strlen(cur));
        logLine(chCensus, b);
    }

    const SkipReason skip = ShouldSkipDescriptor(d.descMode14, d.gender, d.appearance);
    if (skip == SkipReason::DescMode) { ++g_c.texBodySkip; return; }
    if (skip != SkipReason::None) { ++g_c.texSkipRow; return; }

    bool isFemaleTex = false;
    if (!IsCommonerTexture(cur, isFemaleTex)) { ++g_c.texNonCommoner; return; }
    ++g_c.texCommoner;

    const Stash* st = stashFind(pdPtr, d.appearance);
    if (st) ++g_c.texStashHit; else ++g_c.texStashMiss;
    const uint32_t seed = ResolveTextureSeed(st != nullptr, st ? st->hash : 0u, pdPtr, ret);

    char newTex[32] = {};
    int sw = 0;
    int slot = 0;
    if (ShouldDiversifyTexture(cur, seed, newTex, sizeof newTex)) {
        slot = ComputeSlot(seed);
        if (saferead::regionWritable(texPtr, texPtr + kResRefLen)) {
            std::memset(reinterpret_cast<void*>(static_cast<uintptr_t>(texPtr)), 0, kResRefLen);
            size_t len = std::strlen(newTex);
            if (len > kResRefLen) len = kResRefLen;
            std::memcpy(reinterpret_cast<void*>(static_cast<uintptr_t>(texPtr)), newTex, len);
            sw = 1;
            ++g_c.texSwap;
            if (d.gender < 2 && slot >= 0 && slot < static_cast<int>(nl::kSlotCount)) ++g_c.slot[d.gender][slot];
            if (d.appearance < nl::kAppCount) ++g_c.appTex[d.appearance]; else ++g_c.appOvf;
        }
    }
    if (KP_ENABLED(chTex) && g_bTex.take(300, 500)) {
        char b[nl::kLineMax];
        nl::FormatTex(b, sizeof b, pdPtr, static_cast<uint32_t>(fr), ret, d.appearance, d.gender, d.textureVar,
                      d.descMode14, d.headOverride, d.phenotype, cur, std::strlen(cur), seed, st ? 1 : 0,
                      static_cast<uint32_t>(slot), sw ? "1" : "0", sw);
        logLine(chTex, b);
    }
}

// Remaps the NPC appearance row before the engine builds the body.
__declspec(dllexport) void __cdecl RemapNpcAppearance(void* pCreature, layout::AppearanceDesc* pDesc) {
    saferead::beginScope();
    ensureInit();
    ++g_c.remCalls;
    maybeDump();
    if (!pCreature || !pDesc) { ++g_c.remNullGuard; return; }

    const uintptr_t self = reinterpret_cast<uintptr_t>(pCreature);
    const uint32_t pd = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pDesc));
    layout::AppearanceDesc d{};
    if (!saferead::probeRead(S_DESC, pDesc, sizeof d)) {
        ++g_c.remUnread; logRead(S_DESC, pd, sizeof d); return;
    }
    std::memcpy(&d, pDesc, sizeof d);
    abiCheck(d, pd, "remap");
    oneShotDsc0(pd);
    oneShotCre0(static_cast<uint32_t>(self));

    // Creature facts. All offsets except the descriptor are INFERRED: the log (tp/tl/xyb/oid) validates them.
    uint32_t oid = 0, tagPtr = 0;
    int32_t tagLen = 0;
    float x = 0.0f, y = 0.0f;
    const bool okOid = saferead::readAt(S_CRE_OID, self, layout::kCreOffObjectId, &oid);
    const bool okTag = saferead::readAt(S_CRE_TAGPTR, self, layout::kCreOffTagPtr, &tagPtr) &&
                       saferead::readAt(S_CRE_TAGPTR, self, layout::kCreOffTagLen, &tagLen);
    const bool okXy = saferead::readAt(S_CRE_XY, self, layout::kCreOffPosX, &x) &&
                      saferead::readAt(S_CRE_XY, self, layout::kCreOffPosY, &y);
    char tag[40] = {};
    int tagN = 0;
    if (okTag) tagN = saferead::readExoLoose(S_CRE_TAGSTR, tagPtr, tagLen, tag, 33);
    if (okOid) g_oidSet.insert(oid);
    const bool xyOk = okXy && plausibleCoord(x) && plausibleCoord(y);
    if (xyOk) {
        if (x < g_c.xMin) g_c.xMin = x;
        if (x > g_c.xMax) g_c.xMax = x;
        if (y < g_c.yMin) g_c.yMin = y;
        if (y > g_c.yMax) g_c.yMax = y;
    }

    if (KP_ENABLED(chCensus) && g_cenRemSet.insert(nl::detail::Fmix(d.appearance * 977u + d.gender * 31u + d.textureVar + (tagN > 0 ? charVal(tag[0]) * 131u + charVal(tag[tagN > 3 ? 3 : 0]) * 17u : 0u)))) {
        char b[nl::kLineMax];
        nl::FormatCenRemap(b, sizeof b, d.appearance, d.gender, d.textureVar, tag, tagN > 0 ? static_cast<size_t>(tagN) : 0);
        logLine(chCensus, b);
    }

    const char* why = "ok";
    uint32_t hash = 0;
    uint16_t newApp = d.appearance;

    const SkipReason skip = ShouldSkipDescriptor(d.descMode14, d.gender, d.appearance);
    if (skip == SkipReason::DescMode) { ++g_c.remBodySkip; why = "skip:dm"; }
    else if (skip == SkipReason::Gender) { ++g_c.remSkipGender; why = "skip:gender"; }
    else if (skip != SkipReason::None) { ++g_c.remSkipRow; why = "skip:row"; }
    else {
        // Tag exclusion is a secondary guard (story rows are not in the tables). An unreadable tag does
        // not block the remap: it is counted, and the log shows whether the tag offsets are valid.
        const bool tagUsable = okTag && tagN > 0;
        ExcludeKind kind = ExcludeKind::None;
        if (tagUsable) kind = ClassifyExclusion(tag);
        else ++g_c.remTagUnread;

        if (kind != ExcludeKind::None) {
            ++g_c.remExcluded;
            why = kind == ExcludeKind::Companion ? "excl:comp" : "excl:story";
        } else {
            hash = xyOk ? HashCreaturePosition(x, y, oid, d.appearance)
                        : HashSeedFallback(static_cast<uint32_t>(self), pd);
            if (!xyOk) why = "unread:xy";
            else if (!tagUsable) why = "excl:notag";
            newApp = RemapAppearance(d.appearance, d.gender, hash);
            if (newApp != d.appearance) {
                if (saferead::regionWritable(pd + layout::kDescOffAppearance, pd + layout::kDescOffAppearance + 2)) {
                    pDesc->appearance = newApp;
                    ++g_c.remChanged;
                    if (d.appearance < nl::kAppCount) ++g_c.appRem[newApp < nl::kAppCount ? newApp : d.appearance]; else ++g_c.appOvf;
                } else {
                    newApp = d.appearance;
                    why = "unread:desc";
                }
            } else {
                ++g_c.remNoChange;
                if (why[0] == 'o') why = "nochg";
            }
            stashPut(pd, hash, d.appearance, newApp);
        }
    }

    if (KP_ENABLED(chRemap) && g_bRmp.take(300, 500)) {
        char b[nl::kLineMax];
        nl::FormatRmp(b, sizeof b, static_cast<uint32_t>(self), pd, oid, tagPtr, tagLen, tag,
                      tagN > 0 ? static_cast<size_t>(tagN) : 0, x, y, floatBits(x), floatBits(y), d.appearance,
                      newApp, hash, why);
        logLine(chRemap, b);
    }
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}
