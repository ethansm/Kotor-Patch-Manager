# Reduce: code

Verdicts: confirmed 1, downgraded 4
Cross-file findings: 1

## Verdicts

| finding | status | sev | conf | reason |
|---|---|---|---|---|
| 81dabb3f4f5e | downgraded | medium | high | The header at MouseProbe.cpp:3 says the sanitizeNorm store is the only engine write. Lines 305-306 write clicklogic::MsgLDbl into the engine's message struct at msgp+4 whenever shouldSynthesizeDblClick is true, and synth_dblclk defaults to 1 at line 140. The header claim is false, but the synthesize |
| ff23d43b210c | downgraded | medium | medium | Line 184 null-checks self and lines 220-221 store to self+0x408/+0x40C with no writability check, so the gap is real. But self is the game's own this pointer read from the hooked frame at ebp-0x28, and the engine already dereferences it in the same function. This is a robustness gap, not a routine o |
| 8ea1731f8816 | downgraded | medium | medium | Confirmed mechanism: srv is re-read per tick (720-721) and the buffer is cleared only when srv is null (722-723). Dispatch and re-inject use the stored creature (745-747, 787-800) with no comparison to the current slot creature. probeRead only checks that 4 bytes are readable, which does not prove t |
| d682bb833922 | confirmed | high | medium | QueueHud.cpp:1004-1013 never rejects a non-party caster: memberK falls back to 0. DISPATCH_NOW (1018-1025) stores that creature as member 0's activeAction and returns 0, and BUFFER (1028-1035) pushes it and returns 1, which consumes the engine cast. The only gates are g_dispatching, enabled and comb |
| f77ae8184a09 | downgraded | medium | low | shouldReInject has no completion signal: when the action is absent from the engine queue and a 0x3D follow-leader entry exists, it returns true (QueueHudLogic.h:579-580), and the caller takes that branch before the 500 ms clear (QueueHud.cpp:744, 778). The design and tests (queue_hud_logic_test.cpp: |

## Cross-file

- [medium/medium] patch_manager_mods/queue_hud_src/QueueHud.cpp, patch_manager_mods/queue_hud_src/QueueHudLogic.h, patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp 744 (logic): QueueHud.cpp:744 passes s.paused into qh::shouldReInject. QueueHudLogic.h:576 discards it with (void)paused, and the re-inject return at 580 ignores pause. queue_hud_logic_test.cpp:1173 asserts shouldReInject(...paused=true) is still true, with the comment 'paused/now are ignored by the header'. The

## Clusters

- mouse-engine-write-scope: The mouse probe is documented as read-only apart from one sanitizeNorm path, but it performs additional engine writes (the message-struct rewrite at 305-306 and the unguarded this-field stores at 220-221), and the header does not disclose the first. (3 findings)
- queue-slot-creature-identity: Buffered queue actions are keyed by party slot index but store a raw creature pointer, and neither the non-party fallback at enqueue (memberK=0) nor the dispatch path (no comparison to the current slot creature) checks that the pointer still belongs to that slot. (2 findings)
- reinject-eaten-heuristic: Re-inject and actionInQueue decisions infer an 'eaten' action from absence in the engine queue plus a follow-leader entry, with no completion signal, and the spell-inclusive predicate (734-739) widened the input without a completion check. (2 findings)

## Notes

Verdicts cover only the five high findings in the bundle, per the rules. Medium and low findings were not re-verified and were not given verdicts. Not verified: the SafeRead.h include at MouseProbe.cpp:27 could not be checked because the _shared directory is absent from the snapshot (only _shared_src is present). The overlap of SiteQueue+0x30..0x39 with SiteRules IDs (QueueHud.cpp:986-997, bundle f6) was not checked against the SiteRules definition. hooks.toml was not read, so the set of PerformCastSpell callers, which bears on d682bb833922, is unknown.
