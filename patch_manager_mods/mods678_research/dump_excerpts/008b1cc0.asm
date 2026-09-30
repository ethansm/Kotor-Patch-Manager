; FN 008b1cc0 FUN_008b1cc0
008b1cc0  55                    PUSH EBP
008b1cc1  8bec                  MOV EBP,ESP
008b1cc3  51                    PUSH ECX
008b1cc4  894dfc                MOV dword ptr [EBP + -0x4],ECX
008b1cc7  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
008b1cca  8b88ac030000          MOV ECX,dword ptr [EAX + 0x3ac]
008b1cd0  c1e902                SHR ECX,0x2
008b1cd3  83e101                AND ECX,0x1
008b1cd6  51                    PUSH ECX
008b1cd7  8b55fc                MOV EDX,dword ptr [EBP + -0x4]
008b1cda  8b82ac030000          MOV EAX,dword ptr [EDX + 0x3ac]
008b1ce0  d1e8                  SHR EAX,0x1
008b1ce2  83e001                AND EAX,0x1
008b1ce5  50                    PUSH EAX
008b1ce6  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1ce9  8b91d0010000          MOV EDX,dword ptr [ECX + 0x1d0]
008b1cef  52                    PUSH EDX
008b1cf0  8b4dfc                MOV ECX,dword ptr [EBP + -0x4]
008b1cf3  e858f5ffff            CALL 0x008b1250
008b1cf8  8be5                  MOV ESP,EBP
008b1cfa  5d                    POP EBP
008b1cfb  c3                    RET
