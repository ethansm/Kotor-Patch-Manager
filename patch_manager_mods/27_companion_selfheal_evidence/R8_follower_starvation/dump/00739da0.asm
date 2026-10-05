; FN 00739da0 FUN_00739da0
00739da0  55                    PUSH EBP
00739da1  8bec                  MOV EBP,ESP
00739da3  83ec14                SUB ESP,0x14
00739da6  894df0                MOV dword ptr [EBP + -0x10],ECX
00739da9  6a0c                  PUSH 0xc
00739dab  e873f91d00            CALL 0x00919723
00739db0  83c404                ADD ESP,0x4
00739db3  8945f4                MOV dword ptr [EBP + -0xc],EAX
00739db6  837df400              CMP dword ptr [EBP + -0xc],0x0
00739dba  7425                  JZ 0x00739de1
00739dbc  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00739dbf  c7400800000000        MOV dword ptr [EAX + 0x8],0x0
00739dc6  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
00739dc9  c7410400000000        MOV dword ptr [ECX + 0x4],0x0
00739dd0  8b55f4                MOV EDX,dword ptr [EBP + -0xc]
00739dd3  c70200000000          MOV dword ptr [EDX],0x0
00739dd9  8b45f4                MOV EAX,dword ptr [EBP + -0xc]
00739ddc  8945ec                MOV dword ptr [EBP + -0x14],EAX
00739ddf  eb07                  JMP 0x00739de8
00739de1  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
00739de8  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
00739deb  894df8                MOV dword ptr [EBP + -0x8],ECX
00739dee  8b55f8                MOV EDX,dword ptr [EBP + -0x8]
00739df1  8955fc                MOV dword ptr [EBP + -0x4],EDX
00739df4  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739df7  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739dfa  8b11                  MOV EDX,dword ptr [ECX]
00739dfc  895004                MOV dword ptr [EAX + 0x4],EDX
00739dff  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739e02  c70000000000          MOV dword ptr [EAX],0x0
00739e08  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739e0b  833900                CMP dword ptr [ECX],0x0
00739e0e  740a                  JZ 0x00739e1a
00739e10  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739e13  8b02                  MOV EAX,dword ptr [EDX]
00739e15  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00739e18  8908                  MOV dword ptr [EAX],ECX
00739e1a  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739e1d  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739e20  8902                  MOV dword ptr [EDX],EAX
00739e22  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
00739e25  8b5508                MOV EDX,dword ptr [EBP + 0x8]
00739e28  895108                MOV dword ptr [ECX + 0x8],EDX
00739e2b  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00739e2e  83780400              CMP dword ptr [EAX + 0x4],0x0
00739e32  7509                  JNZ 0x00739e3d
00739e34  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
00739e37  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
00739e3a  895104                MOV dword ptr [ECX + 0x4],EDX
00739e3d  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
00739e40  8b4808                MOV ECX,dword ptr [EAX + 0x8]
00739e43  83c101                ADD ECX,0x1
00739e46  8b55f0                MOV EDX,dword ptr [EBP + -0x10]
00739e49  894a08                MOV dword ptr [EDX + 0x8],ECX
00739e4c  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00739e4f  8be5                  MOV ESP,EBP
00739e51  5d                    POP EBP
00739e52  c20400                RET 0x4
