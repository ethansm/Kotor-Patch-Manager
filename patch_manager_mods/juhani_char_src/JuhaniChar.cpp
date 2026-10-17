// Juhani Char 0.1.0-dev (Steam Aspyr build 6A522E71...): a DLL-only client of asset-vfs (doc 44 VFS-B1).
// KPatchInit registers the private Juhani bundle (JuhaniBundle.cpp) with k2.assetvfs@1 and k2.characterlib@1.
// Providers load before non-providers and the manifest requires asset-vfs, so a NULL interface here means asset-vfs
// is missing or disabled (KP_ERR). It owns no hook; the k1-spawn-probe roster spawns k1p_juhani from the VFS UTC.
// Log: juhani-char_log.txt via KpLog.h (channel main).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stddef.h>
#include <stdint.h>

#include "kpatch_api.h"
#include "KpLog.h"
#include "CharacterProvider.h"
#include "JuhaniBundle.h"

KP_LOG_PATCH("juhani-char");

namespace {

KP_CHANNEL(chMain, "main");

const bool g_fallbackSet = (KpLog_SetFallbackFile("juhani-char_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

k2char::Client g_char;

} // namespace

extern "C" {

__declspec(dllexport) void __cdecl KPatchInit(const KPatchApi* api) {
    if (!api) return;
    KpLog_Init(api);
    g_char.init(api, "juhani-char");
    g_char.connect();
    if (!g_char.vfs || !g_char.lib) {
        KP_ERR("asset-vfs interfaces missing (vfs=%d charlib=%d): nothing registered", g_char.vfs ? 1 : 0, g_char.lib ? 1 : 0);
        return;
    }
    const juhanichar::BundleResult r = juhanichar::registerBundle(g_char.vfs, g_char.lib, "juhani-char");
    if (!r.present) {
        KP_WARN("no bundle compiled in: nothing registered");
        return;
    }
    if (!r.ok) {
        KP_ERR("REGISTER failed assets=%d/%u cells=%d/%u character=%d", static_cast<int>(r.assets),
               static_cast<unsigned>(r.expectedAssets), static_cast<int>(r.cells),
               static_cast<unsigned>(r.expectedCells), static_cast<int>(r.character));
        return;
    }
    KP_LOG(chMain, "REGISTER ok assets=%d/%u cells=%d/%u character=%d", static_cast<int>(r.assets),
           static_cast<unsigned>(r.expectedAssets), static_cast<int>(r.cells),
           static_cast<unsigned>(r.expectedCells), static_cast<int>(r.character));
}

} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinst);
    }
    return TRUE;
}
