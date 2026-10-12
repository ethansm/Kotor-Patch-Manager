// NpcDiversityDispatcher.cpp -- Master NPC Diversity Dispatcher runtime detour hooks (0.1.4-probe).
//
// Strategy B (confirmed by the 0.1.1 play-test: the engine builds N_Comm?NN from descriptor textureVar):
// Hook 2 RemapNpcAppearance: CSWCCreature::SetAppearance entry (Steam 0x76D200), args ecx=this,
//   [esp+4]=pDesc. Remaps the appearance row (fat / Czerka / Twi'lek variants), then writes
//   pDesc->textureVar = PickTextureVar(oid, finalAppearance) so every engine caller builds the same clothing.
//   Seed = (creature oid, appearance) only: position and `this` are not stable.
// 0.1.3: creature position offsets corrected to +0x24/+0x28/+0x2C; hook 2 never writes tv when pDesc.subType >= 3
//   (armor body path); TEX lines carry stype/oid and log pmb*/pfb* body requests for creatures hook 2 saw;
//   one-shot PTR dumps (64 bytes behind the pointers at +0x1C/+0x20/+0x5C/+0x6C) look for the creature tag.
// 0.1.4: only kVaryPct (35%) of generic commoners change (ShouldVary), tv comes from the per-family slot lists
//   (DiversitySlots.h), and named/merchant/uniformed NPCs keep their default (DiversityKeep.h). The creature tag is
//   still unreadable, so keep-default is POSITION-ONLY (DiversityRoles.h kKeepPositions) and companions/story NPCs
//   rely on the row tables.
// Hook 1 OverrideNpcTexture: inside CSWCCreatureAppearance::GetBTypeModelAndTextureResRef, after the
//   texture ResRef copy (Steam 0x85E601). LOG-ONLY in this build: it counts and logs what the engine requests.
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
#include "DiversityKeep.h"
#include "DiversityLogic.h"
#include "DiversityRows.h"
#include "HookDecision.h"
#include "NpcDivLayout.h"
#include "NpcDivLog.h"

KP_LOG_PATCH("npc-diversity");

// KPatchInit (KpLog_Init) is deliberately NOT exported in the probe builds: a hot-swap refuses a DLL whose exports
// differ from the live one, and the live npc-diversity.dll exports NpcDiversity_Init. Logs therefore use the
// header's fallback sink (npc-diversity_log.txt) at INFO. The KPatchInit migration (19f) is a later full reinstall.
extern "C" __declspec(dllexport) void NpcDiversity_Init() {}

namespace {

using namespace NpcDiversity;
namespace nl = NpcDiversity::log;

constexpr const char* kVersion = "0.1.4-probe";
constexpr size_t kResRefLen = 16;

// SafeRead site ids (counted in Counts::fail, printed by STAT read).
enum : unsigned {
    S_FRAME_DESC = 1, S_FRAME_TEX = 2, S_FRAME_RET = 3, S_DESC = 4, S_TEXRES = 5,
    S_CRE_PTR = 6, S_CRE_OID = 7, S_CRE_XY = 8, S_CRE_UNUSED9 = 9, S_DUMP = 10, S_HOOKBYTES = 11,
};

KP_CHANNEL(chInit, "init");
KP_CHANNEL(chTex, "tex");
KP_CHANNEL(chRemap, "remap");
KP_CHANNEL(chStat, "stat");
KP_CHANNEL(chCensus, "census");
KP_CHANNEL(chProbe, "probe");
KP_CHANNEL(chRead, "read");

// Only stores the path (no I/O); old core: same file name the checker expects.
const bool g_fallbackSet = (KpLog_SetFallbackFile("npc-diversity_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

nl::Counts g_c;
nl::U32Set<256> g_frSet, g_retSet, g_oidSet, g_cenTexSet, g_cenRemSet;
nl::Budget g_bTex, g_bRmp, g_bAbi, g_bArm;
uint32_t g_readBudget[nl::kSiteCount + 1] = {};
bool g_inited = false;
bool g_dsc0Done = false;
bool g_cre0Done = false;
uint32_t g_totalCalls = 0;

// One-shot CRE dumps beyond +0x40: solve the tag/oid layout offline from the log.
constexpr uint32_t kCreDumpOffs[] = {0x40, 0x80, 0xC0, 0x100, 0x140, 0x180, 0x1C0};
constexpr unsigned kCreDumpCount = sizeof kCreDumpOffs / sizeof kCreDumpOffs[0];
unsigned g_creDumpDone = 0;  // bitmask over kCreDumpOffs

// Pointer fields whose targets are dumped once each (tag-route probe).
constexpr uint32_t kPtrDumpOffs[] = {layout::kCreOffPtr1C, layout::kCreOffPtr20, layout::kCreOffPtr5C, layout::kCreOffPtr6C};
constexpr unsigned kPtrDumpCount = sizeof kPtrDumpOffs / sizeof kPtrDumpOffs[0];
unsigned g_ptrDumpDone = 0;  // bitmask over kPtrDumpOffs

// pDesc -> creature oid, written by hook 2 and read by hook 1 (the descriptor is rebuilt per call, so entries can go
// stale; a stale hit only mislabels a probe log line). Direct-mapped, 256 entries.
struct PdOid { uint32_t pd, oid; };
PdOid g_pdOid[256];
unsigned pdSlot(uint32_t pd) { return (pd >> 3) & 0xFFu; }
void pdOidPut(uint32_t pd, uint32_t oid) { g_pdOid[pdSlot(pd)] = PdOid{pd, oid}; }
uint32_t pdOidGet(uint32_t pd) { const PdOid& e = g_pdOid[pdSlot(pd)]; return e.pd == pd ? e.oid : 0u; }

void logBanner();

void ensureInit() {
    if (g_inited) return;
    g_inited = true;
    g_c.reset();
    std::memset(g_pdOid, 0, sizeof g_pdOid);
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
// The address table is chosen by probing the code bytes, not by the exe hash (the env var is not visible
// in the game process). A hook is "ok" when the original bytes are still there, "jmp" when a detour whose
// target lies outside the exe image has replaced them.

struct HookAddr { const char* name; uint32_t addr; const unsigned char* want; size_t wantLen; const char* wantHex; };

const unsigned char kTexBytes[] = {0xC6, 0x45, 0xFC, 0x00, 0x8D, 0x4D, 0xB4};
const unsigned char kRemapBytes[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x74};

struct Probe { const char* pre; char got[17]; };

Probe probeHook(const HookAddr& h, uint32_t imgBase, uint32_t imgSize) {
    Probe p{"unread", {}};
    unsigned char raw[8] = {};
    if (!saferead::probeRead(S_HOOKBYTES, reinterpret_cast<const void*>(static_cast<uintptr_t>(h.addr)), h.wantLen)) return p;
    std::memcpy(raw, reinterpret_cast<const void*>(static_cast<uintptr_t>(h.addr)), h.wantLen);
    for (size_t i = 0; i < h.wantLen && i < 8; ++i) std::snprintf(p.got + i * 2, 3, "%02x", raw[i]);
    if (std::memcmp(raw, h.want, h.wantLen) == 0) { p.pre = "ok"; return p; }
    p.pre = "bad";
    if (raw[0] == 0xE9) {
        int32_t rel = 0;
        std::memcpy(&rel, raw + 1, 4);
        const uint32_t target = h.addr + 5u + static_cast<uint32_t>(rel);
        if (target < imgBase || target - imgBase >= imgSize) p.pre = "jmp";
    }
    return p;
}

bool preOk(const Probe& p) { return std::strcmp(p.pre, "ok") == 0 || std::strcmp(p.pre, "jmp") == 0; }

void logBanner() {
    uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)));
    uint32_t ts = 0, lfanew = 0, imgSize = 0;
    if (saferead::readAt(S_DUMP, base, 0x3C, &lfanew)) {
        saferead::readAt(S_DUMP, base, static_cast<int>(lfanew) + 8, &ts);
        saferead::readAt(S_DUMP, base, static_cast<int>(lfanew) + 0x50, &imgSize);
    }

    const HookAddr steam[2] = {
        {"tex", 0x0085E601u, kTexBytes, sizeof kTexBytes, "c645fc008d4db4"},
        {"remap", 0x0076D200u, kRemapBytes, sizeof kRemapBytes, "558bec83ec74"},
    };
    const HookAddr gog[2] = {
        {"tex", 0x0048E691u, kTexBytes, sizeof kTexBytes, "c645fc008d4db4"},
        {"remap", 0x00419070u, kRemapBytes, sizeof kRemapBytes, "558bec83ec74"},
    };
    Probe ps[2] = {probeHook(steam[0], base, imgSize), probeHook(steam[1], base, imgSize)};
    Probe pg[2] = {probeHook(gog[0], base, imgSize), probeHook(gog[1], base, imgSize)};
    const char* table = "none";
    const HookAddr* hooks = steam;  // reported when neither table matches: Steam, with the bad bytes
    Probe* probes = ps;
    if (preOk(ps[0]) && preOk(ps[1])) { table = "steam"; }
    else if (preOk(pg[0]) && preOk(pg[1])) { table = "gog"; hooks = gog; probes = pg; }

    char b[nl::kLineMax];
    nl::FormatBanner0(b, sizeof b, kVersion, KpLog_HasCore() ? 1 : 0, table, base, ts);
    logLine(chInit, b);
    for (int i = 0; i < 2; ++i) {
        nl::FormatBannerHook(b, sizeof b, hooks[i].name, hooks[i].addr, probes[i].pre);
        logLine(chInit, b);
        if (std::strcmp(probes[i].pre, "bad") == 0) {
            nl::FormatHookBytes(b, sizeof b, hooks[i].addr, hooks[i].wantHex, probes[i].got);
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

void oneShotCreN(uint32_t self, uint32_t off) {
    uint8_t bytes[64] = {};
    const uintptr_t a = static_cast<uintptr_t>(self) + off;
    if (!saferead::probeRead(S_DUMP, reinterpret_cast<const void*>(a), sizeof bytes)) return;
    std::memcpy(bytes, reinterpret_cast<const void*>(a), sizeof bytes);
    char b[nl::kLineMax];
    nl::FormatCreN(b, sizeof b, self, off, bytes);
    logLine(chProbe, b);
}

// CRE0 (off 000) plus the higher windows, each once per session, from the first readable creature.
void oneShotCreDumps(uint32_t self) {
    if (!KP_ENABLED(chProbe)) return;
    if (!g_cre0Done) { g_cre0Done = true; oneShotCreN(self, 0); }
    for (unsigned i = 0; i < kCreDumpCount; ++i) {
        if (g_creDumpDone & (1u << i)) continue;
        g_creDumpDone |= 1u << i;
        oneShotCreN(self, kCreDumpOffs[i]);
    }
}

// Dumps 64 bytes behind the pointer stored at self+off, once per offset per session.
void onePtrDumps(uint32_t self) {
    if (!KP_ENABLED(chProbe)) return;
    for (unsigned i = 0; i < kPtrDumpCount; ++i) {
        if (g_ptrDumpDone & (1u << i)) continue;
        uint32_t p = 0;
        if (!saferead::readAt(S_CRE_PTR, self, static_cast<int>(kPtrDumpOffs[i]), &p)) continue;
        g_ptrDumpDone |= 1u << i;
        uint8_t bytes[64] = {};
        char b[nl::kLineMax];
        if (p && saferead::probeRead(S_DUMP, reinterpret_cast<const void*>(static_cast<uintptr_t>(p)), sizeof bytes)) {
            std::memcpy(bytes, reinterpret_cast<const void*>(static_cast<uintptr_t>(p)), sizeof bytes);
        }
        nl::FormatPtrN(b, sizeof b, self, kPtrDumpOffs[i], p, bytes);
        logLine(chProbe, b);
    }
}

bool isArmorBodyRes(const char* cur) {
    const unsigned char a = static_cast<unsigned char>(cur[0]) | 0x20u, b = static_cast<unsigned char>(cur[1]) | 0x20u,
                        c = static_cast<unsigned char>(cur[2]) | 0x20u;
    return a == 'p' && (b == 'm' || b == 'f') && c == 'b';
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

} // namespace

extern "C" {

// LOG-ONLY in 0.1.2-probe. Intercepts CSWCCreatureAppearance::GetBTypeModelAndTextureResRef after the vanilla
// texture lookup and records what the engine requested (texture name, st/tv). It never writes.
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
    if (!saferead::readResRef16(S_TEXRES, texPtr, cur, 0)) { ++g_c.texUnread; logRead(S_TEXRES, texPtr, 16); return; }
    oneShotDsc0(pdPtr);

    if (KP_ENABLED(chCensus) && g_cenTexSet.insert(nl::detail::Fmix(d.appearance * 977u + d.gender * 31u + d.textureVar + (uint32_t)d.descMode14 * 7u + (uint32_t)cur[0] * 131u + (uint32_t)cur[7] * 17u))) {
        char b[nl::kLineMax];
        nl::FormatCenTex(b, sizeof b, d.appearance, d.gender, d.textureVar, d.descMode14, cur, std::strlen(cur));
        logLine(chCensus, b);
    }

    bool isFemaleTex = false;
    if (!IsCommonerTexture(cur, isFemaleTex)) {
        ++g_c.texNonCommoner;
        // Armor-style body parts (pmb*/pfb*) requested for a creature hook 2 saw: log them (probe only).
        const uint32_t armOid = pdOidGet(pdPtr);
        if (armOid && isArmorBodyRes(cur) && KP_ENABLED(chTex) && g_bArm.take(200, 500)) {
            char b[nl::kLineMax];
            nl::FormatTexExt(b, sizeof b, pdPtr, static_cast<uint32_t>(fr), ret, d.appearance, d.gender,
                             d.textureVar, d.descMode14, d.headOverride, d.phenotype, cur, std::strlen(cur), 0u, 0,
                             0u, "0", 0, d.subType, armOid);
            logLine(chTex, b);
        }
        return;
    }
    ++g_c.texCommoner;
    if (d.appearance < nl::kAppCount) ++g_c.appTex[d.appearance]; else ++g_c.appOvf;

    if (KP_ENABLED(chTex) && g_bTex.take(300, 500)) {
        char b[nl::kLineMax];
        nl::FormatTexExt(b, sizeof b, pdPtr, static_cast<uint32_t>(fr), ret, d.appearance, d.gender, d.textureVar,
                         d.descMode14, d.headOverride, d.phenotype, cur, std::strlen(cur), 0u, 0, 0u, "0", 0,
                         d.subType, pdOidGet(pdPtr));
        logLine(chTex, b);
    }
}

// Remaps the NPC appearance row and writes the texture variant before the engine builds the body.
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
    oneShotCreDumps(static_cast<uint32_t>(self));
    onePtrDumps(static_cast<uint32_t>(self));

    // Creature facts. oid (+0x04) and position (+0x24/+0x28, log only) are verified. The tag is NOT readable yet:
    // +0x1C/+0x20 are pointers and the tag offset is unresolved (PTR dumps), so tag stays empty and protection comes
    // from the row tables only.
    uint32_t oid = 0;
    float x = 0.0f, y = 0.0f;
    const bool okOid = saferead::readAt(S_CRE_OID, self, layout::kCreOffObjectId, &oid);
    const bool okXy = saferead::readAt(S_CRE_XY, self, layout::kCreOffPosX, &x) &&
                      saferead::readAt(S_CRE_XY, self, layout::kCreOffPosY, &y);
    const char tag[1] = {};
    const int tagN = 0;
    const uint32_t tagPtr = 0;
    const int32_t tagLen = 0;
    if (okOid) { g_oidSet.insert(oid); pdOidPut(pd, oid); }
    const bool xyOk = okXy && plausibleCoord(x) && plausibleCoord(y);
    if (xyOk) {
        if (x < g_c.xMin) g_c.xMin = x;
        if (x > g_c.xMax) g_c.xMax = x;
        if (y < g_c.yMin) g_c.yMin = y;
        if (y > g_c.yMax) g_c.yMax = y;
    }

    if (KP_ENABLED(chCensus) && g_cenRemSet.insert(nl::detail::Fmix(d.appearance * 977u + d.gender * 31u + d.textureVar + 0u))) {
        char b[nl::kLineMax];
        nl::FormatCenRemap(b, sizeof b, d.appearance, d.gender, d.textureVar, tag, tagN > 0 ? static_cast<size_t>(tagN) : 0);
        logLine(chCensus, b);
    }

    const char* why = "nochg";
    uint32_t hash = 0;
    uint16_t newApp = d.appearance;
    uint8_t newTv = d.textureVar;

    const SkipReason skip = ShouldSkipDescriptor(d.descMode14, d.gender, d.appearance);
    ExcludeKind kind = ExcludeKind::None;
    ++g_c.remTagUnread;

    if (kind != ExcludeKind::None) {
        // Companions and story NPCs keep their appearance and clothing.
        ++g_c.remExcluded;
        why = kind == ExcludeKind::Companion ? "excl:comp" : "excl:story";
    } else if (skip == SkipReason::Gender) {
        ++g_c.remSkipGender;
        why = "skip:gender";
    } else if (IsKeepDefault(tag, x, y, xyOk)) {
        // Named / merchant / uniformed / plot NPCs (position list, plus the tag list once the tag is readable).
        ++g_c.remKeep;
        why = "keep";
    } else if (!okOid || oid == 0) {
        // The seed is the creature id; without it nothing is written (the creature stays vanilla).
        why = "unread:oid";
    } else if (!ShouldVary(oid, d.appearance)) {
        // Only kVaryPct of generic commoners change; the roll is a pure function of (oid, original row).
        ++g_c.remNoVary;
        why = "skip:vary";
    } else {
        ++g_c.remVary;
        // Row remap: only rows in the generic commoner tables (a row already remapped is not in them).
        if (skip == SkipReason::None) {
            hash = HashCreatureId(oid, d.appearance);
            const uint16_t cand = RemapAppearance(d.appearance, d.gender, hash);
            if (cand != d.appearance) {
                if (saferead::regionWritable(pd + layout::kDescOffAppearance, pd + layout::kDescOffAppearance + 2)) {
                    pDesc->appearance = cand;
                    newApp = cand;
                    ++g_c.remChanged;
                    if (cand < nl::kAppCount) ++g_c.appRem[cand]; else ++g_c.appOvf;
                } else {
                    why = "unread:desc";
                }
            } else {
                ++g_c.remNoChange;
            }
        } else {
            ++g_c.remSkipRow;
        }
        // Texture variant for the final row. Idempotent: the seed ignores the old textureVar.
        // subType >= 3 builds PMB/PFB armor parts, whose variants are not N_Comm slots: never write tv there.
        const uint8_t want = PickTextureVar(oid, newApp, d.textureVar);
        if (static_cast<int>(d.subType) > layout::kDescSubTypeMaxCommoner) {
            ++g_c.remTvArmor;
            if (why[0] == 'n') why = "tvarmor";
        } else if (want != 0 && want != d.textureVar) {
            if (saferead::regionWritable(pd + layout::kDescOffTextureVar, pd + layout::kDescOffTextureVar + 1)) {
                pDesc->textureVar = want;
                newTv = want;
                ++g_c.remTvChg;
                if (d.gender < 2 && want < nl::kSlotCount) ++g_c.slot[d.gender][want];
            } else {
                why = "unread:desc";
            }
        }
        if ((why[0] == 'n' || why[0] == 't') && (newApp != d.appearance || newTv != d.textureVar)) why = "ok";
    }

    if (KP_ENABLED(chRemap) && g_bRmp.take(300, 500)) {
        char b[nl::kLineMax];
        nl::FormatRmpExt(b, sizeof b, static_cast<uint32_t>(self), pd, oid, tagPtr, tagLen, tag, 0, x, y,
                         floatBits(x), floatBits(y), d.appearance, newApp, d.textureVar, newTv, hash, why,
                         d.subType);
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
