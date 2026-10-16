// Queue HUD 0.4.1-dev (CS10 strip slot, CS11 kit art, slim strip) (Steam Aspyr build 6A522E71...). A GREEN copy of the vanilla combat action-queue panel (up to 4 icons), drawn
// OUT of combat so a command given while paused (a use-item / cast-spell that waits for unpause) is visible.
// Research: patch_manager_mods/30_queue_hud_evidence/RE.md (0x74DC80 panel filler, 0x74E0B0 icon resolver); logic: QueueHudLogic.h.
//
//   hook H8  0x0074C168  HUD Update, just before the combat-flag test: captures the HUD pointer + combat bit (QueueHudCapture)
//   phase    K2PS_HUD_DRAW prio 400 (portrait-slots): reads party member 0's SERVER creature queue, resolves icons with the
//            engine's own 0x74E0B0 (cached), and draws bg + border + icon over the hidden vanilla slots (hud+0x5F2C+i*0x148)
//   0.2.0    companion strips: for party members 1-2 a row of small green queue icons left of their health bar
//            (strip_side left | above | below the card; HUD slot table via portrait-slots; slot creature matched against the party member's client OR server pointer; per-creature round)
//   0.3.0    kit look: the slots are the kit_frame image (icon inset 15%), flat rects only as the fallback when a kit texture is missing;
//            a Cancel control (kit_cancel / kit_cancel_hi, a HUD label built in hud-host's K2HUD_BUILD phase, k2.hud@1) right of the leader's
//            QUEUE0 and left of each companion strip; LMB calls CServerExoApp::ClearAllActions (0x51CB10, doc 30 R7) for that member.
//            Out of combat only (the vanilla red Cancel Combat bar is untouched). No new hook: BUILD + FRAME_BEGIN are registry phases.
//   CS11/12  the kit art (frame, Cancel X) is the shared HUD kit _shared_src/HudKit.h (kit_* textures, also used by companion-selfheal);
//            the left-side strip icon auto size is the selfheal button side (QueueHudLogic.h stripIconSize), strip_icon_px still overrides.
//   0.4.1    strip hidden for Down (HP<=0) companions; their queue journal poll is skipped too.
// Every pointer deref goes through saferead (site ids 0x7100+, strips 0x7150+, cancel 0x7160+). The only engine write is the Cancel call.
// Without portrait-slots (or an unknown exe) the mod is inert.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <utility>

#include "KpLog.h"
#include "SafeRead.h"
#include "ActQueue.h"
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "HudHost.h"
#include "SelfHealUi.h"   // shui::stripReserve: the fallback selfheal footprint when hud-host strip slots are unavailable
#include "EmbedTex.h"
#include "HudKit.h"
#include "GameAddr.h"
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
constexpr unsigned SiteCap = 0x7100, SiteHud = 0x7101, SiteQueue = 0x7110, SiteRound = 0x7120, SitePause = 0x7130, SiteRules = 0x7140, SiteStrip = 0x7150, SiteCancel = 0x7160;

// Steam Aspyr addresses / offsets (RE.md; the manifest supports the Steam exe only).
constexpr uintptr_t AddrIconResolver = 0x0074E0B0;   // thiscall(hud ignored) 6 stack args, RET 0x18
constexpr uintptr_t AddrRulesGlobal = 0x00A1B4D0;    // [rules+0x104] -> spell table {count@0, data@4}, row stride 0x1A8
constexpr int HudCombatFlags = 0xA904;               // bit 1 = combat mode (set by 0x74E910 just before the hook)
constexpr int HudSlotBase = 0x5F2C, HudSlotStride = 0x148;
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
using PerformCastSpellFn = void(__thiscall*)(void* creature, uint32_t spellId, int arg2, uint32_t targetId, int multispell, int metamagic, int domain, const float* pos);
constexpr uintptr_t AddrPerformCastSpell = gameaddr::PerformCastSpell;

// ---- capture state (written by the hook, read by the draw callback; same game thread) ----
uintptr_t g_hud = 0;
int g_combat = 0;
bool g_combatUnreadable = false;   // last capture could not read the HUD flags (g_combat is then forced to 1)
bool g_softCombat = false;         // softBannerTick verdict this frame: in combat but only own-party targets (false when not in combat)
bool g_bannerHidden = false;       // we cleared the visible bit of >= 1 banner/red-bar control and have not restored it yet
unsigned g_bannerMask = 0;         // which kBannerCtl[] entries WE cleared (restore sets only these)
uintptr_t g_bannerHud = 0;         // hud the mask belongs to
DWORD g_capTick = 0;

template <typename T> bool rd(unsigned site, uintptr_t base, int off, T* out) { return saferead::readAt(site, base, off, out); }

bool g_logOn = true;
#define QLOG(ch, ...) do { if (g_logOn) KP_LOG(ch, __VA_ARGS__); } while (0)
#define QLOGV(ch, lvl, ...) do { if (g_logOn) KP_LOGV(ch, lvl, __VA_ARGS__); } while (0)
// Queue trace lines (CMD#/WAIT/RESOLVE/DROP/EDGE/QJ): INFO, gated by log=1 and trace_queue=1.
#define QTRACE(...) do { if (g_logOn && g_cfg.traceQueue) KP_LOG(chTrace, __VA_ARGS__); } while (0)

uint32_t g_cmdSeq = 0;                                   // per-mod command id counter (CMD#<n>)
struct OursStamp { const char* tag; DWORD tick; };
OursStamp g_lastOurs[3] = {};                            // last action of ours per member: tag (hook|cancel|...) + GetTickCount()

void onConnect(bool ok, const char* why) { QLOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "portraitslots: %s (%s)", ok ? "connected" : "NOT connected", why ? why : ""); }

// ---- INI: queue_hud.ini in the game dir (CWD); missing file = defaults ----
void loadIni() {
    FILE* f = fopen(".\\queue_hud.ini", "rb");
    if (!f) return;
    char buf[4096];
    size_t n = fread(buf, 1, sizeof buf - 1, f);
    fclose(f);
    buf[n] = 0;
    qh::parseIni(g_cfg, buf);
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
    uint32_t fakeAction[16] = {};
    uintptr_t actPtr = action;
    if (!actPtr && (e.type == qh::T_USE_ITEM || e.type == qh::T_CAST_SPELL)) {
        fakeAction[0] = static_cast<uint32_t>(e.type); // 0x2E or 0x0F
        fakeAction[14] = e.param0;                     // +0x38 = 14*4
        actPtr = reinterpret_cast<uintptr_t>(fakeAction);
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
    if (tbl) {
        for (int k = 0; k < 3; ++k) {
            uintptr_t m = party::member(tbl, k);
            if (m && (m == cr || party::serverCreature(m) == cr)) {
                memberK = k;
                break;
            }
        }
    }
    s.nBuffered = 0;
    if (memberK >= 0 && memberK < 3) {
        const qh::ActionBuffer& ib = g_itemQueue[memberK];
        // The active buffered action is a slot source too: a quickbar press wipes the engine queue, so without it the panel empties until our restore.
        // Appended after the engine entries (buildSlots gives the engine's cur node to the first qualifying entry); not counted in nBuffered.
        if (qh::activeNeedsSlot(ib.active, engineHoldsAction(s)) && s.ne < qh::kMaxEntries) {
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

// ---- kit textures (shared HUD kit: _shared_src/HudKit.h, embedded TGAs kit_frame / kit_frame_hi / kit_cancel / kit_cancel_hi) ----
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
void drawSlot(int x, int y, int w, int h, const qh::Style& st, void* img) {
    static const float white[3] = { 1.0f, 1.0f, 1.0f };
    void* frame = g_cfg.kit ? tex::image(tex::Frame) : nullptr;
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
};
CW g_cw[3];
struct CWant { bool show = false; qh::Rect r{ 0, 0, 0, 0 }; };
CWant g_want[3];             // set by the draw callback each frame, applied at its end
guikit::ClickEdge g_lmb;
struct CPending { bool on = false; int n = 0; };
CPending g_cpend[3];
bool g_cancelMissLogged = false;

// VIS log (identical format in 4 mods): `VIS <what> shown=%d why=%s rect=[x,y,w,h]`, INFO on chMain, change-gated by the caller, cap 64 per process.
void visLog(const char* what, bool shown, const char* why, const qh::Rect& r) {
    static int n = 0;
    if (!g_logOn || n > 64) return;
    if (n == 64) { ++n; KP_LOG(chMain, "VIS cap reached"); return; }
    ++n;
    KP_LOG(chMain, "VIS %s shown=%d why=%s rect=[%d,%d,%d,%d]", what, shown ? 1 : 0, why ? why : "?", r.x, r.y, r.w, r.h);
}
void cwShow(CW& w, bool on, const char* why = "?") {   // clickable only while visible (a shown control swallows the click; hidden = click-through)
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
        if (!g_cfg.cancel || !d.show || d.r.w <= 0) { cwShow(w, false, !g_cfg.cancel ? "cancel-off" : "no-queue"); continue; }
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
    for (CWant& d : g_want) d = CWant();
    for (CPending& p : g_cpend) p = CPending();
    g_lmb = guikit::ClickEdge();
    tex::ensure();
    ProjGuard pg;
    int n = 0, why[3] = {};
    for (int k = 0; k < 3; ++k) {
        const char* tmpl = (k == 0) ? "LBL_MOULDING3" : "LBL_QUEUE1";
        g_cw[k].c = makeCancelLabel(panel, tmpl, &why[k]);
        if (g_cw[k].c) ++n;
    }
    KP_LOGV(chCancel, n == 3 ? KPLOG_INFO : KPLOG_WARN, "BUILD panel=%08x cancel labels n=%d/3 why=%d,%d,%d", (unsigned)panel, n, why[0], why[1], why[2]);
    visLog("build", n == 3, n == 3 ? "ok" : "labels-missing", qh::Rect{ 0, 0, 0, 0 });
}

void onHudConnect(bool ok, const char* why) { QLOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "hud-host: %s (%s)", ok ? "connected" : "NOT connected (no Cancel controls)", why ? why : ""); }

// Edge log: shown / hidden transitions (never per frame).
struct EdgeState { bool shown = false; uint32_t sig = 0; const char* reason = ""; bool primed = false; };
EdgeState g_edge;
// Set at the top of drawAll: the capture age and menu flag that the hidden edge line reports.
DWORD g_hideAge = 0;
bool g_hideMenu = false;

void noteHidden(const char* reason) {
    if (g_edge.primed && !g_edge.shown && g_edge.reason == reason) return;
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
    if (qh::cancelVisible(g_cfg.enabled, g_cfg.cancel, g_combat != 0 && !g_softCombat, ctx->inMenu != 0, age, nd)) {
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
        for (int i = 0; i < nd; ++i) drawSlot(d[i].x, d[i].y, d[i].w, d[i].h, d[i].st, guikit::imageFor(d[i].rr));
    }
}

// QL10/QL11 detach (handoff_detach_held): round nodes held OUT of the engine's list (data pointers only; the nodes stay alive, nobody frees them).
struct Detached { uint32_t ptr[qh::kMaxRound] = {}; qh::RNode rn[qh::kMaxRound]; int n = 0; DWORD at = 0; };   // rn: copies for the icon draw (QL11)
Detached g_detached[3];
constexpr DWORD kDetachMaxMs = 600000;

// QL11: the vanilla combat queue panel cannot show a round node that handoff_detach_held holds out of the list. Draw it as one more (dimmed, waiting)
// slot on the next unused vanilla slot control, with the icon resolved from the node exactly like a round-list slot (the queue's 0x3F entry + the node).
void drawDetachedSlots(uintptr_t hud, const Snapshot& s, int nVanilla, DWORD now) {
    const Detached& d = g_detached[0];
    static uint32_t lastSig = 0;
    if (d.n == 0 || !s.round) { lastSig = 0; return; }
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
        drawSlot(x, y, w, h, qh::slotStyle(g_cfg, idx, true), guikit::imageFor(rr));
        sig = sig * 131u + d.rn[i].ptr + static_cast<uint32_t>(idx);
        ++idx; ++drawn;
    }
    if (drawn > 0 && sig != lastSig) { lastSig = sig; QLOG(chDraw, "detached icons drawn=%d first_slot=%d", drawn, nVanilla); }
}

// The leader's panel over the hidden vanilla slots (unchanged from 0.1.0). The caller already checked enabled / combat / menu / stale / hud.
void drawLeader(uintptr_t hud, DWORD now, DWORD age, const K2PsContext* ctx) {
    Snapshot s;
    if (!gather(s)) { noteHidden("no creature"); return; }
    Held& held = g_heldLeader;
    if (held.creature != s.creature) { held.nd = 0; held.tick = 0; held.creature = s.creature; }   // a Tab switch never shows the old queue
    qh::Slot slots[qh::kMaxSlots];
    const int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
    const bool hardCombat = g_combat != 0 && !g_softCombat;   // soft combat (ally-only targets) draws the strip like out of combat
    if (!qh::shouldShow(g_cfg.enabled, hardCombat, ctx->inMenu != 0, age, n)) {
        if (n == 0 && held.nd > 0 && qh::panelHold(held.tick, now, ctx->inMenu != 0, hardCombat)) { paintLeader(hud, held.d, held.nd, age, ctx); return; }
        if (hardCombat && ctx->inMenu == 0 && age < 250u) drawDetachedSlots(hud, s, n, now);
        noteHidden(g_combat != 0 ? "combat" : "empty queue");
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
void stripsHidden() {
    const qh::Rect none{ 0, 0, 0, 0 };
    for (int k = 1; k <= 2; ++k) stripNote(k, false, 0, "", none, 0);
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

struct StripDraw { qh::Rect r[qh::kMaxSlots]; qh::Style st[qh::kMaxSlots]; char rr[qh::kMaxSlots][17]; int n = 0; bool have = false; };
// Last good strip per party index k (1-2): repainted for kPanelHoldMs on an empty queue, with its edge signature and type text so the log edge does not flip.
struct HeldStrip { StripDraw d; DWORD tick = 0; uintptr_t creature = 0; uint32_t sig = 0; char types[48] = {}; };
HeldStrip g_heldStrip[3];

void drawStrips(uintptr_t hud, DWORD now, bool inMenu) {
    const K2AddrTable* T = k2addr::get();
    if (!T || !T->Slot_VitRect || !T->Slot_CardRect) return;   // unknown exe: no geometry, nothing to anchor to
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
        if (!ok || vr[2] < 1 || vr[2] > 400 || vr[3] < 1 || vr[3] > 400 || cd[2] < 1 || cd[2] > 2000 || cd[3] < 1 || cd[3] > 2000) continue;
        Snapshot s;
        if (!gatherFor(srv[k], s)) continue;
        qh::Slot slots[qh::kMaxSlots];
        const int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
        HeldStrip& hs = g_heldStrip[k];
        if (hs.creature != srv[k]) { hs.d = StripDraw(); hs.tick = 0; hs.creature = srv[k]; }   // a Tab switch never shows the old strip
        if (n == 0 && hs.d.have && qh::panelHold(hs.tick, now, inMenu, g_combat != 0 && !g_cfg.showInCombat)) {
            d[k] = hs.d;   // hold repaint: Cancel below and the draw loop use the held rects; hs.tick is not refreshed
            memcpy(types[k], hs.types, sizeof types[k]);
            sigs[k] = hs.sig;
            continue;
        }
        const int cap = g_cfg.stripMax < qh::kMaxSlots ? g_cfg.stripMax : qh::kMaxSlots;
        StripDraw& sd = d[k];
        uint32_t sig = static_cast<uint32_t>(n) * 31u + (s.waiting ? 7u : 0u);
        int len = 0;
        for (int j = 0; j < n && sd.n < cap; ++j) {
            const qh::Slot& sl = slots[j];
            uintptr_t node = 0;
            if (sl.node == qh::NODE_CUR) node = s.cur.ptr;
            else if (sl.node >= 0 && sl.node < s.nrn) node = s.rn[sl.node].ptr;
            char rr[17] = {};
            if (!resolveIcon(hud, s.round, s.e[sl.entry], s.eData[sl.entry], node, sl.key, now, rr)) continue;
            memcpy(sd.rr[sd.n], rr, 17);
            sd.st[sd.n] = qh::slotStyle(g_cfg, sd.n, s.waiting);
            ++sd.n;
            sig = sig * 131u + static_cast<uint32_t>(s.e[sl.entry].type) + (static_cast<uint32_t>(sl.node + 3) << 8);
            if (len < 40) len += snprintf(types[k] + len, sizeof types[k] - len, "%s0x%02X/%d", len ? "," : "", s.e[sl.entry].type, sl.node);
        }
        if (len > 0 && len < static_cast<int>(sizeof types[k])) types[k][len] = 0;
        if (sd.n == 0) continue;
        const qh::Rect card{ cd[0], cd[1], cd[2], cd[3] }, vit{ vr[0], vr[1], vr[2], vr[3] };
        // Left side: the selfheal footprint comes from the hud-host strip slots (selfheal registers order 0, this row is order 1); without
        // hud-host / selfheal / StripSlot it is the same value computed locally, so the layout is identical either way.
        int reserve = shui::stripReserve(shui::Rect{ vit.x, vit.y, vit.w, vit.h });
        if (g_cfg.stripSide == qh::kSideLeft) {
            const int rowW = qh::stripRowWidth(vit, sd.n, g_cfg), rowH = qh::stripIconSize(vit, g_cfg);
            K2HudRect sr{};
            if (g_hudc.stripSlot("queue-hud", 1, K2HudRect{ vit.x, vit.y, vit.w, vit.h }, rowW, rowH, &sr)) reserve = vit.x - sr.x - rowW;
        }
        sd.n = qh::stripRects(card, vit, sd.n, g_cfg, sd.r, screenW, reserve);
        if (sd.n == 0) continue;
        sd.have = true;
        sigs[k] = sig * 131u + static_cast<uint32_t>(sd.r[0].x) * 7u + static_cast<uint32_t>(sd.r[0].y) * 13u + static_cast<uint32_t>(sd.r[0].w);
        hs.d = sd; hs.tick = now; hs.sig = sigs[k]; memcpy(hs.types, types[k], sizeof hs.types);   // live draw: refresh the hold
    }
    bool any = false;
    for (int k = 1; k <= 2; ++k) {
        if (d[k].have) { any = true; stripNote(k, true, d[k].n, types[k], d[k].r[0], sigs[k]); }
        else stripNote(k, false, 0, "", qh::Rect{ 0, 0, 0, 0 }, 0);
    }
    if (!any) return;
    if (g_cfg.cancel && (g_combat == 0 || g_softCombat))   // Cancel stays hidden in real combat; soft combat (ally-only targets) shows it like out of combat
        for (int k = 1; k <= 2; ++k)
            if (d[k].have) { g_want[k].r = qh::stripCancelRect(d[k].r, d[k].n, g_cfg.stripSide, screenW); g_want[k].show = g_want[k].r.w > 0; }
    gui2d::CapGuard capGuard;
    for (int k = 1; k <= 2; ++k)
        for (int j = 0; d[k].have && j < d[k].n; ++j) drawSlot(d[k].r[j].x, d[k].r[j].y, d[k].r[j].w, d[k].r[j].h, d[k].st[j], guikit::imageFor(d[k].rr[j]));
}

void drawAll(const K2PsContext* ctx) {
    saferead::beginScope();
    const DWORD now = GetTickCount();
    const DWORD age = now - g_capTick;
    g_hideAge = age; g_hideMenu = ctx->inMenu != 0;
    const uintptr_t hud = g_hud;
    uint32_t probe = 0;
    if (!hud || !rd(SiteHud, hud, HudSlotBase, &probe)) { if (hud) noteHidden("hud unreadable"); if (g_cfg.companions) stripsHidden(); return; }
    if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, 1, g_cfg.showInCombat)) {
        noteHidden(g_combat ? "combat" : ctx->inMenu ? "menu" : "stale capture");
        if (g_cfg.companions) stripsHidden();
        return;
    }
    drawLeader(hud, now, age, ctx);
    if (g_cfg.companions) drawStrips(hud, now, ctx->inMenu != 0);
}

// HUD_DRAW prio 400: the wanted Cancel rects are collected by drawLeader / drawStrips, then applied (every other path hides them).
void KPATCH_CALL onHudDraw(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    for (CWant& d : g_want) d = CWant();
    drawAll(ctx);
    cwApply();
}

// ---- Cancel click (FRAME_BEGIN: the UI tick, outside the draw batch) ----
// Queue entries of party member k (0 leader, 1-2 companions), -1 when the creature is unreadable.
// *haveCur (optional) gets the round-node flag from the same snapshot. *gateNe (optional) gets the dispatch-gate count (-1 unreadable);
// the return value stays the raw count (logs).
int queueCountOf(int k, bool* haveCur = nullptr, int* gateNe = nullptr) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    Snapshot s;
    const bool ok = gatherFor(m ? party::serverCreature(m) : 0, s);
    if (haveCur) *haveCur = ok && s.haveCur;
    if (gateNe) *gateNe = ok ? s.gateNe : -1;
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
    if (bad) { KP_LOGV(chCancel, KPLOG_WARN, "CANCEL refused member=%d (%s)", k, bad); return; }
    const int n = queueCountOf(k);
    QLOG(chCancel, "CANCEL member=%d n=%d route=ServerClearAll", k, n);
    if (g_logOn && g_cfg.traceQueue) {
        uint32_t ids[qh::kMaxActionQueue + 1];
        const int ni = qh::bufferedCmdIds(g_itemQueue[k], ids, qh::kMaxActionQueue + 1);
        for (int i = 0; i < ni; ++i) {
            char line[200];
            qh::fmtDrop(line, sizeof line, ids[i], k, qh::DR_CANCEL, n);
            QTRACE("%s", line);
        }
    }
    g_lastOurs[k].tag = "cancel"; g_lastOurs[k].tick = GetTickCount();
    g_itemQueue[k].cancel();
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
    g_cpend[k].on = true; g_cpend[k].n = n;
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
    uint32_t ids[qh::kMaxActionQueue + 1];
    const int ni = qh::bufferedCmdIds(g_itemQueue[k], ids, qh::kMaxActionQueue + 1);
    for (int i = 0; i < ni; ++i) {
        char line[200];
        qh::fmtDrop(line, sizeof line, ids[i], k, why, ni);
        QTRACE("%s", line);
    }
}

bool bufBusy(const qh::ActionBuffer& b) { return b.active || b.count > 0; }

// The creature a busy buffer belongs to: the active action's, else the head of the queue.
static uintptr_t bufOwner(const qh::ActionBuffer& b) { return b.active ? b.activeAction.creature : b.queue[0].creature; }

// Member slot (0..2) that holds creature cr (client or server creature), or -1. Same match rule as the hooks' slot lookup.
static int slotOfCreature(uintptr_t tbl, uintptr_t cr) {
    if (!cr) return -1;
    for (int k = 0; k < 3; ++k) {
        const uintptr_t m = party::member(tbl, k);
        if (m && (m == cr || party::serverCreature(m) == cr)) return k;
    }
    return -1;
}

// Clears the active action and every per-action eaten flag (queued commands stay in the buffer).
static void endAction(qh::ActionBuffer& b) {
    b.active = false;
    b.seenInQueue = false;
    b.eaten = false;
    b.eatenReInjects = 0;
    b.eatenUnpausedMs = 0;
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
static void resolveActive(int k, qh::ActionBuffer& buf, DWORD now) {
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtResolve(line, sizeof line, buf.activeAction.cmdId, k, buf.seenInQueue, buf.activeAction.reInjectCount,
                       now - buf.activeAction.queueTime);
        QTRACE("%s", line);
    }
    g_lastOurs[k].tag = "resolve"; g_lastOurs[k].tick = GetTickCount();
    endAction(buf);
}

void spellWatchArm(int k, uint32_t cmd, DWORD now);   // SPELL_OUT watch (defined with the HANDOFF_OUT watch below)

// Issue one queued command to the engine (our own call: the hooks pass it through via g_dispatching). k = party member (-1 unknown).
void dispatchAction(const qh::QueuedAction& a, int k) {
    g_dispatching = true;
    if (a.kind == qh::ActionKind::Spell) {
        const float* pp = qh::replayPos(a, g_cfg.spellReplayPos);
        if (g_logOn && g_cfg.traceQueue) {
            char line[200];
            qh::fmtSpellArgs(line, sizeof line, true, a.cmdId, k, a.spellId, a.arg2, a.targetId, a.multispell, a.metamagic, a.domain,
                             static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pp)), a.hasPos);
            QTRACE("%s", line);
        }
        if (k >= 0) spellWatchArm(k, a.cmdId, GetTickCount());
        reinterpret_cast<PerformCastSpellFn>(AddrPerformCastSpell)(
            reinterpret_cast<void*>(a.creature),
            a.spellId,
            a.arg2,
            a.targetId,
            a.multispell,
            a.metamagic,
            a.domain,
            pp);
    } else {
        reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)(
            reinterpret_cast<void*>(a.creature),
            a.clientItem,
            a.packedByte,
            a.targetId,
            a.pos);
    }
    g_dispatching = false;
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

// ---- Combat hand-off: at combat start the queued commands (active + buffered) go to the engine in click order, one per frame ----
// headHeld: the head was still held by the engine at the edge; it waits there until the engine releases it (qh::heldHeadAction).
// sawExec latches exec evidence (execEvidence) from the edge on; heldUnpausedMs / lastTick time a head that stays held.
struct Handoff { bool detachTried = false; bool pending = false; bool headValid = false; bool headHeld = false; bool sawExec = false; DWORD edgeAt = 0; DWORD lastTick = 0;
                 int heldUnpausedMs = 0; qh::QueuedAction head; int total = 0; int sent = 0; };
Handoff g_handoff[3];
// QL10 (handoff_keep_new): round nodes already pending at the first send (the player's new click); roundOrderTick keeps them behind our re-sent commands.
struct RoundOrder { uint32_t foreign[qh::kMaxRound] = {}; int n = 0; DWORD armedAt = 0; };
RoundOrder g_roundOrder[3];
constexpr DWORD kRoundOrderMaxMs = 600000;   // safety net; normally disarmed as soon as no foreign node is pending
// Round-list watch: 15 s after each hand-off dispatch, roundWatchTick logs the member's round / cur / entry state on change only.
DWORD g_roundWatchUntil[3] = {};
DWORD g_lastClickTick = 0;   // last left-button press edge (any frame, trace on or off); a head that vanishes right after one was cancelled, not wiped
struct RoundLast { int subs[qh::kMaxRound]; int n = -1; int cur = -2; int ne = -1; } g_lastRound[3];
bool g_prevCombat[3] = {false, false, false};

// The engine already holds this creature's action: an executing use-item / cast-spell entry, or a round node of subtype 9 or 10
// (same test as the tick's actionInQueue). A bare 0x3F entry is NOT proof: combat start queues ordinary attacks as 0x3F too.
bool engineHoldsAction(const Snapshot& s) {
    for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) {
        const int t = s.e[i].type;
        if (t == qh::T_USE_ITEM || t == qh::T_CAST_SPELL) return true;
    }
    for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i)
        if (s.rn[i].subType == 9 || s.rn[i].subType == 10) return true;
    return s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10);
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

void detachRound(int k, uintptr_t srv, const Snapshot& s, DWORD now, const char* why);   // QL10 detach, defined after the hand-off code
void discardDetached(int k, const char* why);
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
            h.headHeld = buf.active && inEngine && g_cfg.handoffResendHeld;   // engine-held head: wait for the combat-start wipe, then re-send
            h.edgeAt = now;
            h.lastTick = now;
            h.heldUnpausedMs = 0;
            h.sawExec = gEdgeOk && execEvidence(s);
            h.headValid = buf.active && (!inEngine || h.headHeld);
            if (h.headValid) h.head = buf.activeAction;
            if (g_logOn && g_cfg.traceQueue) {
                uint32_t ids[qh::kMaxActionQueue];
                for (int i = 0; i < buf.count; ++i) ids[i] = buf.queue[i].cmdId;
                char line[200];
                qh::fmtHandoff(line, sizeof line, k, wasActive, activeCmd, ids, buf.count);
                QTRACE("%s", line);
            }
            // The engine owns the active command now (or already did); the buffered ones stay queued and leave one per frame below.
            buf.active = false; buf.seenInQueue = false; buf.eaten = false; buf.reInjectPending = false;
            h.total = (h.headValid ? 1 : 0) + buf.count;
            h.sent = 0;
            h.pending = h.total > 0;
            g_lastOurs[k].tag = "handoff"; g_lastOurs[k].tick = now;
        }
    }
    for (int k = 0; k < 3; ++k) {
        Handoff& h = g_handoff[k];
        if (!h.pending) continue;
        qh::ActionBuffer& buf = g_itemQueue[k];
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (!srv) { endHandoff(k, qh::DR_NULL_CREATURE); traceDropAll(k, qh::DR_NULL_CREATURE); buf.clear(); continue; }
        Snapshot s;
        if (!gatherFor(srv, s)) continue;
        if (h.headValid && h.headHeld) {   // exec evidence and the stuck clock run even on frames that cannot send yet
            if (execEvidence(s)) h.sawExec = true;
            const bool held = engineHoldsAction(s);
            if (held && !s.paused) { const DWORD d = now - h.lastTick; h.heldUnpausedMs += d < 1000u ? static_cast<int>(d) : 1000; }
            h.lastTick = now;
        }
        if (g_cfg.handoffDetachHeld && g_cfg.handoffKeepNew && h.headValid && h.headHeld && !h.detachTried && s.paused && s.nrn > 0) {
            h.detachTried = true;   // once per hand-off: the nodes pending at the edge are the player's new click
            detachRound(k, srv, s, now, "held_head");
        }
        if (s.nrn + 1 > qh::kMaxRound) continue;   // the engine's round list is full: retry next frame
        if (h.headValid && h.headHeld) {
            const int ms = static_cast<int>(now - h.edgeAt);
            const bool cancelClick = qh::cancelClickRecent(g_lastClickTick, h.edgeAt, now);
            const qh::HeldHeadAction act = qh::heldHeadAction(engineHoldsAction(s), h.sawExec, s.paused, h.heldUnpausedMs, cancelClick);
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
            // Combat start wipes the engine queue; the first send clears this member's creature so the hand-off is not queued behind the engine's attack entries.
            // QL10: only while paused (nothing pops the round list then); unpaused keeps the clear-first behaviour.
            const bool keepNow = g_cfg.handoffKeepNew && s.paused;
            if (qh::keepNewAtHandoff(g_cfg.handoffClearFirst, keepNow, h.sent)) {
                if (s.nrn > 0) {
                    RoundOrder& ro = g_roundOrder[k];
                    ro = RoundOrder();
                    for (int i = 0; i < s.nrn && i < qh::kMaxRound; ++i) ro.foreign[ro.n++] = s.rn[i].ptr;
                    ro.armedAt = now;
                }
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
            dispatchAction(a, k);
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
void roundOrderTick(DWORD now, const bool* edge) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;
    for (int k = 0; k < 3; ++k) {
        RoundOrder& ro = g_roundOrder[k];
        if (ro.n == 0) continue;
        if (static_cast<int>(now - ro.armedAt) > static_cast<int>(kRoundOrderMaxMs)) { ro = RoundOrder(); continue; }
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        Snapshot s;
        if (!srv || !gatherFor(srv, s)) continue;
        uint32_t ptrs[qh::kMaxRound];
        int nForeignPresent = 0;
        for (int i = 0; i < s.nrn; ++i) {
            ptrs[i] = s.rn[i].ptr;
            for (int j = 0; j < ro.n; ++j) if (ro.foreign[j] == ptrs[i]) ++nForeignPresent;
        }
        if (nForeignPresent == 0) { ro = RoundOrder(); continue; }   // the new click ran or was cancelled: nothing left to order
        int order[qh::kMaxRound];
        const int moved = qh::roundPartition(ptrs, s.nrn, ro.foreign, ro.n, order);
        if (moved == 0 || s.nrn < 2) continue;
        uint32_t running = 0, w = 0, internal = 0;
        bool ok = s.round &&
                  saferead::readAt(SiteCancel + 0x30, srv, actq::CrRunning, &running) && running == 0 &&   // not inside RunActions
                  saferead::readAt(SiteCancel + 0x31, s.round, actq::RdList, &w) && w &&
                  saferead::readAt(SiteCancel + 0x32, w, 0, &internal) && internal;
        bool applied = false;
        if (ok) {
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

void traceDetach(int k, int n, const char* act, bool ok, const char* why) {
    if (!g_logOn || !g_cfg.traceQueue) return;
    char line[160];
    qh::fmtRoundDetach(line, sizeof line, k, n, act, ok, why);
    QTRACE("%s", line);
}

// Take every pending round node of member k out of the list (identity-checked against the snapshot; put back untouched on any mismatch).
void detachRound(int k, uintptr_t srv, const Snapshot& s, DWORD now, const char* why) {
    Detached& d = g_detached[k];
    uint32_t w = 0;
    if (d.n != 0 || s.nrn <= 0 || !roundListHandle(srv, s.round, &w)) { traceDetach(k, s.nrn, "detach", false, "guard"); return; }
    auto pop = reinterpret_cast<uint32_t(__thiscall*)(uintptr_t)>(gameaddr::ActListRemoveHead);
    auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
    uint32_t got[qh::kMaxRound];
    int popped = 0;
    for (; popped < s.nrn; ++popped) { got[popped] = pop(w); if (got[popped] == 0) break; }
    bool ok = popped == s.nrn;
    for (int i = 0; i < s.nrn && ok; ++i) ok = got[i] == s.rn[i].ptr;
    if (ok) { for (int i = 0; i < s.nrn; ++i) { d.ptr[i] = got[i]; d.rn[i] = s.rn[i]; } d.n = s.nrn; d.at = now; }
    else for (int i = 0; i < popped; ++i) if (got[i]) push(w, got[i]);   // put it back exactly as it was
    traceDetach(k, s.nrn, "detach", ok, why);
}

// Forget the detached nodes without re-adding them (the player cancelled, combat ended or restarted). The ~0x88 byte nodes are leaked on purpose: the free path is unverified.
void discardDetached(int k, const char* why) {
    Detached& d = g_detached[k];
    if (d.n == 0) return;
    traceDetach(k, d.n, "discard", true, why);
    d = Detached();
}

// Per combat frame: put the detached nodes back once nothing of ours is pending; drop them when the player cancelled the whole queue.
void detachTick(DWORD now) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;
    for (int k = 0; k < 3; ++k) {
        Detached& d = g_detached[k];
        if (d.n == 0) continue;
        if (static_cast<int>(now - d.at) > static_cast<int>(kDetachMaxMs)) { discardDetached(k, "timeout"); continue; }
        const uintptr_t cli = party::member(tbl, k);
        const uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        Snapshot s;
        if (!srv || !gatherFor(srv, s)) continue;
        bool midFlight = false;
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i)
            if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) midFlight = true;
        int oursNodes = 0;
        for (int i = 0; i < s.nrn; ++i) if (s.rn[i].subType == 9 || s.rn[i].subType == 10) ++oursNodes;
        const bool busy = g_handoff[k].pending || bufBusy(g_itemQueue[k]);
        // A click right before the engine went completely quiet = the player's Cancel: the detached power goes too, and so do the buffered commands.
        const bool quiet = !midFlight && s.nrn == 0 && s.engineNe == 0;
        if (quiet && qh::cancelClickRecent(g_lastClickTick, d.at, now)) {
            discardDetached(k, "cancel_click");
            endHandoff(k, qh::DR_CANCEL); traceDropAll(k, qh::DR_CANCEL); g_itemQueue[k].clear();
            continue;
        }
        const int sinceSend = g_lastOurs[k].tick ? static_cast<int>(now - g_lastOurs[k].tick) : 100000;
        if (!qh::reattachReady(busy, midFlight, oursNodes, sinceSend, s.nrn, d.n)) continue;
        uint32_t w = 0;
        bool ok = roundListHandle(srv, s.round, &w);
        if (ok) {
            auto push = reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ActListAddTail);
            for (int i = 0; i < d.n; ++i) push(w, d.ptr[i]);
        }
        traceDetach(k, d.n, "reattach", ok, ok ? "done" : "guard");
        if (ok) d = Detached();
        else if (static_cast<int>(now - d.at) > 5000) discardDetached(k, "reattach_guard");   // never wedge on an unwritable list
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

// Multi-item buffer ticker and follower wipe guard
void tickItemQueues(DWORD now, bool inMenu) {
    outWatchTick(now);
    const bool combatNow = g_combat != 0 && !g_combatUnreadable;
    bool edgeFlags[3] = {false, false, false};
    // unreadable HUD flags: no edge and g_prevCombat is kept (QH-2a-07), so a gap cannot fake a rising edge
    for (int k = 0; k < 3; ++k) edgeFlags[k] = qh::combatEdgeStep(g_prevCombat[k], !g_combatUnreadable, g_combat != 0);
    if (inMenu || g_combat != 0) {
        if (combatNow) { for (int k = 0; k < 3; ++k) if (edgeFlags[k]) { g_roundOrder[k] = RoundOrder(); discardDetached(k, "new_combat"); }   // a new combat starts clean
                         combatEdgeTrace(now, edgeFlags); handoffTick(now, edgeFlags); roundOrderTick(now, edgeFlags); detachTick(now); roundWatchTick(now); }   // unreadable HUD flags: no hand-off and no drop, the buffers wait
        return;
    }
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
    if (!tbl) return;

    // Identity pre-pass: buffer k must hold the creature that party member k is now. Party rotation moves creatures between slots.
    bool skip[3] = {false, false, false};
    for (int k = 0; k < 3; ++k) {
        qh::ActionBuffer& b = g_itemQueue[k];
        if (!bufBusy(b)) { g_identLostMs[k] = 0; g_identLastTick[k] = 0; continue; }
        const int slot = slotOfCreature(tbl, bufOwner(b));
        if (slot == k) { g_identLostMs[k] = 0; g_identLastTick[k] = 0; continue; }
        if (slot >= 0) {   // the creature moved to another slot: swap the two buffers (and their g_lastOurs)
            std::swap(g_itemQueue[k], g_itemQueue[slot]);
            std::swap(g_lastOurs[k], g_lastOurs[slot]);
            if (g_logOn && g_cfg.traceQueue) {
                char line[64];
                qh::fmtIdentSwap(line, sizeof line, k, slot);
                QTRACE("%s", line);
            }
            g_identLostMs[k] = g_identLostMs[slot] = 0;
            g_identLastTick[k] = g_identLastTick[slot] = 0;
            g_prevRiWhy[k] = g_prevRiWhy[slot] = -1;
            g_prevDsWhy[k] = g_prevDsWhy[slot] = -1;
            // One swap per k per tick. If the buffer now at k still belongs to another slot, it is fixed next tick and must not run against member k's snapshot now.
            skip[k] = bufBusy(g_itemQueue[k]) && slotOfCreature(tbl, bufOwner(g_itemQueue[k])) != k;
            continue;
        }
        // The creature is in no slot: never classify another creature's snapshot.
        skip[k] = true;
        if (!playerPaused()) {
            if (g_identLastTick[k] != 0) { const DWORD d = now - g_identLastTick[k]; g_identLostMs[k] += d < 1000u ? d : 1000u; }
            g_identLastTick[k] = now;
            if (g_identLostMs[k] >= qh::kEatenWaitMaxMs) {
                traceDropAll(k, qh::DR_IDENTITY_LOST);
                g_itemQueue[k].clear();
                g_identLostMs[k] = 0; g_identLastTick[k] = 0; skip[k] = false;
            }
        } else {
            g_identLastTick[k] = 0;
        }
    }

    // Combat ended before a hand-off drained: the held head is dropped; the still-buffered commands run through the sequential dispatch below.
    for (int k = 0; k < 3; ++k)
        if (g_handoff[k].pending) endHandoff(k, qh::DR_COMBAT_CLEAR);

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
        if (skip[k]) continue;

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
        if (!gOk) continue;

        // 1. Engine-busy test, deliberately NOT matched to buf.activeAction: dispatching while ANY spell/item holds the creature would ClearAllActions it (doc 30 QH4).
        //    Known limit (BG23): an unrelated engine-initiated cast can look like our eaten action (AV_PRESENT) until it ends.
        bool actionInQueue = false;
        bool hasFollowLeader = false;
        for (int i = 0; i < s.engineNe; ++i) {
            if (s.e[i].type == qh::T_USE_ITEM || s.e[i].type == qh::T_CAST_SPELL) actionInQueue = true;
            if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader
        }
        if (s.haveCur && (s.cur.subType == 9 || s.cur.subType == 10)) actionInQueue = true; // 9 = cast spell, 10 = use item
        for (int i = 0; i < s.nrn; ++i) {
            if (s.rn[i].subType == 9 || s.rn[i].subType == 10) actionInQueue = true;
        }

        // Stale-cur bookkeeping: a flagless item/spell round node that sits unpaused with aa8/ab8 == 0 goes stale.
        if (s.haveCur && !s.paused && s.aa8 == 0 && s.ab8 == 0) { if (buf.staleCurSince == 0) buf.staleCurSince = now; }
        else buf.staleCurSince = 0;
        const bool creal = qh::curReal(s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.aa8, s.ab8, s.paused, buf.staleCurSince, now);
        int types[qh::kMaxEntries];
        for (int i = 0; i < s.engineNe && i < qh::kMaxEntries; ++i) types[i] = s.e[i].type;
        const qh::EngineView ev = qh::makeEngineView(types, s.engineNe < qh::kMaxEntries ? s.engineNe : qh::kMaxEntries, actionInQueue,
                                                     s.haveCur, s.haveCur ? (int)s.cur.subType : -1, s.nrn, s.aa8, s.ab8, s.paused,
                                                     buf.staleCurSince, now);
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

        // 2. Follower wipe protection & active completion tracking
        const qh::ReInjectWhy riWhy = qh::reInjectWhy(buf, actionInQueue, hasFollowLeader, s.paused, now);
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
            if (riWhy == qh::RI_OK) {
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
                    g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                    QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d) cmd=%u",
                         buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                         k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.activeAction.reInjectCount,
                         (unsigned)buf.activeAction.cmdId);
                    buf.reInjectPending = true;
                    dispatchAction(buf.activeAction, k);
                    buf.activeStartTick = now;
                    buf.reInjectPending = false;
                }
            } else if (s.round != 0) {
                buf.noteView(actionInQueue, now);
                buf.accrueEaten(s.paused, now);
                qh::ActiveVerdict av = qh::activeVerdict(buf, ev, now, g_cfg.pauseEatenWait);
                if (!g_cfg.pauseReinject && (av == qh::AV_EATEN_WAIT || av == qh::AV_EATEN_REINJECT || av == qh::AV_GIVE_UP)) av = qh::AV_UNSEEN;
                switch (av) {
                    case qh::AV_PRESENT:
                        buf.notePresent(ev.executing, actionInQueue, now);
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
                        buf.lastReInjectTick = now;
                        g_lastOurs[k].tag = "reinject"; g_lastOurs[k].tick = GetTickCount();
                        QLOG(chMain, "re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d) cmd=%u why=eaten_paused",
                             buf.activeAction.kind == qh::ActionKind::Spell ? "spell" : "item",
                             k, buf.activeAction.clientItem, buf.activeAction.targetId, buf.eatenReInjects,
                             (unsigned)buf.activeAction.cmdId);
                        dispatchAction(buf.activeAction, k);
                        buf.markDispatched(now);
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
        if (dsWhy == qh::DS_OK && g_cfg.dispatchWaitFlags) {
            const bool flagsBusy = !s.paused && (s.aa8 != 0 || s.ab8 != 0);
            if (!g_flagGate[k].step(flagsBusy, now, &flagCapped)) dsWhy = qh::DS_FLAG_BUSY;
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
                    if (flagCapped) QLOG(chMain, "dispatch hold=flag_cap member=%d cmd=%u aa8=%d ab8=%d", k, (unsigned)nextA.cmdId, s.aa8, s.ab8);
                    buf.active = true;
                    buf.activeAction = nextA;
                    buf.markDispatched(now);
                    dispatchAction(nextA, k);
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

        qh::QEntry q[8] = {};
        for (int i = 0; i < a.ne && i < 8; ++i) { q[i].type = a.e[i].type; q[i].param0 = a.e[i].param0; }
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

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    saferead::beginScope();
    tex::ensure();
    tex::heal();   // every frame, before any gate: a GL reset can happen in menus too
    const bool menuKnown = ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t);
    const bool inMenu = menuKnown && (ctx->inMenu != 0);
    const bool ledge = guikit::clickEdge(g_lmb);   // polled every frame so a held press never fires on reopen
    if (ledge) g_lastClickTick = GetTickCount();
    if (ledge && g_logOn && g_cfg.traceQueue && g_cfg.traceClicks) {
        int tx = 0, ty = 0;
        const bool xy = guikit::cursorEngine(&tx, &ty);
        char line[80];
        qh::fmtClick(line, sizeof line, GetTickCount(), xy, tx, ty);
        QTRACE("%s", line);
    }
    for (int k = 0; k < 3; ++k)
        if (g_cpend[k].on) { g_cpend[k].on = false; QLOG(chCancel, "CANCEL done member=%d n=%d", k, queueCountOf(k)); }
    const bool hardCombat = g_combat != 0 && !g_softCombat;
    if (!g_cfg.cancel || inMenu || hardCombat) {
        cwHideAll(inMenu ? "hud-inactive" : hardCombat ? "combat" : "cancel-off");
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
    g_softCombat = false;
    if (!g_cfg.hideBannerSoft && !g_cfg.hideRedbarSoft) return;
    if (g_bannerHidden && hud != g_bannerHud) {   // HUD rebuilt (load): the old controls are gone
        bannerTrace("forget", false, 0);
        g_bannerHidden = false; g_bannerMask = 0; g_bannerLastAct = -1; g_bannerResetLogged = false;
    }
    if (g_combatUnreadable) return;               // no verdict, no writes; the state is kept
    if (g_combat == 0) {
        if (g_bannerHidden) {                     // combat ended: the engine hid the controls itself, do not restore
            bannerTrace("forget", false, 0);
            g_bannerHidden = false; g_bannerMask = 0; g_bannerResetLogged = false;
        }
        g_bannerLastAct = -1;
        return;
    }
    bool soft = false;
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
            }
            soft = qh::softCombat(true, cands, 3);
        }
    }
    g_softCombat = soft;
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
        if (g_bannerMask) { g_bannerHidden = true; g_bannerHud = hud; }
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
    KP_LOG(chMain, "0.4.1-dev: kit=%d frame_alpha=%.2f cancel=%d enabled=%d waiting_dim=%d border_px=%d log=%d companions=%d strip_icon_px=%d strip_side=%s strip_max=%d", g_cfg.kit ? 1 : 0, g_cfg.frameAlpha, g_cfg.cancel ? 1 : 0, g_cfg.enabled ? 1 : 0, g_cfg.waitingDim ? 1 : 0, g_cfg.borderPx, g_cfg.log ? 1 : 0, g_cfg.companions ? 1 : 0, g_cfg.stripIconPx, g_cfg.stripSide == qh::kSideBelow ? "below" : g_cfg.stripSide == qh::kSideAbove ? "above" : "left", g_cfg.stripMax);
    QLOG(chMain, "pause_eaten_wait=%d dispatch_wait_flags=%d spell_replay_pos=%d", g_cfg.pauseEatenWait ? 1 : 0, g_cfg.dispatchWaitFlags ? 1 : 0, g_cfg.spellReplayPos ? 1 : 0);
    QLOG(chMain, "pause_reinject=%d handoff_on_combat=%d handoff_clear_first=%d handoff_keep_new=%d handoff_resend_held=%d show_in_combat=%d",
         g_cfg.pauseReinject ? 1 : 0, g_cfg.handoffOnCombat ? 1 : 0, g_cfg.handoffClearFirst ? 1 : 0, g_cfg.handoffKeepNew ? 1 : 0, g_cfg.handoffResendHeld ? 1 : 0, g_cfg.showInCombat ? 1 : 0);
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
    g_combat = flagsOk ? static_cast<int>((flags >> 1) & 1u) : 1;   // unreadable = treat as combat (hidden)
    g_combatUnreadable = !flagsOk;
    g_capTick = GetTickCount();
    if (g_hud != static_cast<uintptr_t>(static_cast<uint32_t>(hud))) {
        g_hud = static_cast<uintptr_t>(static_cast<uint32_t>(hud));
        QLOG(chCap, "hud=%08x flags=%08x combat=%d", (unsigned)hud, (unsigned)flags, g_combat);
    }
}

// H9: PerformUseItem detour hook. Consumes and buffers out-of-combat item actions of party members only when queue busy.
int __cdecl QueueHudPerformUseItem(int clientCreature, int ebp) {
    if (g_dispatching || !g_cfg.enabled) return 0;   // re-entry from our own dispatch logs nothing here
    const uint32_t cmd = ++g_cmdSeq;
    const uintptr_t crPtr = static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature));
    if (g_combat != 0) {
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
    const bool fb = false;

    bool haveCur = false;
    int gateQ = -1;
    const int engineQ = queueCountOf(memberK, &haveCur, &gateQ);
    const int engqLog = engineQ < 0 ? -1 : engineQ;
    const int bufBefore = g_itemQueue[memberK].count;
    const qh::ActionDecision dec = qh::decideAction(g_itemQueue[memberK], g_combat != 0, gateQ > 0 ? gateQ : 0, haveCur);

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
        QLOGV(chMain, KPLOG_WARN, "item buffer full for member %d, dropped item %08x cmd=%u", memberK, itemId, (unsigned)cmd);
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
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, false, 0, 0, "pass_combat", false, 0, 0, true);
        return 0;
    }
    saferead::beginScope();

    uint32_t spellId = 0;
    int arg2 = 0;
    uint32_t targetId = 0;
    int multispell = 0;
    int metamagic = 0;
    int domain = 0;
    uintptr_t posPtr = 0;
    float posVal[3] = {};

    if (!rd(SiteQueue + 0x30, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 8, &spellId) ||
        !rd(SiteQueue + 0x31, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0xC, &arg2) ||
        !rd(SiteQueue + 0x32, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &targetId) ||
        !rd(SiteQueue + 0x33, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x14, &multispell) ||
        !rd(SiteQueue + 0x34, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x18, &metamagic) ||
        !rd(SiteQueue + 0x35, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x1C, &domain)) {
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, false, 0, 0, "pass_unreadable", false, 0, 0, false);
        return 0;
    }
    if (rd(SiteQueue + 0x36, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x20, &posPtr) && posPtr) {
        rd(SiteQueue + 0x37, posPtr, 0, &posVal[0]);
        rd(SiteQueue + 0x38, posPtr, 4, &posVal[1]);
        rd(SiteQueue + 0x39, posPtr, 8, &posVal[2]);
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
        traceCmd(cmd, -1, crPtr, qh::ActionKind::Spell, true, spellId, targetId, "pass_nonparty", true, 0, 0, false);
        return 0;
    }
    if (g_logOn && g_cfg.traceQueue) {
        char line[200];
        qh::fmtSpellArgs(line, sizeof line, false, cmd, memberK, spellId, arg2, targetId, multispell, metamagic, domain,
                         static_cast<uint32_t>(posPtr), posPtr != 0);
        QTRACE("%s", line);
    }
    const bool fb = false;

    bool haveCur = false;
    int gateQ = -1;
    const int engineQ = queueCountOf(memberK, &haveCur, &gateQ);
    const int engqLog = engineQ < 0 ? -1 : engineQ;
    const int bufBefore = g_itemQueue[memberK].count;
    const qh::ActionDecision dec = qh::decideAction(g_itemQueue[memberK], g_combat != 0, gateQ > 0 ? gateQ : 0, haveCur);

    if (dec == qh::DECIDE_DISPATCH_NOW) {
        g_itemQueue[memberK].userCancelled = false;
        g_itemQueue[memberK].active = true;
        g_itemQueue[memberK].activeAction = qh::makeSpellAction(
            crPtr, spellId, arg2, targetId, multispell, metamagic, domain, posPtr ? posVal : nullptr, GetTickCount(), cmd);
        g_itemQueue[memberK].markDispatched(GetTickCount());
        g_lastOurs[memberK].tag = "hook"; g_lastOurs[memberK].tick = GetTickCount();
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, targetId, "dispatch_now", fb, engqLog, bufBefore, false);
        return 0;
    }

    if (dec == qh::DECIDE_BUFFER) {
        qh::QueuedAction a = qh::makeSpellAction(
            crPtr, spellId, arg2, targetId, multispell, metamagic, domain, posPtr ? posVal : nullptr, GetTickCount(), cmd);
        g_itemQueue[memberK].push(a);
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, targetId, "buffer", fb, engqLog, bufBefore, false);
        QLOG(chMain, "buffered spell use for member %d (spell=%08x, target=%08x, count=%d) cmd=%u",
             memberK, spellId, targetId, g_itemQueue[memberK].count, (unsigned)cmd);
        return 1;
    }

    if (dec == qh::DECIDE_DROP_FULL) {
        traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, targetId, "drop_full", fb, engqLog, bufBefore, false);
        QLOGV(chMain, KPLOG_WARN, "action buffer full for member %d, dropped spell %08x cmd=%u", memberK, spellId, (unsigned)cmd);
        return 1;
    }

    traceCmd(cmd, memberK, crPtr, qh::ActionKind::Spell, true, spellId, targetId, "unknown", fb, engqLog, bufBefore, false);

    return 0;
}

} // extern "C"
