; FN 00615cc0 FUN_00615cc0
00615cc0  55                    PUSH EBP
00615cc1  8bec                  MOV EBP,ESP
00615cc3  83ec0c                SUB ESP,0xc
00615cc6  894df4                MOV dword ptr [EBP + -0xc],ECX
00615cc9  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00615ccc  e85f110c00            CALL 0x006d6e30
00615cd1  8945fc                MOV dword ptr [EBP + -0x4],EAX
00615cd4  837dfc00              CMP dword ptr [EBP + -0x4],0x0
00615cd8  7512                  JNZ 0x00615cec
00615cda  6825699800            PUSH 0x986925
00615cdf  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00615ce2  e859ab0f00            CALL 0x00710840
00615ce7  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615cea  eb61                  JMP 0x00615d4d
00615cec  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00615cef  8a4818                MOV CL,byte ptr [EAX + 0x18]
00615cf2  884dfb                MOV byte ptr [EBP + -0x5],CL
00615cf5  0fb655fb              MOVZX EDX,byte ptr [EBP + -0x5]
00615cf9  85d2                  TEST EDX,EDX
00615cfb  742a                  JZ 0x00615d27
00615cfd  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00615d00  8a88a6020000          MOV CL,byte ptr [EAX + 0x2a6]
00615d06  884dfa                MOV byte ptr [EBP + -0x6],CL
00615d09  6a6d                  PUSH 0x6d
00615d0b  660fb655fa            MOVZX DX,byte ptr [EBP + -0x6]
00615d10  0fb7c2                MOVZX EAX,DX
00615d13  50                    PUSH EAX
00615d14  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00615d17  51                    PUSH ECX
00615d18  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00615d1b  e840f10b00            CALL 0x006d4e60
00615d20  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615d23  eb28                  JMP 0x00615d4d
00615d27  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00615d2a  8a82a4020000          MOV AL,byte ptr [EDX + 0x2a4]
00615d30  8845f9                MOV byte ptr [EBP + -0x7],AL
00615d33  6a6d                  PUSH 0x6d
00615d35  660fb64df9            MOVZX CX,byte ptr [EBP + -0x7]
00615d3a  0fb7d1                MOVZX EDX,CX
00615d3d  52                    PUSH EDX
00615d3e  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615d41  50                    PUSH EAX
00615d42  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00615d45  e816f10b00            CALL 0x006d4e60
00615d4a  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00615d4d  8be5                  MOV ESP,EBP
00615d4f  5d                    POP EBP
00615d50  c20400                RET 0x4
