# hk1 T1: detour windows for the cmdq-probe (log only)

Scope: 11 windows from the brief. Target: steam swkotor2.exe (sha 6A522E71).

## Checks run
- (a) bytes match, (b) boundary, (c) no relative branch in span: `python3 scripts/disasm_va.py steam <va> --len 16` (plus `--len 64/96` for read offsets).
- `python3 scripts/check_hook_spans.py --quiet`: `FAIL=0 WARN=0 INFO=6 PASS=1`, exit 0.
- Overlap: reused the checker's own loaders (`sc.load_mods`, `chs._source_spans`, `chs._hook_span`, `chs._cp_span`, `sc.load_live_config`). Source scope = 27 mods, 232 spans (hooks + code patches, no target filter). Installed scope = 39 live ids, 271 spans. Result: 0 overlaps and 0 spans within 8 bytes, for every window, in both scopes. Positive control `[0x775600,0x775610)` reports `queue-hud` 0x775603-0x775609, so the detector works. Script: `scratchpad/hk1/t1/overlap_checker.py`.
- `rg -n -i <addr>` over the hooks TOMLs and `engine_ai_atlas/hook_catalogue.md`: hits only at 0x58E730 (atlas row AI2-SR, status `batch-only`, not installed) and 0x541080 (companion_ai TOML line 153, a comment on a CALL site, not a hook).

## Window table

| addr | bytes match | boundary OK | rel-branch in span | overlap with live hook | verdict | confirmed read offsets |
|---|---|---|---|---|---|---|
| 0x538ED0 SetPauseState (len 9) | Y | Y (next insn 0x538ED9) | N | none | OK | Doc 29: mask `[esp+4]`, value `[esp+8]` at entry. Disasm 0x538EEE/0x538F08 reads `[ebp+8]` and `[ebp+0xC]`, which is entry `[esp+4]`/`[esp+8]`. |
| 0x565310 AddCastSpellActions (len 9) | Y | Y (0x565319) | N | none | OK | Doc 29: ECX actor, spell `[esp+4]`, target p10, real flag `[esp+0x48]`. Disasm 0x56531F `cmp [ebp+0x4c],0` = entry `[esp+0x48]` (RET 0x48). |
| 0x5648D0 AddItemCastSpellActions (len 9) | Y | Y (0x5648D9) | N | none | OK | Doc 29: ECX actor, item `[esp+4]`, target `[esp+0x1C]`, real flag `[esp+0x2C]`. Disasm 0x5648F4 `cmp [ebp+0x30],0` = entry `[esp+0x2C]` (RET 0x2C). |
| 0x5BB07C cast fire (len 6) | Y | Y (0x5BB082) | N | none | OK | Actor = `[ebp-0x2B8]` (doc `[EBP+0xFFFFFD48]`). Read at 0x5BB0A2 `mov ecx,[ebp-0x2b8]` then `call 0x5482a0`; also 0x5BB0BB `mov eax,[ebp-0x2b8]`. No write to it between 0x5BB07C and 0x5BB0A2. The stolen insn reads `[ebp-0x84]` (the flag tested at 0x5BB0AD), not the actor. |
| 0x5B9414 item fire (len 6) | Y | Y (0x5B941A) | N | none | OK | Actor pointer = `[ebp-0x260]` (doc `[EBP+0xFFFFFDA0]`). The stolen insn is itself `mov eax,[ebp-0x260]`. Next insn 0x5B941A `mov ecx,[eax+0x10dc]`. |
| 0x58E730 StartCombatRound (len 6) | Y | Y (0x58E736) | N | none (atlas AI2-SR is batch-only) | OK | Doc 29: ECX round, owner `[round+0xB04]`. Not re-verified in disasm. |
| 0x58EA53 EndCombatRound AB8 reset (len 10) | Y | Y (0x58EA5D) | N | none | OK | Doc 29: ECX round, A94/A9C. The stolen insn `mov [ecx+0xab8],0` uses ECX as base, so ECX must survive the probe. |
| 0x541080 ClearAllActions (len 6) | Y | Y (0x541086) | N | none (companion_ai line 153 is a comment on a CALL site) | OK | Doc 29: ECX victim, caller `[esp]`. Not re-verified in disasm. |
| 0x77C780 PartyMenu item handler (len 9) | Y | Y (0x77C789) | N | none | OK | Not in doc 29. No layout stated. |
| 0x5D32AB CanUseItem exit (len 7) | Y | Y (0x5D32B2 = `ret 0xc`) | N (ret is outside span) | none | OK | Not in doc 29. Span is an epilogue: `mov eax,[ebp-4]; pop esi; mov esp,ebp; pop ebp`. The probe must leave ESP/EBP intact. |
| 0x752050 portrait callback (len 6, optional) | Y | Y (0x752056) | N | none | OK | Not in doc 29. No layout stated. |

## Notes
- Doc 29 `cmdq-probe` table (lines 53-63) has 11 rows, but not this set. It includes 0x563C90, 0x6D9500 and 0x6DA49F, which are not in the brief. It omits 0x77C780, 0x5D32AB and 0x752050.
- Doc 29 offsets are entry-relative. The probe runs before the stolen bytes, so `[esp+N]` at entry is valid.
- Every window's code after the span reads ECX (for example `mov [ebp-N],ecx`). The probe must preserve ECX. Only row 7 reads ECX inside its span.
- Not checked: inbound jump targets into the span interiors. Doc 29 says PQ1/PQ2 checked this for its own rows only.

## Result
All 11 windows pass (a), (b), (c) and the overlap check. No FIX rows.

## Main-agent amendment (inbound-target check, rows 9-11)
- **0x5D32AB is NOT a clean window**: `JMP 0x005d32ae` at 0x5D2F99 (null-item early return, `5d2f95 JNZ`) lands INSIDE the 7-byte span (0x5D32AB..B1). A detour there would crash on a null item.
- Fix: use **0x5D32AE**, len 7, stolen `5E 8B E5 5D C2 0C 00` (POP ESI; MOV ESP,EBP; POP EBP; RET 0xC). Inbound targets: only 0x5D32AB (fall-through) and 0x5D2F99 JMP, both land on the window start. EAX = the real return value on every path (0x5D32AB loads it from [ebp-4]; the null path XORs it), so read `eax`, NOT `[ebp-4]` (null path leaves [ebp-4]=1). Item = `[ebp+8]`.
- 0x77C780 and 0x752050: only the prologue lines hit; no interior targets. OK.

## Main-agent amendment 2 (T2 gate): extra window
- 0x5DF920 AIActionType47_ReequipResumeAttack entry, len 6, `55 8B EC 83 EC 6C` (boundary 0x5DF926, no rel branch, no live-hook hit). Cold (per 0x47 node). Reads: ecx actor. Channel `clr`, line `REEQ actor=`.
- T2 correction: AddAttackActions never re-adds 0x3F; it re-adds 0x0C. 0x3F re-adds live in AddCutsceneAttackActions 0x5683D0 and siblings. Warm gate sites 0x6D9931/0x6D996B (AIActionCombat clear+front attack) go to batch 2.
