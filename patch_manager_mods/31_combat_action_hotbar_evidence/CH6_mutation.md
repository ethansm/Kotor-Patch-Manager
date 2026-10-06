# CH6 mutation gate: CombatLogic.h (doc 31)

Command: `python3 scripts/mutate_header.py patch_manager_mods/combat_hotbar_src/CombatLogic.h --jobs 4` (sanitizers on; tests: combat_logic_test.cpp, common_contract_test.cpp). CombatLogic.h itself was not changed.

| run | mutants | killed | survived | broken (does not compile) |
|---|---|---|---|---|
| before (0.8.0-dev tests) | 1039 | 585 | 415 | 39 |
| after (0.9.0-rc tests) | 1039 | 817 | 182 | 40 |

Score is killed / (mutants - broken): before 585/1000, after 817/999.
The 232 additional mutants killed by the new tests came from reference-model sweeps (random inputs vs a spec reimplementation of layoutGroupGrid, layoutBannerGrid, layoutBannerGroups, layoutBannerPills, gridBounds), guard/null/exact-size-buffer tests, orderSig against a reference FNV-1a plus pinned values, nameMatches hex/suffix edge cases, parseNameList truncation/duplicate edge cases, orderGroup stale-data and 16-entry cases, formatProfile size edge cases, and struct-size contracts (runtime checks, so a changed size is a failed test, not a build break).
Broken mutants (40) are compile errors under -Werror (`||` -> `&&` precedence warnings, array-bound errors in the tests) and are not counted as killed or survived.
No survivor revealed a bug: F35 not triggered.

## Survivors: all equivalent

Each line is one surviving mutant (id from `--list`).

equivalent mutant: #5 CombatLogic.h:12 `< -> <=` — clampCount: c=0 -> 0 (`<=0`, `<1`), c=16 -> 16 (`>=16`) and `c > 15 ? 16 : c` is the same function as `c > 16 ? 16 : c`
equivalent mutant: #6 CombatLogic.h:12 `0 -> 1` — clampCount: c=0 -> 0 (`<=0`, `<1`), c=16 -> 16 (`>=16`) and `c > 15 ? 16 : c` is the same function as `c > 16 ? 16 : c`
equivalent mutant: #8 CombatLogic.h:12 `> -> >=` — clampCount: c=0 -> 0 (`<=0`, `<1`), c=16 -> 16 (`>=16`) and `c > 15 ? 16 : c` is the same function as `c > 16 ? 16 : c`
equivalent mutant: #10 CombatLogic.h:12 `16 -> 15` — clampCount: c=0 -> 0 (`<=0`, `<1`), c=16 -> 16 (`>=16`) and `c > 15 ? 16 : c` is the same function as `c > 16 ? 16 : c`
equivalent mutant: #37 CombatLogic.h:32 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #38 CombatLogic.h:32 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #41 CombatLogic.h:33 `<= -> <` — maxSlots == 0 or perGroup == 0 still returns 0: the cap trips on the first entry / the per-group count clamps to 0
equivalent mutant: #44 CombatLogic.h:33 `<= -> <` — maxSlots == 0 or perGroup == 0 still returns 0: the cap trips on the first entry / the per-group count clamps to 0
equivalent mutant: #53 CombatLogic.h:37 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #58 CombatLogic.h:44 `16 -> 17` — 17-byte copy; the extra byte is overwritten by `icon[16] = 0` on the next line
equivalent mutant: #63 CombatLogic.h:56 `<= -> <` — n == 0 writes nothing either way
equivalent mutant: #71 CombatLogic.h:59 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #87 CombatLogic.h:62 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #89 CombatLogic.h:63 `< -> <=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #113 CombatLogic.h:78 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #114 CombatLogic.h:78 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #115 CombatLogic.h:78 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #116 CombatLogic.h:78 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #117 CombatLogic.h:78 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #118 CombatLogic.h:78 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #125 CombatLogic.h:80 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #129 CombatLogic.h:80 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #132 CombatLogic.h:81 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #136 CombatLogic.h:81 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #152 CombatLogic.h:88 `< -> <=` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #153 CombatLogic.h:88 `0 -> 1` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #155 CombatLogic.h:89 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #162 CombatLogic.h:93 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #163 CombatLogic.h:93 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #167 CombatLogic.h:94 `< -> <=` — usedCols = min(n, cols): `n <= cols ? n : cols` is the same function
equivalent mutant: #183 CombatLogic.h:97 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #185 CombatLogic.h:98 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #186 CombatLogic.h:98 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #189 CombatLogic.h:99 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #191 CombatLogic.h:100 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #192 CombatLogic.h:100 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #206 CombatLogic.h:111 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #215 CombatLogic.h:117 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #216 CombatLogic.h:117 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #225 CombatLogic.h:119 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #228 CombatLogic.h:121 `0 -> 1` — initial x0/y0/x1/y1 are overwritten by the first used rect, and never read when no rect is used
equivalent mutant: #229 CombatLogic.h:121 `0 -> 1` — initial x0/y0/x1/y1 are overwritten by the first used rect, and never read when no rect is used
equivalent mutant: #230 CombatLogic.h:121 `0 -> 1` — initial x0/y0/x1/y1 are overwritten by the first used rect, and never read when no rect is used
equivalent mutant: #231 CombatLogic.h:121 `0 -> 1` — initial x0/y0/x1/y1 are overwritten by the first used rect, and never read when no rect is used
equivalent mutant: #236 CombatLogic.h:123 `< -> <=` — negative / zero / over-stride counts: `<= 0`, `< 1` and `>= stride` give the same n
equivalent mutant: #237 CombatLogic.h:123 `0 -> 1` — negative / zero / over-stride counts: `<= 0`, `< 1` and `>= stride` give the same n
equivalent mutant: #239 CombatLogic.h:123 `> -> >=` — negative / zero / over-stride counts: `<= 0`, `< 1` and `>= stride` give the same n
equivalent mutant: #246 CombatLogic.h:127 `< -> <=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #247 CombatLogic.h:128 `< -> <=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #249 CombatLogic.h:129 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #252 CombatLogic.h:130 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #256 CombatLogic.h:141 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #257 CombatLogic.h:141 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #258 CombatLogic.h:141 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #259 CombatLogic.h:141 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #269 CombatLogic.h:143 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #272 CombatLogic.h:144 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #276 CombatLogic.h:144 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #288 CombatLogic.h:150 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #304 CombatLogic.h:154 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #305 CombatLogic.h:154 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #306 CombatLogic.h:154 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #308 CombatLogic.h:154 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #312 CombatLogic.h:155 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #313 CombatLogic.h:156 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #314 CombatLogic.h:156 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #315 CombatLogic.h:156 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #316 CombatLogic.h:156 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #317 CombatLogic.h:156 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #322 CombatLogic.h:158 `< -> <=` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #323 CombatLogic.h:158 `0 -> 1` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #325 CombatLogic.h:159 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #331 CombatLogic.h:165 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #345 CombatLogic.h:174 `== -> !=` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #346 CombatLogic.h:174 `0 -> 1` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #347 CombatLogic.h:174 `&& -> ||` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #348 CombatLogic.h:174 `== -> !=` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #349 CombatLogic.h:174 `0 -> 1` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #350 CombatLogic.h:174 `return; -> {}` — empty-grid early exit: a zero-size bounding box means no rect exists to shift, so every variant is a no-op
equivalent mutant: #354 CombatLogic.h:176 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #358 CombatLogic.h:177 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #359 CombatLogic.h:177 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #361 CombatLogic.h:178 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #365 CombatLogic.h:179 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #366 CombatLogic.h:179 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #372 CombatLogic.h:180 `return; -> {}` — dx = dy = 0 early return: skipping it adds 0 to every rect
equivalent mutant: #381 CombatLogic.h:190 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #382 CombatLogic.h:190 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #383 CombatLogic.h:190 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #384 CombatLogic.h:190 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #385 CombatLogic.h:190 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #386 CombatLogic.h:190 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #400 CombatLogic.h:193 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #404 CombatLogic.h:193 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #417 CombatLogic.h:199 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #436 CombatLogic.h:203 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #437 CombatLogic.h:203 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #438 CombatLogic.h:203 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #440 CombatLogic.h:203 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #444 CombatLogic.h:204 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #445 CombatLogic.h:205 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #446 CombatLogic.h:205 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #447 CombatLogic.h:205 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #448 CombatLogic.h:205 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #449 CombatLogic.h:205 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #456 CombatLogic.h:208 `< -> <=` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #457 CombatLogic.h:208 `0 -> 1` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #459 CombatLogic.h:209 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #464 CombatLogic.h:212 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #481 CombatLogic.h:222 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #497 CombatLogic.h:230 `0 -> 1` — container x positions are non-decreasing in g (colW + gapPx >= 0 for the INI-clamped gapPx 0..64) with equal widths and y: cont[0] is the min and cont[2] the max, so starting the min/max scan elsewhere or reading cont[1].w changes nothing
equivalent mutant: #498 CombatLogic.h:230 `+ -> -` — container x positions are non-decreasing in g (colW + gapPx >= 0 for the INI-clamped gapPx 0..64) with equal widths and y: cont[0] is the min and cont[2] the max, so starting the min/max scan elsewhere or reading cont[1].w changes nothing
equivalent mutant: #499 CombatLogic.h:230 `0 -> 1` — container x positions are non-decreasing in g (colW + gapPx >= 0 for the INI-clamped gapPx 0..64) with equal widths and y: cont[0] is the min and cont[2] the max, so starting the min/max scan elsewhere or reading cont[1].w changes nothing
equivalent mutant: #500 CombatLogic.h:231 `1 -> 2` — see L230: scan start 0/1/2 yields the same min/max
equivalent mutant: #501 CombatLogic.h:231 `1 -> 0` — see L230: scan start 0/1/2 yields the same min/max
equivalent mutant: #505 CombatLogic.h:232 `< -> <=` — see L230
equivalent mutant: #507 CombatLogic.h:233 `> -> >=` — see L230
equivalent mutant: #509 CombatLogic.h:235 `0 -> 1` — all containers share y and h: cont[1].y == cont[0].y
equivalent mutant: #510 CombatLogic.h:235 `0 -> 1` — all containers share y and h: cont[1].y == cont[0].y
equivalent mutant: #514 CombatLogic.h:237 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #517 CombatLogic.h:238 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #518 CombatLogic.h:238 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #519 CombatLogic.h:239 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #522 CombatLogic.h:240 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #523 CombatLogic.h:240 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #529 CombatLogic.h:241 `return; -> {}` — dx = dy = 0 early return: skipping it adds 0 to every rect
equivalent mutant: #538 CombatLogic.h:253 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #539 CombatLogic.h:253 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #540 CombatLogic.h:253 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #541 CombatLogic.h:253 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #542 CombatLogic.h:253 `3 -> 4` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #543 CombatLogic.h:253 `3 -> 2` — array-typed parameter: `T p[3]` decays to a pointer, the extent is ignored by the compiler
equivalent mutant: #550 CombatLogic.h:255 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #553 CombatLogic.h:256 `1 -> 0` — size-0 guard: at size 0 the memset covers 0 bytes / the loops run 0 times, so `<0` and `>=0` behave like `<1` and `>=1`
equivalent mutant: #567 CombatLogic.h:261 `0 -> 1` — maxN start 1: nVis > 0 implies some n >= 1, so maxN >= 1 anyway
equivalent mutant: #572 CombatLogic.h:263 `< -> <=` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #573 CombatLogic.h:263 `0 -> 1` — count 0 maps to 0 either way (`< 0 ? 0 : c` vs `<= 0` / `< 1`)
equivalent mutant: #575 CombatLogic.h:264 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #579 CombatLogic.h:267 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #583 CombatLogic.h:271 `< -> <=` — cols<1 guard: `<=1` / `<2` only rewrite cols == 1 to 1
equivalent mutant: #584 CombatLogic.h:271 `1 -> 2` — cols<1 guard: `<=1` / `<2` only rewrite cols == 1 to 1
equivalent mutant: #588 CombatLogic.h:272 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #589 CombatLogic.h:273 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #601 CombatLogic.h:276 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #602 CombatLogic.h:276 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #603 CombatLogic.h:276 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #605 CombatLogic.h:276 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #609 CombatLogic.h:277 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #610 CombatLogic.h:278 `< -> <=` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #611 CombatLogic.h:278 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #612 CombatLogic.h:278 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #613 CombatLogic.h:278 `1 -> 2` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #614 CombatLogic.h:278 `1 -> 0` — redundant clamp pair: `fit >= 1` and `btn >= 1` each guarantee the final btn >= 1, so weakening/removing either one alone cannot change btn
equivalent mutant: #615 CombatLogic.h:279 `< -> <=` — usedCols = min(cols, maxN): `<=` is the same function
equivalent mutant: #617 CombatLogic.h:280 `1 -> 0` — rows start 0 or 1: a visible group has r >= 1, so the max is the same
equivalent mutant: #626 CombatLogic.h:283 `> -> >=` — `outN[g] > 1` / `r >= rows`: a group with one button has r = 1, which never exceeds the start value 1
equivalent mutant: #627 CombatLogic.h:283 `0 -> 1` — `outN[g] > 1` / `r >= rows`: a group with one button has r = 1, which never exceeds the start value 1
equivalent mutant: #629 CombatLogic.h:283 `> -> >=` — `outN[g] > 1` / `r >= rows`: a group with one button has r = 1, which never exceeds the start value 1
equivalent mutant: #666 CombatLogic.h:298 `> -> >=` — equality boundary: at equality the guarded assignment stores the value the variable already holds
equivalent mutant: #685 CombatLogic.h:308 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #688 CombatLogic.h:309 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #689 CombatLogic.h:309 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #690 CombatLogic.h:310 `> -> >=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #693 CombatLogic.h:311 `< -> <=` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #694 CombatLogic.h:311 `0 -> 1` — shift guard: the compared value only differs at the equality boundary, where the shift is 0 either way
equivalent mutant: #700 CombatLogic.h:312 `return; -> {}` — dx = dy = 0 early return: skipping it adds 0 to every rect
equivalent mutant: #770 CombatLogic.h:365 `1 -> 2` — hexVal's error value is only tested with `< 0`
equivalent mutant: #774 CombatLogic.h:371 `<= -> <` — max == 0: the while loop never runs, returns 0 either way
equivalent mutant: #783 CombatLogic.h:378 `< -> <=` — equality boundary: at equality the guarded assignment stores the value the variable already holds (b == e: *b is the terminator, never blank)
equivalent mutant: #790 CombatLogic.h:381 `> -> >=` — `len >= 47 -> 47` / `len > 46 -> 47` equal for every len
equivalent mutant: #792 CombatLogic.h:381 `1 -> 2` — `len >= 47 -> 47` / `len > 46 -> 47` equal for every len
equivalent mutant: #870 CombatLogic.h:419 `<= -> <` — maxOut == 0: w = 0, nothing written, returns 0 either way
equivalent mutant: #897 CombatLogic.h:430 `16 -> 17` — scratch array one element larger: index 15 is the highest used
equivalent mutant: #899 CombatLogic.h:430 `16 -> 17` — scratch array one element larger: index 15 is the highest used
equivalent mutant: #906 CombatLogic.h:434 `16 -> 17` — scratch array one element larger: index 15 is the highest used
equivalent mutant: #921 CombatLogic.h:441 `1 -> 0` — insertion sort starting one earlier only 'inserts' the first rest element in place
equivalent mutant: #935 CombatLogic.h:447 `< -> <=` — `ns <= maxOut`: w is the same at equality
equivalent mutant: #941 CombatLogic.h:455 `32 -> 31` — `s < 31` iterates the same s values {0,8,16,24} as `s < 32`
equivalent mutant: #964 CombatLogic.h:468 `return; -> {}` — after a failed put() ok stays false and buf[0] is cleared on the failure path, so extra memcpys of shorter pieces are not observable
equivalent mutant: #977 CombatLogic.h:474 `2 -> 3` — d[3] vs d[2]: the digit buffer is only read as a C string
equivalent mutant: #1016 CombatLogic.h:498 `0 -> 1` — gi is assigned by isKey() before it is read
equivalent mutant: #1022 CombatLogic.h:504 `< -> <=` — equality boundary: at equality the guarded assignment stores the value the variable already holds (b == e: *b is the terminator, never blank)
