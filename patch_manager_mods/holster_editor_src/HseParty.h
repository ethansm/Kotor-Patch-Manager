#pragma once
#include <stdint.h>
#include "Party.h"
#include "SafeRead.h"
#include "GameAddr.h"
#include "K2Addr.h"

namespace hse {

// Number of party members (0 when the table is unavailable).
inline int memberCount() {
    uintptr_t t = party::table();
    return t ? party::partyCount(t) : 0;
}

// Client creature of party position i (0 = leader), or 0.
inline uintptr_t memberAt(int i) {
    uintptr_t t = party::table();
    return (t && i >= 0) ? party::member(t, i) : 0;
}

// First name of the member into out (NUL terminated, at most cap-1 chars). Returns true only when a non-empty name was read; false leaves out[0] = 0. Every read goes through saferead; the CExoString is always freed.
inline bool memberName(uintptr_t clientMember, char* out, int cap) {
    saferead::beginScope();
    if (!out || cap <= 0) return false;
    out[0] = 0;
    if (!clientMember) return false;
    uintptr_t S = party::serverCreature(clientMember);
    if (!S) return false;
    unsigned char objType = 0;
    if (!saferead::readAt<unsigned char>(0xE70110u, S, 8, &objType) || objType != 5) return false;
    int statsOff = k2addr::off(&K2AddrTable::Creature_Stats);
    if (!statsOff) return false;
    uintptr_t stats = 0;
    if (!saferead::readAt<uintptr_t>(0xE70111u, S, statsOff, &stats) || !stats) return false;
    uintptr_t H = stats + 0x34; // CExoLocString
    uintptr_t cacheObj = 0;
    if (!saferead::readAt<uintptr_t>(0xE70112u, H, 0, &cacheObj) || !cacheObj) return false;
    uintptr_t appGlobalAddr = gameaddr::AppGlobal;
    if (!appGlobalAddr) return false;
    uintptr_t appGlobal = 0;
    if (!saferead::readAt<uintptr_t>(0xE70113u, appGlobalAddr, 0, &appGlobal) || !appGlobal) return false;
    uintptr_t app = 0;
    if (!saferead::readAt<uintptr_t>(0xE70114u, appGlobal, 4, &app) || !app) return false;
    uintptr_t app4 = 0;
    if (!saferead::readAt<uintptr_t>(0xE70115u, app, 4, &app4) || !app4) return false;
    if (!gameaddr::GetClientLanguage || !gameaddr::GetLocString || !gameaddr::ExoDtor) return false;
    int language = reinterpret_cast<int(__fastcall*)(int)>(gameaddr::GetClientLanguage)(app);
    int exo[2] = {0, 0};
    bool ok = reinterpret_cast<bool(__thiscall*)(void*, int, void*, unsigned char)>(gameaddr::GetLocString)(reinterpret_cast<void*>(H), language, exo, 0);
    bool gotText = false;
    if (ok && exo[0]) {
        char tmp[256];
        if (saferead::readExo(0xE70116u, reinterpret_cast<uintptr_t>(exo), tmp, sizeof(tmp), 255, false)) {
            if (tmp[0] != '\0') {
                int k = 0;
                while (tmp[k] && k < cap - 1) { out[k] = tmp[k]; ++k; }
                out[k] = 0;
                gotText = (out[0] != '\0');
            }
        }
    }
    if (exo[0]) {
        reinterpret_cast<void(__thiscall*)(void*)>(gameaddr::ExoDtor)(exo);
    }
    return gotText;
}

} // namespace hse
