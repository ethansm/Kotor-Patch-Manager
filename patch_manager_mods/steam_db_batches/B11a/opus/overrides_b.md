# B11a Opus gate, part b (rows 61-120)

Gate: Opus. Inputs: evidence.tsv, review/sonnet_c1..c5.tsv, cards/key.tsv, GOG/Steam/Mac fp.tsv, Steam asm dumps.
Method: for every row, compare the GOG fp with the Steam fp field by field (size, retn, n_insn, disp, imm), then the
callee multiset two levels deep ("deep2"), then the caller set by (size, retn). For forwarders, the anchor is the
forwarded callee. The callee's own fp must equal the GOG internal body, its deep2 signature must be unique in Steam
(1 hit), and its callers must match. The Get*ByGameObjectID slots were matched by exact vtable displacement.
Scratch scripts are in the session scratchpad and are not committed.

Counts: verified 56, likely 1, label_suspect 3 (2 of them are address corrections), rejected 0, unresolved 0.

## 100% rows (61-66)

| # | GOG | Steam (final) | verdict | evidence |
|---|---|---|---|---|
| 61 | 0x43b150 Int::GetObjectName | 0x78eba0 | verified | fp identical (sz1424 ret8 n347, disps incl. 0x10c/0x310, imm 0x58d). `0078ec19 CALL 0x0053dfb0` (GetGameObject sz167 = GOG 0x469c20). `0078efa4 CALL 0x007fdfa0` (sz618 = CSWCItem::GetName 0x47f250). The forwarder 0x73f0e0 calls it. The reviewer's correction was right; the original candidate 0x78e350 is an ignore-list adder. |
| 62 | 0x444b60 "IndicateObject" | 0x798540 | label_suspect | fp identical (sz63). `00798572 CALL 0x00410800` (PanelExists sz35) and `0079854a CMP [EAX+0x278]` / `00798556 CMP [ECX+0x274]`. Mac `CClientExoAppInternal::GetLoadScreenVisible` 0x1a3ac4 has disps 0x274/0x278 plus one GUI callee. The callers are the GOG forwarder `CClientExoApp::GetLoadScreenVisible` 0x40d480 and UpdateRumble, both mirrored in Steam. **Label rotation:** this body is Int::GetLoadScreenVisible. The real Int::IndicateObject is the target of the GOG forwarder 0x40d6b0, which is GOG 0x443170. The GOG label "Int::GetLoadScreenVisible" at 0x44dff0 (sz64) is suspect in turn. |
| 63 | 0x44aa20 Int::SetAutoPaused | 0x79e420 | verified | fp identical (sz296 ret8). `0079e50c CALL 0x0079bf40` (sz275 ret12 = SetPausedByCombat 0x448540). `0079e42c MOV ECX,[EAX+0x384]`. The callers match. The reviewer's correction was right. |
| 64 | 0x44bd70 Int::ChangeCharacterToNextLivingPartyMember | 0x79f770 | verified | fp identical (sz715 ret12). `0079f8c0 CALL 0x00797f30` (sz355 = Int::ChangeCharacter 0x444550). `0079fa1a CALL 0x0073fb90` (GetSWParty). `0079f851 CMP [EAX+0x11ac],0`. The reviewer's correction was right; 0x79f010 is LookUpAndPerformRumbleWithCutOff. |
| 65 | 0x44deb0 "GetPausedByCombat" | **0x5936d0** (was 0x7a1b90) | label_suspect | The pipeline's 0x7a1b90 is a CMemoryDC dtor (sz86) and is wrong. GOG is sz21 with a single callee, CExoLinkedListInternal::RemoveTail 0x60c210. Steam 0x5936d0 is sz21: `005936dc CALL 0x0073a2a0`, the unique deep2 match of RemoveTail (sz156, with strings). The callers match: SetCaptureEventPlayBack 586 = 0x785b50, CSWSCombatRound::RemoveLastAction 147 = 0x5904c0. The body is a folded `CExoLinkedList<T>::RemoveTail` wrapper, not a pause getter. Mac GetPausedByCombat (0x195cd4, sz45) has a different shape. |
| 66 | 0x44ded0 "InitializeMoviePlayer_2" | **0x7a17a0** (was 0x7a1130) | label_suspect | The pipeline's 0x7a1130 is wrong: sz397, `PUSH 0x61c` alloc plus a 0x892090 panel ctor (credits panel). GOG and Steam 0x7a17a0 are both sz113 ret8 with no callees and identical disps (0x4,0x4,0x4,0x8,0x8) and imms (0x8,0x8,0xc,0xc,0x10). The deep2 match is unique. `007a17f7 CALL dword ptr [EBP+0xc]` sits in a 0xc-stride loop and `007a180e RET 0x8`. The callers match: SelectNearestObject 1336 = 0x79b700, HandleMouseOver 667 = 0x7b3a00. The body is a generic array find-with-callback, not movie init. |

## Sample rows (67-120): all 54 reviewer `match` rows are confirmed

All 54 Steam picks have a GOG-identical top fp, apart from the noted body growth in callees.
- 50 have a unique deep2 signature in Steam (1 hit, the pick).
- The other 4 have no exact deep2 hit only because of str_ref/CRT noise or Steam body growth, which was checked by hand:
  - 0x78e250, 0x78d100: their callee sets are identical by size.
  - 0x73fad0 (StartServices): 65 callees, ret20 on both sides; the body is +208 B in Steam.
  - 0x79f6c0: calls CClientOptions::SaveOptions, which is +163 B in Steam with the same 14 callees.

Each forwarder's callee was checked against the GOG internal body (`pair.py`: SAME, or a DIFF only in str_ref
addresses or Steam growth).

Symmetric pairs, with the distinguishing anchor on both sides:
- **HideMouse/ShowMouse:** 0x740f80 → 0x7b38d0 (sz92 = GOG HideMouse 0x4604e0). ShowMouse is 0x7b3930 (sz197 = GOG 0x460540).
- **PerformRButtonDown/Up:** 0x7b3810 → 0x413350 (sz104 = HandleRMouseDown 0x9117d0). 0x7b3830 → 0x4133c0 (sz84 = HandleRMouseUp 0x911840).
- **Add/RemoveObjectToHitcheckIgnoreList:**
  - Add 0x78d100 calls new 0x919723 + AddTail 0x54aa30 (GOG: operator_new + AddTail).
  - Remove 0x78d190: `0078d1de CALL 0x0064dde0` (Remove) + `0078d1ed CALL 0x009196fd` (free) (GOG: Remove + _free).
- **MovieWindowActivated/Deactivated:**
  - Deactivated is Int 0x7a10f0 sz30: `007a1105 CALL 0x0070bc60` (SetSoundMode wrapper sz39, = GOG 0x63ac00).
  - Activated is Int 0x7a1110 sz22 → 0x70bbe0 (sz29 = ResumeMovieStreamingSounds). The App forwarder 0x741270 calls 0x7a1110.
- **SaveOptions/LoadOptions (App):** 0x740b40 ret0 → 0x79f6c0 (sz39 = Int::SaveOptions). 0x740b60 ret4 → 0x79f6f0 (sz114 = Int::LoadOptions).

Get*ByGameObjectID slots (exact disp from asm vs GOG fp):
- Trigger: 0x73f590 → 0x78bed0, `0078bf27 MOV EAX,[EDX+0x3c]` = GOG 0x4384b0 disp 0x3c.
- Projectile: 0x73f5b0 → 0x78bf40, `0078bf97 MOV EAX,[EDX+0x44]` = GOG 0x438520 disp 0x44.
- AoE: 0x73f610 → 0x78c090, `0078c0e7 MOV EAX,[EDX+0x54]` = GOG 0x438670 disp 0x54.
- GetPlayerByPlayerID: 0x73f7d0 → 0x78bcc0 (sz93 = GOG 0x4382a0, unique, callers match). This is consistent with 0x78bbc0 being the dtor, not a getter.

Region 0x4381a0-0x439570 (shift-by-one trap): every internal reached from my forwarders resolves to the Steam body
whose fp equals the GOG body:
- 0x438910 → 0x78c330, not the pipeline's 0x78bc50 seen in part a's list (part a's concern, noted here).
- 0x4382a0 → 0x78bcc0.
- 0x439530 → 0x78cf50.
- 0x4397f0 → 0x78d220.

The one `likely`: row 117 GetPlayerList 0x73f7b0 is a sz20 getter (`0073f7ba MOV EAX,[EAX+0x4]`, `0073f7bd ADD EAX,0x1c`
= GOG disp 0x4 imm 0x1c). It is unique and its callers match, but it is a thin stub.

The caller-set differences seen were all explained:
- Ghidra chunk splits (CSWParty::SetLeader 1230 vs 2168, CSWCCreature::SetCombatMode split).
- Steam growth (WinMain 3581→3611, CSWGuiMainMenu 3733→3814).

None contradicts a pick.

Pipeline/reviewer disagreements inside the sample were all decoy-style wrong neighbours, which the reviewers rejected:
- c2 0x73f6f0, c5 0x740f40/0x740fa0, c1 0x73fe40/0x73fc30, c1 0x740fc0/0x740f20, c5 0x73f6f0, c2 0x79f430.

All of these neighbours are confirmed wrong by the forwarded-callee identity above.

## AUTO meaning checks

None in this part (rows 241-248 are in a later part).

sample_errors: 0
