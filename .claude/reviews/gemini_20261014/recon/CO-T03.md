# CO-T03
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2c-03 | PARTIAL | patch_manager_mods/companion_ai_src/CompanionAI.cpp:690 anchor exact; :707-712 END; :787-790 cr set then e1Evaluate | no | Spinning goes stale on death/area change; dangling cr not dereferenced in range |

- CompanionAI.cpp:690 anchor `void e1Evaluate(CrTrack& t, DWORD now) {` matches exactly, so it is not NOT-FOUND.
- CompanionAI.cpp:325 CrTrack.cr is a raw creature pointer stored as int, with no validity or death field.
- CompanionAI.cpp:692 e1Evaluate copies t.cr into cr and passes it to roundActive, crPasses, readTag, rawStance, rd (helper bodies outside range).
- CompanionAI.cpp:696-698 SPIN sets spinning=true and e1spins++ only when !t.spinning; a stale true suppresses the next SPIN line and count.
- CompanionAI.cpp:707-712 END sets spinning=false and logs only when a later e1Evaluate sees no spin or round==1.
- CompanionAI.cpp:790 e1Evaluate is called only from AiTraceAttackEntry and only when now - secStart >= 1000.
- CompanionAI.cpp:787 t->cr = cr is assigned immediately before the 790 call, so cr is fresh for every in-range call.
- CompanionAI.cpp:689-691 comment and saferead::beginScope() say secondary pointers use SafeRead; helper bodies are outside range.
- CompanionAI.cpp:711 END duration is now - spinStart, so a stale spin reports the whole gap until the next trace entry.
- No in-range code clears spinning or cr on death or area change; track() at 786 is outside range.
