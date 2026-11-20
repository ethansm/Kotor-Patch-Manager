#pragma once

#include "../Common/ScriptEvents.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace scriptevents {

constexpr int32_t kVmOffRetType = 0x1C;
constexpr int32_t kVmOffRetVal  = 0x20;
constexpr int32_t kVmRetTypeInt = 3;
// OBJECT_SELF in the VM impl (stored by RunScript, see ScriptEvents.h). +0x04 is bObjectSelfValid, not the object id.
constexpr int32_t kCmdProcOffCallerObj = 0x08;

// Address of the caller object id read for K2VmCommandEvent.caller_object_id.
inline uintptr_t callerObjectIdAddr(uintptr_t cmdProc) {
    return cmdProc + static_cast<uintptr_t>(kCmdProcOffCallerObj);
}

constexpr int kMaxScriptListeners = 32;
constexpr int kMaxCmdHandlers = 32;

inline bool strCaseEqual(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = *a++;
        char cb = *b++;
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + ('a' - 'A'));
        if (ca != cb) return false;
    }
    return *a == *b;
}

inline bool matchesFilter(const char* filter, const char* scriptName) {
    if (!filter || filter[0] == '\0') return true;
    if (!scriptName) return false;
    return strCaseEqual(filter, scriptName);
}

struct ScriptListenerEntry {
    int32_t token = 0;
    int32_t kindMask = 0;
    char filter[32] = {0};
    K2ScriptCallback fn = nullptr;
    void* user = nullptr;
    char owner[32] = {0};
    bool active = false;
};

struct CmdHandlerEntry {
    int32_t token = 0;
    int32_t cmdId = 0;
    K2VmCommandCallback fn = nullptr;
    void* user = nullptr;
    char owner[32] = {0};
    bool active = false;
};

struct Dispatcher {
    bool enabled = true;
    ScriptListenerEntry scriptListeners[kMaxScriptListeners] = {};
    CmdHandlerEntry cmdHandlers[kMaxCmdHandlers] = {};
    int32_t nextToken = 1;
    // Active-handler counts, rebuilt by recount() on every register/unregister (main thread only).
    // The SE_On* hooks test these before any SafeRead, so an idle dispatcher costs one compare.
    int32_t preCount = 0;   // active script listeners with K2SC_SCRIPT_PRE
    int32_t postCount = 0;  // active script listeners with K2SC_SCRIPT_POST
    int32_t cmdCount = 0;   // active command handlers

    void recount() {
        preCount = 0;
        postCount = 0;
        cmdCount = 0;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (!scriptListeners[i].active) continue;
            if (scriptListeners[i].kindMask & K2SC_SCRIPT_PRE) ++preCount;
            if (scriptListeners[i].kindMask & K2SC_SCRIPT_POST) ++postCount;
        }
        for (int i = 0; i < kMaxCmdHandlers; ++i) {
            if (cmdHandlers[i].active) ++cmdCount;
        }
    }

    bool wantsScriptPre() const { return enabled && preCount > 0; }
    bool wantsScriptPost() const { return enabled && postCount > 0; }
    bool wantsCommand() const { return enabled && cmdCount > 0; }

    void clear() {
        for (int i = 0; i < kMaxScriptListeners; ++i) scriptListeners[i] = {};
        for (int i = 0; i < kMaxCmdHandlers; ++i) cmdHandlers[i] = {};
        nextToken = 1;
        enabled = true;
        recount();
    }

    bool isScriptListenerActive(int32_t token) const {
        if (token <= 0) return false;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (scriptListeners[i].active && scriptListeners[i].token == token) return true;
        }
        return false;
    }

    bool isCmdHandlerActive(int32_t token) const {
        if (token <= 0) return false;
        for (int i = 0; i < kMaxCmdHandlers; ++i) {
            if (cmdHandlers[i].active && cmdHandlers[i].token == token) return true;
        }
        return false;
    }

    int32_t registerScriptListener(int32_t kindMask, const char* scriptFilter, K2ScriptCallback fn, void* user, const char* owner) {
        if (!fn || (kindMask & K2SC_SCRIPT_ALL) == 0) return 0;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (!scriptListeners[i].active) {
                auto& e = scriptListeners[i];
                e.token = nextToken++;
                if (nextToken <= 0) nextToken = 1;
                e.kindMask = kindMask & K2SC_SCRIPT_ALL;
                e.filter[0] = 0;
                if (scriptFilter) {
                    size_t len = 0;
                    while (scriptFilter[len] && len < sizeof(e.filter) - 1) {
                        e.filter[len] = scriptFilter[len];
                        ++len;
                    }
                    e.filter[len] = 0;
                }
                e.fn = fn;
                e.user = user;
                e.owner[0] = 0;
                if (owner) {
                    size_t len = 0;
                    while (owner[len] && len < sizeof(e.owner) - 1) {
                        e.owner[len] = owner[len];
                        ++len;
                    }
                    e.owner[len] = 0;
                }
                e.active = true;
                recount();
                return e.token;
            }
        }
        return 0; // full
    }

    void unregisterScriptListener(int32_t token) {
        if (token <= 0) return;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (scriptListeners[i].active && scriptListeners[i].token == token) {
                scriptListeners[i].active = false;
                scriptListeners[i].token = 0;
                recount();
                return;
            }
        }
    }

    int32_t registerCommandHandler(int32_t cmdId, K2VmCommandCallback fn, void* user, const char* owner) {
        if (!fn) return 0;
        for (int i = 0; i < kMaxCmdHandlers; ++i) {
            if (!cmdHandlers[i].active) {
                auto& e = cmdHandlers[i];
                e.token = nextToken++;
                if (nextToken <= 0) nextToken = 1;
                e.cmdId = cmdId;
                e.fn = fn;
                e.user = user;
                e.owner[0] = 0;
                if (owner) {
                    size_t len = 0;
                    while (owner[len] && len < sizeof(e.owner) - 1) {
                        e.owner[len] = owner[len];
                        ++len;
                    }
                    e.owner[len] = 0;
                }
                e.active = true;
                recount();
                return e.token;
            }
        }
        return 0; // full
    }

    void unregisterCommandHandler(int32_t token) {
        if (token <= 0) return;
        for (int i = 0; i < kMaxCmdHandlers; ++i) {
            if (cmdHandlers[i].active && cmdHandlers[i].token == token) {
                cmdHandlers[i].active = false;
                cmdHandlers[i].token = 0;
                recount();
                return;
            }
        }
    }

    int32_t dispatchScriptPre(uint8_t* vmInternal, const char* scriptName, uint32_t objSelf, int32_t objSelfValid) {
        if (!wantsScriptPre()) return 0; // no snapshot copy when nobody listens
        const char* safeName = scriptName ? scriptName : "";

        K2ScriptEvent ev;
        ev.struct_size = sizeof(K2ScriptEvent);
        ev.kind = K2SC_SCRIPT_PRE;
        ev.script_name = safeName;
        ev.object_self = objSelf;
        ev.object_self_valid = objSelfValid;
        ev.run_status = 0;

        ScriptListenerEntry snap[kMaxScriptListeners];
        int snapCount = 0;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (scriptListeners[i].active && (scriptListeners[i].kindMask & K2SC_SCRIPT_PRE)) {
                snap[snapCount++] = scriptListeners[i];
            }
        }

        for (int i = 0; i < snapCount; ++i) {
            if (!isScriptListenerActive(snap[i].token)) continue;
            if (!matchesFilter(snap[i].filter, safeName)) continue;

            int32_t hasRetVal = 0;
            int32_t retVal = 0;
            int32_t status = snap[i].fn(&ev, &hasRetVal, &retVal, snap[i].user);
            if (status == K2SC_HANDLED) {
                if (hasRetVal != 0 && vmInternal != nullptr) {
                    *reinterpret_cast<int32_t*>(vmInternal + kVmOffRetType) = kVmRetTypeInt;
                    *reinterpret_cast<int32_t*>(vmInternal + kVmOffRetVal) = retVal;
                }
                return 1;
            }
        }
        return 0;
    }

    void dispatchScriptPost(const char* scriptName, uint32_t objSelf, int32_t objSelfValid, int32_t runStatus) {
        if (!wantsScriptPost()) return; // no snapshot copy when nobody listens
        const char* safeName = scriptName ? scriptName : "";

        K2ScriptEvent ev;
        ev.struct_size = sizeof(K2ScriptEvent);
        ev.kind = K2SC_SCRIPT_POST;
        ev.script_name = safeName;
        ev.object_self = objSelf;
        ev.object_self_valid = objSelfValid;
        ev.run_status = runStatus;

        ScriptListenerEntry snap[kMaxScriptListeners];
        int snapCount = 0;
        for (int i = 0; i < kMaxScriptListeners; ++i) {
            if (scriptListeners[i].active && (scriptListeners[i].kindMask & K2SC_SCRIPT_POST)) {
                snap[snapCount++] = scriptListeners[i];
            }
        }

        for (int i = 0; i < snapCount; ++i) {
            if (!isScriptListenerActive(snap[i].token)) continue;
            if (!matchesFilter(snap[i].filter, safeName)) continue;

            int32_t hasRetVal = 0;
            int32_t retVal = 0;
            snap[i].fn(&ev, &hasRetVal, &retVal, snap[i].user);
        }
    }

    int32_t dispatchExecuteCommand(void* cmdProc, uint32_t callerObjId, int32_t cmdId, int32_t paramCount, int32_t* outCmdStatus) {
        if (!wantsCommand()) return 0; // no snapshot copy when nobody listens

        K2VmCommandEvent ev;
        ev.struct_size = sizeof(K2VmCommandEvent);
        ev.cmd_id = cmdId;
        ev.param_count = paramCount;
        ev.cmd_processor = cmdProc;
        ev.caller_object_id = callerObjId;

        CmdHandlerEntry snap[kMaxCmdHandlers];
        int snapCount = 0;
        for (int i = 0; i < kMaxCmdHandlers; ++i) {
            if (cmdHandlers[i].active) {
                snap[snapCount++] = cmdHandlers[i];
            }
        }

        for (int i = 0; i < snapCount; ++i) {
            if (!isCmdHandlerActive(snap[i].token)) continue;
            if (snap[i].cmdId != K2SC_CMD_ALL && snap[i].cmdId != cmdId) continue;

            int32_t statusVal = 0;
            int32_t res = snap[i].fn(&ev, &statusVal, snap[i].user);
            if (res == K2SC_HANDLED) {
                if (outCmdStatus) {
                    *outCmdStatus = statusVal;
                }
                return 1;
            }
        }
        return 0;
    }
};

} // namespace scriptevents
