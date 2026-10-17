// JuhaniBundle.cpp -- the one TU that links the private Juhani bundle. The generated header is included FIRST:
// its top-level asm blocks change sections (see the header's own note). It is found by absolute path because the
// KPM Windows build ignores test.toml include flags; a host test builds with -DJUHANI_NO_BUNDLE (stub path).
#if !defined(JUHANI_NO_BUNDLE) && defined(__has_include)
#if __has_include("/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/juhani_vfs_bundle.h")
#include "/home/ethansm/Games/KOTOR2_Mod_Workspace/k1_port_stage/vfs_bundle/juhani_vfs_bundle.h"
#define JUHANI_HAVE_BUNDLE 1
#endif
#endif

#include "JuhaniBundle.h"

namespace juhanichar {

BundleResult registerBundle(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner) {
    BundleResult r = {false, false, 0, 0, 0, 0, 0};
#if defined(JUHANI_HAVE_BUNDLE)
    r.present = true;
    r.expectedAssets = k2vfs_juhani::kAssetCount;
    r.expectedCells = k2vfs_juhani::k2DaCellCount;
    k2vfs_juhani::RegisterCounts c = {0, 0, 0};
    r.ok = k2vfs_juhani::registerAll(vfs, charlib, owner, &c);
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
