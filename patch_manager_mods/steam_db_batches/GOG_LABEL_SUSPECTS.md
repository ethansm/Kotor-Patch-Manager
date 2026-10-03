# GOG DB label suspects (for the user; the GOG DB is never edited without approval)

Rule (user, 2026-10-03): the PR changes only the Steam DB. GOG label bugs are recorded here and proposed only when
confirmed. Evidence comes from:
- the GOG binary on the OpenKotOR server (`fp.tsv`: callees, displacements and immediates of the function at the GOG
  address);
- the Mac x86 build's real symbol names (Mach-O symbol table) with its fingerprints;
- the Steam counterpart's decompile.

Rows were confirmed by the Opus calibration gate (`calibration/OPUS_REVIEW.md` section 5 and 4a-4c, 2026-10-03).

How the Steam DB handles them: a row whose Steam body matches the GOG address but whose GOG name contradicts the body
is `label_suspect` and is not shipped under the GOG name. When the Steam function for a name is proven by meaning
(Mac symbol + Steam anchor), the Steam row ships under that name with an Opus override.

## Confirmed
| # | GOG addr | GOG DB / server name | what the GOG body is (evidence) | Steam counterpart of this GOG body | real GOG addr of the name -> Steam |
|---|---|---|---|---|---|
| 1 | 0x4E0C60 | `CGuiInGame::GetMiniMapVisible` | `ShowDialogMessage`: 49 B; calls `vt+0x84` on the `[+0x3C]` panel when `[+0x3C]==[+0x40]` (`007d0570 CMP EDX,[ECX+0x40]`, `JNZ` skips); Mac `ShowDialogMessage` 0x268B38, disp 0x3C,0x40,0x88 (Mac vtable +4) | 0x7D0560 | `GetMiniMapVisible` = GOG 0x4E0CA0 -> Steam 0x7D05A0 |
| 2 | 0x4E0CA0 | `CGuiInGame::RemoveGUIPopUps` | `GetMiniMapVisible`: calls `GetClientOptions` 0x40CCF0 + `GetMiniGame` 0x40D580, reads the minimap bit at `+0x59B4`; Mac `GetMiniMapVisible` same callees, disp 0x98,0x58E4 | 0x7D05A0 | `RemoveGUIPopUps` = GOG 0x4E0D20 -> Steam 0x7D0620 |
| 3 | 0x4E0D20 | `CGuiInGame::ResetInterfaceFont` | `RemoveGUIPopUps` (Mac shape: disp 0x38,0x40,0x54,0x94,0xA0,0xA4,0xB0, one callee) | 0x7D0620 | to locate |
| 4 | 0x4E0BF0 | `CGuiInGame::ShowDialogMessage` (rotation) | fits Mac `GetInterfaceOffset` (disp 0x98,0xA900 vs Mac 0x98,0xA770) | to map | - |
| 5 | 0x4DBDE0 | `CGuiInGame::IsAnimationPlayingInDialog` | `DialogGuiDoneScroll`: `007cb6f3 MOV [ECX+0xfb4],1`, then `vt+0x84` on `[+0x3C]`; Mac `DialogGuiDoneScroll` 0x26563C, disp 0x3C,0x64,0x88,0xFB4 | 0x7CB6E0 | `IsAnimationPlayingInDialog` = GOG 0x4DB4B0 -> Steam 0x7CADB0 |
| 6 | 0x4DB4B0 | `CGuiInGame::DestroyDeathMenuGUI` | `IsAnimationPlayingInDialog` (`007cae2e CALL 0x007ef780` AnimationDialog; Mac disp 0x1B8/0x1C0) | 0x7CADB0 | to locate |
| 7 | 0x4DF1F0 | `CGuiInGame::GetCanOrientInDialog` | a Vector3 getter at `[[this+0x9C]+0x440..0x448]` (`007ceb28 ADD EDX,0x440`) | 0x7CEAF0 | `GetCanOrientInDialog` = GOG 0x4DFE10 -> Steam 0x7CF710 (`ADD ECX,0xf24` list walk, imm 0x7F000000; Mac 0x26161C) |
| 8 | 0x4DFE10 | `CGuiInGame::IsOverlayDialogAnimation` (duplicate of the label at 0x4DB950) | `GetCanOrientInDialog` | 0x7CF710 | - |
| 9 | 0x474890 | `CSWCObject::AnimationPause` | `AnimationPlayOutOfPlace`: pure id-range test (imm 0x4B0,0x52F,0x640,0x6BF); Mac `AnimationPlayOutOfPlace` | 0x7EF730 | `AnimationPause` = GOG 0x4744B0 -> Steam 0x7EF350 (uses the "Pause" column handle: `006eba0a PUSH 0x99af6c` "Pause") |
| 10 | 0x4744B0 | `CSWCObject::AnimationPlayOutOfPlace` | `AnimationPause` (swapped pair with #9) | 0x7EF350 | `AnimationPlayOutOfPlace` = GOG 0x474890 -> Steam 0x7EF730 |

## Unverified
| GOG addr | GOG name | hint |
|---|---|---|
| 0x4DBE50 | `CGuiInGame::IsFadingInDialog` | disp 0x3C,0x40 fits Mac `IsCameraDialog` |

## Impact on GOG today
- KPM GameAPI's `CGuiInGame::GetMiniMapVisible` wrapper (resolved in AdditionalConsoleCommands' init) calls
  `ShowDialogMessage` on GOG: a function with a side effect, not a predicate.
- `RemoveGUIPopUps` on GOG resolves to the minimap predicate (no effect).
- CGuiInGame labels between ShowDialogMessage and ResetInterfaceFont are a **rotation** (Mac order: SetCombatMessage,
  ShowDialogMessage, GetMiniMapVisible, RemoveGUIPopUps), not isolated swaps.

Found by: Steam DB calibration (chunks 1-2, 2026-10-03); confirmed by the Opus calibration gate.
