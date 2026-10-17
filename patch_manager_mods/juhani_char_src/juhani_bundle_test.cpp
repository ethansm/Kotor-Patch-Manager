// Host test for the stub path: test.toml builds with -DJUHANI_NO_BUNDLE, so registerBundle must report
// "no bundle" and make no API call. The bundle path itself is covered by test_k1_port_vfs_pack.py
// (registerAll against the real CharacterProvider.h) and by the build_check DLL size gate.
#include "JuhaniBundle.cpp"

#include <cstdio>

namespace {

int g_calls = 0;
int g_fail = 0;

#define CHECK(c) do { if (!(c)) { std::printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); ++g_fail; } } while (0)

int32_t KPATCH_CALL fakeAsset(const K2VfsAssetEntry*, const char*) { ++g_calls; return 1; }
int32_t KPATCH_CALL fakeAssets(const K2VfsAssetEntry*, uint32_t n, const char*) { ++g_calls; return static_cast<int32_t>(n); }
int32_t KPATCH_CALL fakeCells(const K2Vfs2DaCell*, uint32_t n, const char*) { ++g_calls; return static_cast<int32_t>(n); }
int32_t KPATCH_CALL fakeHas(const char*, uint16_t, uint16_t) { ++g_calls; return 0; }
int32_t KPATCH_CALL fakeChar(const K2CharacterDefinition*, const char*) { ++g_calls; return 1; }

} // namespace

int main() {
    K2AssetVfsApi vfs = {};
    vfs.struct_size = sizeof(vfs);
    vfs.RegisterAsset = fakeAsset;
    vfs.RegisterAssets = fakeAssets;
    vfs.Register2DaCells = fakeCells;
    vfs.HasAsset = fakeHas;
    K2CharacterLibApi lib = {};
    lib.struct_size = sizeof(lib);
    lib.RegisterCharacter = fakeChar;

#if defined(JUHANI_HAVE_BUNDLE)
    CHECK(!"JUHANI_NO_BUNDLE must keep the bundle out of the host test");
#endif
    juhanichar::BundleResult r = juhanichar::registerBundle(&vfs, &lib, "juhani-char");
    CHECK(!r.present);
    CHECK(!r.ok);
    CHECK(r.expectedAssets == 0 && r.expectedCells == 0);
    CHECK(r.assets == 0 && r.cells == 0 && r.character == 0);
    CHECK(g_calls == 0);

    r = juhanichar::registerBundle(nullptr, nullptr, nullptr);
    CHECK(!r.present && !r.ok && g_calls == 0);

    std::printf("juhani_bundle_test: %s (%d failure(s))\n", g_fail ? "FAIL" : "PASS", g_fail);
    return g_fail ? 1 : 0;
}
