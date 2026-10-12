# Q-D recon: origin of 0x3F / 0x3D / 0x30 queue noise (companions unconscious)

Root: ~/orca/workspaces/local-setup/KOTOR-II. Repo-relative anchors below. Read-only; no source edited.

## D1 Type meanings (per source and docs)
- 0x3F = AIActionCombat / item-use or cast node appended at the tail. `patch_manager_mods/_shared_src/ActQueue.h:28`: `T_USE_ITEM = 0x3F; // item use / cast-spell action appended by AddItemCastSpellActions`. Also `patch_manager_mods/queue_hud_src/QueueHud.cpp:938`: "A bare 0x3F entry is NOT proof: combat start queues ordinary attacks as 0x3F too."
- 0x3D = AIActionFollowLeader, a follower's resting action. `ActQueue.h:27`: `T_FOLLOW_LEADER = 0x3D; // FollowLeader: a follower's permanent resting action`.
- 0x30 = AIActionJumpToObject, a stop-loop type. `patch_manager_mods/engine_ai_atlas/AI4_scheduler.md:85`. Not in queue-hud source (undocumented there).

## D2 Who pushes, and clearing for HP<=0
- Per-tick push while following: follow driver 0x5C7370 (state 0) queues 0x40 MoveToPoint into the 0x3D node's group. `patch_manager_mods/engine_ai_atlas/AI7_script_interface.md:96`. `patch_manager_mods/29_paused_command_evidence/PQ2.md:48` gives `[MoveToPoint(1), 0x40, 0x3D]` while moving.
- 0x3F is pushed by the command handler (AddItemCastSpellActions / AddAction 0x53F7F0 tail), not per tick. `patch_manager_mods/29_paused_command_queue.md:13-16`.
- Claim not confirmed by docs: `ActQueue.h:27` and `companion_selfheal_src/CompanionSelfHeal.cpp:9` say 0x3D's handler 0x5BF7F0 "re-adds itself at the head" and cite doc 29 D5. Doc 29 D5 (`29_paused_command_queue.md:33`) only says 0x3D "stays at the head".
- Clearing for an unconscious creature: undocumented. The documented ClearAllActions callers (`27_companion_selfheal_evidence/R1_actor_route/FINDINGS.md:30`) are conversations, stance/attack, 0x51CB10, 0x5D75B0, companion-ai. No HP-based caller. `33_ui_queue_hotbar_fixes.md:125` claims follower AI and heartbeat "periodically" clear, with no evidence cited.
- RunActions runs for party every AIUpdate (`PQ2.md:87`).

## D3 cmdq-probe check, saved output only (`.claude/review/work/play1012/coarse_cmdq-probe.txt`)
- C1 (CLR then ROUND ev=start, combat-start wipe) = 93 total. Only 12 are printed; the rest are cut by "+81 more". Not in saved file.
- The 12 printed hits all have dt < 100 ms. Buckets: <20 ms = 9 (L94, L95, L114, L115, L119, L130, L131, L136, L169); 20-100 ms = 3 (L137 99.3, L148 66.8, L149 66.8); 100-500 ms = 0; >500 ms = 0.
- Printed times run t=548588 to 651011 ms. Per `log_review.md` Open 1, the long Down window is T=706355 to about 1030395 ms. Assuming one clock, no printed hit falls in the Down window.
- Owners in printed hits: 0xcb6a748, 0xc6c0020, 0x87ad2f8, 0xcaf2ea0, 0x868f6b8, victim 0xc1dda10. No mapping to companion names in the saved output.
- Probe blind spot: `cmdq_probe_src/CmdqProbe.cpp:183-186` logs CLR only if slots 0..3 hold 0x3F, 0x0F or 0x2E. `party_only=1` (`cmdq_probe.ini:18`). A queue holding only 0x3D or 0x30 never logs CLR, so the 0x3D/0x30 wipes are invisible to this probe.

## D4 Hooks
- ClearAllActions 0x541080: hooked by cmdq-probe as a detour, `skip_original_bytes = false`, log only. `cmdq_probe_src/kotor2-steam-aspyr.hooks.toml:123-127`.
- Call-site skip hook at 0x586F43 (BumpFriends, FM1): `companion_ai_src/kotor2-steam-aspyr.hooks.toml` ~line 153-160. Leader bump only, not HP-based.
- AddAction 0x53F7F0: no hook in any `*.hooks.toml`; no .cpp hooks it.
- No existing hook drops commands for HP<=0 creatures. A new skip-only detour at 0x541080 or 0x53F7F0 is feasible (precedent: 0x586F43). It would also block legit clears, so it needs an HP gate.

## Verdict
PARTIAL. Type meanings and the follow-driver push are identified. The HP<=0 path is undocumented. The probe cannot see 0x3D.
