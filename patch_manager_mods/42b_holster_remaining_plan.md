# 42b — Holster remaining work: flourish, draw slide, unarmed locomotion, Fsm, weapon swap (implementation plan)

Parent: `patch_manager_mods/42_holstered_weapons_master_plan.md` (rows HW12-HW17 point here). Feature doc: `42_holstered_weapons.md`. Source: `patch_manager_mods/holster_src/` (manifest id `holster`, live 0.4.0-dev, keep 0.x.y). Written 2026-10-20.
Evidence: 12 Haiku recon + 5 Sonnet reviews (static only, no logs, no game). Reviews are tracked in `research/holstered_weapons/hr1/R1..R5_review.md` (full claim tables with anchors). Haiku T-files stay in the gitignored scratch `.claude/waves/hr1/`.
Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/42b_holster_remaining_plan.md` (or `42:HW12`).

## 0. Scope and what changed from the first list

| # | Item | Verdict after review | Row |
|---|---|---|---|
| 1 | Flourish out of combat | Real gap. Window = server state, no hook, hot-swap. Plus a real width bug in `HolsterDelay.h` that hides it. | HW13 (+HW12 mask) |
| 2 | Draw/stow slide | Real gap. Haiku wiring (Fsm in Ent) rejected by review: 3 blocking defects. Use a small continuous `hs::Slide` follower instead. | HW14 |
| 3 | Unarmed anims while holstered | SCOPE SHRANK. Holstered idle is already wield-independent (ids 0x2710/0x276C). Only walk/run of rifle/heavy and dual-wield styles show an armed arm hold. Probe first (SWAP), override only if the probe says it matters (REINSTALL). | HW15 probe, HW16 gated |
| 4 | Fsm `onSwap` bug | Confirmed. Haiku fix was wrong (draws weapon out of combat, restarts stow timer). If the Slide replaces the Fsm, retire the Fsm (bug disappears with the code). Fallback fix is specified in R4. | HW14 |
| 5 | Weapon-set swap (X) | Swap does reach the hook out of combat. NEW real risk: the placement cache key degenerates to the Gob address, so a reused address keeps the OLD weapon's pose. Small guard fix + log, then a play test. | HW12 + HW17 |

Already done and NOT part of this plan: drawgate (hip out of combat, hand in combat), F8 toggle, shorter combat/flourish exit cut (HD1-HD3 play rows still pending), body rotation (BR1-BR3).

## 1. Decisions (defaults apply unless the owner objects; each is cheap to flip)

| Id | Decision | Why |
|---|---|---|
| D1 | Slide = new `hs::Slide` per placement `Ent` (follows the gate boolean, continuous). `Fsm` is not wired and is DELETED with its tests once the Slide lands. | R2: Fsm stacks 1.5 s stow on `combat_exit_ms`, snaps on re-entry, slides spuriously on new Ents. R4's verified Fsm fix exists as a fallback if the owner wants to keep the Fsm. |
| D2 | `draw_slide_ms` 350 (clamp 0..570), `stow_slide_ms` 350 (clamp 0..1000), 0 = instant (kill switch). Stow delay 0: `combat_exit_ms` stays the only "how long after calm" knob. | 570 ms = vanilla draw window; hit/damage/impact timing ignore the Gob (R2 claim 4). |
| D3 | Flourish window applies to the 3 party slots only. NPC/enemy flourishes keep today's behaviour (hip). New ini key `flourish_draw` (default 1, 0 = old behaviour). | R1 D3/D10: only the PC can trigger 0x79F430; DELAY cut exists only for party. |
| D4 | The window signal is "alert stance without client combat flag" (server `+0x520==1`, byte `+0x11E8==2`, client `+0x138==0`), NOT strictly a flourish: equip-from-inventory, ally alert broadcast and the upgrade UI also set it. Accepted (weapon is drawn up to 1.5 s). | R1 claim 8, FL4 sweeps it. |
| D5 | Unarmed override (A: detour at 0x86367C writing the style byte `[EBP-1]`) only after the probe gate passes. Rejected: tick write of `+0xC5` (breaks attacks/flourish/ranged). Not a menu row (mod-config value test). | R3 D1/D2. |
| D6 | One hot-swap install (0.5.2-dev) carries HW12-HW15. HW16 is a separate REINSTALL, only if the gate passes. | Each wave is DLL-only (hooks/exports/imports unchanged). |

## 2. Specs (zero-read: the exact contracts the workers implement)

### 2.1 HW12 correctness guards (holster 0.4.1-dev)
1. `HolsterDelay.h`: `inline uint32_t stanceByte(uint32_t raw) { return raw & 0xFFu; }`; `delayGate` (line ~50) compares `stanceByte(in.plus11E8) == 2`. Reason: Holster.cpp:613 reads `+0x11E8` as a dword (`rdU32Ok`, :352) but the engine writes ONE byte; the 3 padding bytes hold allocator residue, so the flourish branch can never match for some creatures and HD2 would silently never pass. Test: `plus11E8 = 0xAB000002` must still give the flourish verdict.
2. `HolsterDrawGate.h`: `inline bool sameWeaponModel(uintptr_t entModel, uintptr_t liveModel) { return entModel == liveModel; }`.
3. `Holster.cpp` `HolsterPlace` entry cache (~1102-1141): add `uintptr_t model` to `struct Ent`; read `liveModel = rdU32(kSitePlace, g, 0x84)` once before the lookup loop; an entry hits only if node, gob, handle AND `sameWeaponModel`; on creation store the model and reuse `liveModel` for the `mname` read. When (node,gob,handle) matched but the model differed, log (chPlace, `takeLine`, cap 20): `GOBREUSE gob=%x node=%x oldm=%s newm=%s`. Reason: key = (body hand node, Gob, node handle) and node+handle are identical for every weapon ever held in that hand, so the key is the Gob address alone; a freed+reallocated address would keep the old weapon's type pose (R5).
4. `scripts/holster_log_check.py` (+`scripts/test_holster_log_check.py`): print `GOBREUSE total=N` next to `SWAP total` (line ~385); also counters `FLOURISH open/close` and `SLIDE start/end` (added with HW13/HW14). Prove the scanner on a planted positive line.
5. Tests: `holster_drawgate_test.cpp` `testModelIdentity` (same model reuse, different model new entry, zero matches zero only); `holster_delay_test.cpp` width regression.

### 2.2 HW13 flourish window (holster 0.5.0-dev)
```cpp
// HolsterDelay.h (names follow the existing DelayIn struct)
inline bool flourishWindow(const DelayIn& in) {
    if (!in.serverReadable || !in.clientKnown || in.clientCombat != 0) return false;
    if (in.plus520 != 1 || stanceByte(in.plus11E8) != 2) return false;
    if (in.plus528 != 0) return true;                        // script fake combat (1,2,1): the script owns its end
    return in.plus524 > 0 && in.plus524 <= 8000;             // engine countdown (ms of server time) still running
}
```
- `Holster.cpp`: `static bool flourishOpenFor(uintptr_t owner)` after `refreshParty`: match `g_party.cli[i] == owner`, require `g_party.srv[i]`, re-check `[srv+4] == g_party.id[i]`, read +0x520/+0x524/+0x528/+0x11E8 (kSitePlace saferead), build `DelayIn`, return `flourishWindow`. Edge logs (static `s_open[3]`, probe=1 AND ch_toggle=1, cap ~40): `FLOURISH open slot=%d cli=%x srv=%x p524=%d p528=%u b11E8=%u` / `FLOURISH close slot=%d p520=%u p524=%d`.
- Gate line (:1176) becomes `holdActive(...) || (g_flourishDraw && readable && combat == 0 && flourishOpenFor(owner))` as the 4th arg of `shouldWriteHip`; delete `g_flourishOpen` and its TODO (:62-67). Evaluated lazily inside `HolsterPlace` (common out-of-combat case short-circuits first): no 100 ms latency, independent of `combat_exit_ms`.
- `holsterTick` early return (:689-700) must also stay open when only `flourish_draw` is on (party cache refresh, 250 ms). New ini key `flourish_draw` (0/1, `cfgWarn` on junk), CONFIG log line, `holster.ini` comment.
- Re-holster needs no code: when the window closes (DELAY cut writes +0x524=0, or expiry) the gate flips and the slide/hip write resumes. Expected: weapon returns about 1.5-1.6 s after the press; with a hostile within 30 m the DELAY cut does not fire, the window runs to the engine's ~8 s (matches HD3).
- Tests (`holster_delay_test.cpp`): flourishWindow true for p524 in {1,1500,7999,8000}, false for 0/-1/8001, plus520!=1, stance 1/0, client combat 1, unreadable server/client; plus528!=0 true with stance 2 and false with stance 1. `holster_drawgate_test.cpp`: window true with combat 0 keeps the hand; unreadable owner keeps the hand.
- Not built (documented gap): non-party NPC flourishes; follow-up WI "NPC window with 100 ms TTL cache" only if play asks.

### 2.3 HW14 draw/stow slide, Fsm retired (holster 0.5.1-dev)
```cpp
// HolsterSlide.h  (pure, signed wrap-safe, ~16 B)
struct Slide { float from = 0; uint32_t t0 = 0, len = 0, last = 0; uint8_t dir = 0 /*0 hip,1 hand*/, init = 0;
  void  reset(bool hand, uint32_t now);                    // silent: from=dir=hand, no motion
  float frac(uint32_t now) const;                          // d=(int32_t)(now-t0); d<=0 -> from; d>=len -> dir; else from+(dir-from)*d/len
  void  target(bool hand, uint32_t now, uint32_t drawMs, uint32_t stowMs); // no-op if already heading there; else from=frac(now), t0=now, len=ms*|want-from| (min 1; 0 when ms==0)
  void  touch(uint32_t now);                               // gap guard: if (int32_t)(now-last)>100 mid-slide, t0 += now-last; last=now
};
inline Pose slideShown(const Slide&, uint32_t now, const Pose& hip, const Pose& hand); // blendPose(hip, hand, easeSmooth(frac))
inline uint32_t clampDrawSlideMs(long); inline uint32_t clampStowSlideMs(long);
```
- `Holster.cpp` `Ent` gets `hs::Slide slide;` (`*ent = Ent{}` already zeroes). Clock: `slideNowMs()` = QPC converted to ms (QPC already imported, Holster.cpp:587-591); `GetTickCount` stays for `holdActive`.
- Replace the gate tail (:1176-1181): compute `hip` as today; `first = ent->gate.hip < 0 || ent->gate.owner != owner`; after `gateUpdate`: `if (first || !readable) ent->slide.reset(!hip, now); else { ent->slide.touch(now); ent->slide.target(!hip, now, g_drawSlideMs, g_stowSlideMs); }`; `f = ent->slide.frac(now); if (f >= 1.f) return;` (hand, identical to today). Silent init on first sight is what stops a bogus slide after load, cache eviction or a recycled address.
- Hip path end (:1206-1209): `hipPose = applyPlacement(base, pl); shown = f > 0.f ? blendPose(hipPose, in, easeSmooth(f)) : hipPose;` then `poseSane`, `probeWrite`, memcpy `shown`. Both ends are bit-identical to today's hand and hip paths. `in` and `mountPose` are both world poses from the same node slot (vtbl+0xC), so the blend space is valid. The slide is stateless per frame (the engine rewrites g+0xA4 before H7), so nothing accumulates; any early return leaves the vanilla hand pose.
- Keep the whole path behind `g_drawSlideMs || g_stowSlideMs` so 0/0 is exactly 0.4.x behaviour.
- Logging (chPlace, rate-limited like DRAWGATE): `SLIDE start dir=draw|stow owner=%x gob=%x left=%d from=%.2f ms=%u`, `SLIDE end dir=%s owner=%x ms=%u frames=%u maxdt=%u`. `frames` counts calls while active (measures how often H7 runs; this is UNKNOWN today).
- Config: `draw_slide_ms`, `stow_slide_ms` parse + CONFIG log + `holster.ini`.
- Fsm retirement: delete `struct Fsm`, `kSwapMs/kStowDelayMs` if unused, and the Fsm tests (`holster_logic_test.cpp:154-181`); keep whatever else `HolsterLogic.h` provides. `HolsterSlide.h` old `slidePose(Fsm...)` goes with it. Net: the latent `onSwap` bug cannot exist.
- Tests (`holster_slide_test.cpp`): endpoints bit-exact, mid value, retarget continuity (edge at 0.5 stays 0.5, remaining time halves), tick wrap near 0xFFFFFFF0 both directions, stale `now < t0` returns `from`, `ms == 0` instant, gap guard resumes from last fraction, left/right independent, clamps (0, 570, 571, negative), NaN fraction rejected by `poseSane`. Run `mutate_header.py --jobs 4` on `HolsterSlide.h` (copy the test beside the mutant header, as in the SliderRowLogic run).
- Fallback (owner wants to keep the Fsm): apply R4 steps 1-8 (`elapsed`, `startDraw/startStow` rebase, `onCombat(..., stowDelay)`, `onSwap` no-op unless `lastCombat != 0`, `forceDrawn`), test at :157 changes to `phase == PhDrawn`.

### 2.4 HW15 unarmed probe (holster 0.5.2-dev, SWAP) and HW16 override (REINSTALL, gated)
Facts (R3, from the resolver disassembly): resolver 0x863650 spills the style byte to `[EBP-1]` at 0x863676; 0x86367C `8B 82 D4 00 00 00` (6 B, boundary-clean) reads `base+0xD4`; every style switch reads `[EBP-1]`, nothing reads CL after 0x863685; gate byte `[EBP-0xD]==1` selects the wield table. Out-of-combat idle comes from `FUN_007759B0`: id 0x2711 only if `[creature+0x138] != 0`, so a holstered creature asks 0x2710/0x276C (wield-independent). Wield-dependent rows that remain visible: walk 0x2712/0x2713 and run 0x2714 for styles 2/3/4 (when `base+0xE8 != 0`) and 7/9 (rifle/heavy).
- HW15 (probe, no new hook): new pure header `HolsterAnimStyle.h` + `holster_animstyle_test.cpp`: `idInFilter` ({0x2711,0x27A4,0x2712,0x2713,0x2714}), `styleHasOwnLocoRow` ({2,3,4,7,9}), `decide(id,gate,style,inSet,ownerCombatZero)`, `HolsterSet` (64 entries `{owner, base, stamp}`, wrap-safe tick compare, overflow refuses, expiry 500 ms). Wire: in `HolsterPlace` drawgate branch `set.touch(owner, [owner+0x68], now)` when `hip`, `remove` when not; extend the existing `HolsterAnimResolve` ANIMRES line with `hol=%d comb=%d` and widen `animInteresting` with 0x283C..0x283E, 0x2806, 0x28BD.
- GATE after the probe: if no holstered creature shows `gate=1` with style in {2,3,4,7,9} on ids 2712/2713/2714, STOP and close HW16 as a no-op (record in doc 42).
- HW16 (only if the gate passes, REINSTALL, user GO): `kotor2-steam-aspyr.hooks.toml` replace H4 (0x00863650) by H4b at 0x0086367C (`original_bytes = [0x8B,0x82,0xD4,0x00,0x00,0x00]`, params `ebp`, `[ebp-0x34]`, `[ebp+8]` masked 0xFFFF), `exports.def` `HolsterAnimStyle`, handler writes ONE byte `[ebp-1] = 8` only when gate==1, id in filter, style in {2,3,4,7,9}, owner verified in the set (stamp <= 500 ms, `[owner+0x68] == self`, `[owner+0x138] == 0`). Needs `writeU8Safe`, ebp null/alignment guard, `maybeReload()` first, ini key `unarmed_anim` (default 0 in the first reinstall build). `GameAddr.h` site constant after `query_ghidra.py address 0x863650` (0x86367C is mid-function: verify with `check_hook_bytes.py`, `check_hook_spans.py`), pinned in `common_contract_test.cpp`. Not touched: attacks/ranged/flourish (they read the real byte), second read 0x863D3C.
- Honest value: most single-weapon styles already walk/run with the unarmed clips; this only helps rifle/heavy and dual-wield carriers.

### 2.5 HW17 weapon-set swap
No code beyond HW12 item 3. Facts: both swap paths end in executor 0x5DF920 (out of combat immediately, in combat after the 1000 ms round slot); items move by id, the swapped-in weapon gets a model refresh via 0x76D890 -> 0x7637E0; Gob reuse is unproven. New Gob out of combat shows in the hand 0-1 frames then sits on the hip (no draw anim); in combat it stays in hand (combat gate unaffected). Do not wire `onSwap`. Play rows SW1/SW2 (below) settle it.

## 3. Work breakdown (waves; same-wave agents launch in ONE message; one writer per file)

Contracts: Sonnet worker = `Contract: worker` + `Write Scope` + `python3 scripts/agent_contract.py print worker`; Haiku recon/verify = `recon`; reviewer = `review`. `agent_contract.py snapshot --out .claude/waves/hr2/snapshot.json` before each launch, `agent_contract.py check --scope <globs> -` on every reply. Workers never run the gate or install. Scoped `run_tests.py` only. Commit explicit paths.

| Wave | Agent | Scope (write) | Task | Gate |
|---|---|---|---|---|
| A | A1 Sonnet worker | `holster_src/{Holster.cpp,HolsterDelay.h,HolsterDrawGate.h,holster_delay_test.cpp,holster_drawgate_test.cpp,manifest.toml}` | 2.1 items 1-3, 5; version 0.4.1-dev | `run_tests.py holster_delay`, `holster_drawgate`, build_check holster_src |
| A | A1b Sonnet worker | `scripts/holster_log_check.py`, `scripts/test_holster_log_check.py` | 2.1 item 4 (+ FLOURISH/SLIDE counters, planted-positive test) | `run_tests.py holster_log_check` |
| A | A3a Sonnet worker | `holster_src/{HolsterSlide.h,holster_slide_test.cpp}` | 2.3 pure `Slide` + clamps + tests + mutation run | `run_tests.py holster_slide` + mutate_header |
| A | A4a Sonnet worker | `holster_src/{HolsterAnimStyle.h,holster_animstyle_test.cpp}` (+`test.toml` if flags differ) | 2.4 pure header, set, decide, tests + mutation run | `run_tests.py holster_animstyle` |
| A-check | 4 Haiku verifiers (one per worker) | read-only | rerun the worker's scoped tests, `agent_contract.py check`, list files touched vs scope | PASS/FAIL |
| B | A2 Sonnet worker (after A1) | `Holster.cpp`, `HolsterDelay.h`, tests, `holster.ini`, `manifest.toml` (0.5.0-dev) | 2.2 | `run_tests.py holster_delay holster_drawgate` |
| C | A3b Sonnet worker (after A2) | `Holster.cpp`, `HolsterLogic.h`, `holster_logic_test.cpp`, `holster.ini`, `manifest.toml` (0.5.1-dev) | 2.3 wiring + Fsm retirement | `run_tests.py holster` |
| D | A4b Sonnet worker (after A3b) | `Holster.cpp`, `manifest.toml` (0.5.2-dev) | 2.4 probe wiring | `run_tests.py holster` |
| Review (per wave B-D) | 1 Sonnet reviewer each | read-only | diff review vs this spec (blocking defects only), mutation spot-check, EVIDENCE anchors | APPROVE / REQUEST_CHANGES |
| Opus | main session | everything | read every diff, `check_shared_headers.py` (0 drift, never `--sync` inside an install), `compat_check.py` if `_shared_src` touched, `pre_install_gate.sh --mod holster_src` (one gate at a time; investigate any non-holster failure per the verify-after-fix rule), `kpm_hotswap.py --mod holster --prepare` (must say SWAP), user GO with game closed (`pgrep -a swkotor2`), `--apply`, KPM commits, `kpm_status.py` LIVE, play rows, handoff, memory | GATE PASS + SWAP |

Parallelism: wave A is four writers on four disjoint file sets. Waves B-D are strictly sequential because they share `Holster.cpp`. Then one install. Cost reference: the research swarm (12 Haiku + 5 Sonnet, ~2.1 M tokens) cost about $6; expect the build waves to be similar or lower.
HW16 (after play of HW15): a separate plan section is written only if the gate passes; it uses the same worker/review/Opus pattern plus `kpm_reinstall.py` (needs GO).

## 4. Play rows (add to `PLAY_TEST_QUEUE.md` as `pending` when the build is installed; user plays)

Pre-req: live `holster.ini` has `probe=1 bone_test=1 place_mode=drawgate ch_toggle=1 ch_place=1 ch_anim=1`; `kplog.ini [holster] level=info`.

| Id | Steps | Pass signal |
|---|---|---|
| FL1 | out of combat, no enemy within 30 m, press the flourish | weapon in hand at the press, back on hip within ~1.6 s; `FLOURISH open slot=0 ... p524=8000 ... b11E8=2`, then `DELAY ... kind=flourish wrote=1`, then `FLOURISH close` (open-to-close ~1500-1700 ms) |
| FL2 | flourish with a living enemy within 30 m | `FLOURISH open`, no `wrote=1`; weapon stays drawn until the stance ends (~8 s) |
| FL3 | normal fight with the full party, kill all | no `FLOURISH` line while combat != 0; `DELAY kind=combat` still cuts ~1.5 s (closes HD1) |
| FL4 | (a) equip a weapon calm (b) load a save + change area (c) enemy notices at range | record `FLOURISH open` per case; (b) must not stay open > 1.7 s after load (else `flourish_draw=0` and report) |
| SL-DRAW / SL-STOW | start combat; end combat | `SLIDE start dir=draw ms=350`, `SLIDE end` 330-400 ms with `frames>=8`; stow starts ~1.5-1.8 s after the last hit (not ~3 s) |
| SL-REENTRY | re-aggro mid-stow | `SLIDE start dir=draw from=0.2..0.8`, no pop |
| SL-F8 | F8 draw, F8 stow, reversal mid-slide | start/end pairs, `from>0` on reversal, never stuck between hip and hand |
| SL-PAUSE / SL-LOAD / SL-OFF / SL-CROWD | pause mid-slide; load combat + calm saves; `draw_slide_ms=0 stow_slide_ms=0`; crowded fight | valid endpoint after unpause (record whether SLIDE lines appear while paused); no slide right after load; identical to 0.4.0 when off; no new PLACEREFUSE |
| SW1 | out of combat, set 1 sword/saber, set 2 pistol (or reverse): press X twice, screenshot at once and after 2 s | new weapon on the hip within 2 s with ITS OWN type pose; `SWAP total` +1 per press; `GOBREUSE total=` shown (>0 proves the guard fired) |
| SW2 | same save, in combat with a living hostile at 15 m+, press X, hold 3 s, X again, then leave combat 10 s | swapped weapon in hand within ~1 s, back on hip ~10 s after combat with its own type pose; `DRAWGATE ... combat=1` for the new Gob |
| HW7a-P | holstered party (Atton rifle/pistols etc.): stand 10 s, walk 10 s, run 10 s, enter combat, leave | ANIMRES: `hol=1` rows only 2710/276C/2712/2713/2714, never 2711; any `gate=1` + style {2,3,4,7,9} on 2712/2714 (else STOP, HW16 closed); `comb=0` on every `hol=1` line |

Also pending from before and still needed: BR1-BR3, SC1, RS1, HK1, HD1-HD3.

## 5. Risks (top, full lists in the R-files)

| Risk | Mitigation |
|---|---|
| H7 call rate unknown (slide could stutter) | `frames=` in `SLIDE end`; kill switch `draw_slide_ms=0 stow_slide_ms=0` |
| Flourish window false positives (equip, alert broadcast, upgrade UI) | `flourish_draw=0`; FL4; draws for <= 1.5 s (party DELAY cut) |
| `DELAY kind=flourish` label is a client-flag test, not proof of a key press | read with `FLOURISH` lines and play context |
| DELAY cut / HD1-HD3 never observed live | FL3/HD1 first; window closing depends on the cut |
| Unverified: H7 runs while paused; blur/trail effects read the Gob; X key binding hop | SL-PAUSE; cosmetic; SW1 `SWAP total` |
| Handler crash (HW16 only: byte write into the resolver frame) | probe first; saferead everywhere; ebp guards; host fake-frame test; REINSTALL with rollback |
| Log caps hide entry creation late in a session (PLACENODE 40, HANDATTACH 64) | run SW1/SW2 right after launch; GOBREUSE has its own cap |
| Shared worktree: another session's dirty files break scoped tests | verify on a scratch copy, commit own hunks only |

## 6. Corrections to earlier statements (from the review)
- "Unarmed anims while holstered: armed stance with empty hands" is mostly NOT true: idle is already unarmed. See 2.4.
- Haiku claims overturned by Sonnet review: the `+0x11E8==2` guard is flourish-only (false: six callers write (1,2,x)); the T4 Fsm fix (wrong); the Fsm wiring sketch (3 blocking defects); "all 19 style reads below 0x864439" (18, one in the constant table); "default walk row 0x154" (that is the rifle row); "alt-slot changes skip the model refresh" (true but not the swap's path).

## 7. Agents used (research phase)
12 Haiku recon (T1a-T5b, ~1.43 M subagent tokens) + 5 Sonnet reviewers (R1-R5, ~0.68 M). Reviewers returned: R1 REQUEST_CHANGES (1 blocking), R2 REQUEST_CHANGES (3), R3 APPROVE_MINOR (0), R4 REQUEST_CHANGES (2), R5 APPROVE_MINOR (0). All blocking items are in the Haiku reports, resolved in the specs above.
