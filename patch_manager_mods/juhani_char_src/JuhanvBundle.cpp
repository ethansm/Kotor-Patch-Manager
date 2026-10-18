// JuhanvBundle.cpp -- the one TU that links the private Juhani twin bundle "Juhani DLL" (renamed k1p_juhanv*, appearance 820, comparison against the Override rig). Same rules as JuhaniBundle.cpp:
// generated header first (top-level asm switches sections), absolute path, -DJUHANI_NO_BUNDLE selects the stub path.
// The blob labels use the kvfs_juhanv prefix, so this bundle links beside Juhani's in one DLL.
#if !defined(JUHANI_NO_BUNDLE) && defined(__has_include)
#if __has_include("/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/juhanv_vfs_bundle.h")
#include "/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/juhanv_vfs_bundle.h"
#define JUHANV_HAVE_BUNDLE 1
#endif
#endif

#include "JuhaniBundle.h"

namespace juhanichar {

BundleResult registerJuhanvBundle(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner) {
    BundleResult r = {false, false, 0, 0, 0, 0, 0};
#if defined(JUHANV_HAVE_BUNDLE)
    r.present = true;
    r.expectedAssets = k2vfs_juhanv::kAssetCount;
    r.expectedCells = k2vfs_juhanv::k2DaCellCount;
    k2vfs_juhanv::RegisterCounts c = {0, 0, 0};
    r.ok = k2vfs_juhanv::registerAll(vfs, charlib, owner, &c);
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
