# PL-T04
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4c-01 | CONFIRMED | patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp:71-73 | no | No fix stated; unconditional real check would fail off-foreground since post is forwarded. |

- smoke_activationfix.cpp:71 sets fgOurs = GetForegroundWindow()==w; callSite() at :71 runs regardless of fgOurs.
- smoke_activationfix.cpp:72-73 is the only conditional: if fgOurs, check() on g_posts/d3; else printf "SKIP real foreground check".
- smoke_activationfix.cpp:73 SKIP branch calls no check(), so it cannot fail and is not tallied as a check.
- smoke_activationfix.cpp:67-70 setFg(1)/setFg(0) checks run unconditionally via check(); they use the synthetic setFg flag, not real focus.
- smoke_activationfix.cpp:62, 65-66 state, site-rewrite and byte checks run before the foreground section; unaffected by fgOurs.
- smoke_activationfix.cpp:76 IGNORED count expects (fgOurs ? 2 : 1), so the off-foreground run still asserts one IGNORED line.
- smoke_activationfix.cpp:74-77 log assertions run in both cases (patched line, PASSED line, engine flags).
- smoke_activationfix.cpp:76 is the only assertion tied to the real-foreground branch in the SKIP case, via log count, not g_posts/d3.
- Whether SKIP affects the exit status depends on check() and main's tail, which are outside 55-85 and were not read.
- Anchor CreateWindowA (token cited at :49) is outside 55-85 and was not verified by this read.
