// CarthBundle.cpp -- the one TU that links the private Carth bundle (doc 44 VFS-B2). Same rules as JuhaniBundle.cpp:
// generated header first (top-level asm switches sections), absolute path, -DJUHANI_NO_BUNDLE selects the stub path.
// The blob labels use the kvfs_carth prefix, so this bundle links beside Juhani's in one DLL.
#if !defined(JUHANI_NO_BUNDLE) && defined(__has_include)
#if __has_include("/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/carth_vfs_bundle.h")
#include "/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/carth_vfs_bundle.h"
#define CARTH_HAVE_BUNDLE 1
#endif
#endif

#include "JuhaniBundle.h"

namespace juhanichar {

BundleResult registerCarthBundle(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner) {
    BundleResult r = {false, false, 0, 0, 0, 0, 0};
#if defined(CARTH_HAVE_BUNDLE)
    r.present = true;
    r.expectedAssets = k2vfs_carth::kAssetCount;
    r.expectedCells = k2vfs_carth::k2DaCellCount;
    k2vfs_carth::RegisterCounts c = {0, 0, 0};
    r.ok = k2vfs_carth::registerAll(vfs, charlib, owner, &c);
    r.assets = c.assets;
    r.cells = c.cells;
    r.character = c.character;
#else
    (void)vfs;
    (void)charlib;
    (void)owner;
#endif
    return r;
}

} // namespace juhanichar
