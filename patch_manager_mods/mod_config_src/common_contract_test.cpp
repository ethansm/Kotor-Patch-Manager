// common_contract_test.cpp -- Patches/Common CONTRACT test for mod-config (ModConfigApi.h: k2.modconfig + k2.menuui, v1).
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// This pins the registry ABI (kpatch_api.h) that both consumer helpers rely on, the v1 struct layouts (ModConfigApi.h also
// static_asserts them), and the consumer gates (struct_size, null function pointers, grown tables) against the fake registry.
// It never calls into the engine: the provider's engine code is not in scope for host tests.

#include "check.h"
#include "FakeKPatchApi.h"
#include "ModConfigApi.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

#define MCHECK(why, expr) ::kt::Check("mod-config relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define MEQ(why, a, b) ::kt::CheckEq("mod-config relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

namespace {

// Stub provider entry points. The test hands these out and never calls them, so the parameters stay unnamed (-Werror).
int32_t KPATCH_CALL stubRegister(const K2ModConfigEntry*) { return 1; }
void KPATCH_CALL stubUnregister(int32_t) {}
uint32_t KPATCH_CALL stubNewPage(void*, const char*) { return 0; }
int32_t KPATCH_CALL stubAddButton(uint32_t, const char*, const char*) { return 0; }
int32_t KPATCH_CALL stubAddLabel(uint32_t, const char*) { return 0; }
int32_t KPATCH_CALL stubAddCustom(uint32_t, void*, int32_t) { return 0; }
int32_t KPATCH_CALL stubClicked(uint32_t, int32_t) { return 0; }
void KPATCH_CALL stubSetText(uint32_t, int32_t, const char*) {}
void KPATCH_CALL stubSetRect(uint32_t, int32_t, const K2MuiRect*) {}
void KPATCH_CALL stubSetDesc(uint32_t, const char*) {}
int32_t KPATCH_CALL stubPaneRect(void*, int32_t, K2MuiRect*) { return 0; }
void* KPATCH_CALL stubControl(uint32_t, int32_t) { return nullptr; }
int32_t KPATCH_CALL stubSetDescSplit(uint32_t, int32_t) { return 0; }
int32_t KPATCH_CALL stubAddRow(uint32_t, const K2MuiRowSpec*) { return 0; }
void KPATCH_CALL stubSetChangeHandler(uint32_t, K2MuiChangeFn, void*) {}
int32_t KPATCH_CALL stubGetValue(uint32_t, int32_t) { return 0; }
void KPATCH_CALL stubSetValue(uint32_t, int32_t, int32_t) {}
void KPATCH_CALL stubSetRowText(uint32_t, int32_t, const char*) {}
void KPATCH_CALL stubSetRowState(uint32_t, int32_t, int32_t) {}
void KPATCH_CALL stubSetRowRange(uint32_t, int32_t, int32_t, int32_t) {}

void KPATCH_CALL stubSetItemShown(uint32_t, int32_t, int32_t) {}
uint32_t KPATCH_CALL stubNewModal(uint32_t, const K2MuiRect*, const char*) { return 0; }
void KPATCH_CALL stubSetModalOpen(uint32_t, int32_t) {}
int32_t KPATCH_CALL stubAddGear(uint32_t, const char*) { return -1; }
const K2ModConfigApi kCfgTable = { sizeof(K2ModConfigApi), stubRegister, stubUnregister };
const K2MenuUiApi kMuiTable = { sizeof(K2MenuUiApi), stubNewPage, stubAddButton, stubAddLabel, stubAddCustom, stubClicked,
                                stubSetText, stubSetRect, stubSetDesc, stubPaneRect, stubControl, stubSetDescSplit,
                                stubAddRow, stubSetChangeHandler, stubGetValue, stubSetValue, stubSetRowText, stubSetRowState, stubSetRowRange,
                                nullptr, nullptr, nullptr, nullptr };   // slots 19..22 (modal / gear) are not exercised here

// onConnect capture for the not-provided message.
bool g_connOk = true;
const char* g_connWhy = "";
void captureConnect(bool good, const char* why) { g_connOk = good; g_connWhy = why; }

}  // namespace

int main() {
    // 1. Version and name macros the consumers pass to require().
    MCHECK("K2MODCONFIG_IFACE is \"k2.modconfig\"", std::strcmp(K2MODCONFIG_IFACE, "k2.modconfig") == 0);
    MEQ("K2MODCONFIG_VERSION == 1", K2MODCONFIG_VERSION, 1);
    MCHECK("K2MENUUI_IFACE is \"k2.menuui\"", std::strcmp(K2MENUUI_IFACE, "k2.menuui") == 0);
    MEQ("K2MENUUI_VERSION == 1", K2MENUUI_VERSION, 1);
    MEQ("MUI_NOFREE == 1", MUI_NOFREE, 1);
    MEQ("MUI_PANE_COLUMN == 0", MUI_PANE_COLUMN, 0);
    MEQ("MUI_PANE_DESC == 1", MUI_PANE_DESC, 1);
    MEQ("MUI_PANE_TITLE == 2", MUI_PANE_TITLE, 2);
    MEQ("MUI_PANE_STATUS == 3", MUI_PANE_STATUS, 3);
    MEQ("MUI_PANE_BACK == 4", MUI_PANE_BACK, 4);
    MEQ("MUI_PANE_VIEW == 5", MUI_PANE_VIEW, 5);
    MEQ("MUI_PANE_PREVIEW == 6", MUI_PANE_PREVIEW, 6);

    // 2. Struct layout sanity (the header static_asserts these too; the run-time copy fails the test on a host build).
    MEQ("K2ModConfigEntry is 10 pointer slots (struct_size fills one slot)", sizeof(K2ModConfigEntry), 10 * sizeof(void*));
    MEQ("K2ModConfigEntry.destroy at slot 9", offsetof(K2ModConfigEntry, destroy), 9 * sizeof(void*));
    MEQ("K2ModConfigApi is 3 pointer slots", sizeof(K2ModConfigApi), 3 * sizeof(void*));
    MEQ("K2ModConfigApi.Unregister at slot 2", offsetof(K2ModConfigApi, Unregister), 2 * sizeof(void*));
    MEQ("K2MuiRect is 16 bytes", sizeof(K2MuiRect), 16);
    MEQ("K2MuiRect.h at byte 12", offsetof(K2MuiRect, h), 12);
    MEQ("K2MenuUiApi is 23 pointer slots", sizeof(K2MenuUiApi), 23 * sizeof(void*));
    MEQ("K2MenuUiApi.SetDescSplit at slot 11", offsetof(K2MenuUiApi, SetDescSplit), 11 * sizeof(void*));
    MEQ("K2MenuUiApi.AddRow at slot 12", offsetof(K2MenuUiApi, AddRow), 12 * sizeof(void*));
    MEQ("K2MenuUiApi.SetChangeHandler at slot 13", offsetof(K2MenuUiApi, SetChangeHandler), 13 * sizeof(void*));
    MEQ("K2MenuUiApi.GetValue at slot 14", offsetof(K2MenuUiApi, GetValue), 14 * sizeof(void*));
    MEQ("K2MenuUiApi.SetValue at slot 15", offsetof(K2MenuUiApi, SetValue), 15 * sizeof(void*));
    MEQ("K2MenuUiApi.SetRowText at slot 16", offsetof(K2MenuUiApi, SetRowText), 16 * sizeof(void*));
    MEQ("K2MenuUiApi.SetRowState at slot 17", offsetof(K2MenuUiApi, SetRowState), 17 * sizeof(void*));
    MEQ("K2MenuUiApi.SetRowRange at slot 18", offsetof(K2MenuUiApi, SetRowRange), 18 * sizeof(void*));
    MEQ("K2MenuUiApi.SetItemShown at slot 19", offsetof(K2MenuUiApi, SetItemShown), 19 * sizeof(void*));
    MEQ("K2MenuUiApi.NewModal at slot 20", offsetof(K2MenuUiApi, NewModal), 20 * sizeof(void*));
    MEQ("K2MenuUiApi.SetModalOpen at slot 21", offsetof(K2MenuUiApi, SetModalOpen), 21 * sizeof(void*));
    MEQ("K2MenuUiApi.AddGear at slot 22 (the last field)", offsetof(K2MenuUiApi, AddGear), 22 * sizeof(void*));
    MEQ("MUI_ROW_TOGGLE == 0 / MUI_ROW_STEPPER == 1", MUI_ROW_TOGGLE * 10 + MUI_ROW_STEPPER, 1);
    MEQ("MUI_RF_WRAP/RESTART/DISABLED/HIDDEN == 1/2/4/8", MUI_RF_WRAP | MUI_RF_RESTART | MUI_RF_DISABLED | MUI_RF_HIDDEN, 15);
    MEQ("MUI_RF_WRAP == 1", MUI_RF_WRAP, 1);
    MEQ("MUI_RF_RESTART == 2", MUI_RF_RESTART, 2);
    MEQ("MUI_RF_DISABLED == 4", MUI_RF_DISABLED, 4);
    MEQ("MUI_RF_HIDDEN == 8", MUI_RF_HIDDEN, 8);
    MEQ("K2MuiRowSpec.kind at byte 4", offsetof(K2MuiRowSpec, kind), 4);
    MEQ("K2MuiRowSpec.label after kind", offsetof(K2MuiRowSpec, label), 8);
    MEQ("K2MenuUiApi.PaneRect at slot 9", offsetof(K2MenuUiApi, PaneRect), 9 * sizeof(void*));
    MEQ("K2MenuUiApi.Control at slot 10", offsetof(K2MenuUiApi, Control), 10 * sizeof(void*));

    // 3. Registry round trip: provide both v1 tables, require returns exactly the provided pointers.
    fakekp::reset();
    const KPatchApi* api = fakekp::api();
    MEQ("provide k2.modconfig v1 accepted", api->provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfgTable), 0);
    MEQ("provide k2.menuui v1 accepted", api->provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable), 0);
    MCHECK("require k2.modconfig v1 returns the provided table", api->require(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION) == &kCfgTable);
    MCHECK("require k2.menuui v1 returns the provided table", api->require(K2MENUUI_IFACE, K2MENUUI_VERSION) == &kMuiTable);
    MCHECK("require k2.modconfig with major 2 returns null", api->require(K2MODCONFIG_IFACE, 2) == nullptr);
    MCHECK("duplicate provide(k2.menuui, 1) is rejected", api->provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable) != 0);

    // 4. Consumer round trip, the way a consumer runs it: provider present, init then poll on the first frame.
    fakekp::reset();
    fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfgTable);
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable);
    k2mc::ConfigClient cfg;
    cfg.init(fakekp::api());
    cfg.poll();
    MCHECK("ConfigClient ok() after poll() with k2.modconfig v1 provided", cfg.ok());
    MCHECK("ConfigClient keeps the provided k2.modconfig table", cfg.p == &kCfgTable);
    k2mc::MenuClient mui;
    mui.init(fakekp::api());
    mui.poll();
    MCHECK("MenuClient ok() after poll() with k2.menuui v1 provided", mui.ok());
    MCHECK("MenuClient keeps the provided k2.menuui table", mui.p == &kMuiTable);

    // 5. Not provided: the consumer is not ok and reports the reason once.
    fakekp::reset();
    k2mc::ConfigClient absent;
    absent.init(fakekp::api());
    absent.onConnect = captureConnect;
    absent.poll();
    MCHECK("ConfigClient not ok when k2.modconfig is not provided", !absent.ok());
    MEQ("ConfigClient reports 'not provided'", g_connWhy, "k2.modconfig v1 not provided");
    MCHECK("ConfigClient reports failure to onConnect", !g_connOk);

    k2mc::MenuClient absentMui;
    absentMui.init(fakekp::api());
    absentMui.onConnect = captureConnect;
    absentMui.poll();
    MCHECK("MenuClient not ok when k2.menuui is not provided", !absentMui.ok());
    MEQ("MenuClient reports 'not provided'", g_connWhy, "k2.menuui v1 not provided");

    // 6. struct_size gate: a table older than v1 (one field short) is refused, with the 'too small' reason.
    fakekp::reset();
    K2ModConfigApi shortCfg = kCfgTable;
    shortCfg.struct_size = static_cast<uint32_t>(offsetof(K2ModConfigApi, Unregister));   // predates Unregister
    fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &shortCfg);
    k2mc::ConfigClient shortClient;
    shortClient.init(fakekp::api());
    shortClient.onConnect = captureConnect;
    MCHECK("ConfigClient::usable false for struct_size before Unregister", !k2mc::ConfigClient::usable(&shortCfg));
    MCHECK("ConfigClient connect() refuses a too-small k2.modconfig table", !shortClient.connect() && !shortClient.ok());
    MEQ("ConfigClient reports 'table too small'", g_connWhy, "k2.modconfig v1 table too small");

    fakekp::reset();
    K2MenuUiApi shortMui = kMuiTable;
    shortMui.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, Control));   // predates Control
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &shortMui);
    k2mc::MenuClient shortMuiClient;
    shortMuiClient.init(fakekp::api());
    shortMuiClient.onConnect = captureConnect;
    MCHECK("MenuClient::usable false for struct_size before Control", !k2mc::MenuClient::usable(&shortMui));
    MCHECK("MenuClient connect() refuses a too-small k2.menuui table", !shortMuiClient.connect() && !shortMuiClient.ok());
    MEQ("MenuClient reports 'table too small'", g_connWhy, "k2.menuui v1 table too small");

    // 7. A grown table (struct_size past v1, future fields appended) is still usable: structs only grow.
    K2MenuUiApi grownMui = kMuiTable;
    grownMui.struct_size = static_cast<uint32_t>(sizeof(K2MenuUiApi) + 2 * sizeof(void*));
    MCHECK("MenuClient::usable true for a grown k2.menuui table", k2mc::MenuClient::usable(&grownMui));
    K2ModConfigApi grownCfg = kCfgTable;
    grownCfg.struct_size = static_cast<uint32_t>(sizeof(K2ModConfigApi) + sizeof(void*));
    MCHECK("ConfigClient::usable true for a grown k2.modconfig table", k2mc::ConfigClient::usable(&grownCfg));

    // 8. Null function pointers: any missing v1 entry point makes the table unusable.
    K2ModConfigApi nullReg = kCfgTable;
    nullReg.Register = nullptr;
    MCHECK("ConfigClient::usable false with Register null", !k2mc::ConfigClient::usable(&nullReg));
    K2ModConfigApi nullUnreg = kCfgTable;
    nullUnreg.Unregister = nullptr;
    MCHECK("ConfigClient::usable false with Unregister null", !k2mc::ConfigClient::usable(&nullUnreg));
    K2MenuUiApi nullCtl = kMuiTable;
    nullCtl.Control = nullptr;
    MCHECK("MenuClient::usable false with Control null", !k2mc::MenuClient::usable(&nullCtl));
    K2MenuUiApi nullClick = kMuiTable;
    nullClick.Clicked = nullptr;
    MCHECK("MenuClient::usable false with Clicked null", !k2mc::MenuClient::usable(&nullClick));
    MCHECK("ConfigClient::usable false for a null table", !k2mc::ConfigClient::usable(nullptr));
    MCHECK("MenuClient::usable false for a null table", !k2mc::MenuClient::usable(nullptr));

    // 9. SetDescSplit compatibility: an OLD provider (11-slot table, no SetDescSplit) stays usable and reports no split support; a NEW
    // table has it; a null SetDescSplit or a table that stops short of it has not. A consumer built with this header checks hasDescSplit().
    K2MenuUiApi oldMui = kMuiTable;
    oldMui.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, SetDescSplit));   // the pre-split 11-slot size
    oldMui.SetDescSplit = nullptr;
    MCHECK("MenuClient::usable true for an old 11-slot provider table", k2mc::MenuClient::usable(&oldMui));
    MCHECK("MenuClient::hasDescSplit false for an old 11-slot provider table", !k2mc::MenuClient::hasDescSplit(&oldMui));
    MCHECK("MenuClient::hasDescSplit true for the current table", k2mc::MenuClient::hasDescSplit(&kMuiTable));
    K2MenuUiApi nullSplit = kMuiTable;
    nullSplit.SetDescSplit = nullptr;
    MCHECK("MenuClient::hasDescSplit false when SetDescSplit is null", !k2mc::MenuClient::hasDescSplit(&nullSplit));
    MCHECK("MenuClient::usable still true when only SetDescSplit is null", k2mc::MenuClient::usable(&nullSplit));
    K2MenuUiApi shortSplit = kMuiTable;
    shortSplit.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, SetDescSplit) + 1);   // ends inside SetDescSplit
    MCHECK("MenuClient::hasDescSplit false when struct_size ends inside SetDescSplit", !k2mc::MenuClient::hasDescSplit(&shortSplit));
    MCHECK("MenuClient::hasDescSplit false for a null table", !k2mc::MenuClient::hasDescSplit(static_cast<const K2MenuUiApi*>(nullptr)));
    fakekp::reset();
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &oldMui);
    k2mc::MenuClient oldClient;
    oldClient.init(fakekp::api());
    oldClient.poll();
    MCHECK("MenuClient connects to an old 11-slot provider", oldClient.ok());
    MCHECK("MenuClient::hasDescSplit() member false against an old provider", !oldClient.hasDescSplit());
    fakekp::reset();
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable);
    k2mc::MenuClient newClient;
    newClient.init(fakekp::api());
    newClient.poll();
    MCHECK("MenuClient::hasDescSplit() member true against the current provider", newClient.ok() && newClient.hasDescSplit());

    // 10. Option rows (slots 12..18): a short table (the 12-slot SetDescSplit one) is still usable() but has no rows; the full table has them;
    // any null row slot or a table that ends inside SetRowRange has not.
    K2MenuUiApi shortRows = kMuiTable;
    shortRows.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, AddRow));   // the 12-slot table
    shortRows.AddRow = nullptr; shortRows.SetChangeHandler = nullptr; shortRows.GetValue = nullptr; shortRows.SetValue = nullptr;
    shortRows.SetRowText = nullptr; shortRows.SetRowState = nullptr; shortRows.SetRowRange = nullptr;
    MCHECK("MenuClient::usable true for a 12-slot table", k2mc::MenuClient::usable(&shortRows));
    MCHECK("MenuClient::hasDescSplit true for a 12-slot table", k2mc::MenuClient::hasDescSplit(&shortRows));
    MCHECK("MenuClient::hasRows false for a 12-slot table", !k2mc::MenuClient::hasRows(&shortRows));
    MCHECK("MenuClient::hasRows true for the current table", k2mc::MenuClient::hasRows(&kMuiTable));
    MCHECK("MenuClient::hasRows false for a null table", !k2mc::MenuClient::hasRows(static_cast<const K2MenuUiApi*>(nullptr)));
    {   // MCF9: the four modal / gear slots (19..22) are gated by hasModal(), which needs every one non-null and a table that reaches AddGear
        K2MenuUiApi full = kMuiTable;
        full.SetItemShown = stubSetItemShown; full.NewModal = stubNewModal; full.SetModalOpen = stubSetModalOpen; full.AddGear = stubAddGear;   // only presence is checked
        MCHECK("MenuClient::hasModal true when slots 19..22 are present", k2mc::MenuClient::hasModal(&full));
        MCHECK("MenuClient::hasModal false while those slots are null", !k2mc::MenuClient::hasModal(&kMuiTable));
        K2MenuUiApi noGear = full; noGear.AddGear = nullptr;
        MCHECK("MenuClient::hasModal false with AddGear null", !k2mc::MenuClient::hasModal(&noGear));
        K2MenuUiApi old = full; old.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, SetItemShown));
        MCHECK("MenuClient::hasModal false for a 19-slot table (rows without modal)", !k2mc::MenuClient::hasModal(&old) && k2mc::MenuClient::hasRows(&old));
        MCHECK("MenuClient::hasModal false for a null table", !k2mc::MenuClient::hasModal(static_cast<const K2MenuUiApi*>(nullptr)));
    }
    K2MenuUiApi endsInside = kMuiTable;
    endsInside.struct_size = static_cast<uint32_t>(offsetof(K2MenuUiApi, SetRowRange) + 1);
    MCHECK("MenuClient::hasRows false when struct_size ends inside SetRowRange", !k2mc::MenuClient::hasRows(&endsInside));
    K2MenuUiApi nullRow = kMuiTable;
    nullRow.SetRowState = nullptr;
    MCHECK("MenuClient::hasRows false with SetRowState null", !k2mc::MenuClient::hasRows(&nullRow));
    MCHECK("MenuClient::usable still true with a row slot null", k2mc::MenuClient::usable(&nullRow));
    fakekp::reset();
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &shortRows);
    k2mc::MenuClient oldRowsClient;
    oldRowsClient.init(fakekp::api());
    oldRowsClient.poll();
    MCHECK("MenuClient connects to a 12-slot provider and reports no rows", oldRowsClient.ok() && !oldRowsClient.hasRows());
    fakekp::reset();
    fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMuiTable);
    k2mc::MenuClient rowsClient;
    rowsClient.init(fakekp::api());
    rowsClient.poll();
    MCHECK("MenuClient::hasRows() member true against the current provider", rowsClient.ok() && rowsClient.hasRows());

    return ::kt::Report("common_contract_test (mod-config)");
}
