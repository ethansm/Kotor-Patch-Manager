// Queue HUD 0.1.0-dev (Steam Aspyr build 6A522E71...). A GREEN copy of the vanilla combat action-queue panel (up to 4 icons), drawn
// OUT of combat so a command given while paused (a use-item / cast-spell that waits for unpause) is visible.
// Research: patch_manager_mods/30_queue_hud_evidence/RE.md (0x74DC80 panel filler, 0x74E0B0 icon resolver); logic: QueueHudLogic.h.
//
//   hook H8  0x0074C168  HUD Update, just before the combat-flag test: captures the HUD pointer + combat bit (QueueHudCapture)
//   phase    K2PS_HUD_DRAW prio 400 (portrait-slots): reads party member 0's SERVER creature queue, resolves icons with the
//            engine's own 0x74E0B0 (cached), and draws bg + border + icon over the hidden vanilla slots (hud+0x5F2C+i*0x148)
// READ-ONLY toward engine state: no engine memory writes, every pointer deref goes through saferead (site ids 0x7100+).
// Without portrait-slots (or an unknown exe) the mod is inert.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "KpLog.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "Party.h"
#include "PortraitSlots.h"
#include "Gui2D.h"
#include "GuiKit.h"
#include "QueueHudLogic.h"

KP_LOG_PATCH("queue-hud");

namespace {

KP_CHANNEL(chMain, "main");
KP_CHANNEL(chCap, "cap");
KP_CHANNEL(chIcon, "icon");
KP_CHANNEL(chDraw, "draw");

// saferead site ids (0x7100+)
constexpr unsigned SiteCap = 0x7100, SiteHud = 0x7101, SiteQueue = 0x7110, SiteRound = 0x7120, SitePause = 0x7130, SiteRules = 0x7140;

// Steam Aspyr addresses / offsets (RE.md; the manifest supports the Steam exe only).
constexpr uintptr_t AddrIconResolver = 0x0074E0B0;   // thiscall(hud ignored) 6 stack args, RET 0x18
constexpr uintptr_t AddrRulesGlobal = 0x00A1B4D0;    // [rules+0x104] -> spell table {count@0, data@4}, row stride 0x1A8
constexpr int HudCombatFlags = 0xA904;               // bit 1 = combat mode (set by 0x74E910 just before the hook)
constexpr int HudSlotBase = 0x5F2C, HudSlotStride = 0x148;
constexpr int CrQueue = 0x100, CrRound = 0x10DC;
constexpr int RdList = 0xB00, RdCur = 0xB18, RdAA8 = 0xAA8, RdAB8 = 0xAB8;
constexpr int PauseInternal = 0x10078;               // byte [[[[AppGlobal]+8]+4]+0x10078] & 2 = player pause (0x51C720 / 0x538CD0)
constexpr unsigned SpellRowStride = 0x1A8;

qh::Config g_cfg;
k2ps::Client g_ps;

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

// Party member 0's server creature + queue / round, all via saferead. false = chain broken (nothing to show).
bool gather(Snapshot& s) {
    party::Fail why = party::F_None;
    const uintptr_t tbl = party::table(&why);
    const uintptr_t m = tbl ? party::member(tbl, 0) : 0;
    s.creature = m ? party::serverCreature(m) : 0;
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

// ---- drawing ----
void drawSlot(int x, int y, int w, int h, const qh::Style& st, void* img) {
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
    if (img) {
        static const float white[3] = { 1.0f, 1.0f, 1.0f };
        gui2d::drawImage(img, x, y, w, h, 0, white, st.iconA);
    }
}

// Edge log: shown / hidden transitions (never per frame).
struct EdgeState { bool shown = false; uint32_t sig = 0; const char* reason = ""; bool primed = false; };
EdgeState g_edge;

void noteHidden(const char* reason) {
    if (g_edge.primed && !g_edge.shown && g_edge.reason == reason) return;
    g_edge.primed = true; g_edge.shown = false; g_edge.reason = reason; g_edge.sig = 0;
    QLOG(chDraw, "hidden (%s)", reason);
}

void KPATCH_CALL onHudDraw(const K2PsContext* ctx, void*) {
    if (!g_cfg.enabled || !ctx) return;
    saferead::beginScope();
    const DWORD now = GetTickCount();
    const DWORD age = now - g_capTick;
    const uintptr_t hud = g_hud;
    uint32_t probe = 0;
    if (!hud || !rd(SiteHud, hud, HudSlotBase, &probe)) { if (hud) noteHidden("hud unreadable"); return; }
    if (!qh::shouldShow(g_cfg.enabled, g_combat != 0, ctx->inMenu != 0, age, 1)) {
        noteHidden(g_combat ? "combat" : ctx->inMenu ? "menu" : "stale capture");
        return;
    }
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
    gui2d::CapGuard capGuard;
    for (int i = 0; i < nd; ++i) drawSlot(d[i].x, d[i].y, d[i].w, d[i].h, d[i].st, guikit::imageFor(d[i].rr));
}

} // namespace

extern "C" {

void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    loadIni();
    g_logOn = g_cfg.log;
    KP_LOG(chMain, "0.1.0-dev: enabled=%d waiting_dim=%d border_px=%d log=%d", g_cfg.enabled ? 1 : 0, g_cfg.waitingDim ? 1 : 0, g_cfg.borderPx, g_cfg.log ? 1 : 0);
    g_ps.init(api, "queue-hud");
    g_ps.onConnect = onConnect;
    g_ps.add(K2PS_HUD_DRAW, 400, onHudDraw);
    g_ps.connect();
}

// H8 0x0074C168: HUD Update, replaces `mov ecx,[ebp-0x16C]; mov edx,[ecx+0xA904]`. [ebp-0x16C] = hud (this of FUN_0074b3f0).
// Hot path: one safe read + two stores; logs only when the HUD pointer changes.
void __cdecl QueueHudCapture(int hud) {
    saferead::beginScope();
    g_ps.poll();
    uint32_t flags = 0;
    g_combat = rd(SiteCap, static_cast<uintptr_t>(static_cast<uint32_t>(hud)), HudCombatFlags, &flags) ? static_cast<int>((flags >> 1) & 1u) : 1;   // unreadable = treat as combat (hidden)
    g_capTick = GetTickCount();
    if (g_hud != static_cast<uintptr_t>(static_cast<uint32_t>(hud))) {
        g_hud = static_cast<uintptr_t>(static_cast<uint32_t>(hud));
        QLOG(chCap, "hud=%08x flags=%08x combat=%d", (unsigned)hud, (unsigned)flags, g_combat);
    }
}

} // extern "C"
