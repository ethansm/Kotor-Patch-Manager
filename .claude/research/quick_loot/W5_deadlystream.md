# W5: Deadly Stream loot-speed mods and requests (KOTOR 1 / TSL)

Method: Deadly Stream forum and file search pages (`/search/?q=...&type=core_forums_topic_post`), topic and file pages via WebFetch. Searched 2026-10-27. Nexus pages (kotor/1815, kotor2/1183, kotor2/1190) return HTTP 403 to WebFetch, so Nexus facts come from search snippets.

## Hits

| Name | URL | Game | What it does | Method | Status |
|---|---|---|---|---|---|
| K1 Marked Empty Containers (ShaeMyName) | https://deadlystream.com/files/file/3006-k1-marked-empty-containers/ | K1 (K1R flag) | Adds " (empty)" to hover name of empty containers and lootable corpses ("Remains"). Shows no contents, does not change looting | EXE patch of swkotor.exe via Apply/Revert .bat; needs KOTOR Editable Executable | v1.0.1, June 29 (year not shown). Same description as Nexus 1815 |
| MOD:K1 Marked Empty Containers (release thread) | https://deadlystream.com/topic/12279-modk1-marked-empty-containers/ | K1 | Release thread for the row above; 4 replies about label timing only | Same | Released |
| KOTOR Editable Executable | https://deadlystream.com/files/file/1320-kotor-editable-executable/ | K1 | Hex-editable exe base for patching; no loot content | Tool | v1.0.0 (2018); dependency of the row above |
| K1 Loot Overhaul (N-DReW25) | https://deadlystream.com/files/file/2431-k1-loot-overhaul/ | K1 | Enemy drops, placeables, merchant stock, some quest rewards. No UI or speed change | TSLPatcher | v0.2.0 Beta, Oct 12 2024; needs K1 Community Patch |
| Bashing Overhaul (offthegridmorty) | https://deadlystream.com/topic/12389-modbashing-overhaul/ | TSL | Bashing locked containers can break items inside. No speed, auto-loot, or menu change listed | Scripts k_contain_bash.ncs and m_plc_treasdisp.ncs plus 21 .mod; HoloPatcher | Released Aug 28 2026 per page |
| Autodesk Maya KotOR Level Editor and Importer | https://deadlystream.com/topic/12319-autodesk-maya-kotor-level-editor-and-importer-released/ | K1 | Offline Maya tool that edits placements and footlocker loot | Modding tool | Beta; not a player mod |
| Less Annoying Freebies | https://deadlystream.com/files/file/3011-less-annoying-freebies/ | K1 | Companion free items handed out in bulk, no inventory check. Not container loot | Not stated | Adjacent only (snippet) |
| Revenge of Revan (Logan23) | https://deadlystream.com/topic/179-revenge-of-revan/ | TSL sequel | 2010 opening post says the old looting/inventory system "will be tweaked". No quick-loot detail | n/a | Demo 2011 |

No Deadly Stream file or thread exists for take-all, loot-all, auto-loot, quick-loot, container peek contents, footlocker speed, or container open-animation removal.

## Requests and replies

1. Topic 3202, "(TSL) i need help in increasing the credit gained from containers" (xrook, 2015): https://deadlystream.com/topic/3202-tsl-i-need-help-in-increasing-the-credit-gained-from-containers . Credits, not speed. Replies: edit k_inc_treas_k2 and k_inc_treasure, recompile callers; TSLRCM loot fix.
2. Topic 8748, "Playing TSL for PC finally..." (snippet only): https://deadlystream.com/topic/8748-playing-tsl-for-pc-finally-with-mods-will-post-observations-and-questions-as-i-go/ . Complaint that clicking a container locks movement until the character walks to it. No fix offered in the snippet.
3. Topic 4582, "[KotOR] Item gained trigger" (snippet only): https://deadlystream.com/topic/4582-kotor-item-gained-trigger/ . Reply says the trigger fires for game-placed items, not player looting or buying. Why scripts cannot hook player take actions.
4. Topic 9313, "GUI programming" (snippet only): https://deadlystream.com/topic/9313-gui-programming/ . Workaround: spawn a container, add the item, give it to the player, destroy the container. No native take-all hook.

No mod request for quick loot, take-all, or auto-loot exists in the index. Forum search for those terms returns 0 results.

## Search coverage

- 0 results: "quick loot", "loot all", "take all", "take everything", "auto loot", "autoloot", "open container", "container window", "loot window", "without opening", "looting faster", "mod requests" loot.
- Hit counts: footlocker 145, container 200, looting 36, corpse 264, lootable 19.
- Each search page shows about 20 results. Page 2 was read for footlocker, container, and looting. Corpse and page 3+ were not read.

## Verdict

No click-to-loot, take-all, auto-loot, or quick-loot mod exists on Deadly Stream. The only related mod is the K1 hover-label mod "Marked Empty Containers" (exe patch, same mod as Nexus 1815).
