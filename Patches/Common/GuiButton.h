// GuiButton.h -- header-only CSWGuiButton helper for KOTOR2 Patch Manager mods (Steam Aspyr swkotor2.exe only, i686, no STL).
// Creates a blank button from a layout template with the same recipe and failure codes as guikit::createLabelFromTemplate (GuiKit.h),
// and sets its text. Every engine touch goes through the guikit::g_gk seams; the button-only ctor seam is g_btnCtor below.
//
// Engine facts (Steam exe, ghidra dump 41/0041a510.c: `undefined4 * __fastcall FUN_0041a510(undefined4 *param_1)`, body writes
// `*param_1 = CSWGuiButton::vftable`, so the ctor takes this in ECX and no stack args):
//   CSWGuiButton: vftable 0x987A1C, object size 0x1D0, normal border +0x70, highlight border +0xE8, text sub-object +0x160
//   (vtable TextVtable, string sub-object at +0x160+0x18). Control flags +0x48 and id +0x54 are shared with CSWGuiLabel (GuiKit.h).
//   Reference: patch_manager_mods/08_shield_bar.md:500 (vftable, size, border and text offsets).
//
// Steam only: every entry point returns false / 0 / no-op unless k2addr::get() == &kSteam, so a GOG or unknown exe does nothing.
// Host tests: define GUIKIT_HOST before including (the ctor seam defaults to nullptr; a test substitutes a fake).
#pragma once
#include "GuiKit.h"
#include "K2Addr.h"
#include <string.h>

namespace guibutton {

// ---- Steam exe constants (dump / doc citations in the comments above) ----
constexpr uintptr_t ButtonCtor = gameaddr::ButtonCtor;  // CSWGuiButton ctor, __fastcall(this) (dump 41/0041a510.c:3)
constexpr uint32_t ButtonVtable = static_cast<uint32_t>(gameaddr::ButtonVtable);  // CSWGuiButton vftable (08_shield_bar.md:500; dump 41/0041a510.c writes it)
constexpr int ButtonSize = 0x1D0;                // CSWGuiButton object size (08_shield_bar.md:500)
constexpr int Button_Text = 0x160;               // CSWGuiText sub-object, vtable TextVtable (08_shield_bar.md:500)
constexpr int Button_BorderN = 0x70;             // normal border sub-object (08_shield_bar.md:500)
constexpr int Button_BorderH = 0xE8;             // highlight border sub-object (08_shield_bar.md:500)
constexpr int Button_HighlightBit = 0x1;         // ctl+0x48 bit 0 = highlighted (card c11 spec; not re-verified against a dump here)
constexpr int CanaryBytes = 0x40;                // slack after the object; a ctor overrun into it is reported (not an engine value)
constexpr uint8_t CanaryFill = 0xCD;

// Steam-only gate: the running exe's K2Addr table is the Steam Aspyr table (k2addr::kSteam, K2Addr.h).
inline bool steamBuild() { return k2addr::get() == &kSteam; }

// The ctor seam. Same __thiscall type as guikit::g_gk.labelCtor (GuiKit.h CtorFn): the ctor reads `this` from ECX, so a cdecl pointer
// would crash. Game value = the Steam ctor address; nullptr under GUIKIT_HOST (a host test assigns a fake).
#ifdef GUIKIT_HOST
inline guikit::CtorFn g_btnCtor = nullptr;
#else
inline guikit::CtorFn g_btnCtor = reinterpret_cast<guikit::CtorFn>(ButtonCtor);
#endif

// Releases a button that createButtonFromTemplate allocated and then failed on (InitControl ran with addToList = 0, so no panel list
// holds it). Calls its OWN vtable slot 0 with flags = 1, only when the vtable == ButtonVtable and slot 0 is non-zero (same rule as
// guikit::releaseUnlinkedLabel). Reuses the generic slot-0 seam guikit::g_gkCallLabelDtor (it takes the slot value, the object and flags).
inline void releaseUnlinkedButton(uintptr_t btn) {
    using guikit::detail::rd32;
    uint32_t vt = 0, fn = 0;
    if (!steamBuild() || !btn || !guikit::g_gkCallLabelDtor || !rd32(btn, 0, &vt) || vt != ButtonVtable || !rd32(vt, 0, &fn) || !fn) return;
    (void)guikit::detachPooledFills(btn);   // the border dtors delete their fill images: never a pooled one
    guikit::g_gkCallLabelDtor(fn, btn, 1u);
}

// Creates a blank CSWGuiButton from the layout template `tag` (e.g. "BTN_OK"). Same recipe and CF_* codes as
// guikit::createLabelFromTemplate. ONLY valid while the panel's layout GFF is open: inside the panel ctor, before StopLoadFromLayout.
// NOT appended to the panel (call guikit::appendToPanel). Created hidden + click-through.
// Returns the button, or 0 (not Steam, allocation failed, ctor vtable wrong, or the template tag is not in the layout).
// Optional out-params: *why = guikit::CreateFail reason (CF_None on success); *badVt = the vtable read when why == CF_Vtable;
// *canaryOk = true when the CanaryBytes after the object are still CanaryFill (set on the success path; false otherwise).
// A failure after the vtable check passed releases the button (releaseUnlinkedButton); the CF_Vtable failure leaks it (unknown vtable).
inline uintptr_t createButtonFromTemplate(uintptr_t panel, const char* tag, int* why = nullptr, uint32_t* badVt = nullptr,
                                          bool* canaryOk = nullptr) {
    using namespace guikit;
    using guikit::detail::rd32;
    using guikit::detail::rdInt;
    using guikit::detail::wr;
    if (why) *why = CF_None;
    if (badVt) *badVt = 0;
    if (canaryOk) *canaryOk = false;
    if (!steamBuild() || !panel || !tag || !g_gk.alloc || !g_btnCtor || !g_gk.initControl || !g_gk.exoCtor || !g_gk.exoDtor) {
        if (why) *why = CF_Alloc;
        return 0;
    }
    unsigned char* mem = static_cast<unsigned char*>(g_gk.alloc(ButtonSize + CanaryBytes));
    if (!mem) { if (why) *why = CF_Alloc; return 0; }
    memset(mem, 0, ButtonSize);
    memset(mem + ButtonSize, CanaryFill, CanaryBytes);
    g_btnCtor(mem);
    uintptr_t c = reinterpret_cast<uintptr_t>(mem);
    uint32_t vt = 0;
    if (!rd32(c, 0, &vt) || vt != ButtonVtable) { if (why) *why = CF_Vtable; if (badVt) *badVt = vt; return 0; }
    if (!wr<int>(c, Ctl_Id, IdSentinel)) { releaseUnlinkedButton(c); if (why) *why = CF_Vtable; return 0; }
    char exo[16] = {0};
    g_gk.exoCtor(exo, tag);
    g_gk.initControl(reinterpret_cast<void*>(panel), mem, exo, 0, 1);   // addToList = 0, scale = 1
    g_gk.exoDtor(exo);
    int id = 0;
    if (!rdInt(c, Ctl_Id, &id) || id == IdSentinel) { releaseUnlinkedButton(c); if (why) *why = CF_Template; return 0; }   // tag not in layout
    int index = 0;
    if (!rdInt(panel, Panel_CtlCount, &index)) { releaseUnlinkedButton(c); if (why) *why = CF_Count; return 0; }
    wr<int>(c, Ctl_Id, index);                                          // provisional; appendToPanel re-stamps it
    int fl = 0;
    if (!rdInt(c, Ctl_Flags, &fl)) { releaseUnlinkedButton(c); if (why) *why = CF_Flags; return 0; }
    wr<int>(c, Ctl_Flags, (fl & ~FlagVisible) | FlagClickThrough);     // hidden + click-through until the caller shows it
    if (canaryOk) {
        bool ok = true;
        for (int i = 0; i < CanaryBytes; ++i) if (mem[ButtonSize + i] != CanaryFill) { ok = false; break; }
        *canaryOk = ok;
    }
    return c;
}

// Sets the button's text. Gate: [btn+0x160] vtable == TextVtable. TextSet runs with ECX = btn+0x160+0x18 and a CExoString built from text.
inline bool setButtonText(uintptr_t btn, const char* text) {
    using guikit::detail::rd32;
    if (!steamBuild() || !btn || !text || !guikit::g_gk.exoCtor || !guikit::g_gk.exoDtor || !guikit::g_gk.textSet) return false;
    uintptr_t tx = btn + Button_Text;
    uint32_t vt = 0;
    if (!rd32(tx, 0, &vt) || vt != guikit::TextVtable) return false;
    char exo[16] = {0};
    guikit::g_gk.exoCtor(exo, text);
    guikit::g_gk.textSet(reinterpret_cast<void*>(tx + guikit::Text_StringSub), exo);
    guikit::g_gk.exoDtor(exo);
    return true;
}

// True when the button's highlight bit (ctl+0x48 bit 0) is set.
inline bool buttonHighlighted(uintptr_t btn) {
    int fl = 0;
    return steamBuild() && btn && guikit::detail::rdInt(btn, guikit::Ctl_Flags, &fl) && (fl & Button_HighlightBit);
}

}  // namespace guibutton
