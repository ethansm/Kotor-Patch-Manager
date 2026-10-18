# Patch registry

This is how a patch author uses the patch registry (RFC #180). The registry lets a patch publish a small table of functions for other patches, and look up tables other patches published. The core may also publish its own services through the registry.

For how the patcher implements this, see the [KotorPatcher README](../src/KotorPatcher/README.md#patch-registry-and-kpatchinit-rfc-180). The contract itself is the header `Patches/Common/kpatch_api.h`; this page does not repeat everything in its comments.

A patch that does not export `KPatchInit` is unaffected by any of this.

## Including the header

The header is pure C (no `windows.h`, no C++), so it works in any patch on any target. The Windows build adds `Patches/Common` to the include path, but the Linux and macOS builds add nothing, so include it by a relative path, which works on all three:

```cpp
#include "../Common/kpatch_api.h"   // from Patches/<YourPatch>/Foo.cpp
```

A source in a subdirectory of the patch uses `../../Common/kpatch_api.h`.

## Declaring KPatchInit

The patcher calls an export named `KPatchInit` once, right after it loads your module (details under [Rules for KPatchInit](#rules-for-kpatchinit)). It must be exported like a hook function, and `create-patch` finds exports by scanning your `.cpp` files, so the declaration has to be written in a form the scan understands:

```cpp
extern "C" void __cdecl KPatchInit(const KPatchApi* api)
{
    // ...
}
```

`create-patch.py` (and `create-patch.bat`) build `exports.def` from every source line that matches the regular expression `extern.*__cdecl`. On each matching line they split on spaces and `(`, and take the **fifth** token as the exported name. For the line above the tokens are `extern`, `"C"`, `void`, `__cdecl`, `KPatchInit`, so the fifth is `KPatchInit`. What this means in practice:

- **A one-token return type.** `void`, `int`, `int32_t`, `DWORD` and `void*` work. `unsigned int` or `const char*` push the name to the sixth token, so the scan picks up `__cdecl` as the name and your function is not exported. `KPatchInit` returns `void`, so use `void`.
- **Write `__cdecl` literally.** The scan looks for the text `__cdecl`. `extern "C" void KPATCH_CALL KPatchInit(...)` is valid C++ but does not match, so nothing is exported on Windows. Use `KPATCH_CALL` inside the interface structs, where it belongs, and `__cdecl` on the exported function.
- **Keep it on one line**, with `extern`, `"C"`, the return type, `__cdecl` and the name before the `(`. A name on the next line is not found.
- **No `__declspec(dllexport)` in front.** It adds tokens before the return type, so the scan picks up `void` as the name. The `.def` file does the exporting.
- **Any matching line counts**, including a forward declaration or a comment that contains `extern` and `__cdecl`. A repeated name gives a repeated line in `exports.def`, so declare it once.
- **A committed `exports.def` is used as it is.** If your patch already has an `exports.def` (for example `AdditionalConsoleCommands/exports.def`), the scan is skipped. Add a `KPatchInit` line to it by hand.
- **Only `.cpp` files are compiled**, in the patch folder and its immediate subfolders. Write `KPatchInit` in C++ (it can still be `extern "C"`).

Linux and macOS builds take every non-static function with default visibility as an export and do not use `exports.def`, so `extern "C"` is enough there. But `__cdecl` is an MSVC keyword. A patch that builds for the native targets already defines it away, and yours should too:

```cpp
#include "../Common/kpatch_api.h"

#if !defined(_WIN32)
// i386 System V already calls a free function this way. The keyword is MSVC's.
#define __cdecl
#endif
```

## Providing an interface

An interface is a struct whose first field is `uint32_t struct_size`, followed by function pointers declared with `KPATCH_CALL`. Keep the table in static storage: it must stay valid until the patcher cleans up. This provider publishes `mypatch.counter`:

```cpp
#include <cstdint>
#include "../Common/kpatch_api.h"

#if !defined(_WIN32)
#define __cdecl
#endif

typedef struct MyCounter {
    uint32_t struct_size;                       // always first
    int32_t (KPATCH_CALL *Add)(int32_t x);
} MyCounter;

static int32_t KPATCH_CALL CounterAdd(int32_t x)
{
    return x + 1;
}

static const MyCounter g_counter = { sizeof(MyCounter), &CounterAdd };

extern "C" void __cdecl KPatchInit(const KPatchApi* api)
{
    // Returns 0 on success. Nonzero means "mypatch.counter" v1 already has a
    // provider (the first one wins) or the arguments were invalid.
    api->provide("mypatch.counter", 1, &g_counter);
}
```

Ship the struct definition in a header that your consumers can include. Interface structs are plain C types, so a consumer built by a different compiler reads the same layout (the header packs the core's structs to 8 bytes; do the same with `#pragma pack(push, 8)` around yours if your patch builds with a different packing).

## Requiring an interface

Another patch's module may not have been initialised yet when your `KPatchInit` runs, and the provider may not be installed at all. So do not `require` in `KPatchInit`. Store the `api` pointer there, ask for the interface the first time you need it, cache a successful result, and have a fallback for `NULL`:

```cpp
#include <cstdint>
#include "../Common/kpatch_api.h"
#include "MyCounter.h"                          // the provider's struct

#if !defined(_WIN32)
#define __cdecl
#endif

static const KPatchApi* g_api;
static const MyCounter* g_counter;              // cached after the first success

extern "C" void __cdecl KPatchInit(const KPatchApi* api)
{
    g_api = api;                                // store only
}

static int32_t AddOrFallback(int32_t x)
{
    if (!g_counter && g_api) {
        g_counter = (const MyCounter*)g_api->require("mypatch.counter", 1);
    }
    if (!g_counter) {
        return x;                               // provider not installed
    }
    return g_counter->Add(x);
}
```

Notes:

- `require` returns the provider's pointer, or `NULL` if no such interface exists, the major version differs, or the patcher is shutting down.
- While the result is `NULL` this retries on every call, which costs a lock and a map lookup. That is fine for an occasional call. On a hot path, retry every so often instead of every call. Do not cache "absent" for good: on the SteamStub deferred-apply path the patcher loads modules while the game is already running, so a provider can appear after your first hook fires.
- Never call another patch's interface from your DLL detach path. The provider may already be unloaded.

## Names, versions and growth

**Names.** Interface names are plain strings in one shared namespace, so prefix them with something that belongs to you: `"k2.guiinput"`, `"mypatch.counter"`. By convention the `kpatch.` prefix is for services the core provides.

**Versions.** The number you pass to `provide` and `require` is the **major** version. `require("mypatch.counter", 1)` returns only an interface provided as `("mypatch.counter", 1)`. If you must make a breaking change (remove or reorder a field, change a signature), provide a new major. You can provide `("x", 1)` and `("x", 2)` side by side, and old consumers keep working. A second `provide` of the same name and major is rejected; the first stays.

**Growth within a major.** Add new fields at the end of the struct and never remove, reorder or resize existing ones. `struct_size` is how a consumer tells which fields the provider has. Before using a field that is newer than the oldest provider you support, check that the provider's table reaches it:

```cpp
#include <cstddef>

// True if the provider's table is large enough to contain `field`.
#define KP_HAS_FIELD(p, S, field) \
    ((p)->struct_size >= offsetof(S, field) + sizeof(((S*)0)->field))

if (KP_HAS_FIELD(g_counter, MyCounter, Reset)) {   // Reset was added after v1.0
    g_counter->Reset();
} else {
    // older provider: do without
}
```

A consumer should also check `struct_size` against the end of the fields it relies on, not just the newer ones, and treat a smaller table as unusable.

## Rules for KPatchInit

`KPatchInit` is called:

- once per module, in config order (which is not a contract),
- right after the module loads and before the patcher looks up your hook function,
- even if a later hook of your patch fails to apply,
- only for modules the patcher loads, which means DETOUR and DLL-only patches. SIMPLE and REPLACE hooks load no module.

A patch that only provides or uses interfaces and needs no hooks is a DLL-only patch: a hooks file with a `[metadata]` section and no `[[hooks]]`, plus its sources.

It is called in the same situation as a `DllMain`: under the Windows loader lock, or on the worker thread of the SteamStub deferred apply, or in the library constructor on Linux and macOS. So:

- You may store the `api` pointer, call `provide` and `require`, and make cheap registration calls.
- You must not load libraries, wait on other threads, or do file or network I/O.

A patcher that predates the registry never calls `KPatchInit`. Keep your pointers null-checked at every use and have a fallback, so the patch still works there.

Interface pointers stay valid until the patcher starts cleanup, after which `require` returns `NULL`.
