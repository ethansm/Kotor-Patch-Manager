; FN 00796b50 FUN_00796b50
00796b50  55                    PUSH EBP
00796b51  8bec                  MOV EBP,ESP
00796b53  83ec10                SUB ESP,0x10
00796b56  894df4                MOV dword ptr [EBP + -0xc],ECX
00796b59  837d0800              CMP dword ptr [EBP + 0x8],0x0
00796b5d  7411                  JZ 0x00796b70
00796b5f  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
00796b62  8945f0                MOV dword ptr [EBP + -0x10],EAX
00796b65  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00796b68  8b5104                MOV EDX,dword ptr [ECX + 0x4]
00796b6b  8955f0                MOV dword ptr [EBP + -0x10],EDX
00796b6e  eb07                  JMP 0x00796b77
00796b70  c745f00000007f        MOV dword ptr [EBP + -0x10],0x7f000000
00796b77  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00796b7a  8945fc                MOV dword ptr [EBP + -0x4],EAX
00796b7d  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00796b80  51                    PUSH ECX
00796b81  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00796b84  52                    PUSH EDX
00796b85  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00796b88  8b4840                MOV ECX,dword ptr [EAX + 0x40]
00796b8b  e8807b0300            CALL 0x007ce710
00796b90  8be5                  MOV ESP,EBP
00796b92  5d                    POP EBP
00796b93  c20800                RET 0x8
