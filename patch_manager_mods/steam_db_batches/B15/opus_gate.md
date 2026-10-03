# Opus gate B15 (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches; 1 decoy key error(s) excluded (decoy = Opus-shipped counterpart)**

- decoy cards: 40 (per chunk {'1': 10, '2': 10, '3': 10, '4': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

- B15-4-38 0x00784680 -> 0x0066f0a0 (decoy_prev): the decoy is the Opus-shipped counterpart, so the reviewer was right; fix the key

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (8 rows)

- Global::ExecuteCommandChangeFaction gog 0x00784680 -> 0x0066f0a0: reviewer corrected_addr
- Global::ExecuteCommandGetCreatureTalent gog 0x00790420 -> 0x0067ae40: reviewer corrected_addr
- Global::ExecuteCommandGetEmotions gog 0x00791a10 -> 0x0067c430: reviewer corrected_addr
- Global::ExecuteCommandGetMatchedStringsNum gog 0x00798d10 -> 0x00683740: reviewer corrected_addr
- Global::ExecuteCommandGetNearestObject gog 0x00799310 -> 0x00683d40: reviewer corrected_addr
- Global::ExecuteCommandIsAIState gog 0x007a0b00 -> 0x0068b4b0: reviewer corrected_addr
- Global::ExecuteCommandMoveToPoint gog 0x007a2530 -> 0x0068cee0: reviewer corrected_addr
- Global::ExecuteCommandShipBuild gog 0x007a8290 -> 0x00692c40: match cites a layout/offset difference

## Sample of the other reviewer matches (35 of 174)

- Global::ExecuteCommandGetFactionWeakestMember gog 0x00792c60 -> 0x0067d680 (C0)
- Global::ExecuteCommandEffectTemporaryHP gog 0x0078c5f0 -> 0x00677010 (C0)
- Global::ExecuteCommandDoSinglePlayerAutoSave gog 0x00786680 -> 0x006710a0 (C0)
- Global::ExecuteCommandGetGoldPieceValue gog 0x00793530 -> 0x0067df50 (C0)
- Global::ExecuteCommandGetGold gog 0x00793430 -> 0x0067de50 (C0)
- Global::ExecuteCommandEffectBlasterDeflectionIncrease gog 0x0078d130 -> 0x00677b50 (C0)
- Global::ExecuteCommandDeleteJournalWorldEntryStrref gog 0x00785c50 -> 0x00670670 (C0)
- Global::ExecuteCommandEffectACDecrease gog 0x00786fc0 -> 0x006719e0 (C0)
- Global::ExecuteCommandGetFactionStrongestMember gog 0x00792b50 -> 0x0067d570 (C0)
- Global::ExecuteCommandEffectAbilityDecrease gog 0x00786c40 -> 0x00671660 (C0)
- Global::ExecuteCommandEffectFactionModifier gog 0x00789b10 -> 0x00674530 (C0)
- Global::ExecuteCommandEffectBlasterDeflectionDecrease gog 0x0078d250 -> 0x00677c70 (C0)
- Global::ExecuteCommandGetFactionMostDamagedMember gog 0x00792960 -> 0x0067d380 (C0)
- Global::ExecuteCommandGetFactionAverageLevel gog 0x007920d0 -> 0x0067caf0 (C0)
- Global::ExecuteCommandGetStringUpperCase gog 0x0079e7c0 -> 0x00689170 (C0)
- Global::ExecuteCommandEffectVPRegenModifier gog 0x0078a0a0 -> 0x00674ac0 (C0)
- Global::ExecuteCommandGetStringLowerCase gog 0x0079e520 -> 0x00688ed0 (C0)
- Global::ExecuteCommandGetChemical gog 0x007935e0 -> 0x0067e000 (C0)
- Global::ExecuteCommandSetCurrentStealthXP gog 0x007ad080 -> 0x00697a30 (C0)
- CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints gog 0x0078c440 -> 0x00676e60 (C0)
- Global::ExecuteCommandGetChemicalPieceValue gog 0x007936e0 -> 0x0067e100 (C0)
- Global::ExecuteCommandGetMaxForcePoints gog 0x00794030 -> 0x0067ea50 (C0)
- Global::ExecuteCommandIntToString gog 0x007a0a10 -> 0x0068b3c0 (C0)
- Global::ExecuteCommandEffectAttackDecrease gog 0x00787700 -> 0x00672120 (C0)
- Global::ExecuteCommandEffectAttackIncrease gog 0x007878e0 -> 0x00672300 (C0)
- Global::ExecuteCommandEffectACIncrease gog 0x007871e0 -> 0x00671c00 (C0)
- Global::ExecuteCommandIsStealthed gog 0x007a1340 -> 0x0068bcf0 (C0)
- Global::ExecuteCommandEffectForceBody gog 0x00789c80 -> 0x006746a0 (C0)
- Global::ExecuteCommandGetDistanceToObject gog 0x00791000 -> 0x0067ba20 (C0)
- Global::ExecuteCommandGetIdentified gog 0x007940b0 -> 0x0067ead0 (C0)
- Global::ExecuteCommandGetStringRight gog 0x0079e650 -> 0x00689000 (C0)
- Global::ExecuteCommandEffectDamageReduction gog 0x007893f0 -> 0x00673e10 (C0)
- Global::ExecuteCommandPrintObject gog 0x007a4850 -> 0x0068f200 (C0)
- Global::ExecuteCommandEffectMovementSpeedDecrease gog 0x0078afc0 -> 0x006759e0 (C0)
- Global::ExecuteCommandGetFactionAverageGoodEvilAlignment gog 0x00791ff0 -> 0x0067ca10 (C0)

## AUTO meaning checks (5 sig + 3 c2)

- sig: Global::ExecuteCommandGetHitpoints gog 0x00793df0 -> 0x0067e810
- sig: Global::ExecuteCommandGetBattleTrack gog 0x0078fb00 -> 0x0067a520
- sig: Global::ExecuteCommandEquipItem gog 0x0078dce0 -> 0x00678700
- sig: Global::ExecuteCommandGetButtonMashCheck gog 0x007acda0 -> 0x00697750
- sig: Global::ExecuteCommandSetCreatureAILevel gog 0x00780c20 -> 0x0066b640
- c2: Global::ExecuteCommandResetCreatureAILevel gog 0x00780de0 -> 0x0066b800
- c2: Global::ExecuteCommandGiveXPToCreature gog 0x007a04b0 -> 0x0068ae60
- c2: Global::ExecuteCommandSWMG_SetLateralAccelPerSec gog 0x007fbee0 -> 0x006f8920
