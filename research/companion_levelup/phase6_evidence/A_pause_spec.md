# Spec: per-companion "Pause at level N" (0.3.1, user request 2026-10-03)

User: "can we let the user enter a value for the level to pause at?" - e.g. bank ALL of Atton's levels from Scoundrel 3 until the story makes
him a Jedi (Nar Shaddaa), instead of the preset's Scoundrel-to-7.

## Semantics
- `pause` = the LAST character level the companion takes before banking. 0 = preset default (= the preset's own rule: base-class records up to
  convert_at-1, then the existing hold rule v2). Only meaningful for presets with `holdClass != 0 && convertAt > 0` and only when hold is on.
- New rule in `levelNow` (C++ `LevelUpOverride.cpp:547` and twin `simulate_rewrite.py level_now`), evaluated FIRST, before record matching:
  `if (hold && pauseAt > 0 && p->holdClass && creature lacks holdClass && key > pauseAt) return HOLD;` (key = total char level + 1).
  Everything else unchanged. Once the creature has the hold class, the existing record matching applies (REBASE onto the Jedi records; proven
  for early conversion by `phase6_evidence/B2_rebase.md`).
- Normalisation: a pause >= convertAt-1 equals the default -> store 0. A pause < 1 -> 0.
- Valid UI range per preset: `minPause .. convertAt-2`, where minPause = max(1, (char level of the preset's first present record) - 1).

## Config plumbing
- `TagCfg` gets `int pause` (default 0). Dev key `pause.<tag>=N` (ini, like `instant.`/`hold.`). Log strings that print `/i%d/h%d` add `/p%d`.
- `Playthrough.h`: line format `<tag>=<preset>|instant=0|1|hold=0|1|pause=N`; missing pause = 0; serialise `|pause=N` ONLY when N > 0 (old
  files and lines round-trip byte-identical); unknown/garbage pause value -> 0 (not a lossy-parse failure... follow the file's existing policy for
  bad numeric fields and say which you chose). `setTag(..., int pause)`.
- `ptApply(tag, preset, instant, hold, pause)`; the GUID-section -> TagCfg copy (around `LevelUpOverride.cpp:1025`) copies pause.
- Every `levelNow` call site passes `cfg->pause` (LevelUp ~1257, evalStats ~1351, any other); gate/instant/leader/GUI-guard/banked logic all
  flow from evalStats so they follow automatically - verify.

## UI (`LevelUpUi.h`)
- No new control. `C_BANK` becomes a cycler when the preset has a hold class. Page state `r->bank` (bool) + new `r->pause` (int, 0 = default).
  Text: bank off -> "Bank for Jedi: Off"; bank on, pause 0 -> "Pause at level %d" with convertAt-1; bank on, pause N -> "Pause at level N".
  Click order: default (convertAt-1) -> convertAt-2 -> ... -> minPause -> Off -> default. Keep the text <= 18 chars (control width 113).
- Page open: start from the configured cfg (bank = cfg->hold, pause = cfg->pause) else bank on + pause 0. Cycling a ROW to another preset:
  clamp pause into the new preset's range (out of range or no hold class -> 0). "Use recommended" keeps toggles (as today) with the same clamp.
- Apply passes the pause (0 when bank off or no hold class). Log: `UI apply tag=.. preset=.. instant=.. hold=.. pause=.. ok=..`.
- Status line unchanged (it already says "Holding N levels for <class>").

## Version
`0.3.1`: manifest version + the load banner. Hooks and exports MUST NOT change (dll-only swap).

## Tests (all must pass; add the new ones)
- simulate_rewrite.py: level_now pause cases (pause blocks base-class PATH; no effect with hold off / no hold class / creature has the class;
  key == pauseAt -> not held, key == pauseAt+1 -> held).
- parity.py levelNow suite: pass pause through `parity_main.cpp`; add a pause case set (every hold preset x pause in range x class states) ->
  C++ == Python, 0 mismatches. Existing case counts unchanged.
- playthrough_test: parse/serialise pause, missing pause, round-trip of pause-less lines byte-identical, setTag with pause.
- ui_test: Bank cycler order/text per preset (Atton early: 7,6,...,min,Off), clamp on row cycle / recommended, apply payload.
- gate_test: a held-by-pause creature is refused at the hold gates and gets no instant.
