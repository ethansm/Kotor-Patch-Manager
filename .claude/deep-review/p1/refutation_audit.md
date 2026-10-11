# Refutation audit (Sonnet second pass over REFUTED findings)

Advisory only. This report never changes the ledger, the tiers or verified_findings.*; a human promotes items.

Audited specialties: build-config, source-logic. Audited 1 of 1 refuted items; CLI-reported cost $0.068162 over 1 call(s).

| verdict | items |
|---|---|
| real | 0 |
| design-risk | 0 |
| refute-holds | 1 |

Items whose verdict has no verified evidence quote: 0. Prior refutation quality by batch: {'sound': 1}.

## Promote candidates (0): real or design-risk with a verified quote


## real (0)


## design-risk (0)


## refute-holds (1)

### 7cf902d24841  patch_manager_mods/level_up_override_src/LevelUpOverride.cpp:2043-2048  [none] logic (source-logic)
- Evidence quotes verified: 2/2; earlier: Haiku refuted, Sonnet severity none
- Reasoning: I read Party.h table() and the GameAddr.h getter note. table() reads app=*appGlobal, a=*(app+4) (the client), b=*(a+4), then t=*(b+0x270). The engine getter FUN_0073fb90(A) is documented as *(*(A+4)+0x270). With A=client=a, that equals *(*(a+4)+0x270), the same chain. The raw walk is equivalent, and each link is safe-read with a failure code. I did not re-read the getter's disassembly myself. The remaining risk is that the getter body differs on another build.

