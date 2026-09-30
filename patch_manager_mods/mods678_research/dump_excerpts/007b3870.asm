; FN 007b3870 FUN_007b3870
007b3870  55                    PUSH EBP
007b3871  8bec                  MOV EBP,ESP
007b3873  51                    PUSH ECX
007b3874  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b3877  833d8cb4a10000        CMP dword ptr [0x00a1b48c],0x0
007b387e  741a                  JZ 0x007b389a
007b3880  8b4510                MOV EAX,dword ptr [EBP + 0x10]
007b3883  50                    PUSH EAX
007b3884  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
007b3887  51                    PUSH ECX
007b3888  8b5508                MOV EDX,dword ptr [EBP + 0x8]
007b388b  52                    PUSH EDX
007b388c  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007b3892  e8b9c3f7ff            CALL 0x0072fc50
007b3897  894508                MOV dword ptr [EBP + 0x8],EAX
007b389a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
007b389d  83b87402000000        CMP dword ptr [EAX + 0x274],0x0
007b38a4  741a                  JZ 0x007b38c0
007b38a6  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
007b38a9  51                    PUSH ECX
007b38aa  8b550c                MOV EDX,dword ptr [EBP + 0xc]
007b38ad  52                    PUSH EDX
007b38ae  8b4508                MOV EAX,dword ptr [EBP + 0x8]
007b38b1  50                    PUSH EAX
007b38b2  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
007b38b5  8b8974020000          MOV ECX,dword ptr [ECX + 0x274]
007b38bb  e8c001c6ff            CALL 0x00413a80
007b38c0  8be5                  MOV ESP,EBP
007b38c2  5d                    POP EBP
007b38c3  c20c00                RET 0xc
