// Host test of ProviderClient.h: ClientSet (init/poll/allOk/anyMissing over k2mc::ConfigClient, k2mc::MenuClient, k2p3::Client,
// k2holster::Client) against a fake KPatchApi registry, and registerOnce/unregister against a stub k2.modconfig table.
// Build/run: python3 scripts/run_tests.py providerclient
// Needs kpatch_api.h on the include path (see _shared_src/test.toml [providerclient_test]).
#include "check.h"
#include "FakeKPatchApi.h"
#include "ProviderClient.h"
#include "Preview3dApi.h"
#include "HolsterApi.h"
#include <string.h>

namespace {

// ---- stub k2.modconfig ----
int g_regCalls = 0, g_unregCalls = 0;
int32_t g_regResult = 7;      // what Register answers (<= 0 = refused)
int32_t g_lastUnreg = -1;
const K2ModConfigEntry* g_lastEntry = nullptr;
int32_t KPATCH_CALL stubRegister(const K2ModConfigEntry* e) { ++g_regCalls; g_lastEntry = e; return g_regResult; }
void KPATCH_CALL stubUnregister(int32_t t) { ++g_unregCalls; g_lastUnreg = t; }
const K2ModConfigApi kCfg = { sizeof(K2ModConfigApi), stubRegister, stubUnregister };
const K2ModConfigApi kCfgSmall = { 4, stubRegister, stubUnregister };       // struct_size too small
const K2ModConfigApi kCfgNullFn = { sizeof(K2ModConfigApi), nullptr, stubUnregister };

// ---- stub k2.menuui (all 11 pre-SetDescSplit slots non-null, SetDescSplit absent) ----
uint32_t KPATCH_CALL mNewPage(void*, const char*) { return 1; }
int32_t KPATCH_CALL mAddButton(uint32_t, const char*, const char*) { return 0; }
int32_t KPATCH_CALL mAddLabel(uint32_t, const char*) { return 0; }
int32_t KPATCH_CALL mAddCustom(uint32_t, void*, int32_t) { return 0; }
int32_t KPATCH_CALL mClicked(uint32_t, int32_t) { return 0; }
void KPATCH_CALL mSetText(uint32_t, int32_t, const char*) {}
void KPATCH_CALL mSetRect(uint32_t, int32_t, const K2MuiRect*) {}
void KPATCH_CALL mSetDesc(uint32_t, const char*) {}
int32_t KPATCH_CALL mPaneRect(void*, int32_t, K2MuiRect*) { return 0; }
void* KPATCH_CALL mControl(uint32_t, int32_t) { return nullptr; }
const K2MenuUiApi kMenu = { offsetof(K2MenuUiApi, SetDescSplit), mNewPage, mAddButton, mAddLabel, mAddCustom, mClicked, mSetText, mSetRect,
                            mSetDesc, mPaneRect, mControl, nullptr };

// ---- p3 / holster tables: every pointer slot after struct_size is a dummy function ----
void KPATCH_CALL dummyFn() {}
template <class T>
void fillAllSlots(T& t) {
    memset(&t, 0, sizeof t);
    *reinterpret_cast<uint32_t*>(&t) = static_cast<uint32_t>(sizeof t);
    void (KPATCH_CALL *f)() = dummyFn;
    for (size_t off = sizeof(void*); off + sizeof(void*) <= sizeof t; off += sizeof(void*))
        memcpy(reinterpret_cast<char*>(&t) + off, &f, sizeof f);
}

// ---- log capture ----
int g_nLog = 0, g_nInfo = 0, g_nWarn = 0;
char g_lastMsg[96];
void logSink(int level, const char* msg) {
    ++g_nLog;
    if (level == KPLOG_INFO) ++g_nInfo;
    if (level == KPLOG_WARN) ++g_nWarn;
    strncpy(g_lastMsg, msg, sizeof g_lastMsg - 1);
    g_lastMsg[sizeof g_lastMsg - 1] = 0;
}

void resetStubs() {
    g_regCalls = g_unregCalls = 0; g_regResult = 7; g_lastUnreg = -1; g_lastEntry = nullptr;
    g_nLog = g_nInfo = g_nWarn = 0; g_lastMsg[0] = 0;
}

K2ModConfigEntry makeEntry() {
    K2ModConfigEntry e;
    memset(&e, 0, sizeof e);
    e.struct_size = sizeof e;
    e.id = "test-mod";
    e.name = "Test Mod";
    e.desc = "desc";
    return e;
}

}  // namespace

int main() {
    // ---- not provided: first poll tries, then every 256th; stays !ok ----
    {
        fakekp::reset();
        k2mc::ConfigClient cfg; k2mc::MenuClient menu;
        k2pc::ClientSet set(cfg, menu);
        set.init(fakekp::api());
        KT_CHECK(!set.allOk());
        KT_CHECK(set.anyMissing());
        set.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, 2);          // one try per client on the first poll
        for (int i = 0; i < 255; ++i) set.poll();              // polls 2..256: no retry
        KT_CHECK_EQ(fakekp::state().requireCalls, 2);
        set.poll();                                            // poll 257: retry
        KT_CHECK_EQ(fakekp::state().requireCalls, 4);
        KT_CHECK(!cfg.ok() && !menu.ok());
        KT_CHECK(!set.allOk() && set.anyMissing());

        // ---- provided later: connects once, then stops asking ----
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        for (int i = 0; i < 256; ++i) set.poll();              // crosses the next 256th call
        KT_CHECK(cfg.ok());
        KT_CHECK(!menu.ok());                                  // menuui still missing
        KT_CHECK(!set.allOk() && set.anyMissing());
        fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMenu);
        for (int i = 0; i < 256; ++i) set.poll();
        KT_CHECK(menu.ok());
        KT_CHECK(set.allOk() && !set.anyMissing());
        const int calls = fakekp::state().requireCalls;
        for (int i = 0; i < 2000; ++i) set.poll();
        KT_CHECK_EQ(fakekp::state().requireCalls, calls);      // connected clients never ask again
        KT_CHECK(cfg.p == &kCfg);
        KT_CHECK(menu.p == &kMenu);
    }

    // ---- connect exactly once (onConnect fires once) ----
    {
        static int nConnect = 0, nMissing = 0;
        nConnect = nMissing = 0;
        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        k2mc::ConfigClient cfg;
        cfg.onConnect = [](bool ok, const char*) { if (ok) ++nConnect; else ++nMissing; };   // init() leaves onConnect alone
        k2pc::ClientSet set(cfg);
        set.init(fakekp::api());
        for (int i = 0; i < 600; ++i) set.poll();
        KT_CHECK_EQ(nConnect, 1);
        KT_CHECK_EQ(nMissing, 0);
        KT_CHECK(set.allOk());
    }

    // ---- table too small / null function: !ok, no crash, missing reported once ----
    {
        static int nMissing = 0;
        nMissing = 0;
        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfgSmall);
        k2mc::ConfigClient cfg;
        k2pc::ClientSet set(cfg);
        set.init(fakekp::api());
        cfg.onConnect = [](bool ok, const char*) { if (!ok) ++nMissing; };
        for (int i = 0; i < 600; ++i) set.poll();
        KT_CHECK(!cfg.ok());
        KT_CHECK(cfg.p == nullptr);
        KT_CHECK(!set.allOk());
        KT_CHECK_EQ(nMissing, 1);

        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfgNullFn);
        k2mc::ConfigClient cfg2;
        cfg2.init(fakekp::api());
        for (int i = 0; i < 300; ++i) cfg2.poll();
        KT_CHECK(!cfg2.ok());

        // registerOnce on a !ok client is a no-op
        resetStubs();
        K2ModConfigEntry e = makeEntry();
        k2pc::RegisterOnce st;
        KT_CHECK(!k2pc::registerOnce(st, cfg2, e, logSink));
        KT_CHECK(!st.registered);
        KT_CHECK_EQ(g_regCalls, 0);
        KT_CHECK_EQ(g_nLog, 0);
    }

    // ---- no KPatchApi / api too small / null require: !ok, no crash ----
    {
        k2mc::ConfigClient c1;
        c1.init(nullptr);
        c1.poll();
        KT_CHECK(!c1.ok());
        fakekp::reset(8);                                       // struct_size too small to hold require
        k2mc::ConfigClient c2;
        c2.init(fakekp::api());
        c2.poll();
        KT_CHECK(!c2.ok());
        KPatchApi bare = {};
        bare.struct_size = sizeof bare;                         // require == nullptr
        k2mc::MenuClient c3;
        c3.init(&bare);
        c3.poll();
        KT_CHECK(!c3.ok());
        k2pc::ClientSet set(c1, c2, c3);
        KT_CHECK(!set.allOk() && set.anyMissing());
    }

    // ---- duck typing across all four client types, plus an empty set ----
    {
        k2pc::ClientSet<> none;
        KT_CHECK(none.allOk());

        fakekp::reset();
        K2Preview3dApi p3; fillAllSlots(p3);
        K2HolsterApi hol; fillAllSlots(hol);
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        fakekp::provide(K2MENUUI_IFACE, K2MENUUI_VERSION, &kMenu);
        fakekp::provide(K2PREVIEW3D_IFACE, K2PREVIEW3D_VERSION, &p3);
        k2mc::ConfigClient cfg; k2mc::MenuClient menu; k2p3::Client pv; k2holster::Client hc;
        auto set = k2pc::makeSet(cfg, menu, pv, hc);
        set.init(fakekp::api());
        set.poll();                                             // first poll connects whatever is provided
        KT_CHECK(cfg.ok() && menu.ok() && pv.ok());
        KT_CHECK(!hc.ok());
        KT_CHECK(set.anyMissing());
        fakekp::provide(K2HOLSTER_IFACE, K2HOLSTER_VERSION, &hol);
        for (int i = 0; i < 256; ++i) set.poll();
        KT_CHECK(hc.ok());
        KT_CHECK(set.allOk());
        // re-init disconnects all (clients restart from scratch)
        set.init(fakekp::api());
        KT_CHECK(set.anyMissing());
        set.poll();
        KT_CHECK(set.allOk());
    }

    // ---- registerOnce: succeeds exactly once ----
    {
        resetStubs();
        fakekp::reset();
        k2mc::ConfigClient cfg;
        cfg.init(fakekp::api());
        K2ModConfigEntry e = makeEntry();
        k2pc::RegisterOnce st;
        cfg.poll();                                             // not provided yet
        KT_CHECK(!k2pc::registerOnce(st, cfg, e, logSink));
        KT_CHECK_EQ(g_regCalls, 0);
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        for (int i = 0; i < 256; ++i) cfg.poll();
        KT_CHECK(cfg.ok());
        KT_CHECK(k2pc::registerOnce(st, cfg, e, logSink));
        KT_CHECK_EQ(g_regCalls, 1);
        KT_CHECK(g_lastEntry == &e);                            // handed the caller's struct (Register copies it)
        KT_CHECK(st.registered && st.accepted());
        KT_CHECK_EQ(st.token, 7);
        KT_CHECK_EQ(g_nLog, 1);
        KT_CHECK_EQ(g_nInfo, 1);
        KT_CHECK_EQ(g_lastMsg, "page REGISTER ok token=7");
        for (int i = 0; i < 1000; ++i) KT_CHECK(k2pc::registerOnce(st, cfg, e, logSink));
        KT_CHECK_EQ(g_regCalls, 1);                             // never twice
        KT_CHECK_EQ(g_nLog, 1);                                 // logged once
        KT_CHECK_EQ(st.token, 7);

        // null log callback is allowed
        resetStubs();
        k2pc::RegisterOnce st2;
        KT_CHECK(k2pc::registerOnce(st2, cfg, e, nullptr));
        KT_CHECK_EQ(g_regCalls, 1);
        KT_CHECK_EQ(g_nLog, 0);
    }

    // ---- registerOnce: refused (token 0) logs once, never retries ----
    {
        resetStubs();
        g_regResult = 0;
        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        k2mc::ConfigClient cfg;
        cfg.init(fakekp::api());
        cfg.poll();
        K2ModConfigEntry e = makeEntry();
        k2pc::RegisterOnce st;
        KT_CHECK(k2pc::registerOnce(st, cfg, e, logSink));      // answered (refused)
        KT_CHECK(st.registered);
        KT_CHECK(!st.accepted());
        KT_CHECK_EQ(st.token, 0);
        KT_CHECK_EQ(g_regCalls, 1);
        KT_CHECK_EQ(g_nLog, 1);
        KT_CHECK_EQ(g_nWarn, 1);
        for (int i = 0; i < 1000; ++i) { cfg.poll(); k2pc::registerOnce(st, cfg, e, logSink); }
        KT_CHECK_EQ(g_regCalls, 1);                             // no registration storm
        KT_CHECK_EQ(g_nLog, 1);                                 // no log storm
        // negative token counts as refused too
        resetStubs();
        g_regResult = -3;
        k2pc::RegisterOnce st3;
        KT_CHECK(k2pc::registerOnce(st3, cfg, e, logSink));
        KT_CHECK(!st3.accepted());
        KT_CHECK_EQ(st3.token, -3);
        KT_CHECK_EQ(g_nWarn, 1);
        // unregister of a refused registration does not call the provider, and clears the state
        k2pc::unregister(st3, cfg);
        KT_CHECK_EQ(g_unregCalls, 0);
        KT_CHECK(!st3.registered);
    }

    // ---- unregister ----
    {
        resetStubs();
        fakekp::reset();
        fakekp::provide(K2MODCONFIG_IFACE, K2MODCONFIG_VERSION, &kCfg);
        k2mc::ConfigClient cfg;
        cfg.init(fakekp::api());
        cfg.poll();
        K2ModConfigEntry e = makeEntry();
        k2pc::RegisterOnce st;
        k2pc::unregister(st, cfg);                              // never registered: no-op
        KT_CHECK_EQ(g_unregCalls, 0);
        KT_CHECK(k2pc::registerOnce(st, cfg, e, logSink));
        k2pc::unregister(st, cfg);
        KT_CHECK_EQ(g_unregCalls, 1);
        KT_CHECK_EQ(g_lastUnreg, 7);
        KT_CHECK(!st.registered);
        KT_CHECK_EQ(st.token, 0);
        k2pc::unregister(st, cfg);                              // twice: no-op
        KT_CHECK_EQ(g_unregCalls, 1);
        KT_CHECK(k2pc::registerOnce(st, cfg, e, logSink));      // may register again after unregister
        KT_CHECK_EQ(g_regCalls, 2);
        KT_CHECK(st.accepted());
        // multi-digit token formatting
        resetStubs();
        g_regResult = 123456;
        k2pc::RegisterOnce st4;
        k2pc::registerOnce(st4, cfg, e, logSink);
        KT_CHECK_EQ(g_lastMsg, "page REGISTER ok token=123456");
    }

    return kt::Report("providerclient_test");
}
