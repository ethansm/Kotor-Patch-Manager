// Force Power Hotbar (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...). Prototype phase: 30 keybound slots
// (1-0, Ctrl+1-0, Alt+1-0) that invoke a vanilla HUD quick-action; NO on-screen bars yet.
// Research trail and every address: patch_manager_mods/07_force_power_hotbar.md (local-setup/KOTOR-II worktree).
//
// Hooks (cdecl detours, original bytes still run):
//   0x0072CB63  CExoInputInternal::GetEvents+0x383  CaptureHotbarKey(record*, device)  -- swallow bound digits
//   0x0074B3F0  CSWGuiMainInterface::Draw           HotbarHudTick(hud)                 -- gate, config, probe, invoke
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
    for (unsigned char c : s) if (c < 0x20 || c > 0x7E) return "";
    return s;
}

// ---- action decode (shared by the catalog, the INI and the resolver) --------------------------------
struct Entry { int group, index, key, flags; std::string name; hb::ActionRef ref; };

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
            logf("    [%d.%d] key=%08x flags=%08x usable=%d why=%d name=\"%s\" ref=%s raw:%s", g, e.index, (unsigned)e.key, (unsigned)e.flags,
                 e.flags & 1, (e.flags >> 1) & 0xF, e.name.c_str(), hb::Format(e.ref).c_str(), raw);
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

// ---- hook 2: HUD tick ---------------------------------------------------------------------------------
extern "C" void __cdecl HotbarHudTick(void* hudPtr) {
    uintptr_t hud = reinterpret_cast<uintptr_t>(hudPtr);
    ++g_frame; g_frameForLog = g_frame;
    static DWORD lastTickTime = 0; DWORD nowT = GetTickCount();
    if (lastTickTime && nowT - lastTickTime > 1500) logf("tick: gap of %lu ms since previous HUD tick (load/menu/HUD destroyed?)", (unsigned long)(nowT - lastTickTime));
    lastTickTime = nowT;
    if (g_frame % 1800 == 0) logf("heartbeat: frame=%ld gate=%d char='%s' ring=%d cfgMtime=%lld invoke=%d capLogged=%d ctrl/alt/shift=%d%d%d", g_frame, (int)g_hudGateOk, g_charKey.c_str(), (g_ringHead - g_ringTail + 16) % 16, g_cfgMtime, g_cfg.general.invoke, g_capLogged, (int)g_ctrl, (int)g_alt, (int)g_shift);
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
    if (gate) drainRing(hud);
    else if (g_ringTail != g_ringHead) { logf("tick: gate closed, discarding %d queued request(s)", (g_ringHead - g_ringTail + 16) % 16); g_ringTail = g_ringHead; }   // never invoke while the HUD is not the active panel
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("force-power-hotbar 0.1.0-dev loaded; cwd=%s pid=%lu ini=%s", cwd, (unsigned long)GetCurrentProcessId(), IniPath);
        logf("addresses: invoke=%08x kbdevGlobal=%08x app=%08x getCreature=%08x hudVt=%08x whitelist=%08x,%08x,%08x,%08x", (unsigned)HudInvokeGroup, (unsigned)KeyboardDeviceGlobal, (unsigned)AppGlobal, (unsigned)GetControlledCreature, (unsigned)HudVtable, (unsigned)PanelWhitelist[0], (unsigned)PanelWhitelist[1], (unsigned)PanelWhitelist[2], (unsigned)PanelWhitelist[3]);
    }
    return TRUE;
}
