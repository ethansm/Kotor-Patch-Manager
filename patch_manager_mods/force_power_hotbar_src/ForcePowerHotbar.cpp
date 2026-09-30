// Force Power Hotbar (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Prototype phase: 30 keybound slots
// (1-0, Ctrl+1-0, Alt+1-0) that invoke a vanilla HUD quick-action; NO on-screen bars yet.
// Research trail and every address: patch_manager_mods/07_force_power_hotbar.md (local-setup/KOTOR-II worktree).
//
// Hooks (cdecl detours, original bytes still run):
//   0x0072CB63  CExoInputInternal::GetEvents+0x383  CaptureHotbarKey(record*, device)  -- swallow bound digits
//   0x0074B3F0  CSWGuiMainInterface::Draw           HotbarHudTick(hud)                 -- gate, config, probe, invoke, button state
//   0x00748502  HUD ctor (FUN_00747210), just before StopLoadFromLayout  HotbarBuildButtons(panel) -- construct + bind 10 CSWGuiLabel
//               controls LBL_HOTBAR1..9,0 (IDs 111..120, added to mipc28x6_p.gui by scripts/add_hotbar_buttons_gui.py). The panel's own
//               Draw (0x0040EC90) then draws them with the HUD; the tick only swaps icon (border+0x74), alpha (border+0x24) and visibility.
//
// Invoke design: the HUD's own per-group action list (hud+0x78+g*0xC, entry stride 0x3C) holds powers
// (key = spells.2da row | 0x20000000) and items (key = item object id | 0x40000000). We set the group's selection
// key (hud+0x1C7C+g*4) to the wanted entry, call FUN_00751750(hud, g, 0) (the game's own usability checks,
// feedback and action queueing), then restore the previous selection.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <set>
#include <algorithm>
#include "HotbarConfig.h"
#include "TwoDA.h"
#include "Tlk.h"

namespace {

// ---- addresses (Steam) -----------------------------------------------------------------------------
constexpr uintptr_t HudInvokeGroup = 0x00751750;      // thiscall(hud, group 0..6, 0) RET 8
constexpr uintptr_t KeyboardDeviceGlobal = 0x0099C884; // rdata constant 0 (device-array index); verify live
constexpr uintptr_t AppGlobal = 0x00A1B4A4;           // *(app)+4 -> A ; FUN_0073fb90(A) = *(*(A+4)+0x270)
constexpr uintptr_t GetControlledCreature = 0x007E5D80; // thiscall(partyTable) -> creature (party slot 0)
// Panels that do not block gameplay hotkeys (RTTI vtables): messages, fade, pause, auto-pause
constexpr uintptr_t PanelWhitelist[] = { 0x0099E5EC, 0x009A044C, 0x009A7BAC, 0x009A8FCC };
constexpr uintptr_t HudVtable = 0x0099DCC4;

// ---- HUD / manager layout (EKC K2_CONFIG; verified live in the probe log) -------------------------
constexpr int Hud_Manager = 0x1C;
constexpr int Panel_Flags = 0x48;
constexpr int PanelIgnoredByInput = 0x600;
constexpr int Mgr_Panels = 0x88, Mgr_PanelCount = 0x8C, Mgr_Modals = 0x94, Mgr_ModalCount = 0x98;
constexpr int Hud_GroupLists = 0x78;    // {entries ptr @+0, count @+4, cap @+8} * 7 groups
constexpr int Hud_GroupSel = 0x1C7C;    // int key per group, -1 = none
constexpr int Hud_ActiveGroup = 0x1C98;
constexpr int GroupCount = 7;           // 0..5 personal + 6 (unpopulated)
constexpr int EntryStride = 0x3C;
constexpr int Entry_Key = 0x8, Entry_Owner = 0x1C, Entry_Flags = 0x30;
constexpr int MaxEntries = 256;

// ---- on-screen buttons (0.2.0): CSWGuiLabel widgets, see 07_force_power_hotbar.md "Bottom-bar UX" ---------
constexpr uintptr_t FnLabelCtor = 0x00419740;     // CSWGuiLabel ctor, thiscall(ECX=this), no stack args; object size 0x148
constexpr uintptr_t FnInitControl = 0x0040F620;   // thiscall(panel, ctl, CExoString* tag, addToList, scale) RET 16
constexpr uintptr_t FnExoCtor = 0x00733570, FnExoDtor = 0x00733780;
constexpr uintptr_t FnNew = 0x00919723;           // cdecl(size)
constexpr uintptr_t FnLoadImage = 0x0047EB60;     // cdecl(resref char[17]) -> new image wrapper (8 bytes) each call: pooled
constexpr uintptr_t LabelVtable = 0x009878BC, BorderVtable = 0x009875BC;
constexpr int LabelSize = 0x148;
constexpr int Label_Border = 0x60;                // embedded CSWGuiBorder
constexpr int Ctl_Flags = 0x48;                   // bit1 = visible (panel Draw skips the control otherwise)
constexpr int Ctl_Id = 0x54;
constexpr int Border_W = 0xC, Border_H = 0x10, Border_Alpha = 0x24, Border_Fill = 0x74;
constexpr int Panel_CtlArray = 0x24, Panel_CtlCount = 0x28;
constexpr int ButtonCount = 10, FirstButtonId = 111;   // frames 111..120, icon labels 121..130 (FIRST_ID in scripts/add_hotbar_buttons_gui.py)
constexpr int IconIdOffset = 10;
constexpr uintptr_t ProjectionFlag = 0x00A1B9F0;
// (FUN_0073FEA0 screen-info getter is a THISCALL reading [ecx+0x274]: never call it without a valid this)
constexpr uintptr_t GuiRefW = 0x009F4390, GuiRefH = 0x009F4394;   // ints (800, 600): the 800x600 layout space
// config UI (0.3.0), IDs continue after the 20 hotbar labels; MUST match ALL_TAGS order in scripts/add_hotbar_buttons_gui.py.
// The tooltip pair is last so it draws above the picker (panel Draw is ascending by ID).
constexpr int EditId = FirstButtonId + 2 * ButtonCount, PbgId = EditId + 1, TitleId = EditId + 2, TabId0 = EditId + 3, TabCount = 3;
constexpr int PrevId = EditId + 6, NextId = EditId + 7, ClearId = EditId + 8;
constexpr int CellCount = 8, CellBgId0 = EditId + 9, CellIcId0 = CellBgId0 + CellCount;
constexpr int TipBgId = CellIcId0 + CellCount, TipId = TipBgId + 1;   // hover tooltip frame + text
constexpr int TotalNewControls = TipId - FirstButtonId + 1;
constexpr int CellTipBase = 100, ToggleTipId = 200, OtherTipBase = 210;                  // tooltip owner ids >= this are picker cells
constexpr uintptr_t FnTextSet = 0x00416E30;       // thiscall(ECX = CSWGuiText+0x18 = label+0xF0, CExoString*) RET 4: sets text + relayouts
constexpr uintptr_t TextVtable = 0x009876B4;
constexpr int Label_Text = 0xD8, Text_Renderer = 0x14, Text_StringSub = 0x18;
constexpr DWORD TipDelayMs = 450;
constexpr int TipCharW = 21, TipLineH = 32, TipPad = 22, TipWrap = 38, TipMaxDescLines = 3;   // px estimates for dialogfont16x16 at 2.667 px/unit; calibrate live
const char* const Spells2daPath = "override/spells.2da";
const char* const TlkPath = "dialog.tlk";
const char* const FrameResref = "hotbar_slot";       // TGAs written by the GUI script into override/
const char* const FrameHiResref = "hotbar_slot_hi";
const char* const EditResref = "hotbar_edit", *const EditHiResref = "hotbar_edit_hi";   // thin wrench toggle art
constexpr double EditBoxUnits = 38.0, PickBoxUnits = 112.0, BoxPadUnits = 8.0, TitleUnits = 22.0;   // must match the GUI script layout
constexpr float EmptyFrameAlpha = 0.45f;
constexpr int ButtonRefreshFrames = 8;            // entry re-resolve interval (usable/dim state changes are slow)
constexpr float DimAlpha = 0.35f;

// ---- logging: force_power_hotbar_log.txt in the game dir, capped ------------------------------------
int g_logLines = 0;
long g_frameForLog = 0;   // mirrors g_frame so every line is frame-stamped
constexpr int MaxLogLines = 60000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) { if (g_logLines == MaxLogLines) { ++g_logLines; FILE* f = fopen("force_power_hotbar_log.txt", "a"); if (f) { fputs("LOG CAP REACHED -- further lines dropped\n", f); fclose(f); } } return; }
    FILE* f = fopen("force_power_hotbar_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    fprintf(f, "[%lu f%ld] ", (unsigned long)GetTickCount(), g_frameForLog);
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

template <typename T> bool readAt(uintptr_t base, int off, T* out) {
    if (base == 0 || IsBadReadPtr(reinterpret_cast<const void*>(base + off), sizeof(T))) return false;
    *out = *reinterpret_cast<const T*>(base + off);
    return true;
}
int rdInt(uintptr_t base, int off, int dflt = 0) { int v; return readAt(base, off, &v) ? v : dflt; }

// Guarded CExoString {char* text; int len} read; empty on anything implausible.
std::string readCExoString(uintptr_t addr) {
    int p, len;
    if (!readAt(addr, 0, &p) || !readAt(addr, 4, &len)) return "";
    if (len <= 0 || len > 80 || p == 0 || IsBadReadPtr(reinterpret_cast<const void*>(p), len)) return "";
    std::string s(reinterpret_cast<const char*>(p), len);
    while (!s.empty() && s.back() == '\0') s.pop_back();   // engine lengths include the trailing NUL (live-verified)
    if (s.empty()) return "";
    for (unsigned char c : s) if (c < 0x20 || c > 0x7E) return "";
    return s;
}

// ---- action decode (shared by the catalog, the INI and the resolver) --------------------------------
struct Entry { int group, index, key, flags; std::string name, icon; hb::ActionRef ref; };

hb::ActionRef decodeKey(int key, const std::string& name) {
    hb::ActionRef r;
    if (key & 0x40000000) { r.kind = hb::K_Item; r.name = hb::normalizeItemName(name); if (r.name.empty()) r.kind = hb::K_None; }
    else if (key & 0x20000000) { r.kind = hb::K_Spell; r.id = key & 0x1FFFFFFF; }
    else { r.kind = hb::K_Other; r.id = key; }
    return r;
}

// Reads group g's entry list from the hud (guarded). Returns count read.
int readGroup(uintptr_t hud, int g, std::vector<Entry>* out) {
    out->clear();
    int base = rdInt(hud, Hud_GroupLists + g * 12), count = rdInt(hud, Hud_GroupLists + 4 + g * 12);
    if (count < 0 || count > MaxEntries) { logf("readGroup g=%d: implausible count=%d base=%08x", g, count, (unsigned)base); return 0; }
    if (base == 0 || count == 0) return 0;
    for (int i = 0; i < count; ++i) {
        uintptr_t e = (uintptr_t)base + (uintptr_t)i * EntryStride;
        if (IsBadReadPtr(reinterpret_cast<const void*>(e), EntryStride)) { logf("readGroup g=%d: entry %d at %08x unreadable, stopping", g, i, (unsigned)e); break; }
        Entry en; en.group = g; en.index = i; en.key = rdInt(e, Entry_Key); en.flags = rdInt(e, Entry_Flags);
        en.name = readCExoString(e);
        { char ib[17] = {0}; if (!IsBadReadPtr(reinterpret_cast<const void*>(e + 0x1C + 4), 16)) { memcpy(ib, reinterpret_cast<const void*>(e + 0x20), 16); for (int k = 0; k < 16; ++k) if (ib[k] && (ib[k] < 0x20 || ib[k] > 0x7E)) ib[k] = '?'; } en.icon = ib; }
        en.ref = decodeKey(en.key, en.name);
        out->push_back(en);
    }
    return (int)out->size();
}

// ---- state --------------------------------------------------------------------------------------------
const char* IniPath = "force_power_hotbar.ini";
const char* CatalogPath = "force_power_hotbar_available.txt";
hb::Config g_cfg;
long long g_cfgMtime = -2;
DWORD g_lastCfgCheck = 0;
std::string g_charKey;                 // key of the controlled creature
std::string g_activeDesc;
volatile bool g_slotBound[hb::Rows][hb::Slots];
volatile DWORD g_hudTick = 0;          // last HUD tick time
volatile bool g_hudGateOk = false;
uintptr_t g_hud = 0;
int g_kbDevice = 0;
long g_frame = 0;

struct Req { int row, slot; };
Req g_ring[16]; int g_ringHead = 0, g_ringTail = 0;  // single producer (input) / consumer (tick) on the game thread
bool g_downSwallowed[256];
bool g_ctrl = false, g_alt = false, g_shift = false;
int g_capLogged = 0;

// ---- gate: gameplay HUD is the active panel (EKC IsGameplayHudActive port, guarded) -------------------
bool isIgnored(uintptr_t panel) { return (rdInt(panel, Panel_Flags) & PanelIgnoredByInput) != 0; }
bool isWhitelisted(uintptr_t panel) {
    uintptr_t vt; if (!readAt(panel, 0, &vt)) return false;
    for (uintptr_t w : PanelWhitelist) if (vt == w) return true;
    return false;
}
const char* gateWhy = "";
bool gameplayHudActive(uintptr_t hud) {
    uintptr_t mgr; if (!readAt(hud, Hud_Manager, &mgr) || !mgr) { gateWhy = "no manager"; return false; }
    int modals = rdInt(mgr, Mgr_Modals), modalCount = rdInt(mgr, Mgr_ModalCount);
    if (modalCount < 0 || modalCount > 64) { gateWhy = "bad modal count"; return false; }
    for (int i = modalCount - 1; i >= 0; --i) {
        uintptr_t p; if (!readAt((uintptr_t)modals, i * 4, &p) || !p) { gateWhy = "bad modal ptr"; return false; }
        if (!isIgnored(p)) { gateWhy = "modal open"; return false; }
    }
    int panels = rdInt(mgr, Mgr_Panels), panelCount = rdInt(mgr, Mgr_PanelCount);
    if (panelCount < 0 || panelCount > 256) { gateWhy = "bad panel count"; return false; }
    for (int i = panelCount - 1; i >= 0; --i) {
        uintptr_t p; if (!readAt((uintptr_t)panels, i * 4, &p) || !p) { gateWhy = "bad panel ptr"; return false; }
        if (isIgnored(p)) continue;
        if (p == hud) { gateWhy = "hud on top"; return true; }
        if (!isWhitelisted(p)) { gateWhy = "other panel on top"; return false; }
    }
    gateWhy = "hud not in list"; return false;
}

void logPanels(uintptr_t hud) {
    uintptr_t mgr; if (!readAt(hud, Hud_Manager, &mgr) || !mgr) return;
    int modals = rdInt(mgr, Mgr_Modals), modalCount = rdInt(mgr, Mgr_ModalCount);
    int panels = rdInt(mgr, Mgr_Panels), panelCount = rdInt(mgr, Mgr_PanelCount);
    logf("  panels: hud=%08x hudvt=%08x mgr=%08x modals=%d panels=%d", (unsigned)hud, rdInt(hud, 0), (unsigned)mgr, modalCount, panelCount);
    for (int i = 0; i < modalCount && i < 16; ++i) { int p = rdInt((uintptr_t)modals, i * 4); logf("    modal[%d]=%08x vt=%08x flags=%x", i, p, rdInt(p, 0), rdInt(p, Panel_Flags)); }
    for (int i = 0; i < panelCount && i < 32; ++i) { int p = rdInt((uintptr_t)panels, i * 4); logf("    panel[%d]=%08x vt=%08x flags=%x", i, p, rdInt(p, 0), rdInt(p, Panel_Flags)); }
}

// ---- controlled creature + characterKey ---------------------------------------------------------------
uintptr_t controlledCreature() {
    static const char* lastFail = nullptr;
    auto fail = [&](const char* why) { if (lastFail != why) { lastFail = why; logf("creature: unavailable (%s)", why); } return (uintptr_t)0; };
    int app; if (!readAt(AppGlobal, 0, &app) || !app) return fail("app global null");
    int a; if (!readAt(app, 4, &a) || !a) return fail("app+4 null");
    int b; if (!readAt(a, 4, &b) || !b) return fail("a+4 null");
    int party; if (!readAt(b, 0x270, &party) || !party) return fail("party table null");
    if (rdInt(party, 0) <= 0) return fail("party count 0");   // member count
    lastFail = nullptr;
    using Fn = uintptr_t(__thiscall*)(void*);
    return reinterpret_cast<Fn>(GetControlledCreature)(reinterpret_cast<void*>(party));
}

std::set<int> g_scannedCreatures;
// Probe: log every printable {char*,len} CExoString-shaped field and inline ASCII run in the first 0x800 bytes.
void scanObjectStrings(const char* tag, uintptr_t obj) {
    logf("  scan %s obj=%08x vt=%08x id=%08x", tag, (unsigned)obj, rdInt(obj, 0), rdInt(obj, 4));
    for (int off = 0; off < 0x800; off += 4) {
        std::string s = readCExoString(obj + off);
        if (s.size() >= 2) logf("    +%03x CExoString \"%s\"", off, s.c_str());
    }
    for (int off = 0; off < 0x800; off += 4) {
        char buf[17]; int n = 0;
        while (n < 16) { char c; if (!readAt(obj, off + n, &c)) break; if (c == 0) break; if (c < 0x20 || c > 0x7E) { n = 0; break; } buf[n++] = c; }
        if (n >= 4 && n <= 16) { buf[n] = 0; logf("    +%03x inline \"%s\"", off, buf); }
    }
}

std::string computeCharacterKey() {
    uintptr_t c = controlledCreature();
    if (!c) return "";
    if (g_cfg.general.creatureTagOffset > 0) {
        std::string t = readCExoString(c + g_cfg.general.creatureTagOffset);
        std::string k; for (char ch : t) k += (isalnum((unsigned char)ch) || ch == '_') ? ch : '_';
        if (!k.empty()) return k;
    }
    unsigned oid = (unsigned)rdInt(c, 4);
    if (oid == 0xFFFFFFFFu || oid == 0) return "";   // invalid object id: transient during loads, never a real character
    char b[24]; snprintf(b, sizeof b, "obj%08x", oid);
    // Companion object ids (obj8000xxxx) are runtime heap ids that change across save loads. CSWCObject::GetPortraitId (0x007ED4B0) is
    // the ushort at object+0xB8 = the portraits.2da row, fixed per character; use it for everything but the Exile (objfffffffd, stable).
    unsigned short portrait = 0; bool havePortrait = readAt(c, 0xB8, &portrait);
    static unsigned s_lastLogged = 0;
    if (oid != s_lastLogged) { s_lastLogged = oid; logf("character: oid=%08x portraitId(+0xB8)=%u readable=%d -> %s", oid, (unsigned)portrait, (int)havePortrait, (oid != 0xFFFFFFFDu && havePortrait && portrait != 0 && portrait < 0x1000) ? "portrait key" : "object-id key"); }
    if (oid != 0xFFFFFFFDu && havePortrait && portrait != 0 && portrait < 0x1000) { snprintf(b, sizeof b, "port%u", (unsigned)portrait); return b; }
    if (!g_scannedCreatures.count(rdInt(c, 4))) { g_scannedCreatures.insert(rdInt(c, 4)); scanObjectStrings("controlled", c); }
    return b;
}

// ---- config: load / hot-reload / active slots --------------------------------------------------------
void writeTemplateIni() {
    hb::Config t; t.general.invoke = 0; t.profile("Default");
    t.Save(IniPath);
    logf("cfg: wrote template %s", IniPath);
}

// Effective Bottom row (row 0): the configured binding, else (AutoFill=1) the character's next known power not bound elsewhere on the bar.
hb::ActionRef g_eff[hb::Slots]; bool g_effAuto[hb::Slots]; unsigned g_effHash = 0;

void recomputeBound() {
    int bound = 0;
    for (int r = 0; r < hb::Rows; ++r) for (int s = 0; s < hb::Slots; ++s) {
        if (r == 0 && g_cfg.general.autoFill) continue;   // owned by refreshEffective
        hb::ActionRef a = g_cfg.slotFor(g_charKey, r, s);
        g_slotBound[r][s] = !a.empty(); if (!a.empty()) ++bound;
    }
    logf("cfg: profile for character '%s' -> %d/30 slots bound", g_charKey.c_str(), bound);
    if (g_cfg.general.verbose) for (int r = 0; r < hb::Rows; ++r) for (int s = 0; s < hb::Slots; ++s) {
        hb::ActionRef a = g_cfg.slotFor(g_charKey, r, s);
        if (!a.empty()) logf("    %s = %s", hb::SlotKeyName(r, s).c_str(), hb::Format(a).c_str());
    }
}

void refreshEffective(uintptr_t hud) {
    std::vector<hb::ActionRef> explicitRefs;
    for (int i = 0; i < hb::Slots; ++i) { hb::ActionRef a = g_cfg.slotFor(g_charKey, 0, i); if (!a.empty()) explicitRefs.push_back(a); }
    std::vector<hb::ActionRef> autos;
    if (g_cfg.general.autoFill) {
        std::vector<Entry> es; readGroup(hud, 0, &es);
        for (auto& e : es) {
            if ((e.key & 0x40000000) || !(e.key & 0x20000000)) continue;   // known Force powers only (no items, no mode toggles)
            bool dup = false; for (auto& x : explicitRefs) if (hb::Same(x, e.ref)) dup = true;
            if (!dup) autos.push_back(e.ref);
        }
        // then the selected entry of each item group (1 medpac/stim, 2 shield, 3 mine), like the native quick bar's current pick
        for (int g = 1; g <= 3; ++g) {
            std::vector<Entry> is; readGroup(hud, g, &is);
            if (is.empty()) continue;
            unsigned selKey = (unsigned)rdInt(hud, Hud_GroupSel + g * 4);
            const Entry* pick = &is[0]; for (auto& e : is) if ((unsigned)e.key == selKey) { pick = &e; break; }
            bool dup = false; for (auto& x : explicitRefs) if (hb::Same(x, pick->ref)) dup = true;
            for (auto& x : autos) if (hb::Same(x, pick->ref)) dup = true;
            logf("eff: autofill group %d pick key=%08x '%s' ref=%s%s", g, (unsigned)pick->key, pick->name.c_str(), hb::Format(pick->ref).c_str(), dup ? " (dup, skipped)" : "");
            if (!dup && !pick->ref.empty()) autos.push_back(pick->ref);
        }
    }
    size_t next = 0; unsigned h = 2166136261u; int nAuto = 0;
    for (int s = 0; s < hb::Slots; ++s) {
        hb::ActionRef a = g_cfg.slotFor(g_charKey, 0, s); bool isAuto = false;
        if (a.empty() && !a.blank && next < autos.size()) { a = autos[next++]; isAuto = true; ++nAuto; }   // explicit "none" stays empty
        g_eff[s] = a; g_effAuto[s] = isAuto; g_slotBound[0][s] = !a.empty();
        h = (h ^ (unsigned)hb::Format(a).size() ^ ((unsigned)a.id << 8) ^ (isAuto ? 0x5000000u : 0)) * 16777619u;
        for (char c : hb::Format(a)) h = (h ^ (unsigned char)c) * 16777619u;
    }
    if (h != g_effHash) {
        g_effHash = h;
        logf("eff: character '%s' autofill=%d explicit=%d auto=%d", g_charKey.c_str(), g_cfg.general.autoFill, (int)explicitRefs.size(), nAuto);
        for (int s = 0; s < hb::Slots; ++s) if (!g_eff[s].empty()) logf("    Bottom%c = %s%s", "1234567890"[s], hb::Format(g_eff[s]).c_str(), g_effAuto[s] ? " (auto)" : "");
    }
}

void checkConfig(bool force) {
    DWORD now = GetTickCount();
    if (!force && now - g_lastCfgCheck < 1000) return;
    g_lastCfgCheck = now;
    long long mt = hb::Config::mtimeOf(IniPath);
    if (mt == -1 && g_cfgMtime == -2) { writeTemplateIni(); mt = hb::Config::mtimeOf(IniPath); }
    if (mt == g_cfgMtime) return;
    hb::Config c;
    if (!c.Load(IniPath)) { logf("cfg: cannot read %s (mtime=%lld, keeping previous)", IniPath, mt); g_cfgMtime = mt; return; }
    g_cfg = c; g_cfgMtime = mt;
    logf("cfg: loaded %s: invoke=%d verbose=%d kbdev=%d tagoff=%x profiles=%d warnings=%d",
         IniPath, g_cfg.general.invoke, g_cfg.general.verbose, g_cfg.general.keyboardDevice, g_cfg.general.creatureTagOffset,
         (int)g_cfg.profiles.size(), (int)g_cfg.warnings.size());
    for (auto& w : g_cfg.warnings) logf("cfg: WARNING %s", w.c_str());
    int dev = g_cfg.general.keyboardDevice;
    g_kbDevice = dev >= 0 ? dev : rdInt(KeyboardDeviceGlobal, 0, 0);
    recomputeBound();
}

// ---- probe: dump every group + merge into the catalog -------------------------------------------------
std::set<std::string> g_catalog;
void appendCatalog(const std::string& line) {
    if (!g_catalog.insert(line).second) return;
    FILE* f = fopen(CatalogPath, "a"); if (!f) return;
    fprintf(f, "%s\n", line.c_str()); fclose(f);
}
unsigned g_probeHash = 0; long g_lastProbeFrame = -100000;

void probe(uintptr_t hud, bool force) {
    unsigned h = 2166136261u; std::vector<std::vector<Entry>> all(GroupCount);
    for (int g = 0; g < GroupCount; ++g) {
        readGroup(hud, g, &all[g]);
        h = (h ^ (unsigned)all[g].size()) * 16777619u;
        for (auto& e : all[g]) h = (h ^ (unsigned)e.key) * 16777619u;
        h = (h ^ (unsigned)rdInt(hud, Hud_GroupSel + g * 4)) * 16777619u;
    }
    bool changed = h != g_probeHash;
    if (!changed && !force && g_frame - g_lastProbeFrame < 600) return;
    g_probeHash = h; g_lastProbeFrame = g_frame;
    logf("probe: frame=%ld char='%s' activeGroup=%d hash=%08x %s", g_frame, g_charKey.c_str(), rdInt(hud, Hud_ActiveGroup), h, changed ? "(changed)" : "(periodic)");
    for (int g = 0; g < GroupCount; ++g) {
        logf("  group %d: count=%d selKey=%08x", g, (int)all[g].size(), (unsigned)rdInt(hud, Hud_GroupSel + g * 4));
        for (auto& e : all[g]) {
            uintptr_t ep = (uintptr_t)rdInt(hud, Hud_GroupLists + g * 12) + (uintptr_t)e.index * EntryStride;
            char raw[300]; int p = 0;
            for (int w = 0; w < EntryStride / 4; ++w) p += snprintf(raw + p, sizeof raw - p, " %08x", (unsigned)rdInt(ep, w * 4));
            logf("    [%d.%d] key=%08x flags=%08x usable=%d why=%d name=\"%s\" icon=\"%s\" ref=%s raw:%s", g, e.index, (unsigned)e.key, (unsigned)e.flags,
                 e.flags & 1, (e.flags >> 1) & 0xF, e.name.c_str(), e.icon.c_str(), hb::Format(e.ref).c_str(), raw);
            if (!e.ref.empty()) appendCatalog(g_charKey + " | " + hb::Format(e.ref) + " | group " + std::to_string(g) + " | " + e.name);
        }
    }
}

// ---- resolver + invoker -------------------------------------------------------------------------------
bool resolve(uintptr_t hud, const hb::ActionRef& want, Entry* out) {
    std::vector<Entry> es;
    for (int g = 0; g < 6; ++g) {
        readGroup(hud, g, &es);
        for (auto& e : es) if (hb::Same(e.ref, want)) { *out = e; return true; }
    }
    return false;
}

void invokeEntry(uintptr_t hud, const Entry& e) {
    int* sel = reinterpret_cast<int*>(hud + Hud_GroupSel + e.group * 4);
    int* active = reinterpret_cast<int*>(hud + Hud_ActiveGroup);
    if (IsBadWritePtr(sel, 4) || IsBadWritePtr(active, 4)) { logf("  invoke: hud selection not writable"); return; }
    logf("  invoke: pre group=%d key=%08x sel=%08x active=%d ownerId=%08x", e.group, (unsigned)e.key, (unsigned)*sel, *active, (unsigned)rdInt((uintptr_t)rdInt(hud, Hud_GroupLists + e.group * 12) + (uintptr_t)e.index * EntryStride, Entry_Owner));
    int oldSel = *sel, oldActive = *active;
    *sel = e.key;
    using Fn = void(__thiscall*)(void*, int, int);
    reinterpret_cast<Fn>(HudInvokeGroup)(reinterpret_cast<void*>(hud), e.group, 0);
    logf("  invoke: returned group=%d key=%08x (sel before=%08x after=%08x active before=%d after=%d)", e.group, (unsigned)e.key, (unsigned)oldSel, (unsigned)*sel, oldActive, *active);
    if (*sel == e.key) { *sel = oldSel; logf("  invoke: selection restored to %08x", (unsigned)oldSel); }
    else logf("  invoke: selection changed by the game to %08x, NOT restoring", (unsigned)*sel);
    *active = oldActive;
}

void drainRing(uintptr_t hud) {
    while (g_ringTail != g_ringHead) {
        Req r = g_ring[g_ringTail]; g_ringTail = (g_ringTail + 1) % 16;
        hb::ActionRef a = r.row == 0 ? g_eff[r.slot] : g_cfg.slotFor(g_charKey, r.row, r.slot);
        std::string sk = hb::SlotKeyName(r.row, r.slot);
        if (a.empty()) { logf("slot %s: unbound now (config changed?)", sk.c_str()); continue; }
        Entry e;
        if (!resolve(hud, a, &e)) {
            logf("slot %s: %s -> entry not available for '%s'; available now:", sk.c_str(), hb::Format(a).c_str(), g_charKey.c_str());
            std::vector<Entry> es;
            for (int g = 0; g < 6; ++g) { readGroup(hud, g, &es); for (auto& x : es) logf("    avail g%d.%d key=%08x %s \"%s\"", g, x.index, (unsigned)x.key, hb::Format(x.ref).c_str(), x.name.c_str()); }
            continue;
        }
        logf("slot %s: %s -> group %d index %d key=%08x flags=%08x usable=%d %s", sk.c_str(), hb::Format(a).c_str(), e.group, e.index,
             (unsigned)e.key, (unsigned)e.flags, e.flags & 1, g_cfg.general.invoke ? "INVOKING" : "(Invoke=0: would invoke)");
        if (g_cfg.general.invoke) invokeEntry(hud, e);
    }
}

// ---- on-screen buttons -----------------------------------------------------------------------------------
// Each slot = two labels: a frame (LBL_HOTBARBG<d>, id 111+s, drawn first) showing "hotbar_slot" / "hotbar_slot_hi" (hover), and
// the icon+digit label (LBL_HOTBAR<d>, id 121+s) on top. Both are hidden when the HUD gate is closed.
using InitCtlFn = void(__thiscall*)(void*, void*, void*, int, int);
using CExoCtorFn = void*(__thiscall*)(void*, const char*);
using CExoDtorFn = void(__thiscall*)(void*);
using LabelCtorFn = void(__fastcall*)(void*);
using NewFn = void*(__cdecl*)(unsigned);
using LoadImageFn = void*(__cdecl*)(const char*);

struct Widget {
    unsigned char* ctl = nullptr;   // 0x148-byte CSWGuiLabel (never freed: the panel dtor does not delete list entries)
    int id = -1;
    float baseAlpha = 1.0f;         // border alpha as loaded from the GFF
    int visible = -1;               // last applied: -1 unknown, 0 hidden, 1 shown
    float alpha = -1.0f;            // last applied border alpha
    std::string icon;               // last applied fill resref ("" = none)
    std::string why;                // last logged reason for hidden/shown
    std::string text;               // last applied runtime text (picker widgets)
};
struct Button { Widget bg, ic; bool avail = false; bool hover = false; Entry entry; };
// ---- in-game editor state (all of it lives in Bar so a HUD rebuild resets it) ----
enum EditMode { M_Normal = 0, M_Edit = 1, M_Pick = 2 };
const char* const ModeName[] = { "Normal", "Edit", "Pick" };
struct Cell { Widget bg, ic; bool avail = false; Entry entry; };
struct Editor {
    int mode = M_Normal, slot = -1, tab = 0, page = 0;
    std::vector<Entry> list; unsigned sig = 0; long lastRefresh = -1000;
    int over = -1; DWORD overStart = 0;
    std::string msg;                  // transient message shown in the title (e.g. a refused save)
    int pbgRect[4] = {0, 0, 0, 0};    // group background rect as bound (pick-mode size, engine px)
    int placed = -1;                  // last applied layout: 0 edit (compact), 1 pick (full)
};
struct Bar {
    uintptr_t panel = 0; int built = 0; long seq = 0; Button b[ButtonCount]; Widget tipBg, tip; int tipSlot = -1; std::string tipText;
    Widget edit, pbg, title, tab[TabCount], prev, next, clear; Cell cell[CellCount]; int editBound = 0; Editor ed;
};
volatile bool g_editOpen = false;    // Edit or Pick mode active (read by the input hook: digits pass through, Esc is swallowed)
volatile bool g_escHit = false;      // set by the input hook when it swallowed an Esc key-down
Bar g_bar;
long g_barSeq = 0;

// one engine image object per icon resref, created lazily and never released (the border only borrows the pointer).
// INVARIANT: the border owns its fill at +0x74 (border dtor 0x00414C60 / fill setter 0x00414840 delete it), and pooled images are shared
// between buttons and HUD instances. Safe only because our labels are never destroyed and the fill setter is never called on them;
// any future cleanup must null border+0x74 before freeing a label.
struct PoolEntry { char resref[17]; void* image; };
constexpr int MaxPoolImages = 256;
PoolEntry g_pool[MaxPoolImages]; int g_poolCount = 0;
void* imageFor(const char* resref) {
    for (int i = 0; i < g_poolCount; ++i) if (strcmp(g_pool[i].resref, resref) == 0) return g_pool[i].image;
    if (g_poolCount >= MaxPoolImages) { logf("img: pool full (%d), cannot load '%s'", g_poolCount, resref); return nullptr; }
    char buf[20] = {}; strncpy(buf, resref, 16);
    void* image = reinterpret_cast<LoadImageFn>(FnLoadImage)(buf);
    PoolEntry& e = g_pool[g_poolCount++]; memset(&e, 0, sizeof e); strncpy(e.resref, resref, 16); e.image = image;
    logf("img: loaded '%s' -> %p (pool %d)", resref, image, g_poolCount);
    return image;
}

char slotDigit(int slot) { return "1234567890"[slot]; }
const char* wname(const Widget& w, const Button& b) { return &w == &b.bg ? "bg" : "ic"; }

// Shown/hidden via ctl+0x48 bit1 (what the panel's Draw loop tests); logs every change.
void setVisible(Button& bt, Widget& b, int slot, bool vis, const char* why) {
    if (!b.ctl) return;
    int fl = 0; if (!readAt(reinterpret_cast<uintptr_t>(b.ctl), Ctl_Flags, &fl)) { logf("btn[%d].%s: flags unreadable, cannot set visible", slot, wname(b, bt)); return; }
    if (b.visible == (vis ? 1 : 0) && b.why == why) return;
    int nf = vis ? (fl | 2) : (fl & ~2);
    if (&b == &bt.bg) nf = vis ? (nf & ~0x20) : (nf | 0x20);   // shown frames are hit-testable so clicks on the bar do not reach the world
    if (nf != fl) *reinterpret_cast<int*>(b.ctl + Ctl_Flags) = nf;
    logf("btn[%d].%s key %c: %s -> %s (%s) flags %08x->%08x", slot, wname(b, bt), slotDigit(slot), b.visible < 0 ? "?" : (b.visible ? "shown" : "hidden"), vis ? "shown" : "hidden", why, (unsigned)fl, (unsigned)nf);
    b.visible = vis ? 1 : 0; b.why = why;
}

void setFill(Button& bt, Widget& b, int slot, const std::string& icon) {
    if (!b.ctl || b.icon == icon) return;
    uintptr_t border = reinterpret_cast<uintptr_t>(b.ctl) + Label_Border;
    void* image = icon.empty() ? nullptr : imageFor(icon.c_str());
    int bvt = rdInt(border, 0);
    if (bvt != (int)BorderVtable) { logf("btn[%d].%s: border vtable %08x != %08x, not setting fill '%s'", slot, wname(b, bt), (unsigned)bvt, (unsigned)BorderVtable, icon.c_str()); return; }
    int old = rdInt(border, Border_Fill);
    *reinterpret_cast<void**>(border + Border_Fill) = image;
    logf("btn[%d].%s key %c: fill '%s' -> '%s' image=%p (old fill %08x) border w=%d h=%d", slot, wname(b, bt), slotDigit(slot), b.icon.c_str(), icon.c_str(), image, (unsigned)old, rdInt(border, Border_W), rdInt(border, Border_H));
    b.icon = icon;
}

void setAlpha(Button& bt, Widget& b, int slot, float a) {
    if (!b.ctl || b.alpha == a) return;
    uintptr_t border = reinterpret_cast<uintptr_t>(b.ctl) + Label_Border;
    if (rdInt(border, 0) != (int)BorderVtable) { logf("btn[%d].%s: border vtable bad, not setting alpha", slot, wname(b, bt)); return; }
    *reinterpret_cast<float*>(border + Border_Alpha) = a;
    logf("btn[%d].%s key %c: alpha %.2f -> %.2f", slot, wname(b, bt), slotDigit(slot), b.alpha, a);
    b.alpha = a;
}

// ---- hover: poll the OS cursor against the frame's pixel rect (labels are click-through, so the game never sees the mouse over them)
HWND g_gameWnd = nullptr;
BOOL CALLBACK enumWnd(HWND h, LPARAM lp) {
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId() && IsWindowVisible(h)) { RECT r; if (GetClientRect(h, &r) && r.right > 320 && r.bottom > 240) { *reinterpret_cast<HWND*>(lp) = h; return FALSE; } }
    return TRUE;
}
bool cursorInGame(POINT* out) {
    if (!g_gameWnd || !IsWindow(g_gameWnd)) {
        static long lastTry = -1000; if (g_frame - lastTry < 120) return false;   // failed lookups retry every 120 frames, not every frame
        lastTry = g_frame;
        g_gameWnd = nullptr; EnumWindows(enumWnd, reinterpret_cast<LPARAM>(&g_gameWnd));
        RECT r = {0, 0, 0, 0}; if (g_gameWnd) GetClientRect(g_gameWnd, &r);
        logf("hover: game window %p client=%ldx%ld", (void*)g_gameWnd, (long)r.right, (long)r.bottom);
        if (!g_gameWnd) return false;
    }
    POINT pt; if (!GetCursorPos(&pt) || !ScreenToClient(g_gameWnd, &pt)) return false;
    *out = pt; return true;
}

// ---- hover tooltip ---------------------------------------------------------------------------------------
hb::TwoDA g_spells; bool g_spellsTried = false;
hb::Tlk g_tlk; bool g_tlkTried = false;

void ensureLookups() {
    if (!g_spellsTried) {
        g_spellsTried = true;
        bool ok = g_spells.Load(Spells2daPath);
        logf("tip: %s -> %s (cols=%d rows=%d%s%s)", Spells2daPath, ok ? "ok" : "FAILED", (int)g_spells.cols.size(), (int)g_spells.rows.size(), ok ? "" : " err=", ok ? "" : g_spells.error.c_str());
    }
    if (!g_tlkTried) {
        g_tlkTried = true;
        bool ok = g_tlk.Open(TlkPath);
        logf("tip: %s -> %s (strings=%u%s%s)", TlkPath, ok ? "ok" : "FAILED", (unsigned)g_tlk.count, ok ? "" : " err=", ok ? "" : g_tlk.error.c_str());
    }
}

std::string nullish(const std::string& s) { return s == "****" ? std::string() : s; }

// greedy word wrap into '\n'-joined lines (at most maxLines, "..." when cut)
std::string wrapText(const std::string& in, int width, int maxLines) {
    std::string out, line; int lines = 0; size_t i = 0;
    while (i < in.size()) {
        size_t e = in.find(' ', i); if (e == std::string::npos) e = in.size();
        std::string w = in.substr(i, e - i); i = e + 1;
        if (!line.empty() && (int)(line.size() + 1 + w.size()) > width) {
            if (++lines >= maxLines) { line += "..."; break; }
            out += line + "\n"; line.clear();
        }
        line += (line.empty() ? "" : " ") + w;
    }
    out += line;
    return out;
}

// Descriptions start with a category/prerequisite header ("Universal Power Restricted By Armor This power ...", "Light Side Power
// Prerequisites: Character Level 6 This power ..."): drop it and keep the first sentence.
std::string shortDesc(const std::string& d) {
    size_t p = d.find(" This "); std::string t = (p != std::string::npos && p < 120) ? d.substr(p + 1) : d;
    size_t e = t.find(". "); if (e != std::string::npos) t.resize(e + 1);
    return t;
}

// Title, key, type, cost and description of slot s's current entry ('\n'-separated lines).
std::string buildTipText(char digit, const Entry& e, bool verboseLog = true) {
    std::string title = e.name; size_t par = title.rfind(" ("); if (par != std::string::npos && par > 0 && title.back() == ')') title.resize(par);
    std::string qty = e.name.size() > title.size() + 3 ? e.name.substr(title.size() + 2, e.name.size() - title.size() - 3) : "";
    std::string out = title + "\n";
    const char* kind = (e.key & 0x40000000) ? "Item" : ((e.key & 0x20000000) ? "Force Power" : "Mode");
    if (digit) out += std::string("Key: ") + digit + "   " + kind + "\n"; else out += std::string(kind) + "\n";
    if ((e.key & 0x20000000) && !(e.key & 0x40000000)) {
        ensureLookups();
        int row = e.key & 0x1FFFFFFF;
        std::string fp = nullish(g_spells.Get(row, "forcepoints")), range = g_spells.Get(row, "range");
        if (!fp.empty() && fp != "0") out += "Force Points: " + fp + "\n";
        if (range == "P") out += "Range: Personal\n"; else if (range == "T") out += "Range: Target\n";
        std::string dsc = nullish(g_spells.Get(row, "spelldesc"));
        if (!dsc.empty()) { std::string d = shortDesc(g_tlk.Get((uint32_t)atoi(dsc.c_str()))); if (!d.empty()) out += wrapText(d, TipWrap, TipMaxDescLines) + "\n"; }
        if (verboseLog) logf("tip: spell row %d fp='%s' range='%s' desc strref='%s'", row, fp.c_str(), range.c_str(), dsc.c_str());
    } else if (e.key & 0x40000000) {
        if (!qty.empty()) out += "Quantity: " + qty + "\n";
    }
    if (!(e.flags & 1)) out += "Not usable right now\n";
    while (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

void setRect(Widget& w, const char* nm, int x, int y, int ww, int hh) {
    if (!w.ctl) return;
    void** vt = *reinterpret_cast<void***>(w.ctl);
    if (IsBadReadPtr(vt + 1, 4)) { logf("tip: %s vtable unreadable", nm); return; }
    int r[4] = { x, y, ww, hh };
    reinterpret_cast<void(__thiscall*)(void*, int*)>(vt[1])(w.ctl, r);
}

void setWidgetVisible(Widget& w, const char* nm, bool vis) {
    int fl = 0; if (!w.ctl || !readAt(reinterpret_cast<uintptr_t>(w.ctl), Ctl_Flags, &fl)) return;
    int nf = vis ? (fl | 2) : (fl & ~2);
    if (nf != fl) *reinterpret_cast<int*>(w.ctl + Ctl_Flags) = nf;
    w.visible = vis ? 1 : 0;
    (void)nm;
}

void hideTip(const char* why) {
    if (g_bar.tipSlot < 0) return;
    logf("tip: hide slot %d (%s)", g_bar.tipSlot, why);
    setWidgetVisible(g_bar.tipBg, "tipbg", false); setWidgetVisible(g_bar.tip, "tip", false);
    g_bar.tipSlot = -1;
}

void showTipText(Widget& anchor, const std::string& text, int s, int topAnchor = -1) {
    if (!g_bar.tip.ctl || !g_bar.tipBg.ctl) { logf("tip: show slot %d skipped: tooltip labels not bound (bg=%p text=%p)", s, (void*)g_bar.tipBg.ctl, (void*)g_bar.tip.ctl); g_bar.tipSlot = s; return; }
    g_bar.tipText = text;
    uintptr_t tc = reinterpret_cast<uintptr_t>(g_bar.tip.ctl), tx = tc + Label_Text;
    if (rdInt(tx, 0) != (int)TextVtable || rdInt(tx, Text_Renderer) == 0) { logf("tip: slot %d: text object bad (vt=%08x renderer=%08x), tooltip skipped", s, (unsigned)rdInt(tx, 0), (unsigned)rdInt(tx, Text_Renderer)); g_bar.tipSlot = s; return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtorFn>(FnExoCtor)(exo, text.c_str());
    reinterpret_cast<void(__thiscall*)(void*, void*)>(FnTextSet)(reinterpret_cast<void*>(tx + Text_StringSub), exo);
    reinterpret_cast<CExoDtorFn>(FnExoDtor)(exo);
    int lines = 1, maxLen = 0, cur = 0;
    for (char c : text) { if (c == '\n') { ++lines; cur = 0; } else if (++cur > maxLen) maxLen = cur; }
    int w = maxLen * TipCharW + 2 * TipPad, h = lines * TipLineH + 2 * TipPad;
    RECT cr = {0, 0, 3840, 1600}; if (g_gameWnd) GetClientRect(g_gameWnd, &cr);
    uintptr_t fc = reinterpret_cast<uintptr_t>(anchor.ctl); int fx = rdInt(fc, 4), fy = rdInt(fc, 8);
    int x = fx, y = fy - h - 12;
    if (topAnchor >= 0) y = topAnchor - h - 10;   // picker tooltips sit above the whole picker so they never cover it
    if (x + w > cr.right - 8) x = cr.right - 8 - w;
    if (x < 8) x = 8;
    if (y < 8) y = 8;
    setRect(g_bar.tipBg, "tipbg", x, y, w, h);
    setRect(g_bar.tip, "tip", x + TipPad / 2, y + TipPad / 2, w - TipPad, h - TipPad);
    setWidgetVisible(g_bar.tipBg, "tipbg", true); setWidgetVisible(g_bar.tip, "tip", true);
    g_bar.tipSlot = s;
    int th = 0; { void** tvt = *reinterpret_cast<void***>(tx); if (!IsBadReadPtr(tvt + 2, 4)) th = reinterpret_cast<int(__fastcall*)(void*)>(tvt[2])(reinterpret_cast<void*>(tx)); }
    logf("tip: show slot %d lines=%d maxLen=%d rect=[%d,%d,%d,%d] client=%ldx%ld textHeight(engine)=%d text=\"%s\"", s, lines, maxLen, x, y, w, h, (long)cr.right, (long)cr.bottom, th, text.c_str());
}

void showTipAt(Widget& anchor, const Entry& entry, char digit, int s) { showTipText(anchor, buildTipText(digit, entry), s); }

// Per frame: which slot (if any) the cursor is over. Only available slots highlight (frame -> "hotbar_slot_hi").
void updateHover(bool gate) {
    static int lastHover = -2; static POINT lastPt = {-1, -1}; static long lastLog = -1000;
    int over = -1; POINT pt = {0, 0};
    if (gate && cursorInGame(&pt)) {
        for (int s = 0; s < ButtonCount && over < 0; ++s) {
            const Widget& w = g_bar.b[s].bg; if (!w.ctl || !w.visible) continue;
            uintptr_t c = reinterpret_cast<uintptr_t>(w.ctl); int x = rdInt(c, 4), y = rdInt(c, 8), ww = rdInt(c, 0xC), hh = rdInt(c, 0x10);
            if (pt.x >= x && pt.x < x + ww && pt.y >= y && pt.y < y + hh) over = s;
        }
    }
    static DWORD hoverStart = 0;
    if (over != lastHover) { logf("hover: slot %d -> %d cursor=(%ld,%ld)", lastHover, over, (long)pt.x, (long)pt.y); lastHover = over; hoverStart = GetTickCount(); hideTip("hover changed"); }
    else if (g_cfg.general.verbose && (pt.x != lastPt.x || pt.y != lastPt.y) && pt.y > 1100 && g_frame - lastLog > 120) { lastLog = g_frame; logf("hover: cursor=(%ld,%ld) over=%d", (long)pt.x, (long)pt.y, over); }
    lastPt = pt;
    for (int s = 0; s < ButtonCount; ++s) {
        Button& b = g_bar.b[s]; bool hov = (s == over) && (b.avail || g_bar.ed.mode != M_Normal); b.hover = hov;
        bool selected = g_bar.ed.mode == M_Pick && g_bar.ed.slot == s;   // slot being edited stays highlighted
        if (b.bg.ctl && b.bg.visible) setFill(b, b.bg, s, (hov || selected) ? FrameHiResref : FrameResref);
    }
    {   // left click on an available slot = same request as its digit key
        static bool prevDown = false;
        bool down = gate && (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        if (down && !prevDown && over >= 0 && g_bar.ed.mode != M_Normal) logf("click: slot %d in %s mode -> handled by the editor, not invoked", over, ModeName[g_bar.ed.mode]);
        else if (down && !prevDown && over >= 0) {
            bool ok = g_bar.b[over].avail;
            logf("click: slot %d %s", over, ok ? "queued" : "ignored (no action)");
            if (ok) { int next = (g_ringHead + 1) % 16; if (next == g_ringTail) logf("click: ring full, dropped"); else { g_ring[g_ringHead] = { 0, over }; g_ringHead = next; } }
        }
        prevDown = down;
    }
    if (over >= 0 && g_bar.b[over].avail && g_bar.tipSlot != over && GetTickCount() - hoverStart >= TipDelayMs) showTipAt(g_bar.b[over].bg, g_bar.b[over].entry, slotDigit(over), over);
    if ((over < 0 || !g_bar.b[over].avail) && g_bar.tipSlot < CellTipBase) hideTip("no hover");
}

// Per-tick button state. gate = HUD is the active panel. Entries are re-resolved every ButtonRefreshFrames frames or when
// the cached view is stale (config/character/gate change), never per frame.
void updateButtons(uintptr_t hud, bool gate) {
    static uintptr_t lastMissingHud = 0; static long lastRefresh = -1000; static bool lastGateSeen = false; static std::string lastChar = "\x01"; static int lastMode = -1;
    if (g_bar.panel != hud || g_bar.built == 0) {
        if (lastMissingHud != hud) { lastMissingHud = hud; logf("buttons: no usable bar for hud %08x (bar panel=%08x built=%d/%d) -- HUD ctor hook did not run for it, or no button bound", (unsigned)hud, (unsigned)g_bar.panel, g_bar.built, ButtonCount); }
        return;
    }
    bool stale = g_frame - lastRefresh >= ButtonRefreshFrames || gate != lastGateSeen || g_charKey != lastChar || g_bar.ed.mode != lastMode;
    if (!stale) return;
    lastRefresh = g_frame; lastGateSeen = gate; lastChar = g_charKey; lastMode = g_bar.ed.mode;
    for (int s = 0; s < ButtonCount; ++s) {
        Button& b = g_bar.b[s];
        if (!b.bg.ctl && !b.ic.ctl) continue;
        if (!gate) { b.avail = false; setVisible(b, b.bg, s, false, "hud not active"); setVisible(b, b.ic, s, false, "hud not active"); continue; }
        // frame + digit are always shown while the HUD is active (an empty slot reads as a dim frame)
        hb::ActionRef want = g_eff[s];
        if (want.empty() && g_bar.ed.mode == M_Normal) {   // unassigned slots are invisible during play; edit mode shows them so they can be filled
            b.avail = false; setVisible(b, b.bg, s, false, "unassigned"); setVisible(b, b.ic, s, false, "unassigned"); continue;
        }
        Entry e; const char* state; bool have = false;
        if (want.empty()) state = "unbound";
        else if (!resolve(hud, want, &e)) state = "entry not available";
        else { have = true; state = (e.flags & 1) ? "usable" : "unusable (dimmed)"; }
        b.avail = have; if (have) b.entry = e;
        if (have && g_bar.tipSlot == s && buildTipText(slotDigit(s), e, false) != g_bar.tipText) { logf("tip: slot %d entry changed while hovered, refreshing", s); showTipAt(b.bg, e, slotDigit(s), s); }
        setVisible(b, b.bg, s, true, state); setVisible(b, b.ic, s, true, state);
        setAlpha(b, b.bg, s, have ? b.bg.baseAlpha : b.bg.baseAlpha * EmptyFrameAlpha);
        setFill(b, b.ic, s, have ? e.icon : std::string());
        setAlpha(b, b.ic, s, (have && !(e.flags & 1)) ? b.ic.baseAlpha * DimAlpha : b.ic.baseAlpha);
    }
}


// ---- generic widget helpers for the editor controls (log every change) -------------------------------------
bool textObjOk(Widget& w) { uintptr_t tx = reinterpret_cast<uintptr_t>(w.ctl) + Label_Text; return rdInt(tx, 0) == (int)TextVtable && rdInt(tx, Text_Renderer) != 0; }
void wVisible(Widget& w, const char* nm, bool vis, bool hit) {
    if (!w.ctl) return;
    int fl = 0; if (!readAt(reinterpret_cast<uintptr_t>(w.ctl), Ctl_Flags, &fl)) { logf("edit: %s flags unreadable", nm); return; }
    int nf = vis ? (fl | 2) : (fl & ~2);
    nf = (vis && hit) ? (nf & ~0x20) : (nf | 0x20);   // hit-testable only while shown, so hidden controls never swallow clicks
    int v = vis ? 1 : 0;
    if (w.visible == v && nf == fl) return;
    *reinterpret_cast<int*>(w.ctl + Ctl_Flags) = nf;
    logf("edit: %s %s -> %s flags %08x->%08x", nm, w.visible < 0 ? "?" : (w.visible ? "shown" : "hidden"), vis ? "shown" : "hidden", (unsigned)fl, (unsigned)nf);
    w.visible = v;
}
void wFill(Widget& w, const char* nm, const std::string& icon) {
    if (!w.ctl || w.icon == icon) return;
    uintptr_t border = reinterpret_cast<uintptr_t>(w.ctl) + Label_Border;
    if (rdInt(border, 0) != (int)BorderVtable) { logf("edit: %s border vtable bad, not setting fill '%s'", nm, icon.c_str()); return; }
    void* image = icon.empty() ? nullptr : imageFor(icon.c_str());
    *reinterpret_cast<void**>(border + Border_Fill) = image;
    logf("edit: %s fill '%s' -> '%s' image=%p", nm, w.icon.c_str(), icon.c_str(), image);
    w.icon = icon;
}
void wAlpha(Widget& w, const char* nm, float a) {
    if (!w.ctl || w.alpha == a) return;
    uintptr_t border = reinterpret_cast<uintptr_t>(w.ctl) + Label_Border;
    if (rdInt(border, 0) != (int)BorderVtable) { logf("edit: %s border vtable bad, not setting alpha", nm); return; }
    *reinterpret_cast<float*>(border + Border_Alpha) = a;
    logf("edit: %s alpha %.2f -> %.2f", nm, w.alpha, a);
    w.alpha = a;
}
void wText(Widget& w, const char* nm, const std::string& text) {
    if (!w.ctl || w.text == text) return;
    if (!textObjOk(w)) { logf("edit: %s text object bad, cannot set \"%s\"", nm, text.c_str()); return; }
    uintptr_t tx = reinterpret_cast<uintptr_t>(w.ctl) + Label_Text;
    char exo[16] = {0};
    reinterpret_cast<CExoCtorFn>(FnExoCtor)(exo, text.c_str());
    reinterpret_cast<void(__thiscall*)(void*, void*)>(FnTextSet)(reinterpret_cast<void*>(tx + Text_StringSub), exo);
    reinterpret_cast<CExoDtorFn>(FnExoDtor)(exo);
    logf("edit: %s text \"%s\" -> \"%s\"", nm, w.text.c_str(), text.c_str());
    w.text = text;
}
bool wHit(const Widget& w, const POINT& pt) {
    if (!w.ctl || w.visible != 1) return false;
    uintptr_t c = reinterpret_cast<uintptr_t>(w.ctl); int x = rdInt(c, 4), y = rdInt(c, 8), ww = rdInt(c, 0xC), hh = rdInt(c, 0x10);
    return pt.x >= x && pt.x < x + ww && pt.y >= y && pt.y < y + hh;
}

// ---- in-game editor: Normal -> (toggle) Edit -> (slot click) Pick -> (cell / Clear) assign+save -> Edit; Esc backs out one level ----
// ---- mouse wheel over the picker: subclass the game window like Mod 1 (DirectInput never sees the wheel). Chains to the old proc; never restored.
WNDPROC g_oldProc = nullptr; volatile LONG g_wheelAccum = 0; volatile LONG g_pickRect[4] = {0, 0, 0, 0}; int g_wheelLogs = 0;
LRESULT CALLBACK wheelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_MOUSEWHEEL && g_editOpen && g_pickRect[2] > 0 && GetTickCount() - g_hudTick < 300) {
        POINT pt = {(short)LOWORD(l), (short)HIWORD(l)};
        bool inside = ScreenToClient(h, &pt) && pt.x >= g_pickRect[0] && pt.x < g_pickRect[0] + g_pickRect[2] && pt.y >= g_pickRect[1] && pt.y < g_pickRect[1] + g_pickRect[3];
        if (g_wheelLogs < 40) { ++g_wheelLogs; logf("wheel: delta=%d client=(%ld,%ld) rect=[%ld,%ld,%ld,%ld] inside=%d", (int)(short)HIWORD(w), (long)pt.x, (long)pt.y, (long)g_pickRect[0], (long)g_pickRect[1], (long)g_pickRect[2], (long)g_pickRect[3], (int)inside); }
        if (inside) { InterlockedExchangeAdd(&g_wheelAccum, (short)HIWORD(w)); return 0; }
    }
    return CallWindowProcA(g_oldProc, h, m, w, l);
}
void ensureWheelHook() {
    if (g_oldProc || !g_gameWnd || !IsWindow(g_gameWnd)) return;
    g_oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_gameWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wheelProc)));
    logf("wheel: subclassed window %p, old proc %p (err=%lu)", (void*)g_gameWnd, (void*)g_oldProc, g_oldProc ? 0UL : GetLastError());
}
// Group background + title: full size in Pick mode; in Edit mode a compact box (title only) hugging the bar. Sizes in units -> px via the
// background's own height (u px per unit; x and y scale equally after the aspect fix).
void moveWidget(Widget& w, const char* nm, int x, int y, int ww, int hh) {
    if (!w.ctl) return;
    setRect(w, nm, x, y, ww, hh);
    logf("edit: %s moved to [%d,%d,%d,%d]", nm, x, y, ww, hh);
}
void placePanel(bool pick) {
    Editor& ed = g_bar.ed;
    if (ed.placed == (pick ? 1 : 0) || ed.pbgRect[3] <= 0) return;
    const int* R = ed.pbgRect; double u = R[3] / PickBoxUnits; int pad = (int)(BoxPadUnits * u + 0.5), th = (int)(TitleUnits * u + 0.5);
    int boxH = pick ? R[3] : (int)(EditBoxUnits * u + 0.5), boxY = R[1] + R[3] - boxH;
    moveWidget(g_bar.pbg, "pbg", R[0], boxY, R[2], boxH);
    moveWidget(g_bar.title, "title", R[0] + pad, boxY + pad, R[2] - 2 * pad, th);
    ed.placed = pick ? 1 : 0;
    g_pickRect[0] = R[0]; g_pickRect[1] = boxY; g_pickRect[2] = R[2]; g_pickRect[3] = boxH;
}

void setMode(int m, int slot, const char* why) {
    Editor& ed = g_bar.ed;
    logf("edit: mode %s -> %s slot %d -> %d (%s) char='%s'", ModeName[ed.mode], ModeName[m], ed.slot, slot, why, g_charKey.c_str());
    if (m == M_Pick && ed.mode != M_Pick) { ed.page = 0; ed.sig = 0; ed.lastRefresh = -1000; g_wheelAccum = 0; }
    if (m != M_Pick) { ed.over = -1; if (g_bar.tipSlot >= CellTipBase) hideTip("picker closed"); }
    ed.mode = m; ed.slot = slot; ed.msg.clear();
    g_editOpen = (m != M_Normal);
    if (m != M_Normal) placePanel(m == M_Pick); else { g_pickRect[2] = 0; ed.placed = -1; }
}

int tabOf(const Entry& e) { return (e.key & 0x40000000) ? 1 : ((e.key & 0x20000000) ? 0 : 2); }
const char* const TabName[TabCount] = { "Power", "Item", "Mode" };

// Writes (or, with e == nullptr, clears) hotbar slot `slot` of the controlled character's profile ([Profile.obj<id>]) and saves the INI.
// The key is the creature object id (Exile objfffffffd is stable; the invalid id objffffffff seen during loads is treated as 'unknown').
bool assignSlot(uintptr_t hud, int slot, const Entry* e) {
    Editor& ed = g_bar.ed;
    checkConfig(true);   // pick up a hand edit made since the last poll before we overwrite the file
    if (!g_cfg.warnings.empty()) {
        logf("edit: assign REFUSED: config has %d warning(s); saving would drop the lines they concern", (int)g_cfg.warnings.size());
        ed.msg = "Config file has warnings - not saving (see log)"; return false;
    }
    if (g_charKey.empty()) { logf("edit: assign REFUSED: character key unknown (load in progress?)"); ed.msg = "Character not identified yet - try again"; return false; }
    hb::Profile& d = g_cfg.profile(g_charKey);   // per-character profile; slotFor falls back to Default for slots it leaves unset
    int frozen = 0;
    for (int i = 0; i < hb::Slots; ++i) if (d.slots[0][i].empty() && !d.slots[0][i].blank && !g_eff[i].empty()) { d.slots[0][i] = g_eff[i]; d.slots[0][i].blank = false; ++frozen; }
    hb::ActionRef a; if (e) a = e->ref; else a.blank = true;
    if (e && a.empty()) { logf("edit: assign REFUSED: entry '%s' has no stable ref", e->name.c_str()); ed.msg = "That entry cannot be bound"; return false; }
    g_cfg.SetSlot(g_charKey, 0, slot, a, e ? e->name : "cleared");
    static bool backedUp = false;
    if (!backedUp) { backedUp = true; BOOL ok = CopyFileA(IniPath, "force_power_hotbar.ini.bak", FALSE); logf("edit: backup %s -> force_power_hotbar.ini.bak: %s", IniPath, ok ? "ok" : "FAILED (no ini yet?)"); }
    bool ok = g_cfg.Save(IniPath);
    long long mt = hb::Config::mtimeOf(IniPath); if (ok) g_cfgMtime = mt;
    logf("edit: assign slot %d (%s) := %s; froze %d autofilled slot(s) into the profile; character '%s'; Save %s (mtime %lld)", slot, hb::SlotKeyName(0, slot).c_str(),
         e ? hb::Format(a).c_str() : "none", frozen, g_charKey.c_str(), ok ? "ok" : "FAILED", mt);
    if (!ok) { ed.msg = "Saving the config failed (see log)"; return false; }
    recomputeBound(); refreshEffective(hud);
    return true;
}

void updateEditor(uintptr_t hud, bool gate) {
    Editor& ed = g_bar.ed;
    if (g_bar.panel != hud || g_bar.built == 0 || !g_bar.editBound) return;
    static std::string lastChar = "\x01"; static bool prevDown = false; static bool lastFgOk = true;
    ensureWheelHook();
    if (lastChar != g_charKey) { if (lastChar != "\x01" && ed.mode != M_Normal) setMode(M_Normal, -1, "character changed"); lastChar = g_charKey; }
    if (!gate && ed.mode != M_Normal) setMode(M_Normal, -1, "gate closed");
    if (g_escHit) { g_escHit = false; if (ed.mode == M_Pick) setMode(M_Edit, -1, "Esc"); else if (ed.mode == M_Edit) setMode(M_Normal, -1, "Esc"); else logf("edit: Esc flag set in Normal mode (ignored)"); }

    POINT pt = {0, 0}; bool haveCur = gate && cursorInGame(&pt);
    HWND fg = GetForegroundWindow(); DWORD fgPid = 0; if (fg) GetWindowThreadProcessId(fg, &fgPid);
    bool fgOk = fg && fgPid == GetCurrentProcessId();
    if (fgOk != lastFgOk) { logf("edit: foreground window %p pid=%lu ours=%d", (void*)fg, (unsigned long)fgPid, (int)fgOk); lastFgOk = fgOk; }
    bool down = gate && fgOk && (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    bool click = down && !prevDown; prevDown = down;

    // ---- visibility of the always-present toggle and the mode-dependent parts
    bool showTitle = gate && ed.mode != M_Normal, showPick = gate && ed.mode == M_Pick;
    wVisible(g_bar.edit, "toggle", gate, true);
    wFill(g_bar.edit, "toggle", ed.mode == M_Normal ? EditResref : EditHiResref);
    wVisible(g_bar.pbg, "pbg", showTitle, true);
    wVisible(g_bar.title, "title", showTitle, true);
    for (int i = 0; i < TabCount; ++i) wVisible(g_bar.tab[i], "tab", showPick, true);
    wVisible(g_bar.prev, "prev", showPick, true); wVisible(g_bar.next, "next", showPick, true); wVisible(g_bar.clear, "clear", showPick, true);
    for (int i = 0; i < CellCount; ++i) { wVisible(g_bar.cell[i].bg, "cell.bg", showPick, true); wVisible(g_bar.cell[i].ic, "cell.ic", showPick, false); }

    // ---- hover over the tabs / arrows / Clear (highlight + short tooltip)
    Widget* ow[6] = { &g_bar.tab[0], &g_bar.tab[1], &g_bar.tab[2], &g_bar.prev, &g_bar.next, &g_bar.clear };
    static const char* const otip[6] = { "Force powers", "Items: medpacs, stims, shields, mines", "Modes and stances", "Previous page (mouse wheel up)", "Next page (mouse wheel down)", "Empty this slot" };
    int oth = -1;
    if (ed.mode == M_Pick && haveCur) for (int i = 0; i < 6 && oth < 0; ++i) if (wHit(*ow[i], pt)) oth = i;
    {
        static int lastOth = -1; static DWORD othSince = 0;
        if (oth != lastOth) { logf("edit: control hover %d -> %d", lastOth, oth); if (g_bar.tipSlot >= OtherTipBase) hideTip("control hover changed"); lastOth = oth; othSince = GetTickCount(); }
        if (oth >= 0 && g_bar.tipSlot != OtherTipBase + oth && GetTickCount() - othSince >= TipDelayMs) showTipText(*ow[oth], otip[oth], OtherTipBase + oth, g_pickRect[1]);
    }

    // ---- picker contents (rebuilt from the HUD's group lists when stale)
    int pages = 1;
    if (ed.mode == M_Pick) {
        std::vector<Entry> all; unsigned h = 2166136261u;
        for (int g = 0; g < 6; ++g) {
            std::vector<Entry> es; readGroup(hud, g, &es);
            for (auto& x : es) {
                if (tabOf(x) != ed.tab || x.ref.empty()) continue;
                bool dup = false; for (auto& y : all) if (hb::Same(y.ref, x.ref)) dup = true;
                if (dup) continue;
                all.push_back(x); h = (h ^ (unsigned)x.key ^ ((unsigned)x.flags << 3)) * 16777619u;
            }
        }
        h = (h ^ (unsigned)ed.tab ^ ((unsigned)ed.page << 4)) * 16777619u;
        if (h != ed.sig || g_frame - ed.lastRefresh >= 60) {
            if (h != ed.sig) logf("edit: picker list tab=%s page=%d entries=%d (was %d) slot=%d", TabName[ed.tab], ed.page, (int)all.size(), (int)ed.list.size(), ed.slot);
            ed.sig = h; ed.lastRefresh = g_frame; ed.list = all;
        }
        pages = std::max(1, ((int)ed.list.size() + CellCount - 1) / CellCount);
        if (ed.page >= pages) { logf("edit: page %d clamped to %d (pages=%d)", ed.page, pages - 1, pages); ed.page = pages - 1; }
        // wheel over the picker pages through the list (positive delta = wheel up = previous page)
        LONG acc = InterlockedExchange(&g_wheelAccum, 0);
        if (acc) {
            int steps = acc / 120; if (!steps) steps = acc > 0 ? 1 : -1;
            int np = std::max(0, std::min(pages - 1, ed.page - steps));
            logf("edit: wheel acc=%ld steps=%d page %d -> %d (pages=%d)", (long)acc, steps, ed.page, np, pages);
            ed.page = np;
        }
        char t[96]; snprintf(t, sizeof t, "Slot %c - %s %d/%d - Esc: back", slotDigit(ed.slot), TabName[ed.tab], ed.page + 1, pages);
        wText(g_bar.title, "title", ed.msg.empty() ? std::string(t) : ed.msg);
        for (int i = 0; i < TabCount; ++i) wFill(g_bar.tab[i], "tab", (i == ed.tab || oth == i) ? FrameHiResref : FrameResref);
        wFill(g_bar.prev, "prev", oth == 3 ? FrameHiResref : FrameResref); wFill(g_bar.next, "next", oth == 4 ? FrameHiResref : FrameResref); wFill(g_bar.clear, "clear", oth == 5 ? FrameHiResref : FrameResref);
        wAlpha(g_bar.prev, "prev", ed.page > 0 ? 1.0f : 0.4f); wAlpha(g_bar.next, "next", ed.page + 1 < pages ? 1.0f : 0.4f);
        for (int i = 0; i < CellCount; ++i) {
            Cell& c = g_bar.cell[i]; size_t idx = (size_t)ed.page * CellCount + i;
            c.avail = idx < ed.list.size(); if (c.avail) c.entry = ed.list[idx];
            bool hov = ed.over == i && c.avail;
            wFill(c.bg, "cell.bg", hov ? FrameHiResref : FrameResref);
            wAlpha(c.bg, "cell.bg", c.avail ? 1.0f : EmptyFrameAlpha);
            wFill(c.ic, "cell.ic", c.avail ? c.entry.icon : std::string());
            wAlpha(c.ic, "cell.ic", (c.avail && !(c.entry.flags & 1)) ? DimAlpha : 1.0f);
        }
    } else if (ed.mode == M_Edit) {
        wText(g_bar.title, "title", ed.msg.empty() ? std::string("Click a slot to change it") : ed.msg);
    }

    // ---- picker hover (cells) + tooltip
    int over = -1;
    if (ed.mode == M_Pick && haveCur) for (int i = 0; i < CellCount && over < 0; ++i) if (g_bar.cell[i].avail && wHit(g_bar.cell[i].bg, pt)) over = i;
    if (over != ed.over) { logf("edit: cell hover %d -> %d", ed.over, over); if (g_bar.tipSlot >= CellTipBase && g_bar.tipSlot < ToggleTipId) hideTip("cell hover changed"); ed.over = over; ed.overStart = GetTickCount(); }
    if (over >= 0 && g_bar.tipSlot != CellTipBase + over && GetTickCount() - ed.overStart >= TipDelayMs) showTipText(g_bar.cell[over].bg, buildTipText(0, g_bar.cell[over].entry), CellTipBase + over, g_pickRect[1]);

    // ---- toggle hover tooltip
    {
        static bool wasOver = false; static DWORD since = 0;
        bool ov = haveCur && wHit(g_bar.edit, pt);
        if (ov != wasOver) { logf("edit: toggle hover %d -> %d", (int)wasOver, (int)ov); wasOver = ov; since = GetTickCount(); if (!ov && g_bar.tipSlot == ToggleTipId) hideTip("left toggle"); }
        if (ov && g_bar.tipSlot != ToggleTipId && GetTickCount() - since >= TipDelayMs) showTipText(g_bar.edit, ed.mode == M_Normal ? "Edit hotbar" : "Finish editing", ToggleTipId);
    }

    // ---- clicks
    if (!click || !haveCur) return;
    if (wHit(g_bar.edit, pt)) { setMode(ed.mode == M_Normal ? M_Edit : M_Normal, -1, "toggle click"); return; }
    if (ed.mode == M_Normal) return;
    for (int s = 0; s < ButtonCount; ++s) if (wHit(g_bar.b[s].bg, pt)) { setMode(M_Pick, s, "slot click"); return; }
    if (ed.mode != M_Pick) return;
    for (int i = 0; i < TabCount; ++i) if (wHit(g_bar.tab[i], pt)) { logf("edit: tab click %d (%s)", i, TabName[i]); ed.tab = i; ed.page = 0; ed.sig = 0; ed.msg.clear(); return; }
    if (wHit(g_bar.prev, pt)) { if (ed.page > 0) { --ed.page; ed.sig = 0; logf("edit: page -> %d/%d", ed.page + 1, pages); } else logf("edit: prev click ignored (first page)"); return; }
    if (wHit(g_bar.next, pt)) { if (ed.page + 1 < pages) { ++ed.page; ed.sig = 0; logf("edit: page -> %d/%d", ed.page + 1, pages); } else logf("edit: next click ignored (last page)"); return; }
    if (wHit(g_bar.clear, pt)) { logf("edit: Clear click slot %d", ed.slot); if (assignSlot(hud, ed.slot, nullptr)) setMode(M_Edit, -1, "cleared"); return; }
    for (int i = 0; i < CellCount; ++i) if (wHit(g_bar.cell[i].bg, pt)) {
        if (!g_bar.cell[i].avail) { logf("edit: click on empty cell %d ignored", i); return; }
        Entry chosen = g_bar.cell[i].entry;
        logf("edit: cell %d click -> slot %d := group %d key=%08x %s \"%s\"", i, ed.slot, chosen.group, (unsigned)chosen.key, hb::Format(chosen.ref).c_str(), chosen.name.c_str());
        if (assignSlot(hud, ed.slot, &chosen)) setMode(M_Edit, -1, "assigned");
        return;
    }
    logf("edit: click at (%ld,%ld) in Pick mode hit nothing", (long)pt.x, (long)pt.y);
}

void dumpMem(const char* tag, uintptr_t base, int dwords) {
    char line[200];
    for (int i = 0; i < dwords; i += 8) {
        int p = 0; p += snprintf(line + p, sizeof line - p, "%s+%03x:", tag, i * 4);
        for (int j = i; j < i + 8 && j < dwords; ++j) p += snprintf(line + p, sizeof line - p, " %08x", (unsigned)rdInt(base, j * 4));
        logf("%s", line);
    }
}

// Allocates, constructs and binds one label by tag (see hook 3 below).
// With the projection flag cleared InitControl scales x by W/800 and y by H/600 (so 3840x1600 stretches x by 1.8). The HUD is drawn at
// a uniform H/600 px per unit, so squeeze x/width by (H/600)/(W/800). Returns the factor (1 = unknown, leave alone).
double aspectFix() {
    // NOT FnScreenInfo: FUN_0073FEA0 is a thiscall that reads [this+0x274]; calling it as cdecl passes a garbage/null ECX and crashed on load
    // (build 8, null read at 0x73FEAF). The window client size is what the engine renders at.
    short w = 0, h = 0; int rw = rdInt(GuiRefW, 0), rh = rdInt(GuiRefH, 0);
    HWND win = nullptr; EnumWindows(enumWnd, reinterpret_cast<LPARAM>(&win));
    RECT cr = {0, 0, 0, 0}; if (win) GetClientRect(win, &cr);
    w = (short)cr.right; h = (short)cr.bottom;
    double c = (w > 0 && h > 0 && rw > 0 && rh > 0) ? ((double)h / rh) / ((double)w / rw) : 1.0;
    logf("H3: window %p client=%dx%d ref=%dx%d -> x correction %.4f", (void*)win, (int)w, (int)h, rw, rh, c);
    return c;
}
void fixRect(unsigned char* ctl, const char* tag, double c) {
    if (c == 1.0) return;
    uintptr_t cc = reinterpret_cast<uintptr_t>(ctl);
    int r[4]; for (int i = 0; i < 4; ++i) r[i] = rdInt(cc, 4 + 4 * i);
    int n[4] = { (int)(r[0] * c + 0.5), r[1], (int)(r[2] * c + 0.5), r[3] };
    void** vt = *reinterpret_cast<void***>(ctl);
    if (IsBadReadPtr(vt + 1, 4)) { logf("H3 %s: vtable unreadable, no aspect fix", tag); return; }
    reinterpret_cast<void(__thiscall*)(void*, int*)>(vt[1])(ctl, n);
    logf("H3 %s: aspect fix rect [%d,%d,%d,%d] -> [%d,%d,%d,%d]", tag, r[0], r[1], r[2], r[3], n[0], n[1], n[2], n[3]);
}
unsigned char* bindLabel(uintptr_t panel, const char* tagName, int expectId, Widget* w, bool dump, double aspect) {
    unsigned char* ctl = static_cast<unsigned char*>(reinterpret_cast<NewFn>(FnNew)(LabelSize));
    if (!ctl) { logf("H3 %s: allocation failed", tagName); return nullptr; }
    memset(ctl, 0, LabelSize);
    reinterpret_cast<LabelCtorFn>(FnLabelCtor)(ctl);
    int vt = rdInt(reinterpret_cast<uintptr_t>(ctl), 0);
    if (vt != (int)LabelVtable) { logf("H3 %s: ctor left vtable %08x (expected %08x), not binding", tagName, (unsigned)vt, (unsigned)LabelVtable); return nullptr; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtorFn>(FnExoCtor)(exo, tagName);
    reinterpret_cast<InitCtlFn>(FnInitControl)(reinterpret_cast<void*>(panel), ctl, exo, 1, 1);
    reinterpret_cast<CExoDtorFn>(FnExoDtor)(exo);
    if (rdInt(reinterpret_cast<uintptr_t>(ctl), Ctl_Id, -999) == expectId) fixRect(ctl, tagName, aspect);
    uintptr_t c = reinterpret_cast<uintptr_t>(ctl);
    int id = rdInt(c, Ctl_Id, -999), fl = rdInt(c, Ctl_Flags);
    int ext[4]; for (int i = 0; i < 4; ++i) ext[i] = rdInt(c, 4 + 4 * i);
    uintptr_t border = c + Label_Border; float ba = 1.0f; readAt(border, Border_Alpha, &ba);
    logf("H3 %s: ctl=%p id=%d (expect %d) rect=[%d,%d,%d,%d] flags=%08x border vt=%08x w=%d h=%d fill=%08x alpha=%.2f",
         tagName, ctl, id, expectId, ext[0], ext[1], ext[2], ext[3], (unsigned)fl, (unsigned)rdInt(border, 0), rdInt(border, Border_W), rdInt(border, Border_H), (unsigned)rdInt(border, Border_Fill), ba);
    if (dump) dumpMem(tagName, c, LabelSize / 4);
    // hidden until the tick decides. Bit5 (0x20) makes the label's hit-test (0x00418D20) reject the mouse, so the row never
    // swallows clicks. Done BEFORE the id check: InitControl already put the label in the panel list, so a wrong/missing id must
    // still leave it invisible and click-through.
    *reinterpret_cast<int*>(ctl + Ctl_Flags) = (fl & ~2) | 0x20;
    if (id != expectId) { logf("H3 %s: GUI control missing or wrong id (%d) -- run scripts/add_hotbar_buttons_gui.py; label hidden, slot part unusable", tagName, id); return nullptr; }
    w->ctl = ctl; w->id = id; w->baseAlpha = ba; w->visible = 0; w->why = "built";
    return ctl;
}

} // namespace

// ---- hook 1: keyboard capture -------------------------------------------------------------------------
struct DIRecord { DWORD dwOfs, dwData; };

extern "C" void __cdecl CaptureHotbarKey(DIRecord* rec, int dev) {
    if (!rec) return;
    bool logIt = g_capLogged < 300;
    if (dev != g_kbDevice) { if (logIt) { ++g_capLogged; logf("cap: non-kbd dev=%d ofs=%02x data=%08x (kbdev=%d)", dev, (unsigned)rec->dwOfs, (unsigned)rec->dwData, g_kbDevice); } return; }
    DWORD dik = rec->dwOfs; bool down = (rec->dwData & 0x80) != 0;
    bool isMod = true;
    switch (dik) {
    case 0x1D: case 0x9D: g_ctrl = down; break;
    case 0x38: case 0xB8: g_alt = down; break;
    case 0x2A: case 0x36: g_shift = down; break;
    default: isMod = false;
    }
    bool isDigit = dik >= 0x02 && dik <= 0x0B, isEsc = dik == 0x01;
    if (isDigit && down) {   // resync modifiers that missed their key-up (Alt-Tab, modal): only ever CLEAR a stuck flag
        bool c = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0, a = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0, sh = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        if ((g_ctrl && !c) || (g_alt && !a) || (g_shift && !sh)) { logf("cap: stuck modifier cleared (ctrl %d->%d alt %d->%d shift %d->%d)", (int)g_ctrl, (int)(g_ctrl && c), (int)g_alt, (int)(g_alt && a), (int)g_shift, (int)(g_shift && sh)); g_ctrl = g_ctrl && c; g_alt = g_alt && a; g_shift = g_shift && sh; }
    }
    const char* decision = "pass";
    if (isEsc) {   // while the editor is open Esc backs out of it instead of opening the game menu
        if (!down) { if (g_downSwallowed[dik]) { g_downSwallowed[dik] = false; rec->dwOfs = 0; rec->dwData = 0; decision = "swallow-up (edit)"; } }
        else if (g_editOpen && g_hudGateOk && (GetTickCount() - g_hudTick) < 250) { g_downSwallowed[dik] = true; g_escHit = true; rec->dwOfs = 0; rec->dwData = 0; decision = "SWALLOW (edit Esc)"; }
        else decision = "pass(edit closed)";
    }
    if (isDigit) {
        if (!down) {
            if (g_downSwallowed[dik]) { g_downSwallowed[dik] = false; rec->dwOfs = 0; rec->dwData = 0; decision = "swallow-up"; }
        } else if (!g_shift) {
            int row = g_alt ? 2 : (g_ctrl ? 1 : 0), slot = (int)dik - hb::kFirstDik;
            bool active = g_hudGateOk && (GetTickCount() - g_hudTick) < 250;
            if (!active) decision = "pass(hud inactive)";
            else if (g_editOpen) decision = "pass(edit mode: never invoke)";
            else if (!g_slotBound[row][slot]) decision = "pass(unbound)";
            else {
                int next = (g_ringHead + 1) % 16;   // a bound key is ALWAYS swallowed: passing it on would switch party members (keys 1-3)
                g_downSwallowed[dik] = true; rec->dwOfs = 0; rec->dwData = 0;
                if (next == g_ringTail) decision = "SWALLOW(ring full, request dropped)";
                else { g_ring[g_ringHead] = { row, slot }; g_ringHead = next; decision = "SWALLOW"; }
            }
        } else decision = "pass(shift)";
    }
    if (logIt || isDigit || isMod || isEsc) { ++g_capLogged; logf("cap: dev=%d dik=%02x %s mods=%c%c%c -> %s", dev, (unsigned)dik, down ? "down" : "up", g_ctrl ? 'C' : '-', g_alt ? 'A' : '-', g_shift ? 'S' : '-', decision); }
}

// ---- hook 3: HUD constructor -- build the button labels ---------------------------------------------------
// Runs inside CSWGuiMainInterface's constructor between the last InitControl and StopLoadFromLayout, so the layout GFF
// (panel+0x30) is still open. Each label is allocated, constructed and bound by tag exactly like the game's own members.
extern "C" void __cdecl HotbarBuildButtons(int panelArg) {
    uintptr_t panel = static_cast<uintptr_t>(panelArg);
    int before = rdInt(panel, Panel_CtlCount, -1);
    logf("H3 build: panel=%08x vtable=%08x controls=%d gff=%08x (previous bar panel=%08x built=%d)", (unsigned)panel, (unsigned)rdInt(panel, 0), before, (unsigned)rdInt(panel, 0x30), (unsigned)g_bar.panel, g_bar.built);
    if (!panel || IsBadReadPtr(reinterpret_cast<void*>(panel), 0x40)) { logf("H3: panel null/unreadable, abort"); return; }
    if (g_bar.panel) logf("H3: replacing bar of old HUD %08x (its labels stay leaked, never freed)", (unsigned)g_bar.panel);
    g_bar = Bar(); g_bar.panel = panel; g_bar.seq = ++g_barSeq;
    // InitControl (0x0040F620) applies a non-linear aspect projection to the extent while the global at 0x00A1B9F0 is nonzero (live build 3:
    // slots 8 and 0 came out 964 px wide). The ctor clears it right after this hook (0x0074850D), so clear it for our binds and restore.
    int* projFlag = reinterpret_cast<int*>(ProjectionFlag);
    int savedFlag = IsBadReadPtr(projFlag, 4) ? 0 : *projFlag;
    logf("H3: projection flag %08x = %d (cleared for the binds, restored after)", (unsigned)ProjectionFlag, savedFlag);
    if (savedFlag && !IsBadWritePtr(projFlag, 4)) *projFlag = 0;
    double aspect = aspectFix();
    int built = 0;
    for (int s = 0; s < ButtonCount; ++s) {
        Button& b = g_bar.b[s]; char tag[32];
        snprintf(tag, sizeof tag, "LBL_HOTBARBG%c", slotDigit(s)); bool okBg = bindLabel(panel, tag, FirstButtonId + s, &b.bg, s == 0, aspect) != nullptr;
        snprintf(tag, sizeof tag, "LBL_HOTBAR%c", slotDigit(s)); bool okIc = bindLabel(panel, tag, FirstButtonId + IconIdOffset + s, &b.ic, s == 0, aspect) != nullptr;
        if (okBg || okIc) ++built;
    }
    g_editOpen = false; g_escHit = false; g_pickRect[2] = 0; g_wheelAccum = 0;   // editor state is per HUD instance
    {   // editor controls: bound individually; the editor is only enabled when every one of them exists (partial UI would be confusing)
        int eb = 0; char tag[40];
        eb += bindLabel(panel, "LBL_HOTBAREDIT", EditId, &g_bar.edit, false, aspect) != nullptr;
        eb += bindLabel(panel, "LBL_HOTBARPBG", PbgId, &g_bar.pbg, false, aspect) != nullptr;
        eb += bindLabel(panel, "LBL_HOTBARPTITLE", TitleId, &g_bar.title, false, aspect) != nullptr;
        for (int i = 0; i < TabCount; ++i) { snprintf(tag, sizeof tag, "LBL_HOTBARTAB%d", i); eb += bindLabel(panel, tag, TabId0 + i, &g_bar.tab[i], false, aspect) != nullptr; }
        eb += bindLabel(panel, "LBL_HOTBARPREV", PrevId, &g_bar.prev, false, aspect) != nullptr;
        eb += bindLabel(panel, "LBL_HOTBARNEXT", NextId, &g_bar.next, false, aspect) != nullptr;
        eb += bindLabel(panel, "LBL_HOTBARCLEAR", ClearId, &g_bar.clear, false, aspect) != nullptr;
        for (int i = 0; i < CellCount; ++i) {
            snprintf(tag, sizeof tag, "LBL_HOTBARCELLBG%d", i); eb += bindLabel(panel, tag, CellBgId0 + i, &g_bar.cell[i].bg, false, aspect) != nullptr;
            snprintf(tag, sizeof tag, "LBL_HOTBARCELL%d", i); eb += bindLabel(panel, tag, CellIcId0 + i, &g_bar.cell[i].ic, false, aspect) != nullptr;
        }
        const int want = 1 + 1 + 1 + TabCount + 3 + 2 * CellCount;
        g_bar.editBound = (eb == want) ? 1 : 0;
        if (g_bar.pbg.ctl) for (int i = 0; i < 4; ++i) g_bar.ed.pbgRect[i] = rdInt(reinterpret_cast<uintptr_t>(g_bar.pbg.ctl), 4 + 4 * i);
        logf("H3: picker background rect [%d,%d,%d,%d]", g_bar.ed.pbgRect[0], g_bar.ed.pbgRect[1], g_bar.ed.pbgRect[2], g_bar.ed.pbgRect[3]);
        logf("H3: editor controls bound %d/%d -> editor %s", eb, want, g_bar.editBound ? "ENABLED" : "DISABLED (run scripts/add_hotbar_buttons_gui.py --replace)");
    }
    bindLabel(panel, "LBL_HOTBARTIPBG", TipBgId, &g_bar.tipBg, false, aspect);
    bindLabel(panel, "LBL_HOTBARTIP", TipId, &g_bar.tip, true, aspect);
    if (savedFlag && !IsBadWritePtr(projFlag, 4)) *projFlag = savedFlag;
    g_bar.built = built;
    int after = rdInt(panel, Panel_CtlCount, -1);
    logf("H3 done: %d/%d buttons bound, panel controls %d -> %d (expect %d)", built, ButtonCount, before, after, before + TotalNewControls);
    int arr = rdInt(panel, Panel_CtlArray);
    for (int s = 0; s < ButtonCount; ++s) for (const Widget* w : { &g_bar.b[s].bg, &g_bar.b[s].ic, &g_bar.tipBg, &g_bar.tip, &g_bar.edit }) if (w->ctl) logf("H3 check: panel list[%d] = %08x (ctl %p)", w->id, (unsigned)rdInt((uintptr_t)arr, w->id * 4), w->ctl);
}

// ---- hook 2: HUD tick ---------------------------------------------------------------------------------
extern "C" void __cdecl HotbarHudTick(void* hudPtr) {
    uintptr_t hud = reinterpret_cast<uintptr_t>(hudPtr);
    ++g_frame; g_frameForLog = g_frame;
    static DWORD lastTickTime = 0; DWORD nowT = GetTickCount();
    if (lastTickTime && nowT - lastTickTime > 1500) logf("tick: gap of %lu ms since previous HUD tick (load/menu/HUD destroyed?)", (unsigned long)(nowT - lastTickTime));
    lastTickTime = nowT;
    if (g_frame % 1800 == 0) logf("heartbeat: frame=%ld bar(panel=%08x built=%d vis=%d%d%d%d%d%d%d%d%d%d) gate=%d char='%s' ring=%d cfgMtime=%lld invoke=%d capLogged=%d ctrl/alt/shift=%d%d%d", g_frame, (unsigned)g_bar.panel, g_bar.built, g_bar.b[0].ic.visible, g_bar.b[1].ic.visible, g_bar.b[2].ic.visible, g_bar.b[3].ic.visible, g_bar.b[4].ic.visible, g_bar.b[5].ic.visible, g_bar.b[6].ic.visible, g_bar.b[7].ic.visible, g_bar.b[8].ic.visible, g_bar.b[9].ic.visible, (int)g_hudGateOk, g_charKey.c_str(), (g_ringHead - g_ringTail + 16) % 16, g_cfgMtime, g_cfg.general.invoke, g_capLogged, (int)g_ctrl, (int)g_alt, (int)g_shift);
    if (g_frame == 1) logf("tick: first HUD tick hud=%08x kbdev=%d", (unsigned)hud, g_kbDevice);
    if (hud != g_hud) { g_hud = hud; logf("tick: hud pointer now %08x", (unsigned)hud); }
    if (!hud || IsBadReadPtr(hudPtr, 0x1CA0)) { if (g_hudGateOk) logf("tick: hud %08x unreadable -> gate off", (unsigned)hud); g_hudGateOk = false; return; }

    checkConfig(g_frame == 1);

    static bool lastGate = false; static const char* lastWhy = nullptr;
    bool gate = gameplayHudActive(hud);
    g_hudGateOk = gate; g_hudTick = GetTickCount();
    if (gate != lastGate || lastWhy != gateWhy) { lastGate = gate; lastWhy = gateWhy; logf("gate: %s (%s)", gate ? "ACTIVE" : "inactive", gateWhy); if (!gate) logPanels(hud); }

    if (g_frame % 15 == 1) {
        std::string k = computeCharacterKey();
        if (k != g_charKey) { logf("character: '%s' -> '%s'", g_charKey.c_str(), k.c_str()); g_charKey = k; recomputeBound(); }
    }
    if (g_frame % ButtonRefreshFrames == 1 || g_frame == 2) refreshEffective(hud);
    probe(hud, g_frame == 60);
    updateButtons(hud, gate);
    updateHover(gate);
    updateEditor(hud, gate);
    if (gate) drainRing(hud);
    else if (g_ringTail != g_ringHead) { logf("tick: gate closed, discarding %d queued request(s)", (g_ringHead - g_ringTail + 16) % 16); g_ringTail = g_ringHead; }   // never invoke while the HUD is not the active panel
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_DETACH && g_oldProc && g_gameWnd && IsWindow(g_gameWnd) &&
        reinterpret_cast<WNDPROC>(GetWindowLongPtrA(g_gameWnd, GWLP_WNDPROC)) == wheelProc) {
        SetWindowLongPtrA(g_gameWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_oldProc)); g_oldProc = nullptr;   // only if we are still on top (Mod 1/5 do the same)
    }
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("force-power-hotbar 0.3.0-dev loaded; cwd=%s pid=%lu ini=%s", cwd, (unsigned long)GetCurrentProcessId(), IniPath);
        logf("addresses: invoke=%08x kbdevGlobal=%08x app=%08x getCreature=%08x hudVt=%08x whitelist=%08x,%08x,%08x,%08x", (unsigned)HudInvokeGroup, (unsigned)KeyboardDeviceGlobal, (unsigned)AppGlobal, (unsigned)GetControlledCreature, (unsigned)HudVtable, (unsigned)PanelWhitelist[0], (unsigned)PanelWhitelist[1], (unsigned)PanelWhitelist[2], (unsigned)PanelWhitelist[3]);
    }
    return TRUE;
}
