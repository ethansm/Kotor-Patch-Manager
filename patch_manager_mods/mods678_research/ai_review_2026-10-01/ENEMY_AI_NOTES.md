# Enemy AI improvements: notes for a future iteration (user decision 2026-10-01: party first)

None of these are built. Each one makes enemies smarter or tougher, so they need a balance check and a log-only run (`companion-ai` trace with `log_all=1`) first.

**Shared AI (`k_inc_generic`/`k_inc_gensupport`).** These are party-gated in the 2026-10-01 build. Enemy versions would need the 52 NPC scripts that embed the include recompiled, or those NPCs routed to `k_ai_master`.
- **H1 Revitalize:** NPC Jedi Support goes idle whenever a *party member* is knocked out (`GN_CheckIfKnockedOut` reads the player's party). Script design: user decision U2.
- **H2:** the NPC branch of `GN_CheckIfInjured` only checks itself (`fNPC < 0.5`). An enemy healer never heals its allies.
- **M2:** area powers (Storm/Wave/Insanity) can be centred on an NPC's own ally.
- **M3:** the aggressive target search prefers stunned/held/asleep targets (an inverted test).
- **M4:** `nDroid` is shadowed, so NPCs pick organic-only powers against droid party members (HK-47, T3-M4, G0-T0, Remote).
- **M5:** the poison cure never runs.
- **M7:** mining lasers (baseitems 104/105) count as melee, so Peragus mining-droid behaviour may be affected.
- **E3:** a 2 m run at any unseen enemy within 20 m.

**Engine.**
- **P1:** the perception throttle never stores its timestamp. This is shipped OFF (`p1=0`) in `companion-ai`; turn it on after a stealth/aggro test. It changes the cadence of enemy spot/listen rolls.
- **Mod 8 E7 (doc 13):** an enemy ranged AI that holds when the shot is blocked and never walks into melee (`hold_enemies`). Needs a parity argument plus `log_all` Stage 0 first.
- **A1 for NPC stances:** check whether NPC creatures can reach the stance 11/12 skip-approach path (the attacker != controlled-creature test).

Source evidence: `review_ai_scripts_decision.md`, `review_ai_scripts_events.md`, `review_ai_engine.md`, `designs/design_ai_script_fixes.md` section 9.
