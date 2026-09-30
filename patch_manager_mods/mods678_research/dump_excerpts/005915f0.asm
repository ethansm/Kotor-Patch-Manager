; FN 005915f0 FUN_005915f0
005915f0  55                    PUSH EBP
005915f1  8bec                  MOV EBP,ESP
005915f3  83ec0c                SUB ESP,0xc
005915f6  894df4                MOV dword ptr [EBP + -0xc],ECX
005915f9  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005915fc  8a88bc0a0000          MOV CL,byte ptr [EAX + 0xabc]
00591602  884dfb                MOV byte ptr [EBP + -0x5],CL
00591605  0fb655fb              MOVZX EDX,byte ptr [EBP + -0x5]
00591609  8955fc                MOV dword ptr [EBP + -0x4],EDX
0059160c  eb09                  JMP 0x00591617
0059160e  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00591611  83c001                ADD EAX,0x1
00591614  8945fc                MOV dword ptr [EBP + -0x4],EAX
00591617  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0059161a  e8410c0000            CALL 0x00592260
0059161f  0fb6c8                MOVZX ECX,AL
00591622  394dfc                CMP dword ptr [EBP + -0x4],ECX
00591625  7d28                  JGE 0x0059164f
00591627  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0059162a  52                    PUSH EDX
0059162b  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0059162e  e88df2ffff            CALL 0x005908c0
00591633  0fb74064              MOVZX EAX,word ptr [EAX + 0x64]
00591637  85c0                  TEST EAX,EAX
00591639  7412                  JZ 0x0059164d
0059163b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0059163e  51                    PUSH ECX
0059163f  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00591642  e879f2ffff            CALL 0x005908c0
00591647  33d2                  XOR EDX,EDX
00591649  66895064              MOV word ptr [EAX + 0x64],DX
0059164d  ebbf                  JMP 0x0059160e
0059164f  6a00                  PUSH 0x0
00591651  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00591654  81c1680a0000          ADD ECX,0xa68
0059165a  e8911d0000            CALL 0x005933f0
0059165f  8be5                  MOV ESP,EBP
00591661  5d                    POP EBP
00591662  c3                    RET
