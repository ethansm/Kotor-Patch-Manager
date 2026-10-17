// JuhaniBundle.h -- registers the private Juhani and Carth VFS bundles (doc 44 VFS-B1/B2) with asset-vfs.
// JuhaniBundle.cpp / CarthBundle.cpp are the only TUs that include a generated bundle header (its top-level .incbin asm blocks
// switch sections, so it is included first in a TU of its own).
#pragma once

#include <stdint.h>

#include "CharacterProvider.h"

namespace juhanichar {

struct BundleResult {
    bool present;            // the generated bundle header was compiled in
    bool ok;                 // registerAll succeeded (all assets accepted, cells >= 0, character > 0)
    uint32_t expectedAssets; // kAssetCount (0 without a bundle)
    uint32_t expectedCells;  // k2DaCellCount (0 without a bundle)
    int32_t assets;          // RegisterAssets return
    int32_t cells;           // Register2DaCells return
    int32_t character;       // RegisterCharacter return
};

// Registers everything; null tables give ok=false. Without a bundle: present=false, ok=false, no API call.
BundleResult registerBundle(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner);

// Same contract for the Carth bundle (CarthBundle.cpp).
BundleResult registerCarthBundle(const K2AssetVfsApi* vfs, const K2CharacterLibApi* charlib, const char* owner);

} // namespace juhanichar
