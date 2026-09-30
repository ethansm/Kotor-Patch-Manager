; FN 006dab20 FUN_006dab20
006dab20  55                    PUSH EBP
006dab21  8bec                  MOV EBP,ESP
006dab23  83ec08                SUB ESP,0x8
006dab26  894df8                MOV dword ptr [EBP + -0x8],ECX
006dab29  c7450801000000        MOV dword ptr [EBP + 0x8],0x1
006dab30  837d0800              CMP dword ptr [EBP + 0x8],0x0
006dab34  740f                  JZ 0x006dab45
006dab36  6a00                  PUSH 0x0
006dab38  680000007f            PUSH 0x7f000000
006dab3d  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006dab40  e82b28eaff            CALL 0x0057d370
006dab45  6810270000            PUSH 0x2710
006dab4a  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
006dab4d  8b10                  MOV EDX,dword ptr [EAX]
006dab4f  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006dab52  8b427c                MOV EAX,dword ptr [EDX + 0x7c]
006dab55  ffd0                  CALL EAX
006dab57  6a01                  PUSH 0x1
006dab59  6a00                  PUSH 0x0
006dab5b  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006dab5e  e87dcfedff            CALL 0x005b7ae0
006dab63  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
006dab66  8b91dc100000          MOV EDX,dword ptr [ECX + 0x10dc]
006dab6c  8955fc                MOV dword ptr [EBP + -0x4],EDX
006dab6f  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
006dab72  e8796aebff            CALL 0x005915f0
006dab77  8be5                  MOV ESP,EBP
006dab79  5d                    POP EBP
006dab7a  c20400                RET 0x4
