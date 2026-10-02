/* Test fixture: a patch module that uses "test.counter" lazily, the pattern the
 * API header prescribes. KPatchInit only stores the api pointer; the require
 * happens at first use, so it works whatever order the modules were inited in,
 * and a missing provider takes a fallback instead of crashing.
 */
#include "kpatch_api.h"

#define EXPORT __attribute__((visibility("default")))

typedef struct TestCounter {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *Add)(int32_t);
} TestCounter;

static int g_init_count;
static const KPatchApi* g_api;
static const TestCounter* g_counter;
static int g_used_fallback;

EXPORT void KPATCH_CALL KPatchInit(const KPatchApi* api) {
    ++g_init_count;
    g_api = api;
}

/* Provider result when present, else -1000 - x (and the fallback flag is set).
 * The require runs on the first call that finds no cached interface, so a
 * provider inited after this module is still found. */
EXPORT int consumer_call(int32_t x) {
    if (!g_counter && g_api) {
        g_counter = (const TestCounter*)g_api->require("test.counter", 1);
    }
    if (!g_counter) {
        g_used_fallback = 1;
        return -1000 - x;
    }
    return g_counter->Add(x);
}

EXPORT int consumer_used_fallback(void) {
    return g_used_fallback;
}

EXPORT int fixture_init_count(void) {
    return g_init_count;
}
