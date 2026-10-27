# W8: forums + sibling Aurora/Odyssey engines, quick-loot check

Checked 2026-10-27. Reddit: blocked for both search and fetch. GameFAQs and StrategyWiki: WebFetch 403, so those rows are search snippets only. Nexus KOTOR 1815: 403.

## A) Forum hits (KOTOR app 32370, KOTOR II app 208580, LucasForums, Steam)

| URL | Asked | Answer |
|---|---|---|
| steamcommunity.com/app/208580/discussions/0/1750149530476946564/ | Corpse loot indicator slow to appear | Pressing Q or E repeatedly cycles all in-range lootable bodies and containers (cerberusiv, 2020-02-24). No take-all. |
| lucasforumsarchive.com/thread/141323-new-inventory | KOTOR II lacks KOTOR I "New Items" list | Poster routine: "open the container, take all the itms, press Start", then X to reach New Items (2004-12-14). Not a documented take-all key. |
| steamcommunity.com/app/208580/discussions/0/458605613400551264 | Player storage | No take-all. Containers keep leftovers. KOTOR I had empty storage containers, KOTOR II does not (2016-01-09). |
| steamcommunity.com/app/208580/discussions/0/541906989401929548 | Controller bindings | No container or take-all button. B clears a stuck HUD menu that blocks containers (2015-07-25). |
| steamcommunity.com/app/32370/discussions/0/6955341523711954974/ | KOTOR I crates refill | Refill bug only, no take-all (2024-11-18). |
| steamcommunity.com/app/704450/discussions/0/3774483849435730951/ (NWN:EE) | Missing loot-all button | Replies: "There is no auto-loot or loot all button." OP says a button exists but shows "cant do it right now" (2019-02-17). Conflict unresolved. |
| deadlystream.com/topic/7258-personal-item-mod/ | Adding loot to containers | Scripting only, no auto-loot or take-all (2019). |
| lucasforumsarchive.com/forum/321, /554, /524 | Loot or take-all thread titles | Board lists scanned, no loot-titled threads. |
| GameFAQs boards (snippets only) | Multi-item transfer | "select all 11 spikes... dump in a bin" thread, no clear answer. |
| Reddit r/kotor, r/patientgamers | Take-all or auto-loot asks | Not reachable. |

Result: no KOTOR thread asks for take-all and gets a mod or key reply. Q/E cycling (KOTOR II) is the only in-game shortcut found. Whether vanilla container UI has a take-all button is not confirmed.

## B) Sibling engines

- **NWN1 EE, Ravick's Looting System Enhanced (LSE)**, author AEA. https://steamcommunity.com/sharedfiles/filedetails/?id=2307769974 . Technique: NWScript package, not an exe hook. Auto-loots on kill and on container close, filtered by weight and value, configured via a "Player Tool 9" feat. Steam removed the item; author points to a "Memoria Edition" successor. Known bugs: quest items vanish, Loxar kill fails.
- **NWN1, Inventory Sort (Vault).** https://neverwintervault.org/project/nwn1/script/inventory-sort . Technique: script. Feat-targeted conversation with "take all items from this container". No key binding.
- **NWN2 (MotB, SoZ, Kaedrin, Tchos).** No loot-all or quick-loot mod found. Kaedrin hits are PrC gameplay packs only. Tchos HD menu (https://neverwintervault.org/node/5402), Divine GUI EE and Charlie's UI are layout or art only. Loot logic runs in container OnOpen scripts. OC uses gp_treasure_op_de, which MotB, SoZ and MoW do not use (Vault snippet, URL unconfirmed). Placed loot may have no open script at all.
- **Jade Empire.** No quick-loot mod found. "Jade Empire in Style" overhaul (https://steamcommunity.com/app/7110/discussions/0/1607148447808079209 snippet) not verified for loot. Steam thread: no official toolkit, "pure hacking".
- **Mass Effect 1 / Legendary Edition.** No take-all or loot mod found. Mods install via ME3Tweaks manager (PC Gamer snippet). A 2022 Steam QoL thread lists no loot mod.

## Verdict

PARTIAL. Sibling precedent exists (NWN1 LSE script auto-loot, NWN1 take-all script). No KOTOR or KOTOR II take-all mod or documented key found.

## Open question

Does the vanilla KOTOR or KOTOR II container window show a Take All button, and which key triggers it? Check in game (open a container, read its buttons).
