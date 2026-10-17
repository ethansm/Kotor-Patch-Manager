// Host unit test for CharacterProvider.h (Phase VFS-C3).
// Tests ABI struct layouts, k2char::Catalog, and k2char::Client with fake KPatchApi.
// Build & run: PYTHONUNBUFFERED=1 python3 -u scripts/run_tests.py character_provider < /dev/null 2>&1
#include "CharacterProvider.h"
#include "check.h"

#include <cstring>
#include <vector>

// ---- Fake KPatchApi Registry for In-Process Testing ----
namespace fakekp {

struct Entry {
    const char* name;
    uint32_t version;
    const void* iface;
};

static Entry g_entries[32];
static uint32_t g_entryCount = 0;
static int g_provideCalls = 0;
static int g_requireCalls = 0;

static int32_t KPATCH_CALL provide(const char* name, uint32_t version, const void* iface) {
    ++g_provideCalls;
    if (!name || !iface || g_entryCount >= 32) return 1;
    for (uint32_t i = 0; i < g_entryCount; ++i) {
        if (std::strcmp(g_entries[i].name, name) == 0 && g_entries[i].version == version) {
            g_entries[i].iface = iface;
            return 0;
        }
    }
    g_entries[g_entryCount++] = { name, version, iface };
    return 0;
}

static const void* KPATCH_CALL require(const char* name, uint32_t version) {
    ++g_requireCalls;
    if (!name) return nullptr;
    for (uint32_t i = 0; i < g_entryCount; ++i) {
        if (std::strcmp(g_entries[i].name, name) == 0 && g_entries[i].version == version) {
            return g_entries[i].iface;
        }
    }
    return nullptr;
}

static void reset() {
    g_entryCount = 0;
    g_provideCalls = 0;
    g_requireCalls = 0;
}

static const KPatchApi g_api = {
    sizeof(KPatchApi),
    1,
    provide,
    require
};

static const KPatchApi* api() { return &g_api; }

} // namespace fakekp

// ---- Fake Service Providers for End-to-End Delegation ----
namespace fakesvc {

static int g_charRegCalls = 0;
static char g_lastOwner[64] = {};
static K2CharacterDefinition g_lastDef = {};
static int g_hasCharCalls = 0;
static int g_getCharCalls = 0;
static int g_isAppCalls = 0;
static int g_spawnCalls = 0;
static char g_spawnChar[32] = {};
static char g_spawnArea[32] = {};
static float g_spawnX = 0.f, g_spawnY = 0.f, g_spawnZ = 0.f, g_spawnFacing = 0.f;

static int32_t KPATCH_CALL fakeRegisterCharacter(const K2CharacterDefinition* def, const char* owner) {
    ++g_charRegCalls;
    if (owner) std::snprintf(g_lastOwner, sizeof(g_lastOwner), "%s", owner);
    if (def) g_lastDef = *def;
    return 1;
}

static int32_t KPATCH_CALL fakeHasCharacter(const char* character_id) {
    ++g_hasCharCalls;
    return (character_id && k2char::strEqualNoCase(character_id, "juhani")) ? 1 : 0;
}

static int32_t KPATCH_CALL fakeGetCharacter(const char* character_id, K2CharacterDefinition* out_def) {
    ++g_getCharCalls;
    if (!character_id || !out_def) return 0;
    if (k2char::strEqualNoCase(character_id, "juhani")) {
        out_def->struct_size = sizeof(K2CharacterDefinition);
        out_def->character_id = "juhani";
        out_def->display_name = "Juhani";
        out_def->default_tag = "k1p_juhani";
        out_def->default_utc = "k1p_juhani";
        out_def->default_dlg = "kvjdlg01";
        out_def->appearance_id = 818;
        out_def->head_id = 250;
        out_def->soundset_id = 70;
        return 1;
    }
    return 0;
}

static int32_t KPATCH_CALL fakeIsCharacterAppearance(uint32_t appearance_id) {
    ++g_isAppCalls;
    return (appearance_id == 818) ? 1 : 0;
}

static uint32_t KPATCH_CALL fakeSpawn(const char* character_id, const char* area_resref,
                                      float x, float y, float z, float facingRadians) {
    ++g_spawnCalls;
    if (character_id) std::snprintf(g_spawnChar, sizeof(g_spawnChar), "%s", character_id);
    if (area_resref) std::snprintf(g_spawnArea, sizeof(g_spawnArea), "%s", area_resref);
    g_spawnX = x; g_spawnY = y; g_spawnZ = z; g_spawnFacing = facingRadians;
    return 0x12345678u;
}

static const K2CharacterLibApi g_charLibApi = {
    sizeof(K2CharacterLibApi),
    fakeRegisterCharacter,
    fakeHasCharacter,
    fakeGetCharacter,
    fakeIsCharacterAppearance,
    fakeSpawn
};

// Fake AssetVFS
static int g_vfsAssetCalls = 0;
static int g_vfsAssetsCalls = 0;
static int g_vfs2DaCalls = 0;
static int g_vfsHasCalls = 0;
static uint32_t g_lastAssetCount = 0;
static uint32_t g_lastCellCount = 0;

static int32_t KPATCH_CALL fakeRegisterAsset(const K2VfsAssetEntry*, const char*) {
    ++g_vfsAssetCalls;
    return 1;
}

static int32_t KPATCH_CALL fakeRegisterAssets(const K2VfsAssetEntry*, uint32_t count, const char*) {
    ++g_vfsAssetsCalls;
    g_lastAssetCount = count;
    return 1;
}

static int32_t KPATCH_CALL fakeRegister2DaCells(const K2Vfs2DaCell*, uint32_t count, const char*) {
    ++g_vfs2DaCalls;
    g_lastCellCount = count;
    return 1;
}

static int32_t KPATCH_CALL fakeHasAsset(const char* resref, uint16_t, uint16_t) {
    ++g_vfsHasCalls;
    return (resref && *resref) ? 1 : 0;
}

static int g_vfsRegMissCalls = 0;
static int g_vfsUnregMissCalls = 0;
static int g_vfsRegStreamCalls = 0;
static int g_vfsUnregStreamCalls = 0;
static int32_t g_lastUnregMissToken = 0;
static int32_t g_lastUnregStreamToken = 0;

static int32_t KPATCH_CALL fakeRegisterResManMissHandler(K2VfsResManMissFn, void*, const char*) {
    ++g_vfsRegMissCalls;
    return 42;
}

static void KPATCH_CALL fakeUnregisterResManMissHandler(int32_t token) {
    ++g_vfsUnregMissCalls;
    g_lastUnregMissToken = token;
}

static int32_t KPATCH_CALL fakeRegisterStreamFilter(K2VfsStreamFilterFn, void*, const char*) {
    ++g_vfsRegStreamCalls;
    return 99;
}

static void KPATCH_CALL fakeUnregisterStreamFilter(int32_t token) {
    ++g_vfsUnregStreamCalls;
    g_lastUnregStreamToken = token;
}

static const K2AssetVfsApi g_assetVfsApi = {
    sizeof(K2AssetVfsApi),
    fakeRegisterAsset,
    fakeRegisterAssets,
    fakeRegister2DaCells,
    fakeHasAsset,
    fakeRegisterResManMissHandler,
    fakeUnregisterResManMissHandler,
    fakeRegisterStreamFilter,
    fakeUnregisterStreamFilter
};

static void resetAll() {
    fakekp::reset();
    g_charRegCalls = 0;
    g_lastOwner[0] = '\0';
    g_lastDef = {};
    g_hasCharCalls = 0;
    g_getCharCalls = 0;
    g_isAppCalls = 0;
    g_spawnCalls = 0;
    g_spawnChar[0] = '\0';
    g_spawnArea[0] = '\0';
    g_spawnX = g_spawnY = g_spawnZ = g_spawnFacing = 0.f;

    g_vfsAssetCalls = 0;
    g_vfsAssetsCalls = 0;
    g_vfs2DaCalls = 0;
    g_vfsHasCalls = 0;
    g_lastAssetCount = 0;
    g_lastCellCount = 0;
    g_vfsRegMissCalls = 0;
    g_vfsUnregMissCalls = 0;
    g_vfsRegStreamCalls = 0;
    g_vfsUnregStreamCalls = 0;
    g_lastUnregMissToken = 0;
    g_lastUnregStreamToken = 0;
}

} // namespace fakesvc

// ---- Test Suites ----

static void testAbiLayoutAndUsable() {
    // 1. Fixed structure size and offset checks
    KT_CHECK_EQ(sizeof(K2VfsAssetEntry), 2 * sizeof(void*) + 16);
    KT_CHECK_EQ(offsetof(K2VfsAssetEntry, data), 2 * sizeof(void*));
    KT_CHECK_EQ(offsetof(K2AssetVfsApi, HasAsset), 4 * sizeof(void*));
    KT_CHECK_EQ(offsetof(K2AssetVfsApi, UnregisterStreamFilter), 8 * sizeof(void*));
    KT_CHECK_EQ(sizeof(K2AssetVfsApi), 9 * sizeof(void*));

    KT_CHECK_EQ(offsetof(K2CharacterLibApi, Spawn), 5 * sizeof(void*));
    KT_CHECK_EQ(sizeof(K2CharacterLibApi), 6 * sizeof(void*));

    // 2. Client::usable for K2CharacterLibApi
    {
        KT_CHECK(k2char::Client::usable(&fakesvc::g_charLibApi));
        KT_CHECK(!k2char::Client::usable(nullptr));

        // Table too small (ends before Spawn)
        K2CharacterLibApi shortLib = fakesvc::g_charLibApi;
        shortLib.struct_size = offsetof(K2CharacterLibApi, Spawn);
        KT_CHECK(!k2char::Client::usable(&shortLib));

        // Missing function pointer (hole in table)
        K2CharacterLibApi nullFn = fakesvc::g_charLibApi;
        nullFn.Spawn = nullptr;
        KT_CHECK(!k2char::Client::usable(&nullFn));

        nullFn = fakesvc::g_charLibApi;
        nullFn.RegisterCharacter = nullptr;
        KT_CHECK(!k2char::Client::usable(&nullFn));

        nullFn = fakesvc::g_charLibApi;
        nullFn.HasCharacter = nullptr;
        KT_CHECK(!k2char::Client::usable(&nullFn));

        nullFn = fakesvc::g_charLibApi;
        nullFn.GetCharacter = nullptr;
        KT_CHECK(!k2char::Client::usable(&nullFn));

        nullFn = fakesvc::g_charLibApi;
        nullFn.IsCharacterAppearance = nullptr;
        KT_CHECK(!k2char::Client::usable(&nullFn));

        // Table with larger struct_size (future provider extension)
        K2CharacterLibApi largerLib = fakesvc::g_charLibApi;
        largerLib.struct_size = sizeof(K2CharacterLibApi) + 32;
        KT_CHECK(k2char::Client::usable(&largerLib));
    }

    // 3. Client::usableVfs and usableVfsHooks for K2AssetVfsApi
    {
        KT_CHECK(k2char::Client::usableVfs(&fakesvc::g_assetVfsApi));
        KT_CHECK(!k2char::Client::usableVfs(nullptr));

        K2AssetVfsApi shortVfs = fakesvc::g_assetVfsApi;
        shortVfs.struct_size = offsetof(K2AssetVfsApi, HasAsset);
        KT_CHECK(!k2char::Client::usableVfs(&shortVfs));

        K2AssetVfsApi nullVfs = fakesvc::g_assetVfsApi;
        nullVfs.HasAsset = nullptr;
        KT_CHECK(!k2char::Client::usableVfs(&nullVfs));

        nullVfs = fakesvc::g_assetVfsApi;
        nullVfs.RegisterAssets = nullptr;
        KT_CHECK(!k2char::Client::usableVfs(&nullVfs));

        K2AssetVfsApi largerVfs = fakesvc::g_assetVfsApi;
        largerVfs.struct_size = sizeof(K2AssetVfsApi) + 16;
        KT_CHECK(k2char::Client::usableVfs(&largerVfs));

        // usableVfsHooks checks
        KT_CHECK(k2char::Client::usableVfsHooks(&fakesvc::g_assetVfsApi));
        KT_CHECK(!k2char::Client::usableVfsHooks(nullptr));

        // Legacy 5-pointer table is usableVfs but not usableVfsHooks
        K2AssetVfsApi legacyVfs = fakesvc::g_assetVfsApi;
        legacyVfs.struct_size = offsetof(K2AssetVfsApi, HasAsset) + sizeof(legacyVfs.HasAsset);
        KT_CHECK(k2char::Client::usableVfs(&legacyVfs));
        KT_CHECK(!k2char::Client::usableVfsHooks(&legacyVfs));

        // Short table before UnregisterStreamFilter
        K2AssetVfsApi shortHooks = fakesvc::g_assetVfsApi;
        shortHooks.struct_size = offsetof(K2AssetVfsApi, UnregisterStreamFilter);
        KT_CHECK(!k2char::Client::usableVfsHooks(&shortHooks));

        // Holes in hooks
        K2AssetVfsApi nullHook = fakesvc::g_assetVfsApi;
        nullHook.RegisterResManMissHandler = nullptr;
        KT_CHECK(!k2char::Client::usableVfsHooks(&nullHook));

        nullHook = fakesvc::g_assetVfsApi;
        nullHook.UnregisterResManMissHandler = nullptr;
        KT_CHECK(!k2char::Client::usableVfsHooks(&nullHook));

        nullHook = fakesvc::g_assetVfsApi;
        nullHook.RegisterStreamFilter = nullptr;
        KT_CHECK(!k2char::Client::usableVfsHooks(&nullHook));

        nullHook = fakesvc::g_assetVfsApi;
        nullHook.UnregisterStreamFilter = nullptr;
        KT_CHECK(!k2char::Client::usableVfsHooks(&nullHook));
    }
}

static void testCatalog() {
    k2char::Catalog cat;
    cat.clear();
    KT_CHECK_EQ(cat.size(), 0u);

    // Setup Juhani definition
    K2CharacterDefinition juhani = {};
    juhani.struct_size = sizeof(K2CharacterDefinition);
    juhani.character_id = "juhani";
    juhani.display_name = "Juhani";
    juhani.default_tag = "k1p_juhani";
    juhani.default_utc = "k1p_juhani";
    juhani.default_dlg = "kvjdlg01";
    juhani.appearance_id = 818;
    juhani.head_id = 250;
    juhani.soundset_id = 70;

    // Setup Carth definition
    K2CharacterDefinition carth = {};
    carth.struct_size = sizeof(K2CharacterDefinition);
    carth.character_id = "carth";
    carth.display_name = "Carth";
    carth.default_tag = "k1p_carth";
    carth.default_utc = "k1p_carth";
    carth.default_dlg = "k_hcar_dialog";
    carth.appearance_id = 819;
    carth.head_id = 251;
    carth.soundset_id = 71;

    // Register Juhani and Carth
    KT_CHECK_EQ(cat.registerDef(&juhani, "test_vfs"), 1);
    KT_CHECK_EQ(cat.size(), 1u);

    KT_CHECK_EQ(cat.registerDef(&carth, "test_vfs"), 1);
    KT_CHECK_EQ(cat.size(), 2u);

    // has() case-insensitive check
    KT_CHECK_EQ(cat.has("juhani"), 1);
    KT_CHECK_EQ(cat.has("JUHANI"), 1);
    KT_CHECK_EQ(cat.has("Juhani"), 1);
    KT_CHECK_EQ(cat.has("carth"), 1);
    KT_CHECK_EQ(cat.has("CARTH"), 1);
    KT_CHECK_EQ(cat.has("bastila"), 0);
    KT_CHECK_EQ(cat.has(""), 0);
    KT_CHECK_EQ(cat.has(nullptr), 0);

    // get() check
    K2CharacterDefinition out = {};
    KT_CHECK_EQ(cat.get("juhani", &out), 1);
    KT_CHECK_EQ(out.appearance_id, 818u);
    KT_CHECK_EQ(out.head_id, 250u);
    KT_CHECK_EQ(out.soundset_id, 70u);
    KT_CHECK_EQ(out.display_name, "Juhani");
    KT_CHECK_EQ(out.default_tag, "k1p_juhani");
    KT_CHECK_EQ(out.default_utc, "k1p_juhani");
    KT_CHECK_EQ(out.default_dlg, "kvjdlg01");

    // Case-insensitive get
    K2CharacterDefinition outCarth = {};
    KT_CHECK_EQ(cat.get("CARTH", &outCarth), 1);
    KT_CHECK_EQ(outCarth.appearance_id, 819u);
    KT_CHECK_EQ(outCarth.head_id, 251u);
    KT_CHECK_EQ(outCarth.soundset_id, 71u);

    // get() failures
    KT_CHECK_EQ(cat.get("nonexistent", &out), 0);
    KT_CHECK_EQ(cat.get(nullptr, &out), 0);
    KT_CHECK_EQ(cat.get("juhani", nullptr), 0);

    // isAppearance() checks
    KT_CHECK_EQ(cat.isAppearance(818), 1);
    KT_CHECK_EQ(cat.isAppearance(819), 1);
    KT_CHECK_EQ(cat.isAppearance(17), 0);
    KT_CHECK_EQ(cat.isAppearance(0), 0);

    // Invalid definition rejections
    KT_CHECK_EQ(cat.registerDef(nullptr), 0);

    K2CharacterDefinition badSize = juhani;
    badSize.struct_size = 12; // too small
    KT_CHECK_EQ(cat.registerDef(&badSize), 0);

    K2CharacterDefinition badId = juhani;
    badId.character_id = nullptr;
    KT_CHECK_EQ(cat.registerDef(&badId), 0);

    badId.character_id = "";
    KT_CHECK_EQ(cat.registerDef(&badId), 0);

    // Duplicate rejection when allowUpdate = false
    KT_CHECK_EQ(cat.registerDef(&juhani, "another_owner", false), 0);
    KT_CHECK_EQ(cat.size(), 2u);

    // Duplicate update when allowUpdate = true
    juhani.appearance_id = 999;
    KT_CHECK_EQ(cat.registerDef(&juhani, "updated_owner", true), 1);
    KT_CHECK_EQ(cat.size(), 2u); // count did not grow
    KT_CHECK_EQ(cat.isAppearance(999), 1);
    KT_CHECK_EQ(cat.isAppearance(818), 0); // old appearance replaced

    // Revert back
    juhani.appearance_id = 818;
    KT_CHECK_EQ(cat.registerDef(&juhani), 1);
    KT_CHECK_EQ(cat.isAppearance(818), 1);
}

static void testClientInertAndPoll() {
    fakesvc::resetAll();

    // 1. Client uninitialized is inert
    {
        k2char::Client c;
        K2CharacterDefinition def = {};
        K2CharacterDefinition out = {};
        K2VfsAssetEntry asset = {};
        K2Vfs2DaCell cell = {};

        KT_CHECK(!c.connected);
        KT_CHECK(!c.connect());
        KT_CHECK_EQ(c.registerCharacter(&def), 0);
        KT_CHECK_EQ(c.hasCharacter("juhani"), 0);
        KT_CHECK_EQ(c.getCharacter("juhani", &out), 0);
        KT_CHECK_EQ(c.isCharacterAppearance(818), 0);
        KT_CHECK_EQ(c.spawn("juhani", "204tel", 0, 0, 0, 0), 0u);
        KT_CHECK_EQ(c.registerAsset(&asset), 0);
        KT_CHECK_EQ(c.registerAssets(&asset, 1), 0);
        KT_CHECK_EQ(c.register2DaCells(&cell, 1), 0);
        KT_CHECK_EQ(c.hasAsset("res", 0, 0), 0);
    }

    // 2. Client with KPatchApi but no provider provided
    {
        fakesvc::resetAll();
        k2char::Client c;
        c.init(fakekp::api(), "inert_client");

        KT_CHECK(!c.connect());
        KT_CHECK(!c.connected);
        KT_CHECK_EQ(fakekp::g_requireCalls, 2); // charlib + assetvfs queries
    }

    // 3. Lazy poll cadence: connection every 256 calls
    {
        fakesvc::resetAll();
        k2char::Client c;
        c.init(fakekp::api(), "polling_client");

        // First poll call (call 0): require called, returns null -> not connected
        c.poll();
        KT_CHECK(!c.connected);
        KT_CHECK_EQ(fakekp::g_requireCalls, 2);

        // Now provide the interface in registry
        fakekp::provide(K2_CHARLIB_IFACE, K2_CHARLIB_VERSION, &fakesvc::g_charLibApi);

        // Next 255 poll calls (pollCount becomes 1..255): not on 256 boundary, no require called
        for (int i = 0; i < 255; ++i) {
            c.poll();
        }
        KT_CHECK(!c.connected);
        KT_CHECK_EQ(fakekp::g_requireCalls, 2);

        // 257th poll call (pollCount was 256): (256 & 255) == 0 -> calls connect()!
        c.poll();
        KT_CHECK(c.connected);
        KT_CHECK_EQ(fakekp::g_requireCalls, 4); // 2 from initial poll + 2 from connect

        // Subsequent poll calls do nothing because connected == true
        c.poll();
        c.poll();
        KT_CHECK_EQ(fakekp::g_requireCalls, 4);
    }
}

static void testClientEndToEndDelegation() {
    fakesvc::resetAll();

    // Provide both services
    fakekp::provide(K2_CHARLIB_IFACE, K2_CHARLIB_VERSION, &fakesvc::g_charLibApi);
    fakekp::provide(K2_ASSETVFS_IFACE, K2_ASSETVFS_VERSION, &fakesvc::g_assetVfsApi);

    k2char::Client c;
    c.init(fakekp::api(), "diversity_dispatcher");
    KT_CHECK(c.connect());
    KT_CHECK(c.connected);
    KT_CHECK(c.lib != nullptr);
    KT_CHECK(c.vfs != nullptr);

    // 1. registerCharacter delegation
    K2CharacterDefinition juhani = {};
    juhani.struct_size = sizeof(K2CharacterDefinition);
    juhani.character_id = "juhani";
    juhani.display_name = "Juhani";
    juhani.appearance_id = 818;
    juhani.head_id = 250;
    juhani.soundset_id = 70;

    KT_CHECK_EQ(c.registerCharacter(&juhani), 1);
    KT_CHECK_EQ(fakesvc::g_charRegCalls, 1);
    KT_CHECK_EQ(fakesvc::g_lastOwner, "diversity_dispatcher");
    KT_CHECK_EQ(fakesvc::g_lastDef.appearance_id, 818u);

    // 2. hasCharacter delegation
    KT_CHECK_EQ(c.hasCharacter("juhani"), 1);
    KT_CHECK_EQ(c.hasCharacter("carth"), 0);
    KT_CHECK_EQ(fakesvc::g_hasCharCalls, 2);

    // 3. getCharacter delegation
    K2CharacterDefinition out = {};
    KT_CHECK_EQ(c.getCharacter("juhani", &out), 1);
    KT_CHECK_EQ(out.appearance_id, 818u);
    KT_CHECK_EQ(out.display_name, "Juhani");
    KT_CHECK_EQ(fakesvc::g_getCharCalls, 1);

    // 4. isCharacterAppearance delegation
    KT_CHECK_EQ(c.isCharacterAppearance(818), 1);
    KT_CHECK_EQ(c.isCharacterAppearance(17), 0);
    KT_CHECK_EQ(fakesvc::g_isAppCalls, 2);

    // 5. spawn delegation
    uint32_t oid = c.spawn("juhani", "204tel", 12.5f, 34.0f, -1.0f, 3.14159f);
    KT_CHECK_EQ(oid, 0x12345678u);
    KT_CHECK_EQ(fakesvc::g_spawnCalls, 1);
    KT_CHECK_EQ(fakesvc::g_spawnChar, "juhani");
    KT_CHECK_EQ(fakesvc::g_spawnArea, "204tel");
    KT_CHECK(fakesvc::g_spawnX == 12.5f);
    KT_CHECK(fakesvc::g_spawnY == 34.0f);
    KT_CHECK(fakesvc::g_spawnZ == -1.0f);
    KT_CHECK(fakesvc::g_spawnFacing == 3.14159f);

    // 6. registerAsset & registerAssets delegation
    K2VfsAssetEntry entries[2] = {};
    entries[0].resref = "p_juhani";
    entries[0].res_type = 3000;
    entries[1].resref = "p_juhanih";
    entries[1].res_type = 3000;

    KT_CHECK_EQ(c.registerAsset(&entries[0]), 1);
    KT_CHECK_EQ(fakesvc::g_vfsAssetCalls, 1);

    KT_CHECK_EQ(c.registerAssets(entries, 2), 1);
    KT_CHECK_EQ(fakesvc::g_vfsAssetsCalls, 1);
    KT_CHECK_EQ(fakesvc::g_lastAssetCount, 2u);

    // 7. register2DaCells delegation
    K2Vfs2DaCell cells[3] = {};
    cells[0] = { "appearance", 818, 0, "race", "juhani" };
    cells[1] = { "heads", 250, 0, "head", "p_juhanih" };
    cells[2] = { "soundset", 70, 0, "resref", "vs_juhani" };

    KT_CHECK_EQ(c.register2DaCells(cells, 3), 1);
    KT_CHECK_EQ(fakesvc::g_vfs2DaCalls, 1);
    KT_CHECK_EQ(fakesvc::g_lastCellCount, 3u);

    // 8. hasAsset delegation
    KT_CHECK_EQ(c.hasAsset("p_juhani", 3000, 0), 1);
    KT_CHECK_EQ(fakesvc::g_vfsHasCalls, 1);

    // 9. registerResManMissHandler & unregisterResManMissHandler delegation
    auto dummyMissFn = [](const char*, uint16_t, K2VfsAssetEntry*, void*) -> int32_t { return 0; };
    int32_t missTok = c.registerResManMissHandler(dummyMissFn);
    KT_CHECK_EQ(missTok, 42);
    KT_CHECK_EQ(fakesvc::g_vfsRegMissCalls, 1);

    c.unregisterResManMissHandler(missTok);
    KT_CHECK_EQ(fakesvc::g_vfsUnregMissCalls, 1);
    KT_CHECK_EQ(fakesvc::g_lastUnregMissToken, 42);

    // 10. registerStreamFilter & unregisterStreamFilter delegation
    auto dummyFilterFn = [](const char*, uint16_t, const void**, uint32_t*, char*, uint32_t, void*) -> int32_t { return 0; };
    int32_t filterTok = c.registerStreamFilter(dummyFilterFn);
    KT_CHECK_EQ(filterTok, 99);
    KT_CHECK_EQ(fakesvc::g_vfsRegStreamCalls, 1);

    c.unregisterStreamFilter(filterTok);
    KT_CHECK_EQ(fakesvc::g_vfsUnregStreamCalls, 1);
    KT_CHECK_EQ(fakesvc::g_lastUnregStreamToken, 99);

    // 11. release() resets client to inert state
    c.release();
    KT_CHECK(!c.connected);
    KT_CHECK(c.lib == nullptr);
    KT_CHECK(c.vfs == nullptr);
    KT_CHECK_EQ(c.isCharacterAppearance(818), 0);
    KT_CHECK_EQ(c.spawn("juhani", "204tel", 0, 0, 0, 0), 0u);
    KT_CHECK_EQ(c.registerResManMissHandler(dummyMissFn), 0);
    KT_CHECK_EQ(c.registerStreamFilter(dummyFilterFn), 0);
}

int main() {
    testAbiLayoutAndUsable();
    testCatalog();
    testClientInertAndPoll();
    testClientEndToEndDelegation();
    return kt::Report("character_provider_test");
}
