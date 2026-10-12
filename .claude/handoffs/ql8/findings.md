# QL9 P2 play analysis (10-12), frozen logs in ./logs, wave .claude/waves/w2-33-QL9 (u01-u05, Haiku, facts re-checked on disk)

User result: S1 ok; S2 force powers work; S3 ally medpack with no enemies -> full combat UI (red cancel + top bar);
S4 attack with items queued -> combat starts, attack NOT applied, old queue stays; S5 enemy force power -> noncombat queue cleared, combat, power queued.

1. Whole play was mostly PAUSED (PAUSE val=1 for 334.5s-459.8s). probe CMD real=0 is the paused signature (items and spells alike).
2. QL9: log has ZERO buffered spells (all CMD spell = dispatch_now, engq=0). Only replay = cmd 9 at a combat hand-off (L380, pos=null):
   first replayed spell that REACHED the server cast (probe CMD id=22 at T=364854.944, real=0 because paused). exec=0 is paused, not proof of failure.
   Verdict G3: ship spell_replay_pos=1; user confirms force powers work. Flag K on cmd 9 = paused hand-off, expected.
3. QL8 soft combat (G4): SOFT_CAND hostile triple per edge: S3 edge L74 = 0,0,0 (cr0=0 all). Every other edge has hostile=1 on >=1 member
   (L190 1,1,0; L308/L335/L371/L407 1,1,1; L501 1,0,0). Rule: soft = no member has a non-party target id (+ optional all cr0==0). n=1 soft sample.
4. S4/S5 root cause: handoff_clear_first clears the engine queue at combat start, which wipes the player's NEW attack/power (3f entry);
   the old command is then re-sent: L316 cmd 8 (ROUND m0 n=0 then n=1 subs=10), L380 cmd 9 (n=0 then n=1 subs=9). Companion m1 keeps its attack (n=1 subs=1).
   L501: held head (cur=1, 2e) blocks; the queue sits 36 s until the menu edge (HANDOFF_HELD resend).
