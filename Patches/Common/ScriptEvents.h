/* ScriptEvents.h -- the "k2.scriptevents" v1 registry interface + C++ consumer helper.
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width types, function pointers only,
 * KPATCH_CALL (__cdecl), structs packed to 8, structs only grow.
 *
 * Provider: script-events (the project workspace)
 * Owns non-colliding NWScript VM interception points:
 *   - CVirtualMachineInternal::RunScript (0x7022E0 pre, 0x7023E6 post)
 *   - CSWVirtualMachineCommands::ExecuteCommand (0x668FE8 pre-dispatch, before the command table lookup)
 * Registration (the Register and Unregister functions) is main-thread only; the dispatcher takes no locks.
 */
#ifndef K2_SCRIPTEVENTS_H
#define K2_SCRIPTEVENTS_H

#if __has_include("kpatch_api.h")
#include "kpatch_api.h"
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef KPATCH_CALL
#if defined(_MSC_VER) || defined(__MINGW32__)
#define KPATCH_CALL __cdecl
#elif defined(__i386__)
#define KPATCH_CALL __attribute__((cdecl))
#else
#define KPATCH_CALL
#endif
#endif

#ifndef KPATCH_API_H
#define KPATCH_API_H
#pragma pack(push, 8)
typedef struct KPatchApi {
    uint32_t struct_size;
    uint32_t abi_version;
    int32_t (KPATCH_CALL *provide)(const char* name, uint32_t version, const void* iface);
    const void* (KPATCH_CALL *require)(const char* name, uint32_t version);
} KPatchApi;
#pragma pack(pop)
#endif

#define K2_SCRIPTEVENTS_IFACE   "k2.scriptevents"
#define K2_SCRIPTEVENTS_VERSION 1

/* Script event kinds (bitmask when registering, single bit in K2ScriptEvent.kind) */
#define K2SC_SCRIPT_PRE   1 /* Fired at CVirtualMachineInternal::RunScript (0x7022E0) before execution */
#define K2SC_SCRIPT_POST  2 /* Fired at CVirtualMachineInternal::RunScript (0x7023E6) after RunScriptFile */
#define K2SC_SCRIPT_ALL   3

/* Handler return codes for K2SC_SCRIPT_PRE and K2VmCommandCallback */
#define K2SC_CONTINUE     0 /* Observe only; let normal execution proceed */
#define K2SC_HANDLED      1 /* Short-circuit engine execution and return out_return_value / cmd_status */

/* Wildcard command ID for RegisterCommandHandler */
#define K2SC_CMD_ALL     (-1)

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2ScriptEvent {
    uint32_t struct_size;
    int32_t  kind;               /* K2SC_SCRIPT_PRE or K2SC_SCRIPT_POST */
    const char* script_name;     /* Lowercase/raw resref from CExoString (never NULL) */
    uint32_t object_self;        /* Object ID passed to RunScript */
    int32_t  object_self_valid;  /* bObjectSelfValid passed to RunScript */
    int32_t  run_status;         /* For K2SC_SCRIPT_POST: RunScriptFile return code (0 = ok) */
} K2ScriptEvent;

/* For K2SC_SCRIPT_PRE: if callback returns K2SC_HANDLED (1), the engine skips .ncs execution,
 * returns 1 from RunScript exactly like a normal run, and skips the init at 0x7022E9-0x702310
 * and the K2SC_SCRIPT_POST event (POST sits after the skipped RunScriptFile).
 * The return slots are written only when *out_has_return_value != 0 (for StartingConditional
 * scripts): CVirtualMachineInternal +0x1C = 3, +0x20 = *out_return_value. Otherwise they are untouched.
 * For K2SC_SCRIPT_POST: return value is ignored (always return K2SC_CONTINUE). */
typedef int32_t (KPATCH_CALL *K2ScriptCallback)(
    const K2ScriptEvent* ev,
    int32_t* out_has_return_value,
    int32_t* out_return_value,
    void* user);

typedef struct K2VmCommandEvent {
    uint32_t struct_size;
    int32_t  cmd_id;             /* NWScript routine ID (0..0x36C, or custom ID below 0x36D) */
    int32_t  param_count;        /* Number of parameters pushed on VM stack */
    void*    cmd_processor;      /* CSWVirtualMachineCommands* this pointer */
    uint32_t caller_object_id;   /* OBJECT_SELF, read at cmd_processor + 0x08 (+0x04 is bObjectSelfValid) */
} K2VmCommandEvent;

/* Fired at CSWVirtualMachineCommands::ExecuteCommand (0x668FE8), before the command table lookup.
 * Command ids >= 0x36D never reach this hook (JGE at 0x668FE0, JAE after robustness-fixes; the
 * unsigned compare also excludes negative ids). A null-slot id < 0x36D can be handled here.
 * Return K2SC_CONTINUE (0) to let vanilla ExecuteCommand run.
 * Return K2SC_HANDLED (1) to bypass the vanilla command table entry and return *out_cmd_status
 * (default 0 = VM_SUCCESS) from ExecuteCommand. The provider stores *out_cmd_status in the frame
 * slot [EBP-0x04]; the hook then exits through 0x66900D (MOV EAX,[EBP-0x04]). */
typedef int32_t (KPATCH_CALL *K2VmCommandCallback)(
    const K2VmCommandEvent* ev,
    int32_t* out_cmd_status,
    void* user);

typedef struct K2ScriptEventsApi {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *RegisterScriptListener)(int32_t kind_mask, const char* script_filter, K2ScriptCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *UnregisterScriptListener)(int32_t token);
    int32_t (KPATCH_CALL *RegisterCommandHandler)(int32_t cmd_id, K2VmCommandCallback fn, void* user, const char* owner);
    void    (KPATCH_CALL *UnregisterCommandHandler)(int32_t token);
} K2ScriptEventsApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
static_assert(sizeof(K2ScriptEventsApi) == 5 * sizeof(void*), "K2ScriptEventsApi size must be 5 pointers");
static_assert(offsetof(K2ScriptEventsApi, RegisterScriptListener) == sizeof(void*), "RegisterScriptListener offset");
#endif

#ifdef __cplusplus

namespace k2sc {

constexpr int MaxScriptRegs = 16;
constexpr int MaxCmdRegs = 16;

inline bool scriptEventHas(const K2ScriptEvent* ev, size_t fieldEnd) { return ev && ev->struct_size >= fieldEnd; }
#define K2SC_SCRIPT_EVENT_HAS(ev, field) ::k2sc::scriptEventHas((ev), offsetof(K2ScriptEvent, field) + sizeof((ev)->field))

inline bool vmCommandEventHas(const K2VmCommandEvent* ev, size_t fieldEnd) { return ev && ev->struct_size >= fieldEnd; }
#define K2SC_CMD_EVENT_HAS(ev, field) ::k2sc::vmCommandEventHas((ev), offsetof(K2VmCommandEvent, field) + sizeof((ev)->field))

struct Client {
    struct ScriptReg {
        int32_t mask;
        char filter[32];
        K2ScriptCallback fn;
        void* user;
        int32_t token;
    };

    struct CmdReg {
        int32_t cmdId;
        K2VmCommandCallback fn;
        void* user;
        int32_t token;
    };

    const KPatchApi* api = nullptr;
    const K2ScriptEventsApi* sc = nullptr;
    const char* owner = "";

    ScriptReg scriptRegs[MaxScriptRegs] = {};
    int nScriptRegs = 0;

    CmdReg cmdRegs[MaxCmdRegs] = {};
    int nCmdRegs = 0;

    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) {
        api = a;
        owner = id ? id : "";
    }

    bool addScriptListener(int32_t kindMask, const char* scriptFilter, K2ScriptCallback fn, void* user = nullptr) {
        if (nScriptRegs >= MaxScriptRegs || !fn || (kindMask & K2SC_SCRIPT_ALL) == 0) return false;
        auto& r = scriptRegs[nScriptRegs++];
        r.mask = kindMask;
        r.filter[0] = 0;
        if (scriptFilter) {
            size_t len = 0;
            while (scriptFilter[len] && len < sizeof(r.filter) - 1) {
                r.filter[len] = scriptFilter[len];
                ++len;
            }
            r.filter[len] = 0;
        }
        r.fn = fn;
        r.user = user;
        r.token = 0;
        if (connected && sc) {
            r.token = sc->RegisterScriptListener(r.mask, r.filter[0] ? r.filter : nullptr, r.fn, r.user, owner);
        }
        return true;
    }

    bool addCommandHandler(int32_t cmdId, K2VmCommandCallback fn, void* user = nullptr) {
        if (nCmdRegs >= MaxCmdRegs || !fn) return false;
        auto& r = cmdRegs[nCmdRegs++];
        r.cmdId = cmdId;
        r.fn = fn;
        r.user = user;
        r.token = 0;
        if (connected && sc) {
            r.token = sc->RegisterCommandHandler(r.cmdId, r.fn, r.user, owner);
        }
        return true;
    }

    static bool usable(const K2ScriptEventsApi* p) {
        return p && p->struct_size >= offsetof(K2ScriptEventsApi, UnregisterCommandHandler) + sizeof(p->UnregisterCommandHandler)
                 && p->RegisterScriptListener && p->UnregisterScriptListener
                 && p->RegisterCommandHandler && p->UnregisterCommandHandler;
    }

    bool connect() {
        if (connected) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi");
            reportedMissing = true;
            return false;
        }
        const K2ScriptEventsApi* p = static_cast<const K2ScriptEventsApi*>(api->require(K2_SCRIPTEVENTS_IFACE, K2_SCRIPTEVENTS_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.scriptevents v1 table too small" : "k2.scriptevents v1 not provided");
            reportedMissing = true;
            return false;
        }
        sc = p;
        int ok = 0;
        for (int i = 0; i < nScriptRegs; ++i) {
            scriptRegs[i].token = sc->RegisterScriptListener(scriptRegs[i].mask, scriptRegs[i].filter[0] ? scriptRegs[i].filter : nullptr, scriptRegs[i].fn, scriptRegs[i].user, owner);
            if (scriptRegs[i].token > 0) ++ok;
        }
        for (int i = 0; i < nCmdRegs; ++i) {
            cmdRegs[i].token = sc->RegisterCommandHandler(cmdRegs[i].cmdId, cmdRegs[i].fn, cmdRegs[i].user, owner);
            if (cmdRegs[i].token > 0) ++ok;
        }
        connected = true;
        if (onConnect) onConnect(ok == (nScriptRegs + nCmdRegs), ok == (nScriptRegs + nCmdRegs) ? "registered" : "some registrations refused");
        return true;
    }

    void poll() {
        if (!connected && (pollCount++ & 255) == 0) connect();
    }

    void release() {
        if (!connected || !sc) return;
        for (int i = 0; i < nScriptRegs; ++i) {
            if (scriptRegs[i].token > 0) sc->UnregisterScriptListener(scriptRegs[i].token);
            scriptRegs[i].token = 0;
        }
        for (int i = 0; i < nCmdRegs; ++i) {
            if (cmdRegs[i].token > 0) sc->UnregisterCommandHandler(cmdRegs[i].token);
            cmdRegs[i].token = 0;
        }
        sc = nullptr;
        connected = false;
    }

    int32_t registerScriptListener(int32_t kindMask, const char* scriptFilter, K2ScriptCallback fn, void* user = nullptr) const {
        return (connected && sc) ? sc->RegisterScriptListener(kindMask, scriptFilter, fn, user, owner) : 0;
    }

    void unregisterScriptListener(int32_t token) const {
        if (connected && sc && token > 0) sc->UnregisterScriptListener(token);
    }

    int32_t registerCommandHandler(int32_t cmdId, K2VmCommandCallback fn, void* user = nullptr) const {
        return (connected && sc) ? sc->RegisterCommandHandler(cmdId, fn, user, owner) : 0;
    }

    void unregisterCommandHandler(int32_t token) const {
        if (connected && sc && token > 0) sc->UnregisterCommandHandler(token);
    }
};

} // namespace k2sc

#endif /* __cplusplus */

#endif /* K2_SCRIPTEVENTS_H */
