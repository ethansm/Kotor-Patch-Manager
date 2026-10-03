// Forwarding header (shared-logging Phase 0): the canonical copy is Patches/Common/Party.h, which is on every
// patch's include path, so new code writes #include "Party.h". Existing patches include "../_shared/Party.h"
// and keep working unchanged; this file goes away once they have switched.
#pragma once
#include "../Common/Party.h"
