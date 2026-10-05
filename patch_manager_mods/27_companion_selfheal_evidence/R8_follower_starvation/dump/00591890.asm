; FN 00591890 FUN_00591890
00591890  55                    PUSH EBP
00591891  8bec                  MOV EBP,ESP
00591893  6aff                  PUSH -0x1
00591895  687bfa9400            PUSH 0x94fa7b
0059189a  64a100000000          MOV EAX,FS:[0x0]
005918a0  50                    PUSH EAX
005918a1  83ec20                SUB ESP,0x20
005918a4  a1201fa100            MOV EAX,[0x00a11f20]
005918a9  33c5                  XOR EAX,EBP
005918ab  50                    PUSH EAX
005918ac  8d45f4                LEA EAX,[EBP + -0xc]
005918af  64a300000000          MOV FS:[0x0],EAX
005918b5  894dd8                MOV dword ptr [EBP + -0x28],ECX
005918b8  6888000000            PUSH 0x88
005918bd  e8617e3800            CALL 0x00919723
005918c2  83c404                ADD ESP,0x4
005918c5  8945e8                MOV dword ptr [EBP + -0x18],EAX
005918c8  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
005918cf  837de800              CMP dword ptr [EBP + -0x18],0x0
005918d3  740d                  JZ 0x005918e2
005918d5  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
005918d8  e8f3aeffff            CALL 0x0058c7d0
005918dd  8945d4                MOV dword ptr [EBP + -0x2c],EAX
005918e0  eb07                  JMP 0x005918e9
005918e2  c745d400000000        MOV dword ptr [EBP + -0x2c],0x0
005918e9  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
005918ec  8945ec                MOV dword ptr [EBP + -0x14],EAX
005918ef  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
005918f6  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
005918f9  894df0                MOV dword ptr [EBP + -0x10],ECX
005918fc  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
005918ff  c642100a              MOV byte ptr [EDX + 0x10],0xa
00591903  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00591906  8b4d28                MOV ECX,dword ptr [EBP + 0x28]
00591909  898884000000          MOV dword ptr [EAX + 0x84],ECX
0059190f  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00591912  8b4508                MOV EAX,dword ptr [EBP + 0x8]
00591915  894264                MOV dword ptr [EDX + 0x64],EAX
00591918  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
0059191b  8b550c                MOV EDX,dword ptr [EBP + 0xc]
0059191e  895168                MOV dword ptr [ECX + 0x68],EDX
00591921  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00591924  8b4d10                MOV ECX,dword ptr [EBP + 0x10]
00591927  89486c                MOV dword ptr [EAX + 0x6c],ECX
0059192a  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
0059192d  83c238                ADD EDX,0x38
00591930  8b4514                MOV EAX,dword ptr [EBP + 0x14]
00591933  8902                  MOV dword ptr [EDX],EAX
00591935  8b4d18                MOV ECX,dword ptr [EBP + 0x18]
00591938  894a04                MOV dword ptr [EDX + 0x4],ECX
0059193b  8b451c                MOV EAX,dword ptr [EBP + 0x1c]
0059193e  894208                MOV dword ptr [EDX + 0x8],EAX
00591941  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00591944  8b5520                MOV EDX,dword ptr [EBP + 0x20]
00591947  895144                MOV dword ptr [ECX + 0x44],EDX
0059194a  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
0059194d  8b4d24                MOV ECX,dword ptr [EBP + 0x24]
00591950  894848                MOV dword ptr [EAX + 0x48],ECX
00591953  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
00591956  8b82040b0000          MOV EAX,dword ptr [EDX + 0xb04]
0059195c  8945e4                MOV dword ptr [EBP + -0x1c],EAX
0059195f  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
00591962  8b9168050000          MOV EDX,dword ptr [ECX + 0x568]
00591968  8955e0                MOV dword ptr [EBP + -0x20],EDX
0059196b  817de00000007f        CMP dword ptr [EBP + -0x20],0x7f000000
00591972  7518                  JNZ 0x0059198c
00591974  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
00591977  8b88040b0000          MOV ECX,dword ptr [EAX + 0xb04]
0059197d  894ddc                MOV dword ptr [EBP + -0x24],ECX
00591980  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
00591983  8b4520                MOV EAX,dword ptr [EBP + 0x20]
00591986  898268050000          MOV dword ptr [EDX + 0x568],EAX
0059198c  6aff                  PUSH -0x1
0059198e  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00591991  51                    PUSH ECX
00591992  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
00591995  e8d6e8ffff            CALL 0x00590270
0059199a  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
0059199d  64890d00000000        MOV dword ptr FS:[0x0],ECX
005919a4  59                    POP ECX
005919a5  8be5                  MOV ESP,EBP
005919a7  5d                    POP EBP
005919a8  c22400                RET 0x24
