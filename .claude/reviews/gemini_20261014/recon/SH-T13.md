# SH-T13
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3b-04 | PARTIAL | patch_manager_mods/_shared_src/effects_test.cpp:353,362-364,369; patch_manager_mods/_shared_src/EffectIcons.h:33,35 | yes | Half-true: iconFx also gets rows 2, 0, -3, 5000, 120; no ii_/iw_ or nullptr harmful row tested via iconFx. |
- effects_test.cpp:350-352 defines iconFx (type 0x43, ints[0]=row); all its calls sit in lines 353-369, so the range is complete.
- effects_test.cpp:353, 360, 369 pass row 28 (ip_slow) and row 5 (ip_droiddestroy); both harmful (EffectIconRowTable.inc:33, :10).
- effects_test.cpp:362 passes row 2 (ip_speedburst, beneficial, EffectIconRowTable.inc:7), so "only 28 and 5" is false.
- effects_test.cpp:363-364 pass row 120 (out of range), 5000, 0, -3, and nInts=0; all expect no entry.
- EffectIcons.h:33 rejects row<=0, row>=120, good!=0, and null resref; row 0 is caught by the row<=0 test, so the null-resref check is not exercised.
- Harmful null-resref rows exist: EffectIconRowTable.inc:61 (row 56), :110-115 (rows 105-110); no test passes them to iconFx.
- EffectIcons.h:35 ii_/iw_ item branch has no test row; candidates are rows 41-44, 55, 63 (EffectIconRowTable.inc:46-49, :60, :68).
- effects_test.cpp:345 asserts item==true for ii_drdutldev_003, but via the kDebuffIcons spell path (EffectIcons.h:20), not iconFx.
- Implied fix is sound: iconFx(55) expecting item=true and iconFx(56) expecting no entry match EffectIcons.h:33-36.
