#pragma once
#include <cstdint>
#include <string>

#include "kpatch_api.h"

// The patch registry (RFC #180): named, versioned plain-C interfaces that the core
// and the patch modules publish to each other, plus the optional per-patch
// `KPatchInit` export through which a patch gets the registry.
//
// The core is just another provider: it publishes "kpatch.log" through the same
// Provide() path a patch uses, attributed to "patcher".
//
// Threading: every call is safe from any thread. KPatchInit may run under the
// Windows loader lock or on the SteamStub deferred-apply worker, so nothing here
// loads libraries, waits on other threads, or touches files.

namespace KotorPatcher {
namespace Registry {

    // Provide() results. The ABI only promises "0 = accepted, nonzero = rejected";
    // the distinct codes are for the core's own logs and tests.
    constexpr int32_t kProvideOk        = 0;
    constexpr int32_t kProvideInvalid   = -1;  // null/empty name or null interface
    constexpr int32_t kProvideDuplicate = -2;  // name+version already provided
    constexpr int32_t kProvideClosed    = -3;  // Close() was called

    // Create the registry state (once) and open it for provide/require.
    void Init();

    // Begin shutdown: Require() returns null and Provide() is rejected from here
    // on, so no patch can reach an interface whose provider is being unloaded.
    // State is kept (not freed) so a late caller never touches freed memory.
    void Close();

    // Forget every provided interface and every module handle seen, and reopen.
    // Needed after the modules are unloaded because the OS may hand the same
    // handle value to a different module later, which must get its own init.
    void Reset();

    // Publish `iface` under (name, version). The provider is the patch whose
    // KPatchInit is running on this thread, or "patcher" outside one. The first
    // provider of a name+version wins; a duplicate is rejected and logged.
    int32_t Provide(const char* name, uint32_t version, const void* iface);

    // The interface provided under (name, version), or nullptr.
    const void* Require(const char* name, uint32_t version);

    // Call the module's "KPatchInit" export, once per distinct handle. Returns
    // true if it was called; false if the handle was already seen, the module has
    // no KPatchInit (not an error, silent), the registry is closed, or an
    // argument is null. The registry mutex is NOT held during the call because
    // patches re-enter Provide/Require from inside it.
    bool OnModuleLoaded(void* handle, const char* patchId);

    // Test seam: run `fn` exactly as OnModuleLoaded runs a module's KPatchInit
    // (same attribution, same logging), without needing a real module.
    // Returns false if fn is null.
    bool RunInitForTest(const char* patchId, KPatchInitFn fn);

    // The registry struct handed to every KPatchInit. Valid for the process
    // lifetime.
    const KPatchApi* GetApi();

    // Append one line per provided interface, "  <name> v<ver> by <provider>\n",
    // sorted by name then version. For the session log header and diagnostics.
    void DumpProvided(std::string& out);

} // namespace Registry
} // namespace KotorPatcher
