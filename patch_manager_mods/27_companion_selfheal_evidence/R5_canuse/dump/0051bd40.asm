; FN 0051bd40 FUN_0051bd40
0051bd40  55                    PUSH EBP
0051bd41  8bec                  MOV EBP,ESP
0051bd43  83ec08                SUB ESP,0x8
0051bd46  894df8                MOV dword ptr [EBP + -0x8],ECX
0051bd49  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
0051bd4c  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0051bd4f  8b9104000100          MOV EDX,dword ptr [ECX + 0x10004]
0051bd55  8955fc                MOV dword ptr [EBP + -0x4],EDX
0051bd58  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0051bd5b  8be5                  MOV ESP,EBP
0051bd5d  5d                    POP EBP
0051bd5e  c3                    RET
