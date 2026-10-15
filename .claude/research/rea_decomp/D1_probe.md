# D1 live probe: persistent REA MCP session (rea_session.py), 2026-10-15

Exe: `backups/vanilla_clean/swkotor2.exe`, sha256 `6a522e71...bbffef`, unchanged before and after both runs;
no files appeared next to the exe. Guard passed (one other session's REA was live; ours made 2). Driver scripts and
raw JSON live in the session scratchpad (`d1probe/`, `d1probe2/`), not in git.

## Timings
| step | wall |
|---|---|
| spawn + initialize + tools/list (139 tools) | 2.0 s |
| `open_binary` | 0.1 s (lazy: no analysis yet) |
| FIRST analysis call (`procedure_info` / `procedure_pseudo_code`) | 314.7 s and 315.9 s (two sessions) |
| every later call (14 procedure_* calls) | median 0.01 s, max 0.03 s |
| `batch_decompile` 20 addresses | 0.76 s, 32.7 KB, 20/20 ok |
| `analyze_function` 0x7680a0 | 0.26 s, 399 KB |
| `xrefs` global 0x00a1b4a4 / `procedure_references` 0x7680a0 | 0.05 s / 0.10 s |
| `annotate_native_function` | 0.13 s (returns a 135 KB dossier) |

- Memory: JVM RSS ~0.98 GB (ps rss); node MCP process VmHWM ~97 MB (what `runs.tsv` records; it is NOT the JVM).
- The cold cost is per session, not per address. One session amortises it over any number of calls.
- Fix landed: `ReaSession.open()` now runs a warm call (`WARM_CALL`, procedure_info 0x776db0) under
  `open_timeout_s`, so callers' `call_timeout_s=120` never hits the 315 s import.

## Tool behaviour on this target
- Every result is an evidence envelope `{operation, parameters, subject, normalized_result, confidence, ...}`.
  Tools must read `normalized_result`: procedure_info dict (`entrypoint`, `length`, `body.ranges`, ...);
  procedure_assembly str of `0xADDR: INSN` lines; callers/callees list of `0x..`; pseudo_code str;
  batch_decompile `{items[{address, status, pseudocode|error}], total, succeeded, failed}`.
- `analyze_function` gives `procedure.body.ranges`: 0x7680a0 = 3 ranges, total 516, span 520, `non_contiguous: true`.
  `procedure_info` alone does NOT give ranges.
- `resolve_native_call_targets` on vtable dispatch (0x7680e1, 0x768107: `CALL EAX` after `MOV EAX,[EDX+0x4c]`;
  0x922614 `CALL [EAX+0x9aae40]`) returns `status: unresolved, mechanism: computed, targets: []` with the
  limitation "vtable ... mechanisms are not identified". Our static `atlas_common.resolve_indirect` already gives
  slot 0x4c for both 0x7680xx sites; `slot_candidates(0x4c)` lists 5 class_methods rows (any class). So the static
  path is strictly more informative; REA adds nothing for vtable calls.
- `set_addresses_names` and `set_address_name`: declared, but the Ghidra provider rejects them
  (`capability_unavailable ... operation is not declared by this provider`).
- `annotate_native_function {procedure, name, comment}` works: after naming 0x85d320 "ProbeName",
  `procedure_pseudo_code` and `batch_decompile` of 0x775970 print `ProbeName();`. Names live only in the session.

## Gate decisions (Opus, recorded in DECOMP_PLAN.md Decisions)
1. Per-call median 0.01 s (<= 15 s): D3/D4 GO on REA, one session per run.
2. Annotation is visible in pseudo-C via `annotate_native_function` (not `set_addresses_names`): D4 step 2 uses
   per-address `annotate_native_function`; the textual post-pass (3b) stays only as the fallback for addresses the
   annotate call rejects.
3. D3 step 1 changes: indirect targets come from the static slot (`static_facts.indirect_slots`) plus
   `class_methods` candidates, $0; no `resolve_native_call_targets` calls for vtable sites.
   D3 step 2 uses `analyze_function` body ranges. D3 step 3 uses `xrefs` on the function's own address to find
   DATA references (vtable/table slots holding the function), which the static dump does not have.
