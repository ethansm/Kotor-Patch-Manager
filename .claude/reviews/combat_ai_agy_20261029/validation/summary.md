# Combat-AI Agy review: validation summary (2026-10-29)

Source: review.md (14 findings) checked by 6 Haiku skeptics (u1..u6) and consolidated here. Detail: validated.tsv (18 rows).
No Haiku verdict was overturned; #7 was moved REFUTED -> PARTIAL/latent (Rage 271-273 gap is real, review IDs wrong).

## Counts (verdict x real)
| verdict    | yes | latent | known | design | no | total |
|------------|-----|--------|-------|--------|----|-------|
| CONFIRMED  | 3   | 1      | 1     | 0      | 0  | 5     |
| PARTIAL    | 6   | 4      | 0     | 0      | 0  | 10    |
| UNSURE     | 0   | 1      | 0     | 0      | 0  | 1     |
| DOCUMENTED | 0   | 0      | 2     | 0      | 0  | 2     |

Recurring review defect: wrong spell IDs (Enlightenment 180 not 176; Rage 271-273 not 173-175; Horror 30 / Insanity 31 / Sleep 44, Hold 29, Stun 46; Droid Stun 47, Redirection 163, Suppress 48, Crush 177, Breach 19). Gemini family numbers (22/23) were read as spell IDs.

## Real findings (real=yes), ranked
1. medium #1 RangedAI E-H1/E-H2 use !isPC (+0x10E8, stays on Exile while a companion is controlled). Fix: use the controlled-id test CompanionAI.cpp:356 already uses; share the helper.
2. medium #9 Sticky oLastTarget before oIntruder, defense branch not cleared, no GetIsDebilitated at k_inc_generic.nss:754. Fix: oTarget=oClose in defense and skip debilitated now; intruder-first order needs owner call (June 3 vs May 15 comment).
3. medium #11 GEN_TALENT_BUFF rolls one tree per call. Fix: collect trees with a known, not-family-active talent, then pick; gate Crush (tree 6, PC-only, 15 m enemy AoE) on an enemy lacking the effect.
4. medium #4 F4 frees older valid spell nodes, G1 then drops the newest (family active) -> empty round. Fix: skip the older-node drop when the fresh node's family is active; also set g_f4Seen[k].act=0 before FnOpDelete (CompanionAI.cpp:1076) and make f4AddedAt return newest match. Review's "reused pointer = sameRun" is inverted (it reads stale).
5. low #6 O3 reject falls to GN_RunDefaultAIRoutine (grouping helpers ignore effects). Fix: needs owner OK (changes O3 option B); passing oIntruder to default is vanilla and separable.
6. low #2 Force Enlightenment (180) maps to no buff family. Fix: special-case 180 in castFamily/castGuardDrops to satisfy families 1, 2, 3.
7. low #3 O3 matches exact spell id so Horror/Insanity/Sleep overlap still wastes a round. Fix: count enemies free of every spell in that AoE series; do NOT add 30/31/44 to families 22/23.
8. low #13 Drain Force 153-155 missing from the droid list. Fix: add 153,154,155 only (not 19, 48, 177, 47, 163).
9. low #12-L1e combo roll max 28, JEDI_FOCUS 29 never rolled (vanilla). Fix: Random(26)+1 / Random(16)+14 after confirming intent.

Latent (not live, low urgency): #5 F5 drops 2004 shout intruder call (fix: bypass F5 when oIntruder valid), #7 Rage 271-273 in no family (add to family 7 only if party casts it), #8 L2a/L2b/L2c round drop (unshipped; gate on tUse party buff), #12-L1f comment mismatch, #12-L1g ranged 0x1111 validity (mirror GN_GetSeriesMeleeFeat), #10 (below).
Known: #12-L1a-d, #14 (R1/R2 docs). NEW-1 (Insanity decrement tests nHorror, vanilla :3235 / src_party_ai :3267) is already fixed for party by O3a (party_ai_patches.py:517-526).

## Open questions for a live play check
- #10 do lower tiers stay known? In game: high-level party Jedi with Wound+Kill (or Push+Whirlwind) in Jedi Support; run a GetHasSpell probe or watch spell casts. Pass signal: probe log shows lower tier GetHasSpell=0 (finding is a no-op) or =1 (tier-chain fix is worth doing); in a fight, Tier I casts in <=1 of 10 casts means no problem.
- #4 F4+G1 empty round: cast a buff on a companion that already has it while a spell node is pending. Pass: no "F4 drop ... then G1 drop" pair within 200 ms in companion-ai log with the companion idle next round (kotor2_logs.py check companion_ai).
- #1 controlled companion: take control of a ranged companion with stance masks set. Pass: RangedAI log shows E-H1 using the companion id, not skipping or treating the Exile as isPC.
- #11 buff delay: enter combat with 3 unbuffed Jedi companions. Pass: all buffs cast within 2 AI rounds (no >4 round gap).
- NEW-1: confirm O3a is in the installed party script (party_ai_patches.py output). Pass: party Insanity picked at roughly its 1/N rate in a log of 20 Jedi AoE casts.
