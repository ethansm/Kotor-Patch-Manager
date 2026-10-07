// Queue HUD 0.4.0-dev (CS10 strip slot, CS11 kit art, slim strip) (Steam Aspyr build 6A522E71...). A GREEN copy of the vanilla combat action-queue panel (up to 4 icons), drawn
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
// Every pointer deref goes through saferead (site ids 0x7100+, strips 0x7150+, cancel 0x7160+). The only engine write is the Cancel call.
// Without portrait-slots (or an unknown exe) the mod is inert.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

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

// Multi-item queue buffers (per party member)
qh::ItemBuffer g_itemQueue[3];
bool g_dispatching = false;
using PerformUseItemFn = void(__thiscall*)(void* creature, uint32_t itemId, int packedByte, uint32_t targetId, const float* pos);
constexpr uintptr_t AddrPerformUseItem = gameaddr::PerformUseItem;

// ---- capture state (written by the hook, read by the draw callback; same game thread) ----
uintptr_t g_hud = 0;
int g_combat = 0;
DWORD g_capTick = 0;

template <typename T> bool rd(unsigned site, uintptr_t base, int off, T* out) { return saferead::readAt(site, base, off, out); }

bool g_logOn = true;
#define QLOG(ch, ...) do { if (g_logOn) KP_LOG(ch, __VA_ARGS__); } while (0)
#define QLOGV(ch, lvl, ...) do { if (g_logOn) KP_LOGV(ch, lvl, __VA_ARGS__); } while (0)

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
    if (!actPtr && e.type == qh::T_USE_ITEM) {
        fakeAction[0] = qh::T_USE_ITEM; // 0x2E
        fakeAction[14] = e.param0;      // +0x38 = 14*4
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
    qh::QEntry e[qh::kMaxEntries]; uintptr_t eData[qh::kMaxEntries]; int ne = 0;
    int engineNe = 0;
    int nBuffered = 0;
    qh::RNode rn[qh::kMaxRound]; int nrn = 0;
    qh::RNode cur; bool haveCur = false;
    bool paused = false, waiting = false;
};

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
    for (int i = 0; i < a.nrn; ++i) s.rn[i] = toQh(a.rn[i]);
    s.haveCur = a.haveCur;
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
        const qh::ItemBuffer& ib = g_itemQueue[memberK];
        for (int bi = 0; bi < ib.count && s.ne < qh::kMaxEntries; ++bi) {
            s.e[s.ne].type = qh::T_USE_ITEM;
            s.e[s.ne].param0 = ib.queue[bi].clientItem;
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

void drawChevron(int x, int y, int w, int h, const float* rgb, float alpha) {
    if (w <= 0 || h <= 0) return;
    void* img = guikit::imageFor("uibit_arrow_r");
    if (!img) img = guikit::imageFor("gui_arrow_r");
    if (img) {
        gui2d::drawImage(img, x, y, w, h, 0, rgb, alpha);
        return;
    }
    // Scanline rasterized right-pointing triangle fallback
    for (int col = 0; col < w; ++col) {
        int insetY = ((h / 2) * col) / (w > 1 ? (w - 1) : 1);
        int startY = y + insetY;
        int colH = h - 2 * insetY;
        if (colH > 0) {
            gui2d::fillRect(x + col, startY, 1, colH, rgb, alpha);
        }
    }
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

void cwShow(CW& w, bool on) {   // clickable only while visible (a shown control swallows the click; hidden = click-through)
    if (!w.c) return;
    const int v = on ? 1 : 0;
    if (w.vis == v) return;
    guikit::setClickable(w.c, on);
    guikit::setVisible(w.c, on);
    w.vis = v;
    if (!on) { w.hover = false; w.fill = -1; }
}
void cwHideAll() { for (CW& w : g_cw) cwShow(w, false); }

void cwLookLeader(CW& w) {
    static const float cream[3] = { 0.80f, 0.80f, 0.698f };
    static const float tealBorder[3] = { 0.10f, 0.58f, 0.62f };
    static const float tealHover[3]  = { 0.15f, 0.75f, 0.80f };
    const int want = w.hover ? 1 : 0;
    if (w.fill != want) {
        guikit::setFont(w.c, "dialogfont16x16");
        guikit::setAlign(w.c, 18);
        guikit::setText(w.c, "CANCEL QUEUE");
        guikit::setTextColor(w.c, cream[0], cream[1], cream[2]);
        guikit::setBorderTint(w.c, w.hover ? tealHover : tealBorder);
        guikit::setAlpha(w.c, 0.9f);
        w.fill = want;
    }
}

// kit_cancel / kit_cancel_hi untinted; no texture or a refused fill -> a text "X" (once).
void cwLookCompanion(CW& w) {
    static const float white[3] = { 1.0f, 1.0f, 1.0f };
    const int want = w.hover ? 1 : 0;
    if (w.fill == want || w.fill == 2) return;
    void* img = tex::image(want ? tex::CancelHi : tex::Cancel);
    if (img && guikit::setFill(w.c, img, white)) { w.fill = want; return; }
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
        if (!g_cfg.cancel || !d.show || d.r.w <= 0) { cwShow(w, false); continue; }
        if (!w.rectSet || w.r.x != d.r.x || w.r.y != d.r.y || w.r.w != d.r.w || w.r.h != d.r.h) {
            if (guikit::setRect(w.c, d.r.x, d.r.y, d.r.w, d.r.h)) { w.r = d.r; w.rectSet = true; }
            else { cwShow(w, false); continue; }
        }
        cwLook(w, k);
        cwShow(w, true);
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
    if (!panel || !rd(SiteCancel, panel, 0, &vt) || vt != gameaddr::HudVtable) return;
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
}

void onHudConnect(bool ok, const char* why) { QLOGV(chMain, ok ? KPLOG_INFO : KPLOG_WARN, "hud-host: %s (%s)", ok ? "connected" : "NOT connected (no Cancel controls)", why ? why : ""); }

// Edge log: shown / hidden transitions (never per frame).
struct EdgeState { bool shown = false; uint32_t sig = 0; const char* reason = ""; bool primed = false; };
EdgeState g_edge;

void noteHidden(const char* reason) {
    if (g_edge.primed && !g_edge.shown && g_edge.reason == reason) return;
    g_edge.primed = true; g_edge.shown = false; g_edge.reason = reason; g_edge.sig = 0;
    QLOG(chDraw, "hidden (%s)", reason);
}

// The leader's panel over the hidden vanilla slots (unchanged from 0.1.0). The caller already checked enabled / combat / menu / stale / hud.
void drawLeader(uintptr_t hud, DWORD now, DWORD age, const K2PsContext* ctx) {
    Snapshot s;
    if (!gather(s)) { noteHidden("no creature"); return; }
    qh::Slot slots[qh::kMaxSlots];
    const int n = qh::buildSlots(s.e, s.ne, s.rn, s.nrn, s.haveCur ? &s.cur : nullptr, slots);
    if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, n)) { noteHidden("empty queue"); return; }

    struct Draw { int x, y, w, h; qh::Style st; char rr[17]; };
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
    // Cancel: right of the live QUEUE0 extent, sized by the live QUEUE1 height (read every frame; the HUD scale can change).
    if (qh::cancelVisible(g_cfg.enabled, g_cfg.cancel, g_combat != 0, ctx->inMenu != 0, age, nd)) {
        qh::Rect q0{}, q1{};
        if (guikit::getExtent(hud + HudSlotBase, &q0.x, &q0.y, &q0.w, &q0.h) && guikit::getExtent(hud + HudSlotBase + HudSlotStride, &q1.x, &q1.y, &q1.w, &q1.h)) {
            g_want[0].r = qh::cancelRect(q0, q1);
            g_want[0].show = g_want[0].r.w > 0;
        }
    }
    gui2d::CapGuard capGuard;
    for (int i = 0; i < nd; ++i) drawSlot(d[i].x, d[i].y, d[i].w, d[i].h, d[i].st, guikit::imageFor(d[i].rr));
    if (nd > 0) {
        qh::Rect chev = qh::chevronRect(qh::Rect{ d[0].x, d[0].y, d[0].w, d[0].h });
        drawChevron(chev.x, chev.y, chev.w, chev.h, d[0].st.border, d[0].st.borderA);
    }
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

struct StripDraw { qh::Rect r[qh::kMaxSlots]; qh::Style st[qh::kMaxSlots]; char rr[qh::kMaxSlots][17]; int n = 0; bool have = false; };

void drawStrips(uintptr_t hud, DWORD now) {
    const K2AddrTable* T = k2addr::get();
    if (!T || !T->Slot_VitRect || !T->Slot_CardRect) return;   // unknown exe: no geometry, nothing to anchor to
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    uintptr_t cli[3] = {}, srv[3] = {};
    if (tbl) for (int k = 1; k <= 2; ++k) { cli[k] = party::member(tbl, k); srv[k] = cli[k] ? party::serverCreature(cli[k]) : 0; }
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
    }
    bool any = false;
    for (int k = 1; k <= 2; ++k) {
        if (d[k].have) { any = true; stripNote(k, true, d[k].n, types[k], d[k].r[0], sigs[k]); }
        else stripNote(k, false, 0, "", qh::Rect{ 0, 0, 0, 0 }, 0);
    }
    if (!any) return;
    if (g_cfg.cancel)
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
    const uintptr_t hud = g_hud;
    uint32_t probe = 0;
    if (!hud || !rd(SiteHud, hud, HudSlotBase, &probe)) { if (hud) noteHidden("hud unreadable"); if (g_cfg.companions) stripsHidden(); return; }
    if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, 1)) {
        noteHidden(g_combat ? "combat" : ctx->inMenu ? "menu" : "stale capture");
        if (g_cfg.companions) stripsHidden();
        return;
    }
    drawLeader(hud, now, age, ctx);
    if (g_cfg.companions) drawStrips(hud, now);
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
int queueCountOf(int k) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    Snapshot s;
    return gatherFor(m ? party::serverCreature(m) : 0, s) ? s.engineNe : -1;
}

// Route per R7: CServerExoApp::ClearAllActions(serverObjectId), ECX = *(*AppGlobal + 8). Every pointer is saferead-probed first
// (the codebase has no SEH call guard); nothing is called unless all of them resolve.
void doCancel(int k) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, k) : 0;
    const uintptr_t srv = m ? party::serverCreature(m) : 0;
    uint32_t id = 0, app = 0, server = 0;
    const char* bad = nullptr;
    if (!srv) bad = "no server creature";
    else if (!rd(SiteCancel + 1, srv, CrObjectId, &id) || !id) bad = "server object id unreadable";
    else if (!rd(SiteCancel + 2, party::appGlobalAddr(), 0, &app) || !app || !rd(SiteCancel + 3, app, AppToServer, &server) || !server) bad = "server app unreadable";
    else if (GetTickCount() - g_capTick >= 250u) bad = "stale HUD capture";
    if (bad) { KP_LOGV(chCancel, KPLOG_WARN, "CANCEL refused member=%d (%s)", k, bad); return; }
    const int n = queueCountOf(k);
    QLOG(chCancel, "CANCEL member=%d n=%d route=ServerClearAll", k, n);
    g_itemQueue[k].clear();
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
    g_cpend[k].on = true; g_cpend[k].n = n;
}

// Multi-item buffer ticker and follower wipe guard
void tickItemQueues(DWORD now, bool inMenu) {
    if (inMenu || g_combat != 0) {
        if (g_combat != 0) {
            for (int k = 0; k < 3; ++k) g_itemQueue[k].clear();
        }
        return;
    }
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    if (!tbl) return;

    for (int k = 0; k < 3; ++k) {
        qh::ItemBuffer& buf = g_itemQueue[k];
        uintptr_t cli = party::member(tbl, k);
        uintptr_t srv = cli ? party::serverCreature(cli) : 0;
        if (!srv) {
            if (buf.count > 0 || buf.active) buf.clear();
            continue;
        }

        Snapshot s;
        if (!gatherFor(srv, s)) continue;

        // 1. Check if active item action is present in server queue/round
        bool itemInQueue = false;
        bool hasFollowLeader = false;
        for (int i = 0; i < s.engineNe; ++i) {
            if (s.e[i].type == qh::T_USE_ITEM) itemInQueue = true;
            if (s.e[i].type == 0x3D) hasFollowLeader = true; // AIActionFollowLeader
        }
        if (s.haveCur && s.cur.subType == 10) itemInQueue = true; // 10 = use item
        for (int i = 0; i < s.nrn; ++i) {
            if (s.rn[i].subType == 10) itemInQueue = true;
        }

        // 2. Follower wipe protection & active completion tracking
        if (buf.active) {
            if (qh::shouldReInject(buf, itemInQueue, hasFollowLeader, s.paused, now)) {
                QLOG(chMain, "re-injecting eaten item action for member %d (item=%08x, target=%08x)",
                     k, buf.activeAction.clientItem, buf.activeAction.targetId);
                buf.reInjectPending = true;
                g_dispatching = true;
                reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)(
                    reinterpret_cast<void*>(buf.activeAction.creature),
                    buf.activeAction.clientItem,
                    buf.activeAction.packedByte,
                    buf.activeAction.targetId,
                    buf.activeAction.pos);
                g_dispatching = false;
                buf.activeStartTick = now;
                buf.reInjectPending = false;
            } else if (!itemInQueue && (now - buf.activeStartTick > 500)) {
                buf.active = false;
            }
        }

        // 3. Sequential dispatch of next buffered item
        if (qh::shouldDispatchNext(buf, s.engineNe)) {
            qh::QueuedItemAction nextA;
            if (buf.pop(&nextA)) {
                QLOG(chMain, "dispatching buffered item for member %d (item=%08x, target=%08x, remaining=%d)",
                     k, nextA.clientItem, nextA.targetId, buf.count);
                buf.active = true;
                buf.activeAction = nextA;
                buf.activeStartTick = now;
                g_dispatching = true;
                reinterpret_cast<PerformUseItemFn>(AddrPerformUseItem)(
                    reinterpret_cast<void*>(nextA.creature),
                    nextA.clientItem,
                    nextA.packedByte,
                    nextA.targetId,
                    nextA.pos);
                g_dispatching = false;
            }
        }
    }
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    saferead::beginScope();
    tex::ensure();
    tex::heal();   // every frame, before any gate: a GL reset can happen in menus too
    const bool menuKnown = ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t);
    const bool inMenu = menuKnown && (ctx->inMenu != 0);

    const DWORD now = GetTickCount();
    tickItemQueues(now, inMenu);

    const bool ledge = guikit::clickEdge(g_lmb);   // polled every frame so a held press never fires on reopen
    for (int k = 0; k < 3; ++k)
        if (g_cpend[k].on) { g_cpend[k].on = false; QLOG(chCancel, "CANCEL done member=%d n=%d", k, queueCountOf(k)); }
    if (!g_cfg.cancel || inMenu || g_combat != 0) { cwHideAll(); return; }
    for (int k = 0; k < 3; ++k) {
        CW& w = g_cw[k];
        if (w.c && !w.tipSet && guitip::nativeSupported()) {
            const char* tip = (k == 0) ? "Cancel queued actions" : "Cancel queued actions";
            if (guitip::setNative(w.c, tip)) w.tipSet = true;
        }
    }
    int cx = 0, cy = 0, on = -1;
    if (!guikit::cursorEngine(&cx, &cy)) return;
    for (int k = 0; k < 3; ++k) {
        CW& w = g_cw[k];
        const bool h = w.c && guikit::hit(w.c, cx, cy);   // hit() needs the visible flag: a hidden control never matches
        if (h && !w.hover) guikit::playHoverSound();
        w.hover = h;
        if (h) on = k;
    }
    if (ledge && on >= 0) doCancel(on);
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    g_logOn = g_cfg.log;
    KP_LOG(chMain, "0.4.0-dev: kit=%d frame_alpha=%.2f cancel=%d enabled=%d waiting_dim=%d border_px=%d log=%d companions=%d strip_icon_px=%d strip_side=%s strip_max=%d", g_cfg.kit ? 1 : 0, g_cfg.frameAlpha, g_cfg.cancel ? 1 : 0, g_cfg.enabled ? 1 : 0, g_cfg.waitingDim ? 1 : 0, g_cfg.borderPx, g_cfg.log ? 1 : 0, g_cfg.companions ? 1 : 0, g_cfg.stripIconPx, g_cfg.stripSide == qh::kSideBelow ? "below" : g_cfg.stripSide == qh::kSideAbove ? "above" : "left", g_cfg.stripMax);
    g_ps.init(api, "queue-hud");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_HUD_DRAW, 400, onHudDraw);
    g_ps.add(K2PS_FRAME_BEGIN, 300, onFrameBegin);
    g_ps.connect();
    g_hudc.init(api, "queue-hud");
    g_hudc.onConnect = onHudConnect;
    g_hudc.add(K2HUD_BUILD, 300, onHudBuild);
    g_hudc.connect();
}

// H8 0x0074C168: HUD Update, replaces `mov ecx,[ebp-0x16C]; mov edx,[ecx+0xA904]`. [ebp-0x16C] = hud (this of FUN_0074b3f0).
// Hot path: one safe read + two stores (+ a rare hud-host connect retry); logs only when the HUD pointer changes.
void __cdecl QueueHudCapture(int hud) {
    saferead::beginScope();
    g_ps.poll();
    g_hudc.poll();
    uint32_t flags = 0;
    g_combat = rd(SiteCap, static_cast<uintptr_t>(static_cast<uint32_t>(hud)), HudCombatFlags, &flags) ? static_cast<int>((flags >> 1) & 1u) : 1;   // unreadable = treat as combat (hidden)
    g_capTick = GetTickCount();
    if (g_hud != static_cast<uintptr_t>(static_cast<uint32_t>(hud))) {
        g_hud = static_cast<uintptr_t>(static_cast<uint32_t>(hud));
        QLOG(chCap, "hud=%08x flags=%08x combat=%d", (unsigned)hud, (unsigned)flags, g_combat);
    }
}

// H9: PerformUseItem detour hook. Consumes and buffers out-of-combat item actions when queue busy.
int __cdecl QueueHudPerformUseItem(int clientCreature, int ebp) {
    if (g_dispatching || !g_cfg.enabled || g_combat != 0) return 0;
    saferead::beginScope();

    uint32_t itemId = 0;
    int packedByte = 0;
    uint32_t targetId = 0;
    uintptr_t posPtr = 0;
    float posVal[3] = {};

    if (!rd(SiteQueue + 0x20, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 8, &itemId) ||
        !rd(SiteQueue + 0x21, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0xC, &packedByte) ||
        !rd(SiteQueue + 0x22, static_cast<uintptr_t>(static_cast<uint32_t>(ebp)), 0x10, &targetId)) {
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
    if (memberK < 0) memberK = 0;

    const int engineQ = queueCountOf(memberK);
    const qh::ActionDecision dec = qh::decideItemAction(g_itemQueue[memberK], g_combat != 0, engineQ > 0 ? engineQ : 0);

    if (dec == qh::DECIDE_DISPATCH_NOW) {
        g_itemQueue[memberK].active = true;
        g_itemQueue[memberK].activeAction.creature = static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature));
        g_itemQueue[memberK].activeAction.clientItem = itemId;
        g_itemQueue[memberK].activeAction.packedByte = packedByte;
        g_itemQueue[memberK].activeAction.targetId = targetId;
        memcpy(g_itemQueue[memberK].activeAction.pos, posVal, sizeof(posVal));
        g_itemQueue[memberK].activeAction.queueTime = GetTickCount();
        g_itemQueue[memberK].activeStartTick = GetTickCount();
        return 0;
    }

    if (dec == qh::DECIDE_BUFFER) {
        qh::QueuedItemAction a;
        a.creature = static_cast<uintptr_t>(static_cast<uint32_t>(clientCreature));
        a.clientItem = itemId;
        a.packedByte = packedByte;
        a.targetId = targetId;
        memcpy(a.pos, posVal, sizeof(posVal));
        a.queueTime = GetTickCount();
        g_itemQueue[memberK].push(a);
        QLOG(chMain, "buffered item use for member %d (item=%08x, target=%08x, count=%d)",
             memberK, itemId, targetId, g_itemQueue[memberK].count);
        return 1;
    }

    if (dec == qh::DECIDE_DROP_FULL) {
        QLOGV(chMain, KPLOG_WARN, "item buffer full for member %d, dropped item %08x", memberK, itemId);
        return 1;
    }

    return 0;
}

} // extern "C"
