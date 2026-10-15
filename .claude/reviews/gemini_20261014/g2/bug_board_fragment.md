<!-- G2 fragment: table rows go under '## Batch table' after BG25; detail sections go at the end of the doc. -->
## Table rows

| BG26 | **medium** `gem-K1-5a-01` patch_manager_mods/k1_port/k1_port_tex_dxt.py:229 (wrong-behaviour): DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colour; Guard is deliberate alpha protection but silently leaves edited blocks unchange... | - | k1 (doc 38) | OPEN | | Write the failing test: test_k1_port_tex_tpc.py: DXT1 parent with c0<=c1 blocks, masked red edit; assert every masked pixel red-shifted. Fails today. |
| BG27 | **medium** `gem-QH-2a-02` patch_manager_mods/queue_hud_src/QueueHud.cpp:782 (wrong-behaviour): engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy; engineNe counts every entry; ActQueue.h documents 0x3D as a follower's permanent r... | - | queue-hud (doc 30) | OPEN | | Write the failing test: queue_hud_logic_test.cpp: companion queue [0x3D] only must give DECIDE_DISPATCH_NOW / DS_OK; today BUFFER / DS_ENGINE_BUSY. |
| BG28 | **medium** `gem-CO-2d-03` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707 (wrong-behaviour): keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap; keyFor(0) returns slot0 even though c0.tag is read at 886; afte... | - | selfheal (doc 27) | OPEN | | Fix: Key leader by c0.tag (non-empty, incl. 'player') like companions; keep slot0 only as the fallback. |
| BG29 | **medium** `gem-SH-3a-05` patch_manager_mods/_shared_src/GuiKit.h:299 (wrong-behaviour): imageFor pools failed (null) lookups so hudkit::image retry never reloads; imageFor pools null (line 299) so lazy retries in hudkit::image, resolveArrows and ensureTex... | - | - | OPEN | | Write the failing test: hudkit_test.cpp: loader returns null first then a handle; second image(id) call should call loadImage again and return the handle. (compat_check.py required) |
| BG30 | **low** `gem-CO-2c-03` patch_manager_mods/companion_ai_src/CompanionAI.cpp:707 (log-only): CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change; e1Evaluate only runs from AiTraceAttackEntry, so spinning stays t... | - | companion-ai (owner session) | OPEN | | Fix: Expire tracks unseen for N seconds in a periodic sweep: clear spinning and emit E1 END. |
| BG31 | **low** `gem-CO-2c-05` patch_manager_mods/companion_ai_src/CompanionAI.cpp:589 (log-only): g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses; Line 589 overwrites the baseline with 0 on a transient FnGetArea 0, so the next area chan... | - | companion-ai (owner session) | OPEN | | Fix: Store g_lastCtrlArea only when area is non-zero. |
| BG32 | **low** `gem-HN-6h-04` patch_manager_mods/holster_src/Holster.cpp:128 (log-only): Unreadable creature logs false old=0 edge=1 combat transition; rdU32 returns 0 on saferead failure, so an unreadable creature logs old=0 and a bogus edge=1 when new!=0... | - | holster (doc 42) | OPEN | | Fix: Make a read-fail return a sentinel (0xFFFFFFFF) and print old=? edge=? when the read failed. |
| BG33 | **low** `gem-HB-1c-03` patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708 (wrong-behaviour): Stale tooltip kept when new slot name read is empty; tipKey and tipSet update even when setNative is skipped for an empty name, so the prior entry's tooltip persists; ... | - | hotbar (doc 07/31) | OPEN | | Write the failing test: No host seam for updateButtons; extract the tooltip decision into CombatLogic.h and test empty-name clearing. |
| BG34 | **low** `gem-HB-1c-06` patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650 (log-only): Vanilla tint read immediately overwritten; failure branches are dead code; tOk and aOk are forced true right after the reads, so the failure log branch and the 0.85f f... | - | hotbar (doc 07/31) | OPEN | | Fix: Drop the dead tOk/aOk checks and the 0.85f alpha fallback; keep the forced red tint per the :652 comment. |
| BG35 | **low** `gem-QH-2a-07` patch_manager_mods/queue_hud_src/QueueHud.cpp:1253 (wrong-behaviour): Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge; Unreadable frame stores prev=false so next readable combat frame re-fires handof... | - | queue-hud (doc 30) | OPEN | | Write the failing test: Host test needs an extracted edge helper: sequence combat,unreadable,combat must yield one edge; today two. |
| BG36 | **low** `gem-CO-2d-04` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394 (wrong-behaviour): Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes; Pending/auto/cooldown state is slot-indexed with no creature check; a s... | - | selfheal (doc 27) | OPEN | | Fix: Store the server creature in Pending and cancel when v[i].server differs. |
| BG37 | **low** `gem-CO-2d-06` patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176 (log-only): LMB on companion button at FullHp/Pending skips popover close and noneed log; Companion LMB at FullHp/Pending falls through silently and leaves the popover open (it st... | - | selfheal (doc 27) | OPEN | | Fix: Log noneed/pending for companions like the leader branch; closing the popover on a button click is optional UX. |
| BG38 | **low** `gem-CO-2d-09` patch_manager_mods/_shared_src/EmbedTex.h:149 (log-only): heal() logs giving up on the 20th refill even if that fill succeeded; heals counts attempts and ignores fill() result; line 155 fires at heals==20 even when that fill ... | - | - | OPEN | | Write the failing test: embtex_test.cpp: 20th refill succeeds then uploads; assert no giving-up line. Today :93 pins exactly one line. (compat_check.py required) |
| BG39 | **low** `gem-SH-3a-01` patch_manager_mods/_shared_src/GuiKit.h:583 (wrong-behaviour): Wheel callback passes engine screen size as client rect size to clientToEngineF; cw/ch are the provider's engine size (client rect only as fallback), so the INI path m... | - | - | OPEN | | Write the failing test: guikit_test.cpp: wheel event with client 1000x500, screen_w/h 1920x1080, iniW/iniH set; expected engine_x must scale from 1000, not 1920. (compat_check.py required) |
| BG40 | **low** `gem-SH-3a-07` patch_manager_mods/_shared_src/GuiKit.h:174 (wrong-behaviour): Allocated label leaked on createLabelFromTemplate and bindByTag failure paths; Leak real in createLabelFromTemplate and bindByTag:551; bindByTag template/flags failure... | - | - | OPEN | | Write the failing test: guikit_test.cpp: bindByTag with wrong vtable fake; assert alloc released; today leaked. (compat_check.py required) |
| BG41 | **low** `gem-SH-3b-05` patch_manager_mods/_shared_src/SelfHealUi.h:382 (wrong-behaviour): pendingDone2 now==0 sentinel branch untested; Untested sentinel branch is also buggy: storing 1 for now==0 then computing 0-1 wraps and times out at once; one tick per... | - | - | OPEN | | Write the failing test: selfheal_ui_test.cpp: pendingDone2(p, 0, 10, 3, false, &u=0) must be false; today now-1 wraps to 0xFFFFFFFF and returns true. (compat_check.py required) |

## Detail sections

### BG26 — gem-K1-5a-01 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/k1_port/k1_port_tex_dxt.py:229`
- **Claim** (Gemini 2026-10-14, untrusted until checked): DXT1 splice skips RGB edits on parents with c0<=c1 since encode_rgb is always 4-colour
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. Guard is deliberate alpha protection but silently leaves edited blocks unchanged whenever parent is 3-colour (c0<=c1, incl. flat blocks); encoder is always 4-colour. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/k1_port/k1_port_tex_dxt.py:229` `ch = ch & ((p0 <= p1) == (n0 <= n1))`
  - `patch_manager_mods/k1_port/k1_port_tex_dxt.py:168` `"""(h,w,3) uint8 -> (by,bx,8) uint8 DXT1 colour halves, always 4-colour mode (c0 > c1).`
- **Check steps**: test_k1_port_tex_tpc.py: DXT1 parent with c0<=c1 blocks, masked red edit; assert every masked pixel red-shifted. Fails today.
- **Investigate**: fix sketch: Encode changed blocks of c0<=c1 parents in 3-colour mode (keep index 3 pixels) instead of skipping them, or count and report skipped blocks. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (K1-5a-01).
- **Resolution**:

### BG27 — gem-QH-2a-02 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:782`
- **Claim** (Gemini 2026-10-14, untrusted until checked): engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, medium. engineNe counts every entry; ActQueue.h documents 0x3D as a follower's permanent resting action so companions always read engq>=1. Logs only cover m=0. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:782` `return ok ? s.engineNe : -1;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1475` `qh::DispatchWhy dsWhy = qh::dispatchWhy(buf, s.engineNe, creal, s.nrn);`
  - `patch_manager_mods/_shared_src/ActQueue.h:27` `constexpr int T_FOLLOW_LEADER = 0x3D;  // FollowLeader: a follower's permanent resting action; its handler 0x5BF7F0 re-adds itself at the head (doc 29 D5)`
- **Check steps**: queue_hud_logic_test.cpp: companion queue [0x3D] only must give DECIDE_DISPATCH_NOW / DS_OK; today BUFFER / DS_ENGINE_BUSY.
- **Investigate**: fix sketch: Count only non-0x3D entries (and bare 0x3F with no round nodes) in engineNe for decideAction/dispatchWhy, like makeEngineView ignores them. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (QH-2a-02).
- **Resolution**:

### BG28 — gem-CO-2d-03 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707`
- **Claim** (Gemini 2026-10-14, untrusted until checked): keyFor(0) returns slot0 ignoring tag, so settings follow party position not creature after leader swap
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. keyFor(0) returns slot0 even though c0.tag is read at 886; after a leader swap the new leader inherits the previous leader's settings. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:707` `if (mi == 0) return "slot0";`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:887` `c0.cfg = shui::lookup(g_cfg, keyFor(0));`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Key leader by c0.tag (non-empty, incl. 'player') like companions; keep slot0 only as the fallback. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-03).
- **Resolution**:

### BG29 — gem-SH-3a-05 (medium, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:299`
- **Claim** (Gemini 2026-10-14, untrusted until checked): imageFor pools failed (null) lookups so hudkit::image retry never reloads
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, medium. imageFor pools null (line 299) so lazy retries in hudkit::image, resolveArrows and ensureTextures hit the pooled null and never reload. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:299` `memset(&pool[n], 0, sizeof pool[n]); strncpy(pool[n].name, buf, 16); pool[n].im = im; ++n;`
  - `patch_manager_mods/_shared_src/HudKit.h:45` `if (!cache[id]) cache[id] = guikit::imageFor(detail::kHudKitTextures[id].resref);`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:504` `if (!g_arrowNeg) g_arrowNeg = guikit::imageFor("uibit_arrowneg_p");`
- **Check steps**: hudkit_test.cpp: loader returns null first then a handle; second image(id) call should call loadImage again and return the handle. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Do not pool null loadImage results (store only non-null), or return a retry flag so lazy callers re-query once the texture exists. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-05).
- **Resolution**:

### BG30 — gem-CO-2c-03 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_ai_src/CompanionAI.cpp:707`
- **Claim** (Gemini 2026-10-14, untrusted until checked): CrTrack caches raw creature pointer; stale spinning state and dangling cr after death or area change
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, log-only, low. e1Evaluate only runs from AiTraceAttackEntry, so spinning stays true and END is never logged after death or area change; t.cr is never dereferenced stale.
- **Audit evidence**:
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:707` `} else if (t.spinning) {`
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:790` `if (now - t->secStart >= 1000) e1Evaluate(*t, now);`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Expire tracks unseen for N seconds in a periodic sweep: clear spinning and emit E1 END. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2c-03).
- **Resolution**:

### BG31 — gem-CO-2c-05 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_ai_src/CompanionAI.cpp:589`
- **Claim** (Gemini 2026-10-14, untrusted until checked): g_lastCtrlArea overwritten with 0 during transition so PARTY why=area misses
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. Line 589 overwrites the baseline with 0 on a transient FnGetArea 0, so the next area change logs no why=area; probe log only.
- **Audit evidence**:
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:589` `if (have) { g_partyLoaded = true; g_lastCtrl = ctrl; g_lastCtrlArea = area; }`
  - `patch_manager_mods/companion_ai_src/CompanionAI.cpp:587` `else if (have && area && g_lastCtrlArea && area != g_lastCtrlArea) why = "area";`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Store g_lastCtrlArea only when area is non-zero. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2c-05).
- **Resolution**:

### BG32 — gem-HN-6h-04 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/holster_src/Holster.cpp:128`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Unreadable creature logs false old=0 edge=1 combat transition
- **Still real (2026-10-14 G2)**: G1 UNSURE -> v2 CONFIRMED -> skeptic real=yes, log-only, low. rdU32 returns 0 on saferead failure, so an unreadable creature logs old=0 and a bogus edge=1 when new!=0; probe log only.
- **Audit evidence**:
  - `patch_manager_mods/holster_src/Holster.cpp:128` `uint32_t v = 0;`
  - `patch_manager_mods/holster_src/Holster.cpp:161` `const uint32_t oldState = off ? rdU32(kSiteComb, c, off) : 0xFFFFFFFFu;`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Make a read-fail return a sentinel (0xFFFFFFFF) and print old=? edge=? when the read failed. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HN-6h-04).
- **Resolution**:

### BG33 — gem-HB-1c-03 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Stale tooltip kept when new slot name read is empty
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. tipKey and tipSet update even when setNative is skipped for an empty name, so the prior entry's tooltip persists; needs an unreadable name.
- **Audit evidence**:
  - `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708` `if (!b.tipSet || b.tipKey != e.key) {`
  - `patch_manager_mods/_shared_src/GuiTip.h:53` `inline bool clearNative(uintptr_t ctl) { return setNative(ctl, ""); }`
- **Check steps**: No host seam for updateButtons; extract the tooltip decision into CombatLogic.h and test empty-name clearing.
- **Investigate**: fix sketch: Call guitip::clearNative(b.bg.c) when name[0]==0 instead of skipping, so tipKey and the shown tooltip agree. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HB-1c-03).
- **Resolution**:

### BG34 — gem-HB-1c-06 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Vanilla tint read immediately overwritten; failure branches are dead code
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. tOk and aOk are forced true right after the reads, so the failure log branch and the 0.85f fallback are unreachable.
- **Audit evidence**:
  - `patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:650` `tOk[g] = guikit::getBorderTint(btnCtl, tint[g]) && (tint[g][0] > 0.01f || tint[g][1] > 0.01f || tint[g][2] > 0.01f);`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Drop the dead tOk/aOk checks and the 0.85f alpha fallback; keep the forced red tint per the :652 comment. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (HB-1c-06).
- **Resolution**:

### BG35 — gem-QH-2a-07 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/queue_hud_src/QueueHud.cpp:1253`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Unreadable frame stores prev=false so next readable combat frame re-fires handoffTick with a fresh first-send ServerClearAllActions; rare, needs HUD flag read failing mid-combat.
- **Audit evidence**:
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1253` `const bool combatNow = g_combat != 0 && !g_combatUnreadable;`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1255` `for (int k = 0; k < 3; ++k) { edgeFlags[k] = combatNow && !g_prevCombat[k]; g_prevCombat[k] = combatNow; }`
  - `patch_manager_mods/queue_hud_src/QueueHud.cpp:1810` `g_capTick = GetTickCount();`
- **Check steps**: Host test needs an extracted edge helper: sequence combat,unreadable,combat must yield one edge; today two.
- **Investigate**: fix sketch: Do not write g_prevCombat when g_combatUnreadable; keep last readable state so no false rising edge. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (QH-2a-07).
- **Resolution**:

### BG36 — gem-CO-2d-04 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Pending, auto and cooldown state indexed by party slot not creature; leader swap misattributes
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, low. Pending/auto/cooldown state is slot-indexed with no creature check; a swap inside the 6 s window misattributes DONE/timeout and can allow an early second use.
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:394` `else if (mi == 0) { g_pend0 = shui::Pending(); g_pend0.on = true; g_pend0.since = now; g_pend0.preHp = m.cur; g_pend0.preStack = pk.count; g_unqueuedSince0 = 0;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:757` `int hp0 = 0 < cnt ? v[0].cur : g_pend0.preHp;`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:790` `for (int i = 1; i <= 2 && i < cnt; ++i) {   // never member 0 (the controlled leader)`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Store the server creature in Pending and cancel when v[i].server differs. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-04).
- **Resolution**:

### BG37 — gem-CO-2d-06 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176`
- **Claim** (Gemini 2026-10-14, untrusted until checked): LMB on companion button at FullHp/Pending skips popover close and noneed log
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. Companion LMB at FullHp/Pending falls through silently and leaves the popover open (it stays open on valid clicks too); leader branch logs noneed.
- **Audit evidence**:
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1176` `} else if (Card& c = g_card[onBtn]; shui::clickAllowed(c.st)) {`
  - `patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1182` `if (onBtn >= 0) g_popMi = (g_popMi == onBtn) ? -1 : onBtn;   // 0 = leader, 1..2 = companions`
- **Check steps**: Re-read the evidence lines on HEAD; reproduce from the claim.
- **Investigate**: fix sketch: Log noneed/pending for companions like the leader branch; closing the popover on a button click is optional UX. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-06).
- **Resolution**:

### BG38 — gem-CO-2d-09 (low, G2-validated, logic/log-only)
- **Where**: `patch_manager_mods/_shared_src/EmbedTex.h:149` (same root cause: CO-2d-09, QH-2a-11, SH-3a-06)
- **Claim** (Gemini 2026-10-14, untrusted until checked): heal() logs giving up on the 20th refill even if that fill succeeded
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, log-only, low. heals counts attempts and ignores fill() result; line 155 fires at heals==20 even when that fill succeeded. Counter re-arms on upload, so harmless. Opus gate read the lines (g2/gate.md).
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/EmbedTex.h:149` `if (!built && !px) { ++s.heals; detail::fill(s, "refilled after a texture reset"); }`
  - `patch_manager_mods/_shared_src/EmbedTex.h:155` `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills (built=%d missing=%d pixels=%p); art missing, labels still work",`
- **Check steps**: embtex_test.cpp: 20th refill succeeds then uploads; assert no giving-up line. Today :93 pins exactly one line. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Log give-up only when the final fill failed (capture the detail::fill result) or reword the message to say refill attempts. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (CO-2d-09, QH-2a-11, SH-3a-06).
- **Resolution**:

### BG39 — gem-SH-3a-01 (low, G2-validated, logic/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:583`
- **Claim** (Gemini 2026-10-14, untrusted until checked): Wheel callback passes engine screen size as client rect size to clientToEngineF
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 PARTIAL -> skeptic real=yes, wrong-behaviour, low. cw/ch are the provider's engine size (client rect only as fallback), so the INI path maps 1:1 instead of client to INI size; never zero, only differs when client != engine size.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:583` `int sw = ev->screen_w, sh = ev->screen_h;`
  - `patch_manager_mods/gui_input_src/GuiInputProvider.cpp:92` `ex = static_cast<float>(pt.x) * static_cast<float>(sw) / static_cast<float>(r.right);`
- **Check steps**: guikit_test.cpp: wheel event with client 1000x500, screen_w/h 1920x1080, iniW/iniH set; expected engine_x must scale from 1000, not 1920. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: With INI override, scale from client coords using a real client size (add client_w/h to K2WheelEvent) instead of ev->screen_w/h. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-01).
- **Resolution**:

### BG40 — gem-SH-3a-07 (low, G2-validated, memory-safety/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/GuiKit.h:174` (same root cause: SH-3a-07, HB-1c-02)
- **Claim** (Gemini 2026-10-14, untrusted until checked): Allocated label leaked on createLabelFromTemplate and bindByTag failure paths
- **Still real (2026-10-14 G2)**: G1 PARTIAL -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Leak real in createLabelFromTemplate and bindByTag:551; bindByTag template/flags failures keep the control in the panel list (addToList=1), not leaked.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/GuiKit.h:174` `if (!rdInt(c, oId, &id) || id == IdSentinel) { if (why) *why = CF_Template; return 0; }   // template not in the layout`
  - `patch_manager_mods/_shared_src/GuiKit.h:551` `if (!rd32(c, 0, &vt) || vt != labelVt) { r.why = CF_Vtable; r.badVtable = vt; return 0; }`
  - `patch_manager_mods/_shared_src/GuiKit.h:167` `if (!rd32(c, 0, &vt) || vt != labelVt) { if (why) *why = CF_Vtable; if (badVtable) *badVtable = vt; return 0; }`
- **Check steps**: guikit_test.cpp: bindByTag with wrong vtable fake; assert alloc released; today leaked. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Same as HB-1c-02; bindByTag only leaks on the CF_Vtable return at :551. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3a-07, HB-1c-02).
- **Resolution**:

### BG41 — gem-SH-3b-05 (low, G2-validated, test-gap/wrong-behaviour)
- **Where**: `patch_manager_mods/_shared_src/SelfHealUi.h:382`
- **Claim** (Gemini 2026-10-14, untrusted until checked): pendingDone2 now==0 sentinel branch untested
- **Still real (2026-10-14 G2)**: G1 CONFIRMED -> v2 CONFIRMED -> skeptic real=yes, wrong-behaviour, low. Untested sentinel branch is also buggy: storing 1 for now==0 then computing 0-1 wraps and times out at once; one tick per 49.7 day GetTickCount wrap, benign early clear.
- **Audit evidence**:
  - `patch_manager_mods/_shared_src/SelfHealUi.h:382` `if (*unqueuedSince == 0) *unqueuedSince = now ? now : 1u;   // 0 is the "not running" sentinel`
  - `patch_manager_mods/_shared_src/selfheal_ui_test.cpp:335` `KT_CHECK(pendingDone2(p, 1100, 11, 3, true, &u));          // hp up wins even while queued`
- **Check steps**: selfheal_ui_test.cpp: pendingDone2(p, 0, 10, 3, false, &u=0) must be false; today now-1 wraps to 0xFFFFFFFF and returns true. Run `python3 scripts/compat_check.py <files>` after any `_shared_src` change.
- **Investigate**: fix sketch: Compute elapsed against a local start (now ? now : 1 stored, but compare using now - start with start==now on first arm), or return false on arming call. Source rows: `.claude/reviews/gemini_20261014/validated.tsv` (SH-3b-05).
- **Resolution**:

