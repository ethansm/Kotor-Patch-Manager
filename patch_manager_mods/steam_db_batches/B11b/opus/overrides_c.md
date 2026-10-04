# B11b Opus gate, part c (9 rows)

Method: GOG/Steam fp.tsv compared field by field (size, retn, n_insn, disp, imm, callee sizes); Steam callee names
from the Steam address DB (read-only); whole-binary exact-fp uniqueness on Steam (all 9 unique, 1 exact and 1 loose
hit each); Mac x86 shapes checked; Steam asm quoted from steam_6a522e71 dump.

| # | GOG | Steam | verdict | evidence |
|---|---|---|---|---|
| 121 | 0x523d50 SelectPrevPersonalAction | 0x7525d0 | verified | `00752747 CALL 0x004122a0` (PlayGuiSound, arg 7); decomp: index-1, wraps to count-1 = Prev. sz387 ret4, imm 0x2c = GOG; Next is 0x752450 (sz371, imm 0x28). Mac shapes Prev/Next agree. |
| 122 | 0x4ddbd0 SetDialogSelection | 0x7cd4d0 | verified | `007cd4dd MOV [EAX+0xe0],ECX` ... `007cd4f5 MOV [ECX+0xec],EDX`, `RET 0xc`; Mac SetDialogSelection disp 0xe0-0xec. |
| 123 | 0x4dd740 UpdateMessageGUI | 0x7cd040 | verified | 3 calls to 0x758e30/0x7592f0/0x759820, sizes 1197/1319/1262 ret8 = GOG CSWGuiInGameMessages::AddMessages/AddDialogMessages/AddCombatMessages (uniform delta 0x22e900). |
| 124 | 0x4d46f0 LoadCameraModelForDialog | 0x7c4060 | verified | `007c4090 CALL 0x0073f430` (GetModule), `007c40a5 CALL 0x007f66e0` (sz163 ret4 = LoadAnimatedCamera), `007c40ad MOV [ECX+0xf08],1`. |
| 125 | 0x4d8a80 LoadStuntModelsForDialog | 0x7c8390 | verified | `007c83f8 CALL 0x0073f4d0` (GetGameObject), `007c843a CALL 0x00710980` (sz38 = CResRef::CopyToString); disp 0x1ac/0x1b0/0x1b4 = GOG and Mac. |
| 126 | 0x4dc660 HandleDialogEntry | 0x7cbf60 | verified | `007cc521 RET 0x5c` = GOG retn 92 (the GOG DB param 0 is wrong, row carries 92); calls ShowDialogEntry 0x7bfb10 (sz428) and AppendToDialogBuffer 0x7be2d0 (sz226); `007cc4d4 PUSH 0x106d`. |
| 127 | 0x4ce720 AppendToMsgBuffer | 0x7be090 | verified | `007be099 PUSH 0x986925` (empty string; GOG 0x9858f5 is also "") + `CALL 0x00733b60` (CExoString operator!=); `007be0bd MOV [ECX+0x11c],0x3f` (ring wrap, Mac imm 0x3f). |
| 128 | 0x4e1490 GetFadeOverridePanelVisible | 0x7d0d90 | verified | `007d0e9d CALL 0x00740f20` (GetPazaakIsRunning) + 4x `CALL 0x00410800` (PanelExists, sz35); `CMP [EAX+0x128],0`. |
| 129 | 0x4e2730 DisplayDeathMessage | 0x7d2030 | verified | `007d2064 PUSH 0x724` + `CALL 0x00919723` (operator new), `007d208b CALL 0x00760210` (sz724 = CSWGuiDeathDisplay ctor), `007d20e0 CALL 0x00410530` (AddPanel). |

None of the 9 is a forwarder or a mid-function fragment, and none sits on a permuted label: every body fits its name.

## AUTO meaning checks

| row | check | result |
|---|---|---|
| SetDialogSelection | sig | pass |
| UpdateMessageGUI | sig | pass |
| LoadCameraModelForDialog | sig | pass |
| LoadStuntModelsForDialog | sig | pass |
| HandleDialogEntry | sig | pass |
| AppendToMsgBuffer | c2 | pass |
| GetFadeOverridePanelVisible | c2 | pass |
| DisplayDeathMessage | c2 | pass |

sample_errors: 0
