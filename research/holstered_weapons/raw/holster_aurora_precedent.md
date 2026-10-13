# Holster / sheathed-weapon precedent (web recon, 2026-10-13)
Scope: Aurora (NWN1/NWN2), Eclipse (DA:O), Jade Empire, OpenMW, UE/Skyrim analogies. 15 searches, 3 fetches (1 returned 404).

## Ranked techniques (transferable to Odyssey)
1. Dual attach + draw-state swap. Sheath dummy on skeleton; weapon parked there when sheathed, moved to hand on draw. ENGINE CODE: yes. Data: named nodes.
   - https://openmw-improved-docs.readthedocs.io/en/latest/reference/modding/extended.html : needs `Bip01 Weapon` + `Bip01 Sheath` (scabbards), `Bip01 Ammo` (quivers); `[Game] weapon sheathing = true`; bows use "Weapon Bone Left" if present, else "Weapon Bone".
   - https://git.cijber.net/mirror/openmw/commit/f6243fae830dc47889b9bcfbc05febfa592f2395 : PR #1615 (Feature #4673) merge; adds scabbard member to ActorAnimation and a sheathing setting.
   - https://gitlab.com/OpenMW/openmw/issues/4673 : request reused an MWSE mod's data format so existing assets work unchanged.
2. Per-weapon-type sheath bones. Table maps weapon class to attach/sheath bone. ENGINE CODE: yes. Data: node names.
   - https://git.cijber.net/mirror/openmw/commit/6019f6507d70345048bb67f73501c8199f997673 : "Corrections for extended weapon animations" adds per-type attach/sheath bones (snippet only, not fetched).
3. Animation-event swap. Draw/holster key fires an event that re-parents the weapon. ENGINE CODE: yes, unless the engine already dispatches the event. Data: event placement.
   - https://forums.unrealengine.com/t/holster-sheath/4507 : notify-driven attach to socket; "Attach To" vs "Attach Actor" pitfall.
   - https://www.nexusmods.com/skyrim/mods/89418 : tiny script on weapon draw/sheath re-equips shield (Skyrim event-hook analogue).
4. Static sheathed prop. Sheath copy baked into skeleton or animation table; always visible. ENGINE CODE: no. Cannot hide on draw without engine code.
   - https://www.nexusmods.com/dragonage/mods/3700 : hip-dagger mod edits BITM_base.gda (Eclipse animation/placement data).
   - https://www.nexusmods.com/dragonage/mods/6719 : one-handed weapons moved to hip by draw/sheath anim edits; cutscenes still draw from back.
5. Back/hip/thigh sockets + interpolated blends. Placement is animation authoring; trigger still needs item 1 or 3. ENGINE CODE: trigger only.
   - https://mocaponline.com/blogs/mocap-news/weapon-animation-systems-guide : holstered poses need extra sockets; smooth interpolation during transitions.
   - https://developer.valvesoftware.com/wiki/Attachment : each attach point is bound to a bone, position relative to it.

## Sibling engines: results
- Aurora NWN1/NWN2: no sheath-node source found. NWN2 toolset offers swappable multi-part weapon models only: https://www.gamebanshee.com/editorials/28218-neverwinter-nights-2-weapons-a-armor-toolset-diary.html
- NWN:EE: sheathed weapons outside combat is a player request, not a shipped feature: https://forums.beamdog.com/discussion/comment/949944
- NWN:EE: SetHiddenWhenEquipped hides equipped world models per slot (engine-side visibility, not sheathing): https://nwnlexicon.com/SetHiddenWhenEquipped
- NWNX / CEP: searched; no sheath plugin or content found (results off-target).
- Jade Empire: nothing on sheathed visuals; overview only: https://en.wikipedia.org/wiki/Jade_Empire
- Eclipse: only DA:O mod pages searched (item 4).

## KOTOR-format facts
- https://synchro.codeberg.page/rakata/internals/mdl_deep_dive.html : no holster/sheath/hip/back concept in the MDL format; node lookup is by name_index; dummy/base type 0x0001.
- Implication: a new named dummy is a data edit; the engine must be made to query it (engine code).
- https://deadlystream.com/topic/11260-editing-an-existing-lightsaber-mod/ : KOTOR hilt guide (snippet only, URL inferred from result list): hilts are separate models linked to a saber dummy.

## Caveats
- Few game-specific hits; NWN/NWNX results mostly off-target.
- OpenMW docs say no engine modification is needed; true only because OpenMW already implements it.
- Ranking is by transferability to Odyssey, not by mod quality.
