// GuiKit.h -- header-only GUI helper for KOTOR2 Patch Manager mods (Steam Aspyr swkotor2.exe, i686).
// Wraps the engine recipes already proven in Mod 3 (ForcePowerHotbar.cpp) and Mod 14 (CharEffectsPanel.cpp):
// create a CSWGuiLabel from a layout template, append it to a panel, place / show / hide / colour / fill it, and poll
// the OS mouse against its pixel rect. No STL. Every engine read goes through saferead::readAt; every function returns
// false / 0 on failure instead of crashing. Include as "../_shared/GuiKit.h" (it pulls in "SafeRead.h" the same way).
//
// Engine facts (all verified in Mods 3/14):
//   ctl+0x00 vtable (slot 1 = SetRect(int[4] x,y,w,h)); +4/+8/+0xC/+0x10 extent = engine pixels x,y,w,h; +0x48 flags:
//   bit1 (0x2) visible, bit5 (0x20) click-through (hit test 0x00418D20 rejects the mouse); +0x54 id (= index in panel list).
//   Draw order = panel list index (append = on top). Label: +0x60 border (vtable 0x9875BC; +0x24 alpha, +0x28 tint rgb,
//   +0x74 fill image*), +0xD8 text (vtable 0x9876B4; +0x14 renderer, +0x18 string sub-object, colour at +0x18+0x2C).
//   DLL-set text draws left-aligned unless setAlign is called.
// Host tests: define GUIKIT_HOST before including to drop the Win32 platform defaults; every engine call and platform
// query is a function pointer in guikit::Engine so a test substitutes fakes (see research/companion_levelup/tools/parity/guikit_test.cpp).
#pragma once
#include "SafeRead.h"
#include <string.h>
#include <math.h>
#include <ctype.h>

namespace guikit {

// ---- engine addresses / offsets (Steam Aspyr) ----
constexpr uintptr_t AddrNew = 0x00919723;            // cdecl(size)                                   Mod3 FnNew 52
constexpr uintptr_t AddrLabelCtor = 0x00419740;      // CSWGuiLabel ctor, ECX=this, no stack args     Mod3 FnLabelCtor 49
constexpr uintptr_t AddrInitControl = 0x0040F620;    // thiscall(panel, ctl, CExoString*, addToList, scale) RET 16   Mod3 50
constexpr uintptr_t AddrStopLoad = 0x0040F5A0;       // StopLoadFromLayout (createLabelFromTemplate must run BEFORE this)
constexpr uintptr_t AddrExoCtor = 0x00733570, AddrExoDtor = 0x00733780;   // Mod3 51
constexpr uintptr_t AddrTextSet = 0x00416E30;        // thiscall(label+0xF0, CExoString*)             Mod3 FnTextSet 77
constexpr uintptr_t AddrSetFont = 0x00416DE0;        // thiscall(label+0xF0, char resref[16]) RET 4   Mod3 970
constexpr uintptr_t AddrSetAlign = 0x00416FA0;       // thiscall(label+0xF0, unsigned) RET 4          Mod3 971
constexpr uintptr_t AddrTextColor = 0x00417140;      // thiscall(label+0xF0, float rgb[3]) RET 4      Mod14 FnTextColor 47
constexpr uintptr_t AddrPtrListAdd = 0x0083EA60;     // CExoArrayList<void*>::Add thiscall(list, v) RET 4   Mod3 969
constexpr uintptr_t AddrLoadImage = 0x0047EB60;      // cdecl(resref buf) -> image*                   Mod3 53 / Mod14 46
constexpr uintptr_t AddrGuiSoundMgr = 0x00A1B49C, AddrPlayGuiSound = 0x004122A0;   // thiscall(mgr, char idx), idx 1 = hover   Mod3 78
constexpr uintptr_t AddrScreenW = 0x009F42A4, AddrScreenH = 0x009F42A8;           // engine framebuffer size   Mod14 syncScreen 218
constexpr uint32_t LabelVtable = 0x009878BC, BorderVtable = 0x009875BC, TextVtable = 0x009876B4;
constexpr int LabelSize = 0x148;
constexpr int Ctl_Flags = 0x48, Ctl_Id = 0x54, Ctl_X = 4, Ctl_Y = 8, Ctl_W = 0xC, Ctl_H = 0x10;
constexpr int Label_Border = 0x60, Label_Text = 0xD8, Text_Renderer = 0x14, Text_StringSub = 0x18, Text_Color = 0x2C;
constexpr int Border_Alpha = 0x24, Border_Tint = 0x28, Border_Fill = 0x74;
constexpr int Panel_CtlArray = 0x24, Panel_CtlCount = 0x28;
constexpr int FlagVisible = 0x2, FlagClickThrough = 0x20;
constexpr int IdSentinel = -31337;                   // InitControl's Load overwrites ctl+0x54; unchanged = template tag not in layout (Mod3 60)

// Calling conventions: the label ctor is "ECX only", i.e. thiscall with no args (Mod 3 spells it __fastcall; identical).
using NewFn = void* (__cdecl*)(unsigned);
using CtorFn = void(__thiscall*)(void*);
using InitCtlFn = void(__thiscall*)(void*, void*, void*, int, int);
using ExoCtorFn = void*(__thiscall*)(void*, const char*);
using ExoDtorFn = void(__thiscall*)(void*);
using TextSetFn = void(__thiscall*)(void*, void*);
using SetResFn = void(__thiscall*)(void*, const char*);
using SetUIntFn = void(__thiscall*)(void*, unsigned);
using SetColorFn = void(__thiscall*)(void*, float*);
using ListAddFn = void(__thiscall*)(void*, void*);
using LoadImageFn = void* (__cdecl*)(const char*);
using PlaySoundFn = void(__thiscall*)(void*, char);

struct Engine {
    NewFn alloc = reinterpret_cast<NewFn>(AddrNew);
    CtorFn labelCtor = reinterpret_cast<CtorFn>(AddrLabelCtor);
    InitCtlFn initControl = reinterpret_cast<InitCtlFn>(AddrInitControl);
    ExoCtorFn exoCtor = reinterpret_cast<ExoCtorFn>(AddrExoCtor);
    ExoDtorFn exoDtor = reinterpret_cast<ExoDtorFn>(AddrExoDtor);
    TextSetFn textSet = reinterpret_cast<TextSetFn>(AddrTextSet);
    SetResFn setFont = reinterpret_cast<SetResFn>(AddrSetFont);
    SetUIntFn setAlign = reinterpret_cast<SetUIntFn>(AddrSetAlign);
    SetColorFn textColor = reinterpret_cast<SetColorFn>(AddrTextColor);
    ListAddFn listAdd = reinterpret_cast<ListAddFn>(AddrPtrListAdd);
    LoadImageFn loadImage = reinterpret_cast<LoadImageFn>(AddrLoadImage);
    PlaySoundFn playSound = reinterpret_cast<PlaySoundFn>(AddrPlayGuiSound);
    // vtable slot 1 = SetRect. `fn` is the slot's value read from engine memory (a host test ignores it and records the call).
    void (*callSetRect)(uint32_t fn, uintptr_t ctl, int* xywh) = [](uint32_t fn, uintptr_t ctl, int* r) {
        reinterpret_cast<void(__thiscall*)(void*, int*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(ctl), r);
    };
    uintptr_t guiSoundMgrAddr = AddrGuiSoundMgr, screenWAddr = AddrScreenW, screenHAddr = AddrScreenH;
    // Platform queries (defaults below under !GUIKIT_HOST). Client size is the game window's client rect in OS pixels.
    bool (*cursorClient)(int* cx, int* cy, int* cw, int* ch) = nullptr;   // cursor in game-window client px + client size; false = none
    bool (*gameForeground)() = nullptr;                                   // game window is the foreground window of this process
    bool (*lbuttonDown)() = nullptr;                                      // GetAsyncKeyState(VK_LBUTTON) & 0x8000
};
inline Engine g_gk;   // substitutable by tests; defaults are the real Steam values

namespace detail {
constexpr unsigned SiteBase = 0x6B00;
template <typename T> inline bool rd(uintptr_t base, int off, T* out) { return saferead::readAt<T>(SiteBase, base, off, out); }
inline bool rd32(uintptr_t base, int off, uint32_t* out) { return rd<uint32_t>(base, off, out); }
inline bool rdInt(uintptr_t base, int off, int* out) { return rd<int>(base, off, out); }
// Guarded write of a value to engine memory: the target must be readable (same guard level as the originals' direct stores).
template <typename T> inline bool wr(uintptr_t base, int off, T v) {
    T cur; if (!rd<T>(base, off, &cur)) return false;
    *reinterpret_cast<T*>(base + off) = v; return true;
}
inline bool borderOk(uintptr_t ctl) {   // Mod3 setFill/setAlpha border-vtable gate (ForcePowerHotbar.cpp 446-465)
    uint32_t vt = 0; return rd32(ctl + Label_Border, 0, &vt) && vt == BorderVtable;
}
inline bool textObjOk(uintptr_t ctl) {   // Mod14 textObjOk 315
    uint32_t vt = 0, rend = 0; uintptr_t tx = ctl + Label_Text;
    return rd32(tx, 0, &vt) && vt == TextVtable && rd32(tx, Text_Renderer, &rend) && rend;
}
inline void* textSub(uintptr_t ctl) { return reinterpret_cast<void*>(ctl + Label_Text + Text_StringSub); }   // = label+0xF0
}

// ---- creation ----
// Creates a blank label from the layout template `templateTag` (e.g. "LBL_QUEUE1"; recipe = Mod 3 createLabel 982-1043).
// NOT appended to the panel (call appendToPanel). Created hidden + click-through (flags bit1 clear, 0x20 set).
// ONLY valid while the panel's layout GFF is open: inside the panel ctor, before StopLoadFromLayout (0x0040F5A0).
// Returns the label, or 0 (allocation failed, ctor vtable wrong, or the template tag is not in the layout).
inline uintptr_t createLabelFromTemplate(uintptr_t panel, const char* templateTag) {
    using namespace detail;
    if (!panel || !templateTag || !g_gk.alloc || !g_gk.labelCtor || !g_gk.initControl || !g_gk.exoCtor || !g_gk.exoDtor) return 0;
    unsigned char* mem = static_cast<unsigned char*>(g_gk.alloc(LabelSize));
    if (!mem) return 0;
    memset(mem, 0, LabelSize);
    g_gk.labelCtor(mem);
    uintptr_t c = reinterpret_cast<uintptr_t>(mem);
    uint32_t vt = 0;
    if (!rd32(c, 0, &vt) || vt != LabelVtable) return 0;
    if (!wr<int>(c, Ctl_Id, IdSentinel)) return 0;
    char exo[16] = {0};
    g_gk.exoCtor(exo, templateTag);
    g_gk.initControl(reinterpret_cast<void*>(panel), mem, exo, 0, 1);   // addToList = 0, scale = 1 (Mod 3 1031)
    g_gk.exoDtor(exo);
    int id = 0;
    if (!rdInt(c, Ctl_Id, &id) || id == IdSentinel) return 0;           // template not in the layout
    int index = 0;
    if (!rdInt(panel, Panel_CtlCount, &index)) return 0;
    wr<int>(c, Ctl_Id, index);                                          // provisional; appendToPanel re-stamps it
    int fl = 0;
    if (!rdInt(c, Ctl_Flags, &fl)) return 0;
    wr<int>(c, Ctl_Flags, (fl & ~FlagVisible) | FlagClickThrough);     // hidden + click-through until the caller shows it (Mod 3 1034)
    return c;
}

// Appends ctl to the panel's control list (list {ptr,count,cap} at panel+0x24; Mod 3 1039-1043). id := old count so id == list
// index. Verifies count+1 and array[index]==ctl. Draw order = append order.
inline bool appendToPanel(uintptr_t panel, uintptr_t ctl) {
    using namespace detail;
    if (!panel || !ctl || !g_gk.listAdd) return false;
    int index = 0;
    if (!rdInt(panel, Panel_CtlCount, &index) || index < 0) return false;
    if (!wr<int>(ctl, Ctl_Id, index)) return false;
    g_gk.listAdd(reinterpret_cast<void*>(panel + Panel_CtlArray), reinterpret_cast<void*>(ctl));
    int cnt = 0; uint32_t arr = 0, slot = 0;
    if (!rdInt(panel, Panel_CtlCount, &cnt) || cnt != index + 1) return false;
    if (!rd32(panel, Panel_CtlArray, &arr) || !rd32(arr, index * 4, &slot)) return false;
    return slot == static_cast<uint32_t>(ctl);
}

// ---- geometry / flags ----
inline bool getExtent(uintptr_t ctl, int* x, int* y, int* w, int* h) {   // engine px (Mod 3 wHit 729-733, Mod 14 ctlRect 301)
    using namespace detail;
    int e[4];
    if (!ctl) return false;
    for (int k = 0; k < 4; ++k) if (!rdInt(ctl, Ctl_X + 4 * k, &e[k])) return false;
    if (x) *x = e[0];
    if (y) *y = e[1];
    if (w) *w = e[2];
    if (h) *h = e[3];
    return true;
}

// Sets the control rect via vtable slot 1 (Mod 3 1021 / Mod 14 setCtlRect 311).
inline bool setRect(uintptr_t ctl, int x, int y, int w, int h) {
    using namespace detail;
    uint32_t vt = 0, fn = 0;
    if (!ctl || !g_gk.callSetRect || !rd32(ctl, 0, &vt) || !vt || !rd32(vt, 4, &fn) || !fn) return false;
    int r[4] = { x, y, w, h };
    g_gk.callSetRect(fn, ctl, r);
    return true;
}

inline bool setClickable(uintptr_t ctl, bool clickable) {   // clickable = bit5 CLEAR (clicks hit the control, not the world)
    using namespace detail;
    int fl = 0; if (!ctl || !rdInt(ctl, Ctl_Flags, &fl)) return false;
    int nf = clickable ? (fl & ~FlagClickThrough) : (fl | FlagClickThrough);
    return nf == fl ? true : wr<int>(ctl, Ctl_Flags, nf);
}

// bit1 = visible. Shown -> 0x20 cleared (control swallows clicks so they never reach the world); hidden -> 0x20 set (Mod 3 setVisible 436-445).
inline bool setVisible(uintptr_t ctl, bool vis) {
    using namespace detail;
    int fl = 0; if (!ctl || !rdInt(ctl, Ctl_Flags, &fl)) return false;
    int nf = vis ? ((fl | FlagVisible) & ~FlagClickThrough) : ((fl & ~FlagVisible) | FlagClickThrough);
    return nf == fl ? true : wr<int>(ctl, Ctl_Flags, nf);
}
inline bool isVisible(uintptr_t ctl) { int fl = 0; return ctl && detail::rdInt(ctl, Ctl_Flags, &fl) && (fl & FlagVisible); }

// ---- text ----
inline bool setText(uintptr_t ctl, const char* text) {   // Mod 3 text 0x416E30 w/ ECX = label+0xF0; Mod 14 setText 377 (textObjOk gate)
    using namespace detail;
    if (!ctl || !text || !g_gk.exoCtor || !g_gk.exoDtor || !g_gk.textSet || !textObjOk(ctl)) return false;
    char exo[16] = {0};
    g_gk.exoCtor(exo, text);
    g_gk.textSet(textSub(ctl), exo);
    g_gk.exoDtor(exo);
    return true;
}
inline bool setFont(uintptr_t ctl, const char* resref) {   // Mod 3 1022
    using namespace detail;
    if (!ctl || !resref || !g_gk.setFont || !textObjOk(ctl)) return false;
    char rr[16] = {0}; strncpy(rr, resref, sizeof rr - 1);
    g_gk.setFont(textSub(ctl), rr);
    return true;
}
inline bool setAlign(uintptr_t ctl, unsigned align) {   // 9 = top-left, 17 = top-centre, 18 = centre (Mod 3 1033)
    using namespace detail;
    if (!ctl || !g_gk.setAlign || !textObjOk(ctl)) return false;
    g_gk.setAlign(textSub(ctl), align);
    return true;
}
inline bool getTextColor(uintptr_t ctl, float* rgb) {   // readback gate (Mod 14 1140-1150): colour lives at label+0xF0+0x2C
    using namespace detail;
    if (!ctl || !rgb || !textObjOk(ctl)) return false;
    for (int k = 0; k < 3; ++k) if (!rd<float>(ctl + Label_Text + Text_StringSub + Text_Color, 4 * k, &rgb[k])) return false;
    return true;
}
inline bool setTextColor(uintptr_t ctl, float r, float g, float b) {   // Mod 14 setTextColor 388 (0x417140). Gate callers with getTextColor.
    using namespace detail;
    if (!ctl || !g_gk.textColor || !textObjOk(ctl)) return false;
    float v[3] = { r, g, b };
    g_gk.textColor(textSub(ctl), v);
    return true;
}

// ---- border: fill image / tint / alpha ----
// Image wrapper for a resref (lowercased, pooled one per resref, never released: Mod 14 imageFor 289). Returns 0 on failure.
inline void* imageFor(const char* resref) {
    struct E { char name[17]; void* im; };
    static E pool[128]; static int n = 0;
    if (!resref || !*resref || !g_gk.loadImage) return nullptr;
    char buf[20] = {0}; strncpy(buf, resref, 16);
    for (char* p = buf; *p; ++p) *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
    for (int i = 0; i < n; ++i) if (!strcmp(pool[i].name, buf)) return pool[i].im;
    if (n >= 128) return nullptr;
    void* im = g_gk.loadImage(buf);
    memset(&pool[n], 0, sizeof pool[n]); strncpy(pool[n].name, buf, 16); pool[n].im = im; ++n;
    return im;
}
// border+0x74 = image (nullptr clears it); tint (optional rgb) -> border+0x28. Refused unless the border vtable is 0x9875BC.
// Gate tint with getBorderTint first, as Mod 14 does.
inline bool setFill(uintptr_t ctl, void* image, const float* tint = nullptr) {
    using namespace detail;
    if (!ctl || !borderOk(ctl)) return false;
    uintptr_t b = ctl + Label_Border;
    if (!wr<uint32_t>(b, Border_Fill, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(image)))) return false;
    if (tint) for (int k = 0; k < 3; ++k) if (!wr<float>(b, Border_Tint + 4 * k, tint[k])) return false;
    return true;
}
inline bool getBorderTint(uintptr_t ctl, float* rgb) {
    using namespace detail;
    if (!ctl || !rgb || !borderOk(ctl)) return false;
    for (int k = 0; k < 3; ++k) if (!rd<float>(ctl + Label_Border, Border_Tint + 4 * k, &rgb[k])) return false;
    return true;
}
inline bool setAlpha(uintptr_t ctl, float a) {   // border+0x24 (Mod 3 setAlpha 457)
    using namespace detail;
    return ctl && borderOk(ctl) && wr<float>(ctl + Label_Border, Border_Alpha, a);
}
inline bool getAlpha(uintptr_t ctl, float* a) {
    using namespace detail;
    return ctl && a && borderOk(ctl) && rd<float>(ctl + Label_Border, Border_Alpha, a);
}

// ---- layout units -> engine pixels (Mod 14 ux/uy 368-375, scale from LBL_EFX_BACK 1130) ----
// Pixel = ox + (unit - refUnit) * s, where (ox, oy, sx, sy) come from a reference label's pixel rect.
struct Scale { float sx, sy, ox, oy; int refX, refY; bool ok; };
inline Scale scaleFromRef(uintptr_t refCtl, int refUnitX, int refUnitY, int refUnitW, int refUnitH) {
    Scale s = { 1.f, 1.f, 0.f, 0.f, refUnitX, refUnitY, false };
    int x, y, w, h;
    if (refUnitW <= 0 || refUnitH <= 0 || !getExtent(refCtl, &x, &y, &w, &h) || w <= 0 || h <= 0) return s;
    s.sx = static_cast<float>(w) / refUnitW; s.sy = static_cast<float>(h) / refUnitH;
    s.ox = static_cast<float>(x); s.oy = static_cast<float>(y);
    s.ok = true;
    return s;
}
inline int unitX(const Scale& s, float u) { return static_cast<int>(lroundf(s.ox + (u - s.refX) * s.sx)); }
inline int unitY(const Scale& s, float u) { return static_cast<int>(lroundf(s.oy + (u - s.refY) * s.sy)); }
// Rect in layout units -> engine px; width/height are the difference of rounded edges (no gaps between neighbours), as Mod 14 place().
inline void unitRect(const Scale& s, int ux, int uy, int uw, int uh, int* x, int* y, int* w, int* h) {
    int x0 = unitX(s, static_cast<float>(ux)), y0 = unitY(s, static_cast<float>(uy));
    if (x) *x = x0;
    if (y) *y = y0;
    if (w) *w = unitX(s, static_cast<float>(ux + uw)) - x0;
    if (h) *h = unitY(s, static_cast<float>(uy + uh)) - y0;
}

// ---- input ----
// Cursor in ENGINE pixels: OS client position scaled by framebuffer / client size (Mod 14 cursorEngine 256; the framebuffer
// globals 0x9F42A4/A8 as syncScreen). If the globals are unreadable, falls back to client px (Mod 3 cursorInGame 476, which is 1:1).
inline bool cursorEngine(int* x, int* y) {
    int cx, cy, cw, ch;
    if (!g_gk.cursorClient || !x || !y || !g_gk.cursorClient(&cx, &cy, &cw, &ch) || cw < 1 || ch < 1) return false;
    int sw = 0, sh = 0;
    detail::rdInt(g_gk.screenWAddr, 0, &sw); detail::rdInt(g_gk.screenHAddr, 0, &sh);
    if (sw >= 320 && sw <= 16384 && sh >= 200 && sh <= 16384) { *x = static_cast<int>(static_cast<long long>(cx) * sw / cw); *y = static_cast<int>(static_cast<long long>(cy) * sh / ch); }
    else { *x = cx; *y = cy; }
    return true;
}
// Visible (bit1) and (x,y) inside [ext.x, ext.x+w) x [ext.y, ext.y+h) (Mod 3 wHit 729-733, which used a cached visible flag).
inline bool hit(uintptr_t ctl, int x, int y) {
    int ex, ey, ew, eh;
    return isVisible(ctl) && getExtent(ctl, &ex, &ey, &ew, &eh) && x >= ex && x < ex + ew && y >= ey && y < ey + eh;
}
struct ClickEdge { bool prev = false; };
// True exactly once per press: LMB down AND the game window is this process's foreground window (Mod 3 updateEditor 829-832).
// Not-foreground counts as "up", so a press that started elsewhere never fires. AND the result with your own gate.
inline bool clickEdge(ClickEdge& e) {
    bool down = g_gk.gameForeground && g_gk.lbuttonDown && g_gk.gameForeground() && g_gk.lbuttonDown();
    bool click = down && !e.prev;
    e.prev = down;
    return click;
}
// The native quick-button mouse-enter sound (Mod 3 playHoverSound 608). False when the sound manager is null.
inline bool playHoverSound() {
    uint32_t mgr = 0;
    if (!g_gk.playSound || !detail::rd32(g_gk.guiSoundMgrAddr, 0, &mgr) || !mgr) return false;
    g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgr)), 1);
    return true;
}

// ---- Win32 platform defaults (game window lookup as Mod 14 gameWindow 241: largest visible window of this pid, > 320x240 per Mod 3) ----
#ifndef GUIKIT_HOST
namespace detail {
struct EnumCtx { DWORD pid; HWND best; long area; };
inline BOOL CALLBACK enumProc(HWND h, LPARAM lp) {
    EnumCtx* c = reinterpret_cast<EnumCtx*>(lp);
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid != c->pid || !IsWindowVisible(h)) return TRUE;
    RECT r; if (!GetClientRect(h, &r) || r.right <= 320 || r.bottom <= 240) return TRUE;
    long a = r.right * r.bottom;
    if (a > c->area) { c->area = a; c->best = h; }
    return TRUE;
}
inline HWND gameWindow() {
    static HWND hwnd = nullptr; static DWORD lastTry = 0;
    if (hwnd && IsWindow(hwnd)) return hwnd;
    DWORD now = GetTickCount();
    if (lastTry && now - lastTry < 2000) return nullptr;   // failed lookups retry every 2 s, not every frame
    lastTry = now;
    EnumCtx c = { GetCurrentProcessId(), nullptr, 0 };
    EnumWindows(enumProc, reinterpret_cast<LPARAM>(&c));
    hwnd = c.best;
    return hwnd;
}
inline bool winCursorClient(int* cx, int* cy, int* cw, int* ch) {
    HWND h = gameWindow(); if (!h) return false;
    POINT pt; RECT r;
    if (!GetCursorPos(&pt) || !ScreenToClient(h, &pt) || !GetClientRect(h, &r)) return false;
    *cx = pt.x; *cy = pt.y; *cw = r.right; *ch = r.bottom;
    return true;
}
inline bool winForeground() {
    HWND fg = GetForegroundWindow(); DWORD pid = 0;
    if (!fg) return false;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
}
inline bool winLButton() { return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; }
struct InstallPlatform { InstallPlatform() { g_gk.cursorClient = winCursorClient; g_gk.gameForeground = winForeground; g_gk.lbuttonDown = winLButton; } };
inline InstallPlatform g_installPlatform;
}
#endif

} // namespace guikit
