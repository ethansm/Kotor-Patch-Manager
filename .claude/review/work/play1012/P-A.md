# P-A: companion-selfheal Down audit (session 2026-10-12 00:16:23)

Source: `companion-selfheal_log.txt` read only via `scripts/kplog_parse.py` (297 parsed lines). T= is ms since process start.
Source anchors: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp` (`stName`, `stateLog`, `refreshCache`, `uiFrame`, `onFrameBegin` P3 poll, `keyFor`); `patch_manager_mods/_shared_src/SelfHealUi.h` (`resolveState`, `clickAllowed`); `patch_manager_mods/_shared_src/Party.h` (`members`, `vitals`).

Key rules from source:
- `resolveState` order: Hidden > Down > FullHp > NoItem > Pending > AutoOn > Normal.
- `Vitals.down = ok && cur <= 0`. Down is derived each refresh from cur, not from an event.
- `stateLog` is edge-triggered per member index (`last[mi]`), so max-only changes inside one state are not logged.
- Member index = party-table position (`party::members`, `PartyGetAt`). Member 0 cfg key is always `slot0`. Members 1-2 cfg key = creature tag at that position (`keyFor`).
- P3 (HUD portrait slot map) is polled every 2000 ms and logged on change, so P3 timestamps lag the real change by up to 2 s.

## D1. Every STATE line involving Down, plus the next 2 transitions of that member

| # | T= | Transition | hp/max | Next 2 transitions of same member |
|---|----|-----------|--------|-----------------------------------|
| E1 | 686287.149 | m1 AutoOn->Down | -11/21 | 710450.746 Down->Pending (3/21); 711241.253 Pending->AutoOn (3/21) |
| E2 | 706355.621 | m2 AutoOn->Down | -11/72 | 1030395.681 Down->NoItem (1/72); no further m2 STATE line |
| E3 | 712488.577 | m1 Hidden->Down | -11/72 | 1030395.676 Down->FullHp (23/21); no further m1 STATE line |

Down intervals (STATE-to-STATE): E1 24.2 s; E2 324.0 s; E3 317.9 s.
Immediately before E3: 712390.497 m1 AutoOn->Hidden hp=13/21 (98 ms before E3, max changed 21 to 72).

## D2. Wake behaviour

- **m1 (E1) wake path: legitimate wake-then-queued-use.**
  - 710450.651 `call AUTO member=1 hp=3/21 pick="Medpac (8)"`
  - 710450.683 `call USE member=1 src=auto heal=122 hp=3/21` (heal field is an estimate; the DONE reports +10)
  - 710450.685 QDIAG routeA queue=0; 710450.694 CALL routeA returned
  - 710450.746 `STATE Down->Pending`
  - 711241.239 `DONE member=1 hp+10 stack-1 queued=0`; 711241.253 `Pending->AutoOn`
  - No REFUSE/CLICK for m1 in the 707.4-713.4 s window.
  - Down->Pending is correct: after wake (cur>0), resolveState gives Pending because the use was armed in useNow.
  - Returned to AutoOn, which is the pre-Down state.
- **m2 (E2) did not return to AutoOn.** It stayed Down for 324 s and exited at 1030395.681 to NoItem (hp 1/72). NoItem is the correct resolveState output when there is no pick (resolveState checks NoItem before AutoOn). The log has no Medpac count after 704081 (Medpac (8)), so the reason for "no pick" (stack empty vs canUseStrict refusal at 1 HP) is not provable. NoItem also appears for m0 at 962446, which suggests the Medpac stack was empty by then.
- **cfg untouched:** the last CFG line for kreia is 661241.046 (`auto=1 hp=35 keep=2`). No CFG lines after that.
- **Pre-Down auto-heal stall (m2):** at 13/72, AUTO fired at 685516.597, 691608.416, 697847.854, 704081.317. Each one was refused with `REFUSE already queued` (QDIAG queue=1 types=[3f], rounds 2 to 3, paused 1 to 0). No DONE/USE for m2 followed. m2 went from 13 (704081) to -11 (706355) without the queued heal landing.
- **Exit from E3 is not a wake of the Down creature.** At 1030395.676 m1 Down->FullHp hp=23/21 comes from the 21-max creature being back at position 1 (see D4). The 72-max Down creature exits at the same instant as m2 Down->NoItem hp=1/72.

## D3. USE or CLICK while a member is Down

- CLICK lines in the whole session: 0.
- USE lines in the whole session: 1 (710450.683, m1, src=auto), taken with cur=3>0 after the Down->Pending transition was armed. Not inside any Down interval.
- Intervals checked: E1 686.3-710.5 s (m1): no USE/CLICK. E2 706.4-1030.4 s (m2): no USE/CLICK. E3 712.5-1030.4 s (m1): no USE/CLICK. Result: no mod-logged use or click while Down.
- Unlogged consumption (open): Medpac stack was 9 at 697847.854 (m2 AUTO) and 8 at 704081.317 (m2 AUTO), with no USE line between them. The queue went queue=1 (QDIAG at 682943, 685516, 691608, 697847, 704081) to queue=0 by 710450.685. m1 went from cur=-11 (686287) to cur=3 (710450.651) with no mod use in that window. The most likely explanation is an engine-side queued Medpac applied to Down m1 (the queue item was queued before the mod's first REFUSE at 682943). This is not provable from the mod log because the engine does not log its queue execution.

## D4. Max-HP inconsistency: mapping or read?

Verdict: party-position remap, not a read fault. Anchors:

1. The max value that follows a member index changes only at remap events. Each jump matches a P3 remap within its 2 s poll window:
   - 118522-118716: m1 21->81 and m2 81->124 at 118716.655/.666. P3 remap logged at 120158.014 (k-order rotated).
   - 183272-183386: m2 124->Hidden, m1 Normal->FullHp 21/21 at 183386.975, m2 Hidden->Normal 69/81 at 183386.984. P3 rotated back at 184492.041.
   - 712390-712488: m1 21->72 (98 ms gap). P3 remap logged at 714152.460, k-order [0bf44d08, 087ad2f8, 0c6c0020] to [087ad2f8, 0c6c0020, 0bf44d08].
   - 1030395.579 P3 restores the earlier k-order. 1030395.676-.683: m1 Down->FullHp 23/21 (21-max creature), m2 Down->NoItem 1/72 (72-max creature), m0 FullHp->NoItem 2/124.
2. The member 1 max of 21 to 72 happens in one creature replacement (98 ms). A single creature cannot change max HP that way. The 72-max value equals the m2 Down creature (-11/72, 706355), so the new member 1 is that creature, not a read error.
3. cur and max come from one `vitals()` call on one server pointer, so each pair is internally consistent. The one anomaly is 1030395.676 m1 hp=23/21 (cur > max). That is most likely an expired max-HP buff; it is not a cross-creature mix.
4. Earlier evidence that the index is not a stable identity: the CFG key for popover member 2 was `tag=player` at 130923 (PC tag at position 2). The same key came from the member 1 popover at 136324/136650. Later, member 2 is `tag=kreia` at 661241.

Caveat: no client/server pointers are logged for members (`dumpMember` only runs on the probe key; MAP logs once at 43857.698-.699). So creature-by-creature identity is inferred from timing and max/tag values, not proven.

## D5. Errors, warnings, refusals, leader button

- `append failed`: 0.
- ERR level lines: 0.
- WARN lines: 5, all `REFUSE already queued (auto)`:
  - m1 x1: 682943.279
  - m2 x4: 685516.644, 691608.454, 697847.892, 704081.355
- CLICK lines: 0.
- LEADERBTN: 1 line, 43857.723, `rect=[3445,1403,39,39] ring=[3436,1394,58,58]`. Matches the expected values. No rect change in the session.

## Open questions

1. Was an unlogged engine-side Medpac (stack 9->8 between 697847.854 and 704081.317) applied to Down m1, which moved m1 from -11 to 3 HP before 710450?
2. During 712488-1030395 the m2 state stayed Down with no STATE line through the 714152 P3 remap. Either m2 and m1 held the same 72-max creature, or the index mapping is not simply rotated. Pointer dump needed to settle it.
3. Heal estimate at 710450.683 is `heal=122` but the DONE reports `hp+10`. Estimate scale vs. actual heal is unexplained (not a Down-lifecycle fault).

## Contract check note

`agent_contract.py check --type recon --scope .claude/review/work/play1012/P-A.md` reports FAIL OUT_OF_SCOPE for 7 files (handoffs 33/37, master_ledger.jsonl, SelfHealUi.h, selfheal_ui_test.cpp, gen_selfheal_textures.py, selfheal_log_check.py). No snapshot was taken before launch, so those are other sessions' uncommitted edits in the shared worktree. This agent wrote only P-A.md.
