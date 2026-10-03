// Host stub of the few Win32 symbols LevelUpOverride.cpp uses, so its pure algorithm (parseTable/computeRewrite/formatDry)
// compiles natively for parity_main.cpp (and apply_test.cpp, which switches VirtualQuery on). Parity calls only the file/str helpers.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include <sys/stat.h>
typedef unsigned long DWORD; typedef long LONG; typedef int BOOL; typedef void* HANDLE; typedef void* HINSTANCE; typedef void* LPVOID;
typedef const void* LPCVOID; typedef uint64_t SIZE_T;
#define WINAPI
#define __cdecl
#define __thiscall
#define TRUE 1
#define MAX_PATH 260
#define DLL_PROCESS_ATTACH 1
#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD 0x100
#define MEM_COMMIT 0x1000
struct MEMORY_BASIC_INFORMATION { void* BaseAddress; void* AllocationBase; DWORD AllocationProtect; SIZE_T RegionSize; DWORD State, Protect, Type; };
// Off (parity): every probe fails. apply_test.cpp turns it on: all memory reads as one committed read-write region.
inline bool g_stubVirtualQueryAll = false;
inline SIZE_T VirtualQuery(LPCVOID a, MEMORY_BASIC_INFORMATION* m, SIZE_T) {
    if (!g_stubVirtualQueryAll) return 0;
    m->BaseAddress = const_cast<void*>(a); m->AllocationBase = m->BaseAddress; m->AllocationProtect = PAGE_READWRITE;
    // 32-bit (-m32): end the region at UINTPTR_MAX so lo + RegionSize cannot wrap to 0 and loop regionReadable forever.
    m->RegionSize = sizeof(void*) == 8 ? (SIZE_T)1 << 40 : (SIZE_T)(UINTPTR_MAX - (uintptr_t)a);
    m->State = MEM_COMMIT; m->Protect = PAGE_READWRITE; m->Type = 0;
    return sizeof *m;
}
inline DWORD g_stubTick = 0;   // tests advance it to step over the 1000 ms reload throttle
inline DWORD GetTickCount() { return g_stubTick; }
inline DWORD GetCurrentThreadId() { return 0; }
inline LONG InterlockedIncrement(volatile LONG* p) { return ++*p; }
struct FILETIME { DWORD dwLowDateTime, dwHighDateTime; };
struct WIN32_FILE_ATTRIBUTE_DATA { DWORD dwFileAttributes; FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime; DWORD nFileSizeHigh, nFileSizeLow; };
enum { GetFileExInfoStandard };
inline BOOL GetFileAttributesExA(const char* p, int, WIN32_FILE_ATTRIBUTE_DATA* d) {
    struct stat st; if (stat(p, &st)) return 0;
    d->ftLastWriteTime.dwLowDateTime = (DWORD)st.st_mtim.tv_nsec; d->ftLastWriteTime.dwHighDateTime = (DWORD)st.st_mtime;
    d->nFileSizeHigh = 0; d->nFileSizeLow = (DWORD)st.st_size; return 1;
}
inline BOOL DeleteFileA(const char*) { return 0; }
inline BOOL MoveFileA(const char*, const char*) { return 0; }
inline BOOL DisableThreadLibraryCalls(HINSTANCE) { return 1; }
#define _stricmp strcasecmp
