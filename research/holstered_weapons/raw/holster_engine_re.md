# Odyssey hand/hook attach + weapon wield: open-source RE findings (web only)

Scope: web research only, repo untouched. Read: reone, KotOR.js, xoreos, PyKotor (file list), rakata. No swkotor.exe or Ghidra source read.

## 1. Attach node names (literals in engine reimplementations)
- https://github.com/seedhartha/reone/blob/master/src/libs/game/object/creature.cpp: finalizeModel() does body.attach for "headhook", "gogglehook" (mask), "rhand", "lhand"; "talkdummy" in getSelectablePosition(); equip() looks up "rhand".
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/module/ModuleCreature.ts: loadBody/equipItem/unequipSlot use this.model.rhand and this.model.lhand; loadHead adds to this.head.gogglehook; reads model.camerahook and model.hturn_g.
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/three/odyssey/OdysseyModel3D.ts: attachHead() adds the head to this.headhook; sets hturn_g when node name == "hturn_g"; declares headhook, lhand, rhand fields.
- https://www.xoreos.org/doxygen/d5/d02/kotor2_2creature_8cpp_source.html: Creature::loadHead attachModel("headhook"); playDefaultHeadAnimation/playHeadAnimation getAttachedModel("headhook"). No rhand/lhand code.
- Not found in reone or KotOR.js excerpts: "lhand_g", "rhand_g".

## 2. Hardcoded vs data-driven
- https://github.com/seedhartha/reone/blob/master/src/libs/game/object/creature.cpp: g_rightHandNode="rhand", g_leftHandNode="lhand" are static literals. No 2DA lookup for node names.
- https://github.com/seedhartha/reone/blob/master/src/libs/game/object/item.cpp: baseitems columns read are weaponwield, bodyvar, itemclass, ammunitiontype. No hand/hook column.
- https://github.com/seedhartha/reone/blob/master/src/libs/game/object/creature.cpp (getWeaponModelName): model = lowercased itemClass + "_%03d" (modelVariation). Matches the "w_lghtsbr_001" shape if itemclass is w_lghtsbr (unverified vs baseitems.2da).
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/module/ModuleItem.ts (loadModel): resref = baseItem.itemClass + "_" + 3-digit variation, unless defaultModel is "i_null".
- Conflict: https://deadlystream.com/topic/3268-bundle-tutorial-lightsabers-and-droid-armors/ (search snippet) says model files are w_lght_xxx.mdl. Unverified; disagrees with the itemclass_NNN formula.

## 3. weaponwield numbers
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/enums/combat/WeaponWield.ts: 0 INVALID, 1 STUN_BATON, 2 ONE_HANDED_SWORD, 3 TWO_HANDED_SWORD, 4 BLASTER_PISTOL, 5 BLASTER_RIFLE, 6 BLASTER_HEAVY.
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/module/ModuleCreature.ts (getCombatAnimationWeaponType): hardcoded switch maps weaponwield 1..6 to anim numbers 1,2,3,5,7,9; unarmed 8, none 0. Anim names are 'g'+n+'d1', 'g'+n+'g1', 'damage'+n, 'parry'+n. Enum names and switch comments disagree (3 = TWO_HANDED_SWORD vs "Double Blade"). Unresolved.

## 4. Combat vs non-combat display
- https://github.com/seedhartha/reone/blob/master/src/libs/game/object/creature.cpp: equip()/unequip() set the slot and call updateModel(). finalizeModel() attaches weapon nodes with no combat check. Draw is an animation name ("g%dw%d"), not a visibility toggle.
- https://github.com/KobaltBlu/KotOR.js/blob/master/src/module/ModuleCreature.ts: weaponPowered(on) toggles saber power on LEFTHAND/RIGHTHAND items. No visibility toggle found in the first 100k chars read.

## 5. Adding a new node name (inference, not verified in exe)
- If the exe only resolves literal names, a brand-new attach name needs an exe change. Model-only edits can rename or add dummies but cannot make the exe attach to a name it never looks up.
- Data-only levers: baseitems itemclass, UTI modelvariation, appearance 2DA (body/head models).

## 6. Gaps
- Searches returned mostly unrelated pages (Unity XR, Second Life, NWN). No KotOR wiki page for baseitems weaponwield or hook names was found.
- Not checked: PyKotor mdl sources (only file list fetched), KotOR.js OdysseyModelNode, DeadlyStream 2DA column guide (https://deadlystream.com/topic/4725-detailed-information-of-2da-files/), rakata mdl format (https://synchro.codeberg.page/rakata/formats/models/mdl.html, no hook names).
- Counts: 19 WebSearch, about 20 WebFetch.
