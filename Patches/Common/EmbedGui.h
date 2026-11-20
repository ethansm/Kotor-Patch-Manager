// EmbedGui.h -- an in-memory "GUI " layout (GFF bytes held in the process: no file, no resource-table entry) opened on a panel by
// swapping the panel's layout slot P+0x30 (Steam Aspyr swkotor2.exe). Header-only, no STL, logger-neutral (set embedgui::g_log; null = silent).
//
// Origin: holster_edit recon D2F (.claude/research/holster_edit/d2f_embedded_layout.md sections 1, 4, 6) and U05 steps 1-5, 11-13.
// Static facts only: no step is live-proven. Every step logs (EMBEDGUI_LOG) so the live probe can confirm it.
//
// Lifecycle (the caller owns the Handle and runs InitControl / labels between Open and Close; that is not this header's job):
//   Open:  obj = operator new(0xA0); ctor(obj, 0x7FF, "GUI ", "mod_inline_gui") (ID -1, no owner, no data); obj+0x18 = size;
//          Malloc(resman, obj) sets obj+0x10 (the data buffer); memcpy the bytes into it; OnResourceServiced(obj) parses in place.
//          Parse fail -> FreeResourceData(resman, obj), obj->vt[0](1), return null, P+0x30 untouched.
//          Success -> obj+0xC |= 4, obj+4 (ushort) = 1, obj+0x9C = 1, then P+0x30 = obj (the swap window). Returns {obj, saved}.
//   Close: P+0x30 = saved FIRST, then 0x7112B0(obj), then obj->vt[0](1). The handle is cleared, so a second Close is a no-op.
//   Malloc failure returns null with P+0x30 untouched and no free (card rule; the 0xA0-byte object is left behind, and the log says so).
//
// Engine calls go through seven named wrappers (callNew .. callDtor), the only place a call is made. Game build: inline, real Steam
// addresses. EMBEDGUI_HOST: the wrappers are only declared here; embedgui_test.cpp defines them as fakes with the same signatures.
// Checked against the static dumps (review 10-16): ctor 0x716FD0 is __thiscall(obj; ushort type, char* ext, resref) RET 12, as callCtor
// pushes it; Malloc 0x712F30 returns bool (+0x10 != 0) and gives the budget back on failure.
#pragma once
#include <stdint.h>
#include <string.h>
#include "GameAddr.h"

namespace embedgui {

// ---- Steam addresses (D2F, static only; canonical in GameAddr.h) ----
constexpr uintptr_t AddrNew = gameaddr::New;                   // cdecl(size): engine operator new
constexpr uintptr_t AddrCtor = gameaddr::ResGffCtor;           // CResGFF::Constructor_2: __thiscall(ECX=obj; type, ext, resref) RET 12
constexpr uintptr_t AddrMalloc = gameaddr::ResManMalloc;       // CExoResMan::Malloc: __thiscall(ECX=resman; push obj); fills obj+0x10
constexpr uintptr_t AddrParse = gameaddr::ResGffOnServiced;    // CResGFF::OnResourceServiced: __fastcall(ECX=obj) RET 0; 1 = ok, 0 = fail; no bounds check
constexpr uintptr_t AddrFreeData = gameaddr::ResManFreeData;   // CExoResMan::FreeResourceData: __thiscall(ECX=resman; push obj); parse-fail path only
constexpr uintptr_t AddrRelease = gameaddr::ResRelease;        // CRes::Release __fastcall(ECX=obj): success-path release, then obj->vt[0](1)
constexpr uintptr_t ResManCell = gameaddr::ResManGlobal;       // DWORD global: resman = [0xA1B490] (read at call time)

// ---- CResGFF object fields and constants (D2F section 1; Steam) ----
constexpr int ObjBytes = 0xA0;           // operator new size
constexpr int Obj_Ref = 0x04;            // ushort refcount (success path sets 1)
constexpr int Obj_Flags = 0x0C;          // flags (success path sets 0x4 = has data)
constexpr int Obj_Data = 0x10;           // data pointer (Malloc sets it; the GFF bytes live there)
constexpr int Obj_DataSize = 0x18;       // size of the data (set before Malloc)
constexpr int Obj_Flag9C = 0x9C;         // dword set to 1 on success (width assumed dword, like +0x94/+0x98)
constexpr int PanelLayoutSlot = 0x30;    // P+0x30: the layout the panel uses; saved on Open, swapped, restored on Close
constexpr unsigned GffType = 0x7FF;      // ctor type argument
constexpr const char* GffExt = "GUI ";   // ctor ext argument
constexpr const char* DummyResref = "mod_inline_gui";   // ctor resref: must be in no table

inline void (*g_log)(const char* fmt, ...) = nullptr;   // logger-neutral sink; null = silent
#define EMBEDGUI_LOG(...) do { if (::embedgui::g_log) ::embedgui::g_log(__VA_ARGS__); } while (0)

// ---- engine wrappers (the only place engine calls are made) ----
#ifdef EMBEDGUI_HOST
// Host build: the test (embedgui_test.cpp) defines these seven with the same signatures.
void* callNew(unsigned n);               // operator new(n)
void callCtor(void* obj);                // ctor(obj, GffType, GffExt, DummyResref)
bool callMalloc(void* obj);              // Malloc(resman, obj); resman comes from the engine global
int callParse(void* obj);                // OnResourceServiced(obj): 1 ok, 0 fail
void callFreeData(void* obj);            // FreeResourceData(resman, obj)
void callRelease(void* obj);             // 0x7112B0(obj)
void callDtor(void* obj);                // obj->vt[0](1)
#else
inline void* callNew(unsigned n) { return reinterpret_cast<void* (*)(unsigned)>(AddrNew)(n); }
inline void callCtor(void* obj) {
    reinterpret_cast<void(__thiscall*)(void*, unsigned, const char*, const char*)>(AddrCtor)(obj, GffType, GffExt, DummyResref);
}
inline bool callMalloc(void* obj) {
    return reinterpret_cast<bool(__thiscall*)(void*, void*)>(AddrMalloc)(*reinterpret_cast<void**>(ResManCell), obj);
}
inline int callParse(void* obj) { return reinterpret_cast<int(__fastcall*)(void*)>(AddrParse)(obj); }
inline void callFreeData(void* obj) {
    reinterpret_cast<void(__thiscall*)(void*, void*)>(AddrFreeData)(*reinterpret_cast<void**>(ResManCell), obj);
}
inline void callRelease(void* obj) { reinterpret_cast<void(__fastcall*)(void*)>(AddrRelease)(obj); }
inline void callDtor(void* obj) {   // the scalar deleting dtor, flag 1 = delete
    void** vt = *reinterpret_cast<void***>(obj);
    reinterpret_cast<void(__thiscall*)(void*, int)>(vt[0])(obj, 1);
}
#endif

// ---- field access ----
inline void*& layoutSlot(unsigned char* P) { return *reinterpret_cast<void**>(P + PanelLayoutSlot); }
inline unsigned char* dataOf(void* obj) { return *reinterpret_cast<unsigned char**>(static_cast<unsigned char*>(obj) + Obj_Data); }

struct Handle {
    void* obj = nullptr;     // the CResGFF object; null = not open
    void* saved = nullptr;   // P+0x30 as it was before Open
};

// Open the "GUI " layout bytes (copied into the engine buffer; the bytes need not outlive the call) on panel P.
// Returns {obj, saved} with P+0x30 == obj, or a null Handle with P+0x30 unchanged.
inline Handle Open(unsigned char* P, const unsigned char* bytes, unsigned size) {
    if (!P || !bytes || size == 0) {
        EMBEDGUI_LOG("embedgui: open refused (panel %p, bytes %p, size %u)", (void*)P, (const void*)bytes, size);
        return Handle();
    }
    void* saved = layoutSlot(P);
    void* obj = callNew(ObjBytes);
    if (!obj) { EMBEDGUI_LOG("embedgui: operator new(0x%X) failed, no view", ObjBytes); return Handle(); }
    EMBEDGUI_LOG("embedgui: new obj %p (0x%X bytes)", obj, ObjBytes);
    callCtor(obj);
    EMBEDGUI_LOG("embedgui: ctor done on obj %p (type 0x%X, ext GUI, resref mod_inline_gui)", obj, GffType);
    *reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(obj) + Obj_DataSize) = size;
    if (!callMalloc(obj) || !dataOf(obj)) {
        // Card rule: null, no swap, no free. The object is left as-is (a small leak per failed open; logged).
        EMBEDGUI_LOG("embedgui: Malloc(%u) failed on obj %p, no view (obj left unfreed)", size, obj);
        return Handle();
    }
    memcpy(dataOf(obj), bytes, size);   // the buffer Malloc set at obj+0x10, not the address obj+0x10
    EMBEDGUI_LOG("embedgui: Malloc ok, %u bytes copied into %p", size, (void*)dataOf(obj));
    if (!callParse(obj)) {
        EMBEDGUI_LOG("embedgui: OnResourceServiced failed on obj %p, freeing (FreeResourceData, vt[0](1)); P+0x30 unchanged", obj);
        callFreeData(obj);
        callDtor(obj);
        return Handle();
    }
    unsigned char* o = static_cast<unsigned char*>(obj);
    *reinterpret_cast<uint32_t*>(o + Obj_Flags) |= 0x4u;
    *reinterpret_cast<uint16_t*>(o + Obj_Ref) = 1;
    *reinterpret_cast<uint32_t*>(o + Obj_Flag9C) = 1;
    layoutSlot(P) = obj;
    EMBEDGUI_LOG("embedgui: parsed obj %p, P+0x30 swapped (saved %p)", obj, saved);
    Handle h;
    h.obj = obj;
    h.saved = saved;
    return h;
}

// Undo Open: restore P+0x30 first, then release and free the object once. A second Close, or a Close of a null handle, does nothing.
inline void Close(unsigned char* P, Handle& h) {
    if (!h.obj) return;
    void* obj = h.obj;
    h.obj = nullptr;   // cleared before any engine call: a second Close is a no-op
    if (P) layoutSlot(P) = h.saved;
    EMBEDGUI_LOG("embedgui: close obj %p, P+0x30 restored (saved %p)", obj, h.saved);
    callRelease(obj);
    callDtor(obj);
}

}  // namespace embedgui
