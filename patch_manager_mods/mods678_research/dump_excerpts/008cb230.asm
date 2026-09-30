; FN 008cb230 FUN_008cb230
008cb230  55                    PUSH EBP
008cb231  8bec                  MOV EBP,ESP
008cb233  83ec0c                SUB ESP,0xc
008cb236  894df8                MOV dword ptr [EBP + -0x8],ECX
008cb239  c645ff00              MOV byte ptr [EBP + -0x1],0x0
008cb23d  8b45f8                MOV EAX,dword ptr [EBP + -0x8]
008cb240  0fb6882c3d0000        MOVZX ECX,byte ptr [EAX + 0x3d2c]
008cb247  894df4                MOV dword ptr [EBP + -0xc],ECX
008cb24a  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
008cb24d  83ea01                SUB EDX,0x1
008cb250  8955f4                MOV dword ptr [EBP + -0xc],EDX
008cb253  837df403              CMP dword ptr [EBP + -0xc],0x3
008cb257  7720                  JA 0x008cb279
008cb259  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
008cb25c  ff248580b28c00        JMP dword ptr [EAX*0x4 + 0x8cb280]
008cb263  c645ff06              MOV byte ptr [EBP + -0x1],0x6
008cb267  eb10                  JMP 0x008cb279
008cb269  c645ff03              MOV byte ptr [EBP + -0x1],0x3
008cb26d  eb0a                  JMP 0x008cb279
008cb26f  c645ff03              MOV byte ptr [EBP + -0x1],0x3
008cb273  eb04                  JMP 0x008cb279
008cb275  c645ff02              MOV byte ptr [EBP + -0x1],0x2
008cb279  8a45ff                MOV AL,byte ptr [EBP + -0x1]
008cb27c  8be5                  MOV ESP,EBP
008cb27e  5d                    POP EBP
008cb27f  c3                    RET
