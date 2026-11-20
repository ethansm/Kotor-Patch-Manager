// Queue HUD 0.4.11-dev (CS10 strip slot, CS11 kit art, slim strip) (Steam Aspyr build 6A522E71...). A GREEN copy of the vanilla combat action-queue panel (up to 4 icons), drawn
// OUT of combat so a command given while paused (a use-item / cast-spell that waits for unpause) is visible.
// Research: patch_manager_mods/30_queue_hud_evidence/RE.md (0x74DC80 panel filler, 0x74E0B0 icon resolver); logic: QueueHudLogic.h.
//
//   hook H9 PerformUseItem / H10 PerformCastSpell (gameaddr names + 3, hooks.toml): out-of-combat use/cast on a party member: buffer (return 1) or track (dispatch_now)
//   HUD capture (hud pointer + combat bit): hud-host K2HUD_FRAME (old hook H8 0x74C168 migrated, HL4)
//   phase    K2PS_HUD_DRAW prio 400 (portrait-slots): reads party member 0's SERVER creature queue, resolves icons with the
//            engine's own 0x74E0B0 (cached), and draws bg + border + icon over the hidden vanilla slots (hud+0x5F2C+i*0x148)
//   0.2.0    companion strips: for party members 1-2 a row of small green queue icons left of their health bar
//            (strip_side left | above | below the card; HUD slot table via portrait-slots; slot creature matched against the party member's client OR server pointer; per-creature round)
//   0.3.0    kit look: the slots are the kit_frame image (icon inset 15%), flat rects only as the fallback when a kit texture is missing;
//            a Cancel control (kit_cancel / kit_cancel_hi, a HUD label built in hud-host's K2HUD_BUILD phase, k2.hud@1) right of the leader's
//            QUEUE0 and left of each companion strip; LMB calls CServerExoApp::ClearAllActions (0x51CB10, doc 30 R7) for that member.
//            Out of combat only (the vanilla red Cancel Combat bar is untouched). No new hook: BUILD + FRAME_BEGIN are registry phases.
//   CS11/12  the kit art (frame, Cancel X) is the shared HUD kit Common/HudKit.h (kit_* textures, also used by companion-selfheal);
//            the left-side strip icon auto size is the selfheal button side (QueueHudLogic.h stripIconSize), strip_icon_px still overrides.
//   0.4.1    strip hidden for Down (HP<=0) companions; their queue journal poll is skipped too.
// Every pointer deref goes through saferead (site ids 0x7100+, strips 0x7150+, cancel 0x7160+). Engine calls: ClearAllActions (Cancel, hand-off clear-first), the replayed PerformUseItem/PerformCastSpell, AddUseTalentOnObjectActions (QL20 server route), the follower-starvation rotate (ActListRemoveHead/AddTail), round-node detach/reattach.
// Without portrait-slots (or an unknown exe) the mod is inert.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <utility>
#include <initializer_list>

#include "../Common/KpLog.h"
#include "../Common/SafeRead.h"
#include "../Common/ActQueue.h"
#include "../Common/K2Addr.h"
#include "../Common/Party.h"
#include "../Common/PortraitSlots.h"
#include "../Common/Gui2D.h"
#include "../Common/GuiKit.h"
#include "../Common/GuiButton.h"   // B2: leader Cancel = a BTN_CLEARALL clone (thin border like Cancel Combat)
#include "../Common/GuiTip.h"
#include "../Common/HudHost.h"
#include "../Common/SelfHealUi.h"   // shui::stripReserve: the fallback selfheal footprint when hud-host strip slots are unavailable
#include "../Common/EmbedTex.h"
#include "../Common/HudKit.h"
#include "../Common/GameAddr.h"
#include "QueueHudLogic.h"

KP_LOG_PATCH("queue-hud");

namespace {

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCap, "cap");
KP_CHANNEL(chIcon, "icon");
KP_CHANNEL(chDraw, "draw");
KP_CHANNEL(chCancel, "cancel");
KP_CHANNEL(chTrace, "trace");

// Only stores the path and the level (no I/O): safe from a static initialiser.
const bool g_fallbackSet = (KpLog_SetFallbackFile("queue-hud_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

// saferead site ids (0x7100+)
constexpr unsigned SiteCap = 0x7100, SiteHud = 0x7101, SiteQueue = 0x7110, SiteRound = 0x7120, SitePause = 0x7130, SiteRules = 0x7140, SiteStrip = 0x7150, SiteCancel = 0x7160, SiteSrvCast = 0x7170;

// Steam Aspyr addresses / offsets (RE.md; the manifest supports the Steam exe only).
constexpr uintptr_t AddrIconResolver = 0x0074E0B0;   // thiscall(hud ignored) 6 stack args, RET 0x18
constexpr uintptr_t AddrRulesGlobal = 0x00A1B4D0;    // [rules+0x104] -> spell table {count@0, data@4}, row stride 0x1A8
constexpr int HudCombatFlags = 0xA904;               // bit 1 = combat mode (set by 0x74E910 just before the hook)
constexpr int HudSlotBase = 0x5F2C, HudSlotStride = 0x148;
constexpr int HudClearAll = 0x6C3C, HudClearOne = 0x689C, HudClearOne2 = 0x6A6C;   // vanilla BTN_CLEARALL / CLEARONE / CLEARONE2 control bases (R7 FINDINGS sec 1; = the kBannerCtl flag offsets 0x6C84 / 0x68E4 / 0x6AB4 minus 0x48)
constexpr int PauseInternal = 0x10078;               // byte [[[[AppGlobal]+8]+4]+0x10078] & 2 = player pause (0x51C720 / 0x538CD0)
constexpr unsigned SpellRowStride = 0x1A8;
constexpr int AppToServer = 8;                       // *(AppGlobal) + 8 = CServerExoApp (R7: ECX of ClearAllActions)
constexpr int CrObjectId = 4;                        // server creature +4 = server object id (the ClearAllActions argument)

qh::Config g_cfg;
k2ps::Client g_ps;
k2hud::Client g_hudc;

// Unified action queue buffers (per party member)
qh::ActionBuffer g_itemQueue[3];
bool g_dispatching = false;
using PerformUseItemFn = void(__thiscall*)(void* creature, uint32_t itemId, int packedByte, uint32_t targetId, const float* pos);
constexpr uintptr_t AddrPerformUseItem = gameaddr::PerformUseItem;
using PerformCastSpellFn = void(__thiscall*)(void* creature, uint32_t spellId, uint32_t targetClientId, const float* pos, int multispell, int metamagic, int domain, int flag);
constexpr uintptr_t AddrPerformCastSpell = gameaddr::PerformCastSpell;
// QL20 server route: CSWSCreature::AddUseTalentOnObjectActions (doc 27 R1 route A), ECX = the owner's SERVER creature, RET 0x20, returns 0 when refused.
// Stack (talentType 0 = spell, spellId, targetServerId, multiclass byte, itemId 0x7F000000 = none, propIdx -1 = none, byte 0, metamagic byte):
// item-bound powers go to UseItem 0x57B4C0, the rest to AddCastSpellActions 0x565310 (round node + 0x3F, the path PerformCastSpell's server handler takes).
using UseTalentOnObjectFn = int(__thiscall*)(uintptr_t creature, int talentType, uint32_t talentId, uint32_t targetServerId, int multiclass,
                                              uint32_t itemId, uint32_t propIdx, int b8, int metamagic);
constexpr uintptr_t AddrUseTalentOnObject = gameaddr::AddUseTalentOnObject;
constexpr int CrCommandable = 0xEC;   // server creature: 0 = not commandable (0x57D8B0 returns 0 before queueing anything, 0057d8e3)

// ---- capture state (written by the hook, read by the draw callback; same game thread) ----
uintptr_t g_hud = 0;
int g_combat = 0;
bool g_combatUnreadable = false;   // last capture could not read the HUD flags (g_combat is then forced to 1)
bool g_softCombat = false;         // softBannerTick verdict this frame: in combat but only own-party targets (false when not in combat)
qh::SoftLatch g_softLatch;         // QH26: hostile-seen latch + soft confirm delay behind g_softCombat
DWORD g_combatEndTick = 0;         // QH25: tick of the last readable combat 1->0 edge (0 = no tail running)
// Draw / Cancel gate: in combat and not soft; unreadable HUD flags are hard (QH26).
inline bool hardCombatNow() { return qh::hardCombat(g_combat != 0, g_combatUnreadable, g_softCombat); }
bool g_bannerHidden = false;       // we cleared the visible bit of >= 1 banner/red-bar control and have not restored it yet
unsigned g_bannerMask = 0;         // which kBannerCtl[] entries WE cleared (restore sets only these)
uintptr_t g_bannerHud = 0;         // hud the mask belongs to
DWORD g_capTick = 0;
// MODE trace (trace_mode): per-member snapshot, last logged snapshot, and the reasons the draw / cancel / banner code paths decided this frame.
qh::ModeState g_modeNow[3], g_modePrev[3];
bool g_modePrimed[3];
int g_modeLines = 0;
qh::SoftCand g_modeCand[3];       // softBannerTick verdict per member (valid while g_modeCandOk[k])
bool g_modeCandOk[3];
int g_modeHidden[3];             // RL20: qualifying slots beyond the 4 drawn, per member this frame (MODE hidden=N)
const char* g_modeStrip[3];       // strip verdict this frame: "shown" | "hidden:<why>" (static literals; nullptr = nothing decided)
const char* g_cwWhy[3];           // first Cancel hide reason this frame (cwShow / cwHideAll / cwApply), reset after each MODE pass
bool g_bannerRestored = false;    // banner/red bar restored in this combat (until combat ends or the HUD is rebuilt)
bool g_modeDrew = false;          // HUD_DRAW ran since the last FRAME_BEGIN (the menu-frame MODE pass runs only when it did not)

template <typename T> bool rd(unsigned site, uintptr_t base, int off, T* out) { return saferead::readAt(site, base, off, out); }

bool g_logOn = true;
#define QLOG(ch, ...) do { if (g_logOn) KP_LOG(ch, __VA_ARGS__); } while (0)
#define QLOGV(ch, lvl, ...) do { if (g_logOn) KP_LOGV(ch, lvl, __VA_ARGS__); } while (0)
// Queue trace lines (CMD#/WAIT/RESOLVE/DROP/EDGE/QJ): INFO, gated by log=1 and trace_queue=1.
#define QTRACE(...) do { if (g_logOn && g_cfg.traceQueue) KP_LOG(chTrace, __VA_ARGS__); } while (0)

static uint32_t fnvHashStr(const char* s) {
    uint32_t h = 2166136261u;
    if (!s) return h;
    while (*s) { h ^= static_cast<uint8_t>(*s++); h *= 16777619u; }
    return h;
}

static uint32_t g_lastBorderSig[4] = {};
static void borderPathTrace(int siteIdx, const char* where, int m, const char* texName, int slots, bool ownerOk) {
    if (siteIdx < 0 || siteIdx >= 4) return;
    const uint32_t sig = (fnvHashStr(texName) * 31u) ^ (static_cast<uint32_t>(slots) << 4) ^ (ownerOk ? 1u : 0u) ^ 0xA5u;
    if (g_lastBorderSig[siteIdx] == sig) return;
    g_lastBorderSig[siteIdx] = sig;
    if (!g_logOn || !g_cfg.traceQueue) return;
    char line[qh::kLogLineMax];
    qh::fmtBorderPath(line, sizeof line, where, m, texName, slots, ownerOk);
    QTRACE("%s", line);
}

static uint32_t g_lastSkipSig[10] = {};
static void skipTraceClear(int siteIdx) {
    if (siteIdx >= 0 && siteIdx < 10) g_lastSkipSig[siteIdx] = 0;
}
static void skipTrace(int siteIdx, const char* where, int m, bool hasCmd, uint32_t cmdId, int n) {
    if (siteIdx < 0 || siteIdx >= 10) return;
    const uint32_t sig = (hasCmd ? (cmdId + 1u) * 131u : 1u) ^ (static_cast<uint32_t>(n) << 8) ^ 0x5Au;
    if (g_lastSkipSig[siteIdx] == sig) return;
    g_lastSkipSig[siteIdx] = sig;
    if (!g_logOn || !g_cfg.traceQueue) return;
    char line[qh::kLogLineMax];
    qh::fmtSkipTrace(line, sizeof line, where, m, hasCmd, cmdId, n);
    QTRACE("%s", line);
}

static bool bufBusy(const qh::ActionBuffer& b) { return b.active || b.count > 0; }
// The creature a busy buffer belongs to: the active action's, else the head of the queue.
static uintptr_t bufOwner(const qh::ActionBuffer& b) { return b.active ? b.activeAction.creature : b.queue[0].creature; }
// The action a busy buffer acts on next: the active one, else the head of the queue (same pick as bufOwner).
static const qh::QueuedAction& bufHead(const qh::ActionBuffer& b) { return b.active ? b.activeAction : b.queue[0]; }

uint32_t g_cmdSeq = 0;                                   // per-mod command id counter (CMD#<n>)
struct OursStamp { const char* tag; DWORD tick; };
OursStamp g_lastOurs[3] = {};                            // last action of ours per member: tag (hook|cancel|...) + GetTickCount()

void onConnect(bool ok, const char* why) { QLOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "portraitslots: %s (%s)", ok ? "connected" : "NOT connected", why ? why : ""); }

// ---- INI: queue_hud.ini in the game dir (CWD); missing file = defaults ----
int g_iniRejected = 0, g_iniFirstBad = 0;   // RL15: lines parseIni rejected (one startup WARN)
void loadIni() {
    FILE* f = fopen(".\\queue_hud.ini", "rb");
    if (!f) return;
    // QL18: the whole file (the 4095-byte buffer cut keys past byte 4095, e.g. strip_head_pct in the shipped ini); capped at 64 KB.
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 65536) { fclose(f); return; }
    char* buf = static_cast<char*>(malloc(static_cast<size_t>(sz) + 1));
    if (!buf) { fclose(f); return; }
    size_t n = fread(buf, 1, static_cast<size_t>(sz), f);
    fclose(f);
    buf[n] = 0;
    g_iniRejected = qh::parseIni(g_cfg, buf, &g_iniFirstBad);
    free(buf);
}

// ---- once-only log keys (a blank icon must not log every frame) ----
bool firstTime(uint32_t* seen, int& n, int cap, uint32_t key) {
    for (int i = 0; i < n; ++i) if (seen[i] == key) return false;
    if (n < cap) seen[n++] = key;   // table full: stop logging new keys rather than repeating
    else return false;
    return true;
}

// ---- icon cache: key -> resref (blank results expire so a gone item does not stay blank forever) ----
struct IconEntry { uint32_t key; char rr[17]; DWORD tick; bool used; };
constexpr int IconCache = 32;
IconEntry g_icons[IconCache];
int g_iconNext = 0;
constexpr DWORD BlankRetryMs = 500;

IconEntry* iconFind(uint32_t key, DWORD now) {
    for (auto& e : g_icons)
        if (e.used && e.key == key && (e.rr[0] || now - e.tick < BlankRetryMs)) return &e;
    return nullptr;
}
IconEntry* iconStore(uint32_t key, const char* rr, DWORD now) {
    IconEntry* slot = nullptr;
    for (auto& e : g_icons) if (e.used && e.key == key) { slot = &e; break; }   // an expired blank: reuse its slot
    if (!slot) { slot = &g_icons[g_iconNext]; g_iconNext = (g_iconNext + 1) % IconCache; }
    slot->used = true; slot->key = key; slot->tick = now;
    strncpy(slot->rr, rr, 16); slot->rr[16] = 0;
    return slot;
}

bool resrefOk(const char* s) {
    if (!s[0]) return false;
    for (int i = 0; s[i]; ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
    }
    return true;
}

// Engine 0x74E0B0: ECX ignored, (round, action, node, int* scratch, CResRef* out1, CResRef* out2), RET 0x18.
using ResolveFn = void(__thiscall*)(void* hud, uintptr_t round, uintptr_t action, uintptr_t node, int* scratch, void* out1, void* out2);

// The 0x0F path of the engine resolver has NO row NULL check: validate the spell id against [[rules+0x104]] first.
bool spellIdOk(uint32_t id) {
    uint32_t rules = 0, spells = 0, data = 0;
    int count = 0;
    if (!rd(SiteRules, AddrRulesGlobal, 0, &rules) || !rules) return false;
    if (!rd(SiteRules + 1, rules, 0x104, &spells) || !spells) return false;
    if (!rd(SiteRules + 2, spells, 0, &count) || !rd(SiteRules + 3, spells, 4, &data) || !data) return false;
    const int sid = static_cast<int>(id);
    if (sid < 0 || sid >= count) return false;
    return saferead::probeRead(SiteRules + 4, reinterpret_cast<const void*>(static_cast<uintptr_t>(data) + static_cast<uintptr_t>(sid) * SpellRowStride), 0x24);
}

// Resolves one slot's icon resref into rr (17 bytes). false = skip the slot (blank / refused), already logged once per key.
bool resolveIcon(uintptr_t hud, uintptr_t round, const qh::QEntry& e, uintptr_t action, uintptr_t node, uint32_t key, DWORD now, char* rr) {
    static uint32_t seenBlank[16]; static int nBlank = 0;
    static uint32_t seenSpell[16]; static int nSpell = 0;
    if (IconEntry* c = iconFind(key, now)) {
        if (!c->rr[0]) return false;
        memcpy(rr, c->rr, 17);
        return true;
    }
    if (e.type == qh::T_CAST_SPELL && !spellIdOk(e.param0)) {
        iconStore(key, "", now);
        if (firstTime(seenSpell, nSpell, 16, key)) QLOGV(chIcon, KPLOG_WARN, "skip 0x0F entry: spell id %d out of range / row unreadable (key=%08x)", static_cast<int>(e.param0), (unsigned)key);
        return false;
    }
    char out1[16], out2[16];
    memset(out1, 0, sizeof out1); memset(out2, 0, sizeof out2);
    int scratch = -1;
    qh::FakeAction fakeAction;
    uintptr_t actPtr = action;
    if (!actPtr && qh::buildFakeAction(fakeAction, e.type, e.param0)) {
        actPtr = fakeAction.addr();
    }
    reinterpret_cast<ResolveFn>(AddrIconResolver)(reinterpret_cast<void*>(hud), round, actPtr, node, &scratch, out1, out2);
    char name[17];
    memcpy(name, out1, 16); name[16] = 0;
    if (!resrefOk(name)) {
        iconStore(key, "", now);
        if (firstTime(seenBlank, nBlank, 16, key)) QLOGV(chIcon, KPLOG_WARN, "blank icon: type=0x%02X param0=%08x node=%08x key=%08x", e.type, (unsigned)e.param0, (unsigned)node, (unsigned)key);
        return false;
    }
    iconStore(key, name, now);
    QLOG(chIcon, "resolved key=%08x type=0x%02X param0=%08x node=%08x -> '%s'", (unsigned)key, e.type, (unsigned)e.param0, (unsigned)node, name);
    memcpy(rr, name, 17);
    return true;
}

// ---- engine reads ----
bool playerPaused() { return actq::playerPaused(SitePause); }

static_assert(actq::kMaxEntries == qh::kMaxEntries && actq::kMaxRound == qh::kMaxRound, "actq caps match the slot builder");

struct Snapshot {
    uintptr_t creature = 0, round = 0;
    int aa8 = 0, ab8 = 0;
    qh::QEntry e[qh::kMaxEntries]; uintptr_t eData[qh::kMaxEntries]; int ne = 0;
    int engineNe = 0;   // raw entry count: logs and the loops over e[]
    int gateNe = 0;     // QH-2a-02: what the dispatch gates see (qh::gateEngineCount)
    int nBuffered = 0;
    qh::RNode rn[qh::kMaxRound]; int nrn = 0;
    qh::RNode cur; bool haveCur = false;
    bool paused = false, waiting = false;
    bool activeShown = false;   // QH27: gatherFor appended the active buffered action at entry engineNe (qh::slotKind)
    uint32_t staleSince = 0;   // RL20: staleCurSince of the owning buffer (0 = unknown / in combat), so a stale flagless cur node does not count as the engine holding an action
};
bool engineHoldsAction(const Snapshot& s);   // defined with the hand-off code below

qh::RNode toQh(const actq::RNode& r) {
    qh::RNode o; o.subType = r.subType; o.spellId = r.spellId; o.itemId = r.itemId; o.featId = r.featId; o.ptr = r.ptr;
    return o;
}

// One server creature's queue / round, all via saferead (actq::read). false = no creature (nothing to show).
bool gatherFor(uintptr_t cr, Snapshot& s) {
    actq::Snapshot a;
    s.creature = cr;
    if (!actq::read(cr, &a, SiteQueue, SiteRound)) return false;
    s.round = a.round;
    s.engineNe = a.ne;
    s.ne = a.ne;
    for (int i = 0; i < a.ne; ++i) { s.e[i].type = a.e[i].type; s.e[i].param0 = a.e[i].param0; s.eData[i] = a.eData[i]; }
    s.nrn = a.nrn;
    {
        int gateTypes[qh::kMaxEntries];
        for (int i = 0; i < a.ne; ++i) gateTypes[i] = s.e[i].type;
        s.gateNe = qh::gateEngineCount(gateTypes, a.ne, a.nrn);
    }
    for (int i = 0; i < a.nrn; ++i) s.rn[i] = toQh(a.rn[i]);
    s.haveCur = a.haveCur;
    s.aa8 = a.aa8; s.ab8 = a.ab8;
    if (a.haveCur) s.cur = toQh(a.cur);
    s.paused = playerPaused();
    s.waiting = qh::isWaiting(s.paused, a.ab8, a.aa8);

    // Append buffered multi-item actions
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    int memberK = -1;
    uintptr_t slotCli = 0, slotSrv = 0;
    if (tbl) {
        for (int k = 0; k < 3; ++k) {
            uintptr_t m = party::member(tbl, k);
            uintptr_t ms = m ? party::serverCreature(m) : 0;
            if (m && (m == cr || ms == cr)) {
                memberK = k;
                slotCli = m;
                slotSrv = ms;
                break;
            }
        }
    }
    s.nBuffered = 0;
    if (memberK >= 0 && memberK < 3) {
        const qh::ActionBuffer& ib = g_itemQueue[memberK];
        s.staleSince = g_combat == 0 ? ib.staleCurSince : 0;   // RL20: in combat staleCurSince is frozen, so it is not used there
        // The active buffered action is a slot source too: a quickbar press wipes the engine queue, so without it the panel empties until our restore.
        // Appended after the engine entries (buildSlots gives the engine's cur node to the first qualifying entry); not counted in nBuffered.
        if (qh::activeNeedsSlot(ib.active, engineHoldsAction(s)) &&
            qh::ownerMatchesSlot(ib.activeAction.creature, slotCli, slotSrv) &&
            s.ne < qh::kMaxEntries) {
            s.activeShown = true;
            if (ib.activeAction.kind == qh::ActionKind::Spell) {
                s.e[s.ne].type = qh::T_CAST_SPELL;
                s.e[s.ne].param0 = ib.activeAction.spellId;
            } else {
                s.e[s.ne].type = qh::T_USE_ITEM;
                s.e[s.ne].param0 = ib.activeAction.clientItem;
            }
            s.eData[s.ne] = 0;
            s.ne++;
        }
        for (int bi = 0; bi < ib.count && s.ne < qh::kMaxEntries; ++bi) {
            if (!qh::ownerMatchesSlot(ib.queue[bi].creature, slotCli, slotSrv)) continue;
            if (ib.queue[bi].kind == qh::ActionKind::Spell) {
                s.e[s.ne].type = qh::T_CAST_SPELL;
                s.e[s.ne].param0 = ib.queue[bi].spellId;
            } else {
                s.e[s.ne].type = qh::T_USE_ITEM;
                s.e[s.ne].param0 = ib.queue[bi].clientItem;
            }
            s.eData[s.ne] = 0;
            s.ne++;
            s.nBuffered++;
        }
    }
    return true;
}

// Party member 0's (the leader's) server creature.
bool gather(Snapshot& s) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, 0) : 0;
    return gatherFor(m ? party::serverCreature(m) : 0, s);
}

// ---- kit textures (shared HUD kit: Common/HudKit.h, embedded TGAs kit_frame / kit_frame_hi / kit_cancel / kit_cancel_hi) ----
namespace tex {
using hudkit::Frame; using hudkit::FrameHi; using hudkit::Cancel; using hudkit::CancelHi;
bool g_init = false;
bool g_missLogged[hudkit::Count] = {};

void logLine(const char* fmt, ...) {   // embtex::g_log sink
    char buf[256]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOG(chMain, "%s", buf);
}
void ensure() {   // once per process; the textures outlive every HUD (never freed)
    if (g_init) return;
    g_init = true;
    embtex::g_log = logLine;
    hudkit::ensure();   // create + fill + queue (idempotent); heal() refills after a GL reset
}
// A pooled GUI handle keeps the texture alive; looked up lazily (null until the texture exists), the miss is logged once per texture.
void* image(int i) {
    void* im = hudkit::image(i);
    if (!im && !g_missLogged[i]) { g_missLogged[i] = true; KP_LOGV(chMain, KPLOG_WARN, "texture %s unavailable: %s", hudkit::resref(i), i <= FrameHi ? "flat fallback look" : "Cancel shows a text X"); }
    return im;
}
void heal() { embtex::heal(); }   // every frame (GL reset + missing rebuild, 20-refill give-up per texture)
}  // namespace tex

// ---- drawing ----
// Kit look: the kit_frame image (white, alpha = style frameA) with the icon inset 15%; a missing frame texture (or kit = 0) falls back to the
// 0.2.0 flat bg + border rects (logged once).
void drawSlot(int x, int y, int w, int h, const qh::Style& st, void* img, bool hi = false, bool allowKitFrame = true) {   // hi: kit_frame_hi (QH27 hover on a removable strip icon); allowKitFrame: false on leader queue / detached slots (QL23)
    static const float white[3] = { 1.0f, 1.0f, 1.0f };
    void* frame = (g_cfg.kit && allowKitFrame) ? tex::image(hi ? tex::FrameHi : tex::Frame) : nullptr;
    qh::Rect ir{ x, y, w, h };
    if (frame) {
        gui2d::drawImage(frame, x, y, w, h, 0, white, st.frameA);
        ir = qh::insetRect(ir);
    } else {
        int bp = st.borderPx;
        if (bp > w / 2) bp = w / 2;
        if (bp > h / 2) bp = h / 2;
        gui2d::fillRect(x, y, w, h, st.bg, st.bgA);
        if (bp >= 1) {
            gui2d::fillRect(x, y, w, bp, st.border, st.borderA);
            gui2d::fillRect(x, y + h - bp, w, bp, st.border, st.borderA);
            gui2d::fillRect(x, y + bp, bp, h - 2 * bp, st.border, st.borderA);
            gui2d::fillRect(x + w - bp, y + bp, bp, h - 2 * bp, st.border, st.borderA);
        }
    }
    if (img) gui2d::drawImage(img, ir.x, ir.y, ir.w, ir.h, 0, white, st.iconA);
}

// ---- Cancel controls (0.3.0): 3 HUD labels (0 = leader, 1-2 = companions) built from LBL_QUEUE1 in hud-host's BUILD phase ----
struct CW {
    uintptr_t c = 0;
    qh::Rect r{ 0, 0, 0, 0 };
    bool rectSet = false;
    int vis = -1;            // last visible / clickable state sent (-1 = unknown)
    int fill = -1;           // 0 = kit_cancel, 1 = kit_cancel_hi, 2 = text fallback
    bool hover = false, tipSet = false;
    bool btn = false;        // B2: control is a real CSWGuiButton clone (leader only); false = label path
};
CW g_cw[3];
struct CWant { bool show = false; qh::Rect r{ 0, 0, 0, 0 }; };
CWant g_want[3];             // set by the draw callback each frame, applied at its end
// QH27: one invisible (alpha 0) LBL_QUEUE1 clone per companion over its strip row, shown while the row has a removable icon, so a
// per-icon click never reaches the world (the leader would walk there). Same build / visible / click-through rules as the Cancel labels.
struct Eat { uintptr_t c = 0; int vis = -1; qh::Rect r{ 0, 0, 0, 0 }; bool rectSet = false; };
Eat g_eat[3];
void eatShow(Eat& e, bool on) {
    if (!e.c) return;
    const int v = on ? 1 : 0;
    if (e.vis == v) return;
    guikit::setClickable(e.c, on);
    guikit::setVisible(e.c, on);
    e.vis = v;
}
void eatHideAll() { for (Eat& e : g_eat) eatShow(e, false); }
guikit::ClickEdge g_lmb;
struct CPending { bool on = false; int n = 0; int had3D = -1; DWORD at = 0; bool late = false; };   // RQ12: had3D = 0x3D (FollowLeader) entries before the Cancel, -1 unreadable
CPending g_cpend[3];
bool g_cancelMissLogged = false;

// VIS log (identical format in 4 mods): `VIS <what> shown=%d why=%s rect=[x,y,w,h]`, INFO on chMain, change-gated by the caller, cap 64 per process.
void visLog(const char* what, bool shown, const char* why, const qh::Rect& r) {
    static int n = 0, nCancel = 0;
    if (!g_logOn) return;
    if (g_cfg.traceMode && strncmp(what, "cancel", 6) == 0) {   // trace_mode: cancel lines get their own mode_cap budget instead of the shared 64
        if (nCancel > g_cfg.modeCap) return;
        if (nCancel++ == g_cfg.modeCap) { KP_LOG(chMain, "VIS cancel cap reached"); return; }
        KP_LOG(chMain, "VIS %s shown=%d why=%s rect=[%d,%d,%d,%d]", what, shown ? 1 : 0, why ? why : "?", r.x, r.y, r.w, r.h);
        return;
    }
    if (n > 64) return;
    if (n == 64) { ++n; KP_LOG(chMain, "VIS cap reached"); return; }
    ++n;
    KP_LOG(chMain, "VIS %s shown=%d why=%s rect=[%d,%d,%d,%d]", what, shown ? 1 : 0, why ? why : "?", r.x, r.y, r.w, r.h);
}
void cwShow(CW& w, bool on, const char* why = "?") {   // clickable only while visible (a shown control swallows the click; hidden = click-through)
    { const int wi = static_cast<int>(&w - g_cw); if (!on && wi >= 0 && wi < 3 && !g_cwWhy[wi]) g_cwWhy[wi] = why; }   // MODE: first hide reason this frame wins
    if (!w.c) return;
    const int v = on ? 1 : 0;
    if (w.vis == v) return;
    guikit::setClickable(w.c, on);
    guikit::setVisible(w.c, on);
    w.vis = v;
    if (!on) { w.hover = false; w.fill = -1; }
    static const char* const kName[3] = { "cancel0", "cancel1", "cancel2" };
    const int idx = static_cast<int>(&w - g_cw);
    visLog(idx >= 0 && idx < 3 ? kName[idx] : "cancel?", on, why, w.r);
}
void cwHideAll(const char* why = "?") { for (CW& w : g_cw) cwShow(w, false, why); }

void cwLookLeader(CW& w) {
    static const float cream[3] = { 0.80f, 0.80f, 0.698f };
    const qh::CancelStyle style = qh::cancelStyle(true);
    const float* tint = w.hover ? style.hoverTint : style.tint;   // border and fill share one tint field: same rgb for both
    const int want = w.hover ? 1 : 0;
    if (w.btn) {   // B2: vanilla Cancel Combat look (thin border art, bright colour, black fill) in the bright companion teal
        if (w.fill != want) {
            guibutton::setButtonText(w.c, "CANCEL QUEUE");
            void* f = guikit::imageFor("uibit_fill_2bt");
            const qh::CancelStyle bs = qh::cancelButtonStyle();
            guikit::setButtonFill(w.c, f, f, w.hover ? bs.hoverTint : bs.tint);
            guikit::setButtonFillStyle(w.c, 2);
            w.fill = want;
        }
        return;
    }
    if (w.fill != want) {
        guikit::setFont(w.c, "dialogfont16x16");
        guikit::setAlign(w.c, 18);
        guikit::setText(w.c, "CANCEL QUEUE");
        guikit::setTextColor(w.c, cream[0], cream[1], cream[2]);
        guikit::setBorderTint(w.c, tint);
        guikit::setAlpha(w.c, style.alpha);
        void* fillImg = guikit::imageFor("uibit_fill_2wt");
        if (fillImg) guikit::setFill(w.c, fillImg, tint);
        w.fill = want;
    }
}

// kit_cancel / kit_cancel_hi untinted; no texture or a refused fill -> a text "X" (once).
void cwLookCompanion(CW& w) {
    static const float tealBorder[3] = { 0.10f, 0.58f, 0.62f };
    static const float tealHover[3]  = { 0.15f, 0.75f, 0.80f };
    const int want = w.hover ? 1 : 0;
    if (w.fill == want || w.fill == 2) return;
    void* img = tex::image(want ? tex::CancelHi : tex::Cancel);
    const float* tint = w.hover ? tealHover : tealBorder;
    if (img && guikit::setFill(w.c, img, tint)) { w.fill = want; return; }
    guikit::setFont(w.c, "dialogfont10x10"); guikit::setAlign(w.c, 18); guikit::setText(w.c, "X");
    w.fill = 2;
    if (!g_cancelMissLogged) { g_cancelMissLogged = true; KP_LOGV(chCancel, KPLOG_WARN, "Cancel art %s: text fallback", img ? "fill refused" : "texture missing"); }
}

void cwLook(CW& w, int k) {
    if (k == 0) cwLookLeader(w);
    else cwLookCompanion(w);
}

// End of every draw callback: place + show the wanted controls (change-gated), hide the rest.
void cwApply() {
    for (int k = 0; k < 3; ++k) {
        CW& w = g_cw[k];
        if (!w.c) continue;
        const CWant& d = g_want[k];
        if (const char* hid = qh::cancelHideWhy(g_cfg.cancel, d.show, d.r.w)) { cwShow(w, false, hid); continue; }
        if (!w.rectSet || w.r.x != d.r.x || w.r.y != d.r.y || w.r.w != d.r.w || w.r.h != d.r.h) {
            if (guikit::setRect(w.c, d.r.x, d.r.y, d.r.w, d.r.h)) { w.r = d.r; w.rectSet = true; }
            else { cwShow(w, false, "strip-slot-missing"); continue; }
        }
        cwLook(w, k);
        cwShow(w, true, "queue");
    }
}

// RAII: ProjectionFlag cleared for the label binds, restored on EVERY path (selfheal's ProjGuard).
struct ProjGuard {
    int* flag; int saved = 0; bool cleared = false;
    ProjGuard() : flag(reinterpret_cast<int*>(gameaddr::ProjectionFlag)) {
        const bool readOk = saferead::probeRead(SiteCancel + 0x10, flag, 4);
        saved = readOk ? *flag : 0;
        if (saved) { if (saferead::probeWrite(SiteCancel + 0x11, flag, 4)) { *flag = 0; cleared = true; } else KP_LOGV(chCancel, KPLOG_WARN, "BUILD: projection flag not writable, labels bind with the aspect projection"); }
    }
    ~ProjGuard() { if (cleared) { if (saferead::probeWrite(SiteCancel + 0x12, flag, 4)) *flag = saved; else KP_LOGV(chCancel, KPLOG_WARN, "BUILD: projection flag restore failed (saved=%d)", saved); } }
};

// One hidden + click-through label from a template; 0 on failure (only that Cancel is disabled).
uintptr_t makeCancelLabel(uintptr_t panel, const char* tmplName, int* whyOut) {
    int why = guikit::CF_None; uint32_t badVt = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tmplName, &why, &badVt);
    *whyOut = why;
    if (!c) return 0;
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {   // no destroy helper in GuiKit: the control stays hidden + click-through and unreferenced
        *whyOut = guikit::CF_Count;
        KP_LOGV(chCancel, KPLOG_WARN, "BUILD append failed for %s, control %08x left hidden + click-through", tmplName, (unsigned)c);
        return 0;
    }
    return c;
}

// Leader Cancel: a BTN_CLEARALL button clone (canary-checked); any failure falls back to the LBL_MOULDING3 label. *isBtn = which one.
uintptr_t makeCancelLeader(uintptr_t panel, int* whyOut, bool* isBtn) {
    *isBtn = false;
    int why = guikit::CF_None; uint32_t badVt = 0; bool canary = false;
    const uintptr_t b = guibutton::createButtonFromTemplate(panel, "BTN_CLEARALL", &why, &badVt, &canary);
    if (b && canary) {
        guikit::setVisible(b, false);
        guikit::setClickable(b, false);
        if (guikit::appendToPanel(panel, b)) { *isBtn = true; *whyOut = guikit::CF_None; return b; }
        KP_LOGV(chCancel, KPLOG_WARN, "BUILD append failed for BTN_CLEARALL, button %08x left hidden + click-through", (unsigned)b);
        return makeCancelLabel(panel, "LBL_MOULDING3", whyOut);   // the unlinked button stays hidden; fall back to the label
    }
    KP_LOGV(chCancel, KPLOG_WARN, "BUILD BTN_CLEARALL clone failed ctl=%08x why=%d badvt=%08x canary=%d: label fallback", (unsigned)b, why, (unsigned)badVt, canary ? 1 : 0);
    if (b) guibutton::releaseUnlinkedButton(b);
    return makeCancelLabel(panel, "LBL_MOULDING3", whyOut);
}

void KPATCH_CALL onHudBuild(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panel) + sizeof(ctx->panel)) return;
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    saferead::beginScope();
    uint32_t vt = 0;
    if (!panel || !rd(SiteCancel, panel, 0, &vt) || vt != gameaddr::HudVtable) {
        if (panel) KP_LOGV(chCancel, KPLOG_WARN, "BUILD skipped: panel %08x vtable %08x != %08x", (unsigned)panel, (unsigned)vt, (unsigned)gameaddr::HudVtable);
        return;
    }
    for (CW& w : g_cw) w = CW();
    for (Eat& e : g_eat) e = Eat();
    for (CWant& d : g_want) d = CWant();
    for (CPending& p : g_cpend) p = CPending();
    g_lmb = guikit::ClickEdge();
    tex::ensure();
    ProjGuard pg;
    int n = 0, why[3] = {};
    for (int k = 0; k < 3; ++k) {
        if (k == 0) g_cw[0].c = makeCancelLeader(panel, &why[0], &g_cw[0].btn);
        else g_cw[k].c = makeCancelLabel(panel, "LBL_QUEUE1", &why[k]);
        if (g_cw[k].c) ++n;
    }
    KP_LOGV(chCancel, n == 3 ? KPLOG_INFO : KPLOG_WARN, "BUILD panel=%08x cancel labels n=%d/3 why=%d,%d,%d btn=%d", (unsigned)panel, n, why[0], why[1], why[2], g_cw[0].btn ? 1 : 0);
    visLog("build", n == 3, n == 3 ? "ok" : "labels-missing", qh::Rect{ 0, 0, 0, 0 });
    int ne = 0, ewhy[3] = {};
    for (int k = 1; k <= 2; ++k) {
        g_eat[k].c = makeCancelLabel(panel, "LBL_QUEUE1", &ewhy[k]);
        if (g_eat[k].c && !guikit::setAlpha(g_eat[k].c, 0.0f)) g_eat[k].c = 0;   // never shown unless it is invisible (stays hidden + click-through)
        if (g_eat[k].c) ++ne;
    }
    KP_LOGV(chCancel, ne == 2 ? KPLOG_INFO : KPLOG_WARN, "BUILD strip click labels n=%d/2 why=%d,%d", ne, ewhy[1], ewhy[2]);
}

void onHudConnect(bool ok, const char* why) { QLOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "hud-host: %s (%s)", ok ? "connected" : "NOT connected (no Cancel controls)", why ? why : ""); }

// Edge log: shown / hidden transitions (never per frame).
struct EdgeState { bool shown = false; uint32_t sig = 0; const char* reason = ""; bool primed = false; };
EdgeState g_edge;
// Set at the top of drawAll: the capture age and menu flag that the hidden edge line reports.
DWORD g_hideAge = 0;
bool g_hideMenu = false;

// Maps a noteHidden reason to the fixed MODE strip vocabulary.
const char* modeStripOf(const char* r) {
    if (!r) return "hidden:empty";
    if (!strcmp(r, "combat")) return "hidden:combat";
    if (!strcmp(r, "menu")) return "hidden:menu";
    if (!strcmp(r, "stale capture") || !strcmp(r, "hud unreadable")) return "hidden:stale";
    return "hidden:empty";   // no creature / empty queue / nothing drawable
}

void noteHidden(const char* reason) {
    g_modeStrip[0] = modeStripOf(reason);
    if (g_edge.primed && !g_edge.shown && g_edge.reason && reason && strcmp(g_edge.reason, reason) == 0) return;
    g_edge.primed = true; g_edge.shown = false; g_edge.reason = reason; g_edge.sig = 0;
    QLOG(chDraw, "hidden (%s) age=%u menu=%d", reason, (unsigned)g_hideAge, g_hideMenu ? 1 : 0);
}

// One drawn leader slot: placed on the HUD control, with its icon and style.
struct Draw { int x, y, w, h; qh::Style st; char rr[17]; };
// The last good leader panel and its creature. Repainted for kPanelHoldMs when the queue reads empty (a quickbar press wipes the engine queue).
struct Held { Draw d[qh::kMaxSlots]; int nd = 0; DWORD tick = 0; uintptr_t creature = 0; };
Held g_heldLeader;

// Paints the leader's background, icons and Cancel from a Draw list. The live draw and the panel hold both come through here.
void paintLeader(uintptr_t hud, const Draw* d, int nd, DWORD age, const K2PsContext* ctx) {
    // Cancel: right of the live QUEUE0 extent, sized by the live QUEUE1 height (read every frame; the HUD scale can change).
    if (qh::cancelVisible(g_cfg.enabled, g_cfg.cancel, hardCombatNow(), ctx->inMenu != 0, age, nd)) {
        qh::Rect q0{}, q1{};
        if (guikit::getExtent(hud + HudSlotBase, &q0.x, &q0.y, &q0.w, &q0.h) && guikit::getExtent(hud + HudSlotBase + HudSlotStride, &q1.x, &q1.y, &q1.w, &q1.h)) {
            g_want[0].r = qh::cancelRect(q0, q1);
            g_want[0].show = g_want[0].r.w > 0;
        }
    }
    gui2d::CapGuard capGuard;
    static const float white[3] = { 1.0f, 1.0f, 1.0f };
    void* bgImg = guikit::imageFor("lbl_miqueue_teal");
    qh::Rect q0 = (nd > 0) ? qh::Rect{ d[0].x, d[0].y, d[0].w, d[0].h } : qh::Rect{};
    if (nd > 0) {
        const char* texName = bgImg ? "teal" : "flat";
        party::Fail pWhy = party::F_None;
        const uintptr_t pTbl = party::table(&pWhy);
        const uintptr_t cli0 = pTbl ? party::member(pTbl, 0) : 0;
        const bool ownerOk = !bufBusy(g_itemQueue[0]) || qh::ownerMatchesSlot(bufOwner(g_itemQueue[0]), cli0, g_heldLeader.creature);
        borderPathTrace(0, "leader", 0, texName, nd, ownerOk);
    }
    if (bgImg && nd > 0) {
        qh::Rect bg{};
        if (!guikit::getExtent(hud + 0x6594, &bg.x, &bg.y, &bg.w, &bg.h) || bg.w <= 0 || bg.h <= 0) {
            bg = qh::combatBgRect(q0);
        }
        gui2d::drawImage(bgImg, bg.x, bg.y, bg.w, bg.h, 0, white, 0.95f);
        for (int i = 0; i < nd; ++i) {
            void* im = guikit::imageFor(d[i].rr);
            if (im) {
                float a = d[i].st.iconA;
                gui2d::drawImage(im, d[i].x, d[i].y, d[i].w, d[i].h, 0, white, a);
            }
        }
    } else {
        for (int i = 0; i < nd; ++i) drawSlot(d[i].x, d[i].y, d[i].w, d[i].h, d[i].st, guikit::imageFor(d[i].rr), false, false);
    }
}

// QL10/QL11 detach (handoff_detach_held): round nodes held OUT of the engine's list (data pointers only; the nodes stay alive, nobody frees them).
struct Detached { uint32_t ptr[qh::kMaxRound] = {}; qh::RNode rn[qh::kMaxRound]; int n = 0; DWORD at = 0; uintptr_t owner = 0; DWORD endAt = 0; };   // rn: copies for the icon draw (QL11)
Detached g_detached[3];
constexpr DWORD kDetachMaxMs = 600000;

// QL11: the vanilla combat queue panel cannot show a round node that handoff_detach_held holds out of the list. Draw it as one more (dimmed, waiting)
// slot on the next unused vanilla slot control, with the icon resolved from the node exactly like a round-list slot (the queue's 0x3F entry + the node).
void drawDetachedSlots(uintptr_t hud, const Snapshot& s, int nVanilla, DWORD now) {
    const Detached& d = g_detached[0];
    static uint32_t lastSig = 0;
    if (!qh::detachedLeaderDrawOk(d.n, d.owner, s.creature, s.round, g_softLatch.pending)) { lastSig = 0; return; }
    int ci = -1;
    for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) if (s.e[i].type == qh::T_COMBAT) { ci = i; break; }
    if (ci < 0) return;
    gui2d::CapGuard capGuard;
    int idx = nVanilla, drawn = 0;
    uint32_t sig = 0x51u;
    for (int i = 0; i < d.n && idx < qh::kMaxSlots; ++i) {
        if (!qh::nodeIconValid(qh::T_COMBAT, d.rn[i].subType)) continue;
        char rr[17] = {};
        if (!resolveIcon(hud, s.round, s.e[ci], s.eData[ci], d.rn[i].ptr, qh::nodeKey(d.rn[i]), now, rr)) continue;
        const uintptr_t ctl = hud + HudSlotBase + static_cast<uintptr_t>(idx) * HudSlotStride;
        int x = 0, y = 0, w = 0, h = 0;
        if (!guikit::getExtent(ctl, &x, &y, &w, &h) || w < 1 || h < 1 || w > 2000 || h > 2000) continue;
        drawSlot(x, y, w, h, qh::slotStyle(g_cfg, idx, true), guikit::imageFor(rr), false, false);
        sig = sig * 131u + d.rn[i].ptr + static_cast<uint32_t>(idx);
        ++idx; ++drawn;
    }
    if (drawn > 0) {
        borderPathTrace(1, "detached", 0, "flat", drawn, d.owner == s.creature);
    }
    if (drawn > 0 && sig != lastSig) { lastSig = sig; QLOG(chDraw, "detached icons drawn=%d first_slot=%d", drawn, nVanilla); }
}

// The leader's panel over the hidden vanilla slots (unchanged from 0.1.0). The caller already checked enabled / combat / menu / stale / hud.
void drawLeader(uintptr_t hud, DWORD now, DWORD age, const K2PsContext* ctx) {
    const bool hardCombat = hardCombatNow();   // soft combat (ally-only targets) draws the strip like out of combat
    if (hardCombat) {
        g_modeHidden[0] = 0;
        if (g_detached[0].n > 0 && ctx->inMenu == 0 && age < 250u) {
            Snapshot s;
            if (gather(s)) {
                qh::Slot slots[qh::kMaxSlots];
                const int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
                drawDetachedSlots(hud, s, n, now);
            }
        }
        noteHidden("combat");
        return;
    }
    Snapshot s;
    if (!gather(s)) { noteHidden("no creature"); return; }
    Held& held = g_heldLeader;
    if (held.creature != s.creature) { held.nd = 0; held.tick = 0; held.creature = s.creature; }   // a Tab switch never shows the old queue
    qh::Slot slots[qh::kMaxSlots];
    int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
    g_modeHidden[0] = qh::hiddenSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr);   // RL20
    bool tailHid = false;
    if (!g_combat && qh::inCombatTail(g_combatEndTick, now)) {   // QH25: the engine's last combat round is not ours to draw teal
        const int kept = qh::dropTailSlots(slots, n, s.e, s.engineNe);
        if (kept == n) { QLOG(chDraw, "combat tail ended (%u ms after combat)", (unsigned)(now - g_combatEndTick)); g_combatEndTick = 0; }
        else { tailHid = kept == 0; n = kept; }
    }
    if (!qh::shouldShow(g_cfg.enabled, hardCombat, ctx->inMenu != 0, age, n)) {
        if (n == 0 && !tailHid && held.nd > 0 && qh::panelHold(held.tick, now, ctx->inMenu != 0, hardCombat)) { g_modeStrip[0] = "shown"; paintLeader(hud, held.d, held.nd, age, ctx); return; }
        noteHidden(tailHid ? "combat tail" : "empty queue");
        return;
    }

    Draw d[qh::kMaxSlots];
    int nd = 0;
    uint32_t sig = static_cast<uint32_t>(n) * 31u + (s.waiting ? 7u : 0u);
    for (int i = 0; i < n; ++i) {
        const qh::Slot& sl = slots[i];
        sig = sig * 131u + static_cast<uint32_t>(s.e[sl.entry].type) + (static_cast<uint32_t>(sl.node + 3) << 8);
        uintptr_t node = 0;
        if (sl.node == qh::NODE_CUR) node = s.cur.ptr;
        else if (sl.node >= 0 && sl.node < s.nrn) node = s.rn[sl.node].ptr;
        char rr[17] = {};
        if (!resolveIcon(hud, s.round, s.e[sl.entry], s.eData[sl.entry], node, sl.key, now, rr)) continue;
        const uintptr_t ctl = hud + HudSlotBase + static_cast<uintptr_t>(nd) * HudSlotStride;   // compact: a skipped icon leaves no gap
        int x = 0, y = 0, w = 0, h = 0;
        if (!guikit::getExtent(ctl, &x, &y, &w, &h) || w < 1 || h < 1 || w > 2000 || h > 2000) {
            static uint32_t seen[8]; static int ns = 0;
            if (firstTime(seen, ns, 8, 0x100u + static_cast<uint32_t>(nd))) QLOGV(chDraw, KPLOG_WARN, "slot %d control %08x extent invalid (%d,%d,%d,%d): not drawn", nd, (unsigned)ctl, x, y, w, h);
            continue;
        }
        Draw& dd = d[nd++];
        dd.x = x; dd.y = y; dd.w = w; dd.h = h; dd.st = qh::slotStyle(g_cfg, nd - 1, s.waiting);
        memcpy(dd.rr, rr, 17);
    }
    if (nd == 0) { noteHidden("nothing drawable"); return; }
    g_modeStrip[0] = "shown";
    if (!g_edge.shown || g_edge.sig != sig) {
        char types[64]; int len = 0;
        for (int i = 0; i < n && len < 56; ++i) len += snprintf(types + len, sizeof types - len, "%s0x%02X/%d", i ? "," : "", s.e[slots[i].entry].type, slots[i].node);
        types[len] = 0;
        QLOG(chDraw, "%s slots=%d drawn=%d entries=%d round=%d cur=%d waiting=%d paused=%d [%s] slot0=(%d,%d,%d,%d) hudScale=%.2f",
             g_edge.shown ? "changed" : "shown", n, nd, s.ne, s.nrn, s.haveCur ? 1 : 0, s.waiting ? 1 : 0, s.paused ? 1 : 0, types, d[0].x, d[0].y, d[0].w, d[0].h, gui2d::hudScale());
        g_edge.primed = true; g_edge.shown = true; g_edge.sig = sig; g_edge.reason = "";
    }
    held.nd = nd; held.tick = now;   // only a live draw refreshes the hold clock
    memcpy(held.d, d, sizeof(Draw) * nd);
    paintLeader(hud, held.d, held.nd, age, ctx);
}


// ---- companion strips (0.2.0): party members 1-2 ----
struct StripEdge { bool primed = false, shown = false; uint32_t sig = 0; };
StripEdge g_strip[3];

void stripNote(int k, bool shown, int n, const char* types, const qh::Rect& r, uint32_t sig) {
    StripEdge& e = g_strip[k];
    if (e.primed && e.shown == shown && (!shown || e.sig == sig)) return;
    e.primed = true; e.shown = shown; e.sig = sig;
    QLOG(chDraw, "strip k=%d %s n=%d types=[%s] rect=(%d,%d,%d,%d)", k, shown ? "shown" : "hidden", n, types, r.x, r.y, r.w, r.h);
}
void stripsHidden(const char* modeWhy = "hidden:stale") {
    const qh::Rect none{ 0, 0, 0, 0 };
    for (int k = 1; k <= 2; ++k) { g_modeStrip[k] = modeWhy; stripNote(k, false, 0, "", none, 0); }
}

// Down companions (0.4.1): a party member at HP <= 0 shows no strip and its queue journal is not polled. Vitals come from party::vitals,
// refreshed at most every kDownSampleMs per member or when its server creature changes; server 0 is never down.
constexpr DWORD kDownSampleMs = 100;
struct DownCache { DWORD tick = 0; bool known = false; bool down = false; uintptr_t server = 0; };
DownCache g_down[3];

bool memberDown(int k, uintptr_t server, DWORD now) {
    if (!server || k < 0 || k > 2) return false;
    DownCache& c = g_down[k];
    if (!c.known || c.server != server || now - c.tick >= kDownSampleMs) {
        party::Vitals v;
        party::vitals(server, &v);
        c.known = true; c.server = server; c.tick = now;
        c.down = qh::stripHiddenForDown(v.ok, v.down);
    }
    return c.down;
}

struct StripDraw { qh::Rect r[qh::kMaxSlots]; qh::Style st[qh::kMaxSlots]; char rr[qh::kMaxSlots][17]; int n = 0; bool have = false; int cancelSq = 0; };
// Last good strip per party index k (1-2): repainted for kPanelHoldMs on an empty queue, with its edge signature and type text so the log edge does not flip.
struct HeldStrip { StripDraw d; DWORD tick = 0; uintptr_t creature = 0; uint32_t sig = 0; char types[48] = {}; };
HeldStrip g_heldStrip[3];
// QH27: what each drawn strip icon of the last HUD frame stands for. Strips draw in HUD_DRAW, clicks are handled in FRAME_BEGIN (next frame).
struct StripPick { qh::SlotKind kind = qh::SK_ENGINE; int bufIdx = -1; uint32_t cmd = 0; uint32_t node = 0; qh::QueuedAction drawn; };
struct StripHit { qh::Rect r[qh::kMaxSlots]; StripPick p[qh::kMaxSlots]; int n = 0; uintptr_t creature = 0; };
StripHit g_stripHit[3];
int g_stripHover[3] = { -1, -1, -1 };

void drawStrips(uintptr_t hud, DWORD now, bool inMenu) {
    const K2AddrTable* T = k2addr::get();
    if (!T || !T->Slot_VitRect || !T->Slot_CardRect) { g_modeStrip[1] = g_modeStrip[2] = "hidden:stale"; return; }   // unknown exe: no geometry, nothing to anchor to
    g_modeStrip[1] = g_modeStrip[2] = "hidden:empty";   // refined below: shown / stale geometry
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    uintptr_t cli[3] = {}, srv[3] = {};
    if (tbl) for (int k = 1; k <= 2; ++k) { cli[k] = party::member(tbl, k); srv[k] = cli[k] ? party::serverCreature(cli[k]) : 0; }
    bool down[3] = {};
    for (int k = 1; k <= 2; ++k) down[k] = memberDown(k, srv[k], now);   // a Down companion shows no strip (its engine pushes flicker the held repaint)
    int screenW = 0;
    if (const uintptr_t sw = k2addr::addr(&K2AddrTable::ScreenW)) rd(SiteStrip + 1, sw, 0, &screenW);
    if (screenW < 320 || screenW > 16384) screenW = 0;   // unknown: no right-edge clamp
    StripDraw d[3];
    char types[3][48] = {};
    uint32_t sigs[3] = {};
    for (int i = 0; i < K2PS_MAX_SLOTS; ++i) {
        int32_t slot = 0, cr = 0;
        if (!g_ps.slotAt(i, &slot, &cr) || !slot || !cr) continue;
        int k = 0;
        for (int kk = 1; kk <= 2 && !k; ++kk)   // the slot's creature is the client or the server pointer (seen both ways); the leader (0) never matches
            if (cli[kk] && srv[kk] && (static_cast<uintptr_t>(static_cast<uint32_t>(cr)) == cli[kk] || static_cast<uintptr_t>(static_cast<uint32_t>(cr)) == srv[kk])) k = kk;
        if (!k || d[k].have) continue;
        if (down[k]) { g_heldStrip[k] = HeldStrip(); continue; }   // Down: no gather and no held repaint; a stale held strip cannot flash on wake
        int vr[4] = {}, cd[4] = {};
        bool ok = true;
        for (int j = 0; j < 4; ++j) {
            ok = rd(SiteStrip + 2, static_cast<uintptr_t>(static_cast<uint32_t>(slot)), T->Slot_VitRect + 4 * j, &vr[j]) && ok;
            ok = rd(SiteStrip + 3, static_cast<uintptr_t>(static_cast<uint32_t>(slot)), T->Slot_CardRect + 4 * j, &cd[j]) && ok;
        }
        if (!ok || vr[2] < 1 || vr[2] > 400 || vr[3] < 1 || vr[3] > 400 || cd[2] < 1 || cd[2] > 2000 || cd[3] < 1 || cd[3] > 2000) { g_modeStrip[k] = "hidden:stale"; continue; }
        Snapshot s;
        if (!gatherFor(srv[k], s)) continue;
        qh::Slot slots[qh::kMaxSlots];
        const int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
        g_modeHidden[k] = qh::hiddenSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr);   // RL20
        HeldStrip& hs = g_heldStrip[k];
        if (hs.creature != srv[k]) { hs.d = StripDraw(); hs.tick = 0; hs.creature = srv[k]; }   // a Tab switch never shows the old strip
        if (n == 0 && hs.d.have && qh::panelHold(hs.tick, now, inMenu, hardCombatNow() && !g_cfg.showInCombat)) {
            d[k] = hs.d;   // hold repaint: Cancel below and the draw loop use the held rects; hs.tick is not refreshed
            memcpy(types[k], hs.types, sizeof types[k]);
            sigs[k] = hs.sig;
            continue;
        }
        const int cap = g_cfg.stripMax < qh::kMaxSlots ? g_cfg.stripMax : qh::kMaxSlots;
        StripDraw& sd = d[k];
        uint32_t sig = static_cast<uint32_t>(n) * 31u + (s.waiting ? 7u : 0u);
        int len = 0;
        StripPick picks[qh::kMaxSlots];
        const qh::ActionBuffer& ib = g_itemQueue[k];
        for (int j = 0; j < n && sd.n < cap; ++j) {
            const qh::Slot& sl = slots[j];
            uintptr_t node = 0;
            if (sl.node == qh::NODE_CUR) node = s.cur.ptr;
            else if (sl.node >= 0 && sl.node < s.nrn) node = s.rn[sl.node].ptr;
            char rr[17] = {};
            if (!resolveIcon(hud, s.round, s.e[sl.entry], s.eData[sl.entry], node, sl.key, now, rr)) continue;
            memcpy(sd.rr[sd.n], rr, 17);
            StripPick& pk = picks[sd.n];
            pk.kind = qh::slotKind(sl, s.engineNe, s.activeShown, &pk.bufIdx);
            if (pk.kind == qh::SK_ACTIVE) { pk.drawn = ib.activeAction; pk.cmd = pk.drawn.cmdId; }
            else if (pk.kind == qh::SK_BUFFERED) { if (pk.bufIdx >= 0 && pk.bufIdx < ib.count) { pk.drawn = ib.queue[pk.bufIdx]; pk.cmd = pk.drawn.cmdId; } else pk.kind = qh::SK_ENGINE; }
            else if (pk.kind == qh::SK_ROUND) pk.node = static_cast<uint32_t>(node);
            sd.st[sd.n] = qh::slotStyle(g_cfg, sd.n, s.waiting);
            ++sd.n;
            sig = sig * 131u + static_cast<uint32_t>(s.e[sl.entry].type) + (static_cast<uint32_t>(sl.node + 3) << 8);
            if (len < 40) len += snprintf(types[k] + len, sizeof types[k] - len, "%s0x%02X/%d", len ? "," : "", s.e[sl.entry].type, sl.node);
        }
        if (len > 0 && len < static_cast<int>(sizeof types[k])) types[k][len] = 0;
        if (sd.n == 0) {
            if (n > 0) skipTrace(6 + (k - 1), "strip_icon", k, bufBusy(ib), bufBusy(ib) ? bufHead(ib).cmdId : 0u, n);
            continue;
        }
        skipTraceClear(6 + (k - 1));
        const qh::Rect card{ cd[0], cd[1], cd[2], cd[3] }, vit{ vr[0], vr[1], vr[2], vr[3] };
        // Left side: the selfheal footprint comes from the hud-host strip slots (selfheal registers order 0, this row is order 1); without
        // hud-host / selfheal / StripSlot it is the same value computed locally, so the layout is identical either way.
        int reserve = shui::stripReserve(shui::Rect{ vit.x, vit.y, vit.w, vit.h });
        if (g_cfg.stripSide == qh::kSideLeft) {
            const int rowW = qh::stripRowWidth(vit, sd.n, g_cfg), rowH = qh::stripHeadSize(vit, g_cfg);
            K2HudRect sr{};
            if (g_hudc.stripSlot("queue-hud", 1, K2HudRect{ vit.x, vit.y, vit.w, vit.h }, rowW, rowH, &sr)) reserve = vit.x - sr.x - rowW;
        }
        sd.n = qh::stripRects(card, vit, sd.n, g_cfg, sd.r, screenW, reserve);
        if (sd.n == 0) {
            skipTrace(8 + (k - 1), "strip_rect", k, bufBusy(ib), bufBusy(ib) ? bufHead(ib).cmdId : 0u, n);
            continue;
        }
        skipTraceClear(8 + (k - 1));
        sd.have = true;
        StripHit& sh = g_stripHit[k];
        for (int j = 0; j < sd.n; ++j) { sh.r[j] = sd.r[j]; sh.p[j] = picks[j]; }
        sh.n = sd.n; sh.creature = srv[k];
        sd.cancelSq = qh::stripIconSize(vit, g_cfg);   // the Cancel X keeps the plain strip icon size even when the head is bigger
        sigs[k] = sig * 131u + static_cast<uint32_t>(sd.r[0].x) * 7u + static_cast<uint32_t>(sd.r[0].y) * 13u + static_cast<uint32_t>(sd.r[0].w);
        hs.d = sd; hs.tick = now; hs.sig = sigs[k]; memcpy(hs.types, types[k], sizeof hs.types);   // live draw: refresh the hold
    }
    bool any = false;
    for (int k = 1; k <= 2; ++k) {
        if (d[k].have) {
            any = true; g_modeStrip[k] = "shown"; stripNote(k, true, d[k].n, types[k], d[k].r[0], sigs[k]);
            const char* texName = (g_cfg.kit && tex::image(tex::Frame)) ? "kit_frame" : "flat";
            const bool ownerOk = !bufBusy(g_itemQueue[k]) || qh::ownerMatchesSlot(bufOwner(g_itemQueue[k]), cli[k], srv[k]);
            borderPathTrace(2 + (k - 1), "strip", k, texName, d[k].n, ownerOk);
        } else {
            stripNote(k, false, 0, "", qh::Rect{ 0, 0, 0, 0 }, 0);
        }
    }
    if (!any) return;
    if (g_cfg.cancel && !hardCombatNow())   // Cancel stays hidden in real combat; soft combat (ally-only targets) shows it like out of combat
        for (int k = 1; k <= 2; ++k)
            if (d[k].have) { g_want[k].r = qh::stripCancelRect(d[k].r, d[k].n, g_cfg.stripSide, screenW, d[k].cancelSq); g_want[k].show = true; }   // RL18: a strip with no room for the square is "no-room" in cwApply, not "no-queue"
    gui2d::CapGuard capGuard;
    for (int k = 1; k <= 2; ++k)
        for (int j = 0; d[k].have && j < d[k].n; ++j) drawSlot(d[k].r[j].x, d[k].r[j].y, d[k].r[j].w, d[k].r[j].h, d[k].st[j], guikit::imageFor(d[k].rr[j]), j < g_stripHit[k].n && g_stripHover[k] == j && qh::slotRemovable(g_stripHit[k].p[j].kind));
}

void drawAll(const K2PsContext* ctx) {
    saferead::beginScope();
    const DWORD now = GetTickCount();
    const DWORD age = now - g_capTick;
    g_hideAge = age; g_hideMenu = ctx->inMenu != 0;
    const uintptr_t hud = g_hud;
    uint32_t probe = 0;
    if (!hud || !rd(SiteHud, hud, HudSlotBase, &probe)) { g_modeStrip[0] = "hidden:stale"; if (hud) noteHidden("hud unreadable"); if (g_cfg.companions) stripsHidden(); return; }
    if (!qh::shouldShow(g_cfg.enabled, hardCombatNow(), ctx->inMenu != 0, age, 1, g_cfg.showInCombat)) {
        noteHidden(hardCombatNow() ? "combat" : ctx->inMenu ? "menu" : "stale capture");
        if (g_cfg.companions) stripsHidden(g_modeStrip[0]);   // same verdict as the leader
        return;
    }
    drawLeader(hud, now, age, ctx);
    if (g_cfg.companions) drawStrips(hud, now, ctx->inMenu != 0);
}

// MODE trace: one line per party member whenever its snapshot changes (combat/soft/menu/paused, hostile/party targets, strip / cancel / banner verdict).
// Called after cwApply (HUD frame) and from FRAME_BEGIN in a menu frame (the HUD draw does not run there). Read-only; formats only on change.
void modeEmit(bool inMenu) {
    if (!g_logOn || !g_cfg.traceQueue || !g_cfg.traceMode) return;
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const bool inCombat = g_combat != 0 && !g_combatUnreadable;
    const int8_t paused = playerPaused() ? 1 : 0;
    const char* banner = g_bannerHidden ? "hidden" : g_bannerRestored ? "restored" : "vanilla";
    for (int k = 0; k < 3; ++k) {
        qh::ModeState& n = g_modeNow[k];
        if (!tbl || !party::member(tbl, k)) { g_modePrimed[k] = false; continue; }   // no such member: no line
        n.combat = g_combatUnreadable ? -1 : (g_combat != 0 ? 1 : 0);
        n.soft = inCombat ? (g_softCombat ? 1 : 0) : -1;
        n.menu = inMenu ? 1 : 0;
        n.paused = paused;
        n.hostile = inCombat && g_modeCandOk[k] ? (g_modeCand[k].anyHostile ? 1 : 0) : -1;
        n.party = inCombat && g_modeCandOk[k] ? (g_modeCand[k].anyParty ? 1 : 0) : -1;
        n.strip = g_modeStrip[k] ? g_modeStrip[k] : "hidden:empty";
        const CW& w = g_cw[k];
        if (w.c && w.vis == 1) n.cancel = "shown";
        else if (!g_cwWhy[k]) n.cancel = "hidden:empty";
        else if (!strcmp(g_cwWhy[k], "cancel-off")) n.cancel = "hidden:off";
        else if (!strcmp(g_cwWhy[k], "combat")) n.cancel = "hidden:combat";
        else if (!strcmp(g_cwWhy[k], "hud-inactive")) n.cancel = "hidden:menu";
        else if (!strcmp(g_cwWhy[k], "strip-slot-missing")) n.cancel = "hidden:stale";
        else if (!strcmp(g_cwWhy[k], "no-room")) n.cancel = "hidden:no-room";
        else if (!strcmp(g_cwWhy[k], "stale capture")) n.cancel = "hidden:stale";
        else n.cancel = "hidden:empty";   // no-queue
        n.banner = banner;
        n.hidden = static_cast<int8_t>(g_modeHidden[k] > 99 ? 99 : g_modeHidden[k]);   // RL20
        if (g_modePrimed[k] && !qh::modeChanged(g_modePrev[k], n)) continue;
        if (g_modeLines < g_cfg.modeCap) {
            char line[200];
            qh::fmtMode(line, sizeof line, k, n, g_modePrimed[k] ? qh::modeWhy(g_modePrev[k], n) : "ui");
            QTRACE("%s", line);
            ++g_modeLines;
        } else if (g_modeLines == g_cfg.modeCap) { QTRACE("MODE cap reached"); ++g_modeLines; }
        g_modePrev[k] = qh::modeStored(g_modePrev[k], g_modePrimed[k], n); g_modePrimed[k] = true;   // RL13
    }
    for (const char*& r : g_cwWhy) r = nullptr;
}

// QH27: after the strips are drawn, cover each row that has a removable icon (bounds of the whole row, gaps included); hide the rest.
void eatApply() {
    for (int k = 1; k <= 2; ++k) {
        Eat& e = g_eat[k];
        const StripHit& sh = g_stripHit[k];
        bool any = false;
        for (int j = 0; j < sh.n; ++j) any = any || qh::slotRemovable(sh.p[j].kind);
        const qh::Rect b = any && g_cfg.cancel ? qh::stripBounds(sh.r, sh.n) : qh::Rect{ 0, 0, 0, 0 };
        if (!e.c || b.w <= 0) { eatShow(e, false); continue; }
        if (!e.rectSet || e.r.x != b.x || e.r.y != b.y || e.r.w != b.w || e.r.h != b.h) {
            if (guikit::setRect(e.c, b.x, b.y, b.w, b.h)) { e.r = b; e.rectSet = true; }
            else { eatShow(e, false); continue; }
        }
        eatShow(e, true);
    }
}

// HUD_DRAW prio 400: the wanted Cancel rects are collected by drawLeader / drawStrips, then applied (every other path hides them).
void KPATCH_CALL onHudDraw(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    for (CWant& d : g_want) d = CWant();
    g_modeStrip[0] = "hidden:empty";
    g_modeStrip[1] = g_modeStrip[2] = g_cfg.companions ? "hidden:empty" : "hidden:off";
    g_modeHidden[0] = g_modeHidden[1] = g_modeHidden[2] = 0;   // RL20
    for (StripHit& h : g_stripHit) h = StripHit();   // QH27: only strips drawn live this frame are clickable (a hold repaint is an empty queue)
    drawAll(ctx);
    cwApply();
    eatApply();
    modeEmit(ctx->inMenu != 0);
    g_modeDrew = true;
}

// ---- Cancel click (FRAME_BEGIN: the UI tick, outside the draw batch) ----
// Queue entries of party member k (0 leader, 1-2 companions), -1 when the creature is unreadable.
// *haveCur (optional) gets curReal() of the same snapshot (staleCurSince of buffer k, 0 when k is out of range). *gateNe (optional) gets the dispatch-gate count (-1 unreadable); *n3D (optional) gets the FollowLeader (0x3D) entry count (-1 unreadable);
// the return value stays the raw count (logs).
int queueCountOf(int k, bool* haveCur = nullptr, int* gateNe = nullptr, int* n3D = nullptr) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    Snapshot s;
    const bool ok = gatherFor(m ? party::serverCreature(m) : 0, s);
    if (haveCur) {
        // Same judgement as tickItemQueues: a flagless item/spell round node stale for kStaleCurMs is not "running" (curReal), so it must not force a buffer.
        const uint32_t stale = (k >= 0 && k < 3) ? g_itemQueue[k].staleCurSince : 0;
        *haveCur = ok && qh::curReal(s.haveCur, s.haveCur ? static_cast<int>(s.cur.subType) : -1, s.aa8, s.ab8, s.paused, stale, GetTickCount());
    }
    if (gateNe) *gateNe = ok ? s.gateNe : -1;
    if (n3D) { int c = 0; if (ok) for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) if (s.e[i].type == 0x3D) ++c; *n3D = ok ? c : -1; }   // RQ12: 0x3D = AIActionFollowLeader
    return ok ? s.engineNe : -1;
}

// Server-side ids for ServerClearAllActions(serverObjectId) on one member's creature: id = CrObjectId, server = *(*AppGlobal + AppToServer).
// Returns nullptr when both resolve, else the refusal reason. Shared by doCancel and the combat hand-off clear (handoffTick).
static const char* resolveServerClear(uintptr_t srv, uint32_t& id, uint32_t& server) {
    uint32_t app = 0;
    if (!rd(SiteCancel + 1, srv, CrObjectId, &id) || !id) return "server object id unreadable";
    if (!rd(SiteCancel + 2, party::appGlobalAddr(), 0, &app) || !app || !rd(SiteCancel + 3, app, AppToServer, &server) || !server) return "server app unreadable";
    return nullptr;
}

// Route per R7: CServerExoApp::ClearAllActions(serverObjectId), ECX = *(*AppGlobal + 8). Every pointer is saferead-probed first
// (the codebase has no SEH call guard); nothing is called unless all of them resolve.
void doCancel(int k) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    const uintptr_t srv = m ? party::serverCreature(m) : 0;
    uint32_t id = 0, server = 0;
    const char* bad = nullptr;
    if (!srv) bad = "no server creature";
    else bad = resolveServerClear(srv, id, server);
    if (!bad && GetTickCount() - g_capTick >= 250u) bad = "stale HUD capture";
    if (bad) { KP_LOGV(chCancel, KPLOG_WARN, "CANCEL m=%d act=refused n=-1 why=%s", k, bad); return; }
    int had3D = -1;
    const int n = queueCountOf(k, nullptr, nullptr, &had3D);
    const int drawn = k == 0 ? g_heldLeader.nd : g_heldStrip[k].d.n;   // read BEFORE the RL17 reset below
    QLOG(chCancel, "CANCEL m=%d act=click n=%d drawn=%d had3D=%d route=ServerClearAll why=measure", k, n, drawn, had3D);
    if (g_logOn && g_cfg.traceQueue) {
        uint32_t ids[qh::kMaxBufferCap + 1];
        const int ni = qh::bufferedCmdIds(g_itemQueue[k], ids, qh::kMaxBufferCap + 1);
        for (int i = 0; i < ni; ++i) {
            char line[200];
            qh::fmtDrop(line, sizeof line, ids[i], k, qh::DR_CANCEL, n);
            QTRACE("%s", line);
        }
    }
    g_lastOurs[k].tag = "cancel"; g_lastOurs[k].tick = GetTickCount();
    g_itemQueue[k].cancel();
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);   // unchanged: vanilla cleanup (RQ12 is measure-first)
    if (k == 0) g_heldLeader.nd = 0; else g_heldStrip[k] = HeldStrip();   // RL17: the hold repaint must not keep cancelled icons (and the Cancel control) for 400 ms
    g_cpend[k].on = true; g_cpend[k].n = n; g_cpend[k].had3D = had3D; g_cpend[k].at = GetTickCount(); g_cpend[k].late = false;
}

// CMD# trace line for the H9/H10 hooks (one per intercepted command that is not our own re-entry). No-op unless trace_queue is on.
// m < 0 / cr only: the pass_combat and pass_unreadable exits, where nothing beyond the creature pointer is known.
void traceCmd(uint32_t cmd, int m, uintptr_t cr, qh::ActionKind kind, bool known, uint32_t id, uint32_t tgt, const char* dec,
              bool fb, int engq, int buf, bool combat) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    qh::CmdLine c;
    c.cmdId = cmd; c.m = m; c.cr = cr;
    if (known) { c.kind = static_cast<int>(kind); c.hasId = true; c.id = id; c.hasTgt = true; c.tgt = tgt; }
    c.dec = dec; c.fb = fb; c.engq = engq; c.buf = buf;
    c.paused = known ? playerPaused() : false;
    c.combat = combat;
    char line[200];
    qh::fmtCmd(line, sizeof line, c);
    QTRACE("%s", line);
}

// ---- queue trace helpers (EDGE / DROP / WAIT / QJ). Everything is edge-triggered or rate-capped; idle = zero lines. ----
void bufCounts(int* out) { for (int k = 0; k < 3; ++k) out[k] = g_itemQueue[k].count; }

void traceEdge(const char* what, bool oldOn, bool newOn, const char* src) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    int b[3];
    bufCounts(b);
    char line[200];
    qh::fmtEdge(line, sizeof line, what, oldOn, newOn, src, b, 3);
    QTRACE("%s", line);
}

// One DROP per command held by member k (buffered + active), n = how many were lost. Call BEFORE the buffer is cleared.
void traceDropAll(int k, qh::DropWhy why) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    uint32_t ids[qh::kMaxBufferCap + 1];
    const int ni = qh::bufferedCmdIds(g_itemQueue[k], ids, qh::kMaxBufferCap + 1);
    for (int i = 0; i < ni; ++i) {
        char line[200];
        qh::fmtDrop(line, sizeof line, ids[i], k, why, ni);
        QTRACE("%s", line);
    }
}

// Member slot (0..2) that holds creature cr (client or server creature), or -1. Same match rule as the hooks' slot lookup.
static int slotOfCreature(uintptr_t tbl, uintptr_t cr) {
    if (!cr) return -1;
    for (int k = 0; k < 3; ++k) {
        const uintptr_t m = party::member(tbl, k);
        if (m && (m == cr || party::serverCreature(m) == cr)) return k;
    }
    return -1;
}

// B1 + QL20: may action a be sent now? Slot 0 always; members 1-2 only through the server route (a Force power on an object target).
static bool mayDispatch(uintptr_t tbl, const qh::QueuedAction& a) {
    const int slot = slotOfCreature(tbl, a.creature);
    if (!qh::ownerMayDispatch(slot, g_cfg.companionServerCast && qh::serverRouteEligible(a))) return false;
    if (slot <= 0) return true;   // the leader's client route is unchanged
    const uintptr_t cli = party::member(tbl, slot);
    return !memberDown(slot, cli ? party::serverCreature(cli) : 0, GetTickCount());   // RL9: a Down companion neither casts nor re-injects (RI_NOT_CTRL / DS_NOT_CTRL); revival releases the hold
}

// Clears the active action and every per-action eaten flag (queued commands stay in the buffer).
static void endAction(qh::ActionBuffer& b) {
    b.active = false;
    b.seenInQueue = false;
    b.eaten = false;
    b.eatenReInjects = 0;
    b.eatenUnpausedMs = 0;
    b.dispatchDeferred = false;
    b.deferUnpausedMs = 0;
}

// VANISH line: the active action left the engine without a RESOLVE (executed, unseen, eaten). No-op unless trace_queue is on.
static void traceVanish(int k, qh::VanishKind kind, const qh::ActionBuffer& buf, const Snapshot& s, const qh::EngineView& ev, DWORD now) {
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtVanish(line, sizeof line, buf.activeAction.cmdId, k, kind, s.paused, ev.executing, s.engineNe, s.nrn,
                      s.haveCur ? (int)s.cur.subType : -1, s.aa8 != 0, s.ab8 != 0, now - buf.activeAction.queueTime);
        QTRACE("%s", line);
    }
    g_lastOurs[k].tag = "vanish"; g_lastOurs[k].tick = GetTickCount();
}

// RESOLVE line, then clear the active action (normal completion, not an abandon).
static void resolveActive(int k, qh::ActionBuffer& buf, DWORD now, const char* why = nullptr) {
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtResolve(line, sizeof line, buf.activeAction.cmdId, k, buf.seenInQueue, buf.activeAction.reInjectCount,
                       now - buf.activeAction.queueTime, why);
        QTRACE("%s", line);
    }
    g_lastOurs[k].tag = "resolve"; g_lastOurs[k].tick = GetTickCount();
    endAction(buf);
}

void spellWatchArm(int k, uint32_t cmd, DWORD now);   // SPELL_OUT watch (defined with the HANDOFF_OUT watch below)

// QL20: run a Force power on the NON-controlled member `slot` through its own server creature. Every pointer is saferead first.
enum class SendResult : uint8_t { Sent, Retry, Refused };   // Refused = permanent
static SendResult serverCast(const qh::QueuedAction& a, int slot, uintptr_t tbl, bool retry) {
    const uintptr_t cli = party::member(tbl, slot);
    const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
    uint32_t commandable = 0, mask = 0, tgt = 0;
    const char* bad = nullptr;
    bool perm = false;
    if (!srv) bad = "no_server_creature";
    else if (!rd(SiteSrvCast, srv, CrCommandable, &commandable) || !commandable) bad = "not_commandable";
    else if (!rd(SiteSrvCast + 1, gameaddr::ServerIdMask, 0, &mask)) { bad = "id_mask"; perm = true; }
    else tgt = static_cast<uint32_t>(a.arg2) & ~mask;
    if (!bad) {
        spellWatchArm(slot, a.cmdId, GetTickCount());
        g_dispatching = true;
        const int r = reinterpret_cast<UseTalentOnObjectFn>(AddrUseTalentOnObject)(srv, 0, a.spellId, tgt, a.multispell & 0xFF,
                                                                                     qh::kInvalidObjectId, 0xFFFFFFFFu, 0, a.metamagic & 0xFF);
        g_dispatching = false;
        if (!r) bad = "refused";
    }
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtServerCast(line, sizeof line, a.cmdId, slot, a.spellId, tgt, bad == nullptr, bad);
        QTRACE("%s", line);
    }
    if (bad && !retry) QLOGV(chMain, KPLOG_WARN, "server-route cast refused for member %d cmd=%u why=%s", slot, (unsigned)a.cmdId, bad);
    if (!bad) return SendResult::Sent;
    return perm ? SendResult::Refused : SendResult::Retry;
}

// Issue one queued command to the engine (our own call: the hooks pass it through via g_dispatching). k = party member (-1 unknown).
// QL20: the client wrappers act on the CONTROLLED creature, so an action owned by member 1-2 goes through serverCast or is not sent at all.
SendResult dispatchAction(const qh::QueuedAction& a, int k, bool retry = false) {
    {
        party::Fail pf = party::F_None;
        const uintptr_t tbl = party::table(&pf);
        const int slot = tbl ? slotOfCreature(tbl, a.creature) : -1;
        if (slot != 0) {
            if (slot > 0 && g_cfg.companionServerCast && qh::serverRouteEligible(a)) return serverCast(a, slot, tbl, retry);
            if (g_logOn && g_cfg.traceQueue) {
                char line[200];
                qh::fmtServerCast(line, sizeof line, a.cmdId, slot, a.spellId, 0, false, "not_ctrl");
                QTRACE("%s", line);
            }
            return SendResult::Sent;
        }
    }
    g_dispatching = true;
    if (a.kind == qh::ActionKind::Spell) {
        const qh::ReplayArgs ra = qh::buildReplayArgs(a);
        if (g_logOn && g_cfg.traceQueue) {
            char line[200];
            qh::fmtSpellArgs(line, sizeof line, true, a.cmdId, k, a.spellId, a.arg2, a.multispell, a.metamagic, a.domain,
                             ra.pos, ra.flag, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ra.pos)));
            QTRACE("%s", line);
        }
        if (k >= 0) spellWatchArm(k, a.cmdId, GetTickCount());
        reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)(reinterpret_cast<void*>(a.creature), a.spellId,
            static_cast<uint32_t>(a.arg2), ra.pos, a.multispell, a.metamagic, a.domain, ra.flag);
    } else {
        reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)(
            reinterpret_cast<void*>(a.creature),
            a.clientItem,
            a.packedByte,
            a.targetId,
            a.pos);
    }
    g_dispatching = false;
    return SendResult::Sent;
}

// RQ5: settle the active action after a send. Retry keeps it active and defers the next send; Refused (permanent) drops it with a DROP line.
static void afterSend(int k, qh::ActionBuffer& buf, SendResult r, DWORD now) {
    if (r == SendResult::Sent) { buf.dispatchDeferred = false; return; }
    if (r == SendResult::Refused) {
        if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }
        endAction(buf);
        return;
    }
    if (!buf.dispatchDeferred) { buf.dispatchDeferred = true; buf.deferTick = now; buf.deferUnpausedMs = 0; }
    buf.deferLastTry = now;
}

// QL20 starvation kick (selfheal CS12 route B, live-proven 10-05: KICK before=[3d,3d,3f] after=[3f,3d,3d] then DONE): a follower's 0x3D
// FollowLeader re-adds itself at the head, so a 0x3F behind it never runs (our server-route cast, or a power the engine took while this member
// led and that starved after the leader switch: 10-23 log, cmd 17 ran 53 s late, only once the member led again). Rotate the queue (pop head, add tail) until
// the 0x3F leads, under the R8 gates: unpaused, cr+0x20C == 0, cr+0x11AC != 0, round != 0, aa8 == 0, ab8 < 1, RoundHasPending != 0.
// Never ClearAllActions. One rotate per 250 ms per member; one SRV_KICK line per rotate, one SRV_KICK_SKIP line per gate change.
DWORD g_kickLast[3] = {0, 0, 0};
int g_kickSkipWhy[3] = {0, 0, 0};
qh::UnpausedClock g_orphanClk[3];   // RQ4: orphan round node clock per member
qh::UnpausedClock g_kickClk[3];     // RL8: starved server-route cast clock per member
uint32_t g_execIdCmd[3] = {0, 0, 0};   // RQ7: cmd whose EXEC_ID line was already traced
DWORD g_lastCombatTick = 0;            // RQ6: last tick of a readable combat frame
int removeRoundNodes(uintptr_t srv, const Snapshot& s, unsigned removeMask);   // defined after roundListHandle
static void kickSkip(int k, int why, const char* gate) {
    if (g_kickSkipWhy[k] == why) return;
    g_kickSkipWhy[k] = why;
    if (g_logOn && g_cfg.traceQueue) QTRACE("SRV_KICK_SKIP m=%d gate=%s", k, gate);
}
static bool kickStarved(int k, uintptr_t cr, qh::ActionBuffer& buf, const Snapshot& s, DWORD now) {   // true = gave up (buf.active cleared, caller skips the rest of this member's tick)
    if (buf.dispatchDeferred || !buf.active) { g_kickClk[k].reset(); return false; }   // R3: a deferred action was never sent; any starved 0x3F is another mod's (selfheal), not ours
    int types[qh::kMaxEntries];
    const int ne = s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries;
    for (int i = 0; i < ne; ++i) types[i] = s.e[i].type;
    const int at = qh::castStarvedAt(types, ne, s.nrn > 0 || s.haveCur);
    if (at == 0) { g_kickSkipWhy[k] = 0; g_kickClk[k].reset(); return false; }
    const uint32_t starvedMs = g_kickClk[k].tick(s.paused, now);   // RL8: unpaused time this cast has been starved, gates included
    if (starvedMs >= qh::kKickGiveUpMs) {
        const unsigned rmask = qh::headNodeMask(s.rn, s.nrn, buf.activeAction);   // R3: only the active action's own node, never another subtype's
        if (rmask == 0) { g_kickClk[k].reset(); return false; }   // not our cast starving (e.g. selfheal's 0x3F, which gives up on its own 5 s clock): do not drop, do not remove
        const int removed = removeRoundNodes(cr, s, rmask);   // else the starved 0x3F fires late, when the member next leads
        char line[200];
        qh::fmtKickGiveup(line, sizeof line, k, buf.activeAction.cmdId, starvedMs, removed);
        QLOG(chMain, "%s", line);
        if (g_logOn && g_cfg.traceQueue) { qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_EATEN_TIMEOUT, 1); QTRACE("%s", line); }
        endAction(buf);
        g_kickClk[k].reset(); g_kickSkipWhy[k] = 0;
        g_lastOurs[k].tag = "kick"; g_lastOurs[k].tick = now;
        return true;
    }
    if (s.paused) { kickSkip(k, 1, "paused"); return false; }
    if (now - g_kickLast[k] < 250u) return false;
    uint32_t running = 0, c11ac = 0;
    if (!rd(SiteSrvCast + 2, cr, actq::CrRunning, &running) || running != 0) { kickSkip(k, 2, "running"); return false; }
    if (!rd(SiteSrvCast + 3, cr, actq::Cr11AC, &c11ac) || c11ac == 0) { kickSkip(k, 3, "cr11ac"); return false; }
    if (!s.round) { kickSkip(k, 4, "round"); return false; }
    if (s.aa8 != 0) { kickSkip(k, 5, "aa8"); return false; }
    if (s.ab8 >= 1) { kickSkip(k, 6, "ab8"); return false; }
    if (reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::RoundHasPending)(s.round) == 0) { kickSkip(k, 7, "pending"); return false; }
    g_kickLast[k] = now;
    g_kickSkipWhy[k] = 0;
    int done = 0;
    for (int n = 0; n < at; ++n) {
        const uint32_t d = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead)(cr + actq::CrQueue);
        if (d == 0) break;
        reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail)(cr + actq::CrQueue, d);
        ++done;
    }
    g_lastOurs[k].tag = "kick"; g_lastOurs[k].tick = now;
    if (g_logOn && g_cfg.traceQueue) QTRACE("SRV_KICK m=%d at=%d rotated=%d ne=%d nrn=%d", k, at, done, s.engineNe, s.nrn);
    return false;
}

// Last-value globals for the edge-only lines in tickItemQueues (-1 = unknown / nothing to report).
int8_t g_prevTblOk = -1;
int8_t g_prevGatherOk[3] = {-1, -1, -1};
int g_prevRiWhy[3] = {-1, -1, -1};
int g_prevDsWhy[3] = {-1, -1, -1};
DWORD g_lastWaitTick[3] = {0, 0, 0};
qh::FlagGate g_flagGate[3];
// Identity guard: unpaused ms a buffer has been held by a creature that sits in no member slot, and the last tick it was counted.
uint32_t g_identLostMs[3] = {0, 0, 0};
DWORD g_identLastTick[3] = {0, 0, 0};
uintptr_t g_prevSrv[3] = {0, 0, 0};   // RL4: the three members' server creature pointers at the last normal tick (party generation baseline)
DWORD g_downSince[3] = {0, 0, 0};     // RL9: tick a Down companion's non-empty buffer started being held (0 = not holding)

// ---- Combat hand-off: at combat start the queued commands (active + buffered) go to the engine in click order, one per frame ----
// headHeld: the head was still held by the engine at the edge; it waits there until the engine releases it (qh::heldHeadAction).
// sawExec latches exec evidence (execEvidence) from the edge on; heldUnpausedMs / lastTick time a head that stays held.
struct Handoff { bool detachTried = false; bool pending = false; bool headValid = false; bool headHeld = false; bool sawExec = false; DWORD edgeAt = 0; DWORD lastTick = 0;
                 int heldUnpausedMs = 0; qh::QueuedAction head; int total = 0; int sent = 0; bool waiting = false; DWORD waitUntil = 0; };
Handoff g_handoff[3];
bool g_handoffHold[3];   // B1: HANDOFF_HOLD already traced for this episode (reset when no hand-off is pending)
// QL10 (handoff_keep_new): round nodes already pending at the first send (the player's new click); roundOrderTick keeps them behind our re-sent commands.
struct RoundOrder { uint32_t foreign[qh::kMaxRound] = {}; int n = 0; DWORD armedAt = 0; bool watch = false; uint32_t seen[qh::kMaxRound] = {}; int nSeen = 0; DWORD lastScan = 0, clickConsumed = 0; const char* lastSkipWhy = nullptr; };   // RL6: watch/seen/lastScan/clickConsumed grow the foreign set with in-combat clicks
RoundOrder g_roundOrder[3];
constexpr DWORD kRoundOrderMaxMs = 600000;   // safety net; normally disarmed as soon as no foreign node is pending
// Round-list watch: 15 s after each hand-off dispatch, roundWatchTick logs the member's round / cur / entry state on change only.
DWORD g_roundWatchUntil[3] = {};
DWORD g_combatClickTick = 0;   // RL6: last player Force power / item click made in combat (tick)
DWORD g_lastClickTick = 0;   // last left-button press edge (any frame, trace on or off); a head that vanishes right after one was cancelled, not wiped
struct RoundLast { int subs[qh::kMaxRound]; int n = -1; int cur = -2; int ne = -1; } g_lastRound[3];
bool g_prevCombat[3] = {false, false, false};

// The engine already holds this creature's action: an executing use-item / cast-spell entry, or a round node of subtype 9 or 10
// (same test as the tick's actionInQueue). A bare 0x3F entry is NOT proof: combat start queues ordinary attacks as 0x3F too.
bool engineHoldsAction(const Snapshot& s) {
    return qh::holdsAction(s.e, s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries, s.rn, s.nrn, s.haveCur, s.haveCur ? (int)s.cur.subType : -1,
                           s.aa8, s.ab8, s.paused, s.staleSince, GetTickCount());
}

// The engine is (or was just) running the command: aa8/ab8 set or a cur node of subtype 9 / 10, read unpaused (the flags stick in a frozen round).
bool execEvidence(const Snapshot& s) {
    if (s.paused) return false;
    return s.aa8 != 0 || s.ab8 != 0 || (s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10));
}

// HANDOFF_OUT watch: 8 s per hand-off command, independent of buf.active (which goes false at the hand-off). Trace-gated.
constexpr DWORD kHandoffOutMs = 8000;
// A second, independent set (g_spellWatch) does the same for every replayed Force power (SPELL_OUT), so neither flushes the other early.
struct OutWatch { bool on = false; uint32_t cmd = 0; DWORD startAt = 0; bool exec = false; int firstExecMs = -1; } g_outWatch[3], g_spellWatch[3];

void outWatchFlush(OutWatch* arr, bool spell, int k, DWORD now) {
    OutWatch& w = arr[k];
    if (!w.on) return;
    if (g_logOn && g_cfg.traceQueue) {
        char line[160];
        (spell ? qh::fmtSpellOut : qh::fmtHandoffOut)(line, sizeof line, k, w.cmd, w.exec, w.firstExecMs, static_cast<int>(now - w.startAt));
        QTRACE("%s", line);
    }
    w = OutWatch{};
}
void outWatchArmIn(OutWatch* arr, bool spell, int k, uint32_t cmd, DWORD now) {
    outWatchFlush(arr, spell, k, now);
    OutWatch& w = arr[k];
    w.on = true; w.cmd = cmd; w.startAt = now; w.exec = false; w.firstExecMs = -1;
}
void outWatchArm(int k, uint32_t cmd, DWORD now) { outWatchArmIn(g_outWatch, false, k, cmd, now); }
void spellWatchArm(int k, uint32_t cmd, DWORD now) { outWatchArmIn(g_spellWatch, true, k, cmd, now); }
void outWatchTickIn(OutWatch* arr, bool spell, uintptr_t tbl, DWORD now) {
    for (int k = 0; k < 3; ++k) {
        OutWatch& w = arr[k];
        if (!w.on) continue;
        if (tbl && !w.exec) {
            const uintptr_t cli = party::member(tbl, k);
            const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
            Snapshot s;
            if (srv && gatherFor(srv, s) && execEvidence(s)) { w.exec = true; w.firstExecMs = static_cast<int>(now - w.startAt); }
        }
        if (now - w.startAt >= kHandoffOutMs) outWatchFlush(arr, spell, k, now);
    }
}
void outWatchTick(DWORD now) {
    bool any = false;
    for (int k = 0; k < 3; ++k) any = any || g_outWatch[k].on || g_spellWatch[k].on;
    if (!any) return;
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    outWatchTickIn(g_outWatch, false, tbl, now);
    outWatchTickIn(g_spellWatch, true, tbl, now);
}

// Ends one member's hand-off. A held head that was never dispatched is logged as a DROP; the buffer is left alone.
void endHandoff(int k, qh::DropWhy why) {
    Handoff& h = g_handoff[k];
    if (h.headValid && g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtDrop(line, sizeof line, h.head.cmdId, k, why, 1);
        QTRACE("%s", line);
    }
    h = Handoff{};
}

// B1: once per episode, trace that member k's buffer is held because its owner is not the controlled creature.
static void handoffHoldTrace(int k, uint32_t cmdId, int ownerSlot) {
    if (g_handoffHold[k]) return;
    g_handoffHold[k] = true;
    if (g_logOn && g_cfg.traceQueue) {
        char line[120];
        qh::fmtHandoffHold(line, sizeof line, cmdId, k, ownerSlot);
        QTRACE("%s", line);
    }
}

static void handoffHoldReleaseTrace(int k, int ownerSlot) {
    if (!g_handoffHold[k]) return;
    g_handoffHold[k] = false;
    if (g_logOn && g_cfg.traceQueue) {
        char line[120];
        qh::fmtHandoffHoldRel(line, sizeof line, k, ownerSlot);
        QTRACE("%s", line);
    }
}

bool detachRound(int k, uintptr_t srv, const Snapshot& s, DWORD now, const char* why, unsigned mask);   // QL10 detach, defined after the hand-off code
void discardDetached(int k, const char* why, bool always = false);   // always = default-level line (combat-end paths)
// Every frame in combat (readable HUD). The edge frame moves a member's buffer into g_handoff; then each pending member sends one command per frame.
void handoffTick(DWORD now, const bool* edge) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) {   // no party table: nothing can be handed off, so a fresh combat edge drops its buffer (as the no-creature case does)
        for (int k = 0; k < 3; ++k)
            if (edge[k] && bufBusy(g_itemQueue[k])) { endHandoff(k, qh::DR_NULL_CREATURE); traceDropAll(k, qh::DR_NULL_CREATURE); g_itemQueue[k].clear(); }
        return;
    }
    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& buf = g_itemQueue[k];
        if (edge[k] && bufBusy(buf)) {
            {   // B1: the client wrappers would run the buffered commands on the controlled creature; hold the buffer untouched (no ServerClearAllActions, no hand-off). QL20: members 1-2 hand off Force powers via the server route
                const int ownerSlot = slotOfCreature(tbl, bufOwner(buf));
                if (!mayDispatch(tbl, bufHead(buf)) || ownerSlot != k) {   // != k: no identity pre-pass in combat, the clear below targets member k
                    handoffHoldTrace(k, buf.active ? buf.activeAction.cmdId : buf.queue[0].cmdId, ownerSlot);
                    continue;
                }
            }
            endHandoff(k, qh::DR_COMBAT_CLEAR);   // a hand-off left unfinished by an earlier combat
            if (!g_cfg.handoffOnCombat) { traceDropAll(k, qh::DR_COMBAT_CLEAR); buf.clear(); continue; }
            const uintptr_t cli = party::member(tbl, k);
            const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
            if (!srv) { traceDropAll(k, qh::DR_NULL_CREATURE); buf.clear(); continue; }
            Snapshot s;
            const bool gEdgeOk = gatherFor(srv, s);
            const bool inEngine = gEdgeOk && engineHoldsAction(s);
            Handoff& h = g_handoff[k];
            const bool wasActive = buf.active;
            const uint32_t activeCmd = buf.activeAction.cmdId;
            if (wasActive) outWatchArm(k, activeCmd, now);   // ground truth for the command that was active at the edge
            h.edgeAt = now;
            h.lastTick = now;
            h.heldUnpausedMs = 0;
            h.sawExec = gEdgeOk && qh::edgeExecEvidence(inEngine, execEvidence(s), buf.seenInQueue, buf.flagBase, s.aa8, s.ab8);
            const qh::EdgeHeadAction eh = qh::edgeHeadAction(buf.active, inEngine, buf.sawExecuting, h.sawExec,
                                                             now - buf.activeStartTick, gEdgeOk, buf.dispatchDeferred);
            const bool waitOk = eh == qh::EDGE_WAIT && (!inEngine || g_cfg.handoffResendHeld);
            h.headHeld = waitOk;
            h.headValid = waitOk || eh == qh::EDGE_RESEND;
            h.waiting = waitOk && !inEngine;     // not in the engine yet: give a late entry until waitUntil
            h.waitUntil = h.waiting ? (gEdgeOk ? buf.activeStartTick : now) + qh::kMinActiveMs : 0;
            if (h.headValid) h.head = buf.activeAction;
            if (g_logOn && g_cfg.traceQueue) {
                char snapLine[qh::kLogLineMax];
                qh::fmtEdgeSnap(snapLine, sizeof snapLine, k, gEdgeOk, s.e, s.engineNe, s.rn, s.nrn,
                                s.haveCur, s.haveCur ? &s.cur : nullptr, s.aa8 != 0, s.ab8 != 0, s.paused, eh, inEngine);
                QTRACE("%s", snapLine);
                uint32_t ids[qh::kMaxBufferCap];
                const int ni = buf.count < qh::kMaxBufferCap ? buf.count : qh::kMaxBufferCap;
                for (int i = 0; i < ni; ++i) ids[i] = buf.queue[i].cmdId;
                char line[200];
                qh::fmtHandoff(line, sizeof line, k, wasActive, activeCmd, ids, ni);
                QTRACE("%s", line);
            }
            // after the HANDOFF line, so the checker's hand-off window (flag I) sees it
            if (eh == qh::EDGE_DROP_EXEC && g_logOn && g_cfg.traceQueue) {
                char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_EXECUTED, 1); QTRACE("%s", line);
            }
            // The engine owns the active command now (or already did); the buffered ones stay queued and leave one per frame below.
            buf.active = false; buf.seenInQueue = false; buf.eaten = false; buf.reInjectPending = false; buf.dispatchDeferred = false;
            h.total = (h.headValid ? 1 : 0) + buf.count;
            h.sent = 0;
            h.pending = h.total > 0;
            g_lastOurs[k].tag = "handoff"; g_lastOurs[k].tick = now;
        }
    }
    for (int k = 0; k < 3; ++k) {
        Handoff& h = g_handoff[k];
        if (!h.pending) { handoffHoldReleaseTrace(k, -1); continue; }
        qh::ActionBuffer& buf = g_itemQueue[k];
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (!srv) { endHandoff(k, qh::DR_NULL_CREATURE); traceDropAll(k, qh::DR_NULL_CREATURE); buf.clear(); continue; }
        Snapshot s;
        if (!gatherFor(srv, s)) continue;
        if (h.headValid || buf.count > 0) {   // B1: a leader switch during a started hand-off: hold (nothing is sent or cleared) until this member leads again; nothing left = fall through to the pop/clear path
            const qh::QueuedAction& next = h.headValid ? h.head : bufHead(buf);
            const int ownerSlot = slotOfCreature(tbl, next.creature);
            if (!mayDispatch(tbl, next) || ownerSlot != k) {
                handoffHoldTrace(k, h.headValid ? h.head.cmdId : buf.queue[0].cmdId, ownerSlot);
                continue;
            }
            handoffHoldReleaseTrace(k, ownerSlot);
        }
        if (h.headValid && h.headHeld) {   // exec evidence and the stuck clock run even on frames that cannot send yet
            if (execEvidence(s)) h.sawExec = true;
            const bool held = engineHoldsAction(s);
            if (held && !s.paused) { const DWORD d = now - h.lastTick; h.heldUnpausedMs += d < 1000u ? static_cast<int>(d) : 1000; }
            h.lastTick = now;
        }
        const unsigned dmask = qh::handoffDetachMask(s.rn, s.nrn, h.headValid, h.head, h.sent, g_roundOrder[k].foreign, g_roundOrder[k].n);
        if (g_cfg.handoffDetachHeld && g_cfg.handoffKeepNew && !h.detachTried && qh::keepPathAllowed(true, s.paused, g_cfg.handoffKeepUnpaused) && dmask != 0) {
            if (detachRound(k, srv, s, now, (h.headValid && h.headHeld) ? "held_head" : "handoff_keep", dmask)) {
                h.detachTried = true;
                gatherFor(srv, s);
            }
        }
        if (s.nrn + 1 > qh::kMaxRound) continue;   // the engine's round list is full: retry next frame
        if (h.headValid && h.headHeld) {
            const int ms = static_cast<int>(now - h.edgeAt);
            const bool cancelClick = qh::cancelClickRecent(g_lastClickTick, h.edgeAt, now);
            if (engineHoldsAction(s)) h.waiting = false;   // it showed up: from now on a vanish means a real wipe
            const bool holds = engineHoldsAction(s) || (h.waiting && static_cast<int>(h.waitUntil - now) > 0);
            const qh::HeldHeadAction act = qh::heldHeadAction(holds, h.sawExec, s.paused, h.heldUnpausedMs, cancelClick);
            if (act == qh::HELD_WAIT) continue;
            if (g_logOn && g_cfg.traceQueue) {
                char line[200];
                qh::fmtHandoffHeld(line, sizeof line, k, h.head.cmdId, act, ms);
                if (cancelClick && act == qh::HELD_DROP && !h.sawExec) QTRACE("%s why=cancel_click", line);
                else QTRACE("%s", line);
            }
            if (cancelClick && act == qh::HELD_DROP && !h.sawExec) {   // the player cancelled: the whole queue goes, buffered commands too (QL10: the stim re-entered combat after a Cancel)
                endHandoff(k, qh::DR_CANCEL); traceDropAll(k, qh::DR_CANCEL); buf.clear();
                discardDetached(k, "cancel_click");
                continue;
            }
            h.headHeld = false;
            if (act == qh::HELD_DROP) h.headValid = false;   // the engine really ran it (or it already finished): the buffered commands still go
        }
        bool midFlight = false;                   // the previous hand-off is still queued in the engine
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i)
            if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) midFlight = true;
        if (midFlight) continue;
        qh::QueuedAction a;
        if (h.headValid) { a = h.head; h.headValid = false; }
        else if (!buf.pop(&a)) { h.pending = false; continue; }
        if (!saferead::probeRead(SiteQueue, reinterpret_cast<const void*>(a.creature), 4)) {
            QLOGV(chMain, KPLOG_WARN, "handoff skipped: creature %08x unreadable for member %d", (unsigned)a.creature, k);
            if (g_logOn && g_cfg.traceQueue) {
                char line[200];
                qh::fmtDrop(line, sizeof line, a.cmdId, k, qh::DR_UNREADABLE_CREATURE, 1);
                QTRACE("%s", line);
            }
        } else {
            // Combat start wipes the engine queue. Default (handoff_keep_new=1, handoff_keep_unpaused=1): no clear, the player's pending round nodes are remembered (RoundOrder).
            // The clear-first send (ServerClearAllActions) only runs with keep_new=0, or with keep_unpaused=0 when combat starts unpaused.
            // QL10: while paused nothing pops the round list; unpaused it is allowed too (handoff_keep_unpaused) and a node the engine already popped is simply not there to order.
            const bool keepNow = qh::keepPathAllowed(g_cfg.handoffKeepNew, s.paused, g_cfg.handoffKeepUnpaused);
            if (qh::keepNewAtHandoff(keepNow, h.sent)) {
                RoundOrder& ro = g_roundOrder[k];
                ro = RoundOrder();
                for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i) { ro.foreign[ro.n++] = s.rn[i].ptr; ro.seen[ro.nSeen++] = s.rn[i].ptr; }
                ro.watch = true; ro.armedAt = now; ro.lastScan = now;   // RL6: watch for later clicks even when nothing was pending at the first send
                if (g_logOn && g_cfg.traceQueue) {
                    char line[200];
                    qh::fmtHandoffKeep(line, sizeof line, k, a.cmdId, s.engineNe, s.nrn);
                    QTRACE("%s", line);
                }
            }
            if (qh::clearBeforeHandoff(g_cfg.handoffClearFirst, h.sent, keepNow)) {
                uint32_t id = 0, server = 0;
                const char* bad = resolveServerClear(srv, id, server);
                if (g_logOn && g_cfg.traceQueue) {
                    char line[200];
                    qh::fmtHandoffClear(line, sizeof line, k, a.cmdId, s.engineNe, s.nrn);
                    if (bad) QTRACE("%s refused=%s", line, bad);
                    else QTRACE("%s", line);
                }
                if (!bad) reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
            }
            ++h.sent;
            g_lastOurs[k].tag = "handoff"; g_lastOurs[k].tick = now;
            const SendResult r = dispatchAction(a, k);
            if (r != SendResult::Sent && g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, a.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }
            outWatchArm(k, a.cmdId, now);
            g_roundWatchUntil[k] = now + 15000;   // armed outside the trace gate: the watch runs with trace off too
            g_lastRound[k].n = -1;
            if (g_logOn && g_cfg.traceQueue) {
                char line[80];
                qh::fmtHandoffOne(line, sizeof line, a.cmdId, k, h.sent, h.total);
                QTRACE("%s", line);
            }
        }
        if (!h.headValid && buf.count == 0) h = Handoff{};
    }
}

// QL10 (handoff_keep_new): the hand-off no longer clears the engine, so the player's new click (attack / power) stays in the round list AHEAD of our
// re-sent commands. Each frame the pending round nodes are re-ordered with the engine's own list thunks (pop head, append tail: link nodes churn,
// no round node is allocated or freed) so every node that was already pending at the first send runs last. Static RE: ghidra_knowledge/rea_findings/20261015_round-list-rotate.tsv.
static void roundRotSkipTrace(RoundOrder& ro, int k, const char* why, int nrn, int nForeign, int moved) {
    if (ro.lastSkipWhy && strcmp(ro.lastSkipWhy, why) == 0) return;
    ro.lastSkipWhy = why;
    if (!g_logOn || !g_cfg.traceQueue) return;
    char line[qh::kLogLineMax];
    qh::fmtRoundRotSkip(line, sizeof line, k, why, nrn, nForeign, moved);
    QTRACE("%s", line);
}

void roundOrderTick(DWORD now, const bool* edge) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;
    for (int k = 0; k < 3; ++k) {
        RoundOrder& ro = g_roundOrder[k];
        if (ro.n == 0 && !ro.watch) continue;
        if (static_cast<int>(now - ro.armedAt) > static_cast<int>(kRoundOrderMaxMs)) {
            roundRotSkipTrace(ro, k, "timeout", 0, ro.n, 0);
            ro = RoundOrder();
            continue;
        }
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        Snapshot s;
        if (!srv || !gatherFor(srv, s)) { roundRotSkipTrace(ro, k, "gather_fail", 0, ro.n, 0); continue; }
        if (ro.watch) {   // RL6: a node that appears while the hand-off runs, after a combat click, is the player's: order it last too
            int newIdx[qh::kMaxRound], nNew = 0;
            for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i) {
                bool seen = false;
                for (int j = 0; j < ro.nSeen; ++j) seen = seen || ro.seen[j] == s.rn[i].ptr;
                if (!seen) newIdx[nNew++] = i;
            }
            const bool clickPending = g_combatClickTick != 0 && g_combatClickTick != ro.clickConsumed && static_cast<int>(g_combatClickTick - ro.armedAt) >= 0 && now - g_combatClickTick < 1500u;
            const int ownSends = (g_lastOurs[k].tag && !strcmp(g_lastOurs[k].tag, "handoff") && static_cast<int>(g_lastOurs[k].tick - ro.lastScan) >= 0) ? 1 : 0;
            const int add = qh::lateForeignCount(nNew, clickPending, ownSends);
            for (int a = nNew - add; a < nNew && ro.n < qh::kMaxRound; ++a) ro.foreign[ro.n++] = s.rn[newIdx[a]].ptr;
            if (add > 0) {
                ro.clickConsumed = g_combatClickTick;
                if (g_logOn && g_cfg.traceQueue) {
                    char line[qh::kLogLineMax];
                    qh::fmtRoundRotLate(line, sizeof line, k, add, ro.n);
                    QTRACE("%s", line);
                }
            }
            ro.nSeen = 0;
            for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i) ro.seen[ro.nSeen++] = s.rn[i].ptr;
            ro.lastScan = now;
        }
        uint32_t ptrs[qh::kMaxRound];
        int nForeignPresent = 0;
        for (int i = 0; i < s.nrn; ++i) {
            ptrs[i] = s.rn[i].ptr;
            for (int j = 0; j < ro.n; ++j) if (ro.foreign[j] == ptrs[i]) ++nForeignPresent;
        }
        if (nForeignPresent == 0) {
            if (!(ro.watch && g_handoff[k].pending)) {
                roundRotSkipTrace(ro, k, "no_foreign", s.nrn, 0, 0);
                ro = RoundOrder();
            }
            continue;
        }   // the new click ran or was cancelled: nothing left to order
        int order[qh::kMaxRound];
        const int moved = qh::roundPartition(ptrs, s.nrn, ro.foreign, ro.n, order);
        if (moved == 0 || s.nrn < 2) { roundRotSkipTrace(ro, k, "already_ordered", s.nrn, nForeignPresent, moved); continue; }
        uint32_t running = 0, w = 0, internal = 0;
        bool ok = s.round &&
                  saferead::readAt(SiteCancel + 0x30, srv, actq::CrRunning, &running) && running == 0 &&   // not inside RunActions
                  saferead::readAt(SiteCancel + 0x31, s.round, actq::RdList, &w) && w &&
                  saferead::readAt(SiteCancel + 0x32, w, 0, &internal) && internal;
        if (!ok) {
            roundRotSkipTrace(ro, k, "cr_running", s.nrn, nForeignPresent, moved);
            continue;
        }
        ro.lastSkipWhy = nullptr;
        bool applied = false;
        {
            auto pop = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead);
            auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
            uint32_t d[qh::kMaxRound];
            int popped = 0;
            for (; popped < s.nrn; ++popped) { d[popped] = pop(w); if (d[popped] == 0) break; }
            applied = popped == s.nrn;
            for (int i = 0; i < s.nrn && applied; ++i) applied = d[i] == ptrs[i];   // the list is what the snapshot said
            if (applied) for (int i = 0; i < s.nrn; ++i) push(w, d[order[i]]);
            else for (int i = 0; i < popped; ++i) if (d[i]) push(w, d[i]);          // put it back exactly as it was
        }
        if (g_logOn && g_cfg.traceQueue) {
            char line[100];
            qh::fmtRoundRot(line, sizeof line, k, s.nrn, moved, applied);
            QTRACE("%s", line);
        }
        if (!applied) ro = RoundOrder();   // never retry a failed rotate every frame
    }
}

// Guards shared by the round-list writes: creature not inside RunActions, list object + internal list readable. w = the list object ECX for the thunks.
bool roundListHandle(uintptr_t srv, uintptr_t round, uint32_t* w) {
    uint32_t running = 0, internal = 0;
    return round && saferead::readAt(SiteCancel + 0x33, srv, actq::CrRunning, &running) && running == 0 &&
           saferead::readAt(SiteCancel + 0x34, round, actq::RdList, w) && *w &&
           saferead::readAt(SiteCancel + 0x35, *w, 0, &internal) && internal;
}

// Take the round nodes selected by removeMask (bit i = s.rn[i]) out of the list WITHOUT ServerClearAllActions (that also empties cr+0x100: a companion would lose its 0x3D, RQ12).
// Same guards, ActListRemoveHead pop and identity check as detachRound; every node whose bit is clear is pushed straight back (list is empty after the pop, so order is kept); removed nodes
// are leaked on purpose (free path unverified). Returns the count removed; 0 = empty mask (list untouched); -1 = guard/mismatch (all popped nodes restored).
int removeRoundNodes(uintptr_t srv, const Snapshot& s, unsigned removeMask) {
    uint32_t w = 0;
    if (s.nrn <= 0 || s.nrn > qh::kMaxRound || (removeMask & ((1u << s.nrn) - 1u)) == 0) return 0;
    if (!roundListHandle(srv, s.round, &w)) return -1;
    auto pop = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead);
    auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
    uint32_t got[qh::kMaxRound];
    int popped = 0;
    for (; popped < s.nrn; ++popped) { got[popped] = pop(w); if (got[popped] == 0) break; }
    bool ok = popped == s.nrn;
    for (int i = 0; i < s.nrn && ok; ++i) ok = got[i] == s.rn[i].ptr;
    if (!ok) { for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]); return -1; }
    int removed = 0;
    for (int i = 0; i < s.nrn; ++i) { if (removeMask & (1u << i)) ++removed; else push(w, got[i]); }
    return removed;
}

constexpr DWORD kDetachEndGraceMs = 20000;   // RQ9: after combat, how long a detached batch may wait for the buffer to drain before it is discarded
void traceDetach(int k, int n, const char* act, bool ok, const char* why, bool always = false) {
    if (!g_logOn || (!always && !g_cfg.traceQueue)) return;
    char line[160];
    qh::fmtRoundDetach(line, sizeof line, k, n, act, ok, why);
    if (always) QLOG(chMain, "%s", line); else QTRACE("%s", line);
}

// Take every pending round node of member k out of the list (identity-checked against the snapshot; put back untouched on any mismatch).
bool detachRound(int k, uintptr_t srv, const Snapshot& s, DWORD now, const char* why, unsigned mask) {
    Detached& d = g_detached[k];
    uint32_t w = 0;
    if (d.n != 0 || s.nrn <= 0 || mask == 0 || !roundListHandle(srv, s.round, &w)) { traceDetach(k, s.nrn, "detach", false, "guard"); return false; }
    auto pop = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead);
    auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
    uint32_t got[qh::kMaxRound];
    int popped = 0;
    for (; popped < s.nrn; ++popped) { got[popped] = pop(w); if (got[popped] == 0) break; }
    bool ok = popped == s.nrn;
    for (int i = 0; i < s.nrn && ok; ++i) ok = got[i] == s.rn[i].ptr;
    int nDet = 0;
    if (ok) {
        for (int i = 0; i < s.nrn; ++i) {
            if (mask & (1u << i)) { d.ptr[nDet] = got[i]; d.rn[nDet] = s.rn[i]; ++nDet; }
            else push(w, got[i]);                       // the head's own node stays in the list
        }
        d.n = nDet; d.at = now; d.owner = srv;   // RQ10: the batch goes back to its owner's CURRENT slot
    } else for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]);   // put it back exactly as it was
    traceDetach(k, ok ? nDet : s.nrn, "detach", ok, why);
    return ok && nDet > 0;
}

// Forget the detached nodes without re-adding them (the player cancelled, combat ended or restarted). The ~0x88 byte nodes are leaked on purpose: the free path is unverified.
void discardDetached(int k, const char* why, bool always) {
    Detached& d = g_detached[k];
    if (d.n == 0) return;
    traceDetach(k, d.n, "discard", true, why, always);
    d = Detached();
}

// Per combat frame (and, RQ9, per non-combat frame with afterCombat): put the detached nodes back once nothing of ours is pending; drop them when the player cancelled the whole queue.
void detachTick(DWORD now, bool afterCombat) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;
    for (int k = 0; k < 3; ++k) {
        Detached& d = g_detached[k];
        if (d.n == 0) continue;
        if (afterCombat && d.endAt == 0) d.endAt = now;
        if (static_cast<int>(now - d.at) > static_cast<int>(kDetachMaxMs)) { discardDetached(k, "timeout", afterCombat); continue; }
        const int j = qh::detachTarget(slotOfCreature(tbl, d.owner), k);   // RQ10: slotOfCreature matches client or server pointers
        if (j < 0) { discardDetached(k, "identity_lost", afterCombat); continue; }
        const uintptr_t srv = d.owner;
        Snapshot s;
        if (!gatherFor(srv, s)) continue;
        bool midFlight = false;
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i)
            if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) midFlight = true;
        int oursNodes = 0;
        for (int i = 0; i < s.nrn; ++i) if (s.rn[i].subType == 9 || s.rn[i].subType == 10) ++oursNodes;
        const bool busy = g_handoff[k].pending || bufBusy(g_itemQueue[j]);   // R5: g_itemQueue / g_lastOurs follow the owner (slot j), g_handoff stays with the detach slot k
        // A click right before the engine went completely quiet = the player's Cancel: the detached power goes too, and so do the buffered commands.
        const bool quiet = !midFlight && s.nrn == 0 && s.engineNe == 0;
        if (quiet && qh::cancelClickRecent(g_lastClickTick, d.at, now)) {
            discardDetached(k, "cancel_click", afterCombat);
            endHandoff(k, qh::DR_CANCEL); traceDropAll(j, qh::DR_CANCEL); g_itemQueue[j].clear();
            continue;
        }
        const int sinceSend = g_lastOurs[j].tick ? static_cast<int>(now - g_lastOurs[j].tick) : 100000;
        if (!qh::reattachReady(busy, midFlight, oursNodes, sinceSend, s.nrn, d.n)) {
            if (afterCombat && static_cast<int>(now - d.endAt) > static_cast<int>(kDetachEndGraceMs)) discardDetached(k, "combat_end_timeout", true);
            continue;
        }
        uint32_t w = 0;
        bool ok = roundListHandle(srv, s.round, &w);
        if (ok) {
            auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
            for (int i = 0; i < d.n; ++i) push(w, d.ptr[i]);
        }
        traceDetach(k, d.n, "reattach", ok, !ok ? "guard" : afterCombat ? "combat_end" : (j == k ? "done" : "moved"), afterCombat);
        if (ok) d = Detached();
        else if (static_cast<int>(now - d.at) > 5000) discardDetached(k, "reattach_guard", afterCombat);   // never wedge on an unwritable list
    }
}

// Round-list watch (combat only, 15 s per dispatch): one line per change of the member's round subtypes, cur and entry types. Edge only.
void roundWatchTick(DWORD now) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;
    for (int k = 0; k < 3; ++k) {
        if (g_roundWatchUntil[k] == 0 || static_cast<int>(g_roundWatchUntil[k] - now) <= 0) continue;
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (k > 0 && memberDown(k, srv, now)) continue;   // Down companion: no round watch until it is back up
        Snapshot s;
        if (!srv || !gatherFor(srv, s)) continue;
        const int nSub = s.nrn < qh::kMaxRound ? s.nrn : qh::kMaxRound;
        const int nTy = s.ne < qh::kMaxEntries ? s.ne : qh::kMaxEntries;
        const int cur = s.haveCur ? static_cast<int>(s.cur.subType) : -1;
        int subs[qh::kMaxRound];
        int types[qh::kMaxEntries];
        for (int i = 0; i < nSub; ++i) subs[i] = s.rn[i].subType;
        for (int i = 0; i < nTy; ++i) types[i] = s.e[i].type;
        RoundLast& L = g_lastRound[k];
        bool changed = L.n != nSub || L.cur != cur || L.ne != s.ne;
        for (int i = 0; !changed && i < nSub; ++i) changed = L.subs[i] != subs[i];
        if (!changed) continue;
        L.n = nSub; L.cur = cur; L.ne = s.ne;
        for (int i = 0; i < nSub; ++i) L.subs[i] = subs[i];
        if (g_logOn && g_cfg.traceQueue) {
            char line[256];
            qh::fmtRound(line, sizeof line, k, subs, nSub, cur, s.ne, types, nTy);
            QTRACE("%s", line);
        }
    }
}

// Every combat edge: dump the soft-combat evidence (CMBT_FIELDS) for each party member and arm the round watch for 8 s, so an eaten
// attack / force power click shows as ROUND lines even when no hand-off happens. The watch never shortens a running one.
constexpr DWORD kEdgeWatchMs = 8000;
constexpr int CrCombatOff = 0x520, CrTgt550 = 0x550, CrTgt564 = 0x564, CrTgt568 = 0x568;

// One member's three target fields (server creature +0x550/+0x564/+0x568) and whether each read succeeded.
struct MemberTgt { uint32_t t[3] = {0, 0, 0}; bool ok[3] = {false, false, false}; };
void readTargets(uintptr_t srv, MemberTgt& m) {
    m.ok[0] = rd(SiteQueue, srv, CrTgt550, &m.t[0]);
    m.ok[1] = rd(SiteQueue, srv, CrTgt564, &m.t[1]);
    m.ok[2] = rd(SiteQueue, srv, CrTgt568, &m.t[2]);
}
// Party object ids (server creature +CrObjectId, k = 0..2); unreadable members stay 0 (ignored by softCandidate).
void readPartyIds(uintptr_t tbl, uint32_t ids[3]) {
    for (int j = 0; j < 3; ++j) {
        ids[j] = 0;
        const uintptr_t pc = party::member(tbl, j);
        const uintptr_t ps = pc ? party::serverCreature(pc) : 0;
        if (ps) rd(SiteCancel + 1, ps, CrObjectId, &ids[j]);
    }
}
// Shared by the traced SOFT_CAND line and the live banner decision so the two cannot diverge. Unreadable targets count as none.
qh::SoftCand softCandOf(const MemberTgt& m, const uint32_t partyIds[3]) {
    const uint32_t tg[3] = {m.ok[0] ? m.t[0] : 0u, m.ok[1] ? m.t[1] : 0u, m.ok[2] ? m.t[2] : 0u};
    return qh::softCandidate(tg, 3, partyIds, 3);
}

void combatEdgeTrace(DWORD now, const bool* edge) {
    bool any = false;
    for (int k = 0; k < 3; ++k) any = any || edge[k];
    if (!any) return;
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    for (int k = 0; k < 3; ++k) {
        if (!edge[k]) continue;
        if (static_cast<int>(g_roundWatchUntil[k] - (now + kEdgeWatchMs)) < 0 || g_roundWatchUntil[k] == 0) {
            g_roundWatchUntil[k] = now + kEdgeWatchMs;
            g_lastRound[k].n = -1;
        }
        if (!g_logOn || !g_cfg.traceQueue || !tbl) continue;
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (!srv) continue;
        uint32_t cr[qh::kCrCombatDwords] = {};
        bool crOk[qh::kCrCombatDwords] = {};
        for (int i = 0; i < qh::kCrCombatDwords; ++i) crOk[i] = rd(SiteQueue, srv, CrCombatOff + 4 * i, &cr[i]);
        MemberTgt mt;
        readTargets(srv, mt);
        char line[200];
        qh::fmtCmbtFields(line, sizeof line, k, cr, crOk, mt.t[0], mt.ok[0], mt.t[1], mt.ok[1], mt.t[2], mt.ok[2]);
        QTRACE("%s", line);
        // SOFT_CAND: do the target fields name a party member, a hostile, or nothing?
        uint32_t partyIds[3];
        readPartyIds(tbl, partyIds);
        qh::fmtSoftCand(line, sizeof line, k, softCandOf(mt, partyIds), cr[0], crOk[0], cr[1], crOk[1]);
        QTRACE("%s", line);
    }
}

// QL24: keep per-slot buffers and state aligned with the current party table across rotations, loads, and Party Selection swaps.
static void syncPartyIdentity(uintptr_t tbl, DWORD now, bool skip[3]) {
    uintptr_t nowCli[3] = {0, 0, 0}, nowSrv[3] = {0, 0, 0};
    for (int k = 0; k < 3; ++k) {
        nowCli[k] = party::member(tbl, k);
        nowSrv[k] = nowCli[k] ? party::serverCreature(nowCli[k]) : 0;
    }
    {   // RL4: party re-created (load / area): every buffered pointer is dead; drop now instead of the 20 s identity-lost wait
        if (qh::partyGenerationChanged(g_prevSrv, nowSrv)) {
            int dropped = 0;
            for (int k = 0; k < 3; ++k) {
                if (g_handoff[k].pending) endHandoff(k, qh::DR_PARTY_CHANGED);
                if (bufBusy(g_itemQueue[k])) { ++dropped; traceDropAll(k, qh::DR_PARTY_CHANGED); g_itemQueue[k].clear(); }
                discardDetached(k, "party_changed", true);   // RV3 signature (default arg on the forward declaration); true = default-level line, not just a trace
                g_flagGate[k].reset(); g_kickLast[k] = 0; g_downSince[k] = 0; g_identLostMs[k] = 0; g_identLastTick[k] = 0; g_prevRiWhy[k] = g_prevDsWhy[k] = -1;
                g_roundOrder[k] = RoundOrder(); g_cpend[k] = CPending(); g_handoffHold[k] = false; g_heldStrip[k] = HeldStrip();   // R7: stale pointers / icons from the old party
            }
            g_heldLeader = Held();   // R7: dead creature pointer in Held.creature, stale leader icons for 400 ms
            if (g_logOn && g_cfg.traceQueue) QTRACE("PARTY_GEN old=%08x,%08x,%08x new=%08x,%08x,%08x dropped=%d", (unsigned)g_prevSrv[0], (unsigned)g_prevSrv[1], (unsigned)g_prevSrv[2], (unsigned)nowSrv[0], (unsigned)nowSrv[1], (unsigned)nowSrv[2], dropped);
            if (dropped) QLOG(chMain, "party changed (load / area): dropped %d buffered member queue(s)", dropped);
        }
        if (nowSrv[0] || nowSrv[1] || nowSrv[2]) for (int k = 0; k < 3; ++k) g_prevSrv[k] = nowSrv[k];   // a transient all-zero read keeps the baseline
    }

    // Identity pre-pass: buffer k must hold the creature that party member k is now. Party rotation moves creatures between slots.
    bool swapped[3] = {false, false, false};
    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& b = g_itemQueue[k];
        const uintptr_t owner = bufBusy(b) ? bufOwner(b) : (g_handoff[k].pending && g_handoff[k].headValid ? g_handoff[k].head.creature : 0);
        if (!owner) { g_identLostMs[k] = 0; g_identLastTick[k] = 0; continue; }
        const int slot = slotOfCreature(tbl, owner);
        if (slot == k) { g_identLostMs[k] = 0; g_identLastTick[k] = 0; continue; }
        if (slot >= 0) {   // the creature moved to another slot: swap ALL per-slot arrays between k and slot
            std::swap(g_itemQueue[k], g_itemQueue[slot]);
            std::swap(g_lastOurs[k], g_lastOurs[slot]);
            std::swap(g_flagGate[k], g_flagGate[slot]);   // RL2: per-buffer hold / throttle state travels with the buffer
            std::swap(g_kickLast[k], g_kickLast[slot]); std::swap(g_kickSkipWhy[k], g_kickSkipWhy[slot]);
            std::swap(g_lastWaitTick[k], g_lastWaitTick[slot]);
            std::swap(g_downSince[k], g_downSince[slot]);   // RL9: the Down hold timer travels with its buffer
            std::swap(g_orphanClk[k], g_orphanClk[slot]); std::swap(g_kickClk[k], g_kickClk[slot]);
            std::swap(g_handoff[k], g_handoff[slot]);
            std::swap(g_handoffHold[k], g_handoffHold[slot]);
            std::swap(g_roundOrder[k], g_roundOrder[slot]);
            std::swap(g_roundWatchUntil[k], g_roundWatchUntil[slot]);
            std::swap(g_lastRound[k], g_lastRound[slot]);
            std::swap(g_prevGatherOk[k], g_prevGatherOk[slot]);
            std::swap(g_execIdCmd[k], g_execIdCmd[slot]);
            std::swap(g_cpend[k], g_cpend[slot]);
            std::swap(g_outWatch[k], g_outWatch[slot]);
            std::swap(g_spellWatch[k], g_spellWatch[slot]);
            std::swap(g_detached[k], g_detached[slot]);
            std::swap(g_down[k], g_down[slot]);
            if (g_logOn && g_cfg.traceQueue) {
                char line[64];
                qh::fmtIdentSwap(line, sizeof line, k, slot);
                QTRACE("%s", line);
            }
            g_identLostMs[k] = g_identLostMs[slot] = 0;
            g_identLastTick[k] = g_identLastTick[slot] = 0;
            g_prevRiWhy[k] = g_prevRiWhy[slot] = -1;
            g_prevDsWhy[k] = g_prevDsWhy[slot] = -1;
            swapped[k] = swapped[slot] = true;   // judged after the loop
            continue;
        }
        if (qh::companionReplacedInSlot(slot, owner, nowCli[k], nowSrv[k])) {
            if (g_handoff[k].pending) endHandoff(k, qh::DR_IDENTITY_LOST);
            if (bufBusy(g_itemQueue[k])) {
                traceDropAll(k, qh::DR_IDENTITY_LOST);
                g_itemQueue[k].clear();
            }
            discardDetached(k, "identity_lost");
            g_identLostMs[k] = 0; g_identLastTick[k] = 0; skip[k] = false;
            continue;
        }
        // The creature is in no slot (transient unreadable slot): never classify another creature's snapshot.
        skip[k] = true;
        if (!playerPaused()) {
            if (g_identLastTick[k] != 0) { const DWORD d = now - g_identLastTick[k]; g_identLostMs[k] += d < 1000u ? d : 1000u; }
            g_identLastTick[k] = now;
            if (g_identLostMs[k] >= qh::kEatenWaitMaxMs) {
                if (g_handoff[k].pending) endHandoff(k, qh::DR_IDENTITY_LOST);
                if (bufBusy(g_itemQueue[k])) {
                    traceDropAll(k, qh::DR_IDENTITY_LOST);
                    g_itemQueue[k].clear();
                }
                discardDetached(k, "identity_lost");
                g_identLostMs[k] = 0; g_identLastTick[k] = 0; skip[k] = false;
            }
        } else {
            g_identLastTick[k] = 0;
        }
    }

    // RL2: judge swapped buffers after ALL swaps (a 3-way rotation left skip[0] set for a buffer that ended in place). Still in the wrong slot = wait one tick.
    for (int k = 0; k < 3; ++k)
        if (swapped[k]) skip[k] = bufBusy(g_itemQueue[k]) && slotOfCreature(tbl, bufOwner(g_itemQueue[k])) != k;
}

// Multi-item buffer ticker and follower wipe guard
void tickItemQueues(DWORD now, bool inMenu) {
    outWatchTick(now);
    const bool combatNow = g_combat != 0 && !g_combatUnreadable;
    bool edgeFlags[3] = {false, false, false};
    // unreadable HUD flags: no edge and g_prevCombat is kept (QH-2a-07), so a gap cannot fake a rising edge
    for (int k = 0; k < 3; ++k) edgeFlags[k] = qh::combatEdgeStep(g_prevCombat[k], !g_combatUnreadable, g_combat != 0);
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    {
        const int8_t ok = tbl ? 1 : 0;
        if (ok != g_prevTblOk) {
            if (g_prevTblOk >= 0 && (bufBusy(g_itemQueue[0]) || bufBusy(g_itemQueue[1]) || bufBusy(g_itemQueue[2])))
                traceEdge("party_table", g_prevTblOk != 0, ok != 0, "unreadable");
            g_prevTblOk = ok;
        }
    }
    bool skip[3] = {false, false, false};
    if (tbl) {
        syncPartyIdentity(tbl, now, skip);
        static uintptr_t g_lastRotSrv[3] = {0, 0, 0};
        uintptr_t nowSrv[3] = {0, 0, 0};
        for (int k = 0; k < 3; ++k) { const uintptr_t c = party::member(tbl, k); nowSrv[k] = c ? party::serverCreature(c) : 0; }
        if ((nowSrv[0] || nowSrv[1] || nowSrv[2]) &&
            (nowSrv[0] != g_lastRotSrv[0] || nowSrv[1] != g_lastRotSrv[1] || nowSrv[2] != g_lastRotSrv[2])) {
            for (int k = 0; k < 3; ++k) g_lastRotSrv[k] = nowSrv[k];
            if (g_logOn && g_cfg.traceQueue) {
                uintptr_t owners[3] = {};
                for (int k = 0; k < 3; ++k) {
                    const uintptr_t rawOwner = bufBusy(g_itemQueue[k]) ? bufOwner(g_itemQueue[k]) : 0;
                    const int ownerSlot = rawOwner ? slotOfCreature(tbl, rawOwner) : -1;
                    owners[k] = (ownerSlot >= 0) ? nowSrv[ownerSlot] : rawOwner;
                }
                char line[qh::kLogLineMax];
                qh::fmtPartyRot(line, sizeof line, g_combat != 0, inMenu, nowSrv, owners);
                QTRACE("%s", line);
            }
        }
    }
    if (inMenu || g_combat != 0) {
        if (combatNow) { g_lastCombatTick = now; for (int k = 0; k < 3; ++k) if (edgeFlags[k]) { g_roundOrder[k] = RoundOrder(); discardDetached(k, "new_combat"); }   // a new combat starts clean
                         combatEdgeTrace(now, edgeFlags); handoffTick(now, edgeFlags); roundOrderTick(now, edgeFlags); detachTick(now, false); roundWatchTick(now); }   // unreadable HUD flags: no hand-off and no drop, the buffers wait
        return;
    }
    if (!tbl) return;

    // Combat ended before a hand-off drained. RQ8: an unsent re-send head goes back to the front of its buffer (the sequential dispatch below sends it);
    // an engine-held head stays the engine's and is dropped as before. The still-buffered commands run through the sequential dispatch below.
    for (int k = 0; k < 3; ++k) {
        Handoff& h = g_handoff[k];
        if (!h.pending) continue;
        // R2: the vanilla Cancel Combat button (BTN_CLEARALL) ends combat in the same press; a head the player just cancelled must not come back.
        if (qh::cancelClickRecent(g_lastClickTick, h.edgeAt, now)) {
            endHandoff(k, qh::DR_CANCEL); traceDropAll(k, qh::DR_CANCEL); g_itemQueue[k].clear();
            continue;
        }
        qh::QueuedAction lost;
        const uint32_t headCmd = h.head.cmdId;
        // RV1 EDGE_WAIT sets headHeld=true together with h.waiting ("not in the engine yet"): that head is still ours, so it is restorable.
        const qh::RestoreHead rh = qh::restoreHead(h.headValid, h.headHeld && !h.waiting, h.head, g_itemQueue[k], &lost);
        if (rh != qh::RH_NONE) {
            h.headValid = false;   // restored: endHandoff must not log it as a DROP
            QLOG(chMain, "HANDOFF_RESTORE m=%d cmd=%u pos=front dropped=%u", k, (unsigned)headCmd, rh == qh::RH_FRONT_DROPPED ? (unsigned)lost.cmdId : 0u);
            if (rh == qh::RH_FRONT_DROPPED && g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, lost.cmdId, k, qh::DR_BUFFER_FULL, 1); QTRACE("%s", line); }
        }
        endHandoff(k, qh::DR_COMBAT_CLEAR);
    }
    detachTick(now, true);   // RQ9: detached round nodes are reattached (or discarded after kDetachEndGraceMs) once the restored head / buffered commands drained
    // RQ6: an action still tracked from before the last combat (B1 hold keeps buf.active through a fight) has unknown history; stop tracking it, never re-inject it.
    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& b = g_itemQueue[k];
        if (!skip[k] && b.active && !qh::shouldTrackAfterCombat(b.activeStartTick, g_lastCombatTick)) resolveActive(k, b, now, "combat_end");
    }

    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& buf = g_itemQueue[k];
        uintptr_t cli = party::member(tbl, k);
        uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (!srv) {
            if (buf.count > 0 || buf.active) {
                traceDropAll(k, qh::DR_NULL_CREATURE);
                buf.clear();
            }
            continue;
        }
        if (skip[k]) {
            if (bufBusy(buf)) skipTrace(k, "tick_ident", k, true, bufHead(buf).cmdId, buf.count);
            continue;
        }
        skipTraceClear(k);
        if (k > 0 && bufBusy(buf) && memberDown(k, srv, now)) {   // RL9: bounded hold (an invisible stale queue must not fire minutes later)
            if (g_downSince[k] == 0) { g_downSince[k] = now; if (g_logOn && g_cfg.traceQueue) QTRACE("DOWN_HOLD m=%d cmd=%u", k, (unsigned)bufHead(buf).cmdId); }
            else if (qh::downHoldExpired(g_downSince[k], now)) { traceDropAll(k, qh::DR_MEMBER_DOWN); buf.clear(); g_downSince[k] = 0; continue; }
        } else g_downSince[k] = 0;
        // B1 (QL17): the client Perform* wrappers act on the controlled creature (slot 0); a buffer owned by anyone else is held (never dispatched or re-injected)
        // unless its next action can take the server route (QL20: Force power on an object target, dispatchAction -> serverCast).
        const bool ownerCtrl = !bufBusy(buf) || mayDispatch(tbl, bufHead(buf));

        Snapshot s;
        const bool gOk = gatherFor(srv, s);
        if ((gOk ? 1 : 0) != g_prevGatherOk[k]) {
            if (g_prevGatherOk[k] >= 0 && bufBusy(buf)) {
                char nm[16];
                snprintf(nm, sizeof nm, "gather_m%d", k);
                traceEdge(nm, g_prevGatherOk[k] != 0, gOk, "unreadable");
            }
            g_prevGatherOk[k] = gOk ? 1 : 0;
        }
        if (!gOk) {
            if (bufBusy(buf)) skipTrace(3 + k, "tick_gather", k, true, bufHead(buf).cmdId, buf.count);
            continue;
        }
        skipTraceClear(3 + k);

        // Stale-cur bookkeeping: a flagless item/spell round node that sits unpaused with aa8/ab8 == 0 goes stale.
        if (s.haveCur && !s.paused && s.aa8 == 0 && s.ab8 == 0) { if (buf.staleCurSince == 0) buf.staleCurSince = now; }
        else buf.staleCurSince = 0;
        const bool creal = qh::curReal(s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.aa8, s.ab8, s.paused, buf.staleCurSince, now);

        // 1. Engine-busy test, deliberately NOT matched to buf.activeAction: dispatching while ANY spell/item holds the creature would ClearAllActions it (doc 30 QH4).
        //    Known limit (BG23): an unrelated engine-initiated cast can look like our eaten action (AV_PRESENT) until it ends.
        //    RL7: once the action has run (sawExecuting) only an entry/node carrying its own spell / item id keeps AV_PRESENT.
        bool hasFollowLeader = false;
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader
        const int engNe = s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries;
        const bool actionInQueue = qh::actionPresentIn(buf.activeAction, buf.active && buf.sawExecuting, s.e, engNe, s.rn, s.nrn, creal, s.haveCur ? &s.cur : nullptr);
        int types[qh::kMaxEntries];
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) types[i] = s.e[i].type;
        uint32_t p0[qh::kMaxEntries] = {};
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) p0[i] = s.e[i].param0;
        qh::ExecTie tie;
        const qh::ExecTie* tp = nullptr;
        if (buf.active) {   // RQ7: tie the executing evidence to the active action
            buf.noteFlags(qh::flagBits(s.aa8, s.ab8));
            tie.spell = buf.activeAction.kind == qh::ActionKind::Spell;
            tie.id = buf.activeAction.spellId;
            tie.curSpell = s.haveCur ? s.cur.spellId : 0;
            tie.flagBase = buf.flagBase;
            tp = &tie;
        }
        const qh::EngineView ev = qh::makeEngineView(types, s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries, actionInQueue,
                                                     s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.nrn, s.aa8, s.ab8, s.paused,
                                                     buf.staleCurSince, now, p0, tp);
        if (tp && !tie.spell && g_execIdCmd[k] != buf.activeAction.cmdId && g_logOn && g_cfg.traceQueue)
            for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i)
                if (types[i] == 0x2E || types[i] == 0x0F) {
                    g_execIdCmd[k] = buf.activeAction.cmdId;
                    QTRACE("EXEC_ID cmd=%u m=%d type=%02x want=%08x got=%08x", (unsigned)buf.activeAction.cmdId, k, types[i], (unsigned)buf.activeAction.clientItem, (unsigned)p0[i]);
                    break;
                }
        // One STALE_CUR line per episode, only while the buffer is busy (idle path emits nothing).
        if (s.haveCur && !creal) {
            if (!buf.staleCurLogged && bufBusy(buf) && g_logOn && g_cfg.traceQueue) {
                buf.staleCurLogged = true;
                char line[200];
                qh::fmtStaleCur(line, sizeof line, k, (int)s.cur.subType, buf.staleCurSince ? now - buf.staleCurSince : 0,
                                s.aa8 != 0, s.ab8 != 0, s.paused);
                QTRACE("%s", line);
            }
        } else {
            buf.staleCurLogged = false;
        }

        if (buf.active && actionInQueue) buf.seenInQueue = true;   // bookkeeping only (RESOLVE seen=)
        // RQ4: orphan round node. ClearAllActions (object method 0x541080 only; queue-hud's own Cancel, the app-level 0x51CB10, does empty it) never empties the round list, so after a wipe a 9/10 node can sit with no 0x3F/0x0F/0x2E entry to consume it:
        // it keeps the action PRESENT and blocks dispatch (DS_NRN_BUSY). kOrphanMs unpaused with no real cur: remove the list (no ServerClearAllActions), let the verdict re-send.
        {
            bool consumer = false;
            for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) consumer = consumer || qh::entryConsumesRound(s.e[i].type);
            const unsigned omask = qh::itemSpellMask(s.rn, s.nrn);   // R3: remove only the 9/10 orphans, never another subtype's node
            uint32_t age = 0;
            if (bufBusy(buf)) age = qh::orphanRoundAge(consumer, creal, omask != 0, s.paused, g_orphanClk[k], now); else g_orphanClk[k].reset();
            if (age >= qh::kOrphanMs) {
                const int removed = removeRoundNodes(srv, s, omask);
                char line[200];
                const uint32_t cmd = buf.active ? buf.activeAction.cmdId : buf.queue[0].cmdId;
                qh::fmtOrphan(line, sizeof line, k, cmd, removed, age);
                QLOG(chMain, "%s", line);
                if (removed >= 0 && buf.active) buf.eaten = true;   // else rule 3 would resolve it UNSEEN (lost)
                g_orphanClk[k].reset();                             // a failed removal retries after another kOrphanMs, never every frame
                g_lastOurs[k].tag = "orphan"; g_lastOurs[k].tick = now;
                continue;                                           // s is stale: the next frame re-reads the queue
            }
        }
        // QL20: any tracked use/cast on a non-controlled member (server route, or one the engine took directly while the member still led) must not starve behind 0x3D
        if (k > 0 && buf.active && g_cfg.companionServerCast) { if (kickStarved(k, srv, buf, s, now)) continue; }
        else g_kickClk[k].reset();

        // 2. Follower wipe protection & active completion tracking
        qh::ReInjectWhy riWhy = qh::reInjectWhy(buf, actionInQueue, hasFollowLeader, s.paused, now, false,
                                                qh::RiGate{ true, g_cfg.pauseEatenWait, g_cfg.pauseReinject });
        if (riWhy == qh::RI_OK && !ownerCtrl) riWhy = qh::RI_NOT_CTRL;
        if (bufBusy(buf)) {
            if (static_cast<int>(riWhy) != g_prevRiWhy[k]) {
                g_prevRiWhy[k] = static_cast<int>(riWhy);
                if (buf.active && riWhy != qh::RI_OK && g_logOn && g_cfg.traceQueue)   // inactive+buffered is the WAIT line's job
                    QTRACE("REINJECT_NO cmd=%u m=%d why=%s", (unsigned)buf.activeAction.cmdId, k, qh::whyName(riWhy));
            }
        } else {
            g_prevRiWhy[k] = -1;
        }
        if (buf.active) {
            if (buf.dispatchDeferred) {   // RQ5: a refused server cast waits and retries; no verdict, no re-inject meanwhile
                const qh::DeferStep ds = qh::deferStep(buf, s.paused, now);
                if (ds == qh::DEFER_GIVE_UP) {
                    if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_REFUSED, 1); QTRACE("%s", line); }
                    endAction(buf);
                } else if (ds == qh::DEFER_RETRY) {
                    const SendResult r = dispatchAction(buf.activeAction, k, true);
                    if (r == SendResult::Sent) buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8));
                    afterSend(k, buf, r, now);
                }
            } else if (riWhy == qh::RI_OK) {
                if (!saferead::probeRead(SiteQueue, reinterpret_cast<const void*>(buf.activeAction.creature), 4)) {
                    QLOGV(chMain, KPLOG_WARN, "re-inject skipped: creature %08x unreadable for member %d",
                          (unsigned)buf.activeAction.creature, k);
                    if (g_logOn && g_cfg.traceQueue) {   // only the active command is lost; buffered ones stay queued
                        char line[200];
                        qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_UNREADABLE_CREATURE, 1);
                        QTRACE("%s", line);
                    }
                    buf.active = false;
                    buf.seenInQueue = false;
                } else {
                    buf.activeAction.reInjectCount++;
                    buf.lastReInjectTick = now;
                    g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                    QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d, paused=%d) cmd=%u",
                         buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                         k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.activeAction.reInjectCount,
                         s.paused ? 1 : 0, (unsigned)buf.activeAction.cmdId);
                    buf.reInjectPending = true;
                    const SendResult r = dispatchAction(buf.activeAction, k);
                    buf.activeStartTick = now;
                    buf.reInjectPending = false;
                    afterSend(k, buf, r, now);
                }
            } else if (s.round != 0) {
                buf.noteView(actionInQueue, now);
                buf.accrueEaten(s.paused, now);
                qh::ActiveVerdict av = qh::activeVerdict(buf, ev, now, g_cfg.pauseEatenWait);
                if (!g_cfg.pauseReinject && (av == qh::AV_EATEN_WAIT || av == qh::AV_EATEN_REINJECT || av == qh::AV_GIVE_UP)) av = qh::AV_UNSEEN;
                if (!ownerCtrl && av == qh::AV_EATEN_REINJECT) av = qh::AV_EATEN_WAIT;   // B1: hold, do not re-inject on the controlled creature
                switch (av) {
                    case qh::AV_PRESENT:
                        buf.notePresent(ev.executing, actionInQueue, now, ev.expanded);
                        break;
                    case qh::AV_PRESENT_TIMEOUT:   // RQ4 ceiling: PRESENT for kPresentCeilingMs unpaused; abandon tracking (an orphan node, if any, goes through ORPHAN next)
                        if (g_logOn && g_cfg.traceQueue) { char line[200]; qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_PRESENT_TIMEOUT, 1); QTRACE("%s", line); }
                        endAction(buf);
                        break;
                    case qh::AV_EXECUTED:
                        traceVanish(k, qh::VK_EXECUTED, buf, s, ev, now);
                        resolveActive(k, buf, now);
                        break;
                    case qh::AV_UNSEEN:
                        traceVanish(k, qh::VK_UNSEEN, buf, s, ev, now);
                        resolveActive(k, buf, now);
                        break;
                    case qh::AV_EATEN_WAIT: {
                        const bool first = !buf.eaten;
                        buf.eaten = true;
                        if (first) traceVanish(k, qh::VK_EATEN_WAIT, buf, s, ev, now);
                        break;
                    }
                    case qh::AV_EATEN_REINJECT: {
                        const bool first = !buf.eaten;
                        buf.eaten = true;
                        if (!saferead::probeRead(SiteQueue, reinterpret_cast<const void*>(buf.activeAction.creature), 4)) {
                            QLOGV(chMain, KPLOG_WARN, "re-inject skipped: creature %08x unreadable for member %d",
                                  (unsigned)buf.activeAction.creature, k);
                            if (g_logOn && g_cfg.traceQueue) {   // only the active command is lost; buffered ones stay queued
                                char line[200];
                                qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, qh::DR_UNREADABLE_CREATURE, 1);
                                QTRACE("%s", line);
                            }
                            endAction(buf);
                            break;
                        }
                        if (first) traceVanish(k, qh::VK_EATEN_PAUSED, buf, s, ev, now);
                        buf.eatenReInjects++;
                        buf.totalReInjects++;
                        buf.lastReInjectTick = now;
                        g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                        QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d) cmd=%u why=eaten_paused",
                             buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                             k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.eatenReInjects,
                             (unsigned)buf.activeAction.cmdId);
                        const SendResult r = dispatchAction(buf.activeAction, k);
                        buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8));
                        afterSend(k, buf, r, now);
                        break;
                    }
                    case qh::AV_GIVE_UP: {
                        const qh::DropWhy why = buf.eatenUnpausedMs >= qh::kEatenWaitMaxMs ? qh::DR_EATEN_TIMEOUT : qh::DR_REINJECT_EXHAUSTED;
                        if (g_logOn && g_cfg.traceQueue) {
                            char line[200];
                            qh::fmtDrop(line, sizeof line, buf.activeAction.cmdId, k, why, 1);
                            QTRACE("%s", line);
                        }
                        endAction(buf);   // the rest of the buffer continues
                        break;
                    }
                }
            }
        }

        // 3. Sequential dispatch of next buffered action (idle gate: gateNe == 0 && !haveCur && nrn == 0; gateNe skips a resting 0x3D)
        qh::DispatchWhy dsWhy = qh::dispatchWhy(buf, s.gateNe, creal, s.nrn);
        bool flagCapped = false;
        if (dsWhy == qh::DS_OK && !ownerCtrl) dsWhy = qh::DS_NOT_CTRL;   // B1: held until its owner leads again
        if (dsWhy == qh::DS_OK && g_cfg.dispatchWaitFlags) {
            const bool flagsBusy = !s.paused && (s.aa8 != 0 || s.ab8 != 0);
            if (!g_flagGate[k].step(flagsBusy, now, &flagCapped, s.paused)) dsWhy = qh::DS_FLAG_BUSY;
        } else {
            g_flagGate[k].reset();
        }
        if (dsWhy != qh::DS_OK && buf.count > 0) {
            // A buffered command is waiting: one WAIT on edge (why changed) plus a 1 s heartbeat.
            if (static_cast<int>(dsWhy) != g_prevDsWhy[k] || now - g_lastWaitTick[k] >= 1000u) {
                g_prevDsWhy[k] = static_cast<int>(dsWhy);
                g_lastWaitTick[k] = now;
                if (g_logOn && g_cfg.traceQueue) {
                    char line[200];
                    qh::fmtWait(line, sizeof line, buf.queue[0].cmdId, k, dsWhy, buf.count, s.engineNe, now - buf.queue[0].queueTime);
                    QTRACE("%s", line);
                }
            }
        } else {
            g_prevDsWhy[k] = -1;
        }
        if (dsWhy == qh::DS_OK && qh::shouldDispatchNext(buf, s.gateNe, creal, s.nrn)) {
            qh::QueuedAction nextA;
            if (buf.pop(&nextA)) {
                if (!saferead::probeRead(SiteQueue, reinterpret_cast<const void*>(nextA.creature), 4)) {
                    QLOGV(chMain, KPLOG_WARN, "dispatch skipped: creature %08x unreadable for member %d",
                          (unsigned)nextA.creature, k);
                    if (g_logOn && g_cfg.traceQueue) {
                        char line[200];
                        qh::fmtDrop(line, sizeof line, nextA.cmdId, k, qh::DR_UNREADABLE_CREATURE, 1);
                        QTRACE("%s", line);
                    }
                } else {
                    g_lastOurs[k].tag = "dispatch"; g_lastOurs[k].tick = GetTickCount();
                    buf.seenInQueue = false;
                    QLOG(chMain, "dispatching buffered %s for member %d (id=%08x, target=%08x, remaining=%d) cmd=%u",
                         nextA.kind == qh::ActionKind::Spell ? "spell" : "item",
                         k, nextA.clientItem, nextA.targetId, buf.count, (unsigned)nextA.cmdId);
                    if (flagCapped) QLOG(chMain, "dispatch hold=flag_cap member=%d cmd=%u aa8=%d ab8=%d held=%u", k, (unsigned)nextA.cmdId, s.aa8, s.ab8, (unsigned)(now - g_flagGate[k].holdSince));
                    buf.active = true;
                    buf.activeAction = nextA;
                    buf.markDispatched(now, qh::flagBits(s.aa8, s.ab8));
                    const SendResult r = dispatchAction(nextA, k);
                    afterSend(k, buf, r, now);
                }
            }
            g_flagGate[k].reset();
        }
    }
}

// ---- Queue journal (QJ): logs a member's engine queue only when its signature changes. Reads via actq::read only (never gatherFor). ----
constexpr DWORD kJournalSampleMs = 100;    // sample period
constexpr int kJournalPerSec = 4;          // max QJ lines per second per member
constexpr int kJournalPerSecCombat = 4;    // ... while in combat
constexpr int kJournalSessionCap = 5000;   // QJ lines per session, then one journal_cap marker
constexpr DWORD kOursWindowMs = 2000;      // a g_lastOurs stamp newer than this makes the change "ours"

struct JournalMember {
    qh::QueueSig sig;
    bool haveSig = false;
    int8_t crOk = -1;          // last creature state: -1 unknown, 0 lost, 1 readable
    DWORD winStart = 0;
    int winCount = 0;
};
JournalMember g_jm[3];
DWORD g_jLastSample = 0;
int g_jTotal = 0;
bool g_jCapped = false;

void journalTick(DWORD now) {
    if (!g_logOn || !g_cfg.traceQueue || g_jCapped) return;
    if (g_jLastSample && now - g_jLastSample < kJournalSampleMs) return;
    g_jLastSample = now;
    if (g_jTotal >= kJournalSessionCap) {
        g_jCapped = true;
        const int zero = 0;
        char line[200];
        qh::fmtEdge(line, sizeof line, "journal_cap", false, true, "hud", &zero, 1);
        QTRACE("%s", line);
        return;
    }
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    for (int k = 0; k < 3; ++k) {
        JournalMember& jm = g_jm[k];
        const uintptr_t m = tbl ? party::member(tbl, k) : 0;
        const uintptr_t srv = m ? party::serverCreature(m) : 0;
        if (k > 0 && memberDown(k, srv, now)) { jm.haveSig = false; continue; }   // Down companion: not polled; the first poll after wake is a fresh baseline
        actq::Snapshot a;
        const bool ok = srv && actq::read(srv, &a, SiteQueue, SiteRound);
        const int8_t okv = ok ? 1 : 0;
        if (okv != jm.crOk) {
            if (jm.crOk >= 0) traceEdge("creature", jm.crOk != 0, ok, "unreadable");
            jm.crOk = okv;
        }
        if (!ok) continue;   // unreadable is not a change: keep the last signature across the gap

        qh::QEntry q[qh::kMaxEntries] = {};   // RL14: the signature hashes entry types 8..15 too
        for (int i = 0; i < a.ne && i < qh::kMaxEntries; ++i) { q[i].type = a.e[i].type; q[i].param0 = a.e[i].param0; }
        const qh::QueueSig sig = qh::makeQueueSig(q, a.ne, a.nrn, a.haveCur ? static_cast<int>(a.cur.subType) : -1, a.aa8 != 0, a.ab8 != 0);
        if (!jm.haveSig) { jm.sig = sig; jm.haveSig = true; continue; }   // first sight is the baseline, not a change
        if (qh::sigEqual(jm.sig, sig)) continue;

        const bool combat = g_combat != 0;
        if (now - jm.winStart >= 1000u) { jm.winStart = now; jm.winCount = 0; }
        if (jm.winCount >= (combat ? kJournalPerSecCombat : kJournalPerSec)) continue;   // rate-capped: the change is logged once the window rolls
        ++jm.winCount;
        jm.sig = sig;
        ++g_jTotal;

        const DWORD tickNow = GetTickCount();
        const char* tag = nullptr;
        DWORD age = 0;
        if (g_lastOurs[k].tag && tickNow - g_lastOurs[k].tick <= kOursWindowMs) { tag = g_lastOurs[k].tag; age = tickNow - g_lastOurs[k].tick; }
        char line[200];
        qh::fmtQj(line, sizeof line, k, sig, playerPaused(), combat, tag, age);
        QTRACE("%s", line);
    }
}

// EDGE lines for combat / paused / menu: one per change, previous values kept here. First observation is a baseline (silent).
void edgeTick(bool inMenu) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    static bool init = false;
    static bool pCombat = false, pPaused = false, pMenu = false;
    const bool combat = g_combat != 0;
    const bool paused = playerPaused();
    if (!init) { init = true; pCombat = combat; pPaused = paused; pMenu = inMenu; return; }
    if (combat != pCombat) { traceEdge("combat", pCombat, combat, g_combatUnreadable ? "unreadable" : "hud"); pCombat = combat; }
    if (paused != pPaused) { traceEdge("paused", pPaused, paused, "hud"); pPaused = paused; }
    if (inMenu != pMenu) { traceEdge("menu", pMenu, inMenu, "hud"); pMenu = inMenu; }
}

// QH27: remove the one action behind strip icon j of member k (1-2). Buffered / active buffered: out of our buffer (found again by
// command number, the buffer may have moved since the draw). Round node: out of the engine round list (removeRoundNodes, identity
// re-checked against a fresh snapshot); when it was our active action or the held hand-off head, that is dropped too so neither the
// verdict nor the hand-off re-sends it. Not a Cancel click: g_lastClickTick stays (the hand-off would drop the whole queue).
void cancelOne(int k, int j) {
    const StripHit& sh = g_stripHit[k];
    if (j < 0 || j >= sh.n) return;
    const StripPick p = sh.p[j];
    char line[200];
    party::Fail pwhy = party::F_None;
    const uintptr_t tbl = party::table(&pwhy);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    const uintptr_t srv = m ? party::serverCreature(m) : 0;
    const char* bad = nullptr;
    if (!srv || srv != sh.creature) bad = "member_changed";
    else if (GetTickCount() - g_capTick >= 250u) bad = "stale";
    qh::ActionBuffer& buf = g_itemQueue[k];
    uint32_t cmd = p.cmd;
    if (!bad && (p.kind == qh::SK_BUFFERED || p.kind == qh::SK_ACTIVE)) {
        const int at = qh::findBuffered(buf, p.cmd, p.kind == qh::SK_ACTIVE, p.bufIdx, p.drawn);
        qh::QueuedAction a;
        if (at == qh::kBufActive ? qh::dropActive(buf, &a) : qh::removeAt(buf, at, &a)) {
            cmd = a.cmdId;
            if (g_logOn && g_cfg.traceQueue) { qh::fmtDrop(line, sizeof line, a.cmdId, k, qh::DR_CANCEL_ONE, 1); QTRACE("%s", line); }
        } else bad = "gone";
    } else if (!bad && p.kind == qh::SK_ROUND) {
        Snapshot s;
        int i = -1;
        if (!gatherFor(srv, s)) bad = "unreadable";
        else for (int q = 0; q < s.nrn && i < 0; ++q) if (s.rn[q].ptr == p.node) i = q;
        if (!bad && i < 0) bad = "gone";
        if (!bad && removeRoundNodes(srv, s, 1u << i) != 1) bad = "guard";
        if (!bad) {
            const qh::RNode node = s.rn[i];
            if (buf.active && qh::detail::nodeIsAction(buf.activeAction, node)) {
                qh::QueuedAction a;
                qh::dropActive(buf, &a);
                cmd = a.cmdId;
                if (g_logOn && g_cfg.traceQueue) { qh::fmtDrop(line, sizeof line, a.cmdId, k, qh::DR_CANCEL_ONE, 1); QTRACE("%s", line); }
            }
            Handoff& h = g_handoff[k];
            if (h.pending && h.headValid && qh::nodeIsHead(node, h.head)) {
                if (!cmd) cmd = h.head.cmdId;
                if (g_logOn && g_cfg.traceQueue) { qh::fmtDrop(line, sizeof line, h.head.cmdId, k, qh::DR_CANCEL_ONE, 1); QTRACE("%s", line); }
                h.headValid = false; h.headHeld = false;
            }
        }
    } else if (!bad) bad = "engine_owned";   // the click pass never offers these; kept so a future caller cannot remove one by accident
    qh::fmtCancelOne(line, sizeof line, k, j, p.kind, cmd, bad == nullptr, bad);
    QLOGV(chCancel, bad ? KPLOG_WARN : KPLOG_INFO, "%s", line);
    if (!bad) { g_heldStrip[k] = HeldStrip(); g_lastOurs[k].tag = "cancel_one"; g_lastOurs[k].tick = GetTickCount(); }   // RL17: no hold repaint of the removed icon
}

// QH27 click pass (every FRAME_BEGIN): hover + LMB on a removable companion strip icon. Works in combat too (strips show there with
// show_in_combat=1); off with cancel=0, in menus and once the HUD capture is stale (the labels then hide so they never swallow clicks).
void stripClickTick(bool ledge, bool inMenu) {
    const bool live = g_cfg.cancel && g_cfg.companions && !inMenu && GetTickCount() - g_capTick < 250u;
    if (!live) { eatHideAll(); g_stripHover[1] = g_stripHover[2] = -1; return; }
    int cx = 0, cy = 0;
    const bool xy = guikit::cursorEngine(&cx, &cy);
    for (int k = 1; k <= 2; ++k) {
        const StripHit& sh = g_stripHit[k];
        int j = xy ? qh::stripHit(sh.r, sh.n, cx, cy) : -1;
        if (j >= 0 && !qh::slotRemovable(sh.p[j].kind)) j = -1;
        if (j >= 0 && j != g_stripHover[k]) guikit::playHoverSound();
        g_stripHover[k] = j;
        if (ledge && j >= 0) cancelOne(k, j);
    }
}

// RQ11: does a left press at engine px (x,y) land on a Cancel control (ours, or the vanilla combat Cancel / clear-one boxes)? Only these count as the player's cancel.
static bool cancelClickAt(int x, int y) {
    qh::Rect r[6]; int n = 0;
    for (const CW& w : g_cw) if (w.c && w.vis == 1 && n < 6) r[n++] = w.r;
    if (g_hud) for (int off : { HudClearAll, HudClearOne, HudClearOne2 }) {
        qh::Rect q{}; const uintptr_t c = g_hud + off;
        if (n < 6 && guikit::isVisible(c) && guikit::getExtent(c, &q.x, &q.y, &q.w, &q.h)) r[n++] = q;
    }
    return qh::clickIsCancel(r, n, x, y);
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    saferead::beginScope();
    tex::ensure();
    tex::heal();   // every frame, before any gate: a GL reset can happen in menus too
    const bool menuKnown = ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t);
    const bool inMenu = menuKnown && (ctx->inMenu != 0);
    const bool ledge = guikit::clickEdge(g_lmb);   // polled every frame so a held press never fires on reopen
    if (ledge) {
        int cx = 0, cy = 0;
        if (!guikit::cursorEngine(&cx, &cy) || cancelClickAt(cx, cy)) g_lastClickTick = GetTickCount();   // RQ11: unknown cursor keeps the old behaviour; other clicks never read as Cancel
    }
    if (ledge && g_logOn && g_cfg.traceQueue && g_cfg.traceClicks) {
        int tx = 0, ty = 0;
        const bool xy = guikit::cursorEngine(&tx, &ty);
        char line[80];
        qh::fmtClick(line, sizeof line, GetTickCount(), xy, tx, ty);
        QTRACE("%s", line);
    }
    for (int k = 0; k < 3; ++k) {   // RQ12: two samples per Cancel (next frame, ~1500 ms later): does the follower AI re-add its FollowLeader 0x3D?
        CPending& cp = g_cpend[k];
        if (cp.on) {
            const DWORD t = GetTickCount();
            if (!cp.late || t - cp.at >= 1500u) {
                int after3D = -1;
                const int nn = queueCountOf(k, nullptr, nullptr, &after3D);
                QLOG(chCancel, "CANCEL_AFTER m=%d n=%d had3D=%d after3D=%d late=%d", k, nn, cp.had3D, after3D, cp.late ? 1 : 0);
                if (cp.late) {
                    if (k >= 1 && cp.had3D > 0 && after3D == 0) QLOGV(chCancel, KPLOG_WARN, "CANCEL_LOST3D m=%d had3D=%d after3D=%d", k, cp.had3D, after3D);
                    cp = CPending();   // done
                } else cp.late = true;   // keep the entry for the ~1500 ms sample
            }
        }
    }
    const bool hardCombat = hardCombatNow();
    if (!g_cfg.cancel || inMenu || hardCombat) {
        cwHideAll(inMenu ? "hud-inactive" : hardCombat ? "combat" : "cancel-off");
    } else if (GetTickCount() - g_capTick >= 250u) {
        cwHideAll("stale capture");   // RL19: HUD_DRAW stopped (dialogue / cutscene / load): a leftover Cancel must not swallow clicks
    } else {
        for (int k = 0; k < 3; ++k) {
            CW& w = g_cw[k];
            if (w.c && !w.tipSet && guitip::nativeSupported()) {
                const char* tip = (k == 0) ? "Cancel queued actions" : "Cancel queued actions";
                if (guitip::setNative(w.c, tip)) w.tipSet = true;
            }
        }
        int cx = 0, cy = 0, on = -1;
        if (guikit::cursorEngine(&cx, &cy)) {
            for (int k = 0; k < 3; ++k) {
                CW& w = g_cw[k];
                const bool h = w.c && guikit::hit(w.c, cx, cy);   // hit() needs the visible flag: a hidden control never matches
                if (h && !w.hover) guikit::playHoverSound();
                w.hover = h;
                if (h) on = k;
            }
            if (ledge && on >= 0) doCancel(on);
        }
    }
    stripClickTick(ledge, inMenu);

    if (inMenu && !g_modeDrew) { g_modeStrip[0] = g_modeStrip[1] = g_modeStrip[2] = "hidden:menu"; g_modeHidden[0] = g_modeHidden[1] = g_modeHidden[2] = 0; modeEmit(true); }   // HUD_DRAW does not run in a menu frame; one MODE pass per frame either way
    g_modeDrew = false;
    const DWORD now = GetTickCount();
    edgeTick(inMenu);
    journalTick(now);
    tickItemQueues(now, inMenu);
}

// ---- soft-combat banner / red-bar hide ----
// The engine (FUN_0074E910, every frame, acts only on a combat-mode toggle) sets/clears the visible bit (0x2 of the control flags dword at
// control base + 0x48) of these controls. We clear OUR OWN visible bits while the combat is soft and restore them if a hostile appears
// before combat ends. Never flip bit 1 of [hud+0xA904] (reruns the toggle: minimap slide, camera). Never touch hud+0x6E54: on leaving combat
// 0x74E910 reads its bit to decide the panel slide-back. The vanilla queue slots (0x5F2C + i*0x148 + 0x48) are not ours either.
constexpr int kBannerCtl[] = {
    0x6F9C,   // LBL_CMBTMODEMSG flags (banner text)
    0x70E4,   // LBL_CMBTMSGBG flags (banner background)
    0x6494,   // LBL_COMBATBG2 flags (red bar)
    0x65DC,   // LBL_COMBATBG3 flags (red bar)
    0x6724,   // unnamed third combat BG flags (red bar candidate)
    0x68E4,   // BTN_CLEARONE flags
    0x6AB4,   // BTN_CLEARONE2 flags
    0x6C84,   // BTN_CLEARALL flags
};
constexpr int kBannerCtlN = static_cast<int>(sizeof kBannerCtl / sizeof kBannerCtl[0]);
constexpr int kBannerPair = 2;   // kBannerCtl[0..1] = hide_banner_soft, [2..] = hide_redbar_soft
constexpr unsigned kVisibleBit = static_cast<unsigned>(guikit::FlagVisible);
int g_bannerLastAct = -1;        // last BannerAct logged (change-only trace)
bool g_bannerResetLogged = false;

inline bool bannerCtlOn(int i) { return i < kBannerPair ? g_cfg.hideBannerSoft : g_cfg.hideRedbarSoft; }

void bannerTrace(const char* act, bool soft, int nWr) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    int nCtl = 0;
    for (int i = 0; i < kBannerCtlN; ++i) nCtl += bannerCtlOn(i) ? 1 : 0;
    char line[200];
    qh::fmtBanner(line, sizeof line, act, soft ? 1 : 0, nCtl, nWr);
    QTRACE("%s", line);
}

// One frame of the soft-combat decision; sets g_softCombat. Writes happen only while g_combat != 0 and the HUD flags are readable.
void softBannerTick(uintptr_t hud) {
    if (g_combatUnreadable) return;               // no verdict, no writes; the state is kept (g_softCombat too)
    g_softCombat = false;
    if (g_bannerHidden && hud != g_bannerHud) {   // HUD rebuilt (load): the old controls are gone
        bannerTrace("forget", false, 0);
        g_bannerHidden = false; g_bannerMask = 0; g_bannerLastAct = -1; g_bannerResetLogged = false;
        g_bannerRestored = false;
    }
    for (bool& ok : g_modeCandOk) ok = false;     // MODE: per-member verdict is recomputed below while in combat
    if (g_combat == 0) {
        qh::softStep(g_softLatch, false, false, false, 0);   // QH26: the latch resets with the combat
        if (g_bannerHidden) {                     // combat ended: the engine hid the controls itself, do not restore
            bannerTrace("forget", false, 0);
            g_bannerHidden = false; g_bannerMask = 0; g_bannerResetLogged = false;
        }
        g_bannerRestored = false;
        g_bannerLastAct = -1;
        return;
    }
    bool soft = false, anyHostile = false;
    {
        party::Fail why = party::F_None;
        const uintptr_t tbl = party::table(&why);
        if (tbl) {
            uint32_t partyIds[3];
            readPartyIds(tbl, partyIds);
            qh::SoftCand cands[3];
            for (int k = 0; k < 3; ++k) {
                const uintptr_t cli = party::member(tbl, k);
                const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
                if (!srv) continue;               // no info for this member
                MemberTgt mt;
                readTargets(srv, mt);
                cands[k] = softCandOf(mt, partyIds);
                g_modeCand[k] = cands[k]; g_modeCandOk[k] = true;   // recorded before the flags-off early return below
                anyHostile = anyHostile || cands[k].anyHostile;
            }
            soft = qh::softCombat(true, cands, 3);
        }
    }
    {   // QH26: hostile latch + confirm delay; one line when the raw verdict says soft but the latch keeps it hard (edge only)
        const bool latched = g_softLatch.sawHostile;
        static bool blockLogged = false;
        g_softCombat = qh::softStep(g_softLatch, true, anyHostile, soft, GetTickCount());
        if (soft && latched && !blockLogged) { blockLogged = true; QLOG(chCancel, "soft verdict kept hard: a hostile was targeted earlier in this combat (QH26)"); }
        if (!latched) blockLogged = false;
    }
    soft = g_softCombat;
    if (!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft) return;   // flags off: verdict still feeds hardCombat and Cancel; no banner writes
    const qh::BannerAct act = qh::bannerAct(g_bannerHidden, true, soft);
    if (act == qh::BA_NONE) { g_bannerLastAct = act; return; }
    uint32_t before[kBannerCtlN] = {}, after[kBannerCtlN] = {};
    int nWr = 0;
    bool engineReset = false;
    for (int i = 0; i < kBannerCtlN; ++i) {
        if (!bannerCtlOn(i)) continue;
        volatile uint32_t* p = reinterpret_cast<volatile uint32_t*>(hud + static_cast<uintptr_t>(kBannerCtl[i]));
        if (!saferead::probeWrite(SiteCancel + 0x20 + static_cast<unsigned>(i), const_cast<uint32_t*>(p), 4)) continue;
        before[i] = *p;
        if (act == qh::BA_HIDE) {
            if (*p & kVisibleBit) {
                if (g_bannerMask & (1u << i)) engineReset = true;   // we cleared it before and the engine set it again
                *p &= ~kVisibleBit;
                g_bannerMask |= 1u << i;
                ++nWr;
            }
        } else if (g_bannerMask & (1u << i)) {                      // restore only what WE cleared
            *p |= kVisibleBit;
            ++nWr;
        }
        after[i] = *p;
    }
    if (act == qh::BA_HIDE) {
        const bool first = !g_bannerHidden;
        if (g_bannerMask) { g_bannerHidden = true; g_bannerHud = hud; g_bannerRestored = false; }
        if (first && nWr > 0) {
            char line[200];
            int n = snprintf(line, sizeof line, "banner flags before=");
            for (int i = 0; i < kBannerCtlN && n < 150; ++i) n += snprintf(line + n, sizeof line - n, "%s%08x", i ? "," : "", (unsigned)before[i]);
            n += snprintf(line + n, sizeof line - n, " after=");
            for (int i = 0; i < kBannerCtlN && n < 190; ++i) n += snprintf(line + n, sizeof line - n, "%s%02x", i ? "," : "", (unsigned)(after[i] & 0xFF));
            QLOG(chCancel, "%s", line);
        }
        if (engineReset && !g_bannerResetLogged) { g_bannerResetLogged = true; bannerTrace("reset", true, nWr); }
        if (static_cast<int>(act) != g_bannerLastAct) bannerTrace("hide", true, nWr);
    } else {
        bannerTrace("restore", false, nWr);
        g_bannerRestored = true;
        g_bannerHidden = false; g_bannerMask = 0; g_bannerResetLogged = false;
    }
    g_bannerLastAct = static_cast<int>(act);
}

} // namespace

extern "C" {

void __cdecl QueueHudCapture(int hud);
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*);

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    g_logOn = g_cfg.log;
    KP_LOG(chMain, "0.4.11-dev: kit=%d frame_alpha=%.2f cancel=%d enabled=%d waiting_dim=%d border_px=%d log=%d companions=%d strip_icon_px=%d strip_side=%s strip_max=%d waiting_icon_dim=%d strip_head_pct=%d", g_cfg.kit ? 1 : 0, g_cfg.frameAlpha, g_cfg.cancel ? 1 : 0, g_cfg.enabled ? 1 : 0, g_cfg.waitingDim ? 1 : 0, g_cfg.borderPx, g_cfg.log ? 1 : 0, g_cfg.companions ? 1 : 0, g_cfg.stripIconPx, g_cfg.stripSide == qh::kSideBelow ? "below" : g_cfg.stripSide == qh::kSideAbove ? "above" : "left", g_cfg.stripMax, g_cfg.waitingIconDim ? 1 : 0, g_cfg.stripHeadPct);
    if (g_iniRejected > 0) QLOGV(chMain, KPLOG_WARN, "queue_hud.ini: %d line(s) not applied (unknown key, bad value or over 255 bytes), first at line %d; those keys keep their defaults", g_iniRejected, g_iniFirstBad);
    QLOG(chMain, "pause_eaten_wait=%d dispatch_wait_flags=%d", g_cfg.pauseEatenWait ? 1 : 0, g_cfg.dispatchWaitFlags ? 1 : 0);
    if (g_cfg.spellReplayPosSeen) QLOG(chMain, "ini: spell_replay_pos is obsolete and ignored (0.4.6-dev)");
    QLOG(chMain, "pause_reinject=%d handoff_on_combat=%d handoff_clear_first=%d handoff_keep_new=%d handoff_resend_held=%d show_in_combat=%d handoff_keep_unpaused=%d trace_mode=%d mode_cap=%d",
         g_cfg.pauseReinject ? 1 : 0, g_cfg.handoffOnCombat ? 1 : 0, g_cfg.handoffClearFirst ? 1 : 0, g_cfg.handoffKeepNew ? 1 : 0, g_cfg.handoffResendHeld ? 1 : 0, g_cfg.showInCombat ? 1 : 0, g_cfg.handoffKeepUnpaused ? 1 : 0, g_cfg.traceMode ? 1 : 0, g_cfg.modeCap);
    QLOG(chMain, "companion_server_cast=%d", g_cfg.companionServerCast ? 1 : 0);
    g_ps.init(api, "queue-hud");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_HUD_DRAW, 400, onHudDraw);
    g_ps.add(K2PS_FRAME_BEGIN, 300, onFrameBegin);
    g_ps.connect();
    g_hudc.init(api, "queue-hud");
    g_hudc.onConnect = onHudConnect;
    g_hudc.add(K2HUD_FRAME, 300, onHudFrame);
    g_hudc.add(K2HUD_BUILD, 300, onHudBuild);
    g_hudc.connect();
}

// K2HUD_FRAME callback: dispatches on every PopulateMenus via hud-host, captures hud & combat status without hook 0x74C168.
void KPATCH_CALL onHudFrame(const K2HudContext* ctx, void*) {
    if (ctx && ctx->hud) { QueueHudCapture(ctx->hud); softBannerTick(static_cast<uintptr_t>(static_cast<uint32_t>(ctx->hud))); }
}

// H8 0x0074C168 / K2HUD_FRAME handler: retains export for tests/smoke harness.
// Hot path: one safe read + two stores (+ a rare hud-host connect retry); logs only when the HUD pointer changes.
void __cdecl QueueHudCapture(int hud) {
    saferead::beginScope();
    g_ps.poll();
    g_hudc.poll();
    uint32_t flags = 0;
    const bool flagsOk = rd(SiteCap, static_cast<uintptr_t>(static_cast<uint32_t>(hud)), HudCombatFlags, &flags);
    static int lastReadCombat = 0;   // QH25: last READABLE combat bit (an unreadable frame forces g_combat=1, which is no evidence)
    g_combat = flagsOk ? static_cast<int>((flags >> 1) & 1u) : 1;   // unreadable = treat as combat (hidden)
    g_combatUnreadable = !flagsOk;
    g_capTick = GetTickCount();
    if (flagsOk) {
        if (g_combat) g_combatEndTick = 0;                                           // a new combat cancels a running tail
        else if (lastReadCombat) g_combatEndTick = g_capTick ? g_capTick : 1u;       // readable 1->0 edge: the tail starts
        lastReadCombat = g_combat;
    }
    if (g_hud != static_cast<uintptr_t>(static_cast<uint32_t>(hud))) {
        g_hud = static_cast<uintptr_t>(static_cast<uint32_t>(hud));
        QLOG(chCap, "hud=%08x flags=%08x combat=%d", (unsigned)hud, (unsigned)flags, g_combat);
    }
}

// RL10: the engine queue could not be read at click time (server creature unresolved: load / area change). The decision stays decideAction's (empty buffer = dispatch_now); leave a default-level record.
static void noteUnreadableClick(int k, uint32_t cmd, qh::ActionDecision dec) {
    QLOGV(chMain, KPLOG_WARN, "click with unreadable engine queue m=%d cmd=%u dec=%s", k, (unsigned)cmd, dec == qh::DECIDE_DISPATCH_NOW ? "dispatch_now" : dec == qh::DECIDE_BUFFER ? "buffer" : "drop_full");
}

// H9: PerformUseItem detour hook. Consumes and buffers out-of-combat item actions of party members only when queue busy.
int __cdecl QueueHudPerformUseItem(int clientCreature, int ebp) {
    if (g_dispatching || !g_cfg.enabled) return 0;   // re-entry from our own dispatch logs nothing here
    const uint32_t cmd = ++g_cmdSeq;
    const uintptr_t crPtr = static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature));
    if (g_combat != 0) {
        g_combatClickTick = GetTickCount();   // RL6
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Item, false, 0, 0, "pass_combat", false, 0, 0, true);
        return 0;
    }
    saferead::beginScope();

    uint32_t itemId = 0;
    int packedByte = 0;
    uint32_t targetId = 0;
    uintptr_t posPtr = 0;
    float posVal[3] = {};

    if (!rd(SiteQueue + 0x20, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 8, &itemId) ||
        !rd(SiteQueue + 0x21, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0xC, &packedByte) ||
        !rd(SiteQueue + 0x22, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &targetId)) {
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Item, false, 0, 0, "pass_unreadable", false, 0, 0, false);
        return 0;
    }
    if (rd(SiteQueue + 0x23, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x14, &posPtr) && posPtr) {
        rd(SiteQueue + 0x24, posPtr, 0, &posVal[0]);
        rd(SiteQueue + 0x25, posPtr, 4, &posVal[1]);
        rd(SiteQueue + 0x26, posPtr, 8, &posVal[2]);
    }

    int memberK = -1;
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (tbl) {
        for (int k = 0; k < 3; ++k) {
            uintptr_t m = party::member(tbl, k);
            if (m && (m == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)) ||
                      party::serverCreature(m) == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)))) {
                memberK = k;
                break;
            }
        }
    }
    if (!qh::isPartyCaster(memberK)) {
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Item, true, itemId, targetId, "pass_nonparty", true, 0, 0, false);
        return 0;
    }
    if (tbl) {
        bool hookSkip[3] = {false, false, false};
        syncPartyIdentity(tbl, GetTickCount(), hookSkip);
    }
    const bool fb = false;

    bool haveCur = false;
    int gateQ = -1;
    const int engineQ = queueCountOf(memberK, &haveCur, &gateQ);
    const int engqLog = engineQ < 0 ? -1 : engineQ;
    const int bufBefore = g_itemQueue[memberK].count;
    const qh::ActionDecision dec = qh::decideAction(g_itemQueue[memberK], g_combat != 0, gateQ > 0 ? gateQ : 0, haveCur);
    if (gateQ < 0) noteUnreadableClick(memberK, cmd, dec);

    if (dec == qh::DECIDE_DISPATCH_NOW) {
        g_itemQueue[memberK].userCancelled = false;
        g_itemQueue[memberK].active = true;
        g_itemQueue[memberK].activeAction = qh::makeItemAction(
            crPtr, itemId, packedByte, targetId, posVal, GetTickCount(), cmd);
        g_itemQueue[memberK].markDispatched(GetTickCount());
        g_lastOurs[memberK].tag = "hook"; g_lastOurs[memberK].tick = GetTickCount();
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Item, true, itemId, targetId, "dispatch_now", fb, engqLog, bufBefore, false);
        return 0;
    }

    if (dec == qh::DECIDE_BUFFER) {
        qh::QueuedAction a = qh::makeItemAction(
            crPtr, itemId, packedByte, targetId, posVal, GetTickCount(), cmd);
        g_itemQueue[memberK].push(a);
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Item, true, itemId, targetId, "buffer", fb, engqLog, bufBefore, false);
        QLOG(chMain, "buffered item use for member %d (item=%08x, target=%08x, count=%d) cmd=%u",
             memberK, itemId, targetId, g_itemQueue[memberK].count, (unsigned)cmd);
        return 1;
    }

    if (dec == qh::DECIDE_DROP_FULL) {
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Item, true, itemId, targetId, "drop_full", fb, engqLog, bufBefore, false);
        const bool snd = guikit::playHoverSound();   // RL16: audible drop_full cue
        QLOGV(chMain, KPLOG_WARN, "item buffer full for member %d, dropped item %08x cmd=%u sound=%d", memberK, itemId, (unsigned)cmd, snd ? 1 : 0);
        return 1;
    }

    traceCmd(cmd, memberK, crPtr, qh::ActionKind::Item, true, itemId, targetId, "unknown", fb, engqLog, bufBefore, false);

    return 0;
}

// H10: PerformCastSpell detour hook (gameaddr::PerformCastSpell + 3; address in hooks.toml). Consumes and buffers out-of-combat spell actions of party members only when queue busy.
int __cdecl QueueHudPerformCastSpell(int clientCreature, int ebp) {
    if (g_dispatching || !g_cfg.enabled) return 0;   // re-entry from our own dispatch logs nothing here
    const uint32_t cmd = ++g_cmdSeq;
    const uintptr_t crPtr = static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature));
    if (g_combat != 0) {
        g_combatClickTick = GetTickCount();   // RL6
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, false, 0, 0, "pass_combat", false, 0, 0, true);
        return 0;
    }
    saferead::beginScope();

    uint32_t spellId = 0;
    int arg2 = 0;
    int multispell = 0;
    int metamagic = 0;
    int domain = 0;
    uintptr_t posAddr = 0;
    int flag = 0;
    float posVal[3] = {};

    if (!rd(SiteQueue + 0x30, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 8, &spellId) ||
        !rd(SiteQueue + 0x31, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0xC, &arg2) ||
        !rd(SiteQueue + 0x33, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x14, &multispell) ||
        !rd(SiteQueue + 0x34, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x18, &metamagic) ||
        !rd(SiteQueue + 0x35, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x1C, &domain)) {
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, false, 0, 0, "pass_unreadable", false, 0, 0, false);
        return 0;
    }
    const uint32_t tgt = static_cast<uint32_t>(arg2);
    rd(SiteQueue + 0x32, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &posAddr);   // 3rd stack arg = caller's float[3] (stack address, dead after return)
    rd(SiteQueue + 0x36, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x20, &flag);      // 7th stack arg = int flag, NOT a pointer
    if (posAddr) {
        rd(SiteQueue + 0x37, posAddr, 0, &posVal[0]);
        rd(SiteQueue + 0x38, posAddr, 4, &posVal[1]);
        rd(SiteQueue + 0x39, posAddr, 8, &posVal[2]);
    }

    int memberK = -1;
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (tbl) {
        for (int k = 0; k < 3; ++k) {
            uintptr_t m = party::member(tbl, k);
            if (m && (m == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)) ||
                      party::serverCreature(m) == static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature)))) {
                memberK = k;
                break;
            }
        }
    }
    if (!qh::isPartyCaster(memberK)) {
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, true, spellId, tgt, "pass_nonparty", true, 0, 0, false);
        return 0;
    }
    if (tbl) {
        bool hookSkip[3] = {false, false, false};
        syncPartyIdentity(tbl, GetTickCount(), hookSkip);
    }
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtSpellArgs(line, sizeof line, false, cmd, memberK, spellId, arg2, multispell, metamagic, domain,
                         posVal, flag, static_cast<uint32_t>(posAddr));
        QTRACE("%s", line);
    }
    const bool fb = false;

    bool haveCur = false;
    int gateQ = -1;
    const int engineQ = queueCountOf(memberK, &haveCur, &gateQ);
    const int engqLog = engineQ < 0 ? -1 : engineQ;
    const int bufBefore = g_itemQueue[memberK].count;
    const qh::ActionDecision dec = qh::decideAction(g_itemQueue[memberK], g_combat != 0, gateQ > 0 ? gateQ : 0, haveCur);
    if (gateQ < 0) noteUnreadableClick(memberK, cmd, dec);

    if (dec == qh::DECIDE_DISPATCH_NOW) {
        g_itemQueue[memberK].userCancelled = false;
        g_itemQueue[memberK].active = true;
        g_itemQueue[memberK].activeAction = qh::makeSpellAction(
            crPtr, spellId, arg2, multispell, metamagic, domain, posVal, flag, GetTickCount(), cmd);
        g_itemQueue[memberK].markDispatched(GetTickCount());
        g_lastOurs[memberK].tag = "hook"; g_lastOurs[memberK].tick = GetTickCount();
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, tgt, "dispatch_now", fb, engqLog, bufBefore, false);
        return 0;
    }

    if (dec == qh::DECIDE_BUFFER) {
        qh::QueuedAction a = qh::makeSpellAction(
            crPtr, spellId, arg2, multispell, metamagic, domain, posVal, flag, GetTickCount(), cmd);
        g_itemQueue[memberK].push(a);
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, tgt, "buffer", fb, engqLog, bufBefore, false);
        QLOG(chMain, "buffered spell use for member %d (spell=%08x, target=%08x, count=%d) cmd=%u",
             memberK, spellId, tgt, g_itemQueue[memberK].count, (unsigned)cmd);
        return 1;
    }

    if (dec == qh::DECIDE_DROP_FULL) {
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, tgt, "drop_full", fb, engqLog, bufBefore, false);
        const bool snd = guikit::playHoverSound();   // RL16: audible drop_full cue
        QLOGV(chMain, KPLOG_WARN, "action buffer full for member %d, dropped spell %08x cmd=%u sound=%d", memberK, spellId, (unsigned)cmd, snd ? 1 : 0);
        return 1;
    }

    traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, tgt, "unknown", fb, engqLog, bufBefore, false);

    return 0;
}

} // extern "C"
