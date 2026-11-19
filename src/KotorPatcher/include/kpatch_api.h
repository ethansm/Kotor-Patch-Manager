#pragma once
// The canonical header is shared with the patch modules (Patches/Common), so the
// core includes it through this forwarder instead of adding Patches/Common to the
// include path: that directory also holds a Platform.h that would collide
// case-insensitively with this directory's platform.h.
#include "../../../Patches/Common/kpatch_api.h"
