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
// Per-version addresses/offsets: the Addr* / vtable / offset constants below are the Steam column (K2Addr.h kSteam); every helper reads
// the table selected for the running exe (k2addr::get()) and does nothing / returns false when the table is null or a field it
// needs is 0 (never a call into address 0, never a guessed offset). Engine::rebind() re-reads the table for the engine-call fields.
// Host tests: define GUIKIT_HOST before including to drop the Win32 platform defaults; every engine call and platform
// query is a function pointer in guikit::Engine so a test substitutes fakes (see research/companion_levelup/tools/parity/guikit_test.cpp).
#pragma once
#include "SafeRead.h"
#include "K2Addr.h"
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifndef GUIKIT_HOST
#include "WindowHook.h"   // kp::WindowHook for the wheel subclass (included here, not inside namespace guikit)
#endif

namespace guikit {

// ---- engine addresses / offsets (Steam Aspyr) ----
constexpr uintptr_t AddrNew = gameaddr::New;                  // cdecl(size)                                   Mod3 FnNew 52
constexpr uintptr_t AddrLabelCtor = gameaddr::LabelCtor;      // CSWGuiLabel ctor, ECX=this, no stack args     Mod3 FnLabelCtor 49
constexpr uintptr_t AddrInitControl = gameaddr::InitControl;  // thiscall(panel, ctl, CExoString*, addToList, scale) RET 16   Mod3 50
constexpr uintptr_t AddrStopLoad = gameaddr::StopLoadFromLayout;   // StopLoadFromLayout (createLabelFromTemplate must run BEFORE this)
constexpr uintptr_t AddrExoCtor = gameaddr::ExoCtor, AddrExoDtor = gameaddr::ExoDtor;   // Mod3 51
constexpr uintptr_t AddrTextSet = gameaddr::TextSet;          // thiscall(label+0xF0, CExoString*)             Mod3 FnTextSet 77
constexpr uintptr_t AddrSetFont = gameaddr::SetFont;          // thiscall(label+0xF0, char resref[16]) RET 4   Mod3 970
constexpr uintptr_t AddrSetAlign = gameaddr::SetAlign;        // thiscall(label+0xF0, unsigned) RET 4          Mod3 971
constexpr uintptr_t AddrTextColor = gameaddr::TextColor;      // thiscall(label+0xF0, float rgb[3]) RET 4      Mod14 FnTextColor 47
constexpr uintptr_t AddrPtrListAdd = gameaddr::PtrListAdd;    // CExoArrayList<void*>::Add thiscall(list, v) RET 4   Mod3 969
constexpr uintptr_t AddrLoadImage = gameaddr::GuiLoadImage;   // cdecl(resref buf) -> image*                   Mod3 53 / Mod14 46
constexpr uintptr_t AddrGuiSoundMgr = gameaddr::GuiSoundMgr, AddrPlayGuiSound = gameaddr::PlayGuiSound;   // thiscall(mgr, char idx), idx 1 = hover   Mod3 78
constexpr uintptr_t AddrScreenW = gameaddr::ScreenW, AddrScreenH = gameaddr::ScreenH;           // engine framebuffer size   Mod14 syncScreen 218
constexpr uint32_t LabelVtable = gameaddr::LabelVtable, BorderVtable = gameaddr::BorderVtable, TextVtable = gameaddr::TextVtable;
constexpr int LabelSize = 0x148;
constexpr int Ctl_Flags = 0x48, Ctl_Id = 0x54, Ctl_X = 4, Ctl_Y = 8, Ctl_W = 0xC, Ctl_H = 0x10;
constexpr int Label_Border = 0x60, Label_Text = 0xD8, Text_Renderer = 0x14, Text_StringSub = 0x18, Text_Color = 0x2C;
constexpr int Border_Alpha = 0x24, Border_Tint = 0x28, Border_Fill = 0x74, Border_FillStyle = 0x34;
constexpr int Panel_CtlArray = 0x24, Panel_CtlCount = 0x28;
constexpr int FlagVisible = 0x2, FlagClickThrough = 0x20;
constexpr int ImagePoolSize = 256;                   // imageFor pool capacity (one entry per distinct resref)
enum CreateFail { CF_None = 0, CF_Alloc, CF_Vtable, CF_Template, CF_Count, CF_Flags };   // createLabelFromTemplate failure reason
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
    // Engine calls: the K2Addr.h table's function for this exe, nullptr when the table is null or the address is 0 (every helper
    // already refuses on a null pointer). Bound when g_gk is constructed; rebind() re-reads the table.
    NewFn alloc = nullptr;
    CtorFn labelCtor = nullptr;
    InitCtlFn initControl = nullptr;
    ExoCtorFn exoCtor = nullptr;
    ExoDtorFn exoDtor = nullptr;
    TextSetFn textSet = nullptr;
    SetResFn setFont = nullptr;
    SetUIntFn setAlign = nullptr;
    SetColorFn textColor = nullptr;
    ListAddFn listAdd = nullptr;
    LoadImageFn loadImage = nullptr;
    PlaySoundFn playSound = nullptr;
    // vtable slot 1 = SetRect. `fn` is the slot's value read from engine memory (a host test ignores it and records the call).
    void (*callSetRect)(uint32_t fn, uintptr_t ctl, int* xywh) = [](uint32_t fn, uintptr_t ctl, int* r) {
        reinterpret_cast<void(__thiscall*)(void*, int*)>(static_cast<uintptr_t>(fn))(reinterpret_cast<void*>(ctl), r);
    };
    uintptr_t guiSoundMgrAddr = 0, screenWAddr = 0, screenHAddr = 0;   // 0 = unknown for this exe (helpers skip the read)
    // Platform queries (defaults below under !GUIKIT_HOST). Client size is the game window's client rect in OS pixels.
    bool (*cursorClient)(int* cx, int* cy, int* cw, int* ch) = nullptr;   // cursor in game-window client px + client size; false = none
    bool (*gameForeground)() = nullptr;                                   // game window is the foreground window of this process
    bool (*lbuttonDown)() = nullptr;                                      // GetAsyncKeyState(VK_LBUTTON) & 0x8000
    bool (*rbuttonDown)() = nullptr;                                      // GetAsyncKeyState(VK_RBUTTON) & 0x8000

    Engine() { rebind(); }
    // Re-point the engine-call and global-address fields at the current K2Addr.h table (also resets test fakes in those fields).
    void rebind() {
        using k2addr::fnptr; using k2addr::addr;
        alloc = fnptr<NewFn>(&K2AddrTable::New);
        labelCtor = fnptr<CtorFn>(&K2AddrTable::LabelCtor);
        initControl = fnptr<InitCtlFn>(&K2AddrTable::InitControl);
        exoCtor = fnptr<ExoCtorFn>(&K2AddrTable::ExoCtor);
        exoDtor = fnptr<ExoDtorFn>(&K2AddrTable::ExoDtor);
        textSet = fnptr<TextSetFn>(&K2AddrTable::TextSet);
        setFont = fnptr<SetResFn>(&K2AddrTable::SetFont);
        setAlign = fnptr<SetUIntFn>(&K2AddrTable::SetAlign);
        textColor = fnptr<SetColorFn>(&K2AddrTable::TextColor);
        listAdd = fnptr<ListAddFn>(&K2AddrTable::PtrListAdd);
        loadImage = fnptr<LoadImageFn>(&K2AddrTable::GuiLoadImage);
        playSound = fnptr<PlaySoundFn>(&K2AddrTable::PlayGuiSound);
        guiSoundMgrAddr = addr(&K2AddrTable::GuiSoundMgr);
        screenWAddr = addr(&K2AddrTable::ScreenW);
        screenHAddr = addr(&K2AddrTable::ScreenH);
    }
};
inline Engine g_gk;   // substitutable by tests; defaults come from the K2Addr.h table for the running exe (Steam when KOTOR_VERSION_SHA is unset)

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
// Table fields for this exe (0 = unknown): struct offsets and vtable addresses.
inline int off(int K2AddrTable::* m) { return k2addr::off(m); }
inline uint32_t vtab(uintptr_t K2AddrTable::* m) { return static_cast<uint32_t>(k2addr::addr(m)); }
inline bool borderOk(uintptr_t ctl) {   // Mod3 setFill/setAlpha border-vtable gate (ForcePowerHotbar.cpp 446-465)
    uint32_t vt = 0, want = vtab(&K2AddrTable::BorderVtable); int ob = off(&K2AddrTable::Label_Border);
    return want && ob && rd32(ctl + ob, 0, &vt) && vt == want;
}
inline bool textObjOk(uintptr_t ctl) {   // Mod14 textObjOk 315
    uint32_t vt = 0, rend = 0, want = vtab(&K2AddrTable::TextVtable);
    int ot = off(&K2AddrTable::Label_Text), orr = off(&K2AddrTable::Text_Renderer);
    if (!want || !ot || !orr) return false;
    uintptr_t tx = ctl + ot;
    return rd32(tx, 0, &vt) && vt == want && rd32(tx, orr, &rend) && rend;
}
// label+0xF0 (only meaningful after textObjOk passed, which proves both offsets are known)
inline void* textSub(uintptr_t ctl) { return reinterpret_cast<void*>(ctl + off(&K2AddrTable::Label_Text) + off(&K2AddrTable::Text_StringSub)); }
}

// ---- creation ----
// Creates a blank label from the layout template `templateTag` (e.g. "LBL_QUEUE1"; recipe = Mod 3 createLabel 982-1043).
// NOT appended to the panel (call appendToPanel). Created hidden + click-through (flags bit1 clear, 0x20 set).
// ONLY valid while the panel's layout GFF is open: inside the panel ctor, before StopLoadFromLayout (0x0040F5A0).
// Returns the label, or 0 (allocation failed, ctor vtable wrong, or the template tag is not in the layout).
// Optional out-params: *why = CreateFail reason (CF_None on success); *badVtable = the vtable read when why == CF_Vtable.
inline uintptr_t createLabelFromTemplate(uintptr_t panel, const char* templateTag, int* why = nullptr, uint32_t* badVtable = nullptr) {
    using namespace detail;
    if (why) *why = CF_None;
    if (badVtable) *badVtable = 0;
    const int labelSize = off(&K2AddrTable::LabelSize), oId = off(&K2AddrTable::Ctl_Id), oFlags = off(&K2AddrTable::Ctl_Flags);
    const int oCount = off(&K2AddrTable::Panel_CtlCount);
    const uint32_t labelVt = vtab(&K2AddrTable::LabelVtable);
    if (!panel || !templateTag || !g_gk.alloc || !g_gk.labelCtor || !g_gk.initControl || !g_gk.exoCtor || !g_gk.exoDtor ||
        labelSize <= 0 || !oId || !oFlags || !oCount || !labelVt) { if (why) *why = CF_Alloc; return 0; }
    unsigned char* mem = static_cast<unsigned char*>(g_gk.alloc(labelSize));
    if (!mem) { if (why) *why = CF_Alloc; return 0; }
    memset(mem, 0, labelSize);
    g_gk.labelCtor(mem);
    uintptr_t c = reinterpret_cast<uintptr_t>(mem);
    uint32_t vt = 0;
    if (!rd32(c, 0, &vt) || vt != labelVt) { if (why) *why = CF_Vtable; if (badVtable) *badVtable = vt; return 0; }
    if (!wr<int>(c, oId, IdSentinel)) { if (why) *why = CF_Vtable; return 0; }
    char exo[16] = {0};
    g_gk.exoCtor(exo, templateTag);
    g_gk.initControl(reinterpret_cast<void*>(panel), mem, exo, 0, 1);   // addToList = 0, scale = 1 (Mod 3 1031)
    g_gk.exoDtor(exo);
    int id = 0;
    if (!rdInt(c, oId, &id) || id == IdSentinel) { if (why) *why = CF_Template; return 0; }   // template not in the layout
    int index = 0;
    if (!rdInt(panel, oCount, &index)) { if (why) *why = CF_Count; return 0; }
    wr<int>(c, oId, index);                                             // provisional; appendToPanel re-stamps it
    int fl = 0;
    if (!rdInt(c, oFlags, &fl)) { if (why) *why = CF_Flags; return 0; }
    wr<int>(c, oFlags, (fl & ~FlagVisible) | FlagClickThrough);     // hidden + click-through until the caller shows it (Mod 3 1034)
    return c;
}

// Appends ctl to the panel's control list (list {ptr,count,cap} at panel+0x24; Mod 3 1039-1043). id := old count so id == list
// index. Verifies count+1 and array[index]==ctl. Draw order = append order.
inline bool appendToPanel(uintptr_t panel, uintptr_t ctl) {
    using namespace detail;
    const int oCount = off(&K2AddrTable::Panel_CtlCount), oArr = off(&K2AddrTable::Panel_CtlArray), oId = off(&K2AddrTable::Ctl_Id);
    if (!panel || !ctl || !g_gk.listAdd || !oCount || !oArr || !oId) return false;
    int index = 0;
    if (!rdInt(panel, oCount, &index) || index < 0) return false;
    if (!wr<int>(ctl, oId, index)) return false;
    g_gk.listAdd(reinterpret_cast<void*>(panel + oArr), reinterpret_cast<void*>(ctl));
    int cnt = 0; uint32_t arr = 0, slot = 0;
    if (!rdInt(panel, oCount, &cnt) || cnt != index + 1) return false;
    if (!rd32(panel, oArr, &arr) || !rd32(arr, index * 4, &slot)) return false;
    return slot == static_cast<uint32_t>(ctl);
}

// ---- geometry / flags ----
inline bool getExtent(uintptr_t ctl, int* x, int* y, int* w, int* h) {   // engine px (Mod 3 wHit 729-733, Mod 14 ctlRect 301)
    using namespace detail;
    int e[4];
    const int o[4] = { off(&K2AddrTable::Ctl_X), off(&K2AddrTable::Ctl_Y), off(&K2AddrTable::Ctl_W), off(&K2AddrTable::Ctl_H) };
    if (!ctl) return false;
    for (int k = 0; k < 4; ++k) if (!o[k] || !rdInt(ctl, o[k], &e[k])) return false;
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
    const int oSlot = off(&K2AddrTable::Ctl_VtSetRect);
    if (!ctl || !g_gk.callSetRect || !oSlot || !rd32(ctl, 0, &vt) || !vt || !rd32(vt, oSlot, &fn) || !fn) return false;
    int r[4] = { x, y, w, h };
    g_gk.callSetRect(fn, ctl, r);
    return true;
}

inline bool setClickable(uintptr_t ctl, bool clickable) {   // clickable = bit5 CLEAR (clicks hit the control, not the world)
    using namespace detail;
    const int oFlags = off(&K2AddrTable::Ctl_Flags);
    int fl = 0; if (!ctl || !oFlags || !rdInt(ctl, oFlags, &fl)) return false;
    int nf = clickable ? (fl & ~FlagClickThrough) : (fl | FlagClickThrough);
    return nf == fl ? true : wr<int>(ctl, oFlags, nf);
}

// bit1 = visible. Shown -> 0x20 cleared (control swallows clicks so they never reach the world); hidden -> 0x20 set (Mod 3 setVisible 436-445).
inline bool setVisible(uintptr_t ctl, bool vis) {
    using namespace detail;
    const int oFlags = off(&K2AddrTable::Ctl_Flags);
    int fl = 0; if (!ctl || !oFlags || !rdInt(ctl, oFlags, &fl)) return false;
    int nf = vis ? ((fl | FlagVisible) & ~FlagClickThrough) : ((fl & ~FlagVisible) | FlagClickThrough);
    return nf == fl ? true : wr<int>(ctl, oFlags, nf);
}
inline bool isVisible(uintptr_t ctl) {
    int fl = 0, oFlags = detail::off(&K2AddrTable::Ctl_Flags);
    return ctl && oFlags && detail::rdInt(ctl, oFlags, &fl) && (fl & FlagVisible);
}

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
    const int oc = off(&K2AddrTable::Text_Color);
    if (!ctl || !rgb || !oc || !textObjOk(ctl)) return false;
    for (int k = 0; k < 3; ++k) if (!rd<float>(reinterpret_cast<uintptr_t>(textSub(ctl)) + oc, 4 * k, &rgb[k])) return false;
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
// Optional out-params: *loaded = true only when this call invoked loadImage (new pool entry, even if it returned null);
// *poolCount = pool entries after the call (every path). Pool full == !loaded && result==null && *poolCount >= ImagePoolSize.
inline void* imageFor(const char* resref, bool* loaded = nullptr, int* poolCount = nullptr) {
    struct E { char name[17]; void* im; };
    static E pool[ImagePoolSize]; static int n = 0;
    if (loaded) *loaded = false;
    if (poolCount) *poolCount = n;
    if (!resref || !*resref || !g_gk.loadImage) return nullptr;
    char buf[20] = {0}; strncpy(buf, resref, 16);
    for (char* p = buf; *p; ++p) *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
    for (int i = 0; i < n; ++i) if (!strcmp(pool[i].name, buf)) return pool[i].im;
    if (n >= ImagePoolSize) return nullptr;
    void* im = g_gk.loadImage(buf);
    memset(&pool[n], 0, sizeof pool[n]); strncpy(pool[n].name, buf, 16); pool[n].im = im; ++n;
    if (loaded) *loaded = true;
    if (poolCount) *poolCount = n;
    return im;
}
// border+0x74 = image (nullptr clears it); tint (optional rgb) -> border+0x28. Refused unless the border vtable is 0x9875BC.
// Gate tint with getBorderTint first, as Mod 14 does.
inline bool setFill(uintptr_t ctl, void* image, const float* tint = nullptr) {
    using namespace detail;
    const int oFill = off(&K2AddrTable::Border_Fill), oTint = off(&K2AddrTable::Border_Tint);
    if (!ctl || !oFill || !oTint || !borderOk(ctl)) return false;
    uintptr_t b = ctl + off(&K2AddrTable::Label_Border);
    if (!wr<uint32_t>(b, oFill, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(image)))) return false;
    if (tint) for (int k = 0; k < 3; ++k) if (!wr<float>(b, oTint + 4 * k, tint[k])) return false;
    return true;
}
inline bool getBorderTint(uintptr_t ctl, float* rgb) {
    using namespace detail;
    const int oTint = off(&K2AddrTable::Border_Tint);
    if (!ctl || !rgb || !oTint || !borderOk(ctl)) return false;
    for (int k = 0; k < 3; ++k) if (!rd<float>(ctl + off(&K2AddrTable::Label_Border), oTint + 4 * k, &rgb[k])) return false;
    return true;
}
// border+0x28 = rgb. Draw 0x415300 tints corners, edges and fill with it, so this recolours the whole frame
// (vanilla HILIGHT is the same images with another COLOR).
inline bool setBorderTint(uintptr_t ctl, const float* rgb) {
    using namespace detail;
    const int oTint = off(&K2AddrTable::Border_Tint);
    if (!ctl || !rgb || !oTint || !borderOk(ctl)) return false;
    for (int k = 0; k < 3; ++k) if (!wr<float>(ctl + off(&K2AddrTable::Label_Border), oTint + 4 * k, rgb[k])) return false;
    return true;
}
inline bool setAlpha(uintptr_t ctl, float a) {   // border+0x24 (Mod 3 setAlpha 457)
    using namespace detail;
    const int oAlpha = off(&K2AddrTable::Border_Alpha);
    return ctl && oAlpha && borderOk(ctl) && wr<float>(ctl + off(&K2AddrTable::Label_Border), oAlpha, a);
}
inline bool getAlpha(uintptr_t ctl, float* a) {
    using namespace detail;
    const int oAlpha = off(&K2AddrTable::Border_Alpha);
    return ctl && a && oAlpha && borderOk(ctl) && rd<float>(ctl + off(&K2AddrTable::Label_Border), oAlpha, a);
}
// border+0x34 bits 0-1 = FILLSTYLE (0 tile, 1 centre, 2 stretch); the other bits are kept. Doc 02 RE notes 672/1661, doc 08 499.
inline bool setFillStyle(uintptr_t ctl, int style) {
    using namespace detail;
    const int oStyle = off(&K2AddrTable::Border_FillStyle);
    if (!ctl || !oStyle || style < 0 || style > 3 || !borderOk(ctl)) return false;
    uintptr_t b = ctl + off(&K2AddrTable::Label_Border); uint32_t v = 0;
    return rd<uint32_t>(b, oStyle, &v) && wr<uint32_t>(b, oStyle, (v & ~3u) | static_cast<uint32_t>(style));
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
    if (g_gk.screenWAddr) detail::rdInt(g_gk.screenWAddr, 0, &sw);
    if (g_gk.screenHAddr) detail::rdInt(g_gk.screenHAddr, 0, &sh);
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
// Same as clickEdge for the right mouse button (own ClickEdge state per button).
inline bool rclickEdge(ClickEdge& e) {
    bool down = g_gk.gameForeground && g_gk.rbuttonDown && g_gk.gameForeground() && g_gk.rbuttonDown();
    bool click = down && !e.prev;
    e.prev = down;
    return click;
}
// The native quick-button mouse-enter sound (Mod 3 playHoverSound 608). False when the sound manager is null.
inline bool playHoverSound() {
    uint32_t mgr = 0;
    if (!g_gk.playSound || !g_gk.guiSoundMgrAddr || !detail::rd32(g_gk.guiSoundMgrAddr, 0, &mgr) || !mgr) return false;
    g_gk.playSound(reinterpret_cast<void*>(static_cast<uintptr_t>(mgr)), 1);
    return true;
}

// ---- screen size / float cursor / mouse wheel / bind-by-tag (additive; factored from the copies in chareffects (A), inv3d (I), equip3d (E)) ----
// Pure parts (screenValid, pickScreen, clientToEngineF, wheel*, rectContainsF, bindFlags) have no engine or Win32 dependency and are host-tested.
inline bool screenValid(int w, int h) { return w >= 320 && w <= 16384 && h >= 200 && h <= 16384; }   // A/I inline range test, E eq3d::screenValid
enum ScreenSrc { SS_None = 0, SS_Ini = 1, SS_Engine = 2 };
// Screen-size decision of the copies' syncScreen(): an INI override (iniW > 0 or iniH > 0, the copies' screenExplicit) wins and the engine
// globals are NOT consulted; each of *w / *h takes its INI value when > 0 and otherwise keeps the caller's current value (the copies' cfg
// default). No override: the engine globals replace *w/*h only when screenValid(engW, engH); otherwise *w/*h are left untouched (SS_None).
inline int pickScreen(int iniW, int iniH, int engW, int engH, int* w, int* h) {
    if (!w || !h) return SS_None;
    if (iniW > 0 || iniH > 0) { if (iniW > 0) *w = iniW; if (iniH > 0) *h = iniH; return SS_Ini; }
    if (screenValid(engW, engH)) { *w = engW; *h = engH; return SS_Engine; }
    return SS_None;
}
namespace detail {
inline int syncScreenSrc(int iniW, int iniH, int* w, int* h) {
    int ew = 0, eh = 0;
    if (!(iniW > 0 || iniH > 0)) {   // like the copies: the engine globals are only read when the INI does not override
        if (g_gk.screenWAddr) rdInt(g_gk.screenWAddr, 0, &ew);
        if (g_gk.screenHAddr) rdInt(g_gk.screenHAddr, 0, &eh);
    }
    return pickScreen(iniW, iniH, ew, eh, w, h);
}
}
// Engine framebuffer size honouring an INI override (copies' syncScreen). In/out *w,*h = the caller's current size (fallback).
// True when this call set *w/*h from the INI or the engine globals; false (values untouched) when neither was usable.
inline bool syncScreen(int iniW, int iniH, int* w, int* h) { return detail::syncScreenSrc(iniW, iniH, w, h) != SS_None; }
// Client px -> engine px in float, same arithmetic as the copies' `pt.x * screenW / r.right`. Precondition cw >= 1 && ch >= 1.
inline void clientToEngineF(int px, int py, int cw, int ch, float sw, float sh, float* x, float* y) { *x = px * sw / cw; *y = py * sh / ch; }
// Float variant of cursorEngine (same mapping, no int truncation; what the copies' cursorEngine(float*,float*) return). iniW/iniH (optional,
// default 0 = engine globals) are the INI screen override. Falls back to client px (as cursorEngine) when no usable screen size is known.
inline bool cursorEngineF(float* x, float* y, int iniW = 0, int iniH = 0) {
    int cx, cy, cw, ch;
    if (!g_gk.cursorClient || !x || !y || !g_gk.cursorClient(&cx, &cy, &cw, &ch) || cw < 1 || ch < 1) return false;
    int sw = 0, sh = 0;
    int src = detail::syncScreenSrc(iniW, iniH, &sw, &sh);
    if (src == SS_Engine || (src == SS_Ini && sw > 0 && sh > 0)) clientToEngineF(cx, cy, cw, ch, static_cast<float>(sw), static_cast<float>(sh), x, y);
    else { *x = static_cast<float>(cx); *y = static_cast<float>(cy); }
    return true;
}

// ---- mouse wheel (WheelHook) ----
// Subclasses the game window once per DLL, accumulates WM_MOUSEWHEEL deltas that land inside caller-provided engine-px rects while the caller
// keeps "ticking", and chains to the original WndProc for everything else (consumed wheel events are swallowed, as in A/I/E).
// NOTE: L4 `k2.guiinput` is the long-term owner of input subclassing; WheelHook is the interim shared copy of the three per-mod subclasses.
// Usage per tick: touch(GetTickCount()); setRect(0, x, y, w, h) (w <= 0 = inactive; engine px, top-left origin); then n = take().
// Differences between the copies, kept as parameters: A/I/E all gate on (feature enabled && tick < 300 ms old && rect w > 0) -> enabled/freshMs/rects;
// only I calls saferead::beginScope() inside the proc -> `scoped`; A/I/E log the first 20 events -> onEvent/eventLogMax. None of the copies has a
// drag guard (their drag code runs in the tick); `dragGuard` is optional (nullptr = off): while it returns true the wheel is NOT consumed and is
// chained to the game untouched.
constexpr int WheelMaxRects = 4;
constexpr int WheelAccumMax = 1 << 20;   // saturation bound of the accumulator (120 units per notch)
inline int wheelDelta(uintptr_t wParam) { return static_cast<int16_t>((wParam >> 16) & 0xFFFF); }   // (short)HIWORD(wParam)
inline int wheelPtX(uintptr_t lParam) { return static_cast<int16_t>(lParam & 0xFFFF); }             // (short)LOWORD(lParam), screen px
inline int wheelPtY(uintptr_t lParam) { return static_cast<int16_t>((lParam >> 16) & 0xFFFF); }     // (short)HIWORD(lParam)
// cur + delta saturated to +-WheelAccumMax (the copies' InterlockedExchangeAdd could wrap).
inline int wheelAddClamped(int cur, int delta) {
    long long s = static_cast<long long>(cur) + delta;
    return s > WheelAccumMax ? WheelAccumMax : s < -WheelAccumMax ? -WheelAccumMax : static_cast<int>(s);
}
// Half-open box test (left/top inclusive, right/bottom exclusive); a rect with w <= 0 or h <= 0 contains nothing.
inline bool rectContainsF(float x, float y, int rx, int ry, int rw, int rh) { return x >= rx && x < rx + rw && y >= ry && y < ry + rh; }

struct WheelHook {
    volatile int32_t accum = 0;           // summed deltas since the last take()
    volatile uint32_t lastTickMs = 0;     // touch() time
    volatile int32_t rects[WheelMaxRects][4];   // x, y, w, h in engine px; w <= 0 = inactive
    bool enabled = true;                  // A/I/E: cfg.wheelScroll / wheelZoom
    uint32_t freshMs = 300;               // wheel is only seen while touch() is younger than this (the owning panel is up)
    int iniW = 0, iniH = 0;               // INI screen override (0 = engine globals), see syncScreen
    int screenW = 3840, screenH = 1600;   // current / fallback screen size used to map the cursor
    bool scoped = false;                  // call saferead::beginScope() on entry to the Win32 proc (I does)
    bool (*dragGuard)() = nullptr;        // optional: true = a drag is active, do not consume the wheel
    void (*onEvent)(int delta, int px, int py, bool inside) = nullptr;   // optional diagnostic (first eventLogMax events)
    int eventLogMax = 20, eventLogs = 0;
    WheelHook() { for (int i = 0; i < WheelMaxRects; ++i) for (int k = 0; k < 4; ++k) rects[i][k] = 0; }
    void touch(uint32_t nowMs) { __atomic_store_n(&lastTickMs, nowMs, __ATOMIC_RELAXED); }
    void setRect(int i, int x, int y, int w, int h) {
        if (i < 0 || i >= WheelMaxRects) return;
        __atomic_store_n(&rects[i][0], x, __ATOMIC_RELAXED); __atomic_store_n(&rects[i][1], y, __ATOMIC_RELAXED);
        __atomic_store_n(&rects[i][3], h, __ATOMIC_RELAXED); __atomic_store_n(&rects[i][2], w, __ATOMIC_RELAXED);
    }
    void clearRect(int i) { if (i >= 0 && i < WheelMaxRects) __atomic_store_n(&rects[i][2], 0, __ATOMIC_RELAXED); }
    bool anyRect() const { for (int i = 0; i < WheelMaxRects; ++i) if (__atomic_load_n(&rects[i][2], __ATOMIC_RELAXED) > 0) return true; return false; }
    bool fresh(uint32_t nowMs) const { return static_cast<uint32_t>(nowMs - __atomic_load_n(&lastTickMs, __ATOMIC_RELAXED)) < freshMs; }
    // Should the proc look at this wheel event at all (the copies' leading `if`), plus the optional drag guard.
    bool gate(uint32_t nowMs) const { return enabled && fresh(nowMs) && anyRect() && !(dragGuard && dragGuard()); }
    // Engine-px point inside any active rect.
    bool inside(float x, float y) const {
        for (int i = 0; i < WheelMaxRects; ++i) {
            int rw = __atomic_load_n(&rects[i][2], __ATOMIC_RELAXED);
            if (rw > 0 && rectContainsF(x, y, __atomic_load_n(&rects[i][0], __ATOMIC_RELAXED), __atomic_load_n(&rects[i][1], __ATOMIC_RELAXED), rw, __atomic_load_n(&rects[i][3], __ATOMIC_RELAXED))) return true;
        }
        return false;
    }
    // Core of the subclass proc for one WM_MOUSEWHEEL (client px + client size, screenW/H already synced). True = consumed (accumulated, do not chain).
    bool onWheel(uint32_t nowMs, int delta, bool haveClient, int px, int py, int cw, int ch) {
        if (!gate(nowMs)) return false;
        bool in = false;
        if (haveClient && cw > 0 && ch > 0) {
            float x, y; clientToEngineF(px, py, cw, ch, static_cast<float>(screenW), static_cast<float>(screenH), &x, &y);
            in = inside(x, y);
        }
        if (onEvent && eventLogs < eventLogMax) { ++eventLogs; onEvent(delta, px, py, in); }
        if (!in) return false;
        int32_t cur = __atomic_load_n(&accum, __ATOMIC_RELAXED);
        while (!__atomic_compare_exchange_n(&accum, &cur, wheelAddClamped(cur, delta), false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {}
        return true;
    }
    int take() { return __atomic_exchange_n(&accum, 0, __ATOMIC_ACQ_REL); }   // drain; 120 per notch, positive = away from the user
};
inline WheelHook g_wheel;   // the one hook per DLL (inline variable: one instance per module)

// ---- bind a layout-created label by tag (panel ctor, before StopLoadFromLayout; copies: bindLabel A, bindCover I/E) ----
// Decision after InitControl: a control whose id differs from expectId is not the layout's (the layout lacks it): hide it (clear bit1);
// either way the control is made click-through (set bit5).
inline bool bindMatches(int id, int expectId) { return id == expectId; }
inline int bindFlags(int id, int expectId, int fl) { return (bindMatches(id, expectId) ? fl : (fl & ~FlagVisible)) | FlagClickThrough; }
struct BindInfo {   // what the copies logged ("H1 %s: ctl id expect extent flags -> flags")
    uintptr_t ctl = 0; int id = -999, flagsBefore = 0, flagsAfter = 0, ext[4] = {0, 0, 0, 0}; int why = CF_None; uint32_t badVtable = 0;
};
// alloc(labelSize) + zero + label ctor + vtable check + InitControl(panel, ctl, tag, addToList = 1, scale = 1) (so the panel list takes it,
// unlike createLabelFromTemplate) + id/flags/extent readback + bindFlags write. Returns the control iff its id == expectId, else 0 (the
// hidden control stays in the panel list, as in the copies; info->ctl still holds it). labelSize <= 0 = the K2Addr table's LabelSize
// (the copies hard-coded 0x148). `tagName` is the layout tag string (the copies' tagName), not an int.
inline uintptr_t bindByTag(uintptr_t panel, const char* tagName, int expectId, int labelSize = 0, BindInfo* info = nullptr) {
    using namespace detail;
    BindInfo bi; BindInfo& r = info ? *info : bi; r = BindInfo();
    const int size = labelSize > 0 ? labelSize : off(&K2AddrTable::LabelSize), oId = off(&K2AddrTable::Ctl_Id), oFlags = off(&K2AddrTable::Ctl_Flags);
    const uint32_t labelVt = vtab(&K2AddrTable::LabelVtable);
    if (!panel || !tagName || !g_gk.alloc || !g_gk.labelCtor || !g_gk.initControl || !g_gk.exoCtor || !g_gk.exoDtor || size <= 0 || !oId || !oFlags || !labelVt) { r.why = CF_Alloc; return 0; }
    unsigned char* mem = static_cast<unsigned char*>(g_gk.alloc(size));
    if (!mem) { r.why = CF_Alloc; return 0; }
    memset(mem, 0, size);
    g_gk.labelCtor(mem);
    uintptr_t c = reinterpret_cast<uintptr_t>(mem);
    uint32_t vt = 0;
    if (!rd32(c, 0, &vt) || vt != labelVt) { r.why = CF_Vtable; r.badVtable = vt; return 0; }
    char exo[16] = {0};
    g_gk.exoCtor(exo, tagName);
    g_gk.initControl(reinterpret_cast<void*>(panel), mem, exo, 1, 1);
    g_gk.exoDtor(exo);
    r.ctl = c;
    int id = -999, fl = 0;
    rdInt(c, oId, &id); rdInt(c, oFlags, &fl);
    r.id = id; r.flagsBefore = fl;
    getExtent(c, &r.ext[0], &r.ext[1], &r.ext[2], &r.ext[3]);
    r.flagsAfter = bindFlags(id, expectId, fl);
    if (!wr<int>(c, oFlags, r.flagsAfter)) { r.why = CF_Flags; return 0; }
    if (!bindMatches(id, expectId)) { r.why = CF_Template; return 0; }
    return c;
}

// ---- Win32 platform defaults (game window lookup as Mod 14 gameWindow 241: largest visible window of this pid, > 320x240 per Mod 3) ----
#ifndef GUIKIT_HOST
#ifdef K2_GUIINPUT_H
inline k2input::Client g_gkInput;
inline void initGuiInput(const KPatchApi* api, const char* owner) {
    g_gkInput.init(api, owner);
    g_gkInput.connect();
}
#endif

namespace detail {
#ifdef K2_GUIINPUT_H
inline int32_t KPATCH_CALL guikitWheelCb(const K2WheelEvent* ev, void*) {
    WheelHook& s = g_wheel; DWORD now = GetTickCount();
    if (s.gate(now)) {
        if (s.scoped) saferead::beginScope();
        int sw = ev->screen_w, sh = ev->screen_h;
        syncScreen(s.iniW, s.iniH, &sw, &sh);
        float ex = ev->engine_x, ey = ev->engine_y;
        if (s.iniW > 0 || s.iniH > 0) {
            clientToEngineF(ev->client_x, ev->client_y, ev->screen_w, ev->screen_h,
                            static_cast<float>(sw), static_cast<float>(sh), &ex, &ey);
        }
        if (s.inside(ex, ey)) {
            int32_t cur = __atomic_load_n(&s.accum, __ATOMIC_RELAXED);
            while (!__atomic_compare_exchange_n(&s.accum, &cur, wheelAddClamped(cur, ev->delta), false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)) {}
            if (s.onEvent && s.eventLogs < s.eventLogMax) {
                ++s.eventLogs;
                s.onEvent(ev->delta, ev->client_x, ev->client_y, true);
            }
            return 1; // consumed!
        }
    }
    return 0; // not consumed
}
#endif

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
#ifdef K2_GUIINPUT_H
    if (g_gkInput.connected) {
        HWND gh = reinterpret_cast<HWND>(g_gkInput.game_hwnd());
        if (gh) return gh;
    }
#endif
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
#ifdef K2_GUIINPUT_H
    if (g_gkInput.connected) return g_gkInput.foreground();
#endif
    HWND fg = GetForegroundWindow(); DWORD pid = 0;
    if (!fg) return false;
    GetWindowThreadProcessId(fg, &pid);
    return pid == GetCurrentProcessId();
}
inline bool winLButton() { return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0; }
inline bool winRButton() { return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0; }
struct InstallPlatform { InstallPlatform() { g_gk.cursorClient = winCursorClient; g_gk.gameForeground = winForeground; g_gk.lbuttonDown = winLButton; g_gk.rbuttonDown = winRButton; } };
inline InstallPlatform g_installPlatform;
// WheelHook's Win32 side: subclass proc + install (A/I/E wheelProc and the subclass step of their gameWindow()).
// The subclass is a kp::WindowHook (Common WindowHook.h, finding F28 design A): one per DLL, DLL image pinned.
struct WheelTag;
using WheelWin = kp::WindowHook<WheelTag>;
inline LRESULT CALLBACK wheelProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_MOUSEWHEEL && WheelWin::active()) {
        WheelHook& s = g_wheel; DWORD now = GetTickCount();
        if (s.gate(now)) {
            if (s.scoped) saferead::beginScope();
            POINT pt = { wheelPtX(l), wheelPtY(l) }; RECT r = { 0, 0, 0, 0 };
            bool have = ScreenToClient(h, &pt) && GetClientRect(h, &r) && r.right > 0 && r.bottom > 0;
            if (have) syncScreen(s.iniW, s.iniH, &s.screenW, &s.screenH);
            if (s.onWheel(now, wheelDelta(w), have, pt.x, pt.y, r.right, r.bottom)) return 0;
        }
    }
    return WheelWin::forward(h, m, w, l);
}
}
// Subclasses the game window once (idempotent; also re-arms a hook that an earlier wheelUninstall left pass-through). False until the window
// exists (retry; gameWindow() throttles its own lookups to 2 s).
inline HWND g_wheelHwnd = nullptr;
inline bool wheelInstall() {
#ifdef K2_GUIINPUT_H
    g_gkInput.poll();
    if (g_gkInput.connected) {
        static bool subOk = false;
        if (!subOk) {
            int32_t tok = g_gkInput.wheel_subscribe(detail::guikitWheelCb, nullptr);
            if (tok > 0) subOk = true;
        }
        HWND h = reinterpret_cast<HWND>(g_gkInput.game_hwnd());
        if (h) g_wheelHwnd = h;
        return subOk;
    }
#endif
    if (detail::WheelWin::old) { detail::WheelWin::install(detail::WheelWin::hwnd, detail::wheelProc); return true; }
    HWND h = detail::gameWindow(); if (!h) return false;
    bool ok = detail::WheelWin::install(h, detail::wheelProc);
    if (ok) g_wheelHwnd = h;
    return ok;
}
// Unload (design A): the DLL image is pinned, so the proc may stay chained. Restore the saved proc ONLY IF ours is still on top; otherwise the
// hook goes pass-through (inert, forwards everything) instead of unhooking a DLL that subclassed later (tests/static/subclassers.toml rule).
inline void wheelUninstall() {
#ifdef K2_GUIINPUT_H
    if (g_gkInput.connected) {
        g_gkInput.wheel_unsubscribe(detail::guikitWheelCb);
    }
#endif
    detail::WheelWin::uninstall(detail::wheelProc);
    g_wheelHwnd = detail::WheelWin::hwnd;
}
#endif

} // namespace guikit
