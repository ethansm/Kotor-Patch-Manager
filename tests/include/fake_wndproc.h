// fake_wndproc.h - one-HWND fake of the Win32 WndProc subclass chain for host tests (doc 20 T3).
//
// Models exactly the three calls every KOTOR II mod DLL uses to subclass the game window
// (tests/static/subclassers.toml): SetWindowLongPtr(hwnd, GWLP_WNDPROC, p) returning the previous proc,
// GetWindowLongPtr(hwnd, GWLP_WNDPROC), and CallWindowProc(prev, hwnd, msg, w, l). No real window, no message loop:
// FakeSendMessage() is "the OS dispatches a message to whatever proc is currently installed on the window".
//
//   fakewnd::Reset();                         // fresh window whose proc is the counting base "game" proc
//   old = fakewnd::FakeSetWindowLongPtr(fakewnd::kHwnd, myProc);
//   fakewnd::FakeSendMessage(0x20A, 1, 2);    // base proc increments base().hits and returns l + 1
//   fakewnd::base().hits / lastMsg
#pragma once

#include <cstdint>

namespace fakewnd {

using HWND = int;
using Proc = intptr_t (*)(HWND, unsigned, intptr_t, intptr_t);

inline constexpr HWND kHwnd = 0x1234;

struct BaseState { int hits = 0; unsigned lastMsg = 0; };
inline BaseState& base() { static BaseState s; return s; }

// The game's own WndProc: counts every message that reaches the bottom of the chain.
inline intptr_t BaseGameProc(HWND, unsigned msg, intptr_t, intptr_t l) {
    ++base().hits;
    base().lastMsg = msg;
    return l + 1;
}

struct Window { HWND h = kHwnd; Proc proc = &BaseGameProc; };
inline Window& window() { static Window w; return w; }

inline void Reset() { window() = Window{}; base() = BaseState{}; }

inline bool FakeIsWindow(HWND h) { return h == window().h; }

// Returns the previous proc, or nullptr (and changes nothing) for an unknown HWND, like the real call failing.
inline Proc FakeSetWindowLongPtr(HWND h, Proc p) {
    if (!FakeIsWindow(h)) return nullptr;
    Proc old = window().proc;
    window().proc = p;
    return old;
}
inline Proc FakeGetWindowLongPtr(HWND h) { return FakeIsWindow(h) ? window().proc : nullptr; }

inline intptr_t FakeCallWindowProc(Proc p, HWND h, unsigned m, intptr_t w, intptr_t l) { return p ? p(h, m, w, l) : 0; }

// The OS delivering one message to the window's current top proc.
inline intptr_t FakeSendMessage(unsigned m, intptr_t w, intptr_t l) {
    return window().proc(window().h, m, w, l);
}

}  // namespace fakewnd
