; FN 005bda40 FUN_005bda40
005bda40  55                    PUSH EBP
005bda41  8bec                  MOV EBP,ESP
005bda43  83ec38                SUB ESP,0x38
005bda46  894dc8                MOV dword ptr [EBP + -0x38],ECX
005bda49  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005bda4c  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005bda4f  894df4                MOV dword ptr [EBP + -0xc],ECX
005bda52  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
005bda58  8955f0                MOV dword ptr [EBP + -0x10],EDX
005bda5b  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
005bda5e  50                    PUSH EAX
005bda5f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
005bda62  8b4934                MOV ECX,dword ptr [ECX + 0x34]
005bda65  e896771100            CALL 0x006d5200
005bda6a  8945ec                MOV dword ptr [EBP + -0x14],EAX
005bda6d  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
005bda70  33c0                  XOR EAX,EAX
005bda72  837a0400              CMP dword ptr [EDX + 0x4],0x0
005bda76  0f95c0                SETNZ AL
005bda79  85c0                  TEST EAX,EAX
005bda7b  7507                  JNZ 0x005bda84
005bda7d  33c0                  XOR EAX,EAX
005bda7f  e9cc000000            JMP 0x005bdb50
005bda84  8b4dc8                MOV ECX,dword ptr [EBP + -0x38]
005bda87  8b9198110000          MOV EDX,dword ptr [ECX + 0x1198]
005bda8d  8955e8                MOV dword ptr [EBP + -0x18],EDX
005bda90  6a5d                  PUSH 0x5d
005bda92  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005bda95  e856a90f00            CALL 0x006b83f0
005bda9a  85c0                  TEST EAX,EAX
005bda9c  740a                  JZ 0x005bdaa8
005bda9e  b801000000            MOV EAX,0x1
005bdaa3  e9a8000000            JMP 0x005bdb50
005bdaa8  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005bdaaf  eb09                  JMP 0x005bdaba
005bdab1  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
005bdab4  83c001                ADD EAX,0x1
005bdab7  8945fc                MOV dword ptr [EBP + -0x4],EAX
005bdaba  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
005bdabd  8b510c                MOV EDX,dword ptr [ECX + 0xc]
005bdac0  8955e4                MOV dword ptr [EBP + -0x1c],EDX
005bdac3  a1d0b4a100            MOV EAX,[0x00a1b4d0]
005bdac8  8945e0                MOV dword ptr [EBP + -0x20],EAX
005bdacb  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
005bdace  51                    PUSH ECX
005bdacf  8b55e0                MOV EDX,dword ptr [EBP + -0x20]
005bdad2  8b4a34                MOV ECX,dword ptr [EDX + 0x34]
005bdad5  e826771100            CALL 0x006d5200
005bdada  8945dc                MOV dword ptr [EBP + -0x24],EAX
005bdadd  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
005bdae0  660fb68880000000      MOVZX CX,byte ptr [EAX + 0x80]
005bdae8  0fb7d1                MOVZX EDX,CX
005bdaeb  3955fc                CMP dword ptr [EBP + -0x4],EDX
005bdaee  7d5b                  JGE 0x005bdb4b
005bdaf0  8b4508                MOV EAX,dword ptr [EBP + 0x8]
005bdaf3  8b480c                MOV ECX,dword ptr [EAX + 0xc]
005bdaf6  894dd8                MOV dword ptr [EBP + -0x28],ECX
005bdaf9  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
005bdaff  8955d4                MOV dword ptr [EBP + -0x2c],EDX
005bdb02  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
005bdb05  50                    PUSH EAX
005bdb06  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
005bdb09  8b4934                MOV ECX,dword ptr [ECX + 0x34]
005bdb0c  e8ef761100            CALL 0x006d5200
005bdb11  8945d0                MOV dword ptr [EBP + -0x30],EAX
005bdb14  0fb655fc              MOVZX EDX,byte ptr [EBP + -0x4]
005bdb18  52                    PUSH EDX
005bdb19  8b4dd0                MOV ECX,dword ptr [EBP + -0x30]
005bdb1c  e89f741100            CALL 0x006d4fc0
005bdb21  668945f8              MOV word ptr [EBP + -0x8],AX
005bdb25  8b45c8                MOV EAX,dword ptr [EBP + -0x38]
005bdb28  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
005bdb2e  894dcc                MOV dword ptr [EBP + -0x34],ECX
005bdb31  0fb755f8              MOVZX EDX,word ptr [EBP + -0x8]
005bdb35  52                    PUSH EDX
005bdb36  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
005bdb39  e8b2a80f00            CALL 0x006b83f0
005bdb3e  85c0                  TEST EAX,EAX
005bdb40  7504                  JNZ 0x005bdb46
005bdb42  33c0                  XOR EAX,EAX
005bdb44  eb0a                  JMP 0x005bdb50
005bdb46  e966ffffff            JMP 0x005bdab1
005bdb4b  b801000000            MOV EAX,0x1
005bdb50  8be5                  MOV ESP,EBP
005bdb52  5d                    POP EBP
005bdb53  c20800                RET 0x8
