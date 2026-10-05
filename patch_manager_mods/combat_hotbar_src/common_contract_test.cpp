// common_contract_test.cpp -- Patches/Common CONTRACT test for Combat Action Hotbar (Mod 31).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins semantics relied on by CombatHotbar.cpp in SafeRead.h and KpLog.h.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SafeRead.h"
#include "KpLog.h"
#include "check.h"

#define HCHECK(why, expr) ::kt::Check("combat-hotbar relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("combat-hotbar relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("combat-hotbar");
KP_CHANNEL(chTarget, "target");

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", arena != nullptr && arena != MAP_FAILED);

    uintptr_t menu = reinterpret_cast<uintptr_t>(arena);
    saferead::beginScope();

    // 1. Verify readAt for int and int8_t
    int8_t charIdx = 1;
    memcpy(arena + 0x1BAA, &charIdx, sizeof(charIdx));
    int8_t outChar = -1;
    HCHECK("readAt charIdx succeeds", saferead::readAt(1, menu, 0x1BAA, &outChar));
    HEQ("readAt charIdx correct", outChar, 1);

    int count0 = 5;
    memcpy(arena + 0x04, &count0, sizeof(count0));
    int outCount = -1;
    HCHECK("readAt count succeeds", saferead::readAt(2, menu, 0x04, &outCount));
    HEQ("readAt count correct", outCount, 5);

    // 2. Verify readExoLoose
    const char* testStr = "Master Flurry";
    memcpy(arena + 0x500, testStr, strlen(testStr) + 1);
    uint32_t strPtr = static_cast<uint32_t>(menu + 0x500);
    int32_t strLen = static_cast<int32_t>(strlen(testStr));
    char nameBuf[32] = {0};
    int nCopied = saferead::readExoLoose(3, strPtr, strLen, nameBuf, sizeof(nameBuf));
    HCHECK("readExoLoose copied bytes", nCopied > 0);
    HEQ("readExoLoose string matches", (const char*)nameBuf, testStr);

    // 2b. Verify readExo (CExoString {p, len} struct read)
    uint32_t exoStruct[2] = { static_cast<uint32_t>(menu + 0x500), static_cast<uint32_t>(strlen(testStr)) };
    memcpy(arena + 0x700, exoStruct, sizeof(exoStruct));
    char exoBuf[32] = {0};
    int exoLen = saferead::readExo(5, menu + 0x700, exoBuf, sizeof(exoBuf));
    HEQ("readExo returns length", exoLen, static_cast<int>(strlen(testStr)));
    HEQ("readExo string matches", (const char*)exoBuf, testStr);
    char tinyBuf[4] = {0};
    HEQ("readExo rejects len >= cap", saferead::readExo(5, menu + 0x700, tinyBuf, sizeof(tinyBuf)), 0);

    // 3. Verify formatResRef16 / readResRef16
    char rawRef[16] = { 'i', 'p', '_', 'f', 'l', 'u', 'r', 'r', 'y', 0, 0, 0, 0, 0, 0, 0 };
    memcpy(arena + 0x600, rawRef, 16);
    char iconBuf[17] = {0};
    HCHECK("readResRef16 succeeds", saferead::readResRef16(4, menu + 0x600, iconBuf, 0));
    HEQ("readResRef16 matches", (const char*)iconBuf, "ip_flurry");

    munmap(arena, 0x10000);
    printf("common_contract_test (combat-hotbar): PASS\n");
    return 0;
}
