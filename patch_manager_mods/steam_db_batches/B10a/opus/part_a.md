# Opus gate B10a part a (1 of 1)

- batch: B10a, part: a, rows: 30 (rows 1-30 of 30 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWCObject::AnimationPlayOutOfPlace | 0x004744b0 | 0x007ef730 | reviewer corrected_addr, label_suspect |
| 2 | 100% | CSWCObject::AnimationHideEquippedItems | 0x00474540 | 0x007ef910 | reviewer corrected_addr, label_suspect |
| 3 | 100% | CSWCObject::AnimationWalking | 0x004745d0 | 0x007ef3e0 | reviewer corrected_addr, label_suspect |
| 4 | 100% | CSWCObject::AnimationPause | 0x00474890 | 0x007ef350 | reviewer corrected_addr, label_suspect |
| 5 | 100% | CSWCObject::AnimationRunning | 0x00474a70 | 0x007ef470 | reviewer corrected_addr, label_suspect |
| 6 | 100% | CSWSObject::GetActionByGroupId | 0x006a60a0 | 0x0053f3c0 | match on a mac_rotation row |
| 7 | 100% | CSWSObject::RemoveObjectFromDialog | 0x006ad780 | 0x00546aa0 | label_suspect |
| 8 | 100% | CSWSObject::AddMatchedExpressionString | 0x006adc90 | 0x00546fb0 | match cites a layout/offset difference |
| 9 | 100% | CSWSObject::GetAQActionIDByID | 0x006b3410 | 0x005d7220 | match on a mac_rotation row |
| 10 | sample | CSWCObject::Destructor | 0x00472720 | 0x007ed5c0 | C0 |
| 11 | sample | CSWCObject::StopAllVisualEffects | 0x00475b50 | 0x007f09f0 | C0 |
| 12 | sample | CSWCObject::ClientServerSyncHandler_Orientation | 0x004726f0 | 0x007ed590 | C0 |
| 13 | sample | CSWCObject::RemoveAllFireForgetAnimations | 0x00475420 | 0x007f02c0 | C0 |
| 14 | sample | CSWCObject::GetPortraitId | 0x00472610 | 0x007ed4b0 | C0 |
| 15 | sample | CSWCObject::TurnOffShadows | 0x00472660 | 0x007ed500 | C0 |
| 16 | sample | CSWCObject::TurnOnShadows | 0x00472630 | 0x007ed4d0 | C0 |
| 17 | sample | CSWCObject::GetPortrait | 0x004725e0 | 0x007ed480 | C0 |
| 18 | sample | CSWSObject::IsDirtyFlagSet | 0x006b1160 | 0x0054a480 | C0 |
| 19 | sample | CSWCObject::SetId | 0x00410c30 | 0x00764f60 | C0 |
| 20 | sample | CSWCObject::DisableDistortion | 0x00477820 | 0x007f2680 | C0 |
| 21 | sample | CSWCObject::GetCanSeeHeight | 0x00410cb0 | 0x00764fe0 | C0 |
| 22 | sample | CSWSObject::GetPortraitId | 0x006638a0 | 0x0053f000 | C0 |
| 23 | AUTO sig | CSWSObject::AIActionSetCommandable | 0x006bd060 | 0x005e0e70 |  |
| 24 | AUTO sig | CSWCObject::DeleteVisualEffect | 0x00475a80 | 0x007f0920 |  |
| 25 | AUTO sig | CSWSObject::DoSpellImmunity | 0x006aa7c0 | 0x00543ae0 |  |
| 26 | AUTO sig | CSWCObject::ReplaceTexture_2 | 0x00472a30 | 0x007ed8d0 |  |
| 27 | AUTO sig | CSWCObject::AnimationOverlay | 0x00474800 | 0x007ef6a0 |  |
| 28 | AUTO c2 | CSWSObject::ReportOverflow | 0x006b0c50 | 0x00549f70 |  |
| 29 | AUTO c2 | CSWSObject::SaveActionQueue | 0x006a70f0 | 0x00540410 |  |
| 30 | AUTO c2 | CSWCObject::SetFeedbackInfo | 0x00477360 | 0x007f2200 |  |
