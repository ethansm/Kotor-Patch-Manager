// ItemCompareRead.h -- engine memory -> ItemCompare.h CompareInput (the equip-compare read adapter).
// Plan: .claude/research/equip_compare/plan/P2_adapter.md; decisions PLAN_FINAL.md 1, 3-6; critic C1 fixes folded in.
// Readers copied from QuickLoot (quick_loot_src/QuickLoot.cpp: readBaseStats, the rules prop loop, readItemText, itemPropLabel,
// itemPropTexts, tlkText; QuickLootLogic.h stripTags / baseItemIdxOk). Every engine call is a plain function-pointer hook
// (g_getBase, g_invSlot, g_getItem, g_label, g_propText, g_locStr, g_tlk, g_language). The defaults are the engine calls;
// host tests replace every hook. Memory reads go through saferead. The adapter writes only into the caller's structs.
// Ids: ItemFacts.id = low 32 bits of the CSWSItem* (one id space; ItemCompare.h decision 1). Row ids are resolved with viewedFromRow.
#pragma once
#include <stdint.h>
#include <string.h>
#include "SafeRead.h"
#include "GameAddr.h"
#include "ItemRead.h"
#include "ItemCompare.h"

namespace itemcmpread {

using itemcmp::CompareInput;
using itemcmp::ItemFacts;
using itemcmp::PropRow;
using itemcmp::SlotRead;
using itemcmp::SlotState;
using itemcmp::kUnk;
using itemcmp::PROP_DIFF_MAX;

constexpr unsigned SiteCmp = 0x6EA0;                   // saferead failure-ring site id (0x6E00 / 0x6E60 are CompanionSelfHeal's)
constexpr int Item_BaseIdx = 0x0C, Item_Name = 0x2C0;
constexpr int Item_ActivePropCount = 0x288, Item_ActivePropArray = 0x290;
constexpr int Item_PassivePropCount = 0x28C, Item_PassivePropArray = 0x294;
constexpr int Item_PropCount = Item_ActivePropCount, Item_PropArray = Item_ActivePropArray;
constexpr int Item_RowId = 0x1D0;                      // equip-screen row: raw server object id (S3 sec 1); the caller reads it
constexpr int Creature_Inv = 0x1150;                   // CSWSCreature -> CSWInventory*
constexpr int App_Client = 4, Client_Inner = 4;
constexpr int Prop_Stride = 0x1C, Prop_Type = 0, Prop_Subtype = itemread::Prop_Subtype, Prop_CostTable = 4, Prop_CostValue = 6,
              Prop_Param1 = 8, Prop_Param1Value = 9;
constexpr uint16_t KeenType = 0x1C, KeenSubtype = 0;   // Keen = prop 0x1C sub 0 (P2 sec 0.2; live KEEN check pending)
constexpr uint8_t Param1None = 0xFF, DexNoLimit = 0xFF;
constexpr int MaxBaseItemIdx = 1024, MaxPropCount = 256;
constexpr size_t BaseItem_Read = 0xB0;                 // base bytes the readers touch (+0 name, +4 slots, +0x1C..0x1F dice/die/crit, +0x8A AC, +0xAD dex)

// ---- engine addresses used only by the default hooks (QuickLoot.cpp:186, 571-572, 489-493; GameAddr.h for the rest)
constexpr int Rules_ItemPropDef1 = 0x130, Rules_ItemPropDef2 = 0x3C;

// ---- hooks (plain function pointers; the defaults do the __thiscall / __fastcall work)
using GetBaseHook = uintptr_t (*)(uintptr_t item);                        // CSWItem::GetBaseItem -> baseitems row (0 = none)
using InvSlotHook = uintptr_t (*)(uintptr_t inv, uint32_t bit);           // CSWInventory::GetItemInSlot, ONE slot bit -> item or 0
using GetItemHook = uintptr_t (*)(uintptr_t objTable, uint32_t id);       // GetItemByObjectId(objTable, server id) -> item or 0
using LabelHook = bool (*)(uint16_t prop, char* out, unsigned cap);       // itempropdef 'label' column; false = unresolved
struct PropStrings { char name[80]; char sub[80]; char val[80]; };
using PropTextHook = void (*)(uint16_t prop, uint16_t sub, uint16_t costValue, uint8_t param1Value, PropStrings* out);
using LocStrHook = bool (*)(uintptr_t handle, int language, char* out, unsigned cap);   // CExoLocString::GetString, engine string freed inside
using TlkHook = bool (*)(uint32_t strref, char* out, unsigned cap);
using LanguageHook = int (*)();                                           // client language for GetString; -1 = unknown

// ---- default hook bodies (the only code that touches the engine)
inline unsigned stripTags(char* out, unsigned cap, const char* in);   // defined below (text helpers)
struct ExoStr { uint32_t p; int len; };   // CExoString {char* p; int len}, 32-bit fields
using ExoCtorFn = void* (__thiscall*)(void* self, const char* text);
using ExoDtorFn = void (__thiscall*)(void* self);
inline void exoInit(ExoStr& e, const char* text) { e.p = 0; e.len = 0; reinterpret_cast<ExoCtorFn>(gameaddr::ExoCtor)(&e, text); }
inline void exoFree(ExoStr& e) { reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(&e); }
inline int exoRead(const ExoStr& e, char* out, int cap) { return saferead::readExo(SiteCmp, reinterpret_cast<uintptr_t>(&e), out, cap, cap - 1, false); }

inline uintptr_t defaultGetBase(uintptr_t item) { return reinterpret_cast<uintptr_t (__thiscall*)(uintptr_t)>(gameaddr::GetBaseItem)(item); }
inline uintptr_t defaultInvSlot(uintptr_t inv, uint32_t bit) {
    return reinterpret_cast<uintptr_t (__thiscall*)(uintptr_t, uint32_t)>(gameaddr::InvGetItemInSlot)(inv, bit);
}
inline uintptr_t defaultGetItem(uintptr_t objTable, uint32_t id) {
    return reinterpret_cast<uintptr_t (__thiscall*)(uintptr_t, uint32_t)>(gameaddr::GetItemByObjectId)(objTable, id);
}

inline bool defaultLabel(uint16_t prop, char* out, unsigned cap) {
    out[0] = 0;
    uint32_t g = 0, x1 = 0, tda = 0;
    if (!saferead::readAt(SiteCmp, gameaddr::RulesGlobal, 0, &g) || !g || !saferead::readAt(SiteCmp, g, Rules_ItemPropDef1, &x1) || !x1 ||
        !saferead::readAt(SiteCmp, x1, Rules_ItemPropDef2, &tda) || !tda)
        return false;
    ExoStr col, res;
    exoInit(col, "label");
    exoInit(res, "");
    const int ok = reinterpret_cast<int (__thiscall*)(uintptr_t, int, ExoStr*, ExoStr*)>(gameaddr::C2daGetString)(tda, prop, &col, &res);
    if (ok) exoRead(res, out, static_cast<int>(cap));
    exoFree(res);
    exoFree(col);
    return out[0] != 0;
}

inline void defaultPropText(uint16_t prop, uint16_t sub, uint16_t costValue, uint8_t param1Value, PropStrings* out) {
    using PropFn = void (__thiscall*)(uintptr_t unusedEcx, uint16_t, uint16_t, uint16_t, uint8_t, ExoStr*, ExoStr*, ExoStr*, ExoStr*, int);
    ExoStr l, s2, c, p1;
    exoInit(l, ""); exoInit(s2, ""); exoInit(c, ""); exoInit(p1, "");
    reinterpret_cast<PropFn>(gameaddr::ItemPropBuilder)(0, prop, sub, costValue, param1Value, &l, &s2, &c, &p1, 0);
    char tmp[160];
    if (exoRead(l, tmp, sizeof tmp) > 0) stripTags(out->name, sizeof out->name, tmp);
    if (exoRead(s2, tmp, sizeof tmp) > 0) stripTags(out->sub, sizeof out->sub, tmp);
    if (exoRead(c, tmp, sizeof tmp) > 0) stripTags(out->val, sizeof out->val, tmp);
    exoFree(p1); exoFree(c); exoFree(s2); exoFree(l);
}

inline bool defaultLocStr(uintptr_t handle, int lang, char* out, unsigned cap) {
    out[0] = 0;
    int exo[2] = {0, 0};   // CExoString {char* p; uint32 len}
    const bool ok = reinterpret_cast<bool (__thiscall*)(void*, int, void*, unsigned char)>(gameaddr::GetLocString)(
        reinterpret_cast<void*>(handle), lang, exo, 0);
    if (ok && exo[0]) saferead::readExo(SiteCmp, reinterpret_cast<uintptr_t>(exo), out, static_cast<int>(cap), static_cast<int>(cap) - 1, false);
    if (exo[0]) reinterpret_cast<ExoDtorFn>(gameaddr::ExoDtor)(exo);
    return out[0] != 0;
}

inline bool defaultTlk(uint32_t strref, char* out, unsigned cap) {
    out[0] = 0;
    uint32_t app = 0, client = 0;
    if (!saferead::readAt(SiteCmp, gameaddr::AppGlobal, 0, &app) || !app || !saferead::readAt(SiteCmp, app, App_Client, &client) || !client) return false;
    ExoStr t;
    exoInit(t, "");
    reinterpret_cast<uintptr_t (__thiscall*)(uintptr_t, ExoStr*, int)>(gameaddr::TlkGetString)(client, &t, static_cast<int>(strref));
    exoRead(t, out, static_cast<int>(cap));
    exoFree(t);
    return out[0] != 0;
}

inline int defaultLanguage() {
    uint32_t app = 0, client = 0, inner = 0;
    if (!saferead::readAt(SiteCmp, gameaddr::AppGlobal, 0, &app) || !app || !saferead::readAt(SiteCmp, app, App_Client, &client) || !client) return -1;
    if (!saferead::readAt(SiteCmp, client, Client_Inner, &inner) || !inner) return -1;   // GetClientLanguage reads *(*(client+4)+0x148)
    // GetClientLanguage is __fastcall with one argument (ECX only): the same ABI as __thiscall with one argument. The host stub has no __fastcall.
    return reinterpret_cast<int (__thiscall*)(int)>(gameaddr::GetClientLanguage)(static_cast<int>(client));
}

// ---- the injectable seams (host tests replace all of them; the game keeps the defaults)
inline GetBaseHook g_getBase = defaultGetBase;
inline InvSlotHook g_invSlot = defaultInvSlot;
inline GetItemHook g_getItem = defaultGetItem;
inline LabelHook g_label = defaultLabel;
inline PropTextHook g_propText = defaultPropText;
inline LocStrHook g_locStr = defaultLocStr;
inline TlkHook g_tlk = defaultTlk;
inline LanguageHook g_language = defaultLanguage;
inline uintptr_t g_appGlobal = gameaddr::AppGlobal;   // *(app) = app; *(app+8) = object table (ItemRead.h serverItemFor, same hop)

// ---- text helpers (no snprintf: every composed string is bounded by hand)
inline void copyTo(char* dst, unsigned cap, const char* src) {   // at most cap-1 chars, NUL-terminated
    if (!dst || cap == 0) return;
    unsigned n = 0;
    for (; src[n] && n + 1 < cap; ++n) dst[n] = src[n];
    dst[n] = 0;
}
inline void appendTo(char* dst, unsigned cap, const char* src) {
    if (!dst || cap == 0) return;
    unsigned n = 0;
    while (n < cap && dst[n]) ++n;                  // dst must be NUL-terminated within cap; otherwise leave it untouched
    if (n >= cap) return;
    for (unsigned i = 0; src[i] && n + 1 < cap; ++i) dst[n++] = src[i];
    dst[n] = 0;
}
// QuickLootLogic.h stripTags (copied): drops "{...}" tags of at most 12 inner chars, collapses blank runs, trims both ends.
inline unsigned stripTags(char* out, unsigned cap, const char* in) {
    if (!out || cap == 0) return 0;
    unsigned n = 0;
    bool space = false;
    if (in) {
        for (unsigned i = 0; in[i] && n + 1 < cap; ++i) {
            if (in[i] == '{') {
                unsigned j = i + 1;
                while (in[j] && in[j] != '}' && in[j] != '{' && j - i - 1 <= 12) ++j;
                if (in[j] == '}' && j - i - 1 <= 12) { i = j; continue; }
            }
            if (in[i] == ' ' || in[i] == '\t') { space = n > 0; continue; }
            if (space && n + 2 < cap) out[n++] = ' ';
            space = false;
            out[n++] = in[i];
        }
    }
    out[n] = 0;
    return n;
}
// Display text: tags stripped, CR/LF/TAB -> space, other control or non-ASCII bytes -> '?', cut to cap-1.
inline void sanitizeCopy(char* dst, unsigned cap, const char* src) {
    if (!dst || cap == 0) return;
    char t[160];
    stripTags(t, sizeof t, src);
    unsigned n = 0;
    for (; t[n] && n + 1 < cap; ++n) {
        unsigned char c = static_cast<unsigned char>(t[n]);
        if (c == '\r' || c == '\n' || c == '\t') c = ' ';
        else if (c < 0x20 || c > 0x7E) c = '?';
        dst[n] = static_cast<char>(c);
    }
    dst[n] = 0;
}

// ---- readers
struct BaseRead { uint32_t nameRef = 0, equipMask = 0; int numDice = 0, die = 0, critThreat = 0, critMult = 0, baseAC = 0, maxDex = -1; };
struct RawProp { uint16_t type = 0, subtype = 0, costValue = 0; uint8_t costTable = 0, param1 = 0, param1Value = 0; };

template <typename T> inline bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(SiteCmp, base, off, out); }

inline bool baseIdxOk(int idx) { return idx >= 0 && idx < MaxBaseItemIdx; }

// Base row of `item` (QuickLoot readBaseStats). Guards: item+0x0C index plausible, base probe-readable (0xB0 bytes). maxDex: 0xFF = -1 (no limit).
inline bool readBase(uintptr_t item, BaseRead* out) {
    int idx = -1;
    if (!item || !rd(item, Item_BaseIdx, &idx) || !baseIdxOk(idx)) return false;
    const uintptr_t base = g_getBase(item);
    if (!base || !saferead::probeRead(SiteCmp, reinterpret_cast<const void*>(base), BaseItem_Read)) return false;
    uint8_t nd = 0, die = 0, ct = 0, cm = 0, ac = 0, dex = 0;
    if (!rd(base, 0x00, &out->nameRef) || !rd(base, 0x04, &out->equipMask) || !rd(base, 0x1C, &nd) || !rd(base, 0x1D, &die) ||
        !rd(base, 0x1E, &ct) || !rd(base, 0x1F, &cm) || !rd(base, 0x8A, &ac) || !rd(base, 0xAD, &dex))
        return false;
    out->numDice = nd; out->die = die; out->critThreat = ct; out->critMult = cm; out->baseAC = ac;
    out->maxDex = dex == DexNoLimit ? -1 : static_cast<int>(dex);
    return true;
}

inline uint16_t le16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }

// One 0x1C element (QuickLoot prop loop fields). One probeRead per element, then decoded from the copied bytes.
inline bool readRaw(uintptr_t arr, int i, RawProp* out) {
    const uintptr_t e = arr + static_cast<uintptr_t>(i) * static_cast<uintptr_t>(Prop_Stride);
    if (!arr || !saferead::probeRead(SiteCmp, reinterpret_cast<const void*>(e), Prop_Stride)) return false;
    uint8_t b[Prop_Stride];
    memcpy(b, reinterpret_cast<const void*>(e), Prop_Stride);
    out->type = le16(b + Prop_Type);
    out->subtype = le16(b + Prop_Subtype);
    out->costTable = b[Prop_CostTable];
    out->costValue = le16(b + Prop_CostValue);
    out->param1 = b[Prop_Param1];
    out->param1Value = b[Prop_Param1Value];
    return true;
}

// Racial / spell rows the description already states (ItemProps.h:399-400): CastSpell; UseLimitationRacial with sub 0 (Human).
inline bool skipLabel(const char* label, unsigned sub) {
    if (strcmp(label, "CastSpell") == 0) return true;
    return strcmp(label, "UseLimitationRacial") == 0 && sub == 0;
}

// PropRow text: "name: value" ("name: " dropped when empty), value "Yes" when empty; sanitized, cut to 63 chars + NUL.
inline void buildText(PropRow* r, const char* name, const char* value) {
    char raw[160] = {0};
    if (name[0]) { appendTo(raw, sizeof raw, name); appendTo(raw, sizeof raw, ": "); }
    appendTo(raw, sizeof raw, value[0] ? value : "Yes");
    sanitizeCopy(r->text, sizeof r->text, raw);
}

// Item name: CExoLocString at item+0x2C0 through g_locStr (language from g_language), else the base nameRef through g_tlk. false = neither.
// Like QuickLoot readItemText, [handle] and [[handle]] must be readable before GetString (it dereferences both with no null check).
inline bool readName(uintptr_t item, uint32_t baseNameRef, char* out, unsigned cap) {
    out[0] = 0;
    char raw[128] = {0};
    const uintptr_t handle = item + Item_Name;
    uint32_t first = 0, probe = 0;
    if (item && rd(handle, 0, &first) && first && rd(static_cast<uintptr_t>(first), 0, &probe)) {
        const int lang = g_language();
        if (lang >= 0 && g_locStr(handle, lang, raw, sizeof raw) && raw[0]) {
            sanitizeCopy(out, cap, raw);
            if (out[0]) return true;
        }
    }
    if (baseNameRef && baseNameRef != 0xFFFFFFFFu && g_tlk(baseNameRef, raw, sizeof raw)) sanitizeCopy(out, cap, raw);
    return out[0] != 0;
}

// Prop rows of `item` into out->props / nProps / dropped; *keen from the raw scan.
// Reads passive properties (+0x28C count, +0x294 array) first, then active properties (+0x288 count, +0x290 array).
// Pass 1 finds Keen anywhere in either array (before the cap and the skip rules). Pass 2 builds rows: skip rules first (not counted),
// then the 16-row cap (overflow -> dropped), then a row (label miss kept with known = 0, text empty).
// An unreadable element at row i: dropped += cnt - i and stop that list. false = count implausible or array missing (no rows at all).
inline bool readProps(uintptr_t item, ItemFacts* out, bool* keen) {
    *keen = false;
    int32_t actCnt = 0;
    int32_t pasCnt = 0;
    if (!rd(item, Item_ActivePropCount, &actCnt) || actCnt < 0 || actCnt >= MaxPropCount) return false;
    if (!rd(item, Item_PassivePropCount, &pasCnt) || pasCnt < 0 || pasCnt >= MaxPropCount) return false;

    uint32_t pasArr = 0;
    if (pasCnt > 0) {
        if (!rd(item, Item_PassivePropArray, &pasArr) || !pasArr) return false;
    }

    uint32_t actArr = 0;
    if (actCnt > 0) {
        if (!rd(item, Item_ActivePropArray, &actArr) || !actArr) return false;
    }

    struct PropList { uintptr_t arr; int32_t cnt; };
    const PropList lists[2] = {
        { static_cast<uintptr_t>(pasArr), pasCnt },
        { static_cast<uintptr_t>(actArr), actCnt }
    };

    RawProp r;
    for (int l = 0; l < 2; ++l) {
        const uintptr_t a = lists[l].arr;
        const int32_t cnt = lists[l].cnt;
        for (int32_t i = 0; i < cnt; ++i) {
            if (!readRaw(a, i, &r)) continue;   // pass 1 only looks for Keen: an unreadable row must not hide a later one
            if (r.type == KeenType && r.subtype == KeenSubtype) *keen = true;
        }
    }
    for (int l = 0; l < 2; ++l) {
        const uintptr_t a = lists[l].arr;
        const int32_t cnt = lists[l].cnt;
        for (int32_t i = 0; i < cnt; ++i) {
            if (!readRaw(a, i, &r)) { out->dropped += static_cast<unsigned>(cnt - i); break; }
            char label[48] = {0};
            const bool haveLabel = g_label(r.type, label, sizeof label);
            if (haveLabel && skipLabel(label, r.subtype)) continue;
            if (out->nProps >= PROP_DIFF_MAX) { ++out->dropped; continue; }
            PropRow& p = out->props[out->nProps++];
            memset(&p, 0, sizeof p);
            p.type = r.type; p.subtype = r.subtype; p.costTable = r.costTable; p.costValue = r.costValue; p.param1 = r.param1;
            if (!haveLabel) { p.known = 0; continue; }
            PropStrings s;
            memset(&s, 0, sizeof s);
            g_propText(r.type, r.subtype, r.costValue, r.param1 == Param1None ? Param1None : r.param1Value, &s);
            s.name[sizeof s.name - 1] = 0; s.sub[sizeof s.sub - 1] = 0; s.val[sizeof s.val - 1] = 0;
            char value[176] = {0};
            appendTo(value, sizeof value, s.sub);
            if (s.sub[0] && s.val[0]) appendTo(value, sizeof value, " ");
            appendTo(value, sizeof value, s.val);
            buildText(&p, s.name[0] ? s.name : label, value);
            p.known = 1;
        }
    }
    return true;
}

// One item -> ItemFacts. id = (uint32_t)item (one id space, ItemCompare.h decision 1). critBonus = 0 (decision 3).
// ok = false when the base or the prop array is unreadable; the caller then treats the item as unreadable.
// The name falls back to "item" (C1 #5), which does not fail the read.
inline bool readFacts(uintptr_t item, ItemFacts* out) {
    *out = ItemFacts();
    out->id = static_cast<uint32_t>(item);
    saferead::beginScope();
    BaseRead b;
    if (!item || !readBase(item, &b)) return false;
    out->slotMask = b.equipMask;
    out->numDice = b.numDice; out->die = b.die; out->critThreat = b.critThreat; out->critMult = b.critMult;
    out->baseAC = b.baseAC; out->maxDex = b.maxDex;
    if (!readName(item, b.nameRef, out->name, sizeof out->name)) copyTo(out->name, sizeof out->name, "item");
    bool keen = false;
    if (!readProps(item, out, &keen)) return false;
    out->keen = keen;
    out->critBonus = 0;
    out->ok = true;
    return true;
}

// Equip screen: the viewed item from its raw row id (row+0x1D0, server object id, untranslated: S3 sec 1). 0 = show nothing.
inline uintptr_t viewedFromRow(uint32_t rawRowId) {
    if (!rawRowId) return 0;
    saferead::beginScope();
    uint32_t app = 0, objTable = 0;
    if (!rd(g_appGlobal, 0, &app) || !app || !rd(static_cast<uintptr_t>(app), 8, &objTable) || !objTable) return 0;
    return g_getItem(static_cast<uintptr_t>(objTable), rawRowId);
}

// Fills the 7 slots of `in` from the creature's inventory: right 0x10, left 0x20, armor 0x2, head 0x1, gauntlet 0x8, implant 0x200, belt 0x400.
// One single-bit InvGetItemInSlot call per slot. Empty = 0 returned; Unreadable = inventory unreadable or an item that fails readFacts
// (an equipped item that does not read hides the whole compare, C1 #2).
// Live item pointer per slot (same order/bits as fillSlots); 0 = empty or unreadable. The equip screen PREVIEWS the clicked item on the
// creature, so a caller that needs the truly equipped items snapshots these once (see ISF cmpOnSelect) and re-reads facts from the snapshot.
inline void slotItemPtrs(uintptr_t creature, uintptr_t out[7]) {
    for (unsigned k = 0; k < 7; ++k) out[k] = 0;
    saferead::beginScope();
    uint32_t inv = 0;
    if (!(creature != 0 && rd(creature, Creature_Inv, &inv) && inv != 0 &&
          saferead::probeRead(SiteCmp, reinterpret_cast<const void*>(static_cast<uintptr_t>(inv)), 0x20))) return;
    static const uint32_t bits[7] = { 0x10, 0x20, 0x2, 0x1, 0x8, 0x200, 0x400 };
    for (unsigned k = 0; k < 7; ++k) out[k] = g_invSlot(static_cast<uintptr_t>(inv), bits[k]);
}
// Fills the 7 slots from item pointers (0 = Empty). A pointer that no longer reads leaves the slot Unreadable.
inline void fillSlotsFromPtrs(const uintptr_t it[7], CompareInput* in) {
    SlotRead* slots[7] = { &in->right, &in->left, &in->armor, &in->head, &in->gauntlet, &in->implant, &in->belt };
    for (unsigned k = 0; k < 7; ++k) {
        SlotRead& s = *slots[k];
        s.state = SlotState::Unreadable; s.item.ok = false; s.item.id = 0;
        if (!it[k]) { s.state = SlotState::Empty; continue; }
        if (!saferead::probeRead(SiteCmp, reinterpret_cast<const void*>(it[k]), 0x20)) continue;
        if (readFacts(it[k], &s.item)) s.state = SlotState::Item;
    }
}

inline void fillSlots(uintptr_t creature, CompareInput* in) {
    saferead::beginScope();
    uint32_t inv = 0;
    const bool invOk = creature != 0 && rd(creature, Creature_Inv, &inv) && inv != 0 &&
                       saferead::probeRead(SiteCmp, reinterpret_cast<const void*>(static_cast<uintptr_t>(inv)), 0x20);   // g_invSlot derefs inv unguarded (QuickLoot.cpp:605)
    SlotRead* slots[7] = { &in->right, &in->left, &in->armor, &in->head, &in->gauntlet, &in->implant, &in->belt };
    static const uint32_t bits[7] = { 0x10, 0x20, 0x2, 0x1, 0x8, 0x200, 0x400 };
    for (unsigned k = 0; k < 7; ++k) {
        SlotRead& s = *slots[k];
        s.state = SlotState::Unreadable;
        s.item.ok = false;
        s.item.id = 0;
        if (!invOk) continue;
        const uintptr_t it = g_invSlot(static_cast<uintptr_t>(inv), bits[k]);
        if (!it) { s.state = SlotState::Empty; continue; }
        if (readFacts(it, &s.item)) s.state = SlotState::Item;
    }
}

}  // namespace itemcmpread
