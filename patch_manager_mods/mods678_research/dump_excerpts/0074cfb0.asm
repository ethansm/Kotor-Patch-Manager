; FN 0074cfb0 FUN_0074cfb0
0074cfb0  55                    PUSH EBP
0074cfb1  8bec                  MOV EBP,ESP
0074cfb3  51                    PUSH ECX
0074cfb4  894dfc                MOV dword ptr [EBP + -0x4],ECX
0074cfb7  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
0074cfba  8b4868                MOV ECX,dword ptr [EAX + 0x68]
0074cfbd  3b4d08                CMP ECX,dword ptr [EBP + 0x8]
0074cfc0  7418                  JZ 0x0074cfda
0074cfc2  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
0074cfc5  8b4508                MOV EAX,dword ptr [EBP + 0x8]
0074cfc8  894268                MOV dword ptr [EDX + 0x68],EAX
0074cfcb  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
0074cfce  d90574749800          FLD float ptr [0x00987474]
0074cfd4  d9995c590000          FSTP float ptr [ECX + 0x595c]
0074cfda  8be5                  MOV ESP,EBP
0074cfdc  5d                    POP EBP
0074cfdd  c20800                RET 0x8
