// script_events_core_test.cpp -- Unit test for ScriptEventsCore.h dispatcher.
#include "ScriptEventsCore.h"
#include "check.h"

#include <cstring>
#include <vector>

namespace {

struct ScriptEventLog {
    int32_t kind;
    std::string name;
    uint32_t objSelf;
    int32_t objSelfValid;
    int32_t runStatus;
};

std::vector<ScriptEventLog> g_scriptLogs;
int32_t g_preRetValMode = 0; // 0 = continue, 1 = handled no ret, 2 = handled with ret

int32_t testScriptCb(const K2ScriptEvent* ev, int32_t* out_has_ret, int32_t* out_ret, void*) {
    if (ev) {
        g_scriptLogs.push_back({ ev->kind, ev->script_name ? ev->script_name : "", ev->object_self, ev->object_self_valid, ev->run_status });
    }
    if (g_preRetValMode == 1) {
        if (out_has_ret) *out_has_ret = 0;
        return K2SC_HANDLED;
    }
    if (g_preRetValMode == 2) {
        if (out_has_ret) *out_has_ret = 1;
        if (out_ret) *out_ret = 42;
        return K2SC_HANDLED;
    }
    return K2SC_CONTINUE;
}

struct CmdEventLog {
    int32_t cmdId;
    int32_t paramCount;
    void* proc;
    uint32_t callerObj;
};

std::vector<CmdEventLog> g_cmdLogs;
int32_t g_cmdMode = 0; // 0 = continue, 1 = handled with status -5

int32_t testCmdCb(const K2VmCommandEvent* ev, int32_t* out_status, void*) {
    if (ev) {
        g_cmdLogs.push_back({ ev->cmd_id, ev->param_count, ev->cmd_processor, ev->caller_object_id });
    }
    if (g_cmdMode == 1) {
        if (out_status) *out_status = -5;
        return K2SC_HANDLED;
    }
    return K2SC_CONTINUE;
}

} // namespace

int main() {
    scriptevents::Dispatcher disp;

    // 1. Script Pre Listener + Case-Insensitive Filter
    g_scriptLogs.clear();
    g_preRetValMode = 0;
    int32_t t1 = disp.registerScriptListener(K2SC_SCRIPT_PRE, "k_ai_master", testScriptCb, nullptr, "test");
    KT_CHECK(t1 > 0);

    // Matching case
    int handled = disp.dispatchScriptPre(nullptr, "k_ai_master", 1001, 1);
    KT_CHECK_EQ(handled, 0);
    KT_CHECK_EQ(g_scriptLogs.size(), 1u);
    KT_CHECK_EQ(g_scriptLogs[0].kind, K2SC_SCRIPT_PRE);
    KT_CHECK_EQ(g_scriptLogs[0].name.c_str(), "k_ai_master");
    KT_CHECK_EQ(g_scriptLogs[0].objSelf, 1001u);

    // Uppercase matching
    handled = disp.dispatchScriptPre(nullptr, "K_AI_MASTER", 1002, 1);
    KT_CHECK_EQ(handled, 0);
    KT_CHECK_EQ(g_scriptLogs.size(), 2u);

    // Non-matching script
    handled = disp.dispatchScriptPre(nullptr, "k_sp_heart", 1003, 1);
    KT_CHECK_EQ(handled, 0);
    KT_CHECK_EQ(g_scriptLogs.size(), 2u);

    // 2. Short-circuit & StartingConditional return value injection
    uint8_t fakeVm[64] = {0};
    g_preRetValMode = 2; // handled with return value 42
    handled = disp.dispatchScriptPre(fakeVm, "k_ai_master", 1001, 1);
    KT_CHECK_EQ(handled, 1); // short-circuited!
    KT_CHECK_EQ(*reinterpret_cast<int32_t*>(fakeVm + scriptevents::kVmOffRetType), scriptevents::kVmRetTypeInt);
    KT_CHECK_EQ(*reinterpret_cast<int32_t*>(fakeVm + scriptevents::kVmOffRetVal), 42);

    // Short-circuit without return value
    std::memset(fakeVm, 0, sizeof(fakeVm));
    g_preRetValMode = 1;
    handled = disp.dispatchScriptPre(fakeVm, "k_ai_master", 1001, 1);
    KT_CHECK_EQ(handled, 1);
    KT_CHECK_EQ(*reinterpret_cast<int32_t*>(fakeVm + scriptevents::kVmOffRetType), 0);
    KT_CHECK_EQ(*reinterpret_cast<int32_t*>(fakeVm + scriptevents::kVmOffRetVal), 0);

    // 3. Script Post Listener & Wildcard Filter
    int32_t t2 = disp.registerScriptListener(K2SC_SCRIPT_POST, nullptr, testScriptCb, nullptr, "post_listener");
    KT_CHECK(t2 > 0);

    g_scriptLogs.clear();
    disp.dispatchScriptPost("random_script", 2001, 1, 0);
    KT_CHECK_EQ(g_scriptLogs.size(), 1u);
    KT_CHECK_EQ(g_scriptLogs[0].kind, K2SC_SCRIPT_POST);
    KT_CHECK_EQ(g_scriptLogs[0].name.c_str(), "random_script");
    KT_CHECK_EQ(g_scriptLogs[0].runStatus, 0);

    // Pre listener should not have fired for post event
    KT_CHECK_EQ(g_scriptLogs[0].objSelf, 2001u);

    // 4. Command Handlers: Specific ID vs K2SC_CMD_ALL
    g_cmdLogs.clear();
    g_cmdMode = 0;
    int32_t tcSpecific = disp.registerCommandHandler(150, testCmdCb, nullptr, "spec_cmd");
    int32_t tcAll = disp.registerCommandHandler(K2SC_CMD_ALL, testCmdCb, nullptr, "all_cmd");
    KT_CHECK(tcSpecific > 0);
    KT_CHECK(tcAll > 0);

    int32_t outStatus = 0;
    void* fakeProc = reinterpret_cast<void*>(0x12340);
    handled = disp.dispatchExecuteCommand(fakeProc, 500, 150, 2, &outStatus);
    KT_CHECK_EQ(handled, 0);
    // Both tcSpecific and tcAll match cmd 150
    KT_CHECK_EQ(g_cmdLogs.size(), 2u);

    // Command 200: only tcAll matches
    g_cmdLogs.clear();
    handled = disp.dispatchExecuteCommand(fakeProc, 500, 200, 0, &outStatus);
    KT_CHECK_EQ(handled, 0);
    KT_CHECK_EQ(g_cmdLogs.size(), 1u);
    KT_CHECK_EQ(g_cmdLogs[0].cmdId, 200);

    // 5. Command short-circuit & outCmdStatus
    g_cmdMode = 1; // handled with status -5
    outStatus = 0;
    handled = disp.dispatchExecuteCommand(fakeProc, 500, 150, 2, &outStatus);
    KT_CHECK_EQ(handled, 1);
    KT_CHECK_EQ(outStatus, -5);

    // 6. Unregister
    disp.unregisterScriptListener(t1);
    disp.unregisterScriptListener(t2);
    disp.unregisterCommandHandler(tcSpecific);
    disp.unregisterCommandHandler(tcAll);

    g_scriptLogs.clear();
    g_cmdLogs.clear();
    disp.dispatchScriptPre(fakeVm, "k_ai_master", 1001, 1);
    disp.dispatchScriptPost("random_script", 2001, 1, 0);
    disp.dispatchExecuteCommand(fakeProc, 500, 150, 2, &outStatus);
    KT_CHECK_EQ(g_scriptLogs.size(), 0u);
    KT_CHECK_EQ(g_cmdLogs.size(), 0u);

    // 7. Disabled flag suppresses all events
    disp.registerScriptListener(K2SC_SCRIPT_ALL, nullptr, testScriptCb, nullptr, "test");
    disp.registerCommandHandler(K2SC_CMD_ALL, testCmdCb, nullptr, "test");
    disp.enabled = false;

    g_scriptLogs.clear();
    g_cmdLogs.clear();
    KT_CHECK_EQ(disp.dispatchScriptPre(fakeVm, "test", 1, 1), 0);
    disp.dispatchScriptPost("test", 1, 1, 0);
    KT_CHECK_EQ(disp.dispatchExecuteCommand(fakeProc, 1, 1, 0, &outStatus), 0);
    KT_CHECK_EQ(g_scriptLogs.size(), 0u);
    KT_CHECK_EQ(g_cmdLogs.size(), 0u);

    return kt::Report("script_events_core_test");
}
