/* Test fixture: a patch module that logs through "kpatch.log" v1 from KPatchInit.
 *
 * Plain C11, like the other fixtures. Used by the host smoke run (a patcher with a
 * DLL_ONLY entry for this module and a kplog.ini); no unit test loads it.
 *
 * A real patch would defer its logging out of KPatchInit (store the api, log from
 * its first hook call). Submitting from KPatchInit is allowed, though: Submit is
 * non-blocking and does no I/O, which is the DllMain-class contract KPatchInit
 * runs under. This fixture does it to prove a line submitted that early still
 * reaches the file.
 */
#include <string.h>

#include "kpatch_api.h"

#define EXPORT __attribute__((visibility("default")))

EXPORT void KPATCH_CALL KPatchInit(const KPatchApi* api) {
    const KPatchLogApi* log;
    int32_t patch;
    int32_t channel;
    const char* warn = "hello from KPatchInit";
    const char* info = "info line";

    log = (const KPatchLogApi*)api->require(KPATCH_LOG_IFACE, KPATCH_LOG_VERSION);
    /* struct_size is how a consumer knows which fields the provider has. */
    if (!log || log->struct_size < sizeof(KPatchLogApi)) {
        return;
    }

    patch = log->Register("logger-fixture");
    channel = log->Channel(patch, "main");
    if (patch < 0 || channel < 0) {
        return;
    }

    /* WARN is always recorded; INFO only when kplog.ini enables it. */
    log->Submit(channel, KPLOG_WARN, warn, (uint32_t)strlen(warn));
    log->Submit(channel, KPLOG_INFO, info, (uint32_t)strlen(info));
}
