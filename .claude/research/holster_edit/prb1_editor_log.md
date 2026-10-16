# PRB1 holster-editor play log (10-16)

Source: `holster-editor_log.txt` (154 lines, 1 launch, 2 Options open/close cycles). Read via `kotor2_logs.py check` and `kplog_parse.py --bare/--stats` on the frozen copy. Not `cat`/`grep`.
Load line: `holster-editor 0.1.1-probe loaded ... enabled=1 debug=0 button_clone=1 reflow=1 desc=1 free_labels=1 free_view=0 table=ok`

## Signal table

| # | Signal | Result | Count / values | Excerpt (<=120 chars) |
|---|---|---|---|---|
| 1 | BTNCLONE btn + back | PASS | btn ok x2, back ok x2, canary=1 on all, fallback 0 | `BTNCLONE btn_holster ok canary=1` |
| 2 | HOVER hl | PASS | ours=1 x7, all hl=1; ours=0 hl=1 x2 (m08=0); hl=0 x23 | `HOVER ours=1 m08=90d6350 hl=1 btn=90d6350 isButton=1` |
| 3 | DESC via=8A2320 | PASS | 7 lines, all ok, pushed=1, strref=-1; 0 failures | `DESC ok via=8A2320 strref=-1 holder=90c5f40 pushed=1` |
| 4 | RELAYOUT n= | PASS | 0 lines; max n = none; no "gave up" | (source logs only on re-apply, see notes) |
| 5 | REFLOW | PASS (on) | `REFLOW on ... moved=5` x2 (one per session) | `REFLOW on new=(772,645,1132,85) moved=5` |
| 6a | Title / status | PASS | LABEL title + status ok x2; RECT visible=0 in main view (expected) | `LABEL title from LBL_TITLE -> 9156418 why=0 badvt=0` |
| 6b | Focus clear | PASS | focusCleared=1 on 3/3 enters | `EDITOR enter panel=90c4e38 ... focusCleared=1` |
| 6c | Click edges | PASS | enter on btn 2/2 (release overBtn=1); back 3/3 (release overBack=1) | `INPUTCLASS release pressed=2 overBtn=0 overBack=1 editor=1` |
| 7a | FREE / OptDestroy | PASS (FREE); OptDestroy ABSENT | `FREE truncated list 21 -> 16`; btn/back/title/status call=1 per session | `FREE truncated list 21 -> 16` |
| 7b | FREEVIEW | ABSENT (expected) | free_view=0 | none |
| 7c | Clean end | PASS | DTOR editor=0 x2, appended=1, free_labels=1 | `DTOR panel=b99ef20 token=2 editor=0 appended=1 free_labels=1 free_view=0` |
| 8 | WARN / ERR | PASS | 0 (`--stats`: 142 lines, all INFO) | none |

Core signals 1-4: all PASS.

## Notes

- Session 1 ran 2 editor enters (ctl 90d6350), session 2 ran 1 enter (ctl bc9ac50). Same outcome in both.
- RELAYOUT is emitted only on non-initial drift re-applies (`HolsterEditor.cpp` applyTargets). Zero lines means no drift after the first tick, which is the pass condition.
- OptDestroy has no log line in source. The FREE lines are the r7 recipe output.
- Observation, not a signal: parsed lines 55 and 58 show `hl=1` while `m08=0` (pointer over no control). `hl` clears only when another control is hovered (line 61). Unverified whether vanilla buttons behave the same. Check during the PRB1 play pass.
- Session 2 `CTOR flags48=3f0fec4f` differs from session 1 `0x47`. Both pass the `flags48 & 2` gate, so not a failure.
- Log has no area-change marker. The clean-end evidence is DTOR with editor=0 followed by FREE lines.
- `kotor2_logs.py check --new` reported 154 new lines (no prior cursor). Nothing else was written.
