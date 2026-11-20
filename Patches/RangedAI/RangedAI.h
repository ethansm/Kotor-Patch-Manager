/* RangedAI.h -- the "k2.rangedai" v1 registry interface (provider: the ranged-ai patch) + a C++ consumer helper.
 *
 * Why: Ranged AI owns party stance 16 ("Ranged (Kiting)") and the engine rules that map stance 16 to vanilla
 * Ranged (11) under weapon/range conditions (E-H3 hook). Companion AI's A1 hook
 * reads [ebp-0x174] and needs to know whether the attacker's stance is the kite stance and whether it maps to
 * ranged, avoiding silent misses when ranged-ai is absent or ranged_rules=0.
 * Design: project design doc 13:235, 19b_cross_patch_sharing_survey.md:20 (candidate A5).
 *
 * Plain C ABI (kpatch_api.h rules): fixed-width ints and function pointers only, KPATCH_CALL (cdecl), structs only grow,
 * check struct_size before reading a field newer than your oldest supported provider.
 */
#ifndef K2_RANGEDAI_H
#define K2_RANGEDAI_H

#include "kpatch_api.h"
#include <stddef.h>
#include <stdint.h>

#define K2RAI_IFACE   "k2.rangedai"
#define K2RAI_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

typedef struct K2RangedAiApi {
    uint32_t struct_size;
    /* Returns the kite stance id (16 when enabled and ranged_rules is active), or 0 if inactive/disabled. */
    int32_t (KPATCH_CALL *KiteStance)(void);
    /* Returns 1 if raw stance maps to vanilla ranged stance (11) under current weapon/range conditions, 0 otherwise.
     * ebp: stack frame of CSWSCreature::AIActionPhysicalAttacks (can be 0 to check stance only without frame context). */
    int32_t (KPATCH_CALL *MapsToRanged)(int32_t rawStance, intptr_t ebp);
} K2RangedAiApi;

#pragma pack(pop)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
#define K2RAI_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define K2RAI_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#ifdef K2RAI_STATIC_ASSERT
K2RAI_STATIC_ASSERT(offsetof(K2RangedAiApi, KiteStance) == sizeof(void*), "K2RangedAiApi.KiteStance offset");
K2RAI_STATIC_ASSERT(offsetof(K2RangedAiApi, MapsToRanged) == 2 * sizeof(void*), "K2RangedAiApi.MapsToRanged offset");
K2RAI_STATIC_ASSERT(sizeof(K2RangedAiApi) == 3 * sizeof(void*), "K2RangedAiApi size");
#undef K2RAI_STATIC_ASSERT
#endif

#ifdef __cplusplus
namespace k2rai {

struct Client {
    const KPatchApi* api = nullptr;
    const K2RangedAiApi* rai = nullptr;
    const char* owner = "";
    bool connected = false;
    unsigned pollCount = 0;
    void (*onConnect)(bool ok, const char* why) = nullptr;
    bool reportedMissing = false;

    void init(const KPatchApi* a, const char* id) {
        api = a;
        owner = id ? id : "";
        rai = nullptr;
        connected = false;
        pollCount = 0;
        reportedMissing = false;
    }

    static bool usable(const K2RangedAiApi* p) {
        return p && p->struct_size >= sizeof(K2RangedAiApi) && p->KiteStance && p->MapsToRanged;
    }

    bool connect() {
        if (connected && rai) return true;
        if (!api || api->struct_size < offsetof(KPatchApi, require) + sizeof(api->require) || !api->require) {
            if (!reportedMissing && onConnect) onConnect(false, "no KPatchApi (patcher predates the registry?)");
            reportedMissing = true;
            return false;
        }
        const K2RangedAiApi* p = static_cast<const K2RangedAiApi*>(api->require(K2RAI_IFACE, K2RAI_VERSION));
        if (!usable(p)) {
            if (!reportedMissing && onConnect) onConnect(false, p ? "k2.rangedai v1 table too small" : "k2.rangedai v1 not provided");
            reportedMissing = true;
            return false;
        }
        rai = p;
        connected = true;
        if (onConnect) onConnect(true, "connected");
        return true;
    }

    void poll() { if (!connected && (pollCount++ & 255) == 0) connect(); }

    int32_t kiteStance(int32_t fallback = 0) const {
        return (rai && rai->KiteStance) ? rai->KiteStance() : fallback;
    }

    bool mapsToRanged(int32_t rawStance, intptr_t ebp, bool fallback = false) const {
        return (rai && rai->MapsToRanged) ? (rai->MapsToRanged(rawStance, ebp) != 0) : fallback;
    }
};

} // namespace k2rai
#endif

#endif /* K2_RANGEDAI_H */
