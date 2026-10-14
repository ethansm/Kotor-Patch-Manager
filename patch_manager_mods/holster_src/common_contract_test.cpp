// common_contract_test.cpp -- Patches/Common CONTRACT test for holster (doc 42).
// Pins what Holster.cpp relies on: K2Addr.h Steam addresses and creature offsets as INDEPENDENT literals (never read back from the
// table), the GOG table staying inert for them, SafeRead.h readAt/null/unreadable behaviour for the creature and Gob fields the
// probe reads, and KpLog.h fail-soft init. Every message starts with "holster relies on: ". Host-only, deterministic, no game.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#include "SafeRead.h"
#include "KpLog.h"
#include "K2Addr.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("holster relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("holster relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("holster");
KP_CHANNEL(chMain, "main");

// 1. addresses the six hooks sit on (compile time, independent literals; kotor2-steam-aspyr.hooks.toml uses the same values)
#define KADDR(f, val) static_assert(kSteam.f == (val), "holster relies on: kSteam." #f " == " #val " (hook site / engine function)")
KADDR(ClientCombatStateSet, 0x007682B0);
KADDR(WeaponAttach, 0x007637E0);
KADDR(SwapWeaponsExec, 0x005DF920);
KADDR(AnimResolve, 0x00863650);
KADDR(AnimSend, 0x005644B0);
KADDR(GobUpdateDependent, 0x005047B0);
KADDR(GobAttachByName, 0x00461670);
KADDR(GobSetPosition, 0x00461490);
KADDR(GobSetOrientation, 0x004614E0);
#define KOFF(f, val) static_assert(kSteam.f == (val), "holster relies on: kSteam." #f " == " #val " (creature offset)")
KOFF(Creature_ClientCombat, 0x138);
KOFF(Creature_BodyObj, 0x68);
KOFF(Creature_HandleR, 0x278);
KOFF(Creature_HandleL, 0x27C);
KOFF(Creature_AltItemR, 0x264);
KOFF(Creature_AltItemL, 0x268);
static_assert(kGog.ClientCombatStateSet == 0 && kGog.WeaponAttach == 0 && kGog.SwapWeaponsExec == 0 && kGog.GobUpdateDependent == 0,
              "holster relies on: GOG table has no holster addresses (zero = helper inert)");
static_assert(kGog.Creature_ClientCombat == 0 && kGog.Creature_BodyObj == 0, "holster relies on: GOG creature offsets stay zero");

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", arena != nullptr && arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;   // host stub: all memory reads as one committed read-write region
    const uintptr_t A = reinterpret_cast<uintptr_t>(arena);

    // 2. selection: unset env -> Steam; an unknown exe -> no table (the probe's table=none line, every offset read 0)
    HCHECK("unset sha selects the Steam table", k2addr::selectBySha("") == &kSteam);
    HCHECK("unknown sha selects no table", k2addr::selectBySha("DEADBEEF") == nullptr);
    k2addr::setForTest(&kSteam);
    HEQ("off() reads the combat-state offset from the forced table", k2addr::off(&K2AddrTable::Creature_ClientCombat), 0x138);
    k2addr::setForTest(nullptr);
    HEQ("off() is 0 for an unknown exe", k2addr::off(&K2AddrTable::Creature_ClientCombat), 0);
    k2addr::clearForTest();

    // 3. creature / Gob fields the handlers read: readAt returns the stored value; null base and tiny addresses fail without a fault
    {
        saferead::beginScope();
        const uintptr_t creature = A + 0x1000;
        *reinterpret_cast<uint32_t*>(creature + 0x138) = 2;
        *reinterpret_cast<uint32_t*>(creature + 0x68) = 0xB0D1;
        *reinterpret_cast<uint32_t*>(creature + 0x278) = 0x11111111u;
        *reinterpret_cast<uint32_t*>(creature + 0x27C) = 0x22222222u;
        uint32_t v = 0;
        HCHECK("readAt reads the combat int at +0x138", saferead::readAt(1u, creature, 0x138, &v) && v == 2);
        HCHECK("readAt reads the body object at +0x68", saferead::readAt(1u, creature, 0x68, &v) && v == 0xB0D1);
        HCHECK("readAt reads both hand handles", saferead::readAt(1u, creature, 0x278, &v) && v == 0x11111111u && saferead::readAt(1u, creature, 0x27C, &v) && v == 0x22222222u);
        const uintptr_t gob = A + 0x2000;
        *reinterpret_cast<float*>(gob + 0xA4) = 0.25f;
        *reinterpret_cast<float*>(gob + 0xB0) = 1.0f;
        float f = 0.f;
        HCHECK("readAt reads the Gob local position / quaternion floats", saferead::readAt(2u, gob, 0xA4, &f) && f == 0.25f && saferead::readAt(2u, gob, 0xB0, &f) && f == 1.0f);
        v = 77;
        HCHECK("readAt on a null base fails and leaves the output untouched", !saferead::readAt(3u, 0, 0x138, &v) && v == 77);
        HCHECK("readAt below 0x10000 fails", !saferead::readAt(3u, 0x100, 0x138, &v));
    }

    // 4. KpLog: init with no API is fail-soft (the probe must keep running if the log core is old)
    {
        KpLog_Init(nullptr);
        KP_LOG(chMain, "contract probe line");
        HCHECK("KpLog_Init(nullptr) does not crash and logging afterwards is safe", true);
    }
    return kt::Report("holster_common_contract");
}
