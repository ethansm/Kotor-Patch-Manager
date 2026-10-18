// common_contract_test.cpp -- Patches/Common CONTRACT test for holster-editor (Preview3dApi.h, HolsterApi.h, ModConfigApi.h, GuiKit.h, Party.h, SafeRead.h).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// This pins:
//   - k2p3::Client::usable (Preview3dApi.h: k2.preview3d v1)
//   - k2holster::Client::usable (HolsterApi.h: k2.holster v1)
//   - k2mc::ConfigClient / MenuClient (ModConfigApi.h: k2.modconfig v1, k2.menuui v1)
//   - guikit::hit / cursorEngine (GuiKit.h)
//   - party::table / partyCount / member / serverCreature (Party.h)
//   - saferead::readExo (SafeRead.h)
// against the fake registry and host test environment.

#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif

#include "check.h"
#include "FakeKPatchApi.h"
#include "Preview3dApi.h"
#include "HolsterApi.h"
#include "ModConfigApi.h"
#include "GuiKit.h"
#include "Party.h"
#include "SafeRead.h"
#include "K2Addr.h"
#include "GameAddr.h"
#include "HseView.h"

#include <sys/mman.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstring>

#define HCHECK(why, expr) ::kt::Check("holster-editor relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("holster-editor relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

namespace {

// Arena memory for saferead and engine struct fakes
uint8_t* g_arena = nullptr;
size_t g_arenaOff = 0;

uintptr_t arenaAlloc(size_t bytes) {
    size_t align = 16;
    size_t aligned = (g_arenaOff + align - 1) & ~(align - 1);
    if (aligned + bytes > 0x10000) return 0;
    g_arenaOff = aligned + bytes;
    return reinterpret_cast<uintptr_t>(g_arena + aligned);
}

template <typename T>
T& at(uintptr_t base, int off) {
    return *reinterpret_cast<T*>(base + off);
}

// ---- ModConfigApi stubs ----
int32_t KPATCH_CALL stubCfgRegister(const K2ModConfigEntry*) { return 1; }
void KPATCH_CALL stubCfgUnregister(int32_t) {}

uint32_t KPATCH_CALL stubMuiNewPage(void*, const char*) { return 1; }
int32_t KPATCH_CALL stubMuiAddButton(uint32_t, const char*, const char*) { return 0; }
int32_t KPATCH_CALL stubMuiAddLabel(uint32_t, const char*) { return 0; }
int32_t KPATCH_CALL stubMuiAddCustom(uint32_t, void*, int32_t) { return 0; }
int32_t KPATCH_CALL stubMuiClicked(uint32_t, int32_t) { return 0; }
void KPATCH_CALL stubMuiSetText(uint32_t, int32_t, const char*) {}
void KPATCH_CALL stubMuiSetRect(uint32_t, int32_t, const K2MuiRect*) {}
void KPATCH_CALL stubMuiSetDesc(uint32_t, const char*) {}
int32_t KPATCH_CALL stubMuiPaneRect(void*, int32_t, K2MuiRect*) { return 0; }
void* KPATCH_CALL stubMuiControl(uint32_t, int32_t) { return nullptr; }

const K2ModConfigApi kCfgTable = { sizeof(K2ModConfigApi), stubCfgRegister, stubCfgUnregister };
const K2MenuUiApi kMuiTable = { sizeof(K2MenuUiApi), stubMuiNewPage, stubMuiAddButton, stubMuiAddLabel, stubMuiAddCustom,
                                stubMuiClicked, stubMuiSetText, stubMuiSetRect, stubMuiSetDesc, stubMuiPaneRect, stubMuiControl };

// ---- Preview3dApi stubs ----
uint32_t KPATCH_CALL stubP3CreateView(void*, const char*) { return 1; }
int32_t KPATCH_CALL stubP3Bind(uint32_t, void*) { return 1; }
int32_t KPATCH_CALL stubP3Rebuild(uint32_t) { return 1; }
void KPATCH_CALL stubP3SetWeapons(uint32_t, int32_t) {}
void* KPATCH_CALL stubP3WeaponGob(uint32_t, int32_t) { return nullptr; }
void KPATCH_CALL stubP3SetView(uint32_t, float, float, float) {}
void KPATCH_CALL stubP3PlayAnim(uint32_t, const char*) {}
void* KPATCH_CALL stubP3RigGob(uint32_t, int32_t) { return nullptr; }
void KPATCH_CALL stubP3Destroy(uint32_t) {}
void KPATCH_CALL stubP3SetBeforeFree(K2Preview3dBeforeFree, void*) {}
void* KPATCH_CALL stubP3ViewControl(uint32_t) { return nullptr; }

const K2Preview3dApi kP3Table = {
    sizeof(K2Preview3dApi),
    stubP3CreateView,
    stubP3Bind,
    stubP3Rebuild,
    stubP3SetWeapons,
    stubP3WeaponGob,
    stubP3SetView,
    stubP3PlayAnim,
    stubP3RigGob,
    stubP3Destroy,
    stubP3SetBeforeFree,
    stubP3ViewControl
};

// ---- HolsterApi stubs ----
int32_t KPATCH_CALL stubHolRegisterPreview(void*, void*, int32_t) { return 1; }
void KPATCH_CALL stubHolUnregisterPreview(void*) {}
int32_t KPATCH_CALL stubHolBeginEdit(void*, int32_t) { return 1; }
void KPATCH_CALL stubHolNudgeWorld(const float[4], const float[3]) {}
void KPATCH_CALL stubHolNudgeLocal(int32_t, float, float) {}
void KPATCH_CALL stubHolSetValues(const float[3], const float[3]) {}
int32_t KPATCH_CALL stubHolCycleBone(int32_t) { return 1; }
int32_t KPATCH_CALL stubHolGetEdit(K2HolsterEditInfo*) { return 1; }
int32_t KPATCH_CALL stubHolGetMountWorld(float[3], float[4]) { return 1; }
int32_t KPATCH_CALL stubHolGetNodeWorld(void*, const char*, float[3], float[4]) { return 1; }
int32_t KPATCH_CALL stubHolSetScope(int32_t, int32_t) { return 1; }
int32_t KPATCH_CALL stubHolSave() { return 1; }
void KPATCH_CALL stubHolRevert() {}
void KPATCH_CALL stubHolResetScope() {}
void KPATCH_CALL stubHolEndEdit() {}

const K2HolsterApi kHolTable = {
    sizeof(K2HolsterApi),
    stubHolRegisterPreview,
    stubHolUnregisterPreview,
    stubHolBeginEdit,
    stubHolNudgeWorld,
    stubHolNudgeLocal,
    stubHolSetValues,
    stubHolCycleBone,
    stubHolGetEdit,
    stubHolGetMountWorld,
    stubHolGetNodeWorld,
    stubHolSetScope,
    stubHolSave,
    stubHolRevert,
    stubHolResetScope,
    stubHolEndEdit
};

// ---- GuiKit mouse mock ----
int s_curX = 200, s_curY = 100, s_curW = 800, s_curH = 600;
bool s_curOk = true;
bool fakeCursorClient(int* cx, int* cy, int* cw, int* ch) {
    if (!s_curOk) return false;
    *cx = s_curX; *cy = s_curY; *cw = s_curW; *ch = s_curH;
    return true;
}

// ---- Party mock hooks ----
int s_mCalls = 0, s_mLastIdx = -1;
uintptr_t fakeMember(void*, int idx) {
    ++s_mCalls;
    s_mLastIdx = idx;
    return 0x3000u + static_cast<uintptr_t>(idx) * 0x10u;
}

int s_sCalls = 0;
uintptr_t fakeServerCreature(void* mem) {
    ++s_sCalls;
    return reinterpret_cast<uintptr_t>(mem) + 0x400u;
}

}  // namespace

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    constexpr size_t kArenaSize = 0x10000;
    g_arena = static_cast<uint8_t*>(mmap(nullptr, kArenaSize, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", g_arena != nullptr && g_arena != MAP_FAILED);
    std::memset(g_arena, 0, kArenaSize);
    g_stubVirtualQueryAll = true;
    k2addr::setForTest(&kSteam);
    saferead::beginScope();

    // ============================================================
    // 0. Constants, struct layouts and GuiKit hooks the editor reads directly
    // ============================================================
    {
        HCHECK("setForTest(&kSteam) is in effect", k2addr::get() == &kSteam);
        HEQ("k2addr::off(Creature_Stats) is 0x1198", k2addr::off(&K2AddrTable::Creature_Stats), 0x1198);
        HCHECK("gameaddr::AppGlobal non-zero", gameaddr::AppGlobal != 0);
        HCHECK("gameaddr::GetLocString non-zero", gameaddr::GetLocString != 0);
        HCHECK("gameaddr::GetClientLanguage non-zero", gameaddr::GetClientLanguage != 0);
        HCHECK("gameaddr::ExoDtor non-zero", gameaddr::ExoDtor != 0);
        HCHECK("gameaddr::ClientItemGetName non-zero", gameaddr::ClientItemGetName != 0);
        HCHECK("gameaddr::ClientGetItemById non-zero", gameaddr::ClientGetItemById != 0);

        HEQ("MUI_PANE_COLUMN is 0", MUI_PANE_COLUMN, 0);
        HEQ("K2PREVIEW3D_HAND_RIGHT is 0", K2PREVIEW3D_HAND_RIGHT, 0);
        HEQ("K2PREVIEW3D_HAND_LEFT is 1", K2PREVIEW3D_HAND_LEFT, 1);
        HEQ("K2PREVIEW3D_PART_BODY is 0", K2PREVIEW3D_PART_BODY, 0);
        HEQ("K2PREVIEW3D_PART_HEAD is 1", K2PREVIEW3D_PART_HEAD, 1);
        HEQ("K2Preview3dApi.PlayAnim is vtable slot 7", offsetof(K2Preview3dApi, PlayAnim), 7 * sizeof(void*));
        HEQ("K2HOLSTER_SCOPE_WEAPON is 0", K2HOLSTER_SCOPE_WEAPON, 0);
        HEQ("K2HOLSTER_SCOPE_TYPE is 1", K2HOLSTER_SCOPE_TYPE, 1);
        HEQ("K2HOLSTER_SCOPE_BODY is 2", K2HOLSTER_SCOPE_BODY, 2);
        HEQ("sizeof(K2HolsterEditInfo)", sizeof(K2HolsterEditInfo), static_cast<size_t>(108));
        HEQ("K2HolsterEditInfo.scope offset", offsetof(K2HolsterEditInfo, scope), static_cast<size_t>(64));
        HEQ("K2HolsterEditInfo.rot offset", offsetof(K2HolsterEditInfo, rot), static_cast<size_t>(80));
        HEQ("K2HolsterEditInfo.off offset", offsetof(K2HolsterEditInfo, off), static_cast<size_t>(92));
        HEQ("K2HolsterEditInfo.src offset", offsetof(K2HolsterEditInfo, src), static_cast<size_t>(104));
        // U2: the type string feeds the scope button text ("For: all rifles"); the longest category name ("lightsaber") must fit.
        HEQ("K2HolsterEditInfo.type offset", offsetof(K2HolsterEditInfo, type), static_cast<size_t>(21));
        HEQ("K2HolsterEditInfo.type is char[12]", sizeof(static_cast<K2HolsterEditInfo*>(nullptr)->type), static_cast<size_t>(12));
        // U2: the weapon gestures call NudgeWorld (Move) and NudgeLocal (Depth, Tilt, Roll/Turn, RMB+wheel); both return void.
        HEQ("K2HolsterApi.NudgeWorld is vtable slot 4", offsetof(K2HolsterApi, NudgeWorld), 4 * sizeof(void*));
        HEQ("K2HolsterApi.NudgeLocal is vtable slot 5", offsetof(K2HolsterApi, NudgeLocal), 5 * sizeof(void*));
        // U3: the inspector rows send ABSOLUTE rot[3] + off[3] through SetValues (void) and read the six values back with GetEdit.
        HEQ("K2HolsterApi.SetValues is vtable slot 6", offsetof(K2HolsterApi, SetValues), 6 * sizeof(void*));
        HEQ("K2HolsterApi.GetEdit is vtable slot 8", offsetof(K2HolsterApi, GetEdit), 8 * sizeof(void*));
        HEQ("K2HolsterEditInfo.left offset", offsetof(K2HolsterEditInfo, left), static_cast<size_t>(68));
        HEQ("K2HolsterEditInfo.dirty offset", offsetof(K2HolsterEditInfo, dirty), static_cast<size_t>(72));
        HEQ("K2HolsterEditInfo.rot is float[3]", sizeof(static_cast<K2HolsterEditInfo*>(nullptr)->rot), 3 * sizeof(float));
        HEQ("K2HolsterEditInfo.off is float[3]", sizeof(static_cast<K2HolsterEditInfo*>(nullptr)->off), 3 * sizeof(float));
        HEQ("K2HolsterEditInfo.src is uint8_t[3]", sizeof(static_cast<K2HolsterEditInfo*>(nullptr)->src), static_cast<size_t>(3));
        // U3: the left-mirror note reads bit 8 of src[0] / src[1]; the levels (0..2) and the body bit (4) must stay clear of it.
        HEQ("K2HOLSTER_SRC_GLOBAL is 0", K2HOLSTER_SRC_GLOBAL, 0);
        HEQ("K2HOLSTER_SRC_TYPE is 1", K2HOLSTER_SRC_TYPE, 1);
        HEQ("K2HOLSTER_SRC_WEAPON is 2", K2HOLSTER_SRC_WEAPON, 2);
        HEQ("K2HOLSTER_SRC_BODY is 4", K2HOLSTER_SRC_BODY, 4);
        HCHECK("hse::kSrcMirrorBit does not overlap the src levels or the body bit", (hse::kSrcMirrorBit & (K2HOLSTER_SRC_BODY | 3)) == 0);
        // U3: the page needs base + 12 inspector items inside mod-config's per-page cap (kMaxItems 40 in 0.1.1+); the editor degrades, never crashes, on a full page.
        HCHECK("editor page items fit mod-config's 40-item cap", hse::kPageItemsBase + hse::kInspItems <= hse::kPageItemCap);

        // GuiKit input hooks: null under GUIKIT_HOST (the mod must check before calling); the assignment form the mod uses is callable.
        HCHECK("g_gk.lbuttonDown is null under GUIKIT_HOST", guikit::g_gk.lbuttonDown == nullptr);
        HCHECK("g_gk.rbuttonDown is null under GUIKIT_HOST", guikit::g_gk.rbuttonDown == nullptr);
        HCHECK("g_gk.shiftDown is null under GUIKIT_HOST", guikit::g_gk.shiftDown == nullptr);
        HCHECK("g_gk.ctrlDown is null under GUIKIT_HOST", guikit::g_gk.ctrlDown == nullptr);
        HCHECK("g_gk.mbuttonDown is null under GUIKIT_HOST", guikit::g_gk.mbuttonDown == nullptr);
        guikit::g_gk.mbuttonDown = [] { return true; };
        HCHECK("g_gk.mbuttonDown assignment is callable", guikit::g_gk.mbuttonDown && guikit::g_gk.mbuttonDown());
        guikit::g_gk.mbuttonDown = nullptr;

        // Wheel: take() drains to 0; setRect/anyRect/clearRect/fresh behave as cbEnter/cbLeave expect.
        HEQ("g_wheel.take() is 0 when nothing was counted", guikit::g_wheel.take(), 0);
        HCHECK("g_wheel.anyRect() false before setRect", !guikit::g_wheel.anyRect());
        guikit::g_wheel.setRect(0, 10, 20, 30, 40);
        HCHECK("g_wheel.anyRect() true after setRect", guikit::g_wheel.anyRect());
        guikit::g_wheel.touch(1000);
        HCHECK("g_wheel.fresh() true right after touch", guikit::g_wheel.fresh(1000));
        HCHECK("g_wheel.fresh() false once freshMs has passed", !guikit::g_wheel.fresh(1000 + guikit::g_wheel.freshMs));
        guikit::g_wheel.clearRect(0);
        HCHECK("g_wheel.anyRect() false after clearRect", !guikit::g_wheel.anyRect());
    }

    // ============================================================
    // 1. ModConfigApi (k2.modconfig + k2.menuui)
    // ============================================================
    {
        HCHECK("K2MODCONFIG_IFACE is \"k2.modconfig\"", std::strcmp(K2MODCONFIG_IFACE, "k2.modconfig") == 0);
        HEQ("K2MODCONFIG_VERSION == 1", K2MODCONFIG_VERSION, 1);
        HCHECK("K2MENUUI_IFACE is \"k2.menuui\"", std::strcmp(K2MENUUI_IFACE, "k2.menuui") == 0);
        HEQ("K2MENUUI_VERSION == 1", K2MENUUI_VERSION, 1);
        HEQ("MUI_NOFREE == 1", MUI_NOFREE, 1);
        HEQ("MUI_PANE_STATUS == 3", MUI_PANE_STATUS, 3);
        HEQ("MUI_PANE_VIEW == 5", MUI_PANE_VIEW, 5);

        HEQ("K2ModConfigEntry is 10 pointer slots", sizeof(K2ModConfigEntry), 10 * sizeof(void*));
        HEQ("K2ModConfigApi is 3 pointer slots", sizeof(K2ModConfigApi), 3 * sizeof(void*));
        HEQ("K2MenuUiApi is 11 pointer slots", sizeof(K2MenuUiApi), 11 * sizeof(void*));

        // ConfigClient::usable
        HCHECK("ConfigClient::usable true for valid table", k2mc::ConfigClient::usable(&kCfgTable));
        HCHECK("ConfigClient::usable false for null table", !k2mc::ConfigClient::usable(nullptr));
        K2ModConfigApi shortCfg = kCfgTable;
        shortCfg.struct_size = static_cast<uint32_t>(sizeof(K2ModConfigApi) - sizeof(void*));
        HCHECK("ConfigClient::usable false for short table", !k2mc::ConfigClient::usable(&shortCfg));
        K2ModConfigApi grownCfg = kCfgTable;
        grownCfg.struct_size = static_cast<uint32_t>(sizeof(K2ModConfigApi) + sizeof(void*));
        HCHECK("ConfigClient::usable true for grown table", k2mc::ConfigClient::usable(&grownCfg));

        K2ModConfigApi nullReg = kCfgTable; nullReg.Register = nullptr;
        HCHECK("ConfigClient::usable false when Register is null", !k2mc::ConfigClient::usable(&nullReg));
        K2ModConfigApi nullUnreg = kCfgTable; nullUnreg.Unregister = nullptr;
        HCHECK("ConfigClient::usable false when Unregister is null", !k2mc::ConfigClient::usable(&nullUnreg));

        // MenuClient::usable
        HCHECK("MenuClient::usable true for valid table", k2mc::MenuClient::usable(&kMuiTable));
        HCHECK("MenuClient::usable false for null table", !k2mc::MenuClient::usable(nullptr));
        K2MenuUiApi shortMui = kMuiTable;
        shortMui.struct_size = static_cast<uint32_t>(sizeof(K2MenuUiApi) - sizeof(void*));
        HCHECK("MenuClient::usable false for short table", !k2mc::MenuClient::usable(&shortMui));
        K2MenuUiApi grownMui = kMuiTable;
        grownMui.struct_size = static_cast<uint32_t>(sizeof(K2MenuUiApi) + sizeof(void*));
        HCHECK("MenuClient::usable true for grown table", k2mc::MenuClient::usable(&grownMui));

        K2MenuUiApi nullNewPage = kMuiTable; nullNewPage.NewPage = nullptr;
        HCHECK("MenuClient::usable false when NewPage is null", !k2mc::MenuClient::usable(&nullNewPage));
        K2MenuUiApi nullControl = kMuiTable; nullControl.Control = nullptr;
        HCHECK("MenuClient::usable false when Control is null", !k2mc::MenuClient::usable(&nullControl));

        // Registry & client connect/poll
        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfgTable);
        fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable);
        k2mc::ConfigClient cfgClient;
        cfgClient.init(fakekp::api());
        cfgClient.poll();
        HCHECK("ConfigClient ok() after poll() with k2.modconfig v1 provided", cfgClient.ok());
        k2mc::MenuClient menuClient;
        menuClient.init(fakekp::api());
        menuClient.poll();
        HCHECK("MenuClient ok() after poll() with k2.menuui v1 provided", menuClient.ok());
    }

    // ============================================================
    // 2. Preview3dApi (k2.preview3d: k2p3::Client::usable)
    // ============================================================
    {
        HCHECK("K2PREVIEW3D_IFACE is \"k2.preview3d\"", std::strcmp(K2PREVIEW3D_IFACE, "k2.preview3d") == 0);
        HEQ("K2PREVIEW3D_VERSION == 1", K2PREVIEW3D_VERSION, 1);
        HEQ("K2Preview3dApi is 12 pointer slots", sizeof(K2Preview3dApi), 12 * sizeof(void*));

        HCHECK("k2p3::Client::usable true for valid table", k2p3::Client::usable(&kP3Table));
        HCHECK("k2p3::Client::usable false for null table", !k2p3::Client::usable(nullptr));

        K2Preview3dApi shortP3 = kP3Table;
        shortP3.struct_size = static_cast<uint32_t>(sizeof(K2Preview3dApi) - sizeof(void*));
        HCHECK("k2p3::Client::usable false for short table", !k2p3::Client::usable(&shortP3));

        K2Preview3dApi grownP3 = kP3Table;
        grownP3.struct_size = static_cast<uint32_t>(sizeof(K2Preview3dApi) + 2 * sizeof(void*));
        HCHECK("k2p3::Client::usable true for grown table", k2p3::Client::usable(&grownP3));

        K2Preview3dApi nullCreate = kP3Table; nullCreate.CreateView = nullptr;
        HCHECK("k2p3::Client::usable false when CreateView is null", !k2p3::Client::usable(&nullCreate));
        K2Preview3dApi nullBind = kP3Table; nullBind.Bind = nullptr;
        HCHECK("k2p3::Client::usable false when Bind is null", !k2p3::Client::usable(&nullBind));
        K2Preview3dApi nullRebuild = kP3Table; nullRebuild.Rebuild = nullptr;
        HCHECK("k2p3::Client::usable false when Rebuild is null", !k2p3::Client::usable(&nullRebuild));
        K2Preview3dApi nullSetWep = kP3Table; nullSetWep.SetWeapons = nullptr;
        HCHECK("k2p3::Client::usable false when SetWeapons is null", !k2p3::Client::usable(&nullSetWep));
        K2Preview3dApi nullWepGob = kP3Table; nullWepGob.WeaponGob = nullptr;
        HCHECK("k2p3::Client::usable false when WeaponGob is null", !k2p3::Client::usable(&nullWepGob));
        K2Preview3dApi nullSetView = kP3Table; nullSetView.SetView = nullptr;
        HCHECK("k2p3::Client::usable false when SetView is null", !k2p3::Client::usable(&nullSetView));
        K2Preview3dApi nullPlayAnim = kP3Table; nullPlayAnim.PlayAnim = nullptr;
        HCHECK("k2p3::Client::usable false when PlayAnim is null", !k2p3::Client::usable(&nullPlayAnim));
        K2Preview3dApi nullRigGob = kP3Table; nullRigGob.RigGob = nullptr;
        HCHECK("k2p3::Client::usable false when RigGob is null", !k2p3::Client::usable(&nullRigGob));
        K2Preview3dApi nullDestroy = kP3Table; nullDestroy.Destroy = nullptr;
        HCHECK("k2p3::Client::usable false when Destroy is null", !k2p3::Client::usable(&nullDestroy));
        K2Preview3dApi nullBeforeFree = kP3Table; nullBeforeFree.SetBeforeFree = nullptr;
        HCHECK("k2p3::Client::usable false when SetBeforeFree is null", !k2p3::Client::usable(&nullBeforeFree));
        K2Preview3dApi nullViewCtl = kP3Table; nullViewCtl.ViewControl = nullptr;
        HCHECK("k2p3::Client::usable false when ViewControl is null", !k2p3::Client::usable(&nullViewCtl));

        // k2p3::Client connect / poll / ok()
        fakekp::reset();
        k2p3::Client p3Client;
        p3Client.init(fakekp::api());
        p3Client.poll();
        HCHECK("k2p3::Client not ok() when not provided", !p3Client.ok());
        fakekp::provide(K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION, &kP3Table);
        HCHECK("k2p3::Client connects when provided", p3Client.connect());
        HCHECK("k2p3::Client ok() after connect", p3Client.ok());
        HCHECK("k2p3::Client holds table pointer", p3Client.p == &kP3Table);
    }

    // ============================================================
    // 3. HolsterApi (k2.holster: k2holster::Client::usable)
    // ============================================================
    {
        HCHECK("K2HOLSTER_IFACE is \"k2.holster\"", std::strcmp(K2HOLSTER_IFACE, "k2.holster") == 0);
        HEQ("K2HOLSTER_VERSION == 1", K2HOLSTER_VERSION, 1);
        HEQ("K2HolsterApi is 16 pointer slots", sizeof(K2HolsterApi), 16 * sizeof(void*));

        HCHECK("k2holster::Client::usable true for valid table", k2holster::Client::usable(&kHolTable));
        HCHECK("k2holster::Client::usable false for null table", !k2holster::Client::usable(nullptr));

        K2HolsterApi shortHol = kHolTable;
        shortHol.struct_size = static_cast<uint32_t>(sizeof(K2HolsterApi) - sizeof(void*));
        HCHECK("k2holster::Client::usable false for short table", !k2holster::Client::usable(&shortHol));

        K2HolsterApi grownHol = kHolTable;
        grownHol.struct_size = static_cast<uint32_t>(sizeof(K2HolsterApi) + sizeof(void*));
        HCHECK("k2holster::Client::usable true for grown table", k2holster::Client::usable(&grownHol));

        K2HolsterApi nullReg = kHolTable; nullReg.RegisterPreview = nullptr;
        HCHECK("k2holster::Client::usable false when RegisterPreview is null", !k2holster::Client::usable(&nullReg));
        K2HolsterApi nullUnreg = kHolTable; nullUnreg.UnregisterPreview = nullptr;
        HCHECK("k2holster::Client::usable false when UnregisterPreview is null", !k2holster::Client::usable(&nullUnreg));
        K2HolsterApi nullBegin = kHolTable; nullBegin.BeginEdit = nullptr;
        HCHECK("k2holster::Client::usable false when BeginEdit is null", !k2holster::Client::usable(&nullBegin));
        K2HolsterApi nullNW = kHolTable; nullNW.NudgeWorld = nullptr;
        HCHECK("k2holster::Client::usable false when NudgeWorld is null", !k2holster::Client::usable(&nullNW));
        K2HolsterApi nullNL = kHolTable; nullNL.NudgeLocal = nullptr;
        HCHECK("k2holster::Client::usable false when NudgeLocal is null", !k2holster::Client::usable(&nullNL));
        K2HolsterApi nullSetVal = kHolTable; nullSetVal.SetValues = nullptr;
        HCHECK("k2holster::Client::usable false when SetValues is null", !k2holster::Client::usable(&nullSetVal));
        K2HolsterApi nullBone = kHolTable; nullBone.CycleBone = nullptr;
        HCHECK("k2holster::Client::usable false when CycleBone is null", !k2holster::Client::usable(&nullBone));
        K2HolsterApi nullGetEdit = kHolTable; nullGetEdit.GetEdit = nullptr;
        HCHECK("k2holster::Client::usable false when GetEdit is null", !k2holster::Client::usable(&nullGetEdit));
        K2HolsterApi nullMount = kHolTable; nullMount.GetMountWorld = nullptr;
        HCHECK("k2holster::Client::usable false when GetMountWorld is null", !k2holster::Client::usable(&nullMount));
        K2HolsterApi nullNode = kHolTable; nullNode.GetNodeWorld = nullptr;
        HCHECK("k2holster::Client::usable false when GetNodeWorld is null", !k2holster::Client::usable(&nullNode));
        K2HolsterApi nullScope = kHolTable; nullScope.SetScope = nullptr;
        HCHECK("k2holster::Client::usable false when SetScope is null", !k2holster::Client::usable(&nullScope));
        K2HolsterApi nullSave = kHolTable; nullSave.Save = nullptr;
        HCHECK("k2holster::Client::usable false when Save is null", !k2holster::Client::usable(&nullSave));
        K2HolsterApi nullRevert = kHolTable; nullRevert.Revert = nullptr;
        HCHECK("k2holster::Client::usable false when Revert is null", !k2holster::Client::usable(&nullRevert));
        K2HolsterApi nullReset = kHolTable; nullReset.ResetScope = nullptr;
        HCHECK("k2holster::Client::usable false when ResetScope is null", !k2holster::Client::usable(&nullReset));
        K2HolsterApi nullEnd = kHolTable; nullEnd.EndEdit = nullptr;
        HCHECK("k2holster::Client::usable false when EndEdit is null", !k2holster::Client::usable(&nullEnd));

        // k2holster::Client connect / poll / ok()
        fakekp::reset();
        k2holster::Client holClient;
        holClient.init(fakekp::api());
        holClient.poll();
        HCHECK("k2holster::Client not ok() when not provided", !holClient.ok());
        fakekp::provide(K2HOLSTER_IFACE, K2HOLSTER_VERSION, &kHolTable);
        HCHECK("k2holster::Client connects when provided", holClient.connect());
        HCHECK("k2holster::Client ok() after connect", holClient.ok());
        HCHECK("k2holster::Client holds table pointer", holClient.h == &kHolTable);
    }

    // ============================================================
    // 4. GuiKit (guikit::hit / cursorEngine)
    // ============================================================
    {
        HCHECK("guikit::hit signature check", static_cast<bool(*)(uintptr_t, int, int)>(&guikit::hit) != nullptr);
        HCHECK("guikit::cursorEngine signature check", static_cast<bool(*)(int*, int*)>(&guikit::cursorEngine) != nullptr);

        // cursorEngine parameter gates
        int ox = 0, oy = 0;
        HCHECK("cursorEngine rejects null x", !guikit::cursorEngine(nullptr, &oy));
        HCHECK("cursorEngine rejects null y", !guikit::cursorEngine(&ox, nullptr));

        // Without cursorClient hook, cursorEngine returns false
        guikit::g_gk.cursorClient = nullptr;
        HCHECK("cursorEngine false when cursorClient is null", !guikit::cursorEngine(&ox, &oy));

        // With cursorClient hook
        s_curOk = true; s_curX = 120; s_curY = 80; s_curW = 800; s_curH = 600;
        guikit::g_gk.cursorClient = fakeCursorClient;
        guikit::g_gk.screenWAddr = 0; guikit::g_gk.screenHAddr = 0;
        HCHECK("cursorEngine succeeds with valid client rect", guikit::cursorEngine(&ox, &oy));
        HEQ("cursorEngine unscaled x matches client x", ox, 120);
        HEQ("cursorEngine unscaled y matches client y", oy, 80);

        s_curW = 0;
        HCHECK("cursorEngine false when client width < 1", !guikit::cursorEngine(&ox, &oy));
        s_curW = 800; s_curOk = false;
        HCHECK("cursorEngine false when cursorClient reports failure", !guikit::cursorEngine(&ox, &oy));
        s_curOk = true;

        // guikit::hit
        HCHECK("hit(0, x, y) returns false for null control", !guikit::hit(0, 50, 50));

        uintptr_t ctl = arenaAlloc(0x100);
        at<uint32_t>(ctl, 0x48) = 0x2;  // FlagVisible
        at<int>(ctl, 4) = 50;           // Ctl_X
        at<int>(ctl, 8) = 60;           // Ctl_Y
        at<int>(ctl, 0xC) = 100;        // Ctl_W
        at<int>(ctl, 0x10) = 40;        // Ctl_H

        HCHECK("hit returns true on top-left corner", guikit::hit(ctl, 50, 60));
        HCHECK("hit returns true inside control rect", guikit::hit(ctl, 149, 99));
        HCHECK("hit returns false at right edge (boundary)", !guikit::hit(ctl, 150, 60));
        HCHECK("hit returns false at bottom edge (boundary)", !guikit::hit(ctl, 50, 100));
        HCHECK("hit returns false outside left", !guikit::hit(ctl, 49, 60));
        HCHECK("hit returns false outside top", !guikit::hit(ctl, 50, 59));

        at<uint32_t>(ctl, 0x48) = 0;    // Hide control
        HCHECK("hit returns false when control is not visible", !guikit::hit(ctl, 70, 70));
    }

    // ============================================================
    // 5. Party (party::table / partyCount / member / serverCreature)
    // ============================================================
    {
        HCHECK("party::table signature check", static_cast<uintptr_t(*)(party::Fail*)>(&party::table) != nullptr);
        HCHECK("party::partyCount signature check", static_cast<int(*)(uintptr_t)>(&party::partyCount) != nullptr);
        HCHECK("party::member signature check", static_cast<uintptr_t(*)(uintptr_t, int)>(&party::member) != nullptr);
        HCHECK("party::serverCreature signature check", static_cast<uintptr_t(*)(uintptr_t)>(&party::serverCreature) != nullptr);

        // Broken chain: empty app cell
        uintptr_t emptyCell = arenaAlloc(16);
        at<uint32_t>(emptyCell, 0) = 0;
        party::g_appGlobal = emptyCell;
        party::Fail why = party::F_None;
        HEQ("table() returns 0 when app global cell is empty", party::table(&why), 0u);
        HEQ("table() failure reason is F_App", why, party::F_App);

        // Intact chain: cell -> app -> +4 -> +4 -> +0x270 -> table
        uintptr_t cell = arenaAlloc(16);
        uintptr_t app = arenaAlloc(16);
        uintptr_t pa = arenaAlloc(16);
        uintptr_t pb = arenaAlloc(0x300);
        uintptr_t tbl = arenaAlloc(16);
        at<uint32_t>(cell, 0) = static_cast<uint32_t>(app);
        at<uint32_t>(app, 4) = static_cast<uint32_t>(pa);
        at<uint32_t>(pa, 4) = static_cast<uint32_t>(pb);
        at<uint32_t>(pb, 0x270) = static_cast<uint32_t>(tbl);
        at<int>(tbl, 0) = 3;  // party count

        party::g_appGlobal = cell;
        why = party::F_App;
        HEQ("table() resolves party table via intact pointer chain", party::table(&why), tbl);
        HEQ("table() success reason is F_None", why, party::F_None);

        // partyCount
        HEQ("partyCount(0) is 0", party::partyCount(0), 0);
        HEQ("partyCount(tbl) reads count from table offset 0", party::partyCount(tbl), 3);

        // member
        party::g_member = fakeMember;
        s_mCalls = 0; s_mLastIdx = -1;
        HEQ("member(0, 0) returns 0 without calling engine", party::member(0, 0), 0u);
        HEQ("member(0, 0) made 0 engine calls", s_mCalls, 0);
        HEQ("member(tbl, -1) returns 0 (negative index)", party::member(tbl, -1), 0u);
        HEQ("member(tbl, 3) returns 0 (out of bounds index)", party::member(tbl, 3), 0u);
        HEQ("out-of-bounds member calls made 0 engine calls", s_mCalls, 0);

        HEQ("member(tbl, 1) returns client creature pointer", party::member(tbl, 1), 0x3010u);
        HEQ("member(tbl, 1) called engine accessor once", s_mCalls, 1);
        HEQ("member(tbl, 1) passed index 1", s_mLastIdx, 1);

        // serverCreature
        party::g_serverCreature = fakeServerCreature;
        s_sCalls = 0;
        HEQ("serverCreature(0) returns 0 without calling engine", party::serverCreature(0), 0u);
        HEQ("serverCreature(0) made 0 engine calls", s_sCalls, 0);

        HEQ("serverCreature resolves server pointer", party::serverCreature(0x3010u), 0x3410u);
        HEQ("serverCreature called engine accessor once", s_sCalls, 1);
    }

    // ============================================================
    // 6. SafeRead (saferead::readExo)
    // ============================================================
    {
        HCHECK("saferead::readExo signature check",
               static_cast<int(*)(unsigned, uintptr_t, char*, int, int, bool)>(&saferead::readExo) != nullptr);

        char out[96];
        auto mkExo = [&](const void* bytes, int n, int len) -> uintptr_t {
            uintptr_t data = arenaAlloc(n > 0 ? n : 1);
            if (n > 0) std::memcpy(reinterpret_cast<void*>(data), bytes, n);
            uintptr_t st = arenaAlloc(8);
            at<uint32_t>(st, 0) = static_cast<uint32_t>(data);
            at<int>(st, 4) = len;
            return st;
        };
        auto fill = [&] { std::memset(out, 'X', sizeof(out)); };

        // Trailing NUL stripped
        fill();
        uintptr_t e = mkExo("Blaster Pistol\0", 15, 15);
        HEQ("readExo strips engine trailing NUL and returns length", saferead::readExo(1u, e, out, sizeof(out)), 14);
        HCHECK("readExo output matches string", std::strcmp(out, "Blaster Pistol") == 0);

        // Without trailing NUL
        fill();
        e = mkExo("Lightsaber", 10, 10);
        HEQ("readExo accepts string without trailing NUL", saferead::readExo(1u, e, out, sizeof(out)), 10);
        HCHECK("readExo output matches string", std::strcmp(out, "Lightsaber") == 0);

        // MaxLen (default 80)
        char eighty[81];
        std::memset(eighty, 'A', 80);
        eighty[80] = 0;
        fill();
        e = mkExo(eighty, 80, 80);
        HEQ("readExo accepts 80-char string", saferead::readExo(1u, e, out, sizeof(out)), 80);
        HEQ("readExo 80-char output length matches", static_cast<int>(std::strlen(out)), 80);

        char ninety[91];
        std::memset(ninety, 'B', 90);
        ninety[90] = 0;
        fill();
        e = mkExo(ninety, 90, 90);
        HEQ("readExo rejects string exceeding maxLen 80", saferead::readExo(1u, e, out, sizeof(out)), 0);
        HEQ("readExo rejection clears buffer", out[0], 0);

        // Cap overflow
        fill();
        e = mkExo(eighty, 80, 80);
        HEQ("readExo rejects string when len >= cap", saferead::readExo(1u, e, out, 80), 0);
        HEQ("readExo cap rejection clears buffer", out[0], 0);

        // Empty string
        fill();
        e = mkExo("", 0, 0);
        HEQ("readExo returns 0 on empty string", saferead::readExo(1u, e, out, sizeof(out)), 0);
        HEQ("readExo empty string clears buffer", out[0], 0);

        // All-NUL string
        fill();
        e = mkExo("\0\0\0", 3, 3);
        HEQ("readExo returns 0 on all-NUL string", saferead::readExo(1u, e, out, sizeof(out)), 0);
        HEQ("readExo all-NUL string clears buffer", out[0], 0);

        // Null string pointer inside CExoString
        fill();
        uintptr_t nullPtrExo = arenaAlloc(8);
        at<uint32_t>(nullPtrExo, 0) = 0;
        at<int>(nullPtrExo, 4) = 5;
        HEQ("readExo returns 0 on null string pointer", saferead::readExo(1u, nullPtrExo, out, sizeof(out)), 0);
        HEQ("readExo null string pointer clears buffer", out[0], 0);

        // Low / unreadable struct pointer
        fill();
        HEQ("readExo returns 0 on unreadable address", saferead::readExo(1u, 0x20u, out, sizeof(out)), 0);
        HEQ("readExo unreadable address clears buffer", out[0], 0);

        // Non-printable character rejection
        fill();
        e = mkExo("Item\x01Name", 9, 9);
        HEQ("readExo rejects non-printable character with printableOnly=true", saferead::readExo(1u, e, out, sizeof(out)), 0);
        HEQ("readExo non-printable rejection clears buffer", out[0], 0);

        // Wide limits (maxLen 255, cap 256, printableOnly=false): a 255-byte string including 0x01 is accepted, 256 is not.
        {
            char big[256];
            char wide[257];
            std::memset(wide, 'C', 256);
            wide[0] = '\x01';
            wide[256] = 0;
            e = mkExo(wide, 255, 255);
            HEQ("readExo(wide) accepts 255 bytes including 0x01", saferead::readExo(1u, e, big, 256, 255, false), 255);
            HEQ("readExo(wide) keeps the 0x01 byte", static_cast<int>(big[0]), 1);
            e = mkExo(wide, 256, 256);
            HEQ("readExo(wide) rejects 256 bytes", saferead::readExo(1u, e, big, 256, 255, false), 0);
            e = mkExo(wide, 255, 255);
            HEQ("readExo rejects 0x01 when printableOnly=true", saferead::readExo(1u, e, big, 256, 255, true), 0);
        }

        // Null buffer or invalid cap
        HEQ("readExo returns 0 on null buffer without crashing", saferead::readExo(1u, e, nullptr, 10), 0);
        HEQ("readExo returns 0 on cap <= 0 without crashing", saferead::readExo(1u, e, out, 0), 0);
    }

    return ::kt::Report("common_contract_test (holster-editor)");
}
