; FN 00416e30 FUN_00416e30
00416e30  55                    PUSH EBP
00416e31  8bec                  MOV EBP,ESP
00416e33  51                    PUSH ECX
00416e34  894dfc                MOV dword ptr [EBP + -0x4],ECX
00416e37  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00416e3a  50                    PUSH EAX
00416e3b  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00416e3e  e87dc93100            CALL 0x007337c0
00416e43  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00416e46  c74108ffffffff        MOV dword ptr [ECX + 0x8],0xffffffff
00416e4d  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00416e50  837a5000              CMP dword ptr [EDX + 0x50],0x0
00416e54  743b                  JZ 0x00416e91
00416e56  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00416e59  8b4850                MOV ECX,dword ptr [EAX + 0x50]
00416e5c  83791400              CMP dword ptr [ECX + 0x14],0x0
00416e60  742f                  JZ 0x00416e91
00416e62  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00416e65  e8a6d13100            CALL 0x00734010
00416e6a  50                    PUSH EAX
00416e6b  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00416e6e  8b4250                MOV EAX,dword ptr [EDX + 0x50]
00416e71  8b4814                MOV ECX,dword ptr [EAX + 0x14]
00416e74  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00416e77  8b4250                MOV EAX,dword ptr [EDX + 0x50]
00416e7a  8b5014                MOV EDX,dword ptr [EAX + 0x14]
00416e7d  8b01                  MOV EAX,dword ptr [ECX]
00416e7f  8bca                  MOV ECX,EDX
00416e81  8b5018                MOV EDX,dword ptr [EAX + 0x18]
00416e84  ffd2                  CALL EDX
00416e86  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00416e89  8b4850                MOV ECX,dword ptr [EAX + 0x50]
00416e8c  e86f090000            CALL 0x00417800
00416e91  8be5                  MOV ESP,EBP
00416e93  5d                    POP EBP
00416e94  c20400                RET 0x4
