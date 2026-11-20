// ItemRead.h -- guarded reads of the HUD item groups and of a server item's property 0 (hotbar adopts in CS7).
// Copied from action_hotbar_src: readGroup (ActionHotbar.cpp:196-228), serverItemFor (:482), readItemProp (:500), StackCount (HotbarConfig.h:85).
// Memory goes through saferead (guarded). The only engine calls are the two pure object-table lookups of serverItemFor
// (ClientToServerObjectId, GetItemByObjectId); a host test replaces them through g_clientToServer / g_getItem and the app global through g_appGlobal.
#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include "SafeRead.h"
#include "GameAddr.h"

namespace itemread {

constexpr unsigned SiteBase = 0x6D00;                      // saferead site id for every read here
constexpr int Hud_GroupLists = 0x78;                       // {entries ptr @+0, count @+4, cap @+8} * 7 groups
constexpr int EntryStride = 0x3C, Entry_Key = 0x8, Entry_Flags = 0x30, Entry_Icon = 0x20, MaxEntries = 256;
constexpr int32_t ItemKeyFlag = 0x40000000;                // HUD entry key of an item = object id | this
using gameaddr::ClientToServerObjectId;                   // thiscall(objTable, clientId) RET 4 -> server object id
using gameaddr::GetItemByObjectId;                         // thiscall(objTable, serverId) RET 4 -> item ptr (0 = none)
constexpr int Item_PropCount = 0x288, Item_PropArray = 0x290, PropStride = 0x1C, Prop_Type = 0, Prop_Row = 2, Prop_Subtype = 2, Prop_Usable = 0xC;   // Prop_Subtype: alias of Prop_Row (same +2 u16), for the equip-compare reader

struct Entry { int group = 0, index = 0, key = 0, flags = 0; std::string name, icon; };
struct ItemProp { int count = 0; int type = -1; int row = -1; int usable = 0; };

inline int rdInt(uintptr_t base, int off, int dflt = 0) { int v; return saferead::readAt(SiteBase, base, off, &v) ? v : dflt; }

// Reads group g's entry list (guarded; stops at the first unreadable entry). Returns the count read, 0 on an implausible list.
inline int readGroup(uintptr_t hud, int g, std::vector<Entry>* out) {
    out->clear();
    if (g < 0 || g > 6) return 0;
    int base = rdInt(hud, Hud_GroupLists + g * 12), count = rdInt(hud, Hud_GroupLists + 4 + g * 12);
    if (count < 0 || count > MaxEntries || base == 0 || count == 0) return 0;
    for (int i = 0; i < count; ++i) {
        uintptr_t e = (uintptr_t)base + (uintptr_t)i * EntryStride;
        if (!saferead::probeRead(SiteBase, reinterpret_cast<const void*>(e), EntryStride)) break;
        Entry en; en.group = g; en.index = i; en.key = rdInt(e, Entry_Key); en.flags = rdInt(e, Entry_Flags);
        char nb[96];
        if (saferead::readExo(SiteBase, e, nb, sizeof nb)) en.name = nb;
        char ib[17] = {0};
        if (saferead::probeRead(SiteBase, reinterpret_cast<const void*>(e + Entry_Icon), 16)) {
            memcpy(ib, reinterpret_cast<const void*>(e + Entry_Icon), 16);
            for (int k = 0; k < 16; ++k) if (ib[k] && (ib[k] < 0x20 || ib[k] > 0x7E)) ib[k] = '?';
        }
        en.icon = ib;
        out->push_back(en);
    }
    return (int)out->size();
}

// Item stack count from a HUD entry: the engine formats item names "%s (%d)" only when the stack is > 1 (FUN_0074CF00).
// Returns 0 for non-items (key bit 0x40000000 clear), for names without a trailing " (<digits>)" (no sign/spaces inside the
// parens), for "(0)" (the engine only prints counts > 1) and for more than 6 digits (not a plausible stack).
inline int stackCount(int key, const std::string& name) {
    if (!(key & ItemKeyFlag) || name.size() < 4 || name.back() != ')') return 0;
    size_t p = name.rfind(" (");
    if (p == std::string::npos || p + 3 > name.size() - 1) return 0;
    int n = 0;
    if (name.size() - 1 - (p + 2) > 6) return 0;
    for (size_t i = p + 2; i < name.size() - 1; ++i) { if (name[i] < '0' || name[i] > '9') return 0; n = n * 10 + (name[i] - '0'); }
    return n;
}

using LookupFn = uint32_t(__thiscall*)(uintptr_t objTable, uint32_t id);
inline uint32_t defaultClientToServer(uintptr_t t, uint32_t id) { return reinterpret_cast<LookupFn>(ClientToServerObjectId)(t, id); }
inline uint32_t defaultGetItem(uintptr_t t, uint32_t id) { return reinterpret_cast<LookupFn>(GetItemByObjectId)(t, id); }
using LookupHook = uint32_t(*)(uintptr_t objTable, uint32_t id);
inline LookupHook g_clientToServer = defaultClientToServer;
inline LookupHook g_getItem = defaultGetItem;
inline uintptr_t g_appGlobal = gameaddr::AppGlobal;

// Client item id -> engine CSWSItem* (app -> objTable(+8) -> ClientToServerObjectId -> GetItemByObjectId); 0 on any failure.
inline uintptr_t serverItemFor(uint32_t clientItem, uint32_t* serverOut = nullptr) {
    uint32_t app = 0, objTable = 0;   // objTable = *(app+8), exactly as 0x77c7aa (NOT app+4, which is the client/party side)
    if (!saferead::readAt<uint32_t>(SiteBase, g_appGlobal, 0, &app) || !app ||
        !saferead::readAt<uint32_t>(SiteBase, app, 8, &objTable) || !objTable) return 0;
    if (!g_clientToServer || !g_getItem) return 0;
    uint32_t serverItem = g_clientToServer(objTable, clientItem);
    if (!serverItem) return 0;
    uintptr_t itemPtr = g_getItem(objTable, serverItem);
    if (!itemPtr) return 0;
    if (serverOut) *serverOut = serverItem;
    return itemPtr;
}

// Item property 0 (count +0x288, array +0x290, stride 0x1C: +0 type, +2 spells.2da row, +0xC usable flag); unreadable -> count 0.
inline ItemProp readItemProp(uintptr_t itemPtr) {
    ItemProp p; int32_t cnt = 0; uint32_t arr = 0;
    if (!saferead::readAt<int32_t>(SiteBase, itemPtr, Item_PropCount, &cnt) || !saferead::readAt<uint32_t>(SiteBase, itemPtr, Item_PropArray, &arr)) return p;
    if (cnt > 0 && arr) {
        int16_t ty = 0, row = 0; uint8_t us = 0;
        if (saferead::readAt<int16_t>(SiteBase, arr, Prop_Type, &ty) && saferead::readAt<int16_t>(SiteBase, arr, Prop_Row, &row) && saferead::readAt<uint8_t>(SiteBase, arr, Prop_Usable, &us)) { p.count = cnt; p.type = ty; p.row = row; p.usable = us; }
    }
    return p;
}

} // namespace itemread
