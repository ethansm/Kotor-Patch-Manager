// common_contract_test.cpp -- Patches/Common CONTRACT test for Inventory Sort & Filter (doc 20 section 4.1; rule of 2026-10-03).
//
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod. It pins the semantics THIS mod
// relies on, not the header's own behaviour (that is _shared_src/saferead_test.cpp). Every message starts with
// "inventory_sort_filter relies on:" so a failure names the consumer and the reason. Built against ../_shared_src (see test.toml),
// so ANY edit to SafeRead.h re-runs this test: a Common change that would break the mod fails here first.
//
// Mod use (InventorySortFilter.cpp): beginScope at the four hook entries; rd<T> = readAt<T> with a pre-seeded default (T = int,
// unsigned char, unsigned, unsigned short); wr32 = probeRead(addr, 4); dumpMem = readAt<int> per word. Host-only, deterministic.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
// Route SafeRead.h's VirtualQuery through a wrapper that can mark one range unreadable (the stub reports everything readable).
static auto g_realVQ = &VirtualQuery;
static uintptr_t g_badLo = 0, g_badHi = 0;
static SIZE_T fakeVQ(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) {
    SIZE_T r = g_realVQ(a, m, n);
    if (!r) return r;
    uintptr_t p = reinterpret_cast<uintptr_t>(a);
    if (g_badHi > g_badLo) {
        if (p >= g_badLo && p < g_badHi) { m->BaseAddress = reinterpret_cast<void*>(g_badLo); m->RegionSize = g_badHi - g_badLo; m->State = MEM_COMMIT; m->Protect = PAGE_NOACCESS; }
        else if (p < g_badLo && p + m->RegionSize > g_badLo) m->RegionSize = g_badLo - p;
    }
    return r;
}
#define VirtualQuery fakeVQ
#include "SafeRead.h"
#include "check.h"
#include "KpLog.h"
#include "ModConfigApi.h"
#include "ModConfigDecl.h"
#include "ProviderClient.h"
#include "SortOrderPage.h"
#include "HudHost.h"
#include "GuiKit.h"
#include <map>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define HCHECK(why, expr) ::kt::Check("inventory_sort_filter relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("inventory_sort_filter relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))
KP_LOG_PATCH("inventory-sort-filter");
KP_CHANNEL(chMain, "main");

#define U(x) static_cast<unsigned long long>(x)

static uint8_t* g_arena = nullptr; constexpr size_t ArenaSize = 1u << 16;
template <typename T> static T& at(uintptr_t b, int off) { return *reinterpret_cast<T*>(b + off); }

// The mod's own wrappers, verbatim (InventorySortFilter.cpp:81-90 and 92-103), built only from saferead calls.
template <typename T> static T rd(int base, int off, T def = T()) {
    T v = def;
    saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v);
    return v;
}
static bool wr32(int addr, int v) {
    if (!saferead::probeRead(2, reinterpret_cast<const void*>(static_cast<intptr_t>(addr)), 4)) return false;
    *reinterpret_cast<int*>(addr) = v;
    return true;
}
static int dumpWords(int addr, int bytes, bool* got) {   // returns words read; got[k] per word (mirrors dumpMem's per-word readAt<int>)
    int n = 0;
    for (int j = 0; j < bytes; j += 4) {
        int v = 0;
        got[j / 4] = saferead::readAt<int>(3, static_cast<uintptr_t>(addr), j, &v);
        if (got[j / 4]) ++n;
    }
    return n;
}

// ---- Mod Configuration page (WI-155; SortOrderPage.h + InventorySortFilter.cpp mcPageInit / mcPagePoll) ----
// The INI files the page edits, as an in-memory map keyed by section + key (inventory_sort_filter.ini and kplog.ini). Values are stored as written.
static std::map<std::string, std::string> g_kv;
static std::string kvKey(const char* section, const char* key) { return std::string(section) + '\x1f' + key; }
static void setKv(const char* section, const char* key, const char* value) { g_kv[kvKey(section, key)] = value; }
static bool fakeRaw(const char*, const char* section, const char* key, char* out, int cap) {   // = mcdecl::fileRead
    const auto it = g_kv.find(kvKey(section, key));
    if (it == g_kv.end() || cap < 1) return false;
    snprintf(out, cap, "%s", it->second.c_str());
    return true;
}
static bool fakeRead(const char* path, const char* section, const char* key, char* out, int cap) {   // = mcRead in InventorySortFilter.cpp
    return isf::readPageValue(fakeRaw, path, section, key, out, cap);
}
static std::string g_lastWrite;
static bool fakeRawWrite(const char*, const char* section, const char* key, const char* val) {   // = mcdecl::fileWrite: stores the value, remembers it
    g_kv[kvKey(section, key)] = val; g_lastWrite = val; return true;
}
static bool failSortWrite(const char* path, const char* section, const char* key, const char* val) {   // the sort switch write fails, the rest succeeds
    return strcmp(key, "sort") ? fakeRawWrite(path, section, key, val) : false;
}
// readPageValue for an [Inventory] key through the fake file; "<none>" when the read fails.
static std::string invRead(const char* key) {
    char o[32] = {};
    return isf::readPageValue(fakeRaw, "inventory_sort_filter.ini", "Inventory", key, o, sizeof o) ? std::string(o) : std::string("<none>");
}
// The stored [Inventory] value as the file has it; "<none>" when absent.
static std::string invCell(const char* key) {
    const auto it = g_kv.find(kvKey("Inventory", key));
    return it == g_kv.end() ? std::string("<none>") : it->second;
}
// writePageValue as the mod calls it: the reader is passed so a cycle write merges into the current list.
static bool invWrite(const char* key, const char* val) {
    return isf::writePageValue(fakeRawWrite, "inventory_sort_filter.ini", "Inventory", key, val, fakeRaw);
}

static void moddecl_contract() {
    static mcdecl::Page page;   // ~35 KB: static storage (ModConfigDecl.h)
    char err[96] = {};
    HCHECK("mcdecl::Page::load accepts isf::kPageSchema (mcPageInit; a rejected schema leaves the mod without its page)",
           page.load(isf::kPageSchema, strlen(isf::kPageSchema), err, sizeof err));
    HEQ("kPageSchema declares eight rows (sort_order, tint, cycle, cyc_new, cyc_usable, cyc_upgrade, strip, log_level)", page.decl.nRows, 8);
    {   // row order: the page shows the rows in this order
        static const char* const ids[8] = {"sort_order", "tint", "cycle", "cyc_new", "cyc_usable", "cyc_upgrade", "strip", "log_level"};
        bool ok = true;
        for (int k = 0; k < 8; ++k) ok = ok && !strcmp(page.decl.rows[k].id, ids[k]);
        HCHECK("kPageSchema row ids are in the order sort_order, tint, cycle, cyc_new, cyc_usable, cyc_upgrade, strip, log_level", ok);
    }
    {   // toggle rows: 0/1 toggles, default On, bound to inventory_sort_filter.ini [Inventory] with the key the page io maps
        static const char* const togRows[6][2] = {{"tint", "tint"}, {"cycle", "cycle"}, {"cyc_new", "cycle_states.new"},
                                                  {"cyc_usable", "cycle_states.usable"}, {"cyc_upgrade", "cycle_states.upgrade"}, {"strip", "strip"}};
        bool ok = true;
        for (int k = 0; k < 6; ++k) {
            const int t = page.decl.find(togRows[k][0]);
            ok = ok && t >= 0 && page.decl.rows[t].kind == mcdecl::K_TOGGLE && page.decl.rows[t].def == 1 &&
                 !strcmp(page.decl.rows[t].ini, "inventory_sort_filter.ini") && !strcmp(page.decl.rows[t].section, "Inventory") &&
                 !strcmp(page.decl.rows[t].key, togRows[k][1]);
        }
        HCHECK("tint, cycle, cyc_new / cyc_usable / cyc_upgrade and strip are toggles, default On, bound to [Inventory] with the key the page io maps", ok);
    }
    page.io.read = fakeRead;
    {   // Debug logging row: kplog.ini [inventory-sort-filter] level, index <-> the word the KpLog core parses
        const int L = page.decl.find("log_level");
        HCHECK("kPageSchema has the log_level row (last)", L == 7);
        if (L >= 0) {
            const mcdecl::RowDecl& r = page.decl.rows[L];
            HCHECK("log_level is a list row Off, On, Verbose", r.kind == mcdecl::K_LIST && r.nChoices == 3 && !strcmp(r.choices[0], "Off") &&
                   !strcmp(r.choices[1], "On") && !strcmp(r.choices[2], "Verbose"));
            HCHECK("log_level is bound to kplog.ini [inventory-sort-filter] level (section == patch id)",
                   !strcmp(r.ini, "kplog.ini") && !strcmp(r.section, isf::kPageId) && !strcmp(r.key, "level"));
            g_kv.clear(); setKv(isf::kPageId, "level", "debug");
            HEQ("level=debug shows On", page.startValue(L), 1);
            g_kv.clear(); setKv(isf::kPageId, "level", " TRACE ");
            HEQ("level=TRACE (case, spaces) shows Verbose", page.startValue(L), 2);
            g_kv.clear(); setKv(isf::kPageId, "level", "info");
            HEQ("level=info shows Off", page.startValue(L), 0);
            g_kv.clear(); setKv(isf::kPageId, "level", "3");
            HEQ("level=3 (digit debug) shows On", page.startValue(L), 1);
            g_kv.clear(); setKv(isf::kPageId, "level", "warn");
            HEQ("level=warn shows Off", page.startValue(L), 0);
            g_kv.clear(); setKv("action-hotbar", "level", "debug");
            HEQ("another mod's level is ignored (default Off)", page.startValue(L), 0);
            const char* const words[3] = {"info", "debug", "trace"};
            bool ok = true;
            for (int k = 0; k < 3; ++k) {
                char d[16]; snprintf(d, sizeof d, "%d", k);
                ok = ok && isf::writePageValue(fakeRawWrite, "kplog.ini", isf::kPageId, "level", d) && g_lastWrite == words[k];
            }
            HCHECK("writePageValue stores info / debug / trace for 0 / 1 / 2", ok);
        }
    }
    const int i = page.decl.find("sort_order");
    HCHECK("kPageSchema has the sort_order row", i == 0);
    if (i < 0) return;
    const mcdecl::RowDecl& r = page.decl.rows[i];
    HCHECK("sort_order is a list row (not a cycle_states toggle or stepper)", r.kind == mcdecl::K_LIST);
    HEQ("sort_order has 4 choices", r.nChoices, 4);
    HCHECK("sort_order choices are Category, Base item, Name, Vanilla in that order (stored value = index = the digit sort_order accepts)",
           !strcmp(r.choices[0], "Category") && !strcmp(r.choices[1], "Base item") && !strcmp(r.choices[2], "Name") && !strcmp(r.choices[3], "Vanilla"));
    HCHECK("sort_order is bound to inventory_sort_filter.ini [Inventory] sort_order",
           !strcmp(r.ini, "inventory_sort_filter.ini") && !strcmp(r.section, "Inventory") && !strcmp(r.key, "sort_order"));
    HEQ("sort_order defaults to 0 (Category)", r.def, 0);
    HEQ("a list row's range is 0 .. nChoices-1 (min)", r.minV, 0);
    HEQ("a list row's range is 0 .. nChoices-1 (max)", r.maxV, 3);
    HCHECK("page entry id is the patch id and the entry carries the build / enter callbacks",
           !strcmp(page.entry(isf::kPageId).id, "inventory-sort-filter") && page.entry(isf::kPageId).build && page.entry(isf::kPageId).enter);

    // Start value: the row reads the key from the INI when the page is built (Page::startValue through the injected io).
    g_kv.clear(); setKv("Inventory", "sort", "1"); setKv("Inventory", "sort_order", "2"); setKv("Inventory", "tint", "1");
    HEQ("start value = the INI's sort_order digit", page.startValue(i), 2);
    g_kv.clear(); setKv("Inventory", "sort", "1");
    HEQ("start value = default 0 when the INI has no sort_order key", page.startValue(i), 0);
    // The shown value is the order loadIni applies (isf::parseSortOrder), including the word spellings WI-154 accepts.
    g_kv.clear(); setKv("Inventory", "sort_order", "name");
    HEQ("a hand-edited sort_order=name shows Name (index 2)", page.startValue(i), 2);
    g_kv.clear(); setKv("Inventory", "sort_order", " VANILLA ");
    HEQ("sort_order words are case-insensitive and space-trimmed like loadIni (Vanilla = 3)", page.startValue(i), 3);
    g_kv.clear(); setKv("Inventory", "sort_order", "base");
    HEQ("sort_order=base shows Base item (1)", page.startValue(i), 1);
    g_kv.clear(); setKv("Inventory", "sort_order", "7");
    HEQ("an out-of-range sort_order shows Category (0): loadIni falls back to category, not to the nearest choice", page.startValue(i), 0);
    g_kv.clear(); setKv("Inventory", "sort_order", "bogus");
    HEQ("an unknown sort_order word shows Category (0), like loadIni", page.startValue(i), 0);
    g_kv.clear(); setKv("Other", "sort_order", "2"); setKv("Inventory", "sort", "1");
    HEQ("sort_order in another section is ignored", page.startValue(i), 0);
    // sort = 0 (the sort switch off) shows Vanilla whatever the order digit says; sort = 1 leaves the order alone.
    g_kv.clear(); setKv("Inventory", "sort", "0"); setKv("Inventory", "sort_order", "1");
    HEQ("sort=0 with sort_order=1 shows Vanilla (3): the sort switch wins", page.startValue(i), 3);
    g_kv.clear(); setKv("Inventory", "sort", "0");
    HEQ("sort=0 with no sort_order key shows Vanilla (3)", page.startValue(i), 3);
    g_kv.clear(); setKv("Inventory", "sort", "1"); setKv("Inventory", "sort_order", "1");
    HEQ("sort=1 with sort_order=1 shows Base item (1)", page.startValue(i), 1);

    // Index == WI-154's parseSortOrder digit: the stored schema index i is the order parseSortOrder("i") returns, and it is the order of the word.
    {
        static const char* const words[4] = {"category", "base", "name", "vanilla"};
        static const isf::SortOrder orders[4] = {isf::SO_Category, isf::SO_Base, isf::SO_Name, isf::SO_Vanilla};
        bool ok = true;
        for (int c = 0; c < 4; ++c) {
            char digit[16];
            snprintf(digit, sizeof digit, "%d", c);
            ok = ok && isf::parseSortOrder(digit) == orders[c] && isf::parseSortOrder(words[c]) == orders[c] && static_cast<int>(orders[c]) == c;
            // what the page writes (the digit, with its sort switch) and what it then shows after the file is read back
            g_kv.clear(); setKv("Inventory", "sort", "1");
            ok = ok && isf::writePageValue(fakeRawWrite, "inventory_sort_filter.ini", "Inventory", "sort_order", digit) && page.startValue(i) == c;
        }
        HCHECK("schema choice index i == isf::parseSortOrder(\"i\") == the order of the word (0 category, 1 base, 2 name, 3 vanilla), and the written digit reads back as i", ok);
    }

    // Sort order write: sort_order and the sort switch are one change (sort 0 = Vanilla, 1 = the other orders); a failed sort write fails the change.
    g_kv.clear();
    HCHECK("writing sort_order 3 (Vanilla) stores sort_order=3 and sort=0", invWrite("sort_order", "3") && invCell("sort_order") == "3" && invCell("sort") == "0");
    HCHECK("writing sort_order 2 (Name) stores sort_order=2 and sort=1", invWrite("sort_order", "2") && invCell("sort_order") == "2" && invCell("sort") == "1");
    HCHECK("the sort_order write reports failure when the sort write fails", !isf::writePageValue(failSortWrite, "inventory_sort_filter.ini", "Inventory", "sort_order", "1"));
    // Filter cycle states: cycle_states.<name> reads one bit of [Inventory] cycle_states (default new,usable,upgrade); writes merge in canonical order.
    g_kv.clear(); setKv("Inventory", "cycle_states", "new,upgrade");
    HCHECK("cycle_states.new reads 1 from \"new,upgrade\"", invRead("cycle_states.new") == "1");
    HCHECK("cycle_states.usable reads 0 from \"new,upgrade\"", invRead("cycle_states.usable") == "0");
    HCHECK("cycle_states.upgrade reads 1 from \"new,upgrade\"", invRead("cycle_states.upgrade") == "1");
    HEQ("the cyc_usable row shows 0 from \"new,upgrade\"", page.startValue(page.decl.find("cyc_usable")), 0);
    HEQ("the cyc_new row shows 1 from \"new,upgrade\"", page.startValue(page.decl.find("cyc_new")), 1);
    g_kv.clear();
    HCHECK("cycle_states.usable reads 1 when the key is absent (default new,usable,upgrade)", invRead("cycle_states.usable") == "1");
    HEQ("the cyc_usable row shows 1 when cycle_states is absent", page.startValue(page.decl.find("cyc_usable")), 1);
    g_kv.clear(); setKv("Inventory", "cycle_states", "new,usable,upgrade");
    HCHECK("writing cycle_states.usable 0 on \"new,usable,upgrade\" stores \"new,upgrade\"", invWrite("cycle_states.usable", "0") && invCell("cycle_states") == "new,upgrade");
    HCHECK("writing cycle_states.usable 1 on \"new,upgrade\" stores the canonical \"new,usable,upgrade\"", invWrite("cycle_states.usable", "1") && invCell("cycle_states") == "new,usable,upgrade");
    g_kv.clear(); setKv("Inventory", "cycle_states", "new,usable,upgrade");
    HCHECK("clearing all three states stores an empty list", invWrite("cycle_states.new", "0") && invWrite("cycle_states.usable", "0") && invWrite("cycle_states.upgrade", "0") && invCell("cycle_states") == "");
    HCHECK("setting usable on an empty list stores \"usable\"", invWrite("cycle_states.usable", "1") && invCell("cycle_states") == "usable");
    g_kv.clear();
    HCHECK("with no cycle_states key, clearing usable merges into the default list and stores \"new,upgrade\"", invWrite("cycle_states.usable", "0") && invCell("cycle_states") == "new,upgrade");
    HCHECK("a cycle write without the reader is refused (no list to merge into)", !isf::writePageValue(fakeRawWrite, "inventory_sort_filter.ini", "Inventory", "cycle_states.usable", "0"));
    HCHECK("an unknown cycle state name is refused", !invWrite("cycle_states.bogus", "1"));
    // Plain toggles pass through raw.
    g_kv.clear(); setKv("Inventory", "tint", "0");
    HCHECK("tint passes through raw (0 reads 0)", invRead("tint") == "0");
    HCHECK("writePageValue passes tint through raw", invWrite("tint", "1") && invCell("tint") == "1");

    // Persist: a page change is written by iniw::setKey into [Inventory] only, keeping the other keys and comments.
    const char in[] = "[Inventory]\n; backpack order\nsort=1\nsort_order=0\ntint=1\n";
    char out[256];
    size_t n = 0;
    HCHECK("iniw::setKey rewrites [Inventory] sort_order", iniw::setKey(in, sizeof in - 1, "Inventory", "sort_order", "3", out, sizeof out, &n));
    HCHECK("the rewrite changes only the sort_order line (comments and the other keys stay byte for byte)",
           std::string(out, n) == "[Inventory]\n; backpack order\nsort=1\nsort_order=3\ntint=1\n");
    const char noKey[] = "[Inventory]\nsort=1\ntint=1\n\n[Other]\nx=1\n";
    HCHECK("a missing sort_order key is appended inside [Inventory], not at the end of the file",
           iniw::setKey(noKey, sizeof noKey - 1, "Inventory", "sort_order", "1", out, sizeof out, &n) &&
               std::string(out, n) == "[Inventory]\nsort=1\ntint=1\nsort_order=1\n\n[Other]\nx=1\n");

    // Lazy connect without mod-config: no KPatchApi -> never ok, never registers (mcPagePoll is then a no-op and the mod works as before).
    k2mc::ConfigClient cc;
    k2mc::MenuClient mc;
    cc.init(nullptr);
    mc.init(nullptr);
    for (int k = 0; k < 600; ++k) { cc.poll(); mc.poll(); }
    HCHECK("with no KPatchApi the config and menu clients stay unconnected", !cc.ok() && !mc.ok() && !mc.hasRows());
    k2pc::RegisterOnce st;
    HCHECK("registerOnce does nothing while mod-config is missing", !k2pc::registerOnce(st, cc, page.entry(isf::kPageId), nullptr) && !st.registered);
}

int main() {
    moddecl_contract();
    void* m = mmap(nullptr, ArenaSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#ifdef MAP_32BIT
                   | MAP_32BIT
#endif
                   , -1, 0);
    if (m == MAP_FAILED) { perror("mmap"); return 5; }
    g_arena = static_cast<uint8_t*>(m);
    g_stubVirtualQueryAll = true;
    uintptr_t a = reinterpret_cast<uintptr_t>(g_arena);
    const int ib = static_cast<int>(a);   // the mod holds 32-bit addresses in ints

    // ---- beginScope at every hook entry (272/294/352/381)
    saferead::beginScope();
    at<int>(a, 0x10) = 0x12345678; at<unsigned char>(a, 0x20) = 0x7B; at<unsigned>(a, 0x24) = 0xDEADBEEFu; at<unsigned short>(a, 0x28) = 0xBEEF;
    bool rep = true; for (int i = 0; i < 4; ++i) { saferead::beginScope(); rep = rep && rd<int>(ib, 0x10, -1) == 0x12345678; }
    HCHECK("beginScope is callable at each hook entry (4 sites) and reads still work afterwards (InventorySortFilter.cpp:272)", rep);

    // ---- rd<T> (83): value on readable memory for each T the mod uses
    HEQ("rd<int> delivers the value (panel/item/ebp field reads, :134,:145,:222)", (long long)rd<int>(ib, 0x10), 0x12345678ll);
    HEQ("rd<unsigned char> delivers one byte (FilterModeOff / BaseGenderReqOff, :139,:193)", U(rd<unsigned char>(ib, 0x20, 0xFF)), 0x7Bull);
    HEQ("rd<unsigned> delivers 32 bits (BaseSlotMaskOff, :189)", U(rd<unsigned>(ib, 0x24)), 0xDEADBEEFull);
    HEQ("rd<unsigned short> delivers 16 bits (gender fields, :195)", U(rd<unsigned short>(ib, 0x28)), 0xBEEFull);
    at<unsigned>(a, 0x30) = 0x11223344u;
    HEQ("readAt<unsigned char> reads sizeof(T)=1 byte only (low byte of a little-endian word)", U(rd<unsigned char>(ib, 0x30)), 0x44ull);
    HEQ("readAt<unsigned short> reads sizeof(T)=2 bytes only", U(rd<unsigned short>(ib, 0x30)), 0x3344ull);
    at<int>(a, 0x50) = 11; at<int>(a, 0x54) = 22; at<int>(a, 0x58) = 33;
    HEQ("readAt honours base+off (slot array walk base + 4*i, :179,:223)", (long long)(rd<int>(ib + 0x50, 8) * 100 + rd<int>(ib, 0x50 + 4)), 3322ll);

    // ---- failure keeps the pre-seeded default (the mod relies on this to fail open / fail unknown)
    HEQ("rd<int> on base 0 returns the seeded default (:274 TabClickedOff, -1)", (long long)rd<int>(0, 0x10, -1), -1ll);
    HEQ("rd<unsigned char> on base 0 returns 0xFF = unknown filter mode, never a cycle mode (:139)", U(rd<unsigned char>(0, 0x20, 0xFF)), 0xFFull);
    HEQ("rd<int> below 0x10000 returns the seeded default (-1 = open upgrade slot, fail-open, :179)", (long long)rd<int>(0x20, 0, -1), -1ll);
    HEQ("rd<int> failure with no default is 0 (:134 holder chain)", (long long)rd<int>(0, 0), 0ll);
    HEQ("rd<unsigned> on base 0 is 0 (:189)", U(rd<unsigned>(0, 0)), 0ull);
    int v = 0x55555555;
    HCHECK("readAt returns false on base 0", !saferead::readAt<int>(1, 0, 4, &v));
    HEQ("readAt failure leaves *out untouched", (long long)v, 0x55555555ll);
    HCHECK("readAt returns false below 0x10000", !saferead::readAt<int>(1, 0x8000, 0, &v));
    HEQ("readAt failure below 0x10000 leaves *out untouched", (long long)v, 0x55555555ll);

    // ---- unreadable memory (page-sized hole in the arena)
    g_badLo = a + 0x1000; g_badHi = a + 0x2000; saferead::beginScope();
    at<int>(a, 0x0FFC) = 0x0BADF00D;
    HEQ("rd<int> on an unreadable page returns the seeded default", (long long)rd<int>(ib + 0x1000, 0, -1), -1ll);
    HEQ("rd<int> just below an unreadable page still reads", (long long)rd<int>(ib, 0x0FFC), 0x0BADF00Dll);
    HCHECK("readAt that straddles into the unreadable page fails (4-byte read at -2 of the hole)", !saferead::readAt<int>(1, a, 0x0FFE, &v));
    HEQ("straddling readAt leaves *out untouched", (long long)v, 0x55555555ll);

    // ---- wr32 (87): probeRead(addr, 4) gate
    at<int>(a, 0x40) = 0;
    HCHECK("wr32 writes through when the 4 bytes are readable (:87, ebp+F_SelRow patch)", wr32(ib + 0x40, 0x7777) && at<int>(a, 0x40) == 0x7777);
    HCHECK("wr32 refuses an unreadable target and does not write", !wr32(ib + 0x1000, 0x1234) && at<int>(a, 0x1000) == 0);
    HCHECK("wr32 refuses a null/low target (no fault)", !wr32(0, 1) && !wr32(0x20, 1));
    HCHECK("wr32 refuses a word that starts readable but ends in the hole", !wr32(ib + 0x0FFE, 1));
    HEQ("wr32 refusal left the readable bytes before the hole alone", (long long)at<int>(a, 0x0FFC), 0x0BADF00Dll);

    // ---- dump loop (97): tail crosses into unreadable memory
    g_badLo = a + 0x140; g_badHi = a + 0x1000; saferead::beginScope();
    for (int i = 0; i < 0x120; i += 4) at<int>(a, 0x100 + i) = 0x1000 + i;
    bool got[0x120 / 4]; int n = dumpWords(ib + 0x100, 0x120, got);
    HEQ("dump loop reads exactly the readable words (0x40 bytes = 16 of 72)", (long long)n, 16ll);
    bool headOk = true, tailBad = true;
    for (int k = 0; k < 16; ++k) headOk = headOk && got[k];
    for (int k = 16; k < 0x120 / 4; ++k) tailBad = tailBad && !got[k];
    HCHECK("dump loop: words before the hole succeed (earlier words are kept)", headOk);
    HCHECK("dump loop: words in the hole fail cleanly, loop continues to the end (dumpMem prints placeholders)", tailBad);
    int w = -1; HCHECK("dump loop: last readable word value is intact", saferead::readAt<int>(3, a, 0x13C, &w) && w == 0x1000 + 0x3C);
    g_badLo = g_badHi = 0; saferead::beginScope();
    n = dumpWords(ib + 0x100, 0x120, got);
    HEQ("dump loop reads all 72 words once the hole is gone (scope reset drops the cache)", (long long)n, 72ll);

    // ---- KpLog fallback sink (logf forwards here): SetFallbackFile only stores the path, so the DLL's static initialiser does no I/O.
    {
        const char* td = getenv("TMPDIR"); if (!td || !*td) td = "/tmp";
        char path[300]; snprintf(path, sizeof path, "%s/inventory_sort_filter_log.txt", td);
        remove(path);
        KpLog_SetFallbackFile(path); KpLog_SetFallbackLevel(KPLOG_INFO);
        HCHECK("KpLog_SetFallbackFile only stores the path, no I/O (safe from a static initialiser)", access(path, F_OK) != 0);
        KpLog_SetFallbackFile(nullptr);
        remove(path);
    }
    // ---- VU4 strip: HudHost.h PANEL_BUILD context + GuiKit panel list (onPanelBuild / stripLive in InventorySortFilter.cpp) ----
    HEQ("K2HUD_PANEL_BUILD is phase 4 (the phase Register is called with)", (long long)K2HUD_PANEL_BUILD, 4ll);
    HEQ("K2HudContext.panelVtbl sits right after panel (struct_size gate before the read)", (long long)offsetof(K2HudContext, panelVtbl), (long long)(offsetof(K2HudContext, panel) + 4));
    HEQ("K2HudContext.panel is an int32 (cast to uintptr_t for GuiKit)", (long long)sizeof(((K2HudContext*)0)->panel), 4ll);
    HEQ("panel control list array at +0x24 (stripLive re-checks the labels there)", (long long)guikit::Panel_CtlArray, 0x24ll);
    HEQ("panel control count at +0x28 (stripLabel reads the append index)", (long long)guikit::Panel_CtlCount, 0x28ll);
    HEQ("CreateFail CF_Template = 3 (optional LBL_MOULDING3 frame may be missing)", (long long)guikit::CF_Template, 3ll);
    return kt::Report("common_contract_test (inventory_sort_filter)");
}
