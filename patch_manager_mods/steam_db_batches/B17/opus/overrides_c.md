# B17 Opus gate part c (rows 43-62)

All 20 rows: GOG and Steam fp rows identical in size, retn, n_insn, disp list and imm list (callee lists map 1:1 by count).
Per-row anchor (Steam asm/decomp, steam_6a522e71):
- 43 HandleInputEvent 0x8bed80: CALL 0x7cac10 HideExamineBox, CALL 0x4122a0 PlayGuiSound. verified
- 44 ZoomCamera 0x7f44b0: 007f452a MOV [EDX+0x104],0x1 (reviewer anchor was set-level; asm checked). verified
- 45 AurGUIStartLayer 0x4792c0: 004792ca PUSH 0xba2 (GL_VIEWPORT). verified
- 46 GetSpell 0x6c1450: 006c1473 IMUL EAX,EAX,0x1a8. verified
- 47 Camera::Animate 0x480e40: 00480e4c CMP [EAX+0x208],0. verified
- 48 GetShadowLights 0x50a630: 0050a9fb ADD ECX,0xb8 / 0050a80e ADD ECX,0xac. verified
- 49 RenderLinked 0x4f3b40: 004f3d0a CMP [EAX+0x1cc],0; 8781 B unique twin. verified
- 50 ImageReadTGA 0x483490: PUSH 0x98c590 "%s.tga", SUB ESP,0x10c. verified
- 51 SkillsCharGen Destructor_2 0x90e440: PUSH 0x41a3d0 CSWGuiButton::Destructor, callee 0x418640 CSWGuiControl::Destructor. verified
- 52 Transpose 0x514a20: 00514a23 SUB ESP,0x74. verified
- 53 InitOpenGLWindow 0x408af0: MOV EAX,[0x986c00] 'Render Window', PUSH 0x2ca0000. verified
- 54 GetCameraParameters 0x8c2960: CALL 0x73f4d0 GetGameObject, CALL 0x7ed830 GetAnimationBase. verified
- 55 ReadUnmappedRLETGA 0x945990: AND EDX,0x7f / AND ECX,0x80. verified
- 56 HandleMouseOver 0x418750: MOV ECX,[EAX+0x48]; callee PlayGuiSound. verified
- 57 CERFFile::Read 0x720e30: PUSH 0x809, CExoString callees. verified
- 58 SetPartLocalPosition 0x462c50: MOV EAX,[EDX+0x10c]. verified
- 59 KeyMappings::Update 0x901590: MOV ECX,[EAX+0x1368]; PlayGuiSound callee. verified
- 60 StatusSummary ctor 0x75cec0: "statussummary_p", "LBL_JOURNAL". verified
- 61 PlayerUpdate 0x63f550: CALL 0x83ea60 CExoArrayList<uint32>::Add, IsForceUserClass callee. verified
- 62 ExportFilesFromERF 0x721ef0: PUSH 0x809, AND [EBP-0x64],0xfffffffe. verified

Reviewer mismatch rows (c5 0x8bf0e0, c1 0x7f3de0/0x7f4610, c1 0x6c13e0, c7 0x50aa90/0x508f70, c9 0x4f5d90, c14 0x5147e0/0x514b90, c7 0x4087a0/0x408c30) are decoys/alternates, correctly rejected.

## AUTO meaning checks
| # | route | result |
|---|---|---|
| 55 | sig | pass |
| 56 | sig | pass |
| 57 | sig | pass |
| 58 | sig | pass |
| 59 | sig | pass |
| 60 | c2 | pass |
| 61 | c2 | pass |
| 62 | c2 | pass |

sample_errors: 0
