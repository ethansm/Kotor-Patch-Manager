# Opus gate B10a (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 20 (per chunk {'1': 10, '2': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (9 rows)

- CSWCObject::AnimationPlayOutOfPlace gog 0x004744b0 -> 0x007ef730: reviewer corrected_addr, label_suspect
- CSWCObject::AnimationHideEquippedItems gog 0x00474540 -> 0x007ef910: reviewer corrected_addr, label_suspect
- CSWCObject::AnimationWalking gog 0x004745d0 -> 0x007ef3e0: reviewer corrected_addr, label_suspect
- CSWCObject::AnimationPause gog 0x00474890 -> 0x007ef350: reviewer corrected_addr, label_suspect
- CSWCObject::AnimationRunning gog 0x00474a70 -> 0x007ef470: reviewer corrected_addr, label_suspect
- CSWSObject::GetActionByGroupId gog 0x006a60a0 -> 0x0053f3c0: match on a mac_rotation row
- CSWSObject::RemoveObjectFromDialog gog 0x006ad780 -> 0x00546aa0: label_suspect
- CSWSObject::AddMatchedExpressionString gog 0x006adc90 -> 0x00546fb0: match cites a layout/offset difference
- CSWSObject::GetAQActionIDByID gog 0x006b3410 -> 0x005d7220: match on a mac_rotation row

## Sample of the other reviewer matches (13 of 61)

- CSWCObject::Destructor gog 0x00472720 -> 0x007ed5c0 (C0)
- CSWCObject::StopAllVisualEffects gog 0x00475b50 -> 0x007f09f0 (C0)
- CSWCObject::ClientServerSyncHandler_Orientation gog 0x004726f0 -> 0x007ed590 (C0)
- CSWCObject::RemoveAllFireForgetAnimations gog 0x00475420 -> 0x007f02c0 (C0)
- CSWCObject::GetPortraitId gog 0x00472610 -> 0x007ed4b0 (C0)
- CSWCObject::TurnOffShadows gog 0x00472660 -> 0x007ed500 (C0)
- CSWCObject::TurnOnShadows gog 0x00472630 -> 0x007ed4d0 (C0)
- CSWCObject::GetPortrait gog 0x004725e0 -> 0x007ed480 (C0)
- CSWSObject::IsDirtyFlagSet gog 0x006b1160 -> 0x0054a480 (C0)
- CSWCObject::SetId gog 0x00410c30 -> 0x00764f60 (C0)
- CSWCObject::DisableDistortion gog 0x00477820 -> 0x007f2680 (C0)
- CSWCObject::GetCanSeeHeight gog 0x00410cb0 -> 0x00764fe0 (C0)
- CSWSObject::GetPortraitId gog 0x006638a0 -> 0x0053f000 (C0)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWSObject::AIActionSetCommandable gog 0x006bd060 -> 0x005e0e70
- sig: CSWCObject::DeleteVisualEffect gog 0x00475a80 -> 0x007f0920
- sig: CSWSObject::DoSpellImmunity gog 0x006aa7c0 -> 0x00543ae0
- sig: CSWCObject::ReplaceTexture_2 gog 0x00472a30 -> 0x007ed8d0
- sig: CSWCObject::AnimationOverlay gog 0x00474800 -> 0x007ef6a0
- c2: CSWSObject::ReportOverflow gog 0x006b0c50 -> 0x00549f70
- c2: CSWSObject::SaveActionQueue gog 0x006a70f0 -> 0x00540410
- c2: CSWCObject::SetFeedbackInfo gog 0x00477360 -> 0x007f2200
