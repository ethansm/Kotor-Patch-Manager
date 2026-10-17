// common_contract_test.cpp -- Patches/Common CONTRACT test for Enemy Effects Strip (0.3.0-dev).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins the semantics EnemyStrip.cpp relies on in SafeRead.h, KpLog.h, HudHost.h, K2Addr.h, Party.h, Effects.h, EffectIcons.h and GuiKit.h.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <type_traits>
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "SafeRead.h"
#include "KpLog.h"
#include "HudHost.h"
#include "K2Addr.h"
#include "Party.h"
#include "Effects.h"
#include "EffectIcons.h"
#include "GuiKit.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define ECHECK(why, expr) ::kt::Check("enemy-effects-strip relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define EEQ(why, a, b) ::kt::CheckEq("enemy-effects-strip relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("enemy-effects-strip");

// Party.h engine hook (MemberServerCreature, thiscall): records the call and answers member + 0x5000.
static int g_srvCalls = 0;
static uintptr_t g_srvArg = 0;
static uintptr_t __thiscall fakeServerCreature(void* m) {
    ++g_srvCalls;
    g_srvArg = reinterpret_cast<uintptr_t>(m);
    return reinterpret_cast<uintptr_t>(m) + 0x5000;
}

// GuiKit.h imageFor loader fake: counts engine loads; "failnull_img" fails.
static int g_loads = 0;
static void* fakeLoadImage(const char* n) {
    ++g_loads;
    if (!strcmp(n, "failnull_img")) return nullptr;
    return reinterpret_cast<void*>(static_cast<uintptr_t>(0x7000u + 8u * static_cast<unsigned>(g_loads)));
}

// Signatures EnemyStrip.cpp calls (a changed shared signature fails the build of this test, not the live game).
#define SIG(fn, ...) static_assert(std::is_same<decltype(&fn), __VA_ARGS__>::value, #fn " signature changed")
SIG(guikit::createLabelFromTemplate, uintptr_t (*)(uintptr_t, const char*, int*, uint32_t*));
SIG(guikit::appendToPanel, bool (*)(uintptr_t, uintptr_t));
SIG(guikit::setRect, bool (*)(uintptr_t, int, int, int, int));
SIG(guikit::setFill, bool (*)(uintptr_t, void*, const float*));
SIG(guikit::setAlpha, bool (*)(uintptr_t, float));
SIG(guikit::setBorderTint, bool (*)(uintptr_t, const float*));
SIG(guikit::setVisible, bool (*)(uintptr_t, bool));
SIG(guikit::setClickable, bool (*)(uintptr_t, bool));
SIG(guikit::imageFor, void* (*)(const char*, bool*, int*));
SIG(k2fx::iconForSpell, const char* (*)(int, bool, bool*));
SIG(k2fx::iconForEffectIconEffect, const char* (*)(int, bool*));
SIG(k2fx::effectRatio, float (*)(uintptr_t, int, int, bool, float));
SIG(k2fx::currentTime, bool (*)(int*, int*));
SIG(party::serverCreature, uintptr_t (*)(uintptr_t));
SIG(party::table, uintptr_t (*)(party::Fail*));
SIG(party::member, uintptr_t (*)(uintptr_t, int));
SIG(party::partyCount, int (*)(uintptr_t));
SIG(k2fx::remainingRatio, float (*)(int, int, float, int, int, float));
SIG(k2hud::Client::add, bool (k2hud::Client::*)(int32_t, int32_t, K2HudCallback, void*));
#undef SIG

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    uint8_t* arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    const uintptr_t base = reinterpret_cast<uintptr_t>(arena);
    g_stubVirtualQueryAll = true;   // host stub: every address reads as committed memory, so readAt fails only on base 0
    saferead::beginScope();

    // 1. SafeRead: arena allocated; readAt int32 (menu +0x1AA0 bar max) and int8 (leader stats +0x174..+0x17B skill ranks) round-trip.
    const int32_t hpMax = 0x1234;
    memcpy(arena + 0x1AA0, &hpMax, sizeof(hpMax));
    int32_t gotHp = 0;
    const int8_t idx3 = -3;
    memcpy(arena + 0x300 + 0x174 + 3, &idx3, sizeof(idx3));
    int8_t gotSkill = 0;
    ECHECK("arena allocated; readAt int32 and int8 round-trip through it",
           arena != nullptr && arena != MAP_FAILED &&
           saferead::readAt(0x7A05, base, 0x1AA0, &gotHp) && gotHp == 0x1234 &&
           saferead::readAt(0x7A0B, base + 0x300, 0x174 + 3, &gotSkill) && gotSkill == -3);

    // 2. SafeRead: a null base (no target, no leader, no stats) is a failed read, not a dereference.
    int32_t dead = 0;
    ECHECK("readAt with base 0 fails (no target / leader)", !saferead::readAt(0x7A0A, 0, 0x1198, &dead));

    // 3. KpLog: KpLog_Init without a kpatch.log provider is inert (returns 0); the patch-level macro compiles.
    fakekp::reset();
    EEQ("KpLog_Init without a kpatch.log provider returns 0", KpLog_Init(fakekp::api()), 0);

    // 4. HudHost.h: K2HudContext layout EnemyStrip.cpp reads (struct_size, menu, leader, target, targetType) and the POST phase.
    ECHECK("K2HUD_TARGET_MENU_POST is phase 2; K2HudContext v1 layout: size 40, menu 16, leader 20, target 28, targetType 32",
           K2HUD_TARGET_MENU_POST == 2 && sizeof(K2HudContext) == 40 && offsetof(K2HudContext, menu) == 16 &&
           offsetof(K2HudContext, leader) == 20 && offsetof(K2HudContext, target) == 28 && offsetof(K2HudContext, targetType) == 32);

    // 5. HudHost.h: add(TARGET_MENU_POST) is accepted, and connect() without hud-host is inert (the probe still loads).
    k2hud::Client hud;
    hud.init(fakekp::api(), "enemy-effects-strip");
    ECHECK("add() accepts the TARGET_MENU_POST callback, connect() without hud-host is false and leaves the client unconnected",
           hud.add(K2HUD_TARGET_MENU_POST, 100, [](const K2HudContext*, void*) {}) && !hud.connect() && !hud.connected);

    // 6. K2Addr.h + Effects.h: the Steam table offsets EnemyStrip.cpp logs (leader stats, creature effect list, effect fields)
    //    equal the raw constants Effects.h uses, so the two cannot drift.
    k2addr::setForTest(&kSteam);
    ECHECK("Steam table: Creature_Stats 0x1198, effect array 0x148, count 0x14C, and the Effects.h effect field constants match",
           k2addr::off(&K2AddrTable::Creature_Stats) == 0x1198 &&
           k2addr::off(&K2AddrTable::Creature_EffectArray) == 0x148 &&
           k2addr::off(&K2AddrTable::Creature_EffectCount) == 0x14C &&
           k2addr::off(&K2AddrTable::Effect_Type) == k2fx::Effect_Type &&
           k2addr::off(&K2AddrTable::Effect_Duration) == k2fx::Effect_Duration &&
           k2addr::off(&K2AddrTable::Effect_ExpiryDay) == k2fx::Effect_ExpiryDay &&
           k2addr::off(&K2AddrTable::Effect_ExpiryTime) == k2fx::Effect_ExpiryTime &&
           k2addr::off(&K2AddrTable::Effect_Creator) == k2fx::Effect_Creator &&
           k2addr::off(&K2AddrTable::Effect_SpellId) == k2fx::Effect_SpellId);
    k2addr::clearForTest();

    // 7. Effects.h: effect cap (24 logged of up to 512) and the remaining-duration ratio used for the per-effect ratio log.
    ECHECK("k2fx::MaxEffects is 512; remainingRatio: 10 s left of 20 s = 0.5, over-full clamps to 1",
           k2fx::MaxEffects == 512 && k2fx::remainingRatio(0, 10000, 20.0f, 0, 0) == 0.5f && k2fx::remainingRatio(0, 50000, 20.0f, 0, 0) == 1.0f);

    // 8. EffectIcons.h: iconForSpell returns a buff resref for a tracked row, and nullptr for out-of-range rows (both lists).
    bool item = false;
    bool anyBuff = false;
    for (int s = 0; s < kEffectIconTableSize && !anyBuff; ++s) anyBuff = k2fx::iconForSpell(s, false, &item) != nullptr;
    ECHECK("iconForSpell: some buff row has an icon, and rows -1 and table-size give nullptr (buff and debuff)",
           anyBuff && k2fx::iconForSpell(-1, false, &item) == nullptr && k2fx::iconForSpell(kEffectIconTableSize, false, &item) == nullptr &&
           k2fx::iconForSpell(-1, true, &item) == nullptr && k2fx::iconForSpell(kEffectIconTableSize, true, &item) == nullptr);

    // 9. K2Addr.h Steam table: the icon cache (data +0xFFC, count +0x1000), the effect subtype and ints fields the 0.2.0 dumps read,
    //    and the MemberServerCreature engine address that Party.h's serverCreature calls.
    k2addr::setForTest(&kSteam);
    ECHECK("Steam table: icon cache data 0xFFC and count 0x1000, effect subtype 0x0A, ints count 0x30 and ints ptr 0x34, MemberServerCreature 0x0077D800",
           k2addr::off(&K2AddrTable::Creature_IconCacheData) == 0xFFC && k2addr::off(&K2AddrTable::Creature_IconCacheCount) == 0x1000 &&
           k2addr::off(&K2AddrTable::Effect_Subtype) == 0x0A && k2addr::off(&K2AddrTable::Effect_IntCount) == 0x30 &&
           k2addr::off(&K2AddrTable::Effect_Ints) == 0x34 && kSteam.MemberServerCreature == 0x0077D800u);
    k2addr::clearForTest();

    // 10. Party.h serverCreature (the probe resolves target and leader through it): 0 for a 0 member with no engine call; a non-zero
    //     client member goes to the engine accessor once and its answer comes back; a table without the accessor gives 0.
    party::g_serverCreature = fakeServerCreature;
    g_srvCalls = 0;
    const uintptr_t cli = base + 0x100;
    ECHECK("party::serverCreature(0) is 0 with no engine call", party::serverCreature(0) == 0 && g_srvCalls == 0);
    ECHECK("party::serverCreature(client) returns the engine answer for that client member (one call, same pointer)",
           party::serverCreature(cli) == cli + 0x5000 && g_srvCalls == 1 && g_srvArg == cli);
    party::g_serverCreature = party::useTableFn<party::ServerCreatureFn>();
    K2AddrTable noMem = kSteam;
    noMem.MemberServerCreature = 0;
    k2addr::setForTest(&noMem);
    ECHECK("party::serverCreature with the table's MemberServerCreature unknown returns 0", party::serverCreature(cli) == 0);
    k2addr::clearForTest();

    // 11. HudHost.h BUILD phase: K2HUD_BUILD is phase 3 and K2HudContext.panel (offset 36, last field) is what onHudBuild reads and
    //     onTargetMenuPost compares ctx->hud against; hud (offset 12) is the identity-guard field.
    ECHECK("K2HUD_BUILD is phase 3; K2HudContext hud at 12 and panel at 36",
           K2HUD_BUILD == 3 && offsetof(K2HudContext, hud) == 12 && offsetof(K2HudContext, panel) == 36);

    // 12. GuiKit.h imageFor: one engine load per resref (called per icon per refresh), case-insensitive, a failed load is pooled
    //     (the 'image not found' warn path must not ask the engine again).
    guikit::g_gk.loadImage = fakeLoadImage;
    g_loads = 0;
    void* im = guikit::imageFor("ip_stun");
    ECHECK("imageFor loads once and returns the same wrapper for a repeat and an upper-case resref",
           im != nullptr && g_loads == 1 && guikit::imageFor("ip_stun") == im && guikit::imageFor("IP_STUN") == im && g_loads == 1);
    ECHECK("imageFor: a failed load gives nullptr and is pooled (no second engine call)",
           guikit::imageFor("failnull_img") == nullptr && g_loads == 2 && guikit::imageFor("failnull_img") == nullptr && g_loads == 2);
    ECHECK("imageFor: null and empty resrefs give nullptr without a load", guikit::imageFor(nullptr) == nullptr && guikit::imageFor("") == nullptr && g_loads == 2);

    // 13. Guards EnemyStrip.cpp relies on to fail soft: label creation with no panel returns 0 (no engine call), the label setters
    //     refuse control 0, and effectRatio without a world timer is 1.0 without reading the effect.
    int why = -1;
    ECHECK("createLabelFromTemplate(panel 0) returns 0 and reports an alloc/guard failure",
           guikit::createLabelFromTemplate(0, "LBL_QUEUE1", &why) == 0 && why != guikit::CF_None);
    const float white[3] = { 1.f, 1.f, 1.f };
    ECHECK("setRect/setFill/setAlpha/setBorderTint/setVisible/setClickable refuse control 0 (no crash)",
           !guikit::setRect(0, 1, 2, 3, 4) && !guikit::setFill(0, nullptr, nullptr) && !guikit::setAlpha(0, 1.f) &&
           !guikit::setBorderTint(0, white) && !guikit::setVisible(0, true) && !guikit::setClickable(0, false));
    ECHECK("effectRatio with haveTime=false is 1.0 (permanent/unknown) and never reads the effect",
           k2fx::effectRatio(0, 0, 0, false) == 1.0f);
    ECHECK("EffectIcons.h: type-0x43 EffectIcon effects are k2fx::EffType_Icon == 0x43; iconForEffectIconEffect on an unreadable effect is nullptr",
           k2fx::EffType_Icon == 0x43 && [] { bool it = true; const char* r = k2fx::iconForEffectIconEffect(0, &it); return r == nullptr && !it; }());

    munmap(arena, 0x10000);
    return ::kt::Report("common_contract_test (enemy-effects-strip)");
}
