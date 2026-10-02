// Host test for the patch registry (RFC #180): provide/require, versioning,
// duplicates, ABI growth, attribution, re-entrancy, concurrency and shutdown.
// Rows are isolated by Registry::Reset(), which is also what the patcher calls
// after unloading modules.
//
// Registry log lines go to stderr through Platform::Log. Where a row must prove a
// line was emitted, stderr is redirected to a temp file around the call.
#include "registry.h"
#include "platform.h"
#include "check.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>

using namespace KotorPatcher;

namespace {

    // Run `fn` with fd 2 redirected to a temp file and return what it wrote.
    std::string CaptureStderr(const std::function<void()>& fn) {
        std::fflush(stderr);
        std::FILE* tmp = std::tmpfile();
        int saved = dup(2);
        dup2(fileno(tmp), 2);
        fn();
        std::fflush(stderr);
        dup2(saved, 2);
        close(saved);

        std::string text;
        std::rewind(tmp);
        char buf[512];
        size_t n;
        while ((n = std::fread(buf, 1, sizeof(buf), tmp)) > 0) {
            text.append(buf, n);
        }
        std::fclose(tmp);
        return text;
    }

    bool Contains(const std::string& haystack, const char* needle) {
        return haystack.find(needle) != std::string::npos;
    }

    int g_a, g_b, g_c;  // distinct addresses to use as interface pointers

    // A v1 interface that has grown one field since its first release.
    struct Svc {
        uint32_t struct_size;
        int (*basic)();
        int (*added)();
    };
    int BasicFn() { return 1; }
    int AddedFn() { return 2; }

    // What a consumer built against the grown struct does: read the new field only
    // if the provider's struct is big enough to contain it.
    int ConsumerCall(const void* iface) {
        const Svc* svc = static_cast<const Svc*>(iface);
        const size_t need = offsetof(Svc, added) + sizeof(svc->added);
        if (svc->struct_size >= need && svc->added) {
            return svc->added();
        }
        return svc->basic() + 100;  // fallback for an older provider
    }

    const KPatchApi* g_initApi = nullptr;
    const void* g_requiredInInit = nullptr;
    int g_rc[4];

    void KPATCH_CALL InnerInit(const KPatchApi* inner) {
        g_rc[1] = inner->provide("re.inner", 1, &g_b);
    }

    void KPATCH_CALL ReentrantInit(const KPatchApi* api) {
        g_initApi = api;
        g_rc[0] = api->provide("re.one", 1, &g_a);
        g_requiredInInit = api->require("re.one", 1);
        // Nested init: a patch's init must not corrupt the outer attribution.
        Registry::RunInitForTest("inner", InnerInit);
        g_rc[2] = api->provide("re.two", 1, &g_c);
    }

    void KPATCH_CALL DupInitA(const KPatchApi* api) { g_rc[0] = api->provide("dup.svc", 1, &g_a); }
    void KPATCH_CALL DupInitB(const KPatchApi* api) { g_rc[1] = api->provide("dup.svc", 1, &g_b); }

    void KPATCH_CALL DumpInit(const KPatchApi* api) {
        api->provide("x.b", 1, &g_a);
        api->provide("x.a", 2, &g_b);
    }

    void Row(const char* name) { std::printf("  -- %s\n", name); }

} // namespace

int main() {
    std::printf("  registry, %zu-bit\n", sizeof(void*) * 8);
    Registry::Init();

    Row("provide / require");
    {
        Registry::Reset();
        int rc = Registry::Provide("test.svc", 1, &g_a);
        kptest::Check("provide accepted", rc == 0);
        kptest::Check("require returns same pointer", Registry::Require("test.svc", 1) == &g_a);
        kptest::Check("missing name -> null", Registry::Require("no.such", 1) == nullptr);
        kptest::Check("require(null/empty) -> null",
                      Registry::Require(nullptr, 1) == nullptr && Registry::Require("", 1) == nullptr);
    }

    Row("versioning");
    {
        Registry::Reset();
        Registry::Provide("ver.svc", 1, &g_a);
        kptest::Check("other major -> null", Registry::Require("ver.svc", 2) == nullptr);
        kptest::Check("second major accepted", Registry::Provide("ver.svc", 2, &g_b) == 0);
        kptest::Check("v1 still its own pointer", Registry::Require("ver.svc", 1) == &g_a);
        kptest::Check("v2 its own pointer", Registry::Require("ver.svc", 2) == &g_b);
        kptest::Check("v3 -> null", Registry::Require("ver.svc", 3) == nullptr);
    }

    Row("duplicate");
    {
        Registry::Reset();
        g_rc[0] = g_rc[1] = 99;
        std::string log = CaptureStderr([] {
            Registry::RunInitForTest("provA", DupInitA);
            Registry::RunInitForTest("provB", DupInitB);
        });
        kptest::Check("first provide accepted", g_rc[0] == 0);
        kptest::Check("duplicate rejected (nonzero)", g_rc[1] != 0);
        kptest::Check("first pointer still served", Registry::Require("dup.svc", 1) == &g_a);
        kptest::Check("duplicate log line emitted",
                      Contains(log, "[KotorPatcher] Registry: duplicate provide of 'dup.svc' v1 "
                                    "by 'provB' rejected (provided by 'provA')\n"));
        // Same name, other major is not a duplicate.
        kptest::Check("same name other major ok", Registry::Provide("dup.svc", 2, &g_c) == 0);
    }

    Row("invalid arguments");
    {
        Registry::Reset();
        std::string log = CaptureStderr([] {
            kptest::Check("null name rejected", Registry::Provide(nullptr, 1, &g_a) != 0);
            kptest::Check("empty name rejected", Registry::Provide("", 1, &g_a) != 0);
            kptest::Check("null iface rejected", Registry::Provide("bad.svc", 1, nullptr) != 0);
        });
        kptest::Check("nothing registered", Registry::Require("bad.svc", 1) == nullptr);
        kptest::Check("rejections logged", Contains(log, "invalid provide of '(null)'") &&
                                           Contains(log, "invalid provide of 'bad.svc'"));
    }

    Row("struct_size growth");
    {
        Registry::Reset();
        Svc grown = { sizeof(Svc), BasicFn, AddedFn };
        Svc old   = { static_cast<uint32_t>(offsetof(Svc, added)), BasicFn, nullptr };
        Registry::Provide("grow.new", 1, &grown);
        Registry::Provide("grow.old", 1, &old);
        kptest::Check("grown provider: new field used", ConsumerCall(Registry::Require("grow.new", 1)) == 2);
        kptest::Check("old provider: fallback taken", ConsumerCall(Registry::Require("grow.old", 1)) == 101);
    }

    Row("KPatchApi contents");
    {
        const KPatchApi* api = Registry::GetApi();
        kptest::Check("api non-null", api != nullptr);
        kptest::Check("struct_size", api && api->struct_size == sizeof(KPatchApi));
        kptest::Check("abi_version", api && api->abi_version == KPATCH_ABI_VERSION);
        kptest::Check("provide/require non-null", api && api->provide && api->require);
        Registry::Reset();
        kptest::Check("api->provide works", api && api->provide("api.svc", 1, &g_a) == 0);
        kptest::Check("api->require works", api && api->require("api.svc", 1) == &g_a);
        kptest::Check("api pointer is stable", Registry::GetApi() == api);
    }

    Row("re-entrant provide in init");
    {
        Registry::Reset();
        for (int& r : g_rc) r = 99;
        g_requiredInInit = nullptr;
        std::atomic<bool> done{false};
        std::thread worker([&] {
            Registry::RunInitForTest("reentrant", ReentrantInit);
            done = true;
        });
        // A deadlock would hang the whole suite, so give it a deadline.
        for (int i = 0; i < 500 && !done; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (!done) {
            kptest::Check("init completes (no deadlock)", false);
            kptest::Report();
            std::fflush(stdout);
            _exit(1);
        }
        worker.join();
        kptest::Check("init completes (no deadlock)", true);
        kptest::Check("provide inside init ok", g_rc[0] == 0 && g_rc[1] == 0 && g_rc[2] == 0);
        kptest::Check("require inside init sees own provide", g_requiredInInit == &g_a);
        kptest::Check("init received the api", g_initApi == Registry::GetApi());

        std::string dump;
        Registry::DumpProvided(dump);
        kptest::Check("attribution: outer survives nested init",
                      dump == "  re.inner v1 by inner\n  re.one v1 by reentrant\n  re.two v1 by reentrant\n");
        kptest::Check("RunInitForTest(null fn) -> false", !Registry::RunInitForTest("x", nullptr));
    }

    Row("dump and attribution");
    {
        Registry::Reset();
        std::string empty;
        Registry::DumpProvided(empty);
        kptest::Check("empty registry dumps nothing", empty.empty());

        Registry::Provide("x.a", 1, &g_c);              // outside init: the core
        Registry::RunInitForTest("alpha", DumpInit);
        Registry::Provide("x.c", 1, &g_c);              // attribution cleared again
        std::string dump;
        Registry::DumpProvided(dump);
        kptest::Check("sorted, one line per provider",
                      dump == "  x.a v1 by patcher\n  x.a v2 by alpha\n  x.b v1 by alpha\n  x.c v1 by patcher\n");

        std::string appended = "head\n";
        Registry::DumpProvided(appended);
        kptest::Check("dump appends", appended == "head\n" + dump);
    }

    Row("OnModuleLoaded");
    {
        Registry::Reset();
        kptest::Check("null handle -> false", !Registry::OnModuleLoaded(nullptr, "p"));
        void* handle = Platform::LoadModule("libm.so.6");
        kptest::Check("test module loaded", handle != nullptr);
        std::string log = CaptureStderr([&] {
            kptest::Check("no KPatchInit -> false", !Registry::OnModuleLoaded(handle, "libm"));
            kptest::Check("again -> false", !Registry::OnModuleLoaded(handle, "libm"));
        });
        kptest::Check("absent KPatchInit logs nothing", log.find("Registry") == std::string::npos);
        Platform::UnloadModule(handle);
    }

    Row("concurrent provide / require");
    {
        Registry::Reset();
        constexpr int kThreads = 8;
        constexpr int kRounds = 200;
        static int objs[kThreads];
        static int shared;
        Registry::Provide("shared.svc", 1, &shared);

        std::atomic<bool> go{false};
        std::atomic<int> badShared{0}, badOwn{0}, raceWins{0};
        std::atomic<const void*> raceWinner{nullptr};
        std::vector<std::thread> threads;
        for (int t = 0; t < kThreads; ++t) {
            threads.emplace_back([&, t] {
                while (!go) std::this_thread::yield();
                std::string own = "thr." + std::to_string(t);
                // Everyone contends for the same name; exactly one may win.
                if (Registry::Provide("race.svc", 1, &objs[t]) == 0) {
                    ++raceWins;
                    raceWinner = &objs[t];
                }
                for (int i = 0; i < kRounds; ++i) {
                    if (i == 0 && Registry::Provide(own.c_str(), 1, &objs[t]) != 0) ++badOwn;
                    if (Registry::Require("shared.svc", 1) != &shared) ++badShared;
                    const void* mine = Registry::Require(own.c_str(), 1);
                    if (mine != nullptr && mine != &objs[t]) ++badOwn;
                    if (i > 0 && mine != &objs[t]) ++badOwn;
                    // Other threads' names resolve to null or their own object, never garbage.
                    int other = (t + 1) % kThreads;
                    std::string theirs = "thr." + std::to_string(other);
                    const void* p = Registry::Require(theirs.c_str(), 1);
                    if (p != nullptr && p != &objs[other]) ++badOwn;
                }
            });
        }
        go = true;
        for (auto& th : threads) th.join();

        kptest::Check("shared lookups consistent", badShared == 0);
        kptest::Check("own lookups consistent", badOwn == 0);
        kptest::Check("exactly one race winner", raceWins == 1);
        kptest::Check("winner is the served pointer", Registry::Require("race.svc", 1) == raceWinner.load());
        bool all = true;
        for (int t = 0; t < kThreads; ++t) {
            all = all && Registry::Require(("thr." + std::to_string(t)).c_str(), 1) == &objs[t];
        }
        kptest::Check("all distinct names registered", all);
        std::string dump;
        Registry::DumpProvided(dump);
        size_t lines = 0;
        for (char c : dump) lines += (c == '\n');
        kptest::Check("dump has every interface", lines == static_cast<size_t>(kThreads) + 2);
    }

    Row("close / reset");
    {
        Registry::Reset();
        Registry::Provide("life.svc", 1, &g_a);
        Registry::Close();
        kptest::Check("require after close -> null", Registry::Require("life.svc", 1) == nullptr);
        std::string log = CaptureStderr([] {
            kptest::Check("provide after close rejected", Registry::Provide("late.svc", 1, &g_b) != 0);
        });
        kptest::Check("close rejection logged", Contains(log, "provide of 'late.svc' v1 by 'patcher' rejected"));
        kptest::Check("api->require after close -> null", Registry::GetApi()->require("life.svc", 1) == nullptr);
        kptest::Check("OnModuleLoaded after close -> false", !Registry::OnModuleLoaded(&g_a, "p"));

        Registry::Reset();
        kptest::Check("reset clears the table", Registry::Require("life.svc", 1) == nullptr);
        kptest::Check("reset reopens", Registry::Provide("life.svc", 1, &g_c) == 0 &&
                                       Registry::Require("life.svc", 1) == &g_c);
        Registry::Close();
        Registry::Init();
        kptest::Check("init reopens after close", Registry::Require("life.svc", 1) == &g_c);
    }

    return kptest::Report();
}
