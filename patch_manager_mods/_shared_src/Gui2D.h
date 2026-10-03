// Gui2D.h -- the engine's immediate-mode 2D GUI calls for KOTOR2 Patch Manager mods (Steam Aspyr, i686), header-only.
// Extracted from BuffDurationHUD.cpp (Mods 2 + 4, commit 17b508d) for the buff / shield split (doc 21). Include as
// "../_shared/Gui2D.h".
//
// Engine facts (doc 02, doc 08 sessions 4-6, DB lessons 206+):
//   BeginGui2D 0x4792c0 / EndGui2D 0x4793f0 (cdecl, no args): open / close the 2D batch (ortho matrix, viewport).
//   PushClipRect 0x479460 cdecl(x, y, w, h, const float* color|0xA1BA3C, flag, float alpha) -> nonzero if pushed; it also
//     makes (x, y) the drawing origin. With a real colour it FILLS the rect (used for 1 px lines). PopClipRect 0x4799c0.
//   FlushGuiBatch 0x47ea60 cdecl(float = *(float*)0x9f5f60): draws the batched quads.
//   HUD scale 0x479ab0 cdecl -> float in st0.  Clip stack index at 0xa309c0 (low 16 bits): 0 in the HUD frame, 0xFFFF in menu frames
//   after the menu closed its batch.  Panel rect 0x410020 thiscall(panel, int r[4]).
//   Image wrapper vtable slot 6 = Draw thiscall(img, x, y, w, h, 0, rotBits, const float* rgb, float alpha).
// Rules learned the hard way (doc 08 session 4 root cause): our flushes/colour clips leave GL caps (GL_LIGHTING) toggled, so every
// draw block runs under a CapGuard; never draw in a menu frame without reopening the batch (Gui2DScope) and pushing the panel clip.
// Host tests: define GUI2D_HOST to drop the GL / CapGuard part; engine calls go through gui2d::g_g2d (fakes).
// Per-version addresses: the Addr* constants below are the Steam column; the default engine calls read the K2Addr.h table selected
// for the running exe (k2addr::get()) at call time and do nothing / return the neutral value when the table is null or the
// address is 0 (never a call into address 0).
#pragma once
#include "SafeRead.h"
#include "K2Addr.h"
#include <stdint.h>

namespace gui2d {

// Steam column (values live in K2Addr.h kSteam).
constexpr uintptr_t AddrBeginGui2D = kSteam.BeginGui2D, AddrEndGui2D = kSteam.EndGui2D;
constexpr uintptr_t AddrPushClipRect = kSteam.PushClipRect, AddrPopClipRect = kSteam.PopClipRect;
constexpr uintptr_t AddrFlushGuiBatch = kSteam.FlushGuiBatch, AddrFlushArg = kSteam.FlushArg;
constexpr uintptr_t AddrHudScale = kSteam.HudScale;
constexpr uintptr_t AddrClipDepth = kSteam.ClipDepth;
constexpr uintptr_t AddrClipNoFillColor = kSteam.ClipNoFillColor;     // what the engine's own icon row passes: clip only, no fill
constexpr uintptr_t AddrPanelRect = kSteam.PanelRect;
constexpr int ImageVtDraw = kSteam.ImageVt_Draw;

using DrawFn = void(__thiscall*)(void*, int, int, int, int, int, int, const float*, float);

struct Engine {
    void (*begin)() = [] {
        auto f = k2addr::fnptr<void(__cdecl*)()>(&K2AddrTable::BeginGui2D);
        if (f) f();
    };
    void (*end)() = [] {
        auto f = k2addr::fnptr<void(__cdecl*)()>(&K2AddrTable::EndGui2D);
        if (f) f();
    };
    int (*push)(int, int, int, int, const void*, int, float) = [](int x, int y, int w, int h, const void* c, int fl, float a) {
        auto f = k2addr::fnptr<int(__cdecl*)(int, int, int, int, const void*, int, float)>(&K2AddrTable::PushClipRect);
        return f ? f(x, y, w, h, c, fl, a) : 0;
    };
    void (*pop)() = [] {
        auto f = k2addr::fnptr<void(__cdecl*)()>(&K2AddrTable::PopClipRect);
        if (f) f();
    };
    void (*flush)(float) = [](float a) {
        auto f = k2addr::fnptr<void(__cdecl*)(float)>(&K2AddrTable::FlushGuiBatch);
        if (f) f(a);
    };
    float (*hudScale)() = [] {
        auto f = k2addr::fnptr<float(__cdecl*)()>(&K2AddrTable::HudScale);
        return f ? f() : 1.0f;
    };
    void (*panelRect)(uintptr_t, int*) = [](uintptr_t panel, int* r) {
        auto f = k2addr::fnptr<void(__thiscall*)(void*, int*)>(&K2AddrTable::PanelRect);
        if (f) f(reinterpret_cast<void*>(panel), r);
    };
    void (*draw)(void*, int, int, int, int, int, int, const float*, float) =
        [](void* img, int x, int y, int w, int h, int a5, int rot, const float* rgb, float alpha) {
            int slot = k2addr::off(&K2AddrTable::ImageVt_Draw);
            if (!img || slot <= 0) return;
            reinterpret_cast<DrawFn>((*reinterpret_cast<void***>(img))[slot])(img, x, y, w, h, a5, rot, rgb, alpha);
        };
    // UseTable / nullptr = the table's FlushArg / ClipDepth / ClipNoFillColor; a host test sets arena addresses (0 = none).
    uintptr_t flushArgAddr = k2addr::UseTable, clipDepthAddr = k2addr::UseTable;
    const void* clipNoFill = nullptr;
};
inline Engine g_g2d;

template <typename T> inline bool rd(uintptr_t base, int off, T* out) { return saferead::readAt(1, base, off, out); }

// The no-fill clip colour; nullptr when this exe's address is unknown (pushClip then refuses).
inline const void* clipNoFill() {
    return g_g2d.clipNoFill ? g_g2d.clipNoFill : reinterpret_cast<const void*>(k2addr::addr(&K2AddrTable::ClipNoFillColor));
}
inline float flushArg() {
    uintptr_t a = g_g2d.flushArgAddr != k2addr::UseTable ? g_g2d.flushArgAddr : k2addr::addr(&K2AddrTable::FlushArg);
    float v = 0.0f;
    if (a) rd(a, 0, &v);
    return v;
}
inline void flush(float arg) { g_g2d.flush(arg); }
// Clip + new origin at (x, y); nonzero = pushed (caller must popClip). color = clipNoFill() for a pure clip.
inline int pushClip(int x, int y, int w, int h, const void* color = nullptr, float alpha = 1.0f) {
    const void* c = color ? color : clipNoFill();
    if (!c) return 0;   // no-fill colour address unknown for this exe: nothing pushed, so the caller never pops
    return g_g2d.push(x, y, w, h, c, 0, alpha);
}
inline void popClip() { g_g2d.pop(); }
// A filled rect (PushClipRect with a colour, then pop at once): 1 px lines, seams.
inline void fillRect(int x, int y, int w, int h, const float* rgb, float alpha) { if (pushClip(x, y, w, h, rgb, alpha)) popClip(); }
inline void drawImage(void* img, int x, int y, int w, int h, int rotBits, const float* rgb, float alpha) {
    g_g2d.draw(img, x, y, w, h, 0, rotBits, rgb, alpha);
}
// HUD scale, 1.0 when the engine returns nonsense.
inline float hudScale() { float s = g_g2d.hudScale(); return (s > 0.1f && s < 10.0f) ? s : 1.0f; }
inline int clipDepthRaw() {
    uintptr_t a = g_g2d.clipDepthAddr != k2addr::UseTable ? g_g2d.clipDepthAddr : k2addr::addr(&K2AddrTable::ClipDepth);
    int cd = 0;
    if (a) rd(a, 0, &cd);
    return cd;
}
// Menu frame: the clip stack index is not 0 (0xFFFF after a menu closed its batch). Drawing needs a Gui2DScope there.
inline bool inMenuFrame() { return (clipDepthRaw() & 0xffff) != 0; }
// Panel rect via the engine; false when it looks wrong (0 or > 8192 wide/high).
inline bool panelRect(uintptr_t panel, int r[4]) {
    r[0] = r[1] = r[2] = r[3] = 0;
    g_g2d.panelRect(panel, r);
    return !(r[2] < 1 || r[3] < 1 || r[2] > 8192 || r[3] > 8192);
}

// Reopen the 2D batch for the scope (menu frames). Balanced by construction; never nest inside another Gui2DScope.
struct Gui2DScope {
    Gui2DScope() { g_g2d.begin(); }
    ~Gui2DScope() { g_g2d.end(); }
    Gui2DScope(const Gui2DScope&) = delete;
    Gui2DScope& operator=(const Gui2DScope&) = delete;
};
// pushClip for the scope; pops only if the push succeeded. if (ClipScope c{...}) { draw }.
struct ClipScope {
    int ok;
    ClipScope(int x, int y, int w, int h, const void* color = nullptr, float alpha = 1.0f) : ok(pushClip(x, y, w, h, color, alpha)) {}
    ~ClipScope() { if (ok) popClip(); }
    explicit operator bool() const { return ok != 0; }
    ClipScope(const ClipScope&) = delete;
    ClipScope& operator=(const ClipScope&) = delete;
};

#ifndef GUI2D_HOST
// ---- GL state guard -------------------------------------------------------------------------------------------------------
struct GlFns {
    bool tried = false, ok = false;
    int (WINAPI* getError)() = nullptr;
    void (WINAPI* getIntegerv)(unsigned, int*) = nullptr;
    void (WINAPI* getBooleanv)(unsigned, unsigned char*) = nullptr;
    unsigned char (WINAPI* isEnabled)(unsigned) = nullptr;
    void (WINAPI* readPixels)(int, int, int, int, unsigned, unsigned, void*) = nullptr;
    void (WINAPI* enable)(unsigned) = nullptr;
    void (WINAPI* disable)(unsigned) = nullptr;
};
// Optional diagnostics callbacks a DLL may set (its own log file / channel).
struct Hooks {
    void (*onGlInit)(const GlFns&) = nullptr;                 // once, right after opengl32 lookups
    void (*onCapFix)(unsigned cap, int was, int now) = nullptr; // a cap our draws changed was put back (first 20 per DLL)
};
inline GlFns g_gl;
inline Hooks g_hooks;

inline GlFns& gl() {
    if (g_gl.tried) return g_gl;
    g_gl.tried = true;
    HMODULE m = GetModuleHandleA("opengl32.dll");
    if (m) {
        g_gl.getError = reinterpret_cast<decltype(g_gl.getError)>(GetProcAddress(m, "glGetError"));
        g_gl.getIntegerv = reinterpret_cast<decltype(g_gl.getIntegerv)>(GetProcAddress(m, "glGetIntegerv"));
        g_gl.getBooleanv = reinterpret_cast<decltype(g_gl.getBooleanv)>(GetProcAddress(m, "glGetBooleanv"));
        g_gl.isEnabled = reinterpret_cast<decltype(g_gl.isEnabled)>(GetProcAddress(m, "glIsEnabled"));
        g_gl.readPixels = reinterpret_cast<decltype(g_gl.readPixels)>(GetProcAddress(m, "glReadPixels"));
        g_gl.enable = reinterpret_cast<decltype(g_gl.enable)>(GetProcAddress(m, "glEnable"));
        g_gl.disable = reinterpret_cast<decltype(g_gl.disable)>(GetProcAddress(m, "glDisable"));
        g_gl.ok = g_gl.getError && g_gl.getIntegerv && g_gl.getBooleanv && g_gl.isEnabled;
        if (g_hooks.onGlInit) g_hooks.onGlInit(g_gl);
    }
    return g_gl;
}

// Snapshot the on/off GL capabilities our GUI draws can toggle (scissor, stencil, depth, lighting, blend, fog, tex2d, cull, alpha
// test) and put back any that changed when the scope ends. One per draw block, every frame (probe 02:34: PRE light=0 -> POST light=1).
struct CapGuard {
    static constexpr unsigned Caps[9] = { 0x0C11, 0x0B90, 0x0B71, 0x0B50, 0x0BE2, 0x0B60, 0x0DE1, 0x0B44, 0x0BC0 };
    unsigned char before[9] = {}; bool armed = false;
    CapGuard() {
        GlFns& g = gl();
        if (!g.ok || !g.enable || !g.disable) return;
        for (int i = 0; i < 9; ++i) before[i] = g.isEnabled(Caps[i]);
        armed = true;
    }
    ~CapGuard() {
        if (!armed) return;
        for (int i = 0; i < 9; ++i) {
            unsigned char now = g_gl.isEnabled(Caps[i]);
            if (now != before[i]) {
                static int logged = 0;
                if (logged < 20 && g_hooks.onCapFix) { ++logged; g_hooks.onCapFix(Caps[i], before[i], now); }
                (before[i] ? g_gl.enable : g_gl.disable)(Caps[i]);
            }
        }
    }
    CapGuard(const CapGuard&) = delete;
    CapGuard& operator=(const CapGuard&) = delete;
};
#endif // GUI2D_HOST

} // namespace gui2d
