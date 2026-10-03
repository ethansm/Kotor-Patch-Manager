/* Test fixture: a patch module that provides "test.counter" v1 from KPatchInit.
 *
 * Plain C11 on purpose. Fixtures are patch-side code, so they include the
 * canonical kpatch_api.h directly, which also proves the header compiles as C.
 *
 * fixture_init_count() lets the host test see how many times KPatchInit ran in
 * this module instance; its statics are reset only by a real unload.
 */
#include "kpatch_api.h"

#define EXPORT __attribute__((visibility("default")))

typedef struct TestCounter {
    uint32_t struct_size;
    int32_t (KPATCH_CALL *Add)(int32_t);
} TestCounter;

static int g_init_count;
static const KPatchApi* g_api;

static int32_t KPATCH_CALL CounterAdd(int32_t x) {
    return x + 1;
}

static const TestCounter g_counter = { sizeof(TestCounter), CounterAdd };

EXPORT void KPATCH_CALL KPatchInit(const KPatchApi* api) {
    ++g_init_count;
    g_api = api;
    api->provide("test.counter", 1, &g_counter);
}

EXPORT int fixture_init_count(void) {
    return g_init_count;
}

/* Whether the api pointer was handed over, so the test can check it too. */
EXPORT int fixture_has_api(void) {
    return g_api != NULL;
}
