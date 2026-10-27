# W1: Fallout 4 container peek (Quick Loot) findings

Scope: Fallout 4 only. Fallout 76 and Starfield not checked.
Status: 4 pages fetched OK. UESP (403), gamesas (TLS cert error), Nexus (403) could not be fetched.
Facts tagged [snippet] come from search-result text only, not from a fetched page.

## 1) Interaction flow
- Looking at a container (or a fallen enemy) opens a small Quick Loot pop-up of its contents.
  - https://falloutck.uesp.net/wiki/HUDMenu [snippet]
  - https://www.gamesas.com/the-new-looting-system-for-fallout-works-way-describe-t390087.html [snippet, late-2015 pre-release thread]
- Take All works from the pop-up without opening the regular menu. Same gamesas thread [snippet].
- The pop-up does not auto-loot. You choose what to take. Same gamesas thread [snippet].
- E: pressing E on a container item moves the selected item to inventory.
  - https://steamcommunity.com/app/377160/discussions/0/154645299384638401 (troy_cummings, Dec 31 2016) [fetched, user post]
- R: CONFLICT between sources.
  - Dec 2016 post: R on a container "takes everything inside" (same thread as E above) [fetched].
  - Dec 2015 thread: the full screen is called "Transfer (R)" (F4TES, Dec 1 2015) [fetched]. https://steamcommunity.com/app/377160/discussions/0/492379159717872389
- Full transfer screen: a Transfer screen exists and is used beyond containers. GM Pax (May 24 2016) describes it as "like buying from an NPC, or looting a locked container". https://steamcommunity.com/app/377160/discussions/0/364042262889933067 [fetched, workbench context]
- Scroll-wheel selection: no confirmed source. Only a pre-release guess (Crux, Oct 31 2015). https://steamcommunity.com/app/377160/discussions/0/496879865902828418?ctp=5 [fetched]

## 2) Information shown
- Item names: shown, implied by the list design. No direct quote found.
- Weight and value: NOT shown in the pop-up. A Nexus forum user says no stats are shown and asks for a split two-panel quick loot [snippet, URL not confirmed].
- Owned or stolen items: the game marks them. Nexus mod "Intentional Stealing" (ID not verified) says quick-loot takes of owned or stolen goods need a second USE press, and crouching skips it [snippet].
- Visible stolen or owned icon: not confirmed.
- Stack count, locked state: not found.

## 3) Limits
- Targets: containers and fallen enemies [snippet, UESP].
- Maximum rows: not found.
- Empty container: not found.
- Locked containers: GM Pax's wording suggests locked containers use the Transfer-style screen [fetched, implicit only].
- Scripted containers: a Fallout 3 / Tale of Two Wastelands Quick Loot mod (not vanilla FO4) skipped scripted events; those had to be opened normally [snippet, https://taleoftwowastelands.com/viewtopic.php@t=4777].
- Pause: one comment says R's detailed view pauses the game while looting [snippet]. Another says it is unknown whether Transfer pauses time [snippet]. Unresolved.

## 4) Why players say it is faster
- Take All works without opening the full menu [snippet, gamesas thread above].
- Dec 2016 post: R take-everything "saves time" [fetched, steam 154645299384638401].
- Inference, not sourced: fewer keypresses and no full-screen transition. No designer statement found.

## 5) Known UX complaints
- No weight or value in the pop-up, so players cannot judge whether an item is worth taking [snippet, Nexus forum].
- New players confused by E and R: E sometimes appeared to make items vanish [fetched, steam 154645299384638401].
- Full Transfer screen lacked the Pip-Boy categories; sort arrows exist but were hard to find [fetched, steam 492379159717872389]. Concerns the full menu, not the peek.
- Mod-based quick loot skipped scripted containers [snippet, TTW thread].

## Gaps
- No official Bethesda, UESP, or Fallout Wiki text for the key map. The Fallout Wiki mirror returned 404.
- The gamesas and Nexus primary pages were not readable.
