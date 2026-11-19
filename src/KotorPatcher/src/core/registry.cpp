#include "registry.h"
#include "platform.h"

#include <atomic>
#include <cstdio>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>

namespace KotorPatcher {
namespace Registry {

    namespace {

        // Who is providing right now. KPatchInit calls Provide() with no argument
        // saying which patch it is, so the attribution rides along in a
        // thread_local that RunInit sets around the call. It points at the caller's
        // string, which outlives the call, so it needs no destructor (a
        // thread_local std::string would run one at thread exit, possibly after the
        // state it relates to is gone). Outside any init it is null, which reads as
        // "patcher": the core publishing its own services.
        thread_local const char* t_currentPatch = nullptr;

        const char* CurrentProvider() {
            return (t_currentPatch && *t_currentPatch) ? t_currentPatch : "patcher";
        }

        // Safe to print: a patch may hand us a null name.
        const char* Printable(const char* s) {
            return s ? s : "(null)";
        }

        struct Entry {
            const void* iface;
            std::string provider;
        };

        // Keyed by (name, major version). std::map keeps that order, which is
        // exactly the order DumpProvided has to print in.
        using Key = std::pair<std::string, uint32_t>;

        struct State {
            std::mutex mutex;
            std::map<Key, Entry> table;
            std::set<void*> seenHandles;
            std::atomic<bool> closed{false};
            KPatchApi api;
        };

        // The trampolines behind KPatchApi. They are free functions with the ABI's
        // calling convention, because the struct holds plain function pointers
        // (no this, no std::function) and a patch built by another compiler calls
        // them directly.
        int32_t KPATCH_CALL ApiProvide(const char* name, uint32_t version, const void* iface) {
            return Provide(name, version, iface);
        }

        const void* KPATCH_CALL ApiRequire(const char* name, uint32_t version) {
            return Require(name, version);
        }

        // Heap-allocated and intentionally never deleted. A function-local static
        // object would be destroyed at process exit, and by then a patch thread may
        // still call require/provide: on Linux exit() runs static destructors while
        // other threads are alive, and on Windows ExitProcess kills threads that may
        // be holding the mutex, so destroying the mutex (or freeing the table under
        // a running reader) is the worse failure. A one-time leak of a few hundred
        // bytes at process end is harmless. The function-local static pointer makes
        // construction thread-safe (C++11 magic statics) with no #ifdef.
        State& S() {
            static State* state = [] {
                State* s = new State();
                s->api.struct_size = sizeof(KPatchApi);
                s->api.abi_version = KPATCH_ABI_VERSION;
                s->api.provide = &ApiProvide;
                s->api.require = &ApiRequire;
                return s;
            }();
            return *state;
        }

        // The one place KPatchInit is invoked, shared by OnModuleLoaded and the test
        // seam so the tests exercise the real attribution and logging path. The
        // previous attribution is restored rather than cleared so an init that
        // somehow triggers another one still attributes correctly afterwards.
        void RunInit(const char* patchId, KPatchInitFn fn) {
            char msg[256];
            std::snprintf(msg, sizeof(msg), "[KotorPatcher] Registry: KPatchInit called for '%s'\n",
                          Printable(patchId));
            Platform::Log(msg);

            const char* previous = t_currentPatch;
            t_currentPatch = patchId;
            fn(GetApi());
            t_currentPatch = previous;
        }

    } // namespace

    void Init() {
        S().closed.store(false);
    }

    void Close() {
        State& s = S();
        std::lock_guard<std::mutex> lock(s.mutex);
        s.closed.store(true);
    }

    void Reset() {
        State& s = S();
        {
            std::lock_guard<std::mutex> lock(s.mutex);
            s.table.clear();
            s.seenHandles.clear();
        }
        s.closed.store(false);
    }

    int32_t Provide(const char* name, uint32_t version, const void* iface) {
        State& s = S();
        char msg[512];
        const std::string provider = CurrentProvider();

        if (!name || !*name || !iface) {
            std::snprintf(msg, sizeof(msg),
                          "[KotorPatcher] Registry: invalid provide of '%s' v%u by '%s' rejected "
                          "(name and interface are required)\n",
                          Printable(name), version, provider.c_str());
            Platform::Log(msg);
            return kProvideInvalid;
        }

        // The closed check is made under the lock so a Provide racing Close() either
        // lands before it or is rejected, never inserted after Close returned.
        std::string firstProvider;
        bool duplicate = false;
        bool closed = false;
        {
            std::lock_guard<std::mutex> lock(s.mutex);
            if (s.closed.load()) {
                closed = true;
            } else {
                auto result = s.table.emplace(Key(name, version), Entry{iface, provider});
                if (!result.second) {
                    duplicate = true;
                    firstProvider = result.first->second.provider;
                }
            }
        }

        if (closed) {
            std::snprintf(msg, sizeof(msg),
                          "[KotorPatcher] Registry: provide of '%s' v%u by '%s' rejected "
                          "(registry closed)\n",
                          name, version, provider.c_str());
            Platform::Log(msg);
            return kProvideClosed;
        }

        // Logged after the lock is released: Platform::Log is a write to stderr or
        // the debugger and has no business inside the registry's critical section.
        if (duplicate) {
            std::snprintf(msg, sizeof(msg),
                          "[KotorPatcher] Registry: duplicate provide of '%s' v%u by '%s' rejected "
                          "(provided by '%s')\n",
                          name, version, provider.c_str(), firstProvider.c_str());
            Platform::Log(msg);
            return kProvideDuplicate;
        }
        return kProvideOk;
    }

    const void* Require(const char* name, uint32_t version) {
        State& s = S();
        if (!name || !*name || s.closed.load()) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(s.mutex);
        auto it = s.table.find(Key(name, version));
        return it == s.table.end() ? nullptr : it->second.iface;
    }

    bool RunInitForTest(const char* patchId, KPatchInitFn fn) {
        if (!fn) {
            return false;
        }
        RunInit(patchId, fn);
        return true;
    }

    bool OnModuleLoaded(void* handle, const char* patchId) {
        State& s = S();
        if (!handle || s.closed.load()) {
            return false;
        }

        // ApplyPatch loads a module once per hook entry, so a 20-hook DLL arrives
        // here 20 times with the same handle. The handle is recorded whether or
        // not it has KPatchInit, so a module without one is also looked up once.
        {
            std::lock_guard<std::mutex> lock(s.mutex);
            if (!s.seenHandles.insert(handle).second) {
                return false;
            }
        }

        // A patch with no KPatchInit is the normal case today, so its absence is
        // not logged.
        void* symbol = Platform::GetSymbol(handle, "KPatchInit");
        if (!symbol) {
            return false;
        }

        RunInit(patchId, reinterpret_cast<KPatchInitFn>(symbol));
        return true;
    }

    const KPatchApi* GetApi() {
        return &S().api;
    }

    void DumpProvided(std::string& out) {
        State& s = S();
        std::lock_guard<std::mutex> lock(s.mutex);
        for (const auto& item : s.table) {
            out += "  ";
            out += item.first.first;
            out += " v";
            out += std::to_string(item.first.second);
            out += " by ";
            out += item.second.provider;
            out += "\n";
        }
    }

} // namespace Registry
} // namespace KotorPatcher
