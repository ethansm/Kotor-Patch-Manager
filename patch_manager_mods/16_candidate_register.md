# Engine-bug candidate register (doc 16): Phase 0/1 of the doc-15 research

Status: **IN PROGRESS** (started 2026-10-01). Plan: `15_engine_bug_candidates_research_plan.md`; context: `15a_research_session_context_2026-10-01.md`.
Static analysis only. No game launch, no installs, nothing written to the game dir.

## Hand-off summary (top section; updated at the end of Phase 1)
_Pending. Filled in S6._

## Register
Status values: `open` (not yet analysed), `confirmed` (mechanism read in K2 code and reachable here), `killed` (K2 code differs / unreachable / not worth it), `parked` (needs a repro or user test).

| ID | Lead | Source | Status | Evidence | User-visible impact | Hooks needed | Risk | Kill / park reason |
|---|---|---|---|---|---|---|---|---|
| L1 | Dialogue letterbox sized from screen width; negative past 21:9 (user: 3840x1600 = 2.4:1) | KMRP K7 | **confirmed (geometry)**; visible severity needs Phase 3 screenshot | V1 | bar = trunc((H - trunc(W/2.3333330154)) / 2): 3840x1600 -> -22, 3440x1440 -> -17, 5120x1440 -> -377; bars vanish (SPECULATIVE whether a stray strip draws). K2 clamps reply panel/bottom edge, so only the bars are lost | 1 data write (8 bytes at .rdata 0x9A7D30: c = 1.5*W/H -> bar = H/6) or 7 operand re-points | low (no live overlap; semi-transparent-letterbox only alpha) | |
| L2 | `CSWGuiListBox` row-height inflation on repopulate | KMRP K2 | **killed** | V2 | none (cycle broken at every call site; Mod 6 path unaffected) | 0 | | K2 restores saved rects in ClearItems (flag bit 2) / row SetExtent discards height; Mod 6 uses restore=1 |
| L3 | Line-breaker infinite loop on unbreakable text | KMRP K1 | **killed** (no hang) | V3 | none on vanilla; mod-made labels narrower than 2 glyphs lose text silently | 0 | | K2 has the same bad progress compare but a 100-line cap (0x47B0E8) bounds it; no OOM |
| L4 | Glyph advance under-measured ~3% (text under scrollbars) | KMRP K6 | **confirmed (cosmetic)**; needs Phase 3 check | V3 | wrapped lines can be ~0.25 px/char wider than box | 1 data patch (re-point operand at 0x47B234 from 0.25 @0x98C1D8 to 0.5 @0x987F08) | low | |
| L5 | Minimap (KMRP K8 is a zoom mismatch, not pan clamp; L5 as written is a misreading) | KMRP K8 | open (Wave B) | V4 | | | | |
| L6 | Message popup auto-fit capped at a 640x480-authored width | KMRP message-popup.md | open | V4 | | | | |
| L7 | `QueryPerformanceCounter` frame-pacing lead | Peixoto's Patch | open | V5 | | | | |
| L8 | Symbols from the Aspyr Linux/macOS build | KPM | open | decision in 15a s7.1; gate S5 | | | | |
| L9 | 2DA token > 255 chars smashes 256-byte stack buffer (`0x71C3D0`/`0x71C180`) | R5 | open | V6 + `engine_bug_research/scan_2da_limits_output.txt` | | | | |
| L10 | Consumer-side fixed caps in 2DA-backed tables | Gemini #3 | open | V6 + community table | | | | |
| L11 | Stuck idle/combat animation after conversation exit | Gemini #5 | parked | none | | | | needs a repro save |
| L12 | Save reset to Level 1 | Gemini #2 | parked | R4 (no leak, no stat overwrite) | | | | needs a repro save |
| L13 | Soft Shadows slowdown/crash | Gemini #4 | parked | strings only | | | | needs a user toggle test |
| L15 | Last text line dropped when taller than its box (KMRP K5); latent: vanilla labels safe to 3840x1600, 18-unit labels in character_p.gui vanish at 5120x1440 (SPECULATIVE metrics) | V3 | **confirmed (latent)** | V3 | none at 3840x1600 | 2 caves (9-byte @0x47B7DB, 11-byte @0x47B6D4, each re-emits a rel Jcc) | medium | not worth building unless resolution grows |
| L14 | Existing doc-10 perf candidates (F1, C2, TX, ...) | doc 10 | open (continues under doc 10) | doc 10 | | | | gated on perf-lab run A |

## Phase 3 test cards (user-run; nothing is launched by the session)
_Pending (S4)._

## Ranking
_Pending (S6)._
