# HB-T08

| fid | verdict | evidence (path:line) | fix_sound | note |
|---|---|---|---|---|
| HB-1d-03 | CONFIRMED | patch_manager_mods/action_hotbar_src/keybif_test.cpp:53-63; patch_manager_mods/action_hotbar_src/KeyBif.h:123,150,152,154,163,176 | yes | Five named branches have no ExpectFail case; no test needle matches their errors. |

- keybif_test.cpp:53-63 holds the ten ExpectFail calls. Their needles: cannot open key, bad key magic, out of range, bif index 3 out of range, resref not found, bad arguments, resource too large, resource out of range, bad bif magic, cannot open bif.
- keybif_test.cpp:63 is `ExpectFail("missing bif", bad + "/bad_res.key", bad, "ghost", "cannot open bif");`. It matches the anchor exactly once the CSV-doubled quotes are removed. The NOT-FOUND mark is an escaping artifact, not an invented finding.
- KeyBif.h:123 "key too short" (24-byte header read fails). No test case; the string "too short" appears nowhere in keybif_test.cpp.
- KeyBif.h:150 "bad bif name size" (nameSize 0 or over 260) and KeyBif.h:154 "empty bif name" (all-NUL name). No test case for either.
- KeyBif.h:152 "bif name out of range" (name bytes past key EOF). No test case.
- KeyBif.h:163 "bif too short" (BIF under 20 bytes). No test case.
- KeyBif.h:176 "resource id not in bif" (no var entry with matching low 20 bits). No test case; the needle is absent from the test file.
- Extra uncovered branches, not in the claim: KeyBif.h:147, 166, 167, 173, 181 ("key file table truncated", "resource index N out of range", "bif var table out of range", "bif var table truncated", "resource read failed").
- keybif_test.cpp:55 uses the loose needle "out of range", which several branches emit (KeyBif.h:126,127). It does not pin which branch fires.
- Fix direction: add ExpectFail rows with crafted KEY/BIF bytes, or temp files written at runtime. Test-only change, so the fix is safe. Use full needles, not "out of range".
