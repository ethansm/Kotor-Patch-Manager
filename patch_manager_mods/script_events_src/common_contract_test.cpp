// common_contract_test.cpp -- Patches/Common CONTRACT test for script-events.
// Rule: every Patches/Common API a mod uses gets a mod-owned contract test colocated with the mod.
// Pins the semantics ScriptEventsProvider.cpp relies on in SafeRead.h, KpLog.h, ScriptEvents.h, and ScriptEventsCore.h.
// Every message starts with "script-events relies on: ". Host-only, deterministic, no game.
#ifndef GUIKIT_HOST
#define GUIKIT_HOST 1
#endif
#include <windows.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "SafeRead.h"
#include "KpLog.h"
#include "ScriptEvents.h"
#include "ScriptEventsCore.h"
#include "check.h"
#include "FakeKPatchApi.h"

#define HCHECK(why, expr) ::kt::Check("script-events relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))
#define HEQ(why, a, b) ::kt::CheckEq("script-events relies on: " why " (line " KT_STR(__LINE__) ")", (a), (b))

KP_LOG_PATCH("script-events");
KP_CHANNEL(chCore, "core");

static uint8_t* g_arena;

static void putExo(uint32_t at, uint32_t strAt, const char* s, int len) {
    memcpy(g_arena + strAt, s, strlen(s) + 1);
    uint32_t p = uint32_t(reinterpret_cast<uintptr_t>(g_arena + strAt));
    memcpy(g_arena + at, &p, 4);
    memcpy(g_arena + at + 4, &len, 4);
}

static int32_t condScriptCb(const K2ScriptEvent*, int32_t* out_has_ret, int32_t* out_ret, void*) {
    if (out_has_ret) *out_has_ret = 1;
    if (out_ret) *out_ret = 1;
    return K2SC_HANDLED;
}

static int32_t cmdCb(const K2VmCommandEvent*, int32_t* out_status, void*) {
    if (out_status) *out_status = 0; // VM_SUCCESS
    return K2SC_HANDLED;
}

int main() {
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(MAP_32BIT)
    flags |= MAP_32BIT;
#endif
    g_arena = static_cast<uint8_t*>(mmap(nullptr, 0x10000, PROT_READ | PROT_WRITE, flags, -1, 0));
    HCHECK("arena allocated", g_arena != nullptr && g_arena != MAP_FAILED);
    g_stubVirtualQueryAll = true;
    const uintptr_t A = reinterpret_cast<uintptr_t>(g_arena);

    // -------------------------------------------------------------
    // 1. SafeRead.h: negative and positive stack offset reads
    // -------------------------------------------------------------
    {
        saferead::beginScope();
        // Simulate a stack frame at A + 0x1000
        const uintptr_t fakeEbp = A + 0x1000;
        uint32_t vmInternalVal = 0x2000;
        uint32_t exoScriptVal = 0x3000;
        uint32_t objSelfVal = 0x00030001;
        int32_t objValidVal = 1;

        memcpy(reinterpret_cast<void*>(fakeEbp - 0x0C), &vmInternalVal, 4);
        memcpy(reinterpret_cast<void*>(fakeEbp + 0x08), &exoScriptVal, 4);
        memcpy(reinterpret_cast<void*>(fakeEbp + 0x0C), &objSelfVal, 4);
        memcpy(reinterpret_cast<void*>(fakeEbp + 0x10), &objValidVal, 4);

        uint32_t readVm = 0, readExoPtr = 0;
        uint32_t readObj = 0;
        int32_t readValid = 0;

        HCHECK("readAt<uint32_t> with negative offset -0x0C succeeds", saferead::readAt(0x7501, fakeEbp, -0x0C, &readVm));
        HEQ("vmInternal pointer matches", (long long)readVm, (long long)vmInternalVal);

        HCHECK("readAt<uint32_t> with offset +0x08 succeeds", saferead::readAt(0x7501, fakeEbp, 0x08, &readExoPtr));
        HEQ("exoScript pointer matches", (long long)readExoPtr, (long long)exoScriptVal);

        HCHECK("readAt<uint32_t> with offset +0x0C succeeds", saferead::readAt(0x7501, fakeEbp, 0x0C, &readObj));
        HEQ("objSelf matches", (long long)readObj, (long long)objSelfVal);

        HCHECK("readAt<int32_t> with offset +0x10 succeeds", saferead::readAt(0x7501, fakeEbp, 0x10, &readValid));
        HEQ("objSelfValid matches", (long long)readValid, (long long)objValidVal);

        // CExoString read via readExo
        const uint32_t exoAt = 0x500;
        putExo(exoAt, 0x600, "k_ai_master", 12);
        char scriptBuf[64] = {0};
        HEQ("readExo reads script name without NUL", saferead::readExo(0x7501, A + exoAt, scriptBuf, sizeof(scriptBuf)), 11);
        HCHECK("readExo content matches", strcmp(scriptBuf, "k_ai_master") == 0);
    }

    // -------------------------------------------------------------
    // 2. KpLog.h: fail-soft init, HasCore, and channel logging
    // -------------------------------------------------------------
    {
        HEQ("KpLog_Init(nullptr) = 0", KpLog_Init(nullptr), 0);
        fakekp::reset();
        HEQ("KpLog_Init without service = 0", KpLog_Init(fakekp::api()), 0);
        HCHECK("KpLog_HasCore is false without service", !KpLog_HasCore());

        fakekp::withLog();
        HEQ("KpLog_Init with service = 1", KpLog_Init(fakekp::api()), 1);
        HCHECK("KpLog_HasCore is true with service", KpLog_HasCore());

        KP_LOG(chCore, "script-events test contract log");
        HCHECK("KP_LOG reaches log buffer", !fakekp::state().logLines.empty());
    }

    // -------------------------------------------------------------
    // 3. ScriptEventsCore.h: StartingConditional return value & command short-circuit
    // -------------------------------------------------------------
    {
        scriptevents::Dispatcher disp;
        int32_t tScript = disp.registerScriptListener(K2SC_SCRIPT_PRE, "k_ai_master", condScriptCb, nullptr, "test_owner");
        HCHECK("registerScriptListener returns valid token", tScript > 0);

        uint8_t vmInternalBuf[64] = {0};
        int32_t handled = disp.dispatchScriptPre(vmInternalBuf, "K_AI_MASTER", 0x1234, 1);
        HEQ("dispatchScriptPre returns 1 (short-circuit)", handled, 1);
        HEQ("StartingConditional return type injected at +0x1C is kVmRetTypeInt (3)",
            *reinterpret_cast<int32_t*>(vmInternalBuf + scriptevents::kVmOffRetType), scriptevents::kVmRetTypeInt);
        HEQ("StartingConditional return value injected at +0x20 is 1",
            *reinterpret_cast<int32_t*>(vmInternalBuf + scriptevents::kVmOffRetVal), 1);

        // Command handler
        int32_t tCmd = disp.registerCommandHandler(120, cmdCb, nullptr, "test_owner");
        HCHECK("registerCommandHandler returns valid token", tCmd > 0);

        int32_t outCmdStatus = -1;
        void* fakeProc = reinterpret_cast<void*>(A + 0x2000);
        int32_t cmdHandled = disp.dispatchExecuteCommand(fakeProc, 0x1234, 120, 2, &outCmdStatus);
        HEQ("dispatchExecuteCommand returns 1", cmdHandled, 1);
        HEQ("outCmdStatus is set to 0 (VM_SUCCESS)", outCmdStatus, 0);
    }

    return kt::Report("script_events_common_contract_test");
}
