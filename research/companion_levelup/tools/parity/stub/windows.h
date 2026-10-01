// Host stub of the few Win32 symbols LevelUpOverride.cpp uses, so its pure algorithm (parseTable/computeRewrite/formatDry)
// compiles natively for parity_main.cpp. Nothing here is called by the parity test except the file/str helpers.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <strings.h>
#include <sys/stat.h>
typedef unsigned long DWORD; typedef long LONG; typedef int BOOL; typedef void* HANDLE; typedef void* HINSTANCE; typedef void* LPVOID;
typedef const void* LPCVOID; typedef uint64_t SIZE_T;
#define WINAPI
#define __cdecl
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
inline SIZE_T VirtualQuery(LPCVOID, MEMORY_BASIC_INFORMATION*, SIZE_T) { return 0; }
inline DWORD GetTickCount() { return 0; }
inline DWORD GetCurrentThreadId() { return 0; }
inline LONG InterlockedIncrement(volatile LONG* p) { return ++*p; }
struct FILETIME { DWORD dwLowDateTime, dwHighDateTime; };
struct WIN32_FILE_ATTRIBUTE_DATA { DWORD dwFileAttributes; FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime; DWORD nFileSizeHigh, nFileSizeLow; };
enum { GetFileExInfoStandard };
inline BOOL GetFileAttributesExA(const char* p, int, WIN32_FILE_ATTRIBUTE_DATA* d) {
    struct stat st; if (stat(p, &st)) return 0;
    d->ftLastWriteTime.dwLowDateTime = (DWORD)st.st_mtime; d->ftLastWriteTime.dwHighDateTime = 0;
    d->nFileSizeHigh = 0; d->nFileSizeLow = (DWORD)st.st_size; return 1;
}
inline BOOL DeleteFileA(const char*) { return 0; }
inline BOOL MoveFileA(const char*, const char*) { return 0; }
inline BOOL DisableThreadLibraryCalls(HINSTANCE) { return 1; }
#define _stricmp strcasecmp
