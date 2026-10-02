#ifndef KPATCH_API_H
#define KPATCH_API_H

/*
 * kpatch_api.h -- the plain-C ABI between KotorPatcher and the patch modules.
 *
 * This is the canonical copy. It is deliberately pure C (compiles as C11 and
 * C++17, needs only <stdint.h>/<stddef.h>, no windows.h) because MSVC patches,
 * MinGW patches and the core all share one process and cannot share a C++ ABI:
 * no std::string, no exceptions, no bool/enum/double across the boundary, only
 * fixed-width integers, const char* and function pointers.
 *
 * Mechanism (RFC LaneDibello/Kotor-Patch-Manager#180)
 * ---------------------------------------------------
 * After the patcher loads a patch module it looks for an optional export
 *
 *     void __cdecl KPatchInit(const KPatchApi* api);      (export name "KPatchInit")
 *
 * and, if present, calls it once per distinct module handle. A patch without
 * the export is untouched. Through `api` a patch can `provide` a named
 * interface (a table of function pointers) for other patches, and `require` an
 * interface another patch (or the core, e.g. "kpatch.log") provided.
 *
 * KPatchInit contract
 * -------------------
 * - It is DllMain-class code. On Windows it runs under the loader lock, and it
 *   may run on a worker thread (the SteamStub deferred-apply path). It may store
 *   the api pointer and call api->provide, api->require and the cheap
 *   registration calls of an interface (e.g. KPatchLogApi::Register/Channel).
 *   It must NOT load libraries, wait on other threads, or do file I/O.
 * - Init order between patches is config order and is not a contract. A patch
 *   that needs another patch's interface should `require` it lazily at first
 *   use, not in KPatchInit, and must handle a null result (the provider may not
 *   be installed).
 * - KPatchInit is called at load time, even if a later hook of that patch fails.
 *
 * Lifetime
 * --------
 * - The api pointer and every interface pointer returned by `require` stay
 *   valid until the patcher begins cleanup. After that `require` returns NULL.
 * - A patch must never call another patch's interface from its DLL detach path:
 *   the provider may already be unloaded.
 *
 * ABI rules
 * ---------
 * - Interface names are strings; the version number is the MAJOR version.
 *   `require("x", 1)` only matches an interface provided as ("x", 1); a second
 *   major may be provided side by side.
 * - Within a major, structs only grow: new fields are appended, never removed,
 *   reordered or resized. The first field of every interface is
 *   `uint32_t struct_size`; a consumer must check
 *   `struct_size >= offsetof(S, field) + sizeof(((S*)0)->field)` before reading
 *   a field newer than its oldest supported provider.
 * - The first provider of a (name, major) wins; a duplicate is rejected.
 * - Structs are packed to 8 here regardless of the including file's packing
 *   (Common.h opens pack(push, 4)), so every compiler agrees on the layout.
 */

#include <stddef.h>
#include <stdint.h>

/* The calling convention of every function pointer in this ABI. x86-32 Windows
 * compilers differ in defaults (and a patch may build with __stdcall as its
 * default), so it is pinned to cdecl. Elsewhere (x86_64, ARM) there is one
 * convention and the macro is empty. */
#if defined(_MSC_VER) || defined(__MINGW32__)
#define KPATCH_CALL __cdecl
#elif defined(__i386__)
#define KPATCH_CALL __attribute__((cdecl))
#else
#define KPATCH_CALL
#endif

#define KPATCH_ABI_VERSION 1u

/* Log levels (plain defines so no enum crosses the boundary). */
#define KPLOG_ERR   0
#define KPLOG_WARN  1
#define KPLOG_INFO  2
#define KPLOG_DEBUG 3
#define KPLOG_TRACE 4

/* The logging service the core provides through the registry. */
#define KPATCH_LOG_IFACE   "kpatch.log"
#define KPATCH_LOG_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

/* The registry handed to KPatchInit. */
typedef struct KPatchApi {
    uint32_t struct_size;   /* sizeof(KPatchApi) as built into the patcher */
    uint32_t abi_version;   /* KPATCH_ABI_VERSION */

    /* Publish `iface` under (name, version). Returns 0 on success, nonzero if
     * rejected (null/empty name, null iface, duplicate name+version, registry
     * closed). `iface` must stay valid until the patcher cleans up. */
    int32_t (KPATCH_CALL *provide)(const char* name, uint32_t version, const void* iface);

    /* Look up (name, version). Returns the provider's pointer, or NULL if no
     * such interface exists (or the registry is closed). Cheap; callers
     * normally cache the result. */
    const void* (KPATCH_CALL *require)(const char* name, uint32_t version);
} KPatchApi;

/* The "kpatch.log" v1 interface. Fields are only ever appended. */
typedef struct KPatchLogApi {
    uint32_t struct_size;

    /* Idempotent. Returns a patch handle, or -1 on bad input. */
    int32_t (KPATCH_CALL *Register)(const char* patchId);

    /* Returns a channel handle for (patch, name), or -1 (bad input or the
     * 256-channel limit). */
    int32_t (KPATCH_CALL *Channel)(int32_t patch, const char* name);

    /* Nonzero if (channel, level) would be recorded. Cheap; call before
     * formatting. */
    int32_t (KPATCH_CALL *Enabled)(int32_t channel, int32_t level);

    /* Queue one line (`len` bytes, no trailing newline needed). Non-blocking;
     * the line is copied. */
    void (KPATCH_CALL *Submit)(int32_t channel, int32_t level, const char* line, uint32_t len);

    /* Write a marker line to every open log file. */
    void (KPATCH_CALL *Mark)(const char* label);

    /* Advance the frame counter. The first patch to call it owns the counter;
     * calls from other patches are ignored. */
    void (KPATCH_CALL *FrameTick)(int32_t patch);

    /* Current frame number. */
    uint32_t (KPATCH_CALL *Frame)(void);

    /* Monotonic microseconds since the service started. */
    uint64_t (KPATCH_CALL *NowUs)(void);
} KPatchLogApi;

#pragma pack(pop)

/* Signature of the optional per-patch export named "KPatchInit". */
typedef void (KPATCH_CALL *KPatchInitFn)(const KPatchApi* api);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Layout checks: a mismatch here means two compilers would disagree about the
 * structs. Offsets are in pointer-size units because the structs open with a
 * uint32_t and then hold pointers, so 64-bit hosts (the tests) pad to 8. */
#if defined(__cplusplus)
#define KPATCH_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define KPATCH_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef KPATCH_STATIC_ASSERT
KPATCH_STATIC_ASSERT(offsetof(KPatchApi, provide) == 8, "KPatchApi.provide offset");
KPATCH_STATIC_ASSERT(offsetof(KPatchApi, require) == 8 + sizeof(void*), "KPatchApi.require offset");
KPATCH_STATIC_ASSERT(sizeof(KPatchApi) == 8 + 2 * sizeof(void*), "KPatchApi size");

KPATCH_STATIC_ASSERT(offsetof(KPatchLogApi, Register) == sizeof(void*), "KPatchLogApi.Register offset");
KPATCH_STATIC_ASSERT(offsetof(KPatchLogApi, NowUs) == 8 * sizeof(void*), "KPatchLogApi.NowUs offset");
KPATCH_STATIC_ASSERT(sizeof(KPatchLogApi) == 9 * sizeof(void*), "KPatchLogApi size");
#undef KPATCH_STATIC_ASSERT
#endif

#endif /* KPATCH_API_H */
