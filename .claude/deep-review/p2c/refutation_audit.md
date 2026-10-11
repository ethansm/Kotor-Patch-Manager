# Refutation audit (Sonnet second pass over REFUTED findings)

Advisory only. This report never changes the ledger, the tiers or verified_findings.*; a human promotes items.

Audited specialties: build-config, source-logic. Audited 6 of 11 refuted items; CLI-reported cost $0.234879 over 1 call(s).

| verdict | items |
|---|---|
| real | 0 |
| design-risk | 1 |
| refute-holds | 5 |

Items whose verdict has no verified evidence quote: 0. Prior refutation quality by batch: {'sound': 1}.

## INCOMPLETE

- missing batches: batch_02
- missing items: 0fc98e09742d, 3c032fcc205b, 89fc8d95002c, b5a62bbbd646, d56064e2a823
- re-run `run_refute_audit.sh` (idempotent)

## Promote candidates (1): real or design-risk with a verified quote

- 6572c3d2f7ca patch_manager_mods/_shared_src/GameAddr.h:69 [low] design-risk (source-logic)

## real (0)


## design-risk (1)

### 6572c3d2f7ca  patch_manager_mods/_shared_src/GameAddr.h:69  [low] other (source-logic)
- Evidence quotes verified: 2/2; earlier: Haiku refuted, Sonnet severity none
- Reasoning: Read GameAddr.h 1-10 and 66-69, plus a grep of PerformCastSpell and PerformUseItem. The header promises one K2AddrTable field per name, spelled the same. PerformUseItem has K2Addr.h entries (lines 187, 361, 526) and a k2addr_test check. PerformCastSpell appears only in GameAddr.h and QueueHud.cpp, with no table field or test. QueueHud is Steam-only, so there is no present GOG crash. The gap matters if GOG-capable code later uses gameaddr::PerformCastSpell directly.
  - check: Compare the GameAddr.h names against the K2AddrTable fields and the k2addr_test.cpp coverage.
  - check: Confirm no GOG-targeted mod references gameaddr::PerformCastSpell.
  - quote patch_manager_mods/_shared_src/GameAddr.h:4: `names from the per-version table instead: K2Addr.h defines struct K2AddrTable (one field per name here, same spelling, plus the`
  - quote patch_manager_mods/_shared_src/GameAddr.h:69: `constexpr uintptr_t PerformCastSpell = 0x00775700;`


## refute-holds (5)

### 97f5356f31c8  patch_manager_mods/_shared_src/SelfHealUi.h:46-49  [none] logic (source-logic)
- Evidence quotes verified: 2/2; earlier: Haiku intended-behaviour, Sonnet severity none; also covers duplicates c1437e3479e9
- Reasoning: Read SelfHealUi.h 44-49 and the leaderButtonRect grep. The 'left of the vitality bar, vertically centred' comment describes the companion buttonRect, not leaderButtonRect. The docs (27_companion_selfheal.md:278 and 33 SH2, DONE) specify a fixed 58x58 button at the leader portrait's top-left. The one caller (CompanionSelfHeal.cpp:824) uses the same 58 px size in its fallback. The behaviour is deliberate and documented. Missing unit-test coverage is hygiene only.

### fc0796ea9fce  patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1809-1817  [none] logic (source-logic)
- Evidence quotes verified: 3/3; earlier: Haiku refuted, Sonnet severity none
- Reasoning: Read ActionHotbar.cpp 1809-1817, SeDispatch.h 120-145 and SaveEventsProvider.cpp 102-159. emitFolder drops events whose folder is unrecognised (abs.empty()) before dispatch. Hub::emit also returns -1 for an empty folder on non-NEW_GAME kinds. So LOAD_REQUEST never reaches restoreFromSave with an empty folder. restoreFromSave is called only at line 1881. Its empty-dir branch is a defensive guard and not a reachable lost-restore path.

### 10750a5a4474  patch_manager_mods/action_hotbar_src/HotbarLogic.h:89-93  [none] logic (source-logic)
- Evidence quotes verified: 2/2; earlier: Haiku intended-behaviour, Sonnet severity none
- Reasoning: Read HotbarLogic.h 73-105. The header comment lists the sentinel range 0xFFFFFFF0..0xFFFFFFFE as a standalone Exile criterion, and test AH3 pins it. Companions with tags are handled earlier by the tag branch. Dynamic companion ids are obj8000xxxx per the docs. Nothing in the tree shows a companion holding a sentinel id. The untested combination is a coverage gap, not a defect.

### f0b28bacd0ef  patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:775-780  [none] logic (source-logic)
- Evidence quotes verified: 2/2; earlier: Haiku intended-behaviour, Sonnet severity none
- Reasoning: Read CompanionSelfHeal.cpp 735-795. k is derived from the slot's position relative to the HUD (line 759), so k==0 is the first HUD portrait slot, which by construction is party position 0 and the leader. The companion branch needs a creature match because member indices 1 and 2 can map to any slot. The leader card is keyed on position, which is the same indexing useNow uses (v[0]). Docs record the slot->creature mapping as log-verified. No mismatch is demonstrated.

### 7326b0db3b09  patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml:29  [none] hook (build-config)
- Evidence quotes verified: 2/2; earlier: Haiku refuted, Sonnet severity none
- Reasoning: Read hooks.toml 25-42, GameAddr.h 69 and the QueueHud.cpp grep. 0x00775700 + 3 = 0x00775703, which matches the TOML address and the comment at QueueHud.cpp:972. The 6 original_bytes start with 83 EC 0C (SUB ESP,0xC), the instruction after the prologue. The values agree today. The hand-synced duplication is a latent drift risk only, and the repo has byte-verification scripts for it.

