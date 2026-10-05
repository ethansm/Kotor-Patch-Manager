// CodePatch.h -- code-patch helpers for KOTOR2 patch DLLs: committed-memory probe, protect/copy/restore/flush writes,
// rel32 builders and the IAT patcher. Header-only, namespace codepatch, everything inline.
//
// Replaces the drifting private copies in robustness_fixes_src (committed, writeCode, jmpTo), equip_icon_fix_src (committed,
// callBytes, writeCode), perf_lab_src (writeCode with expect/seen, putRel32, patchIat), load_hang_probe_src (patchIat) and
// activation_fix_src (writeCode, putRel32). Semantics are those copies' exactly:
//   write         1 ok | -2 VirtualProtect failed (bytes untouched). Order: protect(RWX), memcpy, restore protection, flush.
//   writeChecked  1 ok | -1 bytes differ from `expect` (`seen` is filled BEFORE the compare; no protect, nothing written)
//                 | -2 VirtualProtect failed (`seen` already filled).
// The return codes are the helper's only contract: callers keep their own state codes (g_state / -3 "not committed" /
// bad-address bookkeeping) and map these onto them. (activation_fix's writeCode also returns -3 on a non-committed page; that
// stays caller-side: call committed() first.)
//
// Testing: every Win32 call goes through ops() (query/protect/flush), which host tests replace with fakes. Under -DGUIKIT_HOST
// the stub windows.h has no VirtualProtect/FlushInstructionCache/GetCurrentProcess, so the default protect/flush are host fakes
// (protect succeeds and writes back the old protection, flush is a no-op) and the PE structs patchIat walks are defined here.
#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>

#ifdef GUIKIT_HOST
typedef void* HMODULE; typedef DWORD* PDWORD;   // missing from the stub (identical redefinition is harmless)
#endif

namespace codepatch {

struct Ops {   // injectable for host tests; defaults = Win32
    SIZE_T (*query)(LPCVOID, MEMORY_BASIC_INFORMATION*, SIZE_T);
    BOOL   (*protect)(LPVOID, SIZE_T, DWORD, PDWORD);
    void   (*flush)(const void*, SIZE_T);
};

#ifdef GUIKIT_HOST
// Host stub gaps (see research/companion_levelup/tools/parity/stub/windows.h); layouts match the PE32 structs of <winnt.h>.
namespace pe {
struct DosHeader { uint8_t pad[0x3C]; int32_t e_lfanew; };
struct DataDir { uint32_t VirtualAddress, Size; };
struct NtHeaders { uint32_t Signature; uint8_t FileHeader[20]; struct { uint8_t pre[96]; DataDir DataDirectory[16]; } OptionalHeader; };
struct ImportDescriptor { uint32_t OriginalFirstThunk, TimeDateStamp, ForwarderChain, Name, FirstThunk; };
struct Thunk32 { union { uint32_t Function, Ordinal, AddressOfData; } u1; };
struct ImportByName { uint16_t Hint; char Name[1]; };
constexpr uint32_t OrdinalFlag32 = 0x80000000u;
constexpr int DirImport = 1;
}
inline SIZE_T hostQuery(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) { return VirtualQuery(a, m, n); }
inline BOOL hostProtect(LPVOID, SIZE_T, DWORD, PDWORD old) { *old = PAGE_READWRITE; return TRUE; }
inline void hostFlush(const void*, SIZE_T) {}
inline Ops defaultOps() { return { hostQuery, hostProtect, hostFlush }; }
#else
namespace pe {
using DosHeader = IMAGE_DOS_HEADER;
using NtHeaders = IMAGE_NT_HEADERS;
using ImportDescriptor = IMAGE_IMPORT_DESCRIPTOR;
using Thunk32 = IMAGE_THUNK_DATA32;
using ImportByName = IMAGE_IMPORT_BY_NAME;
constexpr uint32_t OrdinalFlag32 = IMAGE_ORDINAL_FLAG32;
constexpr int DirImport = IMAGE_DIRECTORY_ENTRY_IMPORT;
}
inline SIZE_T winQuery(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T n) { return VirtualQuery(a, m, n); }
inline BOOL winProtect(LPVOID p, SIZE_T n, DWORD np, PDWORD old) { return VirtualProtect(p, n, np, old); }
inline void winFlush(const void* p, SIZE_T n) { FlushInstructionCache(GetCurrentProcess(), p, n); }
inline Ops defaultOps() { return { winQuery, winProtect, winFlush }; }
#endif

inline Ops& ops() { static Ops o = defaultOps(); return o; }

// [at, at+n) inside one committed region?
inline bool committed(uintptr_t at, size_t n) {
    MEMORY_BASIC_INFORMATION mb;
    return ops().query(reinterpret_cast<void*>(at), &mb, sizeof(mb)) && mb.State == MEM_COMMIT &&
           at + n <= reinterpret_cast<uintptr_t>(mb.BaseAddress) + mb.RegionSize;
}

// Overwrite n code bytes at `at`: 1 | -2 (protect failed, nothing written).
inline int write(uintptr_t at, const uint8_t* repl, int n) {
    auto p = reinterpret_cast<uint8_t*>(at);
    DWORD old;
    if (!ops().protect(p, n, PAGE_EXECUTE_READWRITE, &old)) return -2;
    memcpy(p, repl, n);
    ops().protect(p, n, old, &old);
    ops().flush(p, n);
    return 1;
}

// As write(), but only if the n bytes at `at` equal `expect`: 1 | -1 (mismatch) | -2 (protect failed). `seen` (n bytes) receives
// the bytes found at `at` before the compare, so every return code leaves the caller the evidence.
inline int writeChecked(uintptr_t at, const uint8_t* expect, const uint8_t* repl, int n, uint8_t* seen) {
    auto p = reinterpret_cast<uint8_t*>(at);
    memcpy(seen, p, n);
    if (memcmp(p, expect, n) != 0) return -1;
    DWORD old;
    if (!ops().protect(p, n, PAGE_EXECUTE_READWRITE, &old)) return -2;
    memcpy(p, repl, n);
    ops().protect(p, n, old, &old);
    ops().flush(p, n);
    return 1;
}

// 32-bit displacement for a rel32 field that will live at `fieldSite` (not necessarily where `field` is staged): target - (fieldSite + 4).
inline void putRel32(uint8_t* field, uintptr_t fieldSite, uintptr_t target) {
    int32_t d = static_cast<int32_t>(target - (fieldSite + 4));
    memcpy(field, &d, 4);
}

// E9 rel32 for an instruction at `at`, NOP-padded (0x90) to n bytes (n >= 5).
inline void jmpRel(uintptr_t at, uintptr_t target, uint8_t* out, int n) {
    out[0] = 0xE9; int32_t r = static_cast<int32_t>(target - (at + 5)); memcpy(out + 1, &r, 4);
    for (int i = 5; i < n; ++i) out[i] = 0x90;
}

// Executable scratch memory for a code cave: n bytes, zero-filled, committed RWX (VirtualAlloc MEM_COMMIT|MEM_RESERVE,
// PAGE_EXECUTE_READWRITE). nullptr when n == 0 or the allocation fails. Never freed (a cave lives as long as its hook). Fill it,
// then hook with jmpRel(at, cave, out, n) + write(at, out, n); flush the cave with ops().flush(cave, used) after filling.
// Host build (GUIKIT_HOST): calloc-backed (the stub has no VirtualAlloc); the bytes are data only.
inline uint8_t* allocCave(size_t n) {
    if (n == 0) return nullptr;
#ifdef GUIKIT_HOST
    return static_cast<uint8_t*>(calloc(n, 1));
#else
    return static_cast<uint8_t*>(VirtualAlloc(nullptr, n, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
#endif
}

// E8 rel32 for a call instruction at `at`.
inline void callRel(uintptr_t at, uintptr_t target, uint8_t out[5]) {
    out[0] = 0xE8; int32_t d = static_cast<int32_t>(target - (at + 5)); memcpy(out + 1, &d, 4);
}

// Hook an import: finds `fn` (by name; ordinal imports are skipped) in the import descriptor whose DLL name equals `dll`
// (case-insensitive) and swaps its IAT slot for `hook`, returning the old value in *orig. PE32 only (32-bit thunks).
// false: no import directory, no such dll/function, or VirtualProtect failed (slot untouched, *orig untouched).
inline bool patchIat(HMODULE mod, const char* dll, const char* fn, void* hook, void** orig) {
    auto base = reinterpret_cast<uint8_t*>(mod);
    auto dos = reinterpret_cast<pe::DosHeader*>(base);
    auto nt = reinterpret_cast<pe::NtHeaders*>(base + dos->e_lfanew);
    auto& dir = nt->OptionalHeader.DataDirectory[pe::DirImport];
    if (!dir.VirtualAddress) return false;
    for (auto d = reinterpret_cast<pe::ImportDescriptor*>(base + dir.VirtualAddress); d->Name; ++d) {
        if (_stricmp(reinterpret_cast<const char*>(base + d->Name), dll) != 0) continue;
        auto oft = reinterpret_cast<pe::Thunk32*>(base + (d->OriginalFirstThunk ? d->OriginalFirstThunk : d->FirstThunk));
        auto ft = reinterpret_cast<pe::Thunk32*>(base + d->FirstThunk);
        for (; oft->u1.AddressOfData; ++oft, ++ft) {
            if (oft->u1.Ordinal & pe::OrdinalFlag32) continue;
            auto ibn = reinterpret_cast<pe::ImportByName*>(base + oft->u1.AddressOfData);
            if (strcmp(reinterpret_cast<const char*>(ibn->Name), fn) != 0) continue;
            DWORD old;
            if (!ops().protect(&ft->u1.Function, 4, PAGE_READWRITE, &old)) return false;
            *orig = reinterpret_cast<void*>(static_cast<uintptr_t>(ft->u1.Function));
            ft->u1.Function = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(hook));
            ops().protect(&ft->u1.Function, 4, old, &old);
            return true;
        }
    }
    return false;
}

}  // namespace codepatch
