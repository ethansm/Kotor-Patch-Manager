// Host stub layer for enemy_strip_draw_test.cpp. Sits in front of the shared parity stub windows.h (#include_next) and adds what
// EnemyStrip.cpp needs beyond it: the __declspec shim for the KPatchInit export, an in-memory enemy_strip.ini
// (GetPrivateProfileIntA / GetPrivateProfileStringA / WritePrivateProfileStringA), and UINT. Nothing here touches the disk.
#pragma once
#include_next <windows.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>

#ifndef __declspec
#define __declspec(x)
#endif

typedef unsigned int UINT;

// In-memory INI: key = lower("section|key"). The path argument is ignored (the mod has one INI).
inline std::map<std::string, std::string>& stubIni() { static std::map<std::string, std::string> m; return m; }
inline bool g_stubIniWriteFails = false;   // WritePrivateProfileStringA returns FALSE and stores nothing
inline int g_stubIniWrites = 0;            // successful writes
inline std::string stubIniKey(const char* sec, const char* key) {
    std::string k = std::string(sec ? sec : "") + "|" + (key ? key : "");
    for (char& c : k) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return k;
}
inline void stubIniPut(const char* sec, const char* key, const char* val) { stubIni()[stubIniKey(sec, key)] = val; }

inline UINT GetPrivateProfileIntA(const char* sec, const char* key, int def, const char*) {
    auto it = stubIni().find(stubIniKey(sec, key));
    if (it == stubIni().end()) return static_cast<UINT>(def);
    return static_cast<UINT>(strtol(it->second.c_str(), nullptr, 10));
}
inline DWORD GetPrivateProfileStringA(const char* sec, const char* key, const char* def, char* out, DWORD size, const char*) {
    auto it = stubIni().find(stubIniKey(sec, key));
    const char* v = it == stubIni().end() ? (def ? def : "") : it->second.c_str();
    if (!out || size == 0) return 0;
    strncpy(out, v, size - 1);
    out[size - 1] = 0;
    return static_cast<DWORD>(strlen(out));
}
inline BOOL WritePrivateProfileStringA(const char* sec, const char* key, const char* val, const char*) {
    if (g_stubIniWriteFails) return 0;
    stubIniPut(sec, key, val ? val : "");
    ++g_stubIniWrites;
    return 1;
}
