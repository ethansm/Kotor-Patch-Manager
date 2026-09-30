; FN 00571570 FUN_00571570
00571570  55                    PUSH EBP
00571571  8bec                  MOV EBP,ESP
00571573  83ec34                SUB ESP,0x34
00571576  894dd0                MOV dword ptr [EBP + -0x30],ECX
00571579  d9ee                  FLDZ
0057157b  d95df8                FSTP float ptr [EBP + -0x8]
0057157e  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
00571581  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
00571587  894de4                MOV dword ptr [EBP + -0x1c],ECX
0057158a  6a10                  PUSH 0x10
0057158c  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0057158f  e88cf01500            CALL 0x006d0620
00571594  8945fc                MOV dword ptr [EBP + -0x4],EAX
00571597  837dfc00              CMP dword ptr [EBP + -0x4],0x0
0057159b  0f8498000000          JZ 0x00571639
005715a1  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005715a4  e887581600            CALL 0x006d6e30
005715a9  8945e0                MOV dword ptr [EBP + -0x20],EAX
005715ac  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
005715af  0fb6421a              MOVZX EAX,byte ptr [EDX + 0x1a]
005715b3  33c9                  XOR ECX,ECX
005715b5  85c0                  TEST EAX,EAX
005715b7  0f9fc1                SETG CL
005715ba  85c9                  TEST ECX,ECX
005715bc  7e7b                  JLE 0x00571639
005715be  837d1000              CMP dword ptr [EBP + 0x10],0x0
005715c2  740b                  JZ 0x005715cf
005715c4  d9056cbd9800          FLD float ptr [0x0098bd6c]
005715ca  d95df8                FSTP float ptr [EBP + -0x8]
005715cd  eb68                  JMP 0x00571637
005715cf  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
005715d2  e859581600            CALL 0x006d6e30
005715d7  8945dc                MOV dword ptr [EBP + -0x24],EAX
005715da  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
005715dd  d94234                FLD float ptr [EDX + 0x34]
005715e0  d95dd8                FSTP float ptr [EBP + -0x28]
005715e3  d945d8                FLD float ptr [EBP + -0x28]
005715e6  d95df8                FSTP float ptr [EBP + -0x8]
005715e9  d945f8                FLD float ptr [EBP + -0x8]
005715ec  dc1d386b9800          FCOMP double ptr [0x00986b38]
005715f2  dfe0                  FNSTSW AX
005715f4  f6c444                TEST AH,0x44
005715f7  7a09                  JP 0x00571602
005715f9  d905f0199900          FLD float ptr [0x009919f0]
005715ff  d95df8                FSTP float ptr [EBP + -0x8]
00571602  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
00571605  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
0057160b  894dd4                MOV dword ptr [EBP + -0x2c],ECX
0057160e  6a00                  PUSH 0x0
00571610  6a00                  PUSH 0x0
00571612  6a03                  PUSH 0x3
00571614  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
00571617  e894651400            CALL 0x006b7bb0
0057161c  8845f7                MOV byte ptr [EBP + -0x9],AL
0057161f  0fbe55f7              MOVSX EDX,byte ptr [EBP + -0x9]
00571623  85d2                  TEST EDX,EDX
00571625  7e10                  JLE 0x00571637
00571627  0fbe45f7              MOVSX EAX,byte ptr [EBP + -0x9]
0057162b  8945cc                MOV dword ptr [EBP + -0x34],EAX
0057162e  db45cc                FILD dword ptr [EBP + -0x34]
00571631  d845f8                FADD float ptr [EBP + -0x8]
00571634  d95df8                FSTP float ptr [EBP + -0x8]
00571637  eb54                  JMP 0x0057168d
00571639  837d0c00              CMP dword ptr [EBP + 0xc],0x0
0057163d  7435                  JZ 0x00571674
0057163f  d9ee                  FLDZ
00571641  d95de8                FSTP float ptr [EBP + -0x18]
00571644  d9ee                  FLDZ
00571646  d95dec                FSTP float ptr [EBP + -0x14]
00571649  d9ee                  FLDZ
0057164b  d95df0                FSTP float ptr [EBP + -0x10]
0057164e  6a00                  PUSH 0x0
00571650  6a00                  PUSH 0x0
00571652  8d4df8                LEA ECX,[EBP + -0x8]
00571655  51                    PUSH ECX
00571656  8d55e8                LEA EDX,[EBP + -0x18]
00571659  52                    PUSH EDX
0057165a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0057165d  50                    PUSH EAX
0057165e  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
00571661  e8aa480100            CALL 0x00585f10
00571666  d945f8                FLD float ptr [EBP + -0x8]
00571669  dc05f07d9800          FADD double ptr [0x00987df0]
0057166f  d95df8                FSTP float ptr [EBP + -0x8]
00571672  eb19                  JMP 0x0057168d
00571674  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00571677  51                    PUSH ECX
00571678  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0057167b  52                    PUSH EDX
0057167c  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
0057167f  e81c000000            CALL 0x005716a0
00571684  dc05087f9800          FADD double ptr [0x00987f08]
0057168a  d95df8                FSTP float ptr [EBP + -0x8]
0057168d  d945f8                FLD float ptr [EBP + -0x8]
00571690  8be5                  MOV ESP,EBP
00571692  5d                    POP EBP
00571693  c20c00                RET 0xc
