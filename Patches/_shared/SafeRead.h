// Forwarding header (shared-logging Phase 0): the canonical copy is Patches/Common/SafeRead.h, which is on every
// patch's include path, so new code writes #include "SafeRead.h". Existing patches include "../_shared/SafeRead.h"
// and keep working unchanged; this file goes away once they have switched.
#pragma once
#include "../Common/SafeRead.h"
