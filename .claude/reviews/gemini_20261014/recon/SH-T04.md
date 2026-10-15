# SH-T04
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3a-05 | CONFIRMED | patch_manager_mods/_shared_src/GuiKit.h:296,298-300; patch_manager_mods/_shared_src/HudKit.h:45 | no | Null loadImage result is pooled; retry hits pooled null without reload. Un-pooling re-runs loadImage per call. |
| SH-3c-01 | CONFIRMED | patch_manager_mods/_shared_src/hudkit_test.cpp:60,67,68 | yes | No ensure() call and no repeat-null assertion; line 60 is the no-pool path. |

- GuiKit.h:293: imageFor returns nullptr before touching the pool when loadImage is null or resref is empty.
- GuiKit.h:296: pool hit returns the stored pointer even when that pointer is null.
- GuiKit.h:297: pool-full path returns nullptr without calling loadImage and without pooling.
- GuiKit.h:298-300: loadImage result stored unconditionally (null included), n incremented, *loaded set true.
- GuiKit.h:285-287: header comment documents that a null result occupies a pool entry by design.
- HudKit.h:42-46: image() caches only non-null in its static cache; null re-enters imageFor and gets the pooled null back.
- HudKit.h:38-39: ensure() only forwards to embtex::ensure; no guikit pool reset exists in the ranges read.
- hudkit_test.cpp:59-60: loader nulled first; the null branch returns at GuiKit.h:293 before pooling, so it never tests negative caching.
- hudkit_test.cpp:62-67: fakeLoad refuses kit_cancel_hi; line 67 pools a null once but asserts only the return value.
- hudkit_test.cpp:1-71: whole file read; no ensure token anywhere; g_loads is only checked as >=4 (line 68).
