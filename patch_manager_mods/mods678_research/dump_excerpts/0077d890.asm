; FN 0077d890 FUN_0077d890
0077d890  55                    PUSH EBP
0077d891  8bec                  MOV EBP,ESP
0077d893  83ec34                SUB ESP,0x34
0077d896  894dd0                MOV dword ptr [EBP + -0x30],ECX
0077d899  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0077d89c  e85fffffff            CALL 0x0077d800
0077d8a1  8945f4                MOV dword ptr [EBP + -0xc],EAX
0077d8a4  837df400              CMP dword ptr [EBP + -0xc],0x0
0077d8a8  7505                  JNZ 0x0077d8af
0077d8aa  e97c010000            JMP 0x0077da2b
0077d8af  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
0077d8b2  8b8898110000          MOV ECX,dword ptr [EAX + 0x1198]
0077d8b8  894df0                MOV dword ptr [EBP + -0x10],ECX
0077d8bb  837df000              CMP dword ptr [EBP + -0x10],0x0
0077d8bf  7505                  JNZ 0x0077d8c6
0077d8c1  e965010000            JMP 0x0077da2b
0077d8c6  c745f8ffffffff        MOV dword ptr [EBP + -0x8],0xffffffff
0077d8cd  c745fcff000000        MOV dword ptr [EBP + -0x4],0xff
0077d8d4  8b5508                MOV EDX,dword ptr [EBP + 0x8]
0077d8d7  8955cc                MOV dword ptr [EBP + -0x34],EDX
0077d8da  8b45cc                MOV EAX,dword ptr [EBP + -0x34]
0077d8dd  2d10040000            SUB EAX,0x410
0077d8e2  8945cc                MOV dword ptr [EBP + -0x34],EAX
0077d8e5  837dcc04              CMP dword ptr [EBP + -0x34],0x4
0077d8e9  0f87db000000          JA 0x0077d9ca
0077d8ef  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
0077d8f2  ff248d34da7700        JMP dword ptr [ECX*0x4 + 0x77da34]
0077d8f9  c745f83ce30100        MOV dword ptr [EBP + -0x8],0x1e33c
0077d900  ba09000000            MOV EDX,0x9
0077d905  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0077d908  66899046010000        MOV word ptr [EAX + 0x146],DX
0077d90f  c745fc3d000000        MOV dword ptr [EBP + -0x4],0x3d
0077d916  e9af000000            JMP 0x0077d9ca
0077d91b  c745f83ee30100        MOV dword ptr [EBP + -0x8],0x1e33e
0077d922  b90b000000            MOV ECX,0xb
0077d927  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0077d92a  66898a46010000        MOV word ptr [EDX + 0x146],CX
0077d931  c745fc3f000000        MOV dword ptr [EBP + -0x4],0x3f
0077d938  e98d000000            JMP 0x0077d9ca
0077d93d  c745f83fe30100        MOV dword ptr [EBP + -0x8],0x1e33f
0077d944  b80c000000            MOV EAX,0xc
0077d949  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0077d94c  66898146010000        MOV word ptr [ECX + 0x146],AX
0077d953  c745fc40000000        MOV dword ptr [EBP + -0x4],0x40
0077d95a  eb6e                  JMP 0x0077d9ca
0077d95c  8b55d0                MOV EDX,dword ptr [EBP + -0x30]
0077d95f  8b8210030000          MOV EAX,dword ptr [EDX + 0x310]
0077d965  8945ec                MOV dword ptr [EBP + -0x14],EAX
0077d968  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0077d96b  8a5132                MOV DL,byte ptr [ECX + 0x32]
0077d96e  8855eb                MOV byte ptr [EBP + -0x15],DL
0077d971  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
0077d974  8b8810030000          MOV ECX,dword ptr [EAX + 0x310]
0077d97a  894de4                MOV dword ptr [EBP + -0x1c],ECX
0077d97d  0fb655eb              MOVZX EDX,byte ptr [EBP + -0x15]
0077d981  83ea01                SUB EDX,0x1
0077d984  52                    PUSH EDX
0077d985  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
0077d988  e8b3740c00            CALL 0x00844e40
0077d98d  0fb6c0                MOVZX EAX,AL
0077d990  50                    PUSH EAX
0077d991  e89a50f7ff            CALL 0x006f2a30
0077d996  83c404                ADD ESP,0x4
0077d999  85c0                  TEST EAX,EAX
0077d99b  7410                  JZ 0x0077d9ad
0077d99d  c745f83eec0100        MOV dword ptr [EBP + -0x8],0x1ec3e
0077d9a4  c745fc41000000        MOV dword ptr [EBP + -0x4],0x41
0077d9ab  eb0e                  JMP 0x0077d9bb
0077d9ad  c745f866ec0100        MOV dword ptr [EBP + -0x8],0x1ec66
0077d9b4  c745fc42000000        MOV dword ptr [EBP + -0x4],0x42
0077d9bb  b90d000000            MOV ECX,0xd
0077d9c0  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0077d9c3  66898a46010000        MOV word ptr [EDX + 0x146],CX
0077d9ca  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0077d9cf  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0077d9d2  894de0                MOV dword ptr [EBP + -0x20],ECX
0077d9d5  6a00                  PUSH 0x0
0077d9d7  680000007f            PUSH 0x7f000000
0077d9dc  680000007f            PUSH 0x7f000000
0077d9e1  0fb655fc              MOVZX EDX,byte ptr [EBP + -0x4]
0077d9e5  52                    PUSH EDX
0077d9e6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
0077d9e9  e83230fcff            CALL 0x00740a20
0077d9ee  a1a4b4a100            MOV EAX,[0x00a1b4a4]
0077d9f3  8b4804                MOV ECX,dword ptr [EAX + 0x4]
0077d9f6  894ddc                MOV dword ptr [EBP + -0x24],ECX
0077d9f9  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
0077d9fc  e84f1dfcff            CALL 0x0073f750
0077da01  8945d8                MOV dword ptr [EBP + -0x28],EAX
0077da04  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
0077da07  8b8298000000          MOV EAX,dword ptr [EDX + 0x98]
0077da0d  8945d4                MOV dword ptr [EBP + -0x2c],EAX
0077da10  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
0077da13  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
0077da16  899110540100          MOV dword ptr [ECX + 0x15410],EDX
0077da1c  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
0077da1f  d905786b9800          FLD float ptr [0x00986b78]
0077da25  d9980c540100          FSTP float ptr [EAX + 0x1540c]
0077da2b  8be5                  MOV ESP,EBP
0077da2d  5d                    POP EBP
0077da2e  c20800                RET 0x8
