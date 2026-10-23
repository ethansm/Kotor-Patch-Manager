# Review wave rv-qac: shared context (read this first, ~1 page)

Game: Star Wars KOTOR II, Steam Aspyr exe, single player (client + server objects live in one process). Mods are DLL patches
(Kotor Patch Manager) that hook engine functions. Two mods are under review:
- **queue-hud** (`patch_manager_mods/queue_hud_src/`): out-of-combat "action queue". While the game is PAUSED (or busy), use-item and
  cast-Force-power clicks are captured by detours on the client wrappers PerformUseItem 0x775600 / PerformCastSpell 0x775700, BUFFERED
  (max 3 per party member + 1 active), drawn as icons (leader panel + small strips next to companion portraits, with Cancel buttons),
  and dispatched one at a time once the engine is idle. At combat start a "hand-off" re-sends the buffer to the engine in click order.
  Pure logic is in QueueHudLogic.h (unit tested); engine glue in QueueHud.cpp.
- **action-hotbar** (`patch_manager_mods/action_hotbar_src/`): non-combat on-screen hotbar, keys 1-0 bound to powers/items; invokes the
  HUD's own quick-action path FUN_00751750 (which ends in the same Perform* wrappers), per-slot target modes (@self, @lowhp), per-save INI.

Verified engine facts (trust these):
- Party slot 0 = the controlled leader. Client Perform* wrappers send a message the server applies to the CONTROLLED creature, whatever
  creature pointer is passed. Switching leader (Tab / portrait click) reorders party slots; queue-hud re-files buffers by owner (IDENT_SWAP).
- A non-controlled companion's engine queue normally holds 0x3D FollowLeader at its head; that handler re-adds itself at the head, so a
  cast/use (0x3F combat action + a "round node") appended behind it never runs ("starvation") until the companion leads again.
- Server action queue at creature+0x100; round object at creature+0x10DC; round flags aa8/ab8 (busy windows ~0.7 s after a cast).
  Paused: nothing pops. ServerClearAllActions 0x51CB10 empties queue + round list.
- QL20 (installed today): queue-hud now sends a companion's buffered Force power (object target) via AddUseTalentOnObjectActions
  0x57D8B0 on that companion's SERVER creature (`serverCast`), and `kickStarved` rotates a starved 0x3F to the head (same method as the
  companion-selfheal mod). Items and location-targeted powers for companions are still HELD until that companion leads.
- PerformCastSpell stack args are really (spellId, targetClientId, float* pos, multiclass byte, meta, domain, int flag); queue-hud's
  field names are historical: `arg2` = target id, `targetId` = the pos POINTER (a stack address captured at click time), `posPtr` = the int flag.

What to look for: real bugs, unhandled edge cases (pause/unpause, leader switch, party change, death/Down, area transition, save/load,
menu open, combat start/end mid-action, dialogue/cutscene, item stack used up, FP run out, target dies/leaves, GetTickCount wrap,
buffer full, re-entrancy, two mods acting on the same queue), wrong assumptions, doc-vs-code mismatches, logging blind spots, UX problems.
Do NOT report style nits. Do NOT report something as a bug unless you can point at the line and give a concrete failure scenario.

## Output (write ONLY your output file; markdown)
Header line: `# <unit id> findings`. Then up to 15 findings, most severe first, each exactly:
```
## <unit id>-F<n> [high|med|low] <category: bug|edge-case|bad-assumption|doc-mismatch|logging|ux|perf>
- Where: <repo-relative path:line[-line]> (more anchors ';'-separated)
- Claim: <one or two sentences>
- Scenario: <concrete steps/state -> wrong result>
- Fix: <suggested resolution, 1-3 sentences>
- Confidence: <high|med|low> (<why>)
```
Quote at most 3 lines of code per finding. If you read past your windows to confirm something, note it in Where.
Then reply with the 5-line RECON block only.
