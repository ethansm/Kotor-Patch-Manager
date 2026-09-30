; FN 008c8880 FUN_008c8880
008c8880  55                    PUSH EBP
008c8881  8bec                  MOV EBP,ESP
008c8883  6aff                  PUSH -0x1
008c8885  68dbae9700            PUSH 0x97aedb
008c888a  64a100000000          MOV EAX,FS:[0x0]
008c8890  50                    PUSH EAX
008c8891  81ec8c000000          SUB ESP,0x8c
008c8897  a1201fa100            MOV EAX,[0x00a11f20]
008c889c  33c5                  XOR EAX,EBP
008c889e  50                    PUSH EAX
008c889f  8d45f4                LEA EAX,[EBP + -0xc]
008c88a2  64a300000000          MOV FS:[0x0],EAX
008c88a8  898d70ffffff          MOV dword ptr [EBP + 0xffffff70],ECX
008c88ae  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c88b4  8a88780c0000          MOV CL,byte ptr [EAX + 0xc78]
008c88ba  888d6cffffff          MOV byte ptr [EBP + 0xffffff6c],CL
008c88c0  80bd6cffffff00        CMP byte ptr [EBP + 0xffffff6c],0x0
008c88c7  760d                  JBE 0x008c88d6
008c88c9  80bd6cffffff04        CMP byte ptr [EBP + 0xffffff6c],0x4
008c88d0  7602                  JBE 0x008c88d4
008c88d2  eb02                  JMP 0x008c88d6
008c88d4  eb57                  JMP 0x008c892d
008c88d6  33d2                  XOR EDX,EDX
008c88d8  83e201                AND EDX,0x1
008c88db  c1e207                SHL EDX,0x7
008c88de  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c88e4  8b4848                MOV ECX,dword ptr [EAX + 0x48]
008c88e7  81e17fffffff          AND ECX,0xffffff7f
008c88ed  0bca                  OR ECX,EDX
008c88ef  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c88f5  894a48                MOV dword ptr [EDX + 0x48],ECX
008c88f8  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c88fe  8b4848                MOV ECX,dword ptr [EAX + 0x48]
008c8901  c1e908                SHR ECX,0x8
008c8904  83e103                AND ECX,0x3
008c8907  83f902                CMP ECX,0x2
008c890a  741c                  JZ 0x008c8928
008c890c  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8912  8b4248                MOV EAX,dword ptr [EDX + 0x48]
008c8915  25fffcffff            AND EAX,0xfffffcff
008c891a  0d00010000            OR EAX,0x100
008c891f  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8925  894148                MOV dword ptr [ECX + 0x48],EAX
008c8928  e9ec060000            JMP 0x008c9019
008c892d  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8933  83ba8c0c000000        CMP dword ptr [EDX + 0xc8c],0x0
008c893a  0f8492060000          JZ 0x008c8fd2
008c8940  a1a4b4a100            MOV EAX,[0x00a1b4a4]
008c8945  8b4808                MOV ECX,dword ptr [EAX + 0x8]
008c8948  894df0                MOV dword ptr [EBP + -0x10],ECX
008c894b  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8951  8b828c0c0000          MOV EAX,dword ptr [EDX + 0xc8c]
008c8957  8b88a8020000          MOV ECX,dword ptr [EAX + 0x2a8]
008c895d  894dc8                MOV dword ptr [EBP + -0x38],ECX
008c8960  8b55c8                MOV EDX,dword ptr [EBP + -0x38]
008c8963  52                    PUSH EDX
008c8964  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c8967  e89437c5ff            CALL 0x0051c100
008c896c  8945e4                MOV dword ptr [EBP + -0x1c],EAX
008c896f  c745e800000000        MOV dword ptr [EBP + -0x18],0x0
008c8976  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008c8979  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
008c897f  894dc4                MOV dword ptr [EBP + -0x3c],ECX
008c8982  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8988  8b828c0c0000          MOV EAX,dword ptr [EDX + 0xc8c]
008c898e  50                    PUSH EAX
008c898f  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
008c8992  e8097ce0ff            CALL 0x006d05a0
008c8997  8945ec                MOV dword ptr [EBP + -0x14],EAX
008c899a  68bc030000            PUSH 0x3bc
008c899f  e87f0d0500            CALL 0x00919723
008c89a4  83c404                ADD ESP,0x4
008c89a7  8945cc                MOV dword ptr [EBP + -0x34],EAX
008c89aa  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
008c89b1  837dcc00              CMP dword ptr [EBP + -0x34],0x0
008c89b5  7410                  JZ 0x008c89c7
008c89b7  8b4dcc                MOV ECX,dword ptr [EBP + -0x34]
008c89ba  e8c1750000            CALL 0x008cff80
008c89bf  898568ffffff          MOV dword ptr [EBP + 0xffffff68],EAX
008c89c5  eb0a                  JMP 0x008c89d1
008c89c7  c78568ffffff00000000  MOV dword ptr [EBP + 0xffffff68],0x0
008c89d1  8b8d68ffffff          MOV ECX,dword ptr [EBP + 0xffffff68]
008c89d7  894dd0                MOV dword ptr [EBP + -0x30],ECX
008c89da  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
008c89e1  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c89e7  8b45d0                MOV EAX,dword ptr [EBP + -0x30]
008c89ea  8982900c0000          MOV dword ptr [EDX + 0xc90],EAX
008c89f0  837dec00              CMP dword ptr [EBP + -0x14],0x0
008c89f4  7e41                  JLE 0x008c8a37
008c89f6  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c89f9  8b5104                MOV EDX,dword ptr [ECX + 0x4]
008c89fc  8955c0                MOV dword ptr [EBP + -0x40],EDX
008c89ff  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8a05  8b888c0c0000          MOV ECX,dword ptr [EAX + 0xc8c]
008c8a0b  8b5114                MOV EDX,dword ptr [ECX + 0x14]
008c8a0e  8955bc                MOV dword ptr [EBP + -0x44],EDX
008c8a11  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
008c8a14  50                    PUSH EAX
008c8a15  8b4dc0                MOV ECX,dword ptr [EBP + -0x40]
008c8a18  51                    PUSH ECX
008c8a19  6a01                  PUSH 0x1
008c8a1b  8b55bc                MOV EDX,dword ptr [EBP + -0x44]
008c8a1e  52                    PUSH EDX
008c8a1f  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8a25  8b88900c0000          MOV ECX,dword ptr [EAX + 0xc90]
008c8a2b  e8507d0000            CALL 0x008d0780
008c8a30  c745e801000000        MOV dword ptr [EBP + -0x18],0x1
008c8a37  837de800              CMP dword ptr [EBP + -0x18],0x0
008c8a3b  0f84a5040000          JZ 0x008c8ee6
008c8a41  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8a47  8b91900c0000          MOV EDX,dword ptr [ECX + 0xc90]
008c8a4d  8b82d8010000          MOV EAX,dword ptr [EDX + 0x1d8]
008c8a53  8945b8                MOV dword ptr [EBP + -0x48],EAX
008c8a56  837db810              CMP dword ptr [EBP + -0x48],0x10
008c8a5a  741f                  JZ 0x008c8a7b
008c8a5c  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8a62  8b91900c0000          MOV EDX,dword ptr [ECX + 0xc90]
008c8a68  8b82d8010000          MOV EAX,dword ptr [EDX + 0x1d8]
008c8a6e  8945b4                MOV dword ptr [EBP + -0x4c],EAX
008c8a71  837db420              CMP dword ptr [EBP + -0x4c],0x20
008c8a75  0f85e0010000          JNZ 0x008c8c5b
008c8a7b  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8a7e  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
008c8a84  8955b0                MOV dword ptr [EBP + -0x50],EDX
008c8a87  6a10                  PUSH 0x10
008c8a89  8b4db0                MOV ECX,dword ptr [EBP + -0x50]
008c8a8c  e88f7be0ff            CALL 0x006d0620
008c8a91  8945dc                MOV dword ptr [EBP + -0x24],EAX
008c8a94  8b45e4                MOV EAX,dword ptr [EBP + -0x1c]
008c8a97  8b8850110000          MOV ECX,dword ptr [EAX + 0x1150]
008c8a9d  894dac                MOV dword ptr [EBP + -0x54],ECX
008c8aa0  6a20                  PUSH 0x20
008c8aa2  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
008c8aa5  e8767be0ff            CALL 0x006d0620
008c8aaa  8945e0                MOV dword ptr [EBP + -0x20],EAX
008c8aad  837de000              CMP dword ptr [EBP + -0x20],0x0
008c8ab1  7572                  JNZ 0x008c8b25
008c8ab3  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8ab9  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8abf  83e0fb                AND EAX,0xfffffffb
008c8ac2  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8ac8  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8ace  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8ad4  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8ada  83e0fe                AND EAX,0xfffffffe
008c8add  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8ae3  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8ae9  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8aef  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8af5  83e0fd                AND EAX,0xfffffffd
008c8af8  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8afe  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8b04  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8b0a  c782800c00000000007f  MOV dword ptr [EDX + 0xc80],0x7f000000
008c8b14  8b45dc                MOV EAX,dword ptr [EBP + -0x24]
008c8b17  50                    PUSH EAX
008c8b18  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8b1b  e83054caff            CALL 0x0056df50
008c8b20  e931010000            JMP 0x008c8c56
008c8b25  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8b2b  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008c8b2e  3b918c0c0000          CMP EDX,dword ptr [ECX + 0xc8c]
008c8b34  0f8586000000          JNZ 0x008c8bc0
008c8b3a  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8b40  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8b46  83e1fb                AND ECX,0xfffffffb
008c8b49  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8b4f  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8b55  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8b5b  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8b61  83c901                OR ECX,0x1
008c8b64  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8b6a  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8b70  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8b76  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8b7c  83e1fd                AND ECX,0xfffffffd
008c8b7f  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8b85  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8b8b  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008c8b8e  8b4814                MOV ECX,dword ptr [EAX + 0x14]
008c8b91  894da8                MOV dword ptr [EBP + -0x58],ECX
008c8b94  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8b9a  8b45a8                MOV EAX,dword ptr [EBP + -0x58]
008c8b9d  8982800c0000          MOV dword ptr [EDX + 0xc80],EAX
008c8ba3  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008c8ba6  51                    PUSH ECX
008c8ba7  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8baa  e8a153caff            CALL 0x0056df50
008c8baf  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008c8bb2  52                    PUSH EDX
008c8bb3  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8bb6  e89553caff            CALL 0x0056df50
008c8bbb  e996000000            JMP 0x008c8c56
008c8bc0  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8bc6  8b4de0                MOV ECX,dword ptr [EBP + -0x20]
008c8bc9  3b888c0c0000          CMP ECX,dword ptr [EAX + 0xc8c]
008c8bcf  0f8581000000          JNZ 0x008c8c56
008c8bd5  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8bdb  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8be1  83e0fb                AND EAX,0xfffffffb
008c8be4  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8bea  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8bf0  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8bf6  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8bfc  83c801                OR EAX,0x1
008c8bff  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8c05  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8c0b  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8c11  8b827c0c0000          MOV EAX,dword ptr [EDX + 0xc7c]
008c8c17  83c802                OR EAX,0x2
008c8c1a  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8c20  89817c0c0000          MOV dword ptr [ECX + 0xc7c],EAX
008c8c26  8b55dc                MOV EDX,dword ptr [EBP + -0x24]
008c8c29  8b4214                MOV EAX,dword ptr [EDX + 0x14]
008c8c2c  8945a4                MOV dword ptr [EBP + -0x5c],EAX
008c8c2f  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8c35  8b55a4                MOV EDX,dword ptr [EBP + -0x5c]
008c8c38  8991800c0000          MOV dword ptr [ECX + 0xc80],EDX
008c8c3e  8b45e0                MOV EAX,dword ptr [EBP + -0x20]
008c8c41  50                    PUSH EAX
008c8c42  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8c45  e80653caff            CALL 0x0056df50
008c8c4a  8b4ddc                MOV ECX,dword ptr [EBP + -0x24]
008c8c4d  51                    PUSH ECX
008c8c4e  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8c51  e8fa52caff            CALL 0x0056df50
008c8c56  e989020000            JMP 0x008c8ee4
008c8c5b  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8c61  8b82900c0000          MOV EAX,dword ptr [EDX + 0xc90]
008c8c67  8b88d8010000          MOV ECX,dword ptr [EAX + 0x1d8]
008c8c6d  894da0                MOV dword ptr [EBP + -0x60],ECX
008c8c70  817da000000400        CMP dword ptr [EBP + -0x60],0x40000
008c8c77  7422                  JZ 0x008c8c9b
008c8c79  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8c7f  8b82900c0000          MOV EAX,dword ptr [EDX + 0xc90]
008c8c85  8b88d8010000          MOV ECX,dword ptr [EAX + 0x1d8]
008c8c8b  894d9c                MOV dword ptr [EBP + -0x64],ECX
008c8c8e  817d9c00000800        CMP dword ptr [EBP + -0x64],0x80000
008c8c95  0f85e3010000          JNZ 0x008c8e7e
008c8c9b  8b55e4                MOV EDX,dword ptr [EBP + -0x1c]
008c8c9e  8b8250110000          MOV EAX,dword ptr [EDX + 0x1150]
008c8ca4  894598                MOV dword ptr [EBP + -0x68],EAX
008c8ca7  6800000400            PUSH 0x40000
008c8cac  8b4d98                MOV ECX,dword ptr [EBP + -0x68]
008c8caf  e86c79e0ff            CALL 0x006d0620
008c8cb4  8945d4                MOV dword ptr [EBP + -0x2c],EAX
008c8cb7  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8cba  8b9150110000          MOV EDX,dword ptr [ECX + 0x1150]
008c8cc0  895594                MOV dword ptr [EBP + -0x6c],EDX
008c8cc3  6800000800            PUSH 0x80000
008c8cc8  8b4d94                MOV ECX,dword ptr [EBP + -0x6c]
008c8ccb  e85079e0ff            CALL 0x006d0620
008c8cd0  8945d8                MOV dword ptr [EBP + -0x28],EAX
008c8cd3  837dd800              CMP dword ptr [EBP + -0x28],0x0
008c8cd7  7572                  JNZ 0x008c8d4b
008c8cd9  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8cdf  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8ce5  83c904                OR ECX,0x4
008c8ce8  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8cee  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8cf4  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8cfa  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8d00  83e1fe                AND ECX,0xfffffffe
008c8d03  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8d09  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8d0f  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8d15  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8d1b  83e1fd                AND ECX,0xfffffffd
008c8d1e  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8d24  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8d2a  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8d30  c780800c00000000007f  MOV dword ptr [EAX + 0xc80],0x7f000000
008c8d3a  8b4dd4                MOV ECX,dword ptr [EBP + -0x2c]
008c8d3d  51                    PUSH ECX
008c8d3e  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8d41  e80a52caff            CALL 0x0056df50
008c8d46  e931010000            JMP 0x008c8e7c
008c8d4b  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8d51  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008c8d54  3b828c0c0000          CMP EAX,dword ptr [EDX + 0xc8c]
008c8d5a  0f8586000000          JNZ 0x008c8de6
008c8d60  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8d66  8b917c0c0000          MOV EDX,dword ptr [ECX + 0xc7c]
008c8d6c  83ca04                OR EDX,0x4
008c8d6f  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8d75  89907c0c0000          MOV dword ptr [EAX + 0xc7c],EDX
008c8d7b  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8d81  8b917c0c0000          MOV EDX,dword ptr [ECX + 0xc7c]
008c8d87  83ca01                OR EDX,0x1
008c8d8a  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8d90  89907c0c0000          MOV dword ptr [EAX + 0xc7c],EDX
008c8d96  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8d9c  8b917c0c0000          MOV EDX,dword ptr [ECX + 0xc7c]
008c8da2  83e2fd                AND EDX,0xfffffffd
008c8da5  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8dab  89907c0c0000          MOV dword ptr [EAX + 0xc7c],EDX
008c8db1  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008c8db4  8b5114                MOV EDX,dword ptr [ECX + 0x14]
008c8db7  895590                MOV dword ptr [EBP + -0x70],EDX
008c8dba  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8dc0  8b4d90                MOV ECX,dword ptr [EBP + -0x70]
008c8dc3  8988800c0000          MOV dword ptr [EAX + 0xc80],ECX
008c8dc9  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
008c8dcc  52                    PUSH EDX
008c8dcd  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8dd0  e87b51caff            CALL 0x0056df50
008c8dd5  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008c8dd8  50                    PUSH EAX
008c8dd9  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8ddc  e86f51caff            CALL 0x0056df50
008c8de1  e996000000            JMP 0x008c8e7c
008c8de6  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8dec  8b55d8                MOV EDX,dword ptr [EBP + -0x28]
008c8def  3b918c0c0000          CMP EDX,dword ptr [ECX + 0xc8c]
008c8df5  0f8581000000          JNZ 0x008c8e7c
008c8dfb  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8e01  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8e07  83c904                OR ECX,0x4
008c8e0a  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8e10  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8e16  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8e1c  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8e22  83c901                OR ECX,0x1
008c8e25  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8e2b  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8e31  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8e37  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8e3d  83c902                OR ECX,0x2
008c8e40  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8e46  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8e4c  8b45d4                MOV EAX,dword ptr [EBP + -0x2c]
008c8e4f  8b4814                MOV ECX,dword ptr [EAX + 0x14]
008c8e52  894d8c                MOV dword ptr [EBP + -0x74],ECX
008c8e55  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8e5b  8b458c                MOV EAX,dword ptr [EBP + -0x74]
008c8e5e  8982800c0000          MOV dword ptr [EDX + 0xc80],EAX
008c8e64  8b4dd8                MOV ECX,dword ptr [EBP + -0x28]
008c8e67  51                    PUSH ECX
008c8e68  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8e6b  e8e050caff            CALL 0x0056df50
008c8e70  8b55d4                MOV EDX,dword ptr [EBP + -0x2c]
008c8e73  52                    PUSH EDX
008c8e74  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8e77  e8d450caff            CALL 0x0056df50
008c8e7c  eb66                  JMP 0x008c8ee4
008c8e7e  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8e84  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8e8a  83e1fb                AND ECX,0xfffffffb
008c8e8d  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8e93  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8e99  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8e9f  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8ea5  83e1fe                AND ECX,0xfffffffe
008c8ea8  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8eae  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8eb4  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8eba  8b887c0c0000          MOV ECX,dword ptr [EAX + 0xc7c]
008c8ec0  83e1fd                AND ECX,0xfffffffd
008c8ec3  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8ec9  898a7c0c0000          MOV dword ptr [EDX + 0xc7c],ECX
008c8ecf  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8ed5  8b888c0c0000          MOV ECX,dword ptr [EAX + 0xc8c]
008c8edb  51                    PUSH ECX
008c8edc  8b4de4                MOV ECX,dword ptr [EBP + -0x1c]
008c8edf  e86c50caff            CALL 0x0056df50
008c8ee4  eb5f                  JMP 0x008c8f45
008c8ee6  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8eec  8b828c0c0000          MOV EAX,dword ptr [EDX + 0xc8c]
008c8ef2  8b88cc020000          MOV ECX,dword ptr [EAX + 0x2cc]
008c8ef8  894d88                MOV dword ptr [EBP + -0x78],ECX
008c8efb  837d8801              CMP dword ptr [EBP + -0x78],0x1
008c8eff  7e21                  JLE 0x008c8f22
008c8f01  6a01                  PUSH 0x1
008c8f03  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8f09  8b8a8c0c0000          MOV ECX,dword ptr [EDX + 0xc8c]
008c8f0f  e8acbfd3ff            CALL 0x00604ec0
008c8f14  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8f1a  89818c0c0000          MOV dword ptr [ECX + 0xc8c],EAX
008c8f20  eb23                  JMP 0x008c8f45
008c8f22  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8f28  8b828c0c0000          MOV EAX,dword ptr [EDX + 0xc8c]
008c8f2e  50                    PUSH EAX
008c8f2f  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
008c8f32  e89939c5ff            CALL 0x0051c8d0
008c8f37  8bc8                  MOV ECX,EAX
008c8f39  e8f2ead2ff            CALL 0x005f7a30
008c8f3e  8bc8                  MOV ECX,EAX
008c8f40  e83bcfd3ff            CALL 0x00605e80
008c8f45  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8f4b  8a91780c0000          MOV DL,byte ptr [ECX + 0xc78]
008c8f51  885583                MOV byte ptr [EBP + -0x7d],DL
008c8f54  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8f5a  8b88940c0000          MOV ECX,dword ptr [EAX + 0xc94]
008c8f60  894d84                MOV dword ptr [EBP + -0x7c],ECX
008c8f63  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
008c8f66  8a4583                MOV AL,byte ptr [EBP + -0x7d]
008c8f69  88822c3d0000          MOV byte ptr [EDX + 0x3d2c],AL
008c8f6f  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8f75  8b918c0c0000          MOV EDX,dword ptr [ECX + 0xc8c]
008c8f7b  899578ffffff          MOV dword ptr [EBP + 0xffffff78],EDX
008c8f81  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8f87  8b88940c0000          MOV ECX,dword ptr [EAX + 0xc94]
008c8f8d  898d7cffffff          MOV dword ptr [EBP + 0xffffff7c],ECX
008c8f93  8b957cffffff          MOV EDX,dword ptr [EBP + 0xffffff7c]
008c8f99  8b8578ffffff          MOV EAX,dword ptr [EBP + 0xffffff78]
008c8f9f  8982343d0000          MOV dword ptr [EDX + 0x3d34],EAX
008c8fa5  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8fab  8b511c                MOV EDX,dword ptr [ECX + 0x1c]
008c8fae  899574ffffff          MOV dword ptr [EBP + 0xffffff74],EDX
008c8fb4  6a01                  PUSH 0x1
008c8fb6  6a03                  PUSH 0x3
008c8fb8  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c8fbe  8b88940c0000          MOV ECX,dword ptr [EAX + 0xc94]
008c8fc4  51                    PUSH ECX
008c8fc5  8b8d74ffffff          MOV ECX,dword ptr [EBP + 0xffffff74]
008c8fcb  e86075b4ff            CALL 0x00410530
008c8fd0  eb47                  JMP 0x008c9019
008c8fd2  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8fd8  e853000000            CALL 0x008c9030
008c8fdd  6a00                  PUSH 0x0
008c8fdf  6a00                  PUSH 0x0
008c8fe1  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c8fe7  83c168                ADD ECX,0x68
008c8fea  e88158b5ff            CALL 0x0041e870
008c8fef  6a00                  PUSH 0x0
008c8ff1  8b9570ffffff          MOV EDX,dword ptr [EBP + 0xffffff70]
008c8ff7  83c268                ADD EDX,0x68
008c8ffa  52                    PUSH EDX
008c8ffb  8b8570ffffff          MOV EAX,dword ptr [EBP + 0xffffff70]
008c9001  8b10                  MOV EDX,dword ptr [EAX]
008c9003  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c9009  8b4208                MOV EAX,dword ptr [EDX + 0x8]
008c900c  ffd0                  CALL EAX
008c900e  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
008c9014  e8a763b4ff            CALL 0x0040f3c0
008c9019  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
008c901c  64890d00000000        MOV dword ptr FS:[0x0],ECX
008c9023  59                    POP ECX
008c9024  8be5                  MOV ESP,EBP
008c9026  5d                    POP EBP
008c9027  c3                    RET
