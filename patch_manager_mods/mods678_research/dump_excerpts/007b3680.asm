; FN 007b3680 FUN_007b3680
007b3680  55                    PUSH EBP
007b3681  8bec                  MOV EBP,ESP
007b3683  83ec28                SUB ESP,0x28
007b3686  894dd8                MOV dword ptr [EBP + -0x28],ECX
007b3689  c705c0fca70000000000  MOV dword ptr [0x00a7fcc0],0x0
007b3693  833d8cb4a10000        CMP dword ptr [0x00a1b48c],0x0
007b369a  740d                  JZ 0x007b36a9
007b369c  6a00                  PUSH 0x0
007b369e  8b0d8cb4a100          MOV ECX,dword ptr [0x00a1b48c]
007b36a4  e887c5f7ff            CALL 0x0072fc30
007b36a9  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
007b36ac  83b87402000000        CMP dword ptr [EAX + 0x274],0x0
007b36b3  7436                  JZ 0x007b36eb
007b36b5  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b36b8  8b9174020000          MOV EDX,dword ptr [ECX + 0x274]
007b36be  8b4238                MOV EAX,dword ptr [EDX + 0x38]
007b36c1  8945f8                MOV dword ptr [EBP + -0x8],EAX
007b36c4  6a00                  PUSH 0x0
007b36c6  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b36c9  8b9174020000          MOV EDX,dword ptr [ECX + 0x274]
007b36cf  8b421c                MOV EAX,dword ptr [EDX + 0x1c]
007b36d2  c1e802                SHR EAX,0x2
007b36d5  83e001                AND EAX,0x1
007b36d8  50                    PUSH EAX
007b36d9  8b4df8                MOV ECX,dword ptr [EBP + -0x8]
007b36dc  51                    PUSH ECX
007b36dd  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b36e0  8b8a74020000          MOV ECX,dword ptr [EDX + 0x274]
007b36e6  e89503c6ff            CALL 0x00413a80
007b36eb  a1bcfca700            MOV EAX,[0x00a7fcbc]
007b36f0  8945fc                MOV dword ptr [EBP + -0x4],EAX
007b36f3  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007b36f9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
007b36fc  8955f4                MOV dword ptr [EBP + -0xc],EDX
007b36ff  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
007b3702  e8f9c3f7ff            CALL 0x0072fb00
007b3707  8945f0                MOV dword ptr [EBP + -0x10],EAX
007b370a  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
007b370d  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b3710  d1e9                  SHR ECX,0x1
007b3712  83e101                AND ECX,0x1
007b3715  7424                  JZ 0x007b373b
007b3717  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b371a  83ba9c00000002        CMP dword ptr [EDX + 0x9c],0x2
007b3721  7418                  JZ 0x007b373b
007b3723  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
007b3726  83b89c00000003        CMP dword ptr [EAX + 0x9c],0x3
007b372d  740c                  JZ 0x007b373b
007b372f  33c9                  XOR ECX,ECX
007b3731  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b3735  0f94c1                SETZ CL
007b3738  894dfc                MOV dword ptr [EBP + -0x4],ECX
007b373b  837dfc00              CMP dword ptr [EBP + -0x4],0x0
007b373f  750c                  JNZ 0x007b374d
007b3741  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b3744  83ba9c00000004        CMP dword ptr [EDX + 0x9c],0x4
007b374b  751c                  JNZ 0x007b3769
007b374d  8b45d8                MOV EAX,dword ptr [EBP + -0x28]
007b3750  83b89c00000002        CMP dword ptr [EAX + 0x9c],0x2
007b3757  7410                  JZ 0x007b3769
007b3759  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b375c  83b99c00000003        CMP dword ptr [ECX + 0x9c],0x3
007b3763  0f8595000000          JNZ 0x007b37fe
007b3769  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b376c  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007b3772  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b3775  894dec                MOV dword ptr [EBP + -0x14],ECX
007b3778  837dec00              CMP dword ptr [EBP + -0x14],0x0
007b377c  7454                  JZ 0x007b37d2
007b377e  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b3781  8b8274020000          MOV EAX,dword ptr [EDX + 0x274]
007b3787  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b378a  894de8                MOV dword ptr [EBP + -0x18],ECX
007b378d  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
007b3790  8b4240                MOV EAX,dword ptr [EDX + 0x40]
007b3793  8b8898000000          MOV ECX,dword ptr [EAX + 0x98]
007b3799  894de4                MOV dword ptr [EBP + -0x1c],ECX
007b379c  8b55e8                MOV EDX,dword ptr [EBP + -0x18]
007b379f  52                    PUSH EDX
007b37a0  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
007b37a3  81c1cc000000          ADD ECX,0xcc
007b37a9  e84212f9ff            CALL 0x007449f0
007b37ae  0fb6c0                MOVZX EAX,AL
007b37b1  83f801                CMP EAX,0x1
007b37b4  751c                  JNZ 0x007b37d2
007b37b6  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
007b37bc  8b5108                MOV EDX,dword ptr [ECX + 0x8]
007b37bf  8955e0                MOV dword ptr [EBP + -0x20],EDX
007b37c2  837de000              CMP dword ptr [EBP + -0x20],0x0
007b37c6  7408                  JZ 0x007b37d0
007b37c8  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b37cb  e8d0040000            CALL 0x007b3ca0
007b37d0  eb2c                  JMP 0x007b37fe
007b37d2  8b0d9cb4a100          MOV ECX,dword ptr [0x00a1b49c]
007b37d8  e803fbc5ff            CALL 0x004132e0
007b37dd  85c0                  TEST EAX,EAX
007b37df  7404                  JZ 0x007b37e5
007b37e1  eb1b                  JMP 0x007b37fe
007b37e5  a1a4b4a100            MOV EAX,[0x00a1b4a4]
007b37ea  8b4808                MOV ECX,dword ptr [EAX + 0x8]
007b37ed  894ddc                MOV dword ptr [EBP + -0x24],ECX
007b37f0  837ddc00              CMP dword ptr [EBP + -0x24],0x0
007b37f4  7408                  JZ 0x007b37fe
007b37f6  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
007b37f9  e8a2040000            CALL 0x007b3ca0
007b37fe  8be5                  MOV ESP,EBP
007b3800  5d                    POP EBP
007b3801  c3                    RET
