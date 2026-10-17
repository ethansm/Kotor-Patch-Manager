// script_events_test.cpp -- Unit test for ScriptEvents.h and k2sc::Client.
#include "ScriptEvents.h"
#include "check.h"

#include <cstring>
#include <vector>

namespace fakekp {

struct Entry {
    const char* name;
    uint32_t version;
    const void* iface;
};

static Entry g_entries[32];
static uint32_t g_entryCount = 0;
static int g_provideCalls = 0;
static int g_requireCalls = 0;

static int32_t KPATCH_CALL provide(const char* name, uint32_t version, const void* iface) {
    ++g_provideCalls;
    if (!name || !iface || g_entryCount >= 32) return 1;
    for (uint32_t i = 0; i < g_entryCount; ++i) {
        if (std::strcmp(g_entries[i].name, name) == 0 && g_entries[i].version == version) {
            g_entries[i].iface = iface;
            return 0;
        }
    }
    g_entries[g_entryCount++] = { name, version, iface };
    return 0;
}

static const void* KPATCH_CALL require(const char* name, uint32_t version) {
    ++g_requireCalls;
    if (!name) return nullptr;
    for (uint32_t i = 0; i < g_entryCount; ++i) {
        if (std::strcmp(g_entries[i].name, name) == 0 && g_entries[i].version == version) {
            return g_entries[i].iface;
        }
    }
    return nullptr;
}

static void reset() {
    g_entryCount = 0;
    g_provideCalls = 0;
    g_requireCalls = 0;
}

static const KPatchApi g_api = {
    sizeof(KPatchApi),
    1,
    provide,
    require
};

static const KPatchApi* api() { return &g_api; }

} // namespace fakekp

namespace {

struct FakeScriptReg {
    int32_t token;
    int32_t kindMask;
    std::string filter;
    K2ScriptCallback fn;
    void* user;
    std::string owner;
};

struct FakeCmdReg {
    int32_t token;
    int32_t cmdId;
    K2VmCommandCallback fn;
    void* user;
    std::string owner;
};

struct FakeProviderState {
    std::vector<FakeScriptReg> scriptRegs;
    std::vector<FakeCmdReg> cmdRegs;
    std::vector<int32_t> unregScriptTokens;
    std::vector<int32_t> unregCmdTokens;
    int32_t nextToken = 100;

    void reset() {
        scriptRegs.clear();
        cmdRegs.clear();
        unregScriptTokens.clear();
        unregCmdTokens.clear();
        nextToken = 100;
    }
} g_fakeProvider;

int32_t KPATCH_CALL fakeRegisterScriptListener(int32_t kind_mask, const char* script_filter, K2ScriptCallback fn, void* user, const char* owner) {
    if (!fn) return 0;
    int32_t t = g_fakeProvider.nextToken++;
    g_fakeProvider.scriptRegs.push_back({ t, kind_mask, script_filter ? script_filter : "", fn, user, owner ? owner : "" });
    return t;
}

void KPATCH_CALL fakeUnregisterScriptListener(int32_t token) {
    g_fakeProvider.unregScriptTokens.push_back(token);
}

int32_t KPATCH_CALL fakeRegisterCommandHandler(int32_t cmd_id, K2VmCommandCallback fn, void* user, const char* owner) {
    if (!fn) return 0;
    int32_t t = g_fakeProvider.nextToken++;
    g_fakeProvider.cmdRegs.push_back({ t, cmd_id, fn, user, owner ? owner : "" });
    return t;
}

void KPATCH_CALL fakeUnregisterCommandHandler(int32_t token) {
    g_fakeProvider.unregCmdTokens.push_back(token);
}

const K2ScriptEventsApi g_fakeApi = {
    sizeof(K2ScriptEventsApi),
    fakeRegisterScriptListener,
    fakeUnregisterScriptListener,
    fakeRegisterCommandHandler,
    fakeUnregisterCommandHandler
};

int32_t dummyScriptCb(const K2ScriptEvent*, int32_t*, int32_t*, void*) {
    return K2SC_CONTINUE;
}

int32_t dummyCmdCb(const K2VmCommandEvent*, int32_t*, void*) {
    return K2SC_CONTINUE;
}

} // namespace

int main() {
    // 1. ABI Struct Layouts
    KT_CHECK_EQ(sizeof(K2ScriptEventsApi), 5 * sizeof(void*));
    KT_CHECK_EQ(offsetof(K2ScriptEventsApi, RegisterScriptListener), sizeof(void*));
    KT_CHECK_EQ(offsetof(K2ScriptEventsApi, UnregisterScriptListener), 2 * sizeof(void*));
    KT_CHECK_EQ(offsetof(K2ScriptEventsApi, RegisterCommandHandler), 3 * sizeof(void*));
    KT_CHECK_EQ(offsetof(K2ScriptEventsApi, UnregisterCommandHandler), 4 * sizeof(void*));

    KT_CHECK(sizeof(K2ScriptEvent) >= 24);
    KT_CHECK(sizeof(K2VmCommandEvent) >= 16);

    // 2. Safe no-ops before connect
    fakekp::reset();
    g_fakeProvider.reset();

    k2sc::Client client;
    client.init(fakekp::api(), "test-mod");
    KT_CHECK(!client.connected);

    KT_CHECK_EQ(client.registerScriptListener(K2SC_SCRIPT_PRE, "k_ai_master", dummyScriptCb), 0);
    KT_CHECK_EQ(client.registerCommandHandler(10, dummyCmdCb), 0);
    client.unregisterScriptListener(999);
    client.unregisterCommandHandler(999);

    // 3. Queue registrations before connect
    KT_CHECK(client.addScriptListener(K2SC_SCRIPT_PRE, "k_ai_master", dummyScriptCb));
    KT_CHECK(client.addScriptListener(K2SC_SCRIPT_POST, nullptr, dummyScriptCb));
    KT_CHECK(client.addCommandHandler(K2SC_CMD_ALL, dummyCmdCb));
    KT_CHECK(client.addCommandHandler(42, dummyCmdCb));

    // Connect fails before provider is available
    KT_CHECK(!client.connect());
    KT_CHECK(!client.connected);

    // Provide interface
    fakekp::provide(K2_SCRIPTEVENTS_IFACE, K2_SCRIPTEVENTS_VERSION, &g_fakeApi);

    // Connect succeeds now and auto-registers queued callbacks
    KT_CHECK(client.connect());
    KT_CHECK(client.connected);
    KT_CHECK_EQ(g_fakeProvider.scriptRegs.size(), 2u);
    KT_CHECK_EQ(g_fakeProvider.cmdRegs.size(), 2u);
    KT_CHECK_EQ(g_fakeProvider.scriptRegs[0].filter.c_str(), "k_ai_master");
    KT_CHECK_EQ(g_fakeProvider.scriptRegs[0].owner.c_str(), "test-mod");
    KT_CHECK_EQ(g_fakeProvider.cmdRegs[1].cmdId, 42);

    // Adding after connect registers immediately
    KT_CHECK(client.addScriptListener(K2SC_SCRIPT_ALL, "k_sp_heart", dummyScriptCb));
    KT_CHECK_EQ(g_fakeProvider.scriptRegs.size(), 3u);

    // Direct wrappers work while connected
    int32_t directToken = client.registerCommandHandler(77, dummyCmdCb);
    KT_CHECK(directToken > 0);
    KT_CHECK_EQ(g_fakeProvider.cmdRegs.size(), 3u);

    client.unregisterCommandHandler(directToken);
    KT_CHECK_EQ(g_fakeProvider.unregCmdTokens.size(), 1u);
    KT_CHECK_EQ(g_fakeProvider.unregCmdTokens[0], directToken);

    // 4. Release unregisters everything
    client.release();
    KT_CHECK(!client.connected);
    // 3 script listeners + 2 queued cmd handlers should have been unregistered
    KT_CHECK_EQ(g_fakeProvider.unregScriptTokens.size(), 3u);
    KT_CHECK_EQ(g_fakeProvider.unregCmdTokens.size(), 3u); // 1 direct + 2 queued

    // 5. Lazy poll connection test
    fakekp::reset();
    g_fakeProvider.reset();
    k2sc::Client pollClient;
    pollClient.init(fakekp::api(), "poll-mod");
    pollClient.addScriptListener(K2SC_SCRIPT_PRE, "", dummyScriptCb);

    // Before provide: 256 polls without provider
    for (int i = 0; i < 256; ++i) {
        pollClient.poll();
    }
    KT_CHECK(!pollClient.connected);

    // Now provide and poll again
    fakekp::provide(K2_SCRIPTEVENTS_IFACE, K2_SCRIPTEVENTS_VERSION, &g_fakeApi);
    pollClient.poll(); // at count 256, connects!
    KT_CHECK(pollClient.connected);
    KT_CHECK_EQ(g_fakeProvider.scriptRegs.size(), 1u);

    return kt::Report("script_events_test");
}
