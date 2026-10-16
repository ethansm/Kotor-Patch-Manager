// common_contract_test.cpp -- Patches/Common CONTRACT test for preview-3d (scaffold).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Scope is UNVERIFIED until provider code exists: this pins only the registry ABI (kpatch_api.h)
// and the KpLog entry points a provider calls from KPatchInit. Preview3dProvider.cpp also uses GameAddr.h
// (section 4: the constants it names) and K2Addr.h (the Creature_HandleR/L pair it logs). SafeRead.h and SceneView.h
// are used too, but need the host Win32 stub / fake engine (test.toml has neither): equip3d's contract test pins them.

#include "check.h"
#include "KpLog.h"
#include "FakeKPatchApi.h"
#include "GameAddr.h"
#include "K2Addr.h"
#include <cstddef>
#include <cstdint>

#define PCHECK(why, expr) ::kt::Check("preview-3d relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define PEQ(why, a, b) ::kt::CheckEq("preview-3d relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("preview-3d");
KP_CHANNEL(chCore, "core");

namespace {
constexpr const char kIface[] = "k2.preview3d";
constexpr uint32_t kVersion = 1;
int g_table = 0;   // stands in for the provider's interface table
}  // namespace

int main() {
    // 1. Registry ABI: a provider trusts api fields only inside struct_size.
    fakekp::reset();
    PEQ("fresh api: struct_size == sizeof(KPatchApi)", fakekp::api()->struct_size, sizeof(KPatchApi));
    fakekp::reset(8);
    PCHECK("struct_size 8 is shorter than the require field end (provider must refuse)",
           fakekp::api()->struct_size < offsetof(KPatchApi, require) + sizeof(void*));

    // 2. Registry: first provider of (name, major) wins; require matches name and major.
    fakekp::reset();
    const KPatchApi* api = fakekp::api();
    PEQ("provide k2.preview3d v1 accepted", api->provide(kIface, kVersion, &g_table), 0);
    PCHECK("duplicate provide(k2.preview3d, 1) is rejected", api->provide(kIface, kVersion, &g_table) != 0);
    PCHECK("require k2.preview3d v1 returns the provided table", api->require(kIface, kVersion) == &g_table);
    PCHECK("require k2.preview3d with another major returns null", api->require(kIface, 2) == nullptr);
    PCHECK("require of an unknown name returns null", api->require("k2.none", kVersion) == nullptr);
    PCHECK("provide with a null iface is rejected", api->provide("k2.null", kVersion, nullptr) != 0);

    // 3. KpLog entry points used from KPatchInit.
    PEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
    fakekp::reset();
    PEQ("KpLog_Init without kpatch.log service = 0", KpLog_Init(fakekp::api()), 0);
    fakekp::withLog();
    PEQ("KpLog_Init with kpatch.log service = 1", KpLog_Init(fakekp::api()), 1);
    KP_LOG(chCore, "provided %s v%d", kIface, 1);
    PCHECK("KP_LOG reaches the core service", !fakekp::state().logLines.empty());

    // 4. GameAddr.h constants the provider names instead of raw hex (Steam Aspyr), and K2Addr hand-handle offsets (logged, never identity).
    static_assert(gameaddr::New == 0x00919723, "preview-3d relies on: gameaddr::New == 0x00919723 (engine operator new)");
    static_assert(gameaddr::AppGlobal == 0x00A1B4A4, "preview-3d relies on: gameaddr::AppGlobal == 0x00A1B4A4 (*(app)+4 = item manager)");
    static_assert(gameaddr::GobVtable == 0x0098B5CC, "preview-3d relies on: gameaddr::GobVtable == 0x0098B5CC (Gob free / attach guard)");
    static_assert(gameaddr::ClientCreatureVtable == 0x0099EE14, "preview-3d relies on: gameaddr::ClientCreatureVtable == 0x0099EE14 (creature alloc / dtor guard)");
    PEQ("K2Addr Steam Creature_HandleR (logged next to the +0x238 item id)", ::kSteam.Creature_HandleR, 0x278);
    PEQ("K2Addr Steam Creature_HandleL (logged next to the +0x23C item id)", ::kSteam.Creature_HandleL, 0x27C);

    return ::kt::Report("common_contract_test (preview-3d)");
}
