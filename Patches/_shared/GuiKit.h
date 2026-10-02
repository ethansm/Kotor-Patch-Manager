// Forwarding header (shared-logging Phase 0): the canonical copy is Patches/Common/GuiKit.h, which is on every
// patch's include path, so new code writes #include "GuiKit.h". Existing patches include "../_shared/GuiKit.h"
// and keep working unchanged; this file goes away once they have switched.
#pragma once
#include "../Common/GuiKit.h"
