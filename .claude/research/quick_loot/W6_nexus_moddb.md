# W6: Nexus / ModDB / Steam Workshop quick-loot survey

Searched 2026-10-27. Scope: K1 and K2 mods that speed looting (quick loot, take all, auto loot, skip container menu, loot on click, footlocker animation, loot at range, auto credits).

Access limits: every nexusmods.com page returned HTTP 403 to WebFetch (kotor/1815, kotor2/1183, kotor2/1190, kotor2/motm, ?tab=description). Wayback Machine blocked. Nexus facts below come from search snippets and are unverified. Downloads and dates for 1815 are not visible.

## Hits

| Name | URL | Game | What it does | Method | Downloads / date |
|---|---|---|---|---|---|
| Mark Empty Containers (ShaeMyName) | https://www.nexusmods.com/kotor/mods/1815 | K1 | Appends " (empty)" to hover name of containers and lootable corpses ("Remains") that hold nothing; updates live after looting empties one. No loot-speed or menu change. | Patcher exe + Apply/Revert .bat patch swkotor.exe; backup swkotor.exe.MarkEmpty.bak. Needs KOTOR Editable Executable. | Not visible (403). Snippet shows "Safe to use" scan. |
| KOTOR 2 Editable Executable 1.0b (Snigaroo) | https://deadlystream.com/files/file/1321-kotor-2-editable-executable/ | K2 | Hex-editable swkotor2.exe for widescreen patching. Not loot. | Replace exe | 12,871 dl; v1.0b 2023-06-02 |
| Prologue Item Recovery (Leilukin) | https://www.nexusmods.com/kotor2/mods/1183 | K2 | Spawns plasteel cylinder with prologue items in Peragus cargo hold. Not loot-speed. | Module or Override files | Not visible |
| TSLRCM Loot fix base scripts 1.3 (Hassat Hunter) | https://deadlystream.com/files/file/325-tslrcm-loot-fix-base-scripts/ | K2 | Modder resource: base lootscript and recompiled spawn scripts. No player-facing looting change. | 3 .nss files, modders only | 589 dl; 2019-08-14 (changelog also lists 2014) |
| TSLRCM 1.8 (Restored Content Mod) | https://www.nexusmods.com/kotor2/mods/46 ; changelog http://www.moddb.com/mods/the-sith-lords-restored-content-mod-tslrcm/downloads (via https://www.dsogaming.com/?p=31188) | K2 | Changelog: "Fixed many issues with the loot-system"; "Modified random loot system". No auto-loot or take-all. | Full mod | Announced 2012-07-26 |
| Looting More Credits (John Doom) | https://steamcommunity.com/sharedfiles/filedetails/?id=2193750934 | K2 | "Looting gives lower quality items, but more credits." Balance only. | Workshop subscribe | Removed by Steam; page says incompatible with TSL; 24,241 subs before removal; posted 2020-08-10 |
| Container credits thread | https://deadlystream.com/topic/3202-tsl-i-need-help-in-increasing-the-credit-gained-from-containers | K2 | Forum thread (2015) on k_inc_treasure and k_inc_treas_k2 credit scripts. Not a mod. | Script edit + recompile | n/a |
| Custom container how-to | https://deadlystream.com/topic/7258-personal-item-mod/ | K2 | Tutorial for scripting a new placeable container. Not a mod. | KotOR Tool + scripting | n/a |
| K1SE script extender | https://www.nexusmods.com/kotor/mods/1852 | K1 | Script extender (binkw32.dll proxy). No loot feature in snippet. | DLL proxy | Not visible |

Also checked, no hit: Steam Workshop K2 search "loot" (5 items, only "Looting More Credits" is loot-titled, https://steamcommunity.com/workshop/browse/?appid=208580&searchtext=loot); Workshop "container" search (7 items, none loot-related); Nexus K2 Mods of the Month (2026 entries, no loot mods, https://www.nexusmods.com/kotor2/mods/motm); DeadlyStream K2 Sith Lords files (no loot filter).

Non-KOTOR analogues (design reference only): Morrowind OpenMW take-all hotkey R (nexusmods morrowind/58130); Oblivion A-button loot-all (oblivion/27638); Mass Effect LE skip Recovered Items menu (masseffectlegendaryedition/1026); Starfield take-all default (starfield/8291).

## Mark Empty Containers: method and TSL port

- What: hover-label change only. No loot-speed or animation change.
- Method: byte patch of swkotor.exe (K1 binary name) through a patcher that needs KOTOR Editable Executable. Revert restores from backup.
- Prerequisite is presumably the K1 editable exe (DeadlyStream file 1320 per a K1 guide snippet). Not fetched, so this is inference.
- TSL port: none found. Page path is /kotor/, the install targets swkotor.exe, and no K2 counterpart appeared across 6 queries. K2 binary is swkotor2.exe, so a port would need a separate patch.

## Verdict

No K1 or K2 quick-loot, take-all, auto-loot, or skip-menu mod surfaced in about 50 search and fetch calls. Nexus was blocked, so the Nexus catalog check is incomplete.

VERDICT: PARTIAL (no click-to-loot mod found; Nexus unverified).
