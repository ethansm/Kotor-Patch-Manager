#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "SafeRead.h"
#include "ScriptEvents.h"
#include "ScriptEventsCore.h"

KP_LOG_PATCH("script-events");

namespace {

KP_CHANNEL(chCore, "core");
KP_CHANNEL(chScript, "script");
KP_CHANNEL(chCmd, "cmd");

const bool g_fallbackSet = (KpLog_SetFallbackFile("script-events_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

enum Site : unsigned {
    S_RunPre  = 0x7501,
    S_RunPost = 0x7502,
    S_Cmd     = 0x7503
};

constexpr int OffVmInternal = -0x0C;
constexpr int OffExoScript  = 0x08;
constexpr int OffObjSelf    = 0x0C;
constexpr int OffObjValid   = 0x10;
constexpr int OffCmdProc    = -0x08;
constexpr int OffCmdId      = 0x08;
constexpr int OffParamCount = 0x0C;
constexpr int OffCmdStatus  = -0x04;

scriptevents::Dispatcher g_dispatcher;

int32_t KPATCH_CALL ApiRegisterScriptListener(int32_t kind_mask, const char* script_filter, K2ScriptCallback fn, void* user, const char* owner) {
    int32_t t = g_dispatcher.registerScriptListener(kind_mask, script_filter, fn, user, owner);
    KP_LOGV(chCore, t > 0 ? KPLOG_INFO : KPLOG_WARN, "register script listener mask=%d filter='%s' owner='%s' -> token=%d",
            kind_mask, script_filter ? script_filter : "*", owner ? owner : "?", t);
    return t;
}

void KPATCH_CALL ApiUnregisterScriptListener(int32_t token) {
    g_dispatcher.unregisterScriptListener(token);
    KP_LOG(chCore, "unregister script listener token=%d", token);
}

int32_t KPATCH_CALL ApiRegisterCommandHandler(int32_t cmd_id, K2VmCommandCallback fn, void* user, const char* owner) {
    int32_t t = g_dispatcher.registerCommandHandler(cmd_id, fn, user, owner);
    KP_LOGV(chCore, t > 0 ? KPLOG_INFO : KPLOG_WARN, "register cmd handler id=%d owner='%s' -> token=%d",
            cmd_id, owner ? owner : "?", t);
    return t;
}

void KPATCH_CALL ApiUnregisterCommandHandler(int32_t token) {
    g_dispatcher.unregisterCommandHandler(token);
    KP_LOG(chCore, "unregister cmd handler token=%d", token);
}

const K2ScriptEventsApi g_api = {
    sizeof(K2ScriptEventsApi),
    ApiRegisterScriptListener,
    ApiUnregisterScriptListener,
    ApiRegisterCommandHandler,
    ApiUnregisterCommandHandler
};

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->provide) return;
    api->provide(K2_SCRIPTEVENTS_IFACE, K2_SCRIPTEVENTS_VERSION, &g_api);
    KpLog_Init(api);

    int enabledIni = GetPrivateProfileIntA("script_events", "enabled", 1, ".\\script_events.ini");
    if (enabledIni == 0) {
        g_dispatcher.enabled = false;
    }

    KP_LOG(chCore, "provided %s v%d (enabled=%d)", K2_SCRIPTEVENTS_IFACE, K2_SCRIPTEVENTS_VERSION, g_dispatcher.enabled ? 1 : 0);
}

__declspec(dllexport) int __cdecl SE_OnRunScriptPre(uintptr_t ebp) {
    if (!g_dispatcher.enabled || !ebp) return 0;
    saferead::beginScope();

    uint32_t vmInternalAddr = 0;
    uint32_t exoScriptAddr = 0;
    uint32_t objSelf = 0;
    int32_t objSelfValid = 0;

    saferead::readAt(S_RunPre, ebp, OffVmInternal, &vmInternalAddr);
    saferead::readAt(S_RunPre, ebp, OffExoScript, &exoScriptAddr);
    saferead::readAt(S_RunPre, ebp, OffObjSelf, &objSelf);
    saferead::readAt(S_RunPre, ebp, OffObjValid, &objSelfValid);

    char scriptName[64] = {0};
    if (exoScriptAddr) {
        if (saferead::readExo(S_RunPre, static_cast<uintptr_t>(exoScriptAddr), scriptName, sizeof(scriptName)) <= 0) {
            uint32_t strPtr = 0;
            int len = -1;
            saferead::readAt(S_RunPre, static_cast<uintptr_t>(exoScriptAddr), 0, &strPtr);
            saferead::readAt(S_RunPre, static_cast<uintptr_t>(exoScriptAddr), 4, &len);
            saferead::readExoLoose(S_RunPre, strPtr, len, scriptName, sizeof(scriptName));
        }
    }
    for (char* p = scriptName; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') *p = static_cast<char>(*p + ('a' - 'A'));
    }

    return g_dispatcher.dispatchScriptPre(reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(vmInternalAddr)), scriptName, objSelf, objSelfValid);
}

__declspec(dllexport) void __cdecl SE_OnRunScriptPost(uintptr_t ebp, int32_t runStatus) {
    if (!g_dispatcher.enabled || !ebp) return;
    saferead::beginScope();

    uint32_t vmInternalAddr = 0;
    uint32_t exoScriptAddr = 0;
    uint32_t objSelf = 0;
    int32_t objSelfValid = 0;

    saferead::readAt(S_RunPost, ebp, OffVmInternal, &vmInternalAddr);
    saferead::readAt(S_RunPost, ebp, OffExoScript, &exoScriptAddr);
    saferead::readAt(S_RunPost, ebp, OffObjSelf, &objSelf);
    saferead::readAt(S_RunPost, ebp, OffObjValid, &objSelfValid);

    char scriptName[64] = {0};
    if (exoScriptAddr) {
        if (saferead::readExo(S_RunPost, static_cast<uintptr_t>(exoScriptAddr), scriptName, sizeof(scriptName)) <= 0) {
            uint32_t strPtr = 0;
            int len = -1;
            saferead::readAt(S_RunPost, static_cast<uintptr_t>(exoScriptAddr), 0, &strPtr);
            saferead::readAt(S_RunPost, static_cast<uintptr_t>(exoScriptAddr), 4, &len);
            saferead::readExoLoose(S_RunPost, strPtr, len, scriptName, sizeof(scriptName));
        }
    }
    for (char* p = scriptName; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') *p = static_cast<char>(*p + ('a' - 'A'));
    }

    g_dispatcher.dispatchScriptPost(scriptName, objSelf, objSelfValid, runStatus);
}

__declspec(dllexport) int __cdecl SE_OnExecuteCommand(uintptr_t ebp) {
    if (!g_dispatcher.enabled || !ebp) return 0;
    saferead::beginScope();

    uint32_t cmdProc = 0;
    int32_t cmdId = 0;
    int32_t paramCount = 0;
    uint32_t callerObjId = 0;

    saferead::readAt(S_Cmd, ebp, OffCmdProc, &cmdProc);
    saferead::readAt(S_Cmd, ebp, OffCmdId, &cmdId);
    saferead::readAt(S_Cmd, ebp, OffParamCount, &paramCount);

    if (cmdProc) {
        saferead::readAt(S_Cmd, static_cast<uintptr_t>(cmdProc), scriptevents::kCmdProcOffCallerObj, &callerObjId);
    }

    int32_t cmdStatus = 0;
    if (g_dispatcher.dispatchExecuteCommand(reinterpret_cast<void*>(static_cast<uintptr_t>(cmdProc)), callerObjId, cmdId, paramCount, &cmdStatus)) {
        *reinterpret_cast<int32_t*>(ebp + OffCmdStatus) = cmdStatus;
        return 1;
    }
    return 0;
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
