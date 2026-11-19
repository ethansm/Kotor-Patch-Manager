/* Test fixture: a patch module with no KPatchInit export, i.e. every patch that
 * exists today. The registry must treat it as normal: no call, no error log.
 * It includes the header only as a patch that compiles against it would.
 */
#include "kpatch_api.h"

#define EXPORT __attribute__((visibility("default")))

EXPORT int noinit_value(void) {
    return 42;
}
