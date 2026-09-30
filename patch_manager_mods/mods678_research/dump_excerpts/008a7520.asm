; FN 008a7520 FUN_008a7520
008a7520  55                    PUSH EBP
008a7521  8bec                  MOV EBP,ESP
008a7523  83ec18                SUB ESP,0x18
008a7526  894dec                MOV dword ptr [EBP + -0x14],ECX
008a7529  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a752c  81c188050000          ADD ECX,0x588
008a7532  e8595db7ff            CALL 0x0041d290
008a7537  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008a753e  eb09                  JMP 0x008a7549
008a7540  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008a7543  83c001                ADD EAX,0x1
008a7546  8945fc                MOV dword ptr [EBP + -0x4],EAX
008a7549  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a754c  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a754f  3b91b4240000          CMP EDX,dword ptr [ECX + 0x24b4]
008a7555  7d4d                  JGE 0x008a75a4
008a7557  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a755a  8b88b0240000          MOV ECX,dword ptr [EAX + 0x24b0]
008a7560  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a7563  8b0491                MOV EAX,dword ptr [ECX + EDX*0x4]
008a7566  8945f4                MOV dword ptr [EBP + -0xc],EAX
008a7569  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008a756c  894df8                MOV dword ptr [EBP + -0x8],ECX
008a756f  837df800              CMP dword ptr [EBP + -0x8],0x0
008a7573  7413                  JZ 0x008a7588
008a7575  6a01                  PUSH 0x1
008a7577  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
008a757a  8b02                  MOV EAX,dword ptr [EDX]
008a757c  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
008a757f  8b10                  MOV EDX,dword ptr [EAX]
008a7581  ffd2                  CALL EDX
008a7583  8945e8                MOV dword ptr [EBP + -0x18],EAX
008a7586  eb07                  JMP 0x008a758f
008a7588  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
008a758f  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008a7592  8b88b0240000          MOV ECX,dword ptr [EAX + 0x24b0]
008a7598  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008a759b  c7049100000000        MOV dword ptr [ECX + EDX*0x4],0x0
008a75a2  eb9c                  JMP 0x008a7540
008a75a4  6a01                  PUSH 0x1
008a75a6  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
008a75a9  81c1b0240000          ADD ECX,0x24b0
008a75af  e82cf4d6ff            CALL 0x006169e0
008a75b4  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008a75b9  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008a75bc  894df0                MOV dword ptr [EBP + -0x10],ECX
008a75bf  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
008a75c2  81c2bc240000          ADD EDX,0x24bc
008a75c8  52                    PUSH EDX
008a75c9  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008a75cc  e8ff52c7ff            CALL 0x0051c8d0
008a75d1  8bc8                  MOV ECX,EAX
008a75d3  e85804d5ff            CALL 0x005f7a30
008a75d8  8bc8                  MOV ECX,EAX
008a75da  e861e9d5ff            CALL 0x00605f40
008a75df  8be5                  MOV ESP,EBP
008a75e1  5d                    POP EBP
008a75e2  c3                    RET
