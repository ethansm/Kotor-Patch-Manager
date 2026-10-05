; FN 0077fd20 FUN_0077fd20
0077fd20  55                    PUSH EBP
0077fd21  8bec                  MOV EBP,ESP
0077fd23  83ec10                SUB ESP,0x10
0077fd26  894df0                MOV dword ptr [EBP + -0x10],ECX
0077fd29  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0077fd2c  83e001                AND EAX,0x1
0077fd2f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0077fd32  8b9158040000          MOV EDX,dword ptr [ECX + 0x458]
0077fd38  83e2fe                AND EDX,0xfffffffe
0077fd3b  0bd0                  OR EDX,EAX
0077fd3d  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0077fd40  899058040000          MOV dword ptr [EAX + 0x458],EDX
0077fd46  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0077fd49  8b5104                MOV EDX,dword ptr [ECX + 0x4]
0077fd4c  8955fc                MOV dword ptr [EBP + -0x4],EDX
0077fd4f  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0077fd54  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0077fd57  894df8                MOV dword ptr [EBP + -0x8],ECX
0077fd5a  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
0077fd5d  e80ef7fbff            CALL 0x0073f470
0077fd62  3945fc                CMP dword ptr [EBP + -0x4],EAX
0077fd65  7521                  JNZ 0x0077fd88
0077fd67  8b15a4b4a100          MOV EDX,dword ptr [0x00a1b4a4]
0077fd6d  8b4204                MOV EAX,dword ptr [EDX + 0x4]
0077fd70  8945f4                MOV dword ptr [EBP + -0xc],EAX
0077fd73  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0077fd76  8b9158040000          MOV EDX,dword ptr [ECX + 0x458]
0077fd7c  83e201                AND EDX,0x1
0077fd7f  52                    PUSH EDX
0077fd80  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0077fd83  e8f80afcff            CALL 0x00740880
0077fd88  8be5                  MOV ESP,EBP
0077fd8a  5d                    POP EBP
0077fd8b  c20400                RET 0x4
