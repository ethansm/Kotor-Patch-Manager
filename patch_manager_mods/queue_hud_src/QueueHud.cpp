// Queue HUD 0.3.0-dev (Steam Aspyr build 6A522E71...). A GREEN copy of the vanilla combat action-queue panel (up to 4 icons), drawn
// OUT of combat so a command given while paused (a use-item / cast-spell that waits for unpause) is visible.
// Research: patch_manager_mods/30_queue_hud_evidence/RE.md (0x74DC80 panel filler, 0x74E0B0 icon resolver); logic: QueueHudLogic.h.
//
//   hook H8  0x0074C168  HUD Update, just before the combat-flag test: captures the HUD pointer + combat bit (QueueHudCapture)
//   phase    K2PS_HUD_DRAW prio 400 (portrait-slots): reads party member 0's SERVER creature queue, resolves icons with the
//            engine's own 0x74E0B0 (cached), and draws bg + border + icon over the hidden vanilla slots (hud+0x5F2C+i*0x148)
//   0.2.0    companion strips: for party members 1-2 a row of small green queue icons left of their health bar
//            (strip_side left | above | below the card; HUD slot table via portrait-slots; slot creature matched against the party member's client OR server pointer; per-creature round)
//   0.3.0    kit look: the slots are the qh_frame image (icon inset 15%), flat rects only as the fallback when a kit texture is missing;
//            a Cancel control (qh_cancel / qh_cancel_hi, a HUD label built in hud-host's K2HUD_BUILD phase, k2.hud@1) right of the leader's
//            QUEUE0 and left of each companion strip; LMB calls CServerExoApp::ClearAllActions (0x51CB10, doc 30 R7) for that member.
//            Out of combat only (the vanilla red Cancel Combat bar is untouched). No new hook: BUILD + FRAME_BEGIN are registry phases.
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
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "GuiTip.h"
#include "HudHost.h"
#include "EmbedTex.h"
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
constexpr int CrQueue = 0x100, CrRound = 0x10DC;
constexpr int RdList = 0xB00, RdCur = 0xB18, RdAA8 = 0xAA8, RdAB8 = 0xAB8;
constexpr int PauseInternal = 0x10078;               // byte [[[[AppGlobal]+8]+4]+0x10078] & 2 = player pause (0x51C720 / 0x538CD0)
constexpr unsigned SpellRowStride = 0x1A8;
constexpr int AppToServer = 8;                       // *(AppGlobal) + 8 = CServerExoApp (R7: ECX of ClearAllActions)
constexpr int CrObjectId = 4;                        // server creature +4 = server object id (the ClearAllActions argument)

qh::Config g_cfg;
k2ps::Client g_ps;
k2hud::Client g_hudc;

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
    reinterpret_cast<ResolveFn>(AddrIconResolver)(reinterpret_cast<void*>(hud), round, action, node, &scratch, out1, out2);
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
bool playerPaused() {
    uint32_t app = 0, srv = 0, internal = 0;
    uint8_t b = 0;
    if (!rd(SitePause, party::appGlobalAddr(), 0, &app) || !app) return false;
    if (!rd(SitePause + 1, app, 8, &srv) || !srv) return false;
    if (!rd(SitePause + 2, srv, 4, &internal) || !internal) return false;
    return rd(SitePause + 3, internal, PauseInternal, &b) && (b & 2) != 0;
}

bool readRNode(uintptr_t n, qh::RNode* out) {
    uint8_t sub = 0; uint32_t spell = 0, item = 0; uint16_t feat = 0;
    if (!n || !rd(SiteRound + 4, n, 0x10, &sub)) return false;
    rd(SiteRound + 5, n, 0x24, &spell); rd(SiteRound + 6, n, 0x64, &item); rd(SiteRound + 7, n, 0x5C, &feat);
    out->subType = sub; out->spellId = spell; out->itemId = item; out->featId = feat; out->ptr = static_cast<uint32_t>(n);
    return true;
}

struct Snapshot {
    uintptr_t creature = 0, round = 0;
    qh::QEntry e[qh::kMaxEntries]; uintptr_t eData[qh::kMaxEntries]; int ne = 0;
    qh::RNode rn[qh::kMaxRound]; int nrn = 0;
    qh::RNode cur; bool haveCur = false;
    bool paused = false, waiting = false;
};

// One server creature's queue / round, all via saferead. false = no creature (nothing to show).
bool gatherFor(uintptr_t cr, Snapshot& s) {
    s.creature = cr;
    if (!s.creature) return false;
    // queue: inline CExoLinkedList at cr+0x100 -> internal -> head node; node: next @4, data @8
    uint32_t internal = 0, node = 0;
    if (rd(SiteQueue, s.creature, CrQueue, &internal) && internal && rd(SiteQueue + 1, internal, 0, &node)) {
        for (int i = 0; node && i < qh::kMaxEntries; ++i) {
            uint32_t data = 0, next = 0, type = 0, p0 = 0;
            rd(SiteQueue + 2, node, 8, &data);
            rd(SiteQueue + 3, node, 4, &next);
            qh::QEntry& e = s.e[s.ne];
            e.type = -1; e.param0 = 0; s.eData[s.ne] = 0;   // an unreadable entry keeps its index but never qualifies
            if (data && rd(SiteQueue + 4, data, 0, &type)) {
                e.type = static_cast<int>(type);
                rd(SiteQueue + 5, data, 0x38, &p0);
                e.param0 = p0;
                s.eData[s.ne] = data;
            }
            ++s.ne;
            node = next;
        }
    }
    uint32_t round = 0;
    rd(SiteRound, s.creature, CrRound, &round);
    s.round = round;
    int ab8 = 0, aa8 = 0;
    if (round) {
        uint32_t list = 0, curPtr = 0;
        if (rd(SiteRound + 1, round, RdList, &list) && list && rd(SiteRound + 2, list, 0, &internal) && internal && rd(SiteRound + 3, internal, 0, &node)) {
            for (int i = 0; node && i < qh::kMaxRound; ++i) {
                uint32_t data = 0, next = 0;
                rd(SiteRound + 8, node, 8, &data);
                rd(SiteRound + 9, node, 4, &next);
                if (data && readRNode(data, &s.rn[s.nrn])) ++s.nrn;
                node = next;
            }
        }
        if (rd(SiteRound + 10, round, RdCur, &curPtr) && curPtr && readRNode(curPtr, &s.cur)) s.haveCur = true;
        rd(SiteRound + 11, round, RdAB8, &ab8);
        rd(SiteRound + 12, round, RdAA8, &aa8);
    }
    s.paused = playerPaused();
    s.waiting = qh::isWaiting(s.paused, ab8, aa8);
    return true;
}

// Party member 0's (the leader's) server creature.
bool gather(Snapshot& s) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, 0) : 0;
    return gatherFor(m ? party::serverCreature(m) : 0, s);
}

// ---- kit textures (embedded TGAs; recipe copied from companion-selfheal) ----
namespace tex {
#include "QueueHudTextures.inc"
enum { Frame = 0, FrameHi, Cancel, CancelHi };   // order of kQueueHudTextures
static_assert(sizeof kQueueHudTextures / sizeof kQueueHudTextures[0] == 4, "qh_frame, qh_frame_hi, qh_cancel, qh_cancel_hi");
bool g_init = false;
void* g_img[4] = {};
bool g_missLogged[4] = {};

void logLine(const char* fmt, ...) {   // embtex::g_log sink
    char buf[256]; va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    KP_LOG(chMain, "%s", buf);
}
void ensure() {   // once per process; the textures outlive every HUD (never freed)
    if (g_init) return;
    g_init = true;
    embtex::g_log = logLine;
    embtex::ensure(kQueueHudTextures, 4);   // create + fill + queue (idempotent); heal() refills after a GL reset
}
// A pooled GUI handle keeps the texture alive; looked up lazily (null until the texture exists), the miss is logged once per texture.
void* image(int i) {
    if (!g_img[i]) {
        g_img[i] = guikit::imageFor(kQueueHudTextures[i].resref);
        if (!g_img[i] && !g_missLogged[i]) { g_missLogged[i] = true; KP_LOGV(chMain, KPLOG_WARN, "texture %s unavailable: %s", kQueueHudTextures[i].resref, i <= FrameHi ? "flat fallback look" : "Cancel shows a text X"); }
    }
    return g_img[i];
}
void heal() { embtex::heal(); }   // every frame (GL reset + missing rebuild, 20-refill give-up per texture)
}  // namespace tex

// ---- drawing ----
// Kit look: the qh_frame image (white, alpha = style frameA) with the icon inset 15%; a missing frame texture (or kit = 0) falls back to the
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
    int fill = -1;           // 0 = qh_cancel, 1 = qh_cancel_hi, 2 = text fallback
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

// qh_cancel / qh_cancel_hi untinted; no texture or a refused fill -> a text "X" (once).
void cwLook(CW& w) {
    static const float white[3] = { 1.0f, 1.0f, 1.0f };
    const int want = w.hover ? 1 : 0;
    if (w.fill == want || w.fill == 2) return;
    void* img = tex::image(want ? tex::CancelHi : tex::Cancel);
    if (img && guikit::setFill(w.c, img, white)) { w.fill = want; return; }
    guikit::setFont(w.c, "dialogfont10x10"); guikit::setAlign(w.c, 18); guikit::setText(w.c, "X");
    w.fill = 2;
    if (!g_cancelMissLogged) { g_cancelMissLogged = true; KP_LOGV(chCancel, KPLOG_WARN, "Cancel art %s: text fallback", img ? "fill refused" : "texture missing"); }
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
        cwLook(w);
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

// One hidden + click-through label from the LBL_QUEUE1 template; 0 on failure (only that Cancel is disabled).
uintptr_t makeCancelLabel(uintptr_t panel, int* whyOut) {
    int why = guikit::CF_None; uint32_t badVt = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &badVt);
    *whyOut = why;
    if (!c) return 0;
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    if (!guikit::appendToPanel(panel, c)) {   // no destroy helper in GuiKit: the control stays hidden + click-through and unreferenced
        *whyOut = guikit::CF_Count;
        KP_LOGV(chCancel, KPLOG_WARN, "BUILD append failed, control %08x left hidden + click-through", (unsigned)c);
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
    for (int k = 0; k < 3; ++k) { g_cw[k].c = makeCancelLabel(panel, &why[k]); if (g_cw[k].c) ++n; }
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
        sd.n = qh::stripRects(card, vit, sd.n, g_cfg, sd.r, screenW);
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
    return gatherFor(m ? party::serverCreature(m) : 0, s) ? s.ne : -1;
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
    reinterpret_cast<void(__thiscall*)(uintptr_t, uint32_t)>(gameaddr::ServerClearAllActions)(server, id);
    g_cpend[k].on = true; g_cpend[k].n = n;
}

void KPATCH_CALL onFrameBegin(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    saferead::beginScope();
    tex::ensure();
    tex::heal();   // every frame, before any gate: a GL reset can happen in menus too
    const bool ledge = guikit::clickEdge(g_lmb);   // polled every frame so a held press never fires on reopen
    for (int k = 0; k < 3; ++k)
        if (g_cpend[k].on) { g_cpend[k].on = false; QLOG(chCancel, "CANCEL done member=%d n=%d", k, queueCountOf(k)); }
    const bool menuKnown = ctx->struct_size >= offsetof(K2PsContext, inMenu) + sizeof(int32_t);
    if (!g_cfg.cancel || !menuKnown || ctx->inMenu != 0 || g_combat != 0) { cwHideAll(); return; }
    for (CW& w : g_cw)   // the native tooltip, once per control (UI tick, not a draw callback)
        if (w.c && !w.tipSet && guitip::nativeSupported() && guitip::setNative(w.c, "Cancel queued actions")) w.tipSet = true;
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
    KP_LOG(chMain, "0.3.0-dev: kit=%d frame_alpha=%.2f cancel=%d enabled=%d waiting_dim=%d border_px=%d log=%d companions=%d strip_icon_px=%d strip_side=%s strip_max=%d", g_cfg.kit ? 1 : 0, g_cfg.frameAlpha, g_cfg.cancel ? 1 : 0, g_cfg.enabled ? 1 : 0, g_cfg.waitingDim ? 1 : 0, g_cfg.borderPx, g_cfg.log ? 1 : 0, g_cfg.companions ? 1 : 0, g_cfg.stripIconPx, g_cfg.stripSide == qh::kSideBelow ? "below" : g_cfg.stripSide == qh::kSideAbove ? "above" : "left", g_cfg.stripMax);
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

} // extern "C"
