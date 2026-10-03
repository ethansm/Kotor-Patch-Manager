// Host test for the module side of the patch registry (RFC #180): real shared
// objects are loaded through Platform::LoadModule and handed to
// Registry::OnModuleLoaded, so the KPatchInit lookup, init-once-per-handle, lazy
// require and shutdown paths run against plain-C patch code (tests/fixtures/*.c,
// built by `make test` for the width under test into FIXTURE_DIR).
//
// Every row starts with Registry::Reset() and unloads what it loaded, so a row's
// fixtures are fresh module instances: dlopen of a path that is still loaded
// returns the same handle with its old statics, which would hide a missing init.
// The init counts asserted below double as proof that the unload really happened.
#include "registry.h"
#include "platform.h"
#include "check.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <unistd.h>

using namespace KotorPatcher;

#ifndef FIXTURE_DIR
#error "FIXTURE_DIR must point at the directory holding the built fixture .so files"
#endif

namespace {

    typedef int (*IntFn)(void);
    typedef int (*IntArgFn)(int32_t);

    // Every successful LoadModule is recorded so a row can release them all: the
    // module is refcounted, so each load needs its own unload before it is gone.
    std::vector<void*> g_loaded;

    std::string Fixture(const char* name) {
        return std::string(FIXTURE_DIR) + "/" + name + ".so";
    }

    void* Load(const std::string& path) {
        void* handle = Platform::LoadModule(path.c_str());
        if (handle) g_loaded.push_back(handle);
        return handle;
    }

    void UnloadAll() {
        for (void* handle : g_loaded) Platform::UnloadModule(handle);
        g_loaded.clear();
    }

    template <typename Fn>
    Fn Sym(void* handle, const char* name) {
        return reinterpret_cast<Fn>(Platform::GetSymbol(handle, name));
    }

    int InitCount(void* handle) {
        IntFn fn = Sym<IntFn>(handle, "fixture_init_count");
        return fn ? fn() : -1;
    }

    // Run `fn` with fd 2 redirected to a temp file and return what it wrote
    // (same approach as any_registry).
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

    // A byte-for-byte copy under another name. dlopen identifies a library by
    // file identity, so the copy is a distinct module instance with its own
    // statics even while the original is still loaded.
    bool CopyFile(const std::string& from, const std::string& to) {
        std::ifstream in(from, std::ios::binary);
        std::ofstream out(to, std::ios::binary | std::ios::trunc);
        if (!in || !out) return false;
        out << in.rdbuf();
        return static_cast<bool>(out);
    }

    // The same .so is loaded once per hook entry by ApplyPatch, so a 20-hook DLL
    // arrives at OnModuleLoaded 20 times with one handle.
    void SameModuleTwentyTimes() {
        Registry::Reset();
        const std::string path = Fixture("provider");

        void* first = Load(path);
        bool allSame = first != nullptr;
        int inited = 0;
        for (int i = 0; i < 20; ++i) {
            void* handle = i == 0 ? first : Load(path);
            allSame = allSame && handle == first;
            if (Registry::OnModuleLoaded(handle, "twenty")) ++inited;
        }
        kptest::Check("twenty loads return one handle", allSame);
        kptest::Check("OnModuleLoaded true once, false 19 times", inited == 1);
        kptest::Check("KPatchInit ran exactly once", InitCount(first) == 1);
        UnloadAll();
    }

    // The documented pattern: init order is config order and not a contract, so a
    // consumer inited before its provider must still find it at first use.
    void LazyRequire() {
        Registry::Reset();
        void* consumer = Load(Fixture("consumer"));
        void* provider = Load(Fixture("provider"));
        kptest::Check("both fixtures load", consumer && provider);
        if (!consumer || !provider) { UnloadAll(); return; }

        kptest::Check("consumer inited first",
                      Registry::OnModuleLoaded(consumer, "consumer_patch"));
        kptest::Check("provider inited second",
                      Registry::OnModuleLoaded(provider, "provider_patch"));

        IntArgFn call = Sym<IntArgFn>(consumer, "consumer_call");
        IntFn fell = Sym<IntFn>(consumer, "consumer_used_fallback");
        kptest::Check("first call goes through the provider", call && call(41) == 42);
        kptest::Check("no fallback taken", fell && fell() == 0);
        UnloadAll();
    }

    // The provider may simply not be installed. The consumer runs from a copy so
    // it is a fresh instance (no cached interface) whatever happened to the
    // original in earlier rows.
    void ConsumerWithoutProvider() {
        Registry::Reset();
        const std::string copy = Fixture("consumer_copy");
        kptest::Check("fixture copied", CopyFile(Fixture("consumer"), copy));

        void* consumer = Load(copy);
        kptest::Check("copy loads", consumer != nullptr);
        if (!consumer) { UnloadAll(); return; }

        Registry::OnModuleLoaded(consumer, "lonely");
        IntArgFn call = Sym<IntArgFn>(consumer, "consumer_call");
        IntFn fell = Sym<IntFn>(consumer, "consumer_used_fallback");
        kptest::Check("call returns the fallback value", call && call(5) == -1005);
        kptest::Check("fallback flag set", fell && fell() == 1);
        UnloadAll();
        std::remove(copy.c_str());
    }

    // Every patch built before the registry has no KPatchInit; that must be silent.
    void ModuleWithoutInit() {
        Registry::Reset();
        void* noinit = Load(Fixture("noinit"));
        kptest::Check("fixture loads", noinit != nullptr);
        if (!noinit) { UnloadAll(); return; }

        bool called = true;
        std::string log = CaptureStderr([&] { called = Registry::OnModuleLoaded(noinit, "legacy"); });
        kptest::Check("OnModuleLoaded returns false", !called);
        kptest::Check("nothing logged", log.empty(), log.c_str());
        UnloadAll();
    }

    // The attribution is what makes a duplicate-provider log line actionable.
    void ProviderAttribution() {
        Registry::Reset();
        void* provider = Load(Fixture("provider"));
        kptest::Check("fixture loads", provider != nullptr);
        if (!provider) { UnloadAll(); return; }

        Registry::OnModuleLoaded(provider, "attribution_patch");
        std::string dump;
        Registry::DumpProvided(dump);
        const bool named = Contains(dump, "test.counter v1 by attribution_patch");
        kptest::Check("dump names the patch id", named, named ? "" : dump.c_str());
        UnloadAll();
    }

    // Reset exists because a handle value can be recycled by a different module
    // after unload; the fresh instance must be inited again.
    void ReinitAfterReset() {
        Registry::Reset();
        const std::string path = Fixture("provider");
        void* first = Load(path);
        kptest::Check("first init", first && Registry::OnModuleLoaded(first, "reload") &&
                                    InitCount(first) == 1);
        kptest::Check("same handle is not re-inited", first && !Registry::OnModuleLoaded(first, "reload"));

        // The module is refcounted: release every load so it is really unloaded.
        UnloadAll();
        Registry::Reset();

        void* second = Load(path);
        kptest::Check("reload inits again", second && Registry::OnModuleLoaded(second, "reload"));
        // 1 means the unload really dropped the module and its statics. 2 would mean
        // the loader kept the old instance alive (the OnModuleLoaded check above
        // still proves the registry side).
        kptest::Check("fresh instance init count is 1", second && InitCount(second) == 1);
        UnloadAll();
    }

    // Shutdown can interrupt module loading. Whatever was inited stays reachable
    // until Close() (the patch may still be running), a module never reached
    // provided nothing, and after Close nothing is reachable.
    void AbortMidway() {
        Registry::Reset();
        void* a = Load(Fixture("provider"));
        kptest::Check("module A loads", a != nullptr);
        if (!a) { UnloadAll(); return; }
        Registry::OnModuleLoaded(a, "module_a");
        // Module B would provide "test.later"; the loader is aborted before it loads.

        const KPatchApi* api = Registry::GetApi();
        const void* counter = api->require("test.counter", 1);
        kptest::Check("A's interface is served", counter != nullptr);
        kptest::Check("B's interface is null", api->require("test.later", 1) == nullptr);

        Registry::Close();
        kptest::Check("after Close A's interface is null", api->require("test.counter", 1) == nullptr);
        void* late = Load(Fixture("consumer"));
        kptest::Check("a module loaded after Close is not inited",
                      late && !Registry::OnModuleLoaded(late, "late") && InitCount(late) == 0);

        // Leave the registry open for whatever runs next.
        Registry::Reset();
        UnloadAll();
    }

} // namespace

int main() {
    std::printf("  module init, %zu-bit\n", sizeof(void*) * 8);

    Registry::Init();
    SameModuleTwentyTimes();
    LazyRequire();
    ConsumerWithoutProvider();
    ModuleWithoutInit();
    ProviderAttribution();
    ReinitAfterReset();
    AbortMidway();
    return kptest::Report();
}
