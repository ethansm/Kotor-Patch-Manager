// WorkbenchUpgrade.cpp -- Workbench Upgrade (doc 55): switch between the workbench Create/Breakdown and Upgrade screens (BTN_SWITCH built in the DLL via
// edited component_p/upgradeitems_p GUIs), plus a live crafting-compare box. Hooks are entry detours (handler runs BEFORE the original
// bytes, skip_original_bytes=false); most only read memory through SafeRead and log, the switch/compare handlers also drive the UI.
// Output: workbench_upgrade_log.txt via KpLog.h (channel main). Each line: `s<seq> +<ms since previous line> <HOOK> ...`.
// Entry-only hooks cannot see exit state, so every hub/manager event also snapshots the GUI manager (MGR line); the snapshot at the NEXT
// event shows the effect of the previous one.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "../Common/SafeRead.h"
#include "../Common/KpLog.h"
#include "WorkbenchUpgradeLogic.h"
#include "../Common/ItemCompareRead.h"
#include "../Common/GameAddr.h"
#include "../Common/Party.h"
#include "../Common/HudHost.h"
#include "../Common/GuiKit.h"
#include "../Common/GuiButton.h"
#include "../Common/TextFit.h"

namespace itemcmp {
namespace icr = ::itemcmpread;
}

namespace guikit {
inline void init() {
    g_gk.rebind();
}
}

KP_LOG_PATCH("workbench-upgrade");

namespace {

using saferead::readAt;

constexpr const char* kVersion = "0.1.0-dev";
constexpr const char* kLogName = "workbench_upgrade_log.txt";
constexpr int kMaxLogLines = 3000;
constexpr int kListCap = 12;     // panels listed per manager list
constexpr int kCountCap = 256;   // list counts above this are reported as untrusted and not iterated

// Hub (CSWGuiUpgradeSelection) byte offsets, doc 55 sec 2 / 5d.
constexpr int kHubMgr = 0x1c;        // M = [hub+0x1c] (GUI manager), WB-W5 u01
constexpr int kHubUpgradeBtn = 0x2f8;
constexpr int kHubCreateBtn = 0x698;
constexpr int kHubCreatePanel = 0x868;
constexpr int kHubPartyMember = 0x18a8;
constexpr int kHubTab = 0x18bc;
constexpr int kHubItemList = 0x18c0;
constexpr int kHubFlagNoCreate = 0x18c8;
constexpr int kHubFlagNoUpgrade = 0x18cc;
constexpr int kHubCount = 0xc8c;
constexpr int kHubRow = 0xc90;
constexpr int kHubWorkbench = 0xc94;
// Manager (M) offsets, WB-W5.
constexpr int kMgrHidden = 0x70, kMgrL88 = 0x88, kMgrL88n = 0x8c, kMgrL94 = 0x94, kMgrL94n = 0x98, kMgrA0 = 0xa0, kMgrA4 = 0xa4;
constexpr int kPanelFlags = 0x48;
// Workbench (CSWGuiUpgrade) offsets.
constexpr int kWbWeapon = 0x3d34, kWbStaged = 0x3d38, kWbMode = 0x3d2c, kWbLens = 0x3d54, kWbHub = 0x3dc0;

// Verified Ghidra addresses (doc 55 P3; verified via query_ghidra.py):
constexpr uintptr_t kAddrMgrPush = 0x00410530;          // CSWGuiManager::AddPanel (batch B0, mgr=ECX, panel, flags, close)
constexpr uintptr_t kAddrPopModalPanel = 0x004109C0;    // CSWGuiManager::PopModalPanel (batch B0)
constexpr uintptr_t kAddrAddEvent = 0x00418AF0;         // CSWGuiControl::AddEvent (batch B0)
constexpr uintptr_t kAddrSetPanelFlags = 0x0075BAA0;    // FUN_0075baa0 (panel flag update)
constexpr uintptr_t kAddrOnCreateItem = 0x008C73F0;     // CSWGuiUpgradeSelection::OnCreateItem (batch B24)
constexpr uintptr_t kAddrOnItemSelected = 0x008C7F60;   // CSWGuiUpgradeSelection::OnItemSelected (batch B24)
constexpr uintptr_t kAddrCloseCreateItem = 0x008D4420;  // CSWGuiCreateItem::CloseCreateItem (batch B24)
constexpr uintptr_t kAddrGetSkillRank = 0x006B7BB0;     // CSWSCreatureStats::GetSkillRank (batch B0, thiscall)
constexpr uintptr_t kAddrShowUpgradeScreen = 0x007CD7E0;
constexpr uintptr_t kAddrUpdate3DView = 0x008CD2C0;

enum Site : unsigned { S_MGR = 1, S_PANEL, S_HUB, S_WB, S_LIST, S_SLOT, S_ITEM, S_PC, S_PCSRV, S_PCSTAT, S_SKL };

// ---- state (main thread only) ---------------------------------------------------------------------------
int g_logLines = 0;
unsigned g_seq = 0;
DWORD g_lastTick = 0;
bool g_armed = false;   // set by H2/H9 (workbench opened): H1 stays silent before, so general menu traffic cannot fill the log cap
uintptr_t g_hub = 0;    // last hub pointer seen (handler ECX / ctor ECX)
unsigned g_hits[24] = {};
DWORD g_hitTick[24] = {};
wbu::Dedupe g_dedupe;

// Configuration loaded from workbench_upgrade.ini (P5 log tightening)
bool g_cfgLoaded = false;
int g_cfgSwitchButton = 1;
int g_cfgCraftingCompare = 1;
int g_cfgVerbose = 0;
int g_cfgSkipDialog = 1;

void loadConfig() {
    if (g_cfgLoaded) return;
    g_cfgSwitchButton = GetPrivateProfileIntA("workbench_upgrade", "switch_button", 1, ".\\workbench_upgrade.ini");
    g_cfgCraftingCompare = GetPrivateProfileIntA("workbench_upgrade", "crafting_compare", 1, ".\\workbench_upgrade.ini");
    g_cfgVerbose = GetPrivateProfileIntA("workbench_upgrade", "verbose", 0, ".\\workbench_upgrade.ini");
    g_cfgSkipDialog = GetPrivateProfileIntA("workbench_upgrade", "skip_dialog", 1, ".\\workbench_upgrade.ini");
    g_cfgLoaded = true;
}

// Live state machine and mode switch context (doc 55 P3)
wbu::ScreenState g_screenState = wbu::ScreenState::CLOSED;
wbu::CrystalPreviewState g_crystalPreview;
bool g_switchInProgress = false;
DWORD g_lastSwitchTick = 0;
// ShowUpgradeScreen arguments (the hub +0x18c8/+0x18cc flag offsets read garbage in play, 2026-11-08 log): bDisableItemCreation / bDisableUpgrade.
uint32_t g_h2NoCreate = 0, g_h2NoUpgrade = 0;
uint32_t g_activeScreenBaseDepth = 0;
uintptr_t g_pendingCreatePanel = 0;
bool g_createViaHub = false;      // Create was opened from the hub's own switch button (OnCreateItem popped the hub), so the return trip must push it back
uintptr_t g_hubLaidOut = 0;
uintptr_t g_pendingUpgradePanel = 0;
uintptr_t g_activeCreatePanel = 0;
uintptr_t g_activeUpgradePanel = 0;

KP_CHANNEL(chMain, "main");
const bool g_fallbackSet = (KpLog_SetFallbackFile(kLogName), KpLog_SetFallbackLevel(KPLOG_INFO), true);

constexpr int LogPiece = 220;

void emit(const char* s) {
    int n = static_cast<int>(strlen(s));
    if (n <= LogPiece) { KP_LOG(chMain, "%s", s); return; }
    char piece[LogPiece + 1];
    int pos = 0; bool first = true;
    while (pos < n) {
        int len = n - pos < LogPiece - 2 ? n - pos : LogPiece - 2;
        if (pos + len < n) {
            int cut = len;
            while (cut > LogPiece / 2 && s[pos + cut] != ' ') --cut;
            if (s[pos + cut] == ' ') len = cut;
        }
        if (first) { memcpy(piece, s + pos, len); piece[len] = 0; }
        else { piece[0] = '+'; piece[1] = ' '; memcpy(piece + 2, s + pos, len); piece[len + 2] = 0; }
        KP_LOG(chMain, "%s", piece);
        first = false; pos += len;
        if (pos < n && s[pos] == ' ') ++pos;
    }
}

void logf(const char* fmt, ...) {
    if (g_logLines >= kMaxLogLines) return;
    char body[640];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(body, sizeof body, fmt, ap);
    va_end(ap);
    unsigned dropped = 0;
    if (!g_dedupe.admit(body, &dropped)) return;
    ++g_logLines;
    DWORD now = GetTickCount();
    unsigned long ms = g_lastTick ? static_cast<unsigned long>(now - g_lastTick) : 0;
    g_lastTick = now;
    char buf[720];
    int n = snprintf(buf, sizeof buf, "s%u +%lu ", ++g_seq, ms);
    if (dropped) n += snprintf(buf + n, sizeof buf - n, "(prev line x%u more) ", dropped + 1);
    snprintf(buf + n, sizeof buf - n, "%s", body);
    emit(buf);
}

inline unsigned long U(uintptr_t v) { return static_cast<unsigned long>(v); }
inline uintptr_t P(void* p) { return reinterpret_cast<uintptr_t>(p); }

bool rd(unsigned site, uintptr_t base, int off, uint32_t* out) { return base != 0 && readAt(site, base, off, out); }

// "hex" or "??" for base+off.
void fld(char* out, size_t n, unsigned site, uintptr_t base, int off) {
    uint32_t v = 0;
    bool ok = rd(site, base, off, &v);
    wbu::fmtVal(out, n, ok, v);
}

// Tag a panel pointer: H = hub, C = create panel (embedded or pointed-to), I = item list, W = workbench, ? = other.
char tagPanel(uintptr_t p, uintptr_t hub) {
    if (p == 0 || hub == 0) return '?';
    uint32_t v = 0;
    if (p == hub) return 'H';
    if (p == hub + kHubCreatePanel || (rd(S_HUB, hub, kHubCreatePanel, &v) && v == p)) return 'C';
    if (p == hub + kHubItemList || (rd(S_HUB, hub, kHubItemList, &v) && v == p)) return 'I';
    if (rd(S_HUB, hub, kHubWorkbench, &v) && v == p) return 'W';
    return '?';
}

// One manager list: "name=ptr/count [ptr:tag:flags ...]".
void fmtList(char* out, size_t n, uintptr_t M, int offPtr, int offCnt, const char* name, uintptr_t hub) {
    uint32_t arr = 0, cnt = 0;
    bool okA = rd(S_LIST, M, offPtr, &arr), okC = rd(S_LIST, M, offCnt, &cnt);
    if (!okA || !okC) { snprintf(out, n, "%s=unreadable", name); return; }
    int c = wbu::sane(cnt, kCountCap);
    int len = snprintf(out, n, "%s=%x/%d", name, static_cast<unsigned>(arr), c);
    if (c < 0 || arr == 0) return;
    len += snprintf(out + len, n - len, " [");
    for (int i = 0; i < c && i < kListCap && len < static_cast<int>(n) - 40; ++i) {
        uint32_t p = 0, fl = 0;
        if (!rd(S_LIST, arr, i * 4, &p)) { len += snprintf(out + len, n - len, "?? "); continue; }
        char fb[40] = "??";
        if (p != 0 && rd(S_PANEL, p, kPanelFlags, &fl)) wbu::fmtPanelFlags(fb, sizeof fb, fl);
        len += snprintf(out + len, n - len, "%x:%c:%s ", static_cast<unsigned>(p), tagPanel(p, hub), p ? fb : "null");
    }
    snprintf(out + len, n - len, "%s]", c > kListCap ? "..." : "");
}

// GUI manager snapshot. M may be 0/unreadable. Logs two lines so each stays under the core's cut.
void snapMgr(const char* why, uintptr_t M, uintptr_t hub) {
    loadConfig();
    if (!g_cfgVerbose) return;
    if (M == 0) { logf("MGR %s m=0", why); return; }
    saferead::beginScope();
    char hid[16], a0[16], a4[16], s0[16], s0b[16], l88[360], l94[360];
    fld(hid, sizeof hid, S_MGR, M, kMgrHidden);
    fld(a0, sizeof a0, S_MGR, M, kMgrA0);
    fld(a4, sizeof a4, S_MGR, M, kMgrA4);
    // slot 0 of the FUN_004122a0 slot table: treat +0xa0 as an array pointer (UNVERIFIED), read [a0+0] and [[a0+0]+4].
    uint32_t arr = 0, slot0 = 0;
    bool okArr = rd(S_SLOT, M, kMgrA0, &arr), okS0 = okArr && arr != 0 && rd(S_SLOT, arr, 0, &slot0);
    wbu::fmtVal(s0, sizeof s0, okS0, slot0);
    if (okS0 && slot0 != 0) fld(s0b, sizeof s0b, S_SLOT, slot0, 4); else snprintf(s0b, sizeof s0b, "-");
    fmtList(l88, sizeof l88, M, kMgrL88, kMgrL88n, "L88", hub);
    fmtList(l94, sizeof l94, M, kMgrL94, kMgrL94n, "L94", hub);
    logf("MGR %s m=%lx b70=%s a0=%s a4=%s slot0=%s slot0+4=%s", why, U(M), hid, a0, a4, s0, s0b);
    logf("MGR %s %s", why, l88);
    logf("MGR %s %s", why, l94);
}

uintptr_t mgrOfHub(uintptr_t hub) {
    uint32_t m = 0;
    return rd(S_HUB, hub, kHubMgr, &m) ? m : 0;
}

// Hub state line (L5/L6): control pointers, flags, tab, item list, selection fields.
void snapHub(const char* why, uintptr_t hub) {
    loadConfig();
    if (!g_cfgVerbose) return;
    if (hub == 0) { logf("HUB %s hub=0", why); return; }
    saferead::beginScope();
    char up[16], cr[16], cp[16], tab[16], il[16], f8[16], fc[16], cnt[16], row[16], wb[16];
    fld(up, sizeof up, S_HUB, hub, kHubUpgradeBtn);
    fld(cr, sizeof cr, S_HUB, hub, kHubCreateBtn);
    fld(cp, sizeof cp, S_HUB, hub, kHubCreatePanel);
    fld(tab, sizeof tab, S_HUB, hub, kHubTab);
    fld(il, sizeof il, S_HUB, hub, kHubItemList);
    fld(f8, sizeof f8, S_HUB, hub, kHubFlagNoCreate);
    fld(fc, sizeof fc, S_HUB, hub, kHubFlagNoUpgrade);
    fld(cnt, sizeof cnt, S_HUB, hub, kHubCount);
    fld(row, sizeof row, S_HUB, hub, kHubRow);
    fld(wb, sizeof wb, S_HUB, hub, kHubWorkbench);
    logf("HUB %s hub=%lx m=%lx upBtn@2f8=%s crBtn@698=%s crPanel@868=%s tab@18bc=%s items@18c0=%s noCreate@18c8=%s noUpgrade@18cc=%s c8c=%s row@c90=%s wb@c94=%s",
         why, U(hub), U(mgrOfHub(hub)), up, cr, cp, tab, il, f8, fc, cnt, row, wb);
}

// Always-on selection snapshot (preselect investigation, WB-P8): hub list index/count and the item-list object's selection fields.
// +0x2d4 dword holds the listbox index short at +0x2d6 (high half); +0x2ac/+0x2b0 = list array/count (0041ff70.c).
void snapSel(const char* why, uintptr_t hub) {
    if (!hub) { logf("[WBU] SEL %s hub=0", why); return; }
    saferead::beginScope();
    uint32_t d2d4 = 0, l2ac = 0, n2b0 = 0, il = 0, a84 = 0, a8c = 0, a90 = 0, i2d4 = 0, in2b0 = 0;
    const bool okH = rd(S_HUB, hub, 0x2d4, &d2d4) && rd(S_HUB, hub, 0x2ac, &l2ac) && rd(S_HUB, hub, 0x2b0, &n2b0);
    if (rd(S_HUB, hub, kHubItemList, &il) && il) {
        rd(S_PANEL, il, 0xc84, &a84); rd(S_PANEL, il, 0xc8c, &a8c); rd(S_PANEL, il, 0xc90, &a90);
        rd(S_PANEL, il, 0x2d4, &i2d4); rd(S_PANEL, il, 0x2b0, &in2b0);
    }
    logf("[WBU] SEL %s hub=%lx okH=%d hub+2d4=%08x (idx=%d) +2ac=%x +2b0=%u | itemList=%x +c84=%x +c8c=%x +c90=%x +2d4=%08x (idx=%d) +2b0=%u",
         why, U(hub), okH ? 1 : 0, d2d4, static_cast<int>(static_cast<int16_t>(d2d4 >> 16)), l2ac, n2b0, il, a84, a8c, a90, i2d4,
         static_cast<int>(static_cast<int16_t>(i2d4 >> 16)), in2b0);
}

// Append properties from a raw property array (count at offCnt, array at offArr) into ItemFacts.
// Follows itemcmpread::readProps: pass 1 checks Keen, pass 2 builds rows with skipLabel and PROP_DIFF_MAX clamping.
static void appendPropArray(uintptr_t item, int offCnt, int offArr, itemcmp::ItemFacts* out) {
    int32_t cnt = 0;
    uint32_t arr = 0;
    if (!saferead::readAt(itemcmpread::SiteCmp, item, offCnt, &cnt) || cnt <= 0 || cnt >= itemcmpread::MaxPropCount) return;
    if (!saferead::readAt(itemcmpread::SiteCmp, item, offArr, &arr) || !arr) return;

    const uintptr_t a = static_cast<uintptr_t>(arr);
    itemcmpread::RawProp r;
    for (int32_t i = 0; i < cnt; ++i) {
        if (!itemcmpread::readRaw(a, i, &r)) continue;
        if (r.type == itemcmpread::KeenType && r.subtype == itemcmpread::KeenSubtype) {
            out->keen = true;
        }
    }
    for (int32_t i = 0; i < cnt; ++i) {
        if (!itemcmpread::readRaw(a, i, &r)) {
            out->dropped += static_cast<unsigned>(cnt - i);
            break;
        }
        char label[48] = {0};
        const bool haveLabel = itemcmpread::g_label(r.type, label, sizeof(label));
        if (haveLabel && itemcmpread::skipLabel(label, r.subtype)) continue;
        if (out->nProps >= itemcmp::PROP_DIFF_MAX) {
            ++out->dropped;
            continue;
        }
        itemcmp::PropRow& p = out->props[out->nProps++];
        memset(&p, 0, sizeof(p));
        p.type = r.type;
        p.subtype = r.subtype;
        p.costTable = r.costTable;
        p.costValue = r.costValue;
        p.param1 = r.param1;
        if (!haveLabel) {
            p.known = 0;
            continue;
        }
        itemcmpread::PropStrings s;
        memset(&s, 0, sizeof(s));
        itemcmpread::g_propText(r.type, r.subtype, r.costValue,
                                r.param1 == itemcmpread::Param1None ? itemcmpread::Param1None : r.param1Value,
                                &s);
        s.name[sizeof(s.name) - 1] = 0;
        s.sub[sizeof(s.sub) - 1] = 0;
        s.val[sizeof(s.val) - 1] = 0;
        char value[176] = {0};
        itemcmpread::appendTo(value, sizeof(value), s.sub);
        if (s.sub[0] && s.val[0]) itemcmpread::appendTo(value, sizeof(value), " ");
        itemcmpread::appendTo(value, sizeof(value), s.val);
        itemcmpread::buildText(&p, s.name[0] ? s.name : label, value);
        p.known = 1;
    }
}

// WB-F2: Dual-list property reader. Reads List B first (+0x28C count, +0x294 array; passive crafting properties)
// then List A second (+0x288 count, +0x290 array; active properties) up to PROP_DIFF_MAX (16 total).
static bool readItemFactsBothLists(uintptr_t item, itemcmp::ItemFacts* out) {
    if (!itemcmp::icr::readFacts(item, out)) return false;
    out->nProps = 0;
    out->dropped = 0;
    out->keen = false;

    // List B FIRST: count at item + 0x28C, array pointer at item + 0x294
    appendPropArray(item, 0x28C, 0x294, out);

    // List A SECOND: count at item + 0x288, array pointer at item + 0x290
    appendPropArray(item, 0x288, 0x290, out);

    return true;
}

// Snapshot item property rows via ItemCompareRead.h (L4 probe), plus upgradeable bitmask and installed slots (P1b).
void snapItemProps(const char* label, uintptr_t item) {
    loadConfig();
    if (!g_cfgVerbose) return;
    if (item == 0) { logf("PROPS %s item=0", label); return; }
    saferead::beginScope();
    itemcmp::ItemFacts facts;
    bool ok = readItemFactsBothLists(item, &facts);
    char buf[360];
    wbu::fmtProps(buf, sizeof buf, ok, facts.nProps, facts.props, facts.keen, facts.dropped);

    uint32_t upgMask = 0;
    bool okMask = rd(S_ITEM, item, wbu::kItemUpgradesMaskOff, &upgMask);
    char maskBuf[16];
    wbu::fmtVal(maskBuf, sizeof maskBuf, okMask, upgMask);

    uint32_t slots[6] = {};
    bool okSlots[6] = {};
    for (int i = 0; i < 6; ++i) {
        okSlots[i] = rd(S_ITEM, item, wbu::kItemUpgradeSlot0Off + i * 4, &slots[i]);
    }
    char slotsBuf[64];
    wbu::fmtUpgradeSlots(slotsBuf, sizeof slotsBuf, slots, okSlots);

    logf("PROPS %s item=%lx upgMask=%s %s %s", label, U(item), maskBuf, slotsBuf, buf);
}

// Workbench weapon / lens state. The weapon pointer goes stale (WB-W5 u04): log it with the staged pointer and the lens array, never cache it.
void snapWb(const char* why, uintptr_t wb) {
    loadConfig();
    if (!g_cfgVerbose) return;
    if (wb == 0) { logf("WB %s wb=0", why); return; }
    saferead::beginScope();
    uint32_t weapon = 0, staged = 0;
    uint32_t mode = 0;
    bool okW = rd(S_WB, wb, kWbWeapon, &weapon), okS = rd(S_WB, wb, kWbStaged, &staged), okM = rd(S_WB, wb, kWbMode, &mode);
    char w[16], s[16], m[16], w1d0[16] = "-", w4[16] = "-", lens[160];
    wbu::fmtVal(w, sizeof w, okW, weapon);
    wbu::fmtVal(s, sizeof s, okS, staged);
    wbu::fmtVal(m, sizeof m, okM, mode & 0xff);
    if (okW && weapon != 0) { fld(w1d0, sizeof w1d0, S_ITEM, weapon, 0x1d0); fld(w4, sizeof w4, S_ITEM, weapon, 4); }
    int len = 0;
    for (int i = 0; i < 6 && len < static_cast<int>(sizeof lens) - 12; ++i) {
        char f[16];
        fld(f, sizeof f, S_WB, wb, kWbLens + i * 4);
        len += snprintf(lens + len, sizeof lens - len, "%s%s", i ? "," : "", f);
    }
    char hubf[16];
    fld(hubf, sizeof hubf, S_WB, wb, kWbHub);

    char wMaskBuf[16] = "-", wSlotsBuf[64] = "-";
    if (okW && weapon != 0) {
        uint32_t wMask = 0;
        bool okMask = rd(S_ITEM, weapon, wbu::kItemUpgradesMaskOff, &wMask);
        wbu::fmtVal(wMaskBuf, sizeof wMaskBuf, okMask, wMask);
        uint32_t wSlots[6] = {};
        bool okSlots[6] = {};
        for (int i = 0; i < 6; ++i) {
            okSlots[i] = rd(S_ITEM, weapon, wbu::kItemUpgradeSlot0Off + i * 4, &wSlots[i]);
        }
        wbu::fmtUpgradeSlots(wSlotsBuf, sizeof wSlotsBuf, wSlots, okSlots);
    }

    logf("WB %s wb=%lx hub@3dc0=%s mode@3d2c=%s weapon@3d34=%s w+1d0=%s w+4=%s staged@3d38=%s lens@3d54[0..5]=%s wUpgMask=%s w%s",
         why, U(wb), hubf, m, w, w1d0, w4, s, lens, wMaskBuf, wSlotsBuf);
    if (weapon != 0) snapItemProps("wb-weapon", weapon);
    if (staged != 0) snapItemProps("wb-staged", staged);
}

void hit(int idx, const char* name, uintptr_t self, uintptr_t ret) {
    DWORD now = GetTickCount();
    unsigned long since = g_hitTick[idx] ? static_cast<unsigned long>(now - g_hitTick[idx]) : 0;
    g_hitTick[idx] = now;
    ++g_hits[idx];
    loadConfig();
    if (g_cfgVerbose) {
        logf("%s hits=%u since_prev_hit=%lums this=%lx ret=%lx", name, g_hits[idx], since, U(self), U(ret));
    }
}

void popModalPanel(uintptr_t panel) {
    if (!panel) return;
    uint32_t mgr = 0;
    if (!readAt(S_PANEL, panel, kHubMgr, &mgr) || !mgr) {
        if (g_hub) readAt(S_HUB, g_hub, kHubMgr, &mgr);
    }
    if (!mgr) return;

    typedef void (__thiscall* FnSetPanelFlags)(void*, int);
    reinterpret_cast<FnSetPanelFlags>(kAddrSetPanelFlags)(reinterpret_cast<void*>(panel), 0);

    typedef void (__thiscall* FnPopModal)(void*);
    reinterpret_cast<FnPopModal>(kAddrPopModalPanel)(reinterpret_cast<void*>(mgr));
}

void resetWbuColorState();
void handleSwitchClick(wbu::ScreenState fromState, uintptr_t fromPanel) {
    if (!g_hub) {
        logf("[WBU] handleSwitchClick: null g_hub, aborting");
        return;
    }
    if (g_switchInProgress) {
        logf("[WBU] handleSwitchClick: switch in progress, ignoring click");
        return;
    }

    wbu::SwitchContext ctx = {};
    ctx.state = fromState;

    uint32_t valNoUpgrade = g_h2NoUpgrade;
    uint32_t valNoCreate = g_h2NoCreate;
    ctx.disableCreate = (fromState == wbu::ScreenState::UPGRADE_LIST) && (valNoCreate != 0);
    // The bench dialog's Create option passes noUpgrade=1, so on CREATE the flag does not mean "no upgrade side" (T3 never opens CREATE).
    ctx.disableUpgrade = (fromState != wbu::ScreenState::CREATE) && (valNoUpgrade != 0);

    uintptr_t mgr = mgrOfHub(g_hub);
    if (!mgr && fromPanel) mgr = mgrOfHub(fromPanel);
    uint32_t curDepth = 0;
    bool okDepth = rd(S_MGR, mgr, kMgrL94n, &curDepth);
    ctx.subDialogOpen = okDepth && (g_activeScreenBaseDepth > 0) && (curDepth > g_activeScreenBaseDepth);

    ctx.inTransition = g_switchInProgress;
    ctx.hub = g_hub;

    uint32_t createPtr = 0, itemPtr = 0;
    if (rd(S_HUB, g_hub, kHubItemList, &itemPtr) && itemPtr) ctx.itemListPanel = itemPtr;
    else if (g_activeUpgradePanel) ctx.itemListPanel = g_activeUpgradePanel;

    if (rd(S_HUB, g_hub, kHubCreatePanel, &createPtr) && createPtr) ctx.createPanel = createPtr;
    else if (g_activeCreatePanel) ctx.createPanel = g_activeCreatePanel;
    else ctx.createPanel = g_hub + kHubCreatePanel;

    DWORD now = GetTickCount();
    ctx.elapsedMsSinceLastSwitch = g_lastSwitchTick ? static_cast<uint32_t>(now - g_lastSwitchTick) : 1000;

    uint32_t partyId = 0;
    rd(S_HUB, g_hub, kHubPartyMember, &partyId);
    ctx.partyMemberId = partyId;

    wbu::ScreenState nextState = fromState;
    uint32_t preservedPartyIdx = partyId;
    wbu::SwitchAction action = wbu::evaluateSwitchClick(ctx, &nextState, &preservedPartyIdx);
    if (action == wbu::SwitchAction::NONE) {
        logf("[WBU] evaluateSwitchClick returned NONE (subDialog=%d inTransition=%d elapsed=%u hub=%lx)",
             ctx.subDialogOpen, ctx.inTransition, ctx.elapsedMsSinceLastSwitch, U(ctx.hub));
        return;
    }

    if (action == wbu::SwitchAction::OPEN_UPGRADE) {
        g_switchInProgress = true;
        logf("[WBU] SWITCH CREATE->UPGRADE hub=%lx party=%u", U(g_hub), preservedPartyIdx);

        snapSel("switch-before-pop", g_hub);
        popModalPanel(fromPanel);
        g_activeCreatePanel = 0;
        *(uint32_t*)(g_hub + kHubPartyMember) = preservedPartyIdx;
        snapSel("switch-after-pop", g_hub);
        // Mimic the dialog's Upgrade option: show the hub's own "Select Upgrade Item" screen with nothing selected.
        // If Create was opened from the hub button the hub was popped, so push it back; if the hub was opened first (Create on top) it is already underneath.
        if (g_createViaHub) {
            typedef void (__thiscall* FnMgrPush)(void* mgr, void* panel, unsigned flags, int closeFlag);
            const uintptr_t mgr = mgrOfHub(g_hub);
            if (mgr) reinterpret_cast<FnMgrPush>(kAddrMgrPush)(reinterpret_cast<void*>(mgr), reinterpret_cast<void*>(g_hub), 3, 1);
            logf("[WBU] SWITCH pushed hub back mgr=%lx", U(mgr));
            g_createViaHub = false;
        }
        snapSel("switch-after-hub", g_hub);

        g_screenState = nextState;
        g_lastSwitchTick = GetTickCount();
        g_switchInProgress = false;
        KP_LOG(chMain, "[WBU] SWITCH CREATE->UPGRADE hub=%p party=%d", reinterpret_cast<void*>(g_hub), preservedPartyIdx);
    } else if (action == wbu::SwitchAction::OPEN_CREATE) {
        g_switchInProgress = true;
        logf("[WBU] SWITCH UPGRADE->CREATE hub=%lx party=%u", U(g_hub), preservedPartyIdx);

        g_createViaHub = false;   // Create opened by this path: the hub is not popped by us
        popModalPanel(fromPanel);
        g_activeUpgradePanel = 0;
        resetWbuColorState();
        *(uint32_t*)(g_hub + kHubPartyMember) = preservedPartyIdx;

        typedef void (__thiscall* FnOnCreateItem)(void* hub, void* btn);
        reinterpret_cast<FnOnCreateItem>(kAddrOnCreateItem)(reinterpret_cast<void*>(g_hub), reinterpret_cast<void*>(g_hub + kHubCreateBtn));

        g_screenState = nextState;
        g_lastSwitchTick = GetTickCount();
        g_switchInProgress = false;
        KP_LOG(chMain, "[WBU] SWITCH UPGRADE->CREATE hub=%p party=%d", reinterpret_cast<void*>(g_hub), preservedPartyIdx);
    }
}

// Finds a layout control on `panel` by its pristine extent (the member offsets are not anchored, so scan the control list instead).
uintptr_t findCtlByRect(uintptr_t panel, int x, int y, int w, int h, int tol = 0) {
    int arr = 0, cnt = 0;
    if (!guikit::detail::rdInt(panel, guikit::Panel_CtlArray, &arr) || !guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &cnt) || !arr) return 0;
    if (cnt < 0 || cnt > kCountCap) return 0;
    for (int i = 0; i < cnt; ++i) {
        int c = 0;
        if (!guikit::detail::rdInt(arr, 4 * i, &c) || !c) continue;
        int cx, cy, cw, ch;
        if (guikit::getExtent(static_cast<uintptr_t>(c), &cx, &cy, &cw, &ch) && abs(cx - x) <= tol && abs(cy - y) <= tol && abs(cw - w) <= tol && abs(ch - h) <= tol) return static_cast<uintptr_t>(c);
    }
    return 0;
}

// GUI units (800x600 layout) -> engine px. getExtent/setRect work in engine px, so every layout rect must go through this map.
// The reference is the cloned template button (it still carries the stock extent before setRect): x from its x/width, y from the screen height.
bool cmpScreen(int* w, int* h);
static inline void cmpScreenFwd(int* w, int* h) { cmpScreen(w, h); }
struct GuiMap { bool ok = false; double ox = 0, oy = 0, sx = 1, sy = 1; };
GuiMap g_gm;
void toPx(const int (&g)[4], int (&o)[4]) {
    o[0] = static_cast<int>(g_gm.ox + g[0] * g_gm.sx + 0.5); o[1] = static_cast<int>(g_gm.oy + g[1] * g_gm.sy + 0.5);
    o[2] = static_cast<int>(g[2] * g_gm.sx + 0.5);           o[3] = static_cast<int>(g[3] * g_gm.sy + 0.5);
}

// DLL-only switch button (no GUI override): clone the layout button `templateTag` (BTN_Cancel / BTN_BACK) while the layout GFF is open
// (inside StopLoadFromLayout), append it, place it, label it and bind the click. Returns the button or nullptr (nothing left half-built).
void* bindSwitchButton(uintptr_t panel, const char* templateTag, void* callbackFn, const char* label, const int (&r)[4], const int (&tmplGui)[4]) {
    if (!panel || !callbackFn || !templateTag) return nullptr;
    int why = 0; uint32_t badVt = 0; bool canary = false;
    const uintptr_t btn = guibutton::createButtonFromTemplate(panel, templateTag, &why, &badVt, &canary);
    if (!btn) { logf("[WBU] BTN_SWITCH clone of %s failed why=%d vt=%08x panel=%lx", templateTag, why, badVt, U(panel)); return nullptr; }
    if (!canary) logf("[WBU] BTN_SWITCH WARN ctor overran the button block (canary)");
    if (!guikit::appendToPanel(panel, btn)) { logf("[WBU] BTN_SWITCH append failed, button left hidden unlinked"); return nullptr; }
    g_gm = GuiMap();
    {
        int ex = 0, ey = 0, ew = 0, eh = 0, sw = 0, sh = 0;
        cmpScreenFwd(&sw, &sh);
        if (guikit::getExtent(btn, &ex, &ey, &ew, &eh) && ew > 0 && eh > 0 && sh > 0) {
            g_gm.sx = static_cast<double>(ew) / tmplGui[2];
            g_gm.sy = static_cast<double>(sh) / 600.0;
            g_gm.ox = ex - tmplGui[0] * g_gm.sx;
            g_gm.oy = ey - tmplGui[1] * g_gm.sy;
            g_gm.ok = true;
        }
        logf("[WBU] guimap template=%s ext=[%d,%d,%d,%d] screen=%dx%d ok=%d sx=%.3f sy=%.3f ox=%.1f oy=%.1f", templateTag, ex, ey, ew, eh, sw, sh, g_gm.ok ? 1 : 0, g_gm.sx, g_gm.sy, g_gm.ox, g_gm.oy);
    }
    int rp[4]; toPx(r, rp);
    guikit::setRect(btn, rp[0], rp[1], rp[2], rp[3]);
    const bool okTxt = guibutton::setButtonText(btn, label);
    guikit::setVisible(btn, true);
    int ctlId = -1;
    guikit::detail::rdInt(btn, guikit::Ctl_Id, &ctlId);
    typedef void (__thiscall* FnAddEvent)(void*, int, void*, void*);
    reinterpret_cast<FnAddEvent>(kAddrAddEvent)(reinterpret_cast<void*>(btn), 0x27, reinterpret_cast<void*>(panel), callbackFn);
    logf("[WBU] BTN_SWITCH \"%s\" built from %s id=%d rect=[%d,%d,%d,%d] text=%s", label, templateTag, ctlId, r[0], r[1], r[2], r[3], okTxt ? "ok" : "FAILED");
    return reinterpret_cast<void*>(btn);
}

// Moves a pristine layout button (found by its pristine rect) to a new rect; logs when it is not found.
void moveCtl(uintptr_t panel, const int (&from)[4], const int (&to)[4], const char* what) {
    if (!g_gm.ok) {   // no template clone this open (switch hidden): the live log (3840x1600) shows the GUI stretched edge to edge: sx=sw/800, sy=sh/600, origin 0
        int sw = 0, sh = 0; cmpScreenFwd(&sw, &sh);
        if (sw > 0 && sh > 0) { g_gm.sy = sh / 600.0; g_gm.sx = sw / 800.0; g_gm.ox = 0; g_gm.oy = 0; g_gm.ok = true; }
    }
    int fp[4], tp[4]; toPx(from, fp); toPx(to, tp);
    const uintptr_t c = findCtlByRect(panel, fp[0], fp[1], fp[2], fp[3], 3);
    if (!c) { logf("[WBU] layout: %s not found at px [%d,%d,%d,%d] (gui [%d,%d,%d,%d] map ok=%d)", what, fp[0], fp[1], fp[2], fp[3], from[0], from[1], from[2], from[3], g_gm.ok ? 1 : 0); return; }
    guikit::setRect(c, tp[0], tp[1], tp[2], tp[3]);
}

// Logs every panel control's extent in GUI units (post-layout) so a missing or misplaced button can be read from the log.
void dumpCtlRects(uintptr_t panel, const char* tag) {
    int arr = 0, cnt = 0;
    if (!guikit::detail::rdInt(panel, guikit::Panel_CtlArray, &arr) || !guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &cnt) || !arr || cnt < 0 || cnt > kCountCap) return;
    const double sx = g_gm.ok && g_gm.sx > 0 ? g_gm.sx : 1.0, sy = g_gm.ok && g_gm.sy > 0 ? g_gm.sy : 1.0;
    for (int i = 0; i < cnt; ++i) {
        int c = 0, id = -1, x, y, w, h;
        if (!guikit::detail::rdInt(arr, 4 * i, &c) || !c) continue;
        guikit::detail::rdInt(static_cast<uintptr_t>(c), guikit::Ctl_Id, &id);
        if (!guikit::getExtent(static_cast<uintptr_t>(c), &x, &y, &w, &h)) continue;
        logf("[WBU] RECT %s #%d id=%d ctl=%x px=[%d,%d,%d,%d] gui=[%d,%d,%d,%d]", tag, i, id, c, x, y, w, h,
             static_cast<int>((x - g_gm.ox) / sx + 0.5), static_cast<int>((y - g_gm.oy) / sy + 0.5), static_cast<int>(w / sx + 0.5), static_cast<int>(h / sy + 0.5));
    }
}

// ---- Crafting Compare Box (CmpBox) ----
constexpr int CmpRows = 7;        // 1 header + 1 column heading + up to 5 formatter rows; falls back to 6 slots (no heading) when the panel lacks a 7th label pair
constexpr int CmpMinRows = 6;
constexpr int CmpDescIdx = 26;
constexpr int PanelDescListOff = 0x2670;

struct CmpBox {
    uintptr_t panel;
    uintptr_t lb;
    int lbIdx;
    uintptr_t frame;
    int frameIdx;
    uintptr_t name[CmpRows];
    uintptr_t val[CmpRows];
    int nameIdx[CmpRows];
    int valIdx[CmpRows];
    int lbX, lbY, lbW, lbH;
    int boxX, boxY, boxW, boxH;
    bool have, shrunk;
    int reapplied;
    int n;   // label rows actually built (CmpMinRows..CmpRows)
};

CmpBox g_wbCmp = {};

// Hidden, click-through clone of a layout label, appended to panel
[[maybe_unused]] uintptr_t stripLabel(uintptr_t panel, const char* tag, const char* what, int* idx, bool required) {
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tag, &why, &bv);
    if (!c) {
        if (required) logf("[WBU-CMP] WARN %s label (%s) create failed why=%d vtable=%08x", what, tag, why, (unsigned)bv);
        return 0;
    }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    int before = -1;
    guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &before);
    if (!guikit::appendToPanel(panel, c)) {
        logf("[WBU-CMP] WARN %s label append failed (control %08lx left hidden)", what, U(c));
        return 0;
    }
    if (idx) *idx = before;
    return c;
}

bool cmpIdxOk(int arr, int cnt, int idx, uintptr_t c) {
    int v = 0;
    return c && idx >= 0 && idx < cnt && guikit::detail::rdInt(arr, 4 * idx, &v) && static_cast<uintptr_t>(v) == c;
}

[[maybe_unused]] bool cmpLive(uintptr_t panel) {
    if (!panel || g_wbCmp.panel != panel || !g_wbCmp.name[0] || !g_wbCmp.val[0]) return false;
    int arr = 0, cnt = 0;
    if (!guikit::detail::rdInt(panel, guikit::Panel_CtlArray, &arr) || !guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &cnt)) return false;
    if (!arr) return false;
    if (g_wbCmp.frame && !cmpIdxOk(arr, cnt, g_wbCmp.frameIdx, g_wbCmp.frame)) return false;
    for (int i = 0; i < g_wbCmp.n; ++i) {
        if (!cmpIdxOk(arr, cnt, g_wbCmp.nameIdx[i], g_wbCmp.name[i]) || !cmpIdxOk(arr, cnt, g_wbCmp.valIdx[i], g_wbCmp.val[i])) return false;
    }
    if (g_wbCmp.lb) {
        int offVal = 0;
        const bool inArr = g_wbCmp.lbIdx >= 0 ? cmpIdxOk(arr, cnt, g_wbCmp.lbIdx, g_wbCmp.lb)
                                             : (guikit::detail::rdInt(panel, PanelDescListOff, &offVal) && static_cast<uintptr_t>(offVal) == g_wbCmp.lb);
        if (!inArr) return false;
    }
    return true;
}

bool cmpScreen(int* w, int* h) {
    *w = *h = 0;
    if (guikit::g_gk.screenWAddr) guikit::detail::rdInt(guikit::g_gk.screenWAddr, 0, w);
    if (guikit::g_gk.screenHAddr) guikit::detail::rdInt(guikit::g_gk.screenHAddr, 0, h);
    if (*w >= 320 && *w <= 16384 && *h >= 200 && *h <= 16384) return true;
    *w = 800; *h = 600;
    return true;
}

bool cmpDescShape(uintptr_t c) {
    int x = 0, y = 0, w = 0, h = 0;
    return c && guikit::getExtent(c, &x, &y, &w, &h) && w >= 200 && h >= 250;
}

uintptr_t cmpFindDesc(uintptr_t panel, int* idx) {
    int arr = 0, cnt = 0;
    guikit::detail::rdInt(panel, guikit::Panel_CtlArray, &arr);
    guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &cnt);
    int cVal = 0;
    uintptr_t c = (arr && CmpDescIdx < cnt && guikit::detail::rdInt(arr, 4 * CmpDescIdx, &cVal)) ? static_cast<uintptr_t>(cVal) : 0;
    *idx = CmpDescIdx;
    if (!cmpDescShape(c)) {
        if (arr && (CmpDescIdx + 1) < cnt && guikit::detail::rdInt(arr, 4 * (CmpDescIdx + 1), &cVal) && cmpDescShape(static_cast<uintptr_t>(cVal))) {
            c = static_cast<uintptr_t>(cVal);
            *idx = CmpDescIdx + 1;
        } else {
            int offVal = 0;
            if (guikit::detail::rdInt(panel, PanelDescListOff, &offVal)) c = static_cast<uintptr_t>(offVal);
            *idx = -1;
        }
    }
    return cmpDescShape(c) ? c : 0;
}

bool isUpgradePanel(uintptr_t panel) {
    if (!panel) return false;
    int idx = -1;
    return cmpFindDesc(panel, &idx) != 0;
}

[[maybe_unused]] bool cmpShrink(uintptr_t panel, int nRows = 6) {
    if (!g_wbCmp.lb) g_wbCmp.lb = cmpFindDesc(panel, &g_wbCmp.lbIdx);
    if (!g_wbCmp.lb) {
        logf("[WBU-CMP] LB_DESC not found (panel %08lx), box off", U(panel));
        return false;
    }
    int x = 0, y = 0, w = 0, h = 0;
    if (!guikit::getExtent(g_wbCmp.lb, &x, &y, &w, &h)) return false;
    if (!g_wbCmp.have) {
        if (w < 200 || h < 250) return false;   // not laid out yet
        g_wbCmp.lbX = x; g_wbCmp.lbY = y; g_wbCmp.lbW = w; g_wbCmp.lbH = h; g_wbCmp.have = true;
    }
    int sw = 0, sh = 0;
    cmpScreen(&sw, &sh);
    const wbu::BoxGeoIn in{g_wbCmp.lbX, g_wbCmp.lbY, g_wbCmp.lbW, g_wbCmp.lbH, sh, nRows};
    const wbu::BoxGeoOut geo = wbu::computeBoxGeo(in);
    if (!geo.valid) return false;

    g_wbCmp.boxX = geo.boxX;
    g_wbCmp.boxY = geo.boxY;
    g_wbCmp.boxW = geo.boxW;
    g_wbCmp.boxH = geo.boxH;
    int nh = geo.newLbH;

    if (x == g_wbCmp.lbX && y == g_wbCmp.lbY && w == g_wbCmp.lbW && h == nh) {
        g_wbCmp.shrunk = true;
        g_wbCmp.reapplied = 0;   // the shrink held: count only back-to-back engine moves, not one per selection
        return true;
    }
    if (g_wbCmp.shrunk && ++g_wbCmp.reapplied > 3) {
        logf("[WBU-CMP] LB_DESC moved %d times in one selection, box hidden", g_wbCmp.reapplied);
        return false;
    }
    guikit::setRect(g_wbCmp.lb, g_wbCmp.lbX, g_wbCmp.lbY, g_wbCmp.lbW, nh);
    g_wbCmp.shrunk = true;
    return true;
}

void cmpRestoreList() {
    if (!g_wbCmp.shrunk || !g_wbCmp.lb || !g_wbCmp.have) return;
    guikit::setRect(g_wbCmp.lb, g_wbCmp.lbX, g_wbCmp.lbY, g_wbCmp.lbW, g_wbCmp.lbH);
    g_wbCmp.shrunk = false;
}

void cmpHide(uintptr_t panel) {
    if (!g_wbCmp.shrunk) return;
    if (!cmpLive(panel)) return;
    if (g_wbCmp.frame) guikit::setVisible(g_wbCmp.frame, false);
    for (int i = 0; i < g_wbCmp.n; ++i) {
        guikit::setVisible(g_wbCmp.name[i], false);
        guikit::setVisible(g_wbCmp.val[i], false);
    }
    cmpRestoreList();
}

[[maybe_unused]] void cmpColor(uintptr_t c, float r, float g, float b) {
    float cur[3];
    if (guikit::getTextColor(c, cur)) guikit::setTextColor(c, r, g, b);
}

[[maybe_unused]] void cmpRowColor(uintptr_t c, const wbu::CompareRow& row, bool valueCol) {
    (void)valueCol;
    wbu::RgbColor col = wbu::rowDrawColor(row);
    cmpColor(c, col.r, col.g, col.b);
}

[[maybe_unused]] void cmpDrawCrafting(const wbu::CraftingCompareOut& out) {
    int sw = 0, sh = 0;
    if (!cmpScreen(&sw, &sh)) return;
    const int slots = g_wbCmp.n;
    // Column heading under the header, only when some row is split into name/value and a 7th label pair exists.
    bool anySplit = false;
    for (int i = 1; i < out.nRows; ++i) anySplit = anySplit || (out.rows[i].hasSplit && out.rows[i].rightCol[0] != '\0');
    const bool heading = anySplit && slots > CmpMinRows && out.nRows >= 2;
    int effRows = out.nRows + (heading ? 1 : 0);
    if (effRows < 2) effRows = 2;
    if (slots > 0 && effRows > slots) effRows = slots;

    int pad = 6 * sh / 600;
    if (pad < 2) pad = 2;
    const int rowH = (g_wbCmp.boxH - 2 * pad) / effRows;
    const int innerW = g_wbCmp.boxW - 2 * pad;
    const int nameW = innerW * 40 / 100;
    const textfit::TextCtx tc = { sh / 600.0f, 1.03f, 1.0f };
    if (g_wbCmp.frame) {
        guikit::setRect(g_wbCmp.frame, g_wbCmp.boxX, g_wbCmp.boxY, g_wbCmp.boxW, g_wbCmp.boxH);
        guikit::setVisible(g_wbCmp.frame, true);
    }
    int slotOf[wbu::kMaxCompareRows + 1];
    for (int i = 0; i < wbu::kMaxCompareRows; ++i) slotOf[i] = i + ((heading && i >= 1) ? 1 : 0);
    bool slotUsed[CmpRows] = {};
    for (int i = 0; i < out.nRows && i < wbu::kMaxCompareRows; ++i) {
        const wbu::CompareRow& cr = out.rows[i];
        if (!(cr.leftCol[0] != '\0' || cr.fullLine[0] != '\0')) continue;
        const int sl = slotOf[i];
        if (sl >= slots) continue;
        slotUsed[sl] = true;
        const int y = g_wbCmp.boxY + pad + sl * rowH;
        char nm[64] = {0}, vl[64] = {0};
        if (cr.hasSplit && cr.rightCol[0] != '\0') {
            textfit::fitText(tc, cr.leftCol, static_cast<float>(nameW - 4), nm, sizeof nm, textfit::F10);
            textfit::fitText(tc, cr.rightCol, static_cast<float>(innerW - nameW - 8), vl, sizeof vl, textfit::F10);
            guikit::setRect(g_wbCmp.name[sl], g_wbCmp.boxX + pad, y, nameW, rowH);
            guikit::setRect(g_wbCmp.val[sl], g_wbCmp.boxX + pad + nameW, y, innerW - nameW, rowH);
            guikit::setText(g_wbCmp.name[sl], nm);
            cmpColor(g_wbCmp.name[sl], wbu::kNameCyan.r, wbu::kNameCyan.g, wbu::kNameCyan.b); // Name: vanilla cyan
            guikit::setText(g_wbCmp.val[sl], vl);
            cmpRowColor(g_wbCmp.val[sl], cr, true);
            guikit::setVisible(g_wbCmp.name[sl], true);
            guikit::setVisible(g_wbCmp.val[sl], true);
        } else {
            const char* full = cr.fullLine[0] ? cr.fullLine : cr.leftCol;
            textfit::fitText(tc, full, static_cast<float>(innerW - 8), nm, sizeof nm, textfit::F10);
            guikit::setRect(g_wbCmp.name[sl], g_wbCmp.boxX + pad, y, innerW, rowH);
            guikit::setText(g_wbCmp.name[sl], nm);
            cmpRowColor(g_wbCmp.name[sl], cr, false);
            guikit::setVisible(g_wbCmp.name[sl], true);
            guikit::setVisible(g_wbCmp.val[sl], false);
        }
    }
    if (heading) {
        const int sl = 1;
        slotUsed[sl] = true;
        const int y = g_wbCmp.boxY + pad + sl * rowH;
        guikit::setRect(g_wbCmp.name[sl], g_wbCmp.boxX + pad, y, nameW, rowH);
        guikit::setRect(g_wbCmp.val[sl], g_wbCmp.boxX + pad + nameW, y, innerW - nameW, rowH);
        guikit::setText(g_wbCmp.name[sl], "Stat");
        guikit::setText(g_wbCmp.val[sl], "Change");
        cmpColor(g_wbCmp.name[sl], wbu::kVanillaCyan.r * 0.7f, wbu::kVanillaCyan.g * 0.7f, wbu::kVanillaCyan.b * 0.7f);
        cmpColor(g_wbCmp.val[sl], wbu::kVanillaCyan.r * 0.7f, wbu::kVanillaCyan.g * 0.7f, wbu::kVanillaCyan.b * 0.7f);
        guikit::setVisible(g_wbCmp.name[sl], true);
        guikit::setVisible(g_wbCmp.val[sl], true);
    }
    for (int sl = 0; sl < slots; ++sl) {
        if (slotUsed[sl]) continue;
        guikit::setVisible(g_wbCmp.name[sl], false);
        guikit::setVisible(g_wbCmp.val[sl], false);
    }
}

void onUpgradePanelBuild(uintptr_t cp) {
    loadConfig();
    g_wbCmp = {};
    wbu::resetCrystalPreview(&g_crystalPreview);
    // On upgrade_p.gui (CSWGuiUpgrade), +0x3d38 (kWbStaged) is the cloned working weapon,
    // NOT a candidate upgrade item, and LB_DESC displays the assembled weapon's stats while
    // LBL_PROPERTY already displays the selected upgrade item's properties.
    // Drawing CmpBox shrank LB_DESC and compared the entire weapon against an installed lens.
    // Compare box is disabled on upgrade_p.gui so LB_DESC stays pristine and full-height.
    cmpHide(cp);
    logf("[WBU-CMP] upgrade assembly panel %08lx: compare box disabled, LB_DESC left full-height", U(cp));
}

void callUpdate3DView(uintptr_t wb) {
    if (!wb) return;
    typedef void (__fastcall* FnUpdate3DView)(void*);
    reinterpret_cast<FnUpdate3DView>(kAddrUpdate3DView)(reinterpret_cast<void*>(wb));
}

// LB_ITEMS live rect captured (engine px) before the first inline resize; restored verbatim. A hard-coded 4:3 constant widened the
// lens/hilt list on ultrawide (user screenshot 2026-11-09), so the live rect is the only source of truth.
uintptr_t g_lbCapPanel = 0;
int g_lbCap[4] = {};
bool g_lbInline = false;
char g_colorSentence[256] = {};   // flavour sentence of the colour crystal last shown in the right column
bool g_colorSlotSel = false;      // colour-crystal slot is the selected slot
char g_boxText[256] = {};          // text WE last wrote into LBL_PROPERTY (cleared only if the box still holds exactly this)
bool g_fillDue = false;           // picker just closed on the colour slot: write the cached sentence once, unconditionally
void resetWbuColorState() { g_lbInline = false; g_colorSlotSel = false; g_colorSentence[0] = 0; g_boxText[0] = 0; g_fillDue = false; }

void setUpgradeLbItemsRect(uintptr_t wb, const int (&guiRect)[4]) {
    if (!wb) return;
    if (!g_gm.ok) {
        int sw = 0, sh = 0; cmpScreenFwd(&sw, &sh);
        if (sw > 0 && sh > 0) { g_gm.sy = sh / 600.0; g_gm.sx = sw / 800.0; g_gm.ox = 0; g_gm.oy = 0; g_gm.ok = true; }
    }
    if (!g_lbInline || g_lbCapPanel != wb) {
        int x = 0, y = 0, w = 0, h = 0;
        if (!guikit::getExtent(wb + wbu::kWbLbItemsOff, &x, &y, &w, &h) || w <= 0 || h <= 0) return;   // no live rect, no resize
        g_lbCapPanel = wb; g_lbCap[0] = x; g_lbCap[1] = y; g_lbCap[2] = w; g_lbCap[3] = h;
        logf("[WBU-P7] LB_ITEMS live rect captured px=%d,%d,%d,%d", x, y, w, h);
    }
    int rp[4]; toPx(guiRect, rp);
    if (&guiRect == &wbu::kCrystalInlineListRect) {
        // Ultrawide: the 4:3 constant is wider than the pillarboxed bottom boxes (log 2026-11-09: LBL_SLOTNAME/LBL_PROPERTY sit at gui x=163 w=255).
        // Span exactly the live name + property boxes instead (panel+0x3548 / +0x3400, ids 10 / 9).
        int nx, ny, nw, nh, px, py, pw, ph;
        if (!(guikit::getExtent(wb + 0x3548, &nx, &ny, &nw, &nh) && nw > 0 && nh > 0 && guikit::getExtent(wb + 0x3400, &px, &py, &pw, &ph) && pw > 0 && ph > 0)) {
            logf("[WBU-P7] inline list skipped: name/property box rect unreadable");   // never fall back to the 4:3 constant
            return;
        }
        const int x0 = nx < px ? nx : px, x1 = (nx + nw) > (px + pw) ? (nx + nw) : (px + pw);
        const int y0 = ny < py ? ny : py, y1 = (ny + nh) > (py + ph) ? (ny + nh) : (py + ph);
        rp[0] = x0; rp[1] = y0; rp[2] = x1 - x0; rp[3] = y1 - y0;
        logf("[WBU-P7] inline list spans live name+property boxes px=%d,%d,%d,%d", rp[0], rp[1], rp[2], rp[3]);
    }
    guikit::setRect(wb + wbu::kWbLbItemsOff, rp[0], rp[1], rp[2], rp[3]);
    g_lbInline = true;
}

// P9 probe (log only): dumps every panel control with its gui rect and, when it has a text object, the string pointer/length/text.
// Used to find the control behind LBL_PROPERTY [80,388,344,100] and what the engine writes there, per crystal row hover.
void probeDumpTexts(uintptr_t panel, const char* tag) {
    int arr = 0, cnt = 0;
    if (!guikit::detail::rdInt(panel, guikit::Panel_CtlArray, &arr) || !guikit::detail::rdInt(panel, guikit::Panel_CtlCount, &cnt) || !arr || cnt < 0 || cnt > kCountCap) return;
    logf("[WBU-P9] %s panel=%x ctl@+0x3400=%x ctl@+0x3548=%x (ctor 008c9e10 pushes LBL_PROPERTY at +0x3400)", tag, static_cast<unsigned>(panel), static_cast<unsigned>(panel + 0x3400), static_cast<unsigned>(panel + 0x3548));
    const double sx = g_gm.ok && g_gm.sx > 0 ? g_gm.sx : 1.0, sy = g_gm.ok && g_gm.sy > 0 ? g_gm.sy : 1.0;
    for (int i = 0; i < cnt; ++i) {
        int c = 0, id = -1, x, y, w, h;
        if (!guikit::detail::rdInt(arr, 4 * i, &c) || !c) continue;
        if (!guikit::getExtent(static_cast<uintptr_t>(c), &x, &y, &w, &h)) continue;
        guikit::detail::rdInt(static_cast<uintptr_t>(c), guikit::Ctl_Id, &id);
        const int gx = static_cast<int>((x - g_gm.ox) / sx + 0.5), gy = static_cast<int>((y - g_gm.oy) / sy + 0.5);
        const int gw = static_cast<int>(w / sx + 0.5), gh = static_cast<int>(h / sy + 0.5);
        char txt[49] = {};
        int sp = 0, sl = 0;
        const bool tobj = guikit::detail::textObjOk(static_cast<uintptr_t>(c));
        if (tobj) {
            const uintptr_t sub = reinterpret_cast<uintptr_t>(guikit::detail::textSub(static_cast<uintptr_t>(c)));
            guikit::detail::rdInt(sub, 0, &sp);
            guikit::detail::rdInt(sub, 4, &sl);
            if (sp > 0x10000 && sl > 0 && sl < 4096) {
                for (int k = 0; k < 48; k += 4) {
                    int dw = 0;
                    if (!guikit::detail::rdInt(static_cast<uintptr_t>(sp), k, &dw)) break;
                    memcpy(txt + k, &dw, 4);
                }
                for (int k = 0; k < 48; ++k) if (txt[k] && (static_cast<unsigned char>(txt[k]) < 32 || static_cast<unsigned char>(txt[k]) > 126)) txt[k] = '?';
                txt[48] = 0;
                if (sl < 48) txt[sl] = 0;   // stop at the real string end
            }
        }
        logf("[WBU-P9] %s #%d id=%d ctl=%x gui=[%d,%d,%d,%d] txtObj=%d str=%x len=%d \"%s\"", tag, i, id, c, gx, gy, gw, gh, tobj ? 1 : 0, sp, sl, txt);
    }
}

// Puts LB_ITEMS back to the rect captured before the inline resize. No-op when it was never resized on this panel.
void restoreLbItemsRect(uintptr_t wb) {
    if (!wb || !g_lbInline || g_lbCapPanel != wb) return;
    guikit::setRect(wb + wbu::kWbLbItemsOff, g_lbCap[0], g_lbCap[1], g_lbCap[2], g_lbCap[3]);
    g_lbInline = false;
    g_fillDue = g_colorSlotSel;
}

// expectedItem != 0: write only if the panel still holds that exact item (a stale preview must not hit another item).
// The target byte must also be readable before it is written.
void writeWorkingItemModelVar(uintptr_t wb, uint8_t modelVar, uintptr_t expectedItem = 0) {
    if (!wb) return;
    uint32_t item = 0, probe = 0;
    if (!rd(S_WB, wb, wbu::kWbWorkingItemOff, &item) || item == 0) return;
    if (expectedItem != 0 && static_cast<uintptr_t>(item) != expectedItem) return;
    if (!rd(S_ITEM, item, wbu::kItemModelVarOff, &probe)) return;
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(item) + wbu::kItemModelVarOff) = modelVar;
    callUpdate3DView(wb);
}

}  // namespace

extern "C" {

void __fastcall OnSwitchClickFromCreate(void* panel, void* /*edx*/, void* btn) {
    cmpHide(g_wbCmp.panel);
    logf("[WBU] BTN_SWITCH clicked on CREATE panel=%p btn=%p", panel, btn);
    handleSwitchClick(wbu::ScreenState::CREATE, reinterpret_cast<uintptr_t>(panel));
}

void __fastcall OnSwitchClickFromHub(void* panel, void* /*edx*/, void* btn) {
    logf("[WBU] BTN_SWITCH clicked on HUB panel=%p btn=%p", panel, btn);
    uint32_t cp = 0;
    if (!g_hub || !rd(S_HUB, g_hub, kHubCreatePanel, &cp) || !cp) { logf("[WBU] HUB switch: no create panel, ignored"); return; }
    const DWORD nowTick = GetTickCount();
    if (g_lastSwitchTick && nowTick - g_lastSwitchTick < 400) { logf("[WBU] HUB switch: debounced"); return; }
    g_createViaHub = true;
    g_lastSwitchTick = nowTick;
    typedef void (__thiscall* FnOnCreateItem)(void* hub, void* btn);
    reinterpret_cast<FnOnCreateItem>(kAddrOnCreateItem)(reinterpret_cast<void*>(g_hub), reinterpret_cast<void*>(g_hub + kHubCreateBtn));
}

void __fastcall OnSwitchClickFromUpgrade(void* panel, void* /*edx*/, void* btn) {
    cmpHide(g_wbCmp.panel);
    logf("[WBU] BTN_SWITCH clicked on UPGRADE_LIST panel=%p btn=%p", panel, btn);
    handleSwitchClick(wbu::ScreenState::UPGRADE_LIST, reinterpret_cast<uintptr_t>(panel));
}

// H1: FUN_00410530(mgr=ECX, panel, flags, closeFlag): push a panel onto the manager lists. Q1/Q3 core hook.
void __cdecl WbuMgrPush(void* mgr, void* ret, void* panel, unsigned flags, int closeFlag) {
    if (!g_armed) return;
    saferead::beginScope();
    char fb[40] = "??";
    uint32_t fl = 0;
    if (panel && rd(S_PANEL, P(panel), kPanelFlags, &fl)) wbu::fmtPanelFlags(fb, sizeof fb, fl);
    char s19[16] = "??";
    fld(s19, sizeof s19, S_PANEL, P(panel), 0x64);
    if (g_cfgVerbose) {
        logf("H1 MgrPush m=%lx ret=%lx panel=%lx tag=%c slot19=%s flags=%x close=%d panelFlags=%s", U(P(mgr)), U(P(ret)), U(P(panel)), tagPanel(P(panel), g_hub), s19, flags, closeFlag, fb);
    }
    snapMgr("pre-push", P(mgr), g_hub);
    if (tagPanel(P(panel), g_hub) == 'H') snapHub("mgr-push", P(panel));
}

// H2: ShowUpgradeScreen entry. Stack: [esp+4] obj, +8 obj2, +0xC noCreate, +0x10 noUpgrade, +0x14 string (CExoString by value).
void __cdecl WbuShowUpgrade(void* ret, void* a1, void* a2, int noCreate, int noUpgrade) {
    ++g_hits[1];
    g_armed = true;
    // In KotOR 2, clicking a standard placeable workbench runs workbnch.dlg, which reuses k_workbench_t3.ncs
    // (noCreate=1, noUpgrade=0) when choosing "Upgrade Items" and k_workbench_bd.ncs (noCreate=0, noUpgrade=1)
    // when choosing "Create Items". These flags select the initial screen choice rather than a bench capability
    // restriction. We log the raw flags and callerId for diagnostics/future T3 filtering, and allow both paths.
    logf("[WBU] H2 raw flags noCreate=%u noUpgrade=%u", noCreate ? 1u : 0u, noUpgrade ? 1u : 0u);
    g_h2NoCreate = 0;
    g_h2NoUpgrade = 0;
    {
        saferead::beginScope();
        uint32_t vm = 0, vmi = 0, rec = 0, callerId = 0;
        rd(S_PANEL, 0xA1B4A8u, 0, &vm);
        if (vm && rd(S_PANEL, vm, 0x1c, &vmi) && vmi && rd(S_PANEL, vmi, 0x28, &rec) && rec < 32) {
            rd(S_PANEL, vmi, 0x18c + rec * 4, &callerId);
        }
        logf("[WBU] H2 PROBE owner vm=%x vmi=%x rec=%u callerId=%08x (raw flags noCreate=%u noUpgrade=%u)",
             vm, vmi, rec, callerId, noCreate ? 1u : 0u, noUpgrade ? 1u : 0u);
    }
    g_screenState = wbu::ScreenState::HUB;
    logf("H2 ShowUpgradeScreen n=%u ret=%lx obj1=%lx obj2=%lx disableItemCreation=%d disableUpgrade=%d", g_hits[1], U(P(ret)), U(P(a1)), U(P(a2)), noCreate, noUpgrade);
}

// H3: CSWSItem::AddUpgrade (thiscall, ECX = item, [esp+4] = upgrade item, [esp+8] = 2nd arg: 0 at 008ceb00, slot at 008cdb00).
void __cdecl WbuAddUpgrade(void* item, void* ret, void* upg, int arg2) {
    if (g_cfgVerbose) {
        saferead::beginScope();
        char id[16], uid[16];
        fld(id, sizeof id, S_ITEM, P(item), 0x1d0);
        fld(uid, sizeof uid, S_ITEM, P(upg), 0x1d0);
        logf("L3 AddUpgrade item=%lx item+1d0=%s ret=%lx upgrade=%lx upg+1d0=%s arg2=%d", U(P(item)), id, U(P(ret)), U(P(upg)), uid, arg2);
    }
    snapItemProps("add-item", P(item));
    snapItemProps("add-upg", P(upg));
}

// H4: CSWSItem::RemoveUpgrade (thiscall, [esp+4] = arg).
void __cdecl WbuRemoveUpgrade(void* item, void* ret, int arg1, int arg2) {
    if (g_cfgVerbose) {
        saferead::beginScope();
        char id[16];
        fld(id, sizeof id, S_ITEM, P(item), 0x1d0);
        logf("L3 RemoveUpgrade item=%lx item+1d0=%s ret=%lx arg1=%d arg2=%d", U(P(item)), id, U(P(ret)), arg1, arg2);
    }
    snapItemProps("remove-item", P(item));
}

// Small wrapper for CSWSCreatureStats::GetSkillRank (0x006B7BB0, thiscall).
// Calling convention: __thiscall, ECX = stats, [esp+4] = skillId, [esp+8] = vsObj, [esp+12] = baseOnly.
// Null-pointer safe: returns 0 if stats is null.
[[maybe_unused]] inline int callGetSkillRank(uintptr_t stats, uint8_t skillId, uint32_t vsObj = 0, int baseOnly = 0) {
    if (!stats) return 0;
    typedef int8_t (__thiscall* FnGetSkillRank)(void* pStats, uint8_t nSkill, uint32_t oTarget, int nBaseSkillRank);
    const int8_t r = reinterpret_cast<FnGetSkillRank>(kAddrGetSkillRank)(
        reinterpret_cast<void*>(stats), skillId, vsObj, baseOnly);
    return wbu::sanitizeSkillRank(r);
}

// Player effective skill ranks (skills.2da rows 0..7): controlled client creature -> server creature -> stats (+0x1198)
// -> CSWSCreatureStats::GetSkillRank (0x006B7BB0, effective rank with bonuses).
[[maybe_unused]] wbu::SkillView readPlayerSkills() {
    saferead::beginScope();
    wbu::SkillView v = {};
    const int oStats = k2addr::off(&K2AddrTable::Creature_Stats);
    const uintptr_t client = party::controlledCreature();
    uint32_t vt = 0, stats = 0, statVt = 0;
    if (!client || !oStats || !rd(S_PC, client, 0, &vt) || vt == 0) return v;
    const uintptr_t srv = party::serverCreature(client);
    if (!srv || !rd(S_PCSRV, srv, 0, &vt) || vt == 0) return v;
    if (!rd(S_PCSRV, srv, oStats, &stats) || stats == 0) return v;
    if (!rd(S_PCSTAT, stats, 0, &statVt) || statVt == 0) return v;
    for (uint8_t i = 0; i < 8; ++i) {
        v.ranks[i] = callGetSkillRank(stats, i, 0, 0);
    }
    v.readable = true;
    return v;
}

// Compare box for the viewed item against the slot's installed item.
[[maybe_unused]] void refreshCompare(uintptr_t pWb, const char* src) {
    g_wbCmp.reapplied = 0;
    uint32_t weapon = 0;
    // Target weapon pointer at kWbWeapon (0x3d34), fallback to 0x16bc if 0
    if (!rd(S_WB, pWb, kWbWeapon, &weapon) || weapon == 0) {
        rd(S_WB, pWb, 0x16bc, &weapon);
    }

    uint32_t staged = 0;
    rd(S_WB, pWb, kWbStaged, &staged);

    // Read active slot index: local_1c = *(int *)(*(int *)(param_1 + 0x3dbc) + 0x5c) in 008cdb00.c
    uint32_t slotCtrl = 0, slotIdx = 0;
    uint32_t installedItem = 0;
    if (rd(S_WB, pWb, 0x3dbc, &slotCtrl) && slotCtrl != 0) {
        rd(S_WB, slotCtrl, 0x5c, &slotIdx);
        if (slotIdx < 6) {
            rd(S_WB, pWb, kWbLens + slotIdx * 4, &installedItem);
        }
    }

    itemcmp::ItemFacts targetFacts = {};
    itemcmp::ItemFacts installedFacts = {};
    itemcmp::ItemFacts stagedFacts = {};

    bool hasWeapon = (weapon != 0) && readItemFactsBothLists(weapon, &targetFacts);
    bool hasInstalled = (installedItem != 0) && readItemFactsBothLists(installedItem, &installedFacts);
    bool hasStaged = (staged != 0) && readItemFactsBothLists(staged, &stagedFacts);

    const wbu::SkillView skills = readPlayerSkills();
    char skillText[64] = "?";
    if (skills.readable) {
        snprintf(skillText, sizeof(skillText), "%d,%d,%d,%d,%d,%d,%d,%d", skills.ranks[0], skills.ranks[1], skills.ranks[2],
                 skills.ranks[3], skills.ranks[4], skills.ranks[5], skills.ranks[6], skills.ranks[7]);
    }

    wbu::CraftingCompareOut cmpOut = {};
    const itemcmp::ItemFacts* pInstalled = hasInstalled ? &installedFacts : nullptr;
    const itemcmp::ItemFacts* pViewed = hasStaged ? &stagedFacts : (hasWeapon ? &targetFacts : nullptr);

    if (pViewed != nullptr) {
        wbu::formatCraftingCompare(pInstalled, pViewed, &skills, &cmpOut);
        logf("[WBU-CMP] src=%s slot=%u weapon=%lx staged=%lx installed=%lx skills=[%s] | %s | %s (rows=%d delta=%d skillMet=%d)",
             src, static_cast<unsigned>(slotIdx), U(weapon), U(staged), U(installedItem), skillText,
             cmpOut.header, cmpOut.deltaLine, cmpOut.nRows, cmpOut.hasDelta, cmpOut.skillMet ? 1 : 0);
        for (int r = 0; r < cmpOut.nRows && r < wbu::kMaxCompareRows; ++r) {
            wbu::RgbColor col = wbu::rowDrawColor(cmpOut.rows[r]);
            logf("[WBU-CMP]   row[%d] (k=%d unmet=%d rgb=%.2f,%.2f,%.2f): '%s' | '%s'",
                 r, static_cast<int>(cmpOut.rows[r].kind), cmpOut.rows[r].unmet ? 1 : 0, col.r, col.g, col.b,
                 cmpOut.rows[r].leftCol, cmpOut.rows[r].rightCol);
        }
        bool hasSplit = false;
        for (int i = 1; i < cmpOut.nRows; ++i) {
            if (cmpOut.rows[i].rightCol[0] != '\0') hasSplit = true;
        }
        bool heading = hasSplit && g_wbCmp.n > CmpMinRows && cmpOut.nRows >= 2;
        int effRows = cmpOut.nRows + (heading ? 1 : 0);
        if (effRows < 2) effRows = 2;
        if (g_wbCmp.n > 0 && effRows > g_wbCmp.n) effRows = g_wbCmp.n;

        if (cmpLive(pWb) && cmpShrink(pWb, effRows)) {
            cmpDrawCrafting(cmpOut);
        } else {
            cmpHide(pWb);   // a failed shrink must not leave the previous box and text on screen
        }
    } else {
        cmpHide(pWb);
    }
}

// H5: CSWGuiUpgrade::OnUpgradeSelected (ECX = workbench). Event handler, entry detour.
void __cdecl WbuUpgradeSelected(void* wb, void* ret) {
    hit(5, "H5 OnUpgradeSelected", P(wb), P(ret));
    g_screenState = wbu::ScreenState::UPGRADE_SLOTS;
    snapWb("apply", P(wb));

    if (wb != nullptr && g_crystalPreview.active) {
        saferead::beginScope();
        writeWorkingItemModelVar(P(wb), g_crystalPreview.origModelVar, g_crystalPreview.workingItem);
        wbu::commitCrystalPreview(&g_crystalPreview);
    }

    if (wb != nullptr) {
        cmpHide(P(wb));
    }
}

// H6: hub OnCreateItem (ECX = hub). Calls FUN_004122a0(0) (a slot dismiss, NOT a panel pop, doc 55 5d) then FUN_00410530(create panel, 3, 1). Entry-only: the effect shows in the next MGR snapshot (H1 pre-push of the Create panel, then H12/next event).
void __cdecl WbuCreateClick(void* hub, void* ret) {
    g_hub = P(hub);
    hit(6, "H6 OnCreateItem", g_hub, P(ret));
    snapHub("create-click", g_hub);
    snapMgr("create-click", mgrOfHub(g_hub), g_hub);
}

// H7: hub Upgrade click (ECX = hub). Reads hub+0x18c0 unguarded.
void __cdecl WbuUpgradeClick(void* hub, void* ret) {
    g_hub = P(hub);
    hit(7, "H7 UpgradeClick", g_hub, P(ret));
    snapHub("upgrade-click", g_hub);
    snapMgr("upgrade-click", mgrOfHub(g_hub), g_hub);
}

// H8: hub handler (ECX = hub, [esp+4] = event arg): writes +0x18bc from a vtable result.
void __cdecl WbuHub7440(void* hub, void* ret, void* arg) {
    g_hub = P(hub);
    hit(8, "H8 Hub7440", g_hub, P(ret));
    logf("H8 Hub7440 arg=%lx", U(P(arg)));
    snapHub("h7440", g_hub);
}

// H9: hub ctor entry (ECX = new hub; fields not built yet). Control pointers show at the next hub event.
void __cdecl WbuHubCtor(void* hub, void* ret) {
    g_hub = P(hub);
    g_createViaHub = false;
    g_hubLaidOut = 0;   // a new hub may reuse a freed address
    g_armed = true;
    g_screenState = wbu::ScreenState::HUB;
    hit(9, "H9 HubCtor", g_hub, P(ret));
}

// H10: CSWGuiCreateItem ctor (ECX = new Create screen).
void __cdecl WbuCreateCtor(void* self, void* ret) {
    g_pendingCreatePanel = P(self);
    g_activeCreatePanel = P(self);
    hit(10, "H10 CreateCtor", P(self), P(ret));
}

// H11: CSWGuiUpgradeItemSelect ctor (ECX = new item list).
void __cdecl WbuItemListCtor(void* self, void* ret) {
    g_pendingUpgradePanel = P(self);
    g_activeUpgradePanel = P(self);
    resetWbuColorState();
    hit(11, "H11 ItemListCtor", P(self), P(ret));
}

// H12: CSWGuiCreateItem::InitializeStoreType (ECX = Create screen): FUN_004122a0(0) then refresh. Q1: is this the Cancel path?
void __cdecl WbuStoreType(void* self, void* ret) {
    hit(12, "H12 StoreType", P(self), P(ret));
    saferead::beginScope();
    uint32_t backHub = 0;   // Create screen keeps a hub back-pointer at +0x3eec (doc 55 sec 2); fall back to the last hub seen
    uintptr_t hub = rd(S_HUB, P(self), 0x3eec, &backHub) && backHub ? backHub : g_hub;
    if (g_cfgVerbose) {
        logf("H12 StoreType hub=%lx (back-pointer %s)", U(hub), backHub ? "ok" : "unreadable, using last hub");
    }
    snapMgr("store-type", mgrOfHub(hub), hub);
}

// H13: CSWGuiCreateItem::CloseCreateItem (BTN_Cancel handler, ECX = Create screen).
void __cdecl WbuCreateCancel(void* self, void* ret) {
    hit(13, "H13 CreateCancel", P(self), P(ret));
    saferead::beginScope();
    cmpHide(g_wbCmp.panel);
    uint32_t backHub = 0;   // Create screen keeps a hub back-pointer at +0x3eec (doc 55 sec 2)
    uintptr_t hub = rd(S_HUB, P(self), 0x3eec, &backHub) && backHub ? backHub : g_hub;
    if (g_cfgVerbose) {
        logf("H13 CreateCancel hub=%lx (back-pointer %s)", U(hub), backHub ? "ok" : "unreadable, using last hub");
    }
    snapHub("create-cancel", hub);
    snapMgr("create-cancel", mgrOfHub(hub), hub);
    if (!g_switchInProgress) {
        g_screenState = wbu::ScreenState::HUB;
        g_activeCreatePanel = 0;
        logf("[WBU] CREATE closed -> HUB");
    }
}

// H14: CSWGuiPanel::StopLoadFromLayout (ECX = panel).
void __cdecl WbuStopLoadFromLayout(void* panel, void* ret) {
    if (!g_armed || !panel) return;
    guikit::init();
    loadConfig();
    uintptr_t p = P(panel);
    if (p == g_hub && p != g_hubLaidOut) {
        g_hubLaidOut = p;
        // The hub itself is the "Select Upgrade Item" screen (upgradesel_p.gui override rects). Action button under the shortened
        // description box, Back + Create / Breakdown in the footer row.
        // Same rects as the Create screen (kCancelTo / kSwitchTo / kBtnTo / kDescTo below) so the toggle keeps both footer buttons and the action button in place.
        static const int kBackFromH[4] = {239, 514, 93, 26}, kBackToH[4] = {239, 514, 160, 26}, kSwitchToH[4] = {405, 514, 155, 26};
        static const int kDescFromH[4] = {406, 132, 237, 356}, kDescToH[4] = {406, 132, 234, 326};   // bottom edge 458 = Create cost-box bottom (388+70), so the 8-unit gap above the action button matches
        static const int kUpFromH[4] = {335, 514, 225, 26}, kUpToH[4] = {406, 466, 230, 24};

        void* hb = nullptr;
        if (g_cfgSwitchButton != 0 && g_h2NoUpgrade == 0 && g_h2NoCreate == 0) {
            hb = bindSwitchButton(p, "BTN_BACK", reinterpret_cast<void*>(OnSwitchClickFromHub), "Create / Breakdown", kSwitchToH, kBackFromH);
        }
        if (g_cfgSwitchButton != 0) {
            moveCtl(p, kBackFromH, kBackToH, "HUB BTN_BACK");
            moveCtl(p, kDescFromH, kDescToH, "HUB LB_DESCRIPTION");
            moveCtl(p, kUpFromH, kUpToH, "HUB BTN_UPGRADEITEMS");
            dumpCtlRects(p, "HUB");
            logf("[WBU] HUB layout applied panel=%lx switch=%p", U(p), hb);
        } else {
            logf("[WBU] HUB layout skipped (cfg=%d)", g_cfgSwitchButton);
        }
        if (!hb) {
            logf("[WBU] HUB switch button skipped (cfg=%d noUpgrade=%u noCreate=%u)", g_cfgSwitchButton, g_h2NoUpgrade, g_h2NoCreate);
        }
    } else if (isUpgradePanel(p)) {
        onUpgradePanelBuild(p);
    } else if (p == g_pendingCreatePanel) {
        g_pendingCreatePanel = 0;
        g_screenState = wbu::ScreenState::CREATE;
        g_activeCreatePanel = p;

        uintptr_t mgr = mgrOfHub(g_hub ? g_hub : p);
        uint32_t depth = 0;
        if (rd(S_MGR, mgr, kMgrL94n, &depth)) {
            g_activeScreenBaseDepth = depth;
        }

        g_gm = GuiMap();
        int switchBtn = g_cfgSwitchButton;
        const uint32_t noUpgrade = g_h2NoUpgrade;

        void* btn = nullptr;
        if (switchBtn != 0 && wbu::canShowSwitchButton(wbu::ScreenState::CREATE, 0, 0)) {   // noUpgrade=1 is the bench dialog's Create option, still a bench: never pass the live flag on CREATE
            // Layout (doc 55 sec 5e): navigation row at the bottom (Cancel + switch), the item action (Create/Breakdown Item) in the
            // right column under the cost box. Pristine rects: Cancel [239,514,93,26], Accept [335,514,225,26].
            static const int kCancelFrom[4] = {239, 514, 93, 26}, kAcceptFrom[4] = {335, 514, 225, 26};
            static const int kCancelTo[4] = {239, 514, 160, 26}, kAcceptTo[4] = {406, 488, 237, 24}, kSwitchTo[4] = {405, 514, 155, 26};
            btn = bindSwitchButton(p, "BTN_Cancel", reinterpret_cast<void*>(OnSwitchClickFromCreate), "Upgrade Items", kSwitchTo, kCancelFrom);
        }
        if (switchBtn != 0) {
            // Same layout on every bench (store benches have one side disabled and no switch button).
            static const int kCancelFrom2[4] = {239, 514, 93, 26}, kAcceptFrom2[4] = {335, 514, 225, 26};
            static const int kCancelTo2[4] = {239, 514, 160, 26};
            moveCtl(p, kCancelFrom2, kCancelTo2, "BTN_Cancel");
            // Right column, all box edges on x=636 (description list is 4 units wider so its scrollbar sits outside that edge): description box compacted by 30 units, cost box + its labels moved up 30,
            // Create button 8 units under the cost box (clear of the footer rule). Rects are the ultrawide component_p.gui override.
            // "Current Skill" box (LBL_BAR2 frame, LBL_BAR3 + LBL_RELATEDSKILL text) is removed on CREATE to free room for the compare box:
            // the description list takes its top edge (164) and keeps the same bottom edge (382).
            static const int kDescFrom[4] = {406, 206, 237, 206}, kDescTo[4] = {406, 164, 234, 218};
            static const int kDescInvFrom[4] = {406, 164, 237, 248}, kDescInvTo[4] = {406, 164, 234, 218};
            static const int kSkillBarFrom[4] = {406, 164, 230, 36}, kSkillBarTo[4] = {406, 164, 0, 0};
            static const int kSkillBar3From[4] = {417, 163, 207, 20}, kSkillBar3To[4] = {417, 163, 0, 0};
            static const int kSkillRelFrom[4] = {417, 178, 207, 20}, kSkillRelTo[4] = {417, 178, 0, 0};
            static const int kBar1From[4] = {406, 418, 230, 70}, kBar1To[4] = {406, 388, 230, 70};
            static const int kLblX[2] = {416, 542}, kLblW[2] = {137, 85};
            static const int kLblY[3] = {421, 441, 461};
            static const int kBtnTo[4] = {406, 466, 230, 24};
            moveCtl(p, kSkillBarFrom, kSkillBarTo, "CREATE current-skill frame");
            moveCtl(p, kSkillBar3From, kSkillBar3To, "CREATE current-skill bar");
            moveCtl(p, kSkillRelFrom, kSkillRelTo, "CREATE current-skill text");
            moveCtl(p, kDescFrom, kDescTo, "LB_DESCRIPTION");
            moveCtl(p, kDescInvFrom, kDescInvTo, "LB_DESCRIPTIONINV");
            moveCtl(p, kBar1From, kBar1To, "LBL_BAR1");
            for (int row = 0; row < 3; ++row) {
                for (int col = 0; col < 2; ++col) {
                    const int from[4] = {kLblX[col], kLblY[row], kLblW[col], 22}, to[4] = {kLblX[col], kLblY[row] - 30, kLblW[col], 22};
                    moveCtl(p, from, to, "cost label");
                }
            }
            moveCtl(p, kAcceptFrom2, kBtnTo, "BTN_Accept");
        }

        if (btn) {
            logf("[WBU] BTN_SWITCH bound on CREATE panel=%lx btn=%p", U(p), btn);
        } else {
            logf("[WBU] BTN_SWITCH skipped on CREATE panel=%lx (switchBtn=%d noUpgrade=%u)", U(p), switchBtn, noUpgrade);
        }
    } else if (p == g_pendingUpgradePanel) {
        g_pendingUpgradePanel = 0;
        g_screenState = wbu::ScreenState::UPGRADE_LIST;
        g_activeUpgradePanel = p;
        snapSel(g_switchInProgress ? "upgrade-build(switch)" : "upgrade-build(vanilla)", g_hub);

        uintptr_t mgr = mgrOfHub(g_hub ? g_hub : p);
        uint32_t depth = 0;
        if (rd(S_MGR, mgr, kMgrL94n, &depth)) {
            g_activeScreenBaseDepth = depth;
        }

        g_gm = GuiMap();
        int switchBtn = g_cfgSwitchButton;
        const uint32_t noCreate = g_h2NoCreate;

        void* btn = nullptr;
        if (switchBtn != 0 && wbu::canShowSwitchButton(wbu::ScreenState::UPGRADE_LIST, g_h2NoUpgrade, g_h2NoCreate)) {
            // Layout: action (Assemble) under the description in the right column; Back + switch under the item list.
            // Pristine rects: BTN_UPGRADEITEM [113,410,182,28], BTN_BACK [344,410,182,28].
            static const int kUpFrom[4] = {113, 410, 182, 28}, kBackFrom[4] = {344, 410, 182, 28};
            static const int kUpTo[4] = {329, 410, 258, 28}, kBackTo[4] = {55, 410, 132, 28}, kSwitchTo[4] = {193, 410, 132, 28};
            btn = bindSwitchButton(p, "BTN_BACK", reinterpret_cast<void*>(OnSwitchClickFromUpgrade), "Create / Breakdown", kSwitchTo, kBackFrom);
        }
        if (switchBtn != 0) {
            // Same layout on every bench (see CREATE branch).
            static const int kUpFrom2[4] = {113, 410, 182, 28}, kBackFrom2[4] = {344, 410, 182, 28};
            static const int kBackTo2[4] = {55, 410, 132, 28};
            // Action button sits under the (shortened) description box, like the Create screen; Back + switch stay under the item list.
            static const int kDescFrom2[4] = {329, 66, 258, 338}, kDescTo2[4] = {329, 66, 258, 304};
            static const int kUpTo3[4] = {329, 378, 258, 28};
            moveCtl(p, kDescFrom2, kDescTo2, "LB_DESCRIPTION");
            moveCtl(p, kUpFrom2, kUpTo3, "BTN_UPGRADEITEM");
            moveCtl(p, kBackFrom2, kBackTo2, "BTN_BACK");
            dumpCtlRects(p, "UPGRADE_LIST");
        }

        if (btn) {
            logf("[WBU] BTN_SWITCH bound on UPGRADE_LIST panel=%lx btn=%p", U(p), btn);
        } else {
            logf("[WBU] BTN_SWITCH skipped on UPGRADE_LIST panel=%lx (switchBtn=%d noCreate=%u)", U(p), switchBtn, noCreate);
        }
    }
}

// H15: CSWGuiUpgradeItemSelect::HandleInputEvent (ECX = panel, [esp+4] = eventId, [esp+8] = state).
void __cdecl WbuUpgradeSelectInput(void* self, void* ret, void* eventId, void* state) {
    if (!g_armed) return;
    unsigned long ev = reinterpret_cast<uintptr_t>(eventId);
    unsigned long st = reinterpret_cast<uintptr_t>(state);
    if (ev == 0x28 && st == 1) { // BTN_BACK event popping back to Hub
        cmpHide(g_wbCmp.panel);
        if (!g_switchInProgress) {
            g_screenState = wbu::ScreenState::HUB;
            g_activeUpgradePanel = 0;
            logf("[WBU] UPGRADE_LIST closed -> HUB");
        }
    }
}

// H17 (probe, log only): CSWGuiUpgrade::Update3DView 0x008CD2C0 (fastcall, ECX = upgrade screen). Logs what the model-name builder will read:
// screen mode byte +0x3d2c, working item +0x3d34, item base +0x0c, +0x2a4, +0x2a6 (WB-P13: is mode 4 reachable for a lightsaber?).
void __cdecl WbuPreviewRefresh(void* scr, void* ret) {
    const uintptr_t sc = P(scr);
    uint32_t item = 0, base = 0;
    uint32_t mode = 0;
    uint8_t v4 = 0, v6 = 0;
    const bool okM = rd(S_WB, sc, kWbMode, &mode), okI = rd(S_WB, sc, kWbWeapon, &item);
    uint32_t w4 = 0, w6 = 0;
    if (okI && item) {
        rd(S_ITEM, item, 0x0c, &base);
        rd(S_ITEM, item, 0x2a4, &w4);
        rd(S_ITEM, item, 0x2a6, &w6);
        v4 = static_cast<uint8_t>(w4 & 0xff);
        v6 = static_cast<uint8_t>(w6 & 0xff);
    }
    logf("[WBU] PREVIEW refresh scr=%lx ret=%lx mode=%u(%s) item=%lx base=%u +2a4=%u +2a6=%u state=%d",
         U(sc), U(P(ret)), static_cast<unsigned>(mode & 0xff), okM ? "ok" : "unreadable", U(item), base, v4, v6,
         static_cast<int>(g_screenState));
    // The item list (CSWGuiUpgradeItemSelect) never fires H5, so this is the only refresh that reaches it: redraw the compare box for the highlighted component.
    loadConfig();
    if (g_cfgCraftingCompare != 0 && g_armed && g_wbCmp.panel == sc) {
        refreshCompare(sc, "H17");
    }
}

// H16: CGuiInGame::HideUpgradeScreen (ECX = inGameGui).
void __cdecl WbuHideUpgrade(void* inGameGui, void* ret) {
    logf("[WBU] Workbench closed gui=%lx ret=%lx", U(P(inGameGui)), U(P(ret)));
    cmpHide(g_wbCmp.panel);
    g_wbCmp = {};
    wbu::resetCrystalPreview(&g_crystalPreview);
    restoreLbItemsRect(g_activeUpgradePanel);   // put LB_ITEMS / LBL_PROPERTY back if the picker was open at close
    resetWbuColorState();
    g_screenState = wbu::ScreenState::CLOSED;
    g_activeCreatePanel = 0;
    g_activeUpgradePanel = 0;
    g_pendingCreatePanel = 0;
    g_pendingUpgradePanel = 0;
    g_h2NoCreate = 0;
    g_h2NoUpgrade = 0;
    g_hub = 0;
    g_hubLaidOut = 0;
    g_switchInProgress = false;
    g_activeScreenBaseDepth = 0;
    g_createViaHub = false;
    g_gm = {};
    g_armed = false;
}

// H18: CSWGuiUpgrade::OnSlotSelected (ECX = workbench, [esp+0] = ret, [esp+4] = slotBtn).
void __cdecl WbuSlotSelected(void* wb, void* ret, void* slotBtn) {
    (void)ret;
    const uintptr_t sc = P(wb), btn = P(slotBtn);
    if (!sc || !btn) return;
    saferead::beginScope();
    if (g_crystalPreview.active) {
        const uintptr_t prevItem = g_crystalPreview.workingItem;
        const uint8_t orig = wbu::cancelCrystalPreview(&g_crystalPreview);
        writeWorkingItemModelVar(sc, orig, prevItem);
    }
    g_colorSlotSel = false; g_colorSentence[0] = 0; g_fillDue = false;   // re-armed below only for a readable colour slot
    uint32_t modeVal = 0, slotIdxVal = 0, item = 0, mvVal = 0;
    rd(S_WB, sc, wbu::kWbModeOff, &modeVal);
    rd(S_WB, btn, 0x5c, &slotIdxVal);
    const uint8_t mode = static_cast<uint8_t>(modeVal & 0xff);
    const int slotIdx = static_cast<int>(slotIdxVal);
    if (wbu::isColorCrystalSlot(mode, slotIdx)) {
        if (rd(S_WB, sc, wbu::kWbWorkingItemOff, &item) && item != 0 && rd(S_ITEM, item, wbu::kItemModelVarOff, &mvVal)) {
            wbu::beginCrystalPreview(&g_crystalPreview, item, static_cast<uint8_t>(mvVal & 0xff));
            g_colorSlotSel = true;
            setUpgradeLbItemsRect(sc, wbu::kCrystalInlineListRect);
            logf("[WBU-P7] SlotSelected color crystal slot=%d item=%lx origModelVar=%u", slotIdx, U(item), static_cast<unsigned>(mvVal & 0xff));
        }
    } else {
        g_colorSlotSel = false;
        restoreLbItemsRect(sc);
    }
}

// H19: CSWGuiUpgrade::Draw (ECX = workbench, [esp+0] = ret).
void __cdecl WbuUpgradeDraw(void* wb, void* ret) {
    (void)ret;
    if (!wb) return;
    // Name-table-gated fill (not slot-hook gated): the engine pre-selects the crystal slot on open without calling OnSlotSelected (H18).
    saferead::beginScope();
    const uintptr_t sc = P(wb);
    {   // sentence follows the equipped crystal's NAME (slot-name box), so it changes only when OK applies a new crystal, never on row click/hover
        char nm[64] = {};
        int np = 0, nl = 0;
        const bool nameOk = guikit::detail::textObjOk(sc + 0x3548);
        const uintptr_t nsub = nameOk ? reinterpret_cast<uintptr_t>(guikit::detail::textSub(sc + 0x3548)) : 0;
        if (nameOk && guikit::detail::rdInt(nsub, 0, &np) && guikit::detail::rdInt(nsub, 4, &nl) && np > 0x10000 && nl > 0 && nl < 60) {
            bool okRead = true;   // dword copy may read up to 3 bytes past nl; every read is guarded by rdInt, nm is 64 bytes and nl < 60
            for (int k = 0; k < nl; k += 4) { int dw = 0; if (!guikit::detail::rdInt(static_cast<uintptr_t>(np), k, &dw)) { okRead = false; break; } memcpy(nm + k, &dw, 4); }
            if (okRead) {
                nm[nl] = 0;
                const char* line = wbu::colorCrystalSentence(nm, static_cast<size_t>(nl));
                const char* cur = g_colorSentence;
                if (line && strcmp(line, cur) != 0) {
                    snprintf(g_colorSentence, sizeof g_colorSentence, "%s", line);
                    g_fillDue = true;
                    logf("[WBU-P9] colour sentence from slot name \"%s\" -> \"%s\"", nm, line);
                } else if (!line && g_colorSentence[0]) { g_colorSentence[0] = 0; }
            }
        }
    }
    bool listShown = false;
    {   // the box text draws above the list, so keep it hidden while the picker list is visible
        int lf = 0; guikit::detail::rdInt(sc + wbu::kWbLbItemsOff, guikit::Ctl_Flags, &lf);
        listShown = (lf & 2) != 0;
        static int lastShown = -1;
        if (g_cfgVerbose && static_cast<int>(listShown) != lastShown) { lastShown = listShown ? 1 : 0; logf("[WBU-P9] picker list flags %08x shown=%d inline=%d", static_cast<unsigned>(lf), listShown ? 1 : 0, g_lbInline ? 1 : 0); }
        else if (!g_cfgVerbose) lastShown = listShown ? 1 : 0;
        if (listShown && g_colorSentence[0] != 0) {   // only for a matched colour crystal; other slots keep the vanilla box untouched
            int bf = 0; guikit::detail::rdInt(sc + 0x3400, guikit::Ctl_Flags, &bf);
            if (bf & 2) guikit::setVisible(sc + 0x3400, false);
            g_fillDue = true;   // refill/show again once the list is gone
        }
    }
    if (!listShown && g_colorSentence[0] != 0) {   // picker closed on the colour slot: put the flavour sentence in the (vanilla, otherwise empty) LBL_PROPERTY box
        int sp = 0, sl = 0;
        const bool boxOk = guikit::detail::textObjOk(sc + 0x3400);
        const uintptr_t sub = boxOk ? reinterpret_cast<uintptr_t>(guikit::detail::textSub(sc + 0x3400)) : 0;
        if (boxOk) {
            guikit::detail::rdInt(sub, 0, &sp); guikit::detail::rdInt(sub, 4, &sl);
            if (sl > 0 && !g_fillDue) {   // text is there: make sure the engine has not hidden the box again
                int fl = 0; guikit::detail::rdInt(sc + 0x3400, guikit::Ctl_Flags, &fl);
                if ((fl & 2) == 0) { guikit::setVisible(sc + 0x3400, true); static int hiddenLogs = 0; if (g_cfgVerbose && hiddenLogs++ < 10) logf("[WBU-P9] property box was hidden (flags %08x), forced visible", static_cast<unsigned>(fl)); }
            }
            if ((sl <= 0 || g_fillDue) && guikit::setText(sc + 0x3400, g_colorSentence)) {
                int fl0 = 0, fl1 = 0; guikit::detail::rdInt(sc + 0x3400, guikit::Ctl_Flags, &fl0);
                guikit::setVisible(sc + 0x3400, true);   // the engine keeps this box hidden on colour slots (no stats text); the earlier visible build set this
                guikit::detail::rdInt(sc + 0x3400, guikit::Ctl_Flags, &fl1);
                g_fillDue = false;
                snprintf(g_boxText, sizeof g_boxText, "%s", g_colorSentence);
                logf("[WBU-P9] property box filled flags %08x -> %08x \"%s\"", static_cast<unsigned>(fl0), static_cast<unsigned>(fl1), g_colorSentence);
            }
        }
    }
    if (!listShown && g_colorSentence[0] == 0 && g_boxText[0] != 0) {   // slot left the colour crystal: clear only a box we filled and only if it still holds our text (engine stat text on lens/hilt stays)
        const bool boxOk = guikit::detail::textObjOk(sc + 0x3400);
        const uintptr_t sub = boxOk ? reinterpret_cast<uintptr_t>(guikit::detail::textSub(sc + 0x3400)) : 0;
        int bp = 0, bl = 0;
        if (boxOk && guikit::detail::rdInt(sub, 0, &bp) && guikit::detail::rdInt(sub, 4, &bl) && bl >= 0 && bl < 256 && (bl == 0 || bp > 0x10000)) {
            char cur[260] = {};
            bool okRead = true;   // dword copy may read up to 3 bytes past bl; every read is guarded by rdInt, cur is 260 bytes and bl < 256
            for (int k = 0; k < bl; k += 4) { int dw = 0; if (!guikit::detail::rdInt(static_cast<uintptr_t>(bp), k, &dw)) { okRead = false; break; } memcpy(cur + k, &dw, 4); }
            if (okRead) {
                cur[bl] = 0;
                if (static_cast<size_t>(bl) == strlen(g_boxText) && memcmp(cur, g_boxText, static_cast<size_t>(bl)) == 0) {
                    guikit::setText(sc + 0x3400, "");
                    guikit::setVisible(sc + 0x3400, false);
                    logf("[WBU-P9] property box cleared (slot left colour crystal)");
                }
                g_boxText[0] = 0;   // also when the engine already replaced the text
            }
        }   // read failure: state unchanged, retry next Draw
    }
    if (!g_crystalPreview.active) return;
    if (g_cfgVerbose) {   // P9 probe: watch the engine text sink at +0x2ee0 (right-column detail); log on change
        static int lastSp = -1, lastSl = -1;
        int sp = 0, sl = 0;
        const uintptr_t sub = sc + 0x2ee0 + 0xF0;
        guikit::detail::rdInt(sub, 0, &sp); guikit::detail::rdInt(sub, 4, &sl);
        if (sp != lastSp || sl != lastSl) {
            lastSp = sp; lastSl = sl;
            char t[97] = {};
            if (sp > 0x10000 && sl > 0 && sl < 4096) {
                for (int k = 0; k < 96; k += 4) { int dw = 0; if (!guikit::detail::rdInt(static_cast<uintptr_t>(sp), k, &dw)) break; memcpy(t + k, &dw, 4); }
                for (int k = 0; k < 96; ++k) if (t[k] && (static_cast<unsigned char>(t[k]) < 32 || static_cast<unsigned char>(t[k]) > 126)) t[k] = '?';
                if (sl < 96) t[sl] = 0;
            }
            logf("[WBU-P9] sink+0x2ee0 txtObj=%d str=%x len=%d \"%s\"", guikit::detail::textObjOk(sc + 0x2ee0) ? 1 : 0, sp, sl, t);
        }
    }
    uint32_t lsFlags = 0;
    if (!rd(S_WB, sc, wbu::kWb3dModelLsOff + kPanelFlags, &lsFlags)) return;
    if ((lsFlags & 2u) == 0u) {
        *reinterpret_cast<uint32_t*>(sc + wbu::kWb3dModelLsOff + kPanelFlags) |= 2u;
        *reinterpret_cast<uint32_t*>(sc + 0x32b8 + kPanelFlags) |= 2u;
        for (int i = 0; i < 6; ++i) *reinterpret_cast<uint32_t*>(sc + 0x7a8 + i * 0x1d0 + kPanelFlags) |= 2u;
        for (int i = 0; i < 6; ++i) *reinterpret_cast<uint32_t*>(sc + 0x17f8 + i * 0x148 + kPanelFlags) |= 2u;
        setUpgradeLbItemsRect(sc, wbu::kCrystalInlineListRect);
        logf("[WBU-P7] Inline crystal picker controls restored on panel=%lx", U(sc));
    }
}

// H20: CSWGuiUpgrade::OnControlEntered (ECX = workbench, [esp+0] = ret, [esp+4] = rowCtrl).
void __cdecl WbuControlEntered(void* wb, void* ret, void* rowCtrl) {
    (void)ret;
    if (g_cfgVerbose) {   // P9 probe: separates "handler not reached" from "gate closed"
        static int reached = 0;
        if (reached < 12) { ++reached; logf("[WBU-P9] WbuControlEntered reached #%d active=%d wb=%x row=%x", reached, g_crystalPreview.active ? 1 : 0, static_cast<unsigned>(P(wb)), static_cast<unsigned>(P(rowCtrl))); } }
    if (!g_crystalPreview.active || !wb || !rowCtrl) return;
    saferead::beginScope();
    const uintptr_t sc = P(wb), row = P(rowCtrl);
    if (g_cfgVerbose) {   // P9 probe: dump once per distinct hovered row (before the engine handler runs = state of the previous row)
        static uintptr_t lastRow = 0;
        if (lastRow != row) { lastRow = row; probeDumpTexts(sc, "hover"); }
    }
    uint32_t item = 0, base = 0, rowIdxRaw = 0;
    if (!rd(S_WB, sc, wbu::kWbWorkingItemOff, &item) || !item) return;
    if (wbu::checkWeaponChangeReset(&g_crystalPreview, item)) return;
    if (!rd(S_ITEM, item, 0x0c, &base)) return;
    if (!rd(S_WB, row, 0x1e0, &rowIdxRaw)) return;
    const int rowIdx = static_cast<int32_t>(rowIdxRaw);
    const uint8_t targetMv = wbu::crystalRowToModelVar(rowIdx, base, g_crystalPreview.origModelVar);
    if (wbu::updateCrystalPreview(&g_crystalPreview, targetMv)) {
        writeWorkingItemModelVar(sc, targetMv);
        logf("[WBU-P7] Crystal hover preview row=%d base=%u modelVar=%u", rowIdx, base, static_cast<unsigned>(targetMv));
    }
}

// H21: CSWGuiUpgrade::CloseItems (ECX = workbench, [esp+0] = ret).
void __cdecl WbuCloseItems(void* wb, void* ret) {
    (void)ret;
    if (!wb) return;
    saferead::beginScope();
    const uintptr_t sc = P(wb);
    if (g_crystalPreview.active) {
        const uintptr_t prevItem = g_crystalPreview.workingItem;
        const uint8_t orig = wbu::cancelCrystalPreview(&g_crystalPreview);
        writeWorkingItemModelVar(sc, orig, prevItem);
        logf("[WBU-P7] Crystal preview cancelled -> restored modelVar=%u", static_cast<unsigned>(orig));
    }
    restoreLbItemsRect(sc);
}

// H22: Global::ExecuteCommandActionStartConversation bypass for direct workbench open (doc 55 P8).
void __cdecl WbuStartConvBypass(void* ebp) {
    loadConfig();
    if (!g_cfgSkipDialog || !ebp) return;
    saferead::beginScope();
    const uintptr_t bp = P(ebp);
    uint32_t obj = 0, inGameGui = 0, oChar = 0, strPtr = 0;
    if (!rd(S_PANEL, bp, -0x3c, &obj) || !obj) return;
    if (!rd(S_PANEL, bp, -0x104, &inGameGui) || !inGameGui) return;
    if (!rd(S_PANEL, bp, -0x14, &oChar)) return;
    char dlgBuf[17] = {};
    if (rd(S_PANEL, bp, -0x44, &strPtr) && strPtr != 0) {
        uint32_t dw[4] = {};
        for (int i = 0; i < 4; ++i) {
            rd(S_PANEL, static_cast<uintptr_t>(strPtr), i * 4, &dw[i]);
        }
        memcpy(dlgBuf, dw, 16);
        dlgBuf[16] = '\0';
    }
    if (dlgBuf[0] == '\0') {
        uint32_t dw[4] = {};
        for (int i = 0; i < 4; ++i) {
            rd(S_PANEL, static_cast<uintptr_t>(obj) + 0x280, i * 4, &dw[i]);
        }
        memcpy(dlgBuf, dw, 16);
        dlgBuf[16] = '\0';
    }
    if (wbu::isWorkbenchDialogResref(dlgBuf)) {
        *reinterpret_cast<uint32_t*>(bp - 0x10c) = 0;
        typedef void (__thiscall* FnShowUpgradeScreen)(
            void* gui, uint32_t oItem, uint32_t oCharacter, int noCreate, int noUpgrade,
            char* ovrPtr, uint32_t ovrLen);
        reinterpret_cast<FnShowUpgradeScreen>(kAddrShowUpgradeScreen)(
            reinterpret_cast<void*>(inGameGui), 0x7f000000u, oChar, 0, 0, nullptr, 0);
        logf("[WBU-P8] Bypassed workbench dialog '%s' -> ShowUpgradeScreen(gui=%lx, char=%x)", dlgBuf, U(inGameGui), oChar);
    }
}

}  // extern "C"

k2hud::Client g_hudClient;

void KPATCH_CALL onPanelBuild(const K2HudContext* ctx, void* /*user*/) {
    if (!ctx) return;
    guikit::init();
    WbuStopLoadFromLayout(reinterpret_cast<void*>(ctx->panel), nullptr);
}

// Raw append trace (independent of KpLog, so a core that drops lines cannot hide it). Init diagnostics, kept until the
// 2026-11-08 "KPatchInit stops after enter" question is closed.
static void initTrace(const char* fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof buf - 2, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    buf[n++] = '\r'; buf[n++] = '\n';
    HANDLE h = CreateFileA("workbench_upgrade_init_trace.txt", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD w = 0; WriteFile(h, buf, static_cast<DWORD>(n), &w, nullptr); CloseHandle(h);
}

extern "C" __declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    initTrace("KPatchInit enter api=%p", static_cast<const void*>(api));
    guikit::init();
    initTrace("guikit::init done");
    const int logOk = KpLog_Init(api);
    initTrace("KpLog_Init returned %d", logOk);
    loadConfig();
    initTrace("loadConfig done");
    g_hudClient.init(api, "workbench-upgrade");
    initTrace("hud client init done");
    g_hudClient.add(K2HUD_PANEL_BUILD, 100, onPanelBuild);
    initTrace("hud client add done");
    g_hudClient.connect();
    initTrace("hud client connect() returned");
    logf("[WBU] KPatchInit complete (KpLog_Init=%d)", logOk);
    KP_LOG(chMain, "workbench-upgrade %s attached: switch=%d cmp=%d verbose=%d | log %s",
           kVersion, g_cfgSwitchButton, g_cfgCraftingCompare, g_cfgVerbose, kLogName);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        guikit::init();
        loadConfig();
        logf("[WBU] DLL_PROCESS_ATTACH (v%s, switch=%d, cmp=%d, verbose=%d)",
             kVersion, g_cfgSwitchButton, g_cfgCraftingCompare, g_cfgVerbose);
    }
    return TRUE;
}
