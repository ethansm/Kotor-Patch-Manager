#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "ActivationLogic.h"
#include "../Common/GameAddr.h"

namespace {

// The four stdcall arguments (hwnd, msg, wParam, lParam), in stack order. Used for both hooks:
//   synthetic post site: the game pushed them for PostMessageA (hook sees esp+0 = first arg).
//   WndProc entry: esp+0 is the return address, args follow (WndProcFrame below).
struct MsgArgs {
    HWND hwnd;
    UINT msg;
    WPARAM wParam;
    LPARAM lParam;
};

// Foreground window's process id: 0 when there is no foreground window.
DWORD foregroundPid() {
    HWND foreground = GetForegroundWindow();
    if (foreground == nullptr) {
        return 0;
    }
    DWORD processId = 0;
    GetWindowThreadProcessId(foreground, &processId);
    return processId;
}

// The engine's own latch condition (WndProc 0x40A750 case 0x1c): client DAT_00a1b4a4 exists with a non-null +4, and
// CClientExoApp::IsMoviePlaying (0x00740160, size 22, no arguments, pure query) returns 1.
bool moviePlaying() {
    const uintptr_t client = *reinterpret_cast<volatile uintptr_t*>(gameaddr::AppGlobal);
    if (client == 0 || *reinterpret_cast<volatile uintptr_t*>(client + 4) == 0) {
        return false;
    }
    return reinterpret_cast<int(__cdecl*)()>(gameaddr::IsMoviePlaying)() == 1;
}

// WndProc entry frame as the hook delivers it: [esp+0] return address, then the stdcall arguments.
struct WndProcFrame {
    uintptr_t ret;
    MsgArgs args;
};

constexpr LONG kMaxLogged = 50;
volatile LONG g_dropped = 0;

void logDrop(const char* site, const MsgArgs* a, DWORD fgPid) {
    LONG n = InterlockedIncrement(&g_dropped);
    if (n > kMaxLogged) {
        return;
    }
    FILE* f = fopen("k2-activation-fix_drops.txt", "a");
    if (!f) {
        return;
    }
    fprintf(f, "ACTIVATE-DROP #%ld site=%s lp=%08x fgPid=%lu tid=%lu\n", n, site, static_cast<unsigned>(a->lParam),
            static_cast<unsigned long>(fgPid), static_cast<unsigned long>(GetCurrentThreadId()));
    fclose(f);
}

}

// Movie thread's synthetic PostMessageA(WM_ACTIVATEAPP, 0, 0).
extern "C" void __cdecl dropSyntheticDeactivate(MsgArgs* args) {
    if (args == nullptr) {
        return;
    }
    DWORD fg = foregroundPid();
    if (!actfix::shouldDropSyntheticPost(args->msg, args->wParam, static_cast<intptr_t>(args->lParam), fg, GetCurrentProcessId())) {
        return;
    }
    // Turn the post into PostThreadMessage(WM_NULL), which nothing acts on.
    args->hwnd = nullptr;
    args->msg = WM_NULL;
    logDrop("post", args, fg);
}

// WndProc entry: WM_ACTIVATEAPP(wParam==0) from the movie thread while our process owns the foreground.
extern "C" void __cdecl dropForeignDeactivate(WndProcFrame* frame) {
    if (frame == nullptr) {
        return;
    }
    MsgArgs* args = &frame->args;
    if (args->msg != WM_ACTIVATEAPP || args->wParam != 0) {
        return;   // cheap early out: this runs for every window message
    }
    DWORD fg = foregroundPid();
    if (fg == 0 || fg != GetCurrentProcessId() || !actfix::shouldDropDeactivate(args->msg, args->wParam, fg, GetCurrentProcessId(), moviePlaying())) {
        return;
    }
    args->msg = WM_NULL;   // WndProc falls through to DefWindowProc: no latch, no minimize, no AppDeactivated
    logDrop("wndproc", args, fg);
}
