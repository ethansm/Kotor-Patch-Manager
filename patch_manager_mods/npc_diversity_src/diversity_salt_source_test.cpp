// Host test of the NB1 salt source (DiversitySaltSource.h): the real k2nw::Client against a fake k2.npcworld provider table.
// visit_variety off or no provider -> salt 0 (hashes identical to the plain per-creature ones); on -> npc-world's VisitSalt(),
// constant inside a visit, different between visits; the late-provider retry.
#include "DiversitySaltSource.h"
#include "DiversityLogic.h"
#include "NpcWorld.h"
#include "check.h"
#include <set>

using namespace NpcDiversity;

static uint32_t g_salt = 0;
static int g_requires = 0;
static K2NpcWorldApi g_api;
static bool g_providerUp = false;

static int32_t KPATCH_CALL fReg(int32_t, K2NwCallback, void*, const char*) { return 1; }
static void KPATCH_CALL fUnreg(int32_t) {}
static uint32_t KPATCH_CALL fSalt() { return g_salt; }
static int32_t KPATCH_CALL fSeq() { return 0; }
static int32_t KPATCH_CALL fArea(char* b, uint32_t) { if (b) b[0] = 0; return 0; }
static int32_t KPATCH_CALL fTag(const char*) { return 0; }
static uint32_t KPATCH_CALL fSpawn(const char*, const char*, float, float, float, float) { return 0; }
static int32_t KPATCH_CALL fCount() { return 0; }
static const void* KPATCH_CALL fRequire(const char* name, uint32_t) {
    ++g_requires;
    return (g_providerUp && name && !strcmp(name, K2NW_IFACE)) ? &g_api : nullptr;
}

int main() {
    g_api = {};
    g_api.struct_size = sizeof g_api;
    g_api.Register = fReg; g_api.Unregister = fUnreg; g_api.VisitSalt = fSalt; g_api.VisitSeq = fSeq; g_api.CurrentArea = fArea;
    g_api.TagPresent = fTag; g_api.Spawn = fSpawn; g_api.SpawnedCount = fCount; g_api.SpawnCap = fCount;
    KPatchApi api = {};
    api.struct_size = sizeof api;
    api.require = fRequire;

    // 1. no provider: salt 0 with the switch on, and nothing breaks
    {
        k2nw::Client nw; nw.init(&api, "npc-diversity"); g_providerUp = false; g_requires = 0;
        nw.connect();
        KT_CHECK_EQ(VisitSaltFrom(true, nw), 0u);
        KT_CHECK_EQ(VisitSaltFrom(false, nw), 0u);
    }
    // 2. switch off: salt 0 even with a provider, and the provider is not even asked
    {
        k2nw::Client nw; nw.init(&api, "npc-diversity"); g_providerUp = true; g_salt = 0xABCDEF01u; g_requires = 0;
        KT_CHECK_EQ(VisitSaltFrom(false, nw), 0u);
        KT_CHECK_EQ(g_requires, 0);
    }
    // 3. switch on + provider: the library's salt
    {
        k2nw::Client nw; nw.init(&api, "npc-diversity"); g_providerUp = true; g_salt = 0x11112222u;
        KT_CHECK(nw.connect());
        KT_CHECK_EQ(VisitSaltFrom(true, nw), 0x11112222u);
        g_salt = 0x33334444u;   // the next area entry
        KT_CHECK_EQ(VisitSaltFrom(true, nw), 0x33334444u);
        g_salt = 0;             // before the first load
        KT_CHECK_EQ(VisitSaltFrom(true, nw), 0u);
    }
    // 4. the provider loads later than npc-diversity: the first poll() (count 0) connects without waiting for 256 calls
    {
        k2nw::Client nw; nw.init(&api, "npc-diversity"); g_providerUp = false; g_salt = 0x55556666u;
        KT_CHECK(!nw.connect());
        g_providerUp = true;
        uint32_t got = 0;
        for (int i = 0; i < 600 && got == 0; ++i) got = VisitSaltFrom(true, nw);
        KT_CHECK_EQ(got, 0x55556666u);
    }
    // 5. decisions: salt 0 equals the plain hashes (nothing changes by default); a new salt changes some looks; one salt is deterministic
    {
        int diffVary = 0, diffTv = 0, same = 0;
        for (uint32_t oid = 0x80000001u; oid < 0x80000001u + 400; ++oid) {
            const uint16_t app = 596;
            KT_CHECK_EQ(HashCreatureId(oid, app, 0u), HashCreatureId(oid, app));
            const bool v0 = ShouldVary(oid, app, 0u), v1 = ShouldVary(oid, app, 0x11112222u), v2 = ShouldVary(oid, app, 0x33334444u);
            KT_CHECK_EQ(int(ShouldVary(oid, app, 0x11112222u)), int(v1));   // same visit, same answer
            diffVary += (v1 != v2);
            same += (v0 == ShouldVary(oid, app));
            diffTv += (PickTextureVar(oid, 596, 0, 0x11112222u) != PickTextureVar(oid, 596, 0, 0x33334444u));
        }
        KT_CHECK_EQ(same, 400);
        KT_CHECK(diffVary > 20);   // ~kVaryPct of oids flip between two visits
        KT_CHECK(diffTv > 20);
    }
    return kt::Report("diversity_salt_source_test");
}
