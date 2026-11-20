// Character Packs 0.1.0-dev (Steam Aspyr build 6A522E71...): an asset-free, DLL-only client of asset-vfs.
// Loads every <game dir>\k1_imports\*.k2pack (format: patch_manager_mods/k1_port/K2PACK_FORMAT.md, parser: PackFormat.h)
// and registers its assets, 2DA cells and character with k2.assetvfs@1 and k2.characterlib@1. The DLL carries no game
// assets; packs are made on the user's machine by patch_manager_mods/k1_port/k1_port_import.py build/install.
// KPatchInit connects to the two interfaces and runs runLoader() synchronously: asset-vfs drops 2DA cells registered
// after the table was serviced and its character catalog has no lock, so a worker thread would race both. The load
// uses only kernel32 file calls (no library loads) under the loader lock and never waits on a thread.
// asset-vfs does not copy pointers: each pack's file buffer, entry arrays and strings live in a heap Pack that is never
// freed once registration was attempted. A bad pack is skipped alone (log "<file>: SKIP <reason>"); the shared pools
// (448 static assets / 1024 2DA cells / 64 characters, used by ALL mods) are checked up front as a lower bound.
// Log: character-packs_log.txt via KpLog.h (channel main).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <string>
#include <vector>

#include "../Common/kpatch_api.h"
#include "../Common/KpLog.h"
#include "../Common/CharacterProvider.h"
#include "PackFormat.h"

KP_LOG_PATCH("character-packs");

namespace {

KP_CHANNEL(chMain, "main");

const bool g_fallbackSet = (KpLog_SetFallbackFile("character-packs_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

k2char::Client g_char;

// Running totals of what this mod has put into the shared asset-vfs pools.
uint32_t g_assetsUsed = 0;
uint32_t g_cellsUsed = 0;
uint32_t g_charsUsed = 0;
std::vector<std::string> g_ids;  // character ids already registered by this mod

// One loaded pack. Allocated with new and never freed once registration was attempted: asset-vfs keeps raw pointers
// into buffer, assets, cells, character and owner.
struct Pack {
    uint8_t* buffer = nullptr;
    size_t size = 0;
    k2pack::Character character = {};
    std::vector<k2pack::AssetView> assetViews;
    std::vector<k2pack::CellView> cellViews;
    std::vector<K2VfsAssetEntry> assets;
    std::vector<K2Vfs2DaCell> cells;
    K2CharacterDefinition def = {};
    std::string owner;
};

const uint32_t kPoolAssets = k2pack::kMaxAssets;
const uint32_t kPoolCells = 1024;  // asset-vfs >= 0.1.3-dev kMax2DaCells (334859da); k2pack::kMaxCells is the per-pack cap
const uint32_t kPoolChars = k2char::kMaxCharacters;

// Reads the whole file into a malloc'd buffer. Returns the SKIP reason or nullptr on success.
const char* readFile(const std::string& path, uint8_t*& buf, size_t& size) {
    buf = nullptr;
    size = 0;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return "open-failed";
    LARGE_INTEGER sz;
    if (!GetFileSizeEx(h, &sz)) {
        CloseHandle(h);
        return "stat-failed";
    }
    if (sz.QuadPart <= 0) {
        CloseHandle(h);
        return "empty-file";
    }
    if (static_cast<unsigned long long>(sz.QuadPart) > k2pack::kMaxFileBytes) {
        CloseHandle(h);
        return "file-too-large";
    }
    const size_t n = static_cast<size_t>(sz.QuadPart);
    uint8_t* p = static_cast<uint8_t*>(malloc(n));
    if (!p) {
        CloseHandle(h);
        return "out-of-memory";
    }
    size_t got = 0;
    while (got < n) {
        DWORD chunk = 0;
        if (!ReadFile(h, p + got, static_cast<DWORD>(n - got), &chunk, nullptr) || chunk == 0) break;
        got += chunk;
    }
    CloseHandle(h);
    if (got != n) {
        free(p);
        return "read-failed";
    }
    buf = p;
    size = n;
    return nullptr;
}

// Loads one pack file and registers it. Returns true when the pack was fully registered.
bool loadPack(const std::string& dir, const std::string& file) {
    uint8_t* buf = nullptr;
    size_t size = 0;
    if (const char* why = readFile(dir + file, buf, size)) {
        KP_WARN("%s: SKIP %s", file.c_str(), why);
        return false;
    }

    Pack* pk = new Pack();
    pk->buffer = buf;
    pk->size = size;
    k2pack::Err err = k2pack::parse(buf, size, pk->character, pk->assetViews, pk->cellViews);
    if (err != k2pack::Err::Ok) {
        KP_WARN("%s: SKIP %s", file.c_str(), k2pack::errName(err));
        free(buf);
        delete pk;
        return false;
    }

    const char* id = pk->character.id;
    const uint32_t na = static_cast<uint32_t>(pk->assetViews.size());
    const uint32_t nc = static_cast<uint32_t>(pk->cellViews.size());
    bool skip = false;
    if (g_assetsUsed + na > kPoolAssets) {
        KP_WARN("pack %s skipped: would exceed shared pool (assets used=%u+%u>%u)", id, g_assetsUsed, na, kPoolAssets);
        skip = true;
    } else if (g_cellsUsed + nc > kPoolCells) {
        KP_WARN("pack %s skipped: would exceed shared pool (cells used=%u+%u>%u)", id, g_cellsUsed, nc, kPoolCells);
        skip = true;
    } else if (g_charsUsed + 1 > kPoolChars) {
        KP_WARN("pack %s skipped: would exceed shared pool (characters used=%u+1>%u)", id, g_charsUsed, kPoolChars);
        skip = true;
    } else if (std::find(g_ids.begin(), g_ids.end(), std::string(id)) != g_ids.end()) {
        KP_WARN("%s: SKIP duplicate-character-id %s", file.c_str(), id);
        skip = true;
    }
    if (skip) {
        free(buf);
        delete pk;
        return false;
    }

    // Persistent arrays: pointers into pk's vectors stay valid because pk is never freed and the vectors never grow again.
    pk->owner = std::string("character-packs:") + id;
    pk->assets.resize(na);
    for (uint32_t i = 0; i < na; ++i) {
        const k2pack::AssetView& v = pk->assetViews[i];
        K2VfsAssetEntry& e = pk->assets[i];
        e.resref = v.resref;
        e.res_type = v.res_type;
        e.flags = v.flags;
        e.data = buf + v.offset;
        e.size = v.size;
    }
    pk->cells.resize(nc);
    for (uint32_t i = 0; i < nc; ++i) {
        const k2pack::CellView& v = pk->cellViews[i];
        K2Vfs2DaCell& c = pk->cells[i];
        c.table_name = v.table;
        c.row_index = v.row;
        c.template_row = v.template_row;
        c.column_name = v.column;
        c.value = v.value;
    }
    pk->def.struct_size = sizeof(K2CharacterDefinition);
    pk->def.character_id = pk->character.id;
    pk->def.display_name = pk->character.display_name;
    pk->def.default_tag = pk->character.default_tag;
    pk->def.default_utc = pk->character.default_utc;
    pk->def.default_dlg = pk->character.default_dlg;
    pk->def.appearance_id = pk->character.appearance_id;
    pk->def.head_id = pk->character.head_id;
    pk->def.soundset_id = pk->character.soundset_id;

    // From here on the pack is never freed: asset-vfs may hold pointers even after a partial registration.
    const int32_t ra = g_char.vfs->RegisterAssets(pk->assets.data(), na, pk->owner.c_str());
    const int32_t rc = nc ? g_char.vfs->Register2DaCells(pk->cells.data(), nc, pk->owner.c_str()) : 0;
    const int32_t rch = g_char.lib->RegisterCharacter(&pk->def, pk->owner.c_str());
    if (ra > 0) g_assetsUsed += static_cast<uint32_t>(ra);
    if (rc > 0) g_cellsUsed += static_cast<uint32_t>(rc);
    if (rch > 0) {
        g_charsUsed += 1;
        g_ids.push_back(id);
    }

    if (ra != static_cast<int32_t>(na) || rc != static_cast<int32_t>(nc) || rch <= 0) {
        KP_ERR("%s: REGISTER failed assets=%d/%u cells=%d/%u character=%d", id, static_cast<int>(ra), static_cast<unsigned>(na),
               static_cast<int>(rc), static_cast<unsigned>(nc), static_cast<int>(rch));
        return false;
    }
    KP_LOG(chMain, "%s: REGISTER ok assets=%d/%u cells=%d/%u character=%d", id, static_cast<int>(ra), static_cast<unsigned>(na),
           static_cast<int>(rc), static_cast<unsigned>(nc), static_cast<int>(rch));
    return true;
}

// The whole load, called once from KPatchInit.
void runLoader() {
    const DWORD t0 = GetTickCount();

    char exe[MAX_PATH] = {};
    DWORD n = GetModuleFileNameA(nullptr, exe, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        KP_ERR("GetModuleFileNameA failed: nothing loaded");
        return;
    }
    std::string dir(exe, n);
    size_t slash = dir.find_last_of("\\/");
    dir = (slash == std::string::npos) ? std::string() : dir.substr(0, slash + 1);
    dir += "k1_imports\\";

    DWORD attr = GetFileAttributesA(dir.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        KP_LOG(chMain, "no k1_imports dir: nothing to load");
        return;
    }

    std::vector<std::string> names;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((dir + "*.k2pack").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) names.push_back(fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    std::sort(names.begin(), names.end());

    unsigned ok = 0, skipped = 0;
    for (const std::string& f : names) {
        if (loadPack(dir, f)) ++ok;
        else ++skipped;
    }
    KP_LOG(chMain, "LOAD done packs=%u ok=%u skipped=%u elapsed_ms=%lu", static_cast<unsigned>(names.size()), ok, skipped,
           static_cast<unsigned long>(GetTickCount() - t0));
}

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api) return;
    KpLog_Init(api);
    g_char.init(api, "character-packs");
    g_char.connect();
    if (!g_char.vfs || !g_char.lib) {
        KP_ERR("asset-vfs interfaces missing (vfs=%d charlib=%d): nothing registered", g_char.vfs ? 1 : 0, g_char.lib ? 1 : 0);
        return;
    }
    // Synchronous on purpose (user decision 10-18), like juhani-char: asset-vfs never re-splices a 2DA that was already
    // serviced, so cells must be in before the engine first loads appearance/heads.2da, and the character catalog has
    // no lock. CreateFile/ReadFile/FindFirstFile load no DLLs, so they are safe under the loader lock; cost = read time.
    runLoader();
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
