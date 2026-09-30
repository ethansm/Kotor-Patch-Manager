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
constexpr int TipBgId = FirstButtonId + 2 * ButtonCount, TipId = TipBgId + 1;   // hover tooltip frame + text
constexpr uintptr_t FnTextSet = 0x00416E30;       // thiscall(ECX = CSWGuiText+0x18 = label+0xF0, CExoString*) RET 4: sets text + relayouts
constexpr uintptr_t TextVtable = 0x009876B4;
constexpr int Label_Text = 0xD8, Text_Renderer = 0x14, Text_StringSub = 0x18;
constexpr DWORD TipDelayMs = 450;
constexpr int TipCharW = 21, TipLineH = 32, TipPad = 22, TipWrap = 38, TipMaxDescLines = 5;   // px estimates for dialogfont16x16 at 2.667 px/unit; calibrate live
const char* const Spells2daPath = "override/spells.2da";
const char* const TlkPath = "dialog.tlk";
const char* const FrameResref = "hotbar_slot";       // TGAs written by the GUI script into override/
const char* const FrameHiResref = "hotbar_slot_hi";
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
    char b[24]; snprintf(b, sizeof b, "obj%08x", rdInt(c, 4)); 
    if (!g_scannedCreatures.count(rdInt(c, 4))) { g_scannedCreatures.insert(rdInt(c, 4)); scanObjectStrings("controlled", c); }
    return b;
}

// ---- config: load / hot-reload / active slots --------------------------------------------------------
void writeTemplateIni() {
    hb::Config t; t.general.invoke = 0; t.profile("Default");
    t.Save(IniPath);
    logf("cfg: wrote template %s", IniPath);
}

void recomputeBound() {
    int bound = 0;
    for (int r = 0; r < hb::Rows; ++r) for (int s = 0; s < hb::Slots; ++s) {
        hb::ActionRef a = g_cfg.slotFor(g_charKey, r, s);
        g_slotBound[r][s] = !a.empty(); if (!a.empty()) ++bound;
    }
    logf("cfg: profile for character '%s' -> %d/30 slots bound", g_charKey.c_str(), bound);
    if (g_cfg.general.verbose) for (int r = 0; r < hb::Rows; ++r) for (int s = 0; s < hb::Slots; ++s) {
        hb::ActionRef a = g_cfg.slotFor(g_charKey, r, s);
        if (!a.empty()) logf("    %s = %s", hb::SlotKeyName(r, s).c_str(), hb::Format(a).c_str());
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
        hb::ActionRef a = g_cfg.slotFor(g_charKey, r.row, r.slot);
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
};
struct Button { Widget bg, ic; bool avail = false; bool hover = false; Entry entry; };
struct Bar { uintptr_t panel = 0; int built = 0; long seq = 0; Button b[ButtonCount]; Widget tipBg, tip; int tipSlot = -1; };
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

// Title, key, type, cost and description of slot s's current entry ('\n'-separated lines).
std::string buildTipText(int s, const Entry& e) {
    std::string title = e.name; size_t par = title.rfind(" ("); if (par != std::string::npos && par > 0 && title.back() == ')') title.resize(par);
    std::string qty = e.name.size() > title.size() + 3 ? e.name.substr(title.size() + 2, e.name.size() - title.size() - 3) : "";
    std::string out = title + "\n";
    const char* kind = (e.key & 0x40000000) ? "Item" : ((e.key & 0x20000000) ? "Force Power" : "Mode");
    out += std::string("Key: ") + slotDigit(s) + "   " + kind + "\n";
    if ((e.key & 0x20000000) && !(e.key & 0x40000000)) {
        ensureLookups();
        int row = e.key & 0x1FFFFFFF;
        std::string fp = nullish(g_spells.Get(row, "forcepoints")), range = g_spells.Get(row, "range");
        if (!fp.empty() && fp != "0") out += "Force Points: " + fp + "\n";
        if (range == "P") out += "Range: Personal\n"; else if (range == "T") out += "Range: Target\n";
        std::string dsc = nullish(g_spells.Get(row, "spelldesc"));
        if (!dsc.empty()) { std::string d = g_tlk.Get((uint32_t)atoi(dsc.c_str())); if (!d.empty()) out += wrapText(d, TipWrap, TipMaxDescLines) + "\n"; }
        logf("tip: spell row %d fp='%s' range='%s' desc strref='%s'", row, fp.c_str(), range.c_str(), dsc.c_str());
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

void showTip(int s) {
    Button& b = g_bar.b[s];
    if (!g_bar.tip.ctl || !g_bar.tipBg.ctl) { logf("tip: show slot %d skipped: tooltip labels not bound (bg=%p text=%p)", s, (void*)g_bar.tipBg.ctl, (void*)g_bar.tip.ctl); g_bar.tipSlot = s; return; }
    std::string text = buildTipText(s, b.entry);
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
    uintptr_t fc = reinterpret_cast<uintptr_t>(b.bg.ctl); int fx = rdInt(fc, 4), fy = rdInt(fc, 8);
    int x = fx, y = fy - h - 12;
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
        Button& b = g_bar.b[s]; bool hov = (s == over) && b.avail; b.hover = hov;
        if (b.bg.ctl && b.bg.visible) setFill(b, b.bg, s, hov ? FrameHiResref : FrameResref);
    }
    if (over >= 0 && g_bar.b[over].avail && g_bar.tipSlot != over && GetTickCount() - hoverStart >= TipDelayMs) showTip(over);
    if (over < 0 || !g_bar.b[over].avail) hideTip("no hover");
}

// Per-tick button state. gate = HUD is the active panel. Entries are re-resolved every ButtonRefreshFrames frames or when
// the cached view is stale (config/character/gate change), never per frame.
void updateButtons(uintptr_t hud, bool gate) {
    static uintptr_t lastMissingHud = 0; static long lastRefresh = -1000; static bool lastGateSeen = false; static std::string lastChar = "\x01";
    if (g_bar.panel != hud || g_bar.built == 0) {
        if (lastMissingHud != hud) { lastMissingHud = hud; logf("buttons: no usable bar for hud %08x (bar panel=%08x built=%d/%d) -- HUD ctor hook did not run for it, or no button bound", (unsigned)hud, (unsigned)g_bar.panel, g_bar.built, ButtonCount); }
        return;
    }
    bool stale = g_frame - lastRefresh >= ButtonRefreshFrames || gate != lastGateSeen || g_charKey != lastChar;
    if (!stale) return;
    lastRefresh = g_frame; lastGateSeen = gate; lastChar = g_charKey;
    for (int s = 0; s < ButtonCount; ++s) {
        Button& b = g_bar.b[s];
        if (!b.bg.ctl && !b.ic.ctl) continue;
        if (!gate) { b.avail = false; setVisible(b, b.bg, s, false, "hud not active"); setVisible(b, b.ic, s, false, "hud not active"); continue; }
        // frame + digit are always shown while the HUD is active (an empty slot reads as a dim frame)
        hb::ActionRef want = g_cfg.slotFor(g_charKey, 0, s);
        Entry e; const char* state; bool have = false;
        if (want.empty()) state = "unbound";
        else if (!resolve(hud, want, &e)) state = "entry not available";
        else { have = true; state = (e.flags & 1) ? "usable" : "unusable (dimmed)"; }
        b.avail = have; if (have) b.entry = e;
        setVisible(b, b.bg, s, true, state); setVisible(b, b.ic, s, true, state);
        setAlpha(b, b.bg, s, have ? b.bg.baseAlpha : b.bg.baseAlpha * EmptyFrameAlpha);
        setFill(b, b.ic, s, have ? e.icon : std::string());
        setAlpha(b, b.ic, s, (have && !(e.flags & 1)) ? b.ic.baseAlpha * DimAlpha : b.ic.baseAlpha);
    }
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
unsigned char* bindLabel(uintptr_t panel, const char* tagName, int expectId, Widget* w, bool dump) {
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
    bool isDigit = dik >= 0x02 && dik <= 0x0B;
    const char* decision = "pass";
    if (isDigit) {
        if (!down) {
            if (g_downSwallowed[dik]) { g_downSwallowed[dik] = false; rec->dwOfs = 0; rec->dwData = 0; decision = "swallow-up"; }
        } else if (!g_shift) {
            int row = g_alt ? 2 : (g_ctrl ? 1 : 0), slot = (int)dik - hb::kFirstDik;
            bool active = g_hudGateOk && (GetTickCount() - g_hudTick) < 250;
            if (!active) decision = "pass(hud inactive)";
            else if (!g_slotBound[row][slot]) decision = "pass(unbound)";
            else {
                int next = (g_ringHead + 1) % 16;
                if (next == g_ringTail) decision = "pass(ring full)";
                else { g_ring[g_ringHead] = { row, slot }; g_ringHead = next; g_downSwallowed[dik] = true; rec->dwOfs = 0; rec->dwData = 0; decision = "SWALLOW"; }
            }
        } else decision = "pass(shift)";
    }
    if (logIt || isDigit || isMod) { ++g_capLogged; logf("cap: dev=%d dik=%02x %s mods=%c%c%c -> %s", dev, (unsigned)dik, down ? "down" : "up", g_ctrl ? 'C' : '-', g_alt ? 'A' : '-', g_shift ? 'S' : '-', decision); }
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
    int built = 0;
    for (int s = 0; s < ButtonCount; ++s) {
        Button& b = g_bar.b[s]; char tag[32];
        snprintf(tag, sizeof tag, "LBL_HOTBARBG%c", slotDigit(s)); bool okBg = bindLabel(panel, tag, FirstButtonId + s, &b.bg, s == 0) != nullptr;
        snprintf(tag, sizeof tag, "LBL_HOTBAR%c", slotDigit(s)); bool okIc = bindLabel(panel, tag, FirstButtonId + IconIdOffset + s, &b.ic, s == 0) != nullptr;
        if (okBg || okIc) ++built;
    }
    bindLabel(panel, "LBL_HOTBARTIPBG", TipBgId, &g_bar.tipBg, false);
    bindLabel(panel, "LBL_HOTBARTIP", TipId, &g_bar.tip, true);
    g_bar.built = built;
    int after = rdInt(panel, Panel_CtlCount, -1);
    logf("H3 done: %d/%d buttons bound, panel controls %d -> %d (expect %d)", built, ButtonCount, before, after, before + 2 * ButtonCount + 2);
    int arr = rdInt(panel, Panel_CtlArray);
    for (int s = 0; s < ButtonCount; ++s) for (const Widget* w : { &g_bar.b[s].bg, &g_bar.b[s].ic, &g_bar.tipBg, &g_bar.tip }) if (w->ctl) logf("H3 check: panel list[%d] = %08x (ctl %p)", w->id, (unsigned)rdInt((uintptr_t)arr, w->id * 4), w->ctl);
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
    probe(hud, g_frame == 60);
    updateButtons(hud, gate);
    updateHover(gate);
    if (gate) drainRing(hud);
    else if (g_ringTail != g_ringHead) { logf("tick: gate closed, discarding %d queued request(s)", (g_ringHead - g_ringTail + 16) % 16); g_ringTail = g_ringHead; }   // never invoke while the HUD is not the active panel
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("force-power-hotbar 0.2.0-dev loaded; cwd=%s pid=%lu ini=%s", cwd, (unsigned long)GetCurrentProcessId(), IniPath);
        logf("addresses: invoke=%08x kbdevGlobal=%08x app=%08x getCreature=%08x hudVt=%08x whitelist=%08x,%08x,%08x,%08x", (unsigned)HudInvokeGroup, (unsigned)KeyboardDeviceGlobal, (unsigned)AppGlobal, (unsigned)GetControlledCreature, (unsigned)HudVtable, (unsigned)PanelWhitelist[0], (unsigned)PanelWhitelist[1], (unsigned)PanelWhitelist[2], (unsigned)PanelWhitelist[3]);
    }
    return TRUE;
}
