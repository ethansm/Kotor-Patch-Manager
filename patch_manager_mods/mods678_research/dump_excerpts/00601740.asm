; FN 00601740 FUN_00601740
00601740  55                    PUSH EBP
00601741  8bec                  MOV EBP,ESP
00601743  6aff                  PUSH -0x1
00601745  68ab4e9500            PUSH 0x954eab
0060174a  64a100000000          MOV EAX,FS:[0x0]
00601750  50                    PUSH EAX
00601751  81ecd8010000          SUB ESP,0x1d8
00601757  a1201fa100            MOV EAX,[0x00a11f20]
0060175c  33c5                  XOR EAX,EBP
0060175e  8945e8                MOV dword ptr [EBP + -0x18],EAX
00601761  50                    PUSH EAX
00601762  8d45f4                LEA EAX,[EBP + -0xc]
00601765  64a300000000          MOV FS:[0x0],EAX
0060176b  898d40feffff          MOV dword ptr [EBP + 0xfffffe40],ECX
00601771  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601777  8b480c                MOV ECX,dword ptr [EAX + 0xc]
0060177a  894d84                MOV dword ptr [EBP + -0x7c],ECX
0060177d  8b5584                MOV EDX,dword ptr [EBP + -0x7c]
00601780  52                    PUSH EDX
00601781  8d45cc                LEA EAX,[EBP + -0x34]
00601784  50                    PUSH EAX
00601785  6878759900            PUSH 0x997578
0060178a  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0060178d  51                    PUSH ECX
0060178e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601791  e8ea741100            CALL 0x00718c80
00601796  894580                MOV dword ptr [EBP + -0x80],EAX
00601799  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
0060179f  8b4580                MOV EAX,dword ptr [EBP + -0x80]
006017a2  89420c                MOV dword ptr [EDX + 0xc],EAX
006017a5  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006017ab  8b5114                MOV EDX,dword ptr [ECX + 0x14]
006017ae  89957cffffff          MOV dword ptr [EBP + 0xffffff7c],EDX
006017b4  8b857cffffff          MOV EAX,dword ptr [EBP + 0xffffff7c]
006017ba  50                    PUSH EAX
006017bb  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006017c1  e82afdffff            CALL 0x006014f0
006017c6  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006017cc  83c128                ADD ECX,0x28
006017cf  51                    PUSH ECX
006017d0  8d55cc                LEA EDX,[EBP + -0x34]
006017d3  52                    PUSH EDX
006017d4  6838359900            PUSH 0x993538
006017d9  8b450c                MOV EAX,dword ptr [EBP + 0xc]
006017dc  50                    PUSH EAX
006017dd  8d4da0                LEA ECX,[EBP + -0x60]
006017e0  51                    PUSH ECX
006017e1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006017e4  e8a7771100            CALL 0x00718f90
006017e9  89853cfeffff          MOV dword ptr [EBP + 0xfffffe3c],EAX
006017ef  8b953cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe3c]
006017f5  899570ffffff          MOV dword ptr [EBP + 0xffffff70],EDX
006017fb  c745fc00000000        MOV dword ptr [EBP + -0x4],0x0
00601802  8d8574ffffff          LEA EAX,[EBP + 0xffffff74]
00601808  50                    PUSH EAX
00601809  8b8d70ffffff          MOV ECX,dword ptr [EBP + 0xffffff70]
0060180f  e87c2e1300            CALL 0x00734690
00601814  898538feffff          MOV dword ptr [EBP + 0xfffffe38],EAX
0060181a  8b8d38feffff          MOV ECX,dword ptr [EBP + 0xfffffe38]
00601820  898d34feffff          MOV dword ptr [EBP + 0xfffffe34],ECX
00601826  c645fc01              MOV byte ptr [EBP + -0x4],0x1
0060182a  8b9534feffff          MOV EDX,dword ptr [EBP + 0xfffffe34]
00601830  52                    PUSH EDX
00601831  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601837  83c128                ADD ECX,0x28
0060183a  e8811f1300            CALL 0x007337c0
0060183f  c645fc00              MOV byte ptr [EBP + -0x4],0x0
00601843  8d8d74ffffff          LEA ECX,[EBP + 0xffffff74]
00601849  e8321f1300            CALL 0x00733780
0060184e  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00601855  8d4da0                LEA ECX,[EBP + -0x60]
00601858  e8231f1300            CALL 0x00733780
0060185d  6a01                  PUSH 0x1
0060185f  8d45cc                LEA EAX,[EBP + -0x34]
00601862  50                    PUSH EAX
00601863  686c759900            PUSH 0x99756c
00601868  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
0060186b  51                    PUSH ECX
0060186c  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060186f  e88c711100            CALL 0x00718a00
00601874  0fb6d0                MOVZX EDX,AL
00601877  52                    PUSH EDX
00601878  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060187e  e87d540000            CALL 0x00606d00
00601883  b801000000            MOV EAX,0x1
00601888  85c0                  TEST EAX,EAX
0060188a  7414                  JZ 0x006018a0
0060188c  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601892  81c1b0020000          ADD ECX,0x2b0
00601898  898d30feffff          MOV dword ptr [EBP + 0xfffffe30],ECX
0060189e  eb12                  JMP 0x006018b2
006018a0  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
006018a6  81c2b8020000          ADD EDX,0x2b8
006018ac  899530feffff          MOV dword ptr [EBP + 0xfffffe30],EDX
006018b2  8b8530feffff          MOV EAX,dword ptr [EBP + 0xfffffe30]
006018b8  89856cffffff          MOV dword ptr [EBP + 0xffffff6c],EAX
006018be  8b8d6cffffff          MOV ECX,dword ptr [EBP + 0xffffff6c]
006018c4  51                    PUSH ECX
006018c5  8d55cc                LEA EDX,[EBP + -0x34]
006018c8  52                    PUSH EDX
006018c9  68406c9900            PUSH 0x996c40
006018ce  8b450c                MOV EAX,dword ptr [EBP + 0xc]
006018d1  50                    PUSH EAX
006018d2  8d4d98                LEA ECX,[EBP + -0x68]
006018d5  51                    PUSH ECX
006018d6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006018d9  e802781100            CALL 0x007190e0
006018de  89852cfeffff          MOV dword ptr [EBP + 0xfffffe2c],EAX
006018e4  8b952cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe2c]
006018ea  899568ffffff          MOV dword ptr [EBP + 0xffffff68],EDX
006018f0  c745fc02000000        MOV dword ptr [EBP + -0x4],0x2
006018f7  8b8568ffffff          MOV EAX,dword ptr [EBP + 0xffffff68]
006018fd  50                    PUSH EAX
006018fe  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601904  81c1b8020000          ADD ECX,0x2b8
0060190a  e8f13a1300            CALL 0x00735400
0060190f  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00601916  8d4d98                LEA ECX,[EBP + -0x68]
00601919  e8423a1300            CALL 0x00735360
0060191e  33c9                  XOR ECX,ECX
00601920  7414                  JZ 0x00601936
00601922  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601928  81c2b0020000          ADD EDX,0x2b0
0060192e  899528feffff          MOV dword ptr [EBP + 0xfffffe28],EDX
00601934  eb11                  JMP 0x00601947
00601936  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
0060193c  05b8020000            ADD EAX,0x2b8
00601941  898528feffff          MOV dword ptr [EBP + 0xfffffe28],EAX
00601947  8b8d28feffff          MOV ECX,dword ptr [EBP + 0xfffffe28]
0060194d  898d64ffffff          MOV dword ptr [EBP + 0xffffff64],ECX
00601953  8b9564ffffff          MOV EDX,dword ptr [EBP + 0xffffff64]
00601959  52                    PUSH EDX
0060195a  8d45cc                LEA EAX,[EBP + -0x34]
0060195d  50                    PUSH EAX
0060195e  685c759900            PUSH 0x99755c
00601963  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601966  51                    PUSH ECX
00601967  8d5590                LEA EDX,[EBP + -0x70]
0060196a  52                    PUSH EDX
0060196b  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060196e  e86d771100            CALL 0x007190e0
00601973  898524feffff          MOV dword ptr [EBP + 0xfffffe24],EAX
00601979  8b8524feffff          MOV EAX,dword ptr [EBP + 0xfffffe24]
0060197f  898560ffffff          MOV dword ptr [EBP + 0xffffff60],EAX
00601985  c745fc03000000        MOV dword ptr [EBP + -0x4],0x3
0060198c  8b8d60ffffff          MOV ECX,dword ptr [EBP + 0xffffff60]
00601992  51                    PUSH ECX
00601993  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601999  81c1b0020000          ADD ECX,0x2b0
0060199f  e85c3a1300            CALL 0x00735400
006019a4  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
006019ab  8d4d90                LEA ECX,[EBP + -0x70]
006019ae  e8ad391300            CALL 0x00735360
006019b3  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
006019b9  81c2c0020000          ADD EDX,0x2c0
006019bf  52                    PUSH EDX
006019c0  8d45cc                LEA EAX,[EBP + -0x34]
006019c3  50                    PUSH EAX
006019c4  684c759900            PUSH 0x99754c
006019c9  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
006019cc  51                    PUSH ECX
006019cd  8d5588                LEA EDX,[EBP + -0x78]
006019d0  52                    PUSH EDX
006019d1  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006019d4  e807771100            CALL 0x007190e0
006019d9  898520feffff          MOV dword ptr [EBP + 0xfffffe20],EAX
006019df  8b8520feffff          MOV EAX,dword ptr [EBP + 0xfffffe20]
006019e5  89855cffffff          MOV dword ptr [EBP + 0xffffff5c],EAX
006019eb  c745fc04000000        MOV dword ptr [EBP + -0x4],0x4
006019f2  8b8d5cffffff          MOV ECX,dword ptr [EBP + 0xffffff5c]
006019f8  51                    PUSH ECX
006019f9  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006019ff  81c1c0020000          ADD ECX,0x2c0
00601a05  e8f6391300            CALL 0x00735400
00601a0a  c745fcffffffff        MOV dword ptr [EBP + -0x4],0xffffffff
00601a11  8d4d88                LEA ECX,[EBP + -0x78]
00601a14  e847391300            CALL 0x00735360
00601a19  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601a1f  8b82cc020000          MOV EAX,dword ptr [EDX + 0x2cc]
00601a25  898558ffffff          MOV dword ptr [EBP + 0xffffff58],EAX
00601a2b  0fb78d58ffffff        MOVZX ECX,word ptr [EBP + 0xffffff58]
00601a32  51                    PUSH ECX
00601a33  8d55cc                LEA EDX,[EBP + -0x34]
00601a36  52                    PUSH EDX
00601a37  6840759900            PUSH 0x997540
00601a3c  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601a3f  50                    PUSH EAX
00601a40  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601a43  e8b8701100            CALL 0x00718b00
00601a48  0fb7c8                MOVZX ECX,AX
00601a4b  898d54ffffff          MOV dword ptr [EBP + 0xffffff54],ECX
00601a51  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601a57  8b8554ffffff          MOV EAX,dword ptr [EBP + 0xffffff54]
00601a5d  8982cc020000          MOV dword ptr [EDX + 0x2cc],EAX
00601a63  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601a69  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00601a6f  c1ea05                SHR EDX,0x5
00601a72  83e201                AND EDX,0x1
00601a75  899550ffffff          MOV dword ptr [EBP + 0xffffff50],EDX
00601a7b  0fb68550ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff50]
00601a82  50                    PUSH EAX
00601a83  8d4dcc                LEA ECX,[EBP + -0x34]
00601a86  51                    PUSH ECX
00601a87  6838759900            PUSH 0x997538
00601a8c  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00601a8f  52                    PUSH EDX
00601a90  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601a93  e8686f1100            CALL 0x00718a00
00601a98  0fb6c0                MOVZX EAX,AL
00601a9b  89854cffffff          MOV dword ptr [EBP + 0xffffff4c],EAX
00601aa1  8b8d4cffffff          MOV ECX,dword ptr [EBP + 0xffffff4c]
00601aa7  83e101                AND ECX,0x1
00601aaa  c1e105                SHL ECX,0x5
00601aad  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601ab3  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00601ab9  83e0df                AND EAX,0xffffffdf
00601abc  0bc1                  OR EAX,ECX
00601abe  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601ac4  8981c8020000          MOV dword ptr [ECX + 0x2c8],EAX
00601aca  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601ad0  8b82d4020000          MOV EAX,dword ptr [EDX + 0x2d4]
00601ad6  898548ffffff          MOV dword ptr [EBP + 0xffffff48],EAX
00601adc  8b8d48ffffff          MOV ECX,dword ptr [EBP + 0xffffff48]
00601ae2  51                    PUSH ECX
00601ae3  8d55cc                LEA EDX,[EBP + -0x34]
00601ae6  52                    PUSH EDX
00601ae7  682c759900            PUSH 0x99752c
00601aec  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601aef  50                    PUSH EAX
00601af0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601af3  e808711100            CALL 0x00718c00
00601af8  898544ffffff          MOV dword ptr [EBP + 0xffffff44],EAX
00601afe  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601b04  8b9544ffffff          MOV EDX,dword ptr [EBP + 0xffffff44]
00601b0a  8991d4020000          MOV dword ptr [ECX + 0x2d4],EDX
00601b10  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601b16  8b88d4020000          MOV ECX,dword ptr [EAX + 0x2d4]
00601b1c  898d40ffffff          MOV dword ptr [EBP + 0xffffff40],ECX
00601b22  0fb69540ffffff        MOVZX EDX,byte ptr [EBP + 0xffffff40]
00601b29  52                    PUSH EDX
00601b2a  8d45cc                LEA EAX,[EBP + -0x34]
00601b2d  50                    PUSH EAX
00601b2e  681c759900            PUSH 0x99751c
00601b33  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601b36  51                    PUSH ECX
00601b37  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601b3a  e8c16e1100            CALL 0x00718a00
00601b3f  0fb6d0                MOVZX EDX,AL
00601b42  89953cffffff          MOV dword ptr [EBP + 0xffffff3c],EDX
00601b48  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601b4e  8b8d3cffffff          MOV ECX,dword ptr [EBP + 0xffffff3c]
00601b54  8988f0020000          MOV dword ptr [EAX + 0x2f0],ECX
00601b5a  6aff                  PUSH -0x1
00601b5c  8d55cc                LEA EDX,[EBP + -0x34]
00601b5f  52                    PUSH EDX
00601b60  680c759900            PUSH 0x99750c
00601b65  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601b68  50                    PUSH EAX
00601b69  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601b6c  e80f711100            CALL 0x00718c80
00601b71  898538ffffff          MOV dword ptr [EBP + 0xffffff38],EAX
00601b77  33c9                  XOR ECX,ECX
00601b79  c1e102                SHL ECX,0x2
00601b7c  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601b82  8b8538ffffff          MOV EAX,dword ptr [EBP + 0xffffff38]
00601b88  89840af4020000        MOV dword ptr [EDX + ECX*0x1 + 0x2f4],EAX
00601b8f  6aff                  PUSH -0x1
00601b91  8d4dcc                LEA ECX,[EBP + -0x34]
00601b94  51                    PUSH ECX
00601b95  68fc749900            PUSH 0x9974fc
00601b9a  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00601b9d  52                    PUSH EDX
00601b9e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601ba1  e8da701100            CALL 0x00718c80
00601ba6  898534ffffff          MOV dword ptr [EBP + 0xffffff34],EAX
00601bac  b801000000            MOV EAX,0x1
00601bb1  c1e002                SHL EAX,0x2
00601bb4  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601bba  8b9534ffffff          MOV EDX,dword ptr [EBP + 0xffffff34]
00601bc0  899401f4020000        MOV dword ptr [ECX + EAX*0x1 + 0x2f4],EDX
00601bc7  6aff                  PUSH -0x1
00601bc9  8d45cc                LEA EAX,[EBP + -0x34]
00601bcc  50                    PUSH EAX
00601bcd  68ec749900            PUSH 0x9974ec
00601bd2  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601bd5  51                    PUSH ECX
00601bd6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601bd9  e8a2701100            CALL 0x00718c80
00601bde  898530ffffff          MOV dword ptr [EBP + 0xffffff30],EAX
00601be4  ba02000000            MOV EDX,0x2
00601be9  c1e202                SHL EDX,0x2
00601bec  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601bf2  8b8d30ffffff          MOV ECX,dword ptr [EBP + 0xffffff30]
00601bf8  898c10f4020000        MOV dword ptr [EAX + EDX*0x1 + 0x2f4],ECX
00601bff  6aff                  PUSH -0x1
00601c01  8d55cc                LEA EDX,[EBP + -0x34]
00601c04  52                    PUSH EDX
00601c05  68dc749900            PUSH 0x9974dc
00601c0a  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601c0d  50                    PUSH EAX
00601c0e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601c11  e86a701100            CALL 0x00718c80
00601c16  89852cffffff          MOV dword ptr [EBP + 0xffffff2c],EAX
00601c1c  b903000000            MOV ECX,0x3
00601c21  c1e102                SHL ECX,0x2
00601c24  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601c2a  8b852cffffff          MOV EAX,dword ptr [EBP + 0xffffff2c]
00601c30  89840af4020000        MOV dword ptr [EDX + ECX*0x1 + 0x2f4],EAX
00601c37  6aff                  PUSH -0x1
00601c39  8d4dcc                LEA ECX,[EBP + -0x34]
00601c3c  51                    PUSH ECX
00601c3d  68cc749900            PUSH 0x9974cc
00601c42  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00601c45  52                    PUSH EDX
00601c46  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601c49  e832701100            CALL 0x00718c80
00601c4e  898528ffffff          MOV dword ptr [EBP + 0xffffff28],EAX
00601c54  b804000000            MOV EAX,0x4
00601c59  c1e002                SHL EAX,0x2
00601c5c  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601c62  8b9528ffffff          MOV EDX,dword ptr [EBP + 0xffffff28]
00601c68  899401f4020000        MOV dword ptr [ECX + EAX*0x1 + 0x2f4],EDX
00601c6f  6aff                  PUSH -0x1
00601c71  8d45cc                LEA EAX,[EBP + -0x34]
00601c74  50                    PUSH EAX
00601c75  68bc749900            PUSH 0x9974bc
00601c7a  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601c7d  51                    PUSH ECX
00601c7e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601c81  e8fa6f1100            CALL 0x00718c80
00601c86  898524ffffff          MOV dword ptr [EBP + 0xffffff24],EAX
00601c8c  ba05000000            MOV EDX,0x5
00601c91  c1e202                SHL EDX,0x2
00601c94  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601c9a  8b8d24ffffff          MOV ECX,dword ptr [EBP + 0xffffff24]
00601ca0  898c10f4020000        MOV dword ptr [EAX + EDX*0x1 + 0x2f4],ECX
00601ca7  6a00                  PUSH 0x0
00601ca9  8d55cc                LEA EDX,[EBP + -0x34]
00601cac  52                    PUSH EDX
00601cad  6854539900            PUSH 0x995354
00601cb2  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601cb5  50                    PUSH EAX
00601cb6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601cb9  e8426d1100            CALL 0x00718a00
00601cbe  0fb6c8                MOVZX ECX,AL
00601cc1  898d20ffffff          MOV dword ptr [EBP + 0xffffff20],ECX
00601cc7  8b9520ffffff          MOV EDX,dword ptr [EBP + 0xffffff20]
00601ccd  83e201                AND EDX,0x1
00601cd0  c1e203                SHL EDX,0x3
00601cd3  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601cd9  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
00601cdf  83e1f7                AND ECX,0xfffffff7
00601ce2  0bca                  OR ECX,EDX
00601ce4  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601cea  898ac8020000          MOV dword ptr [EDX + 0x2c8],ECX
00601cf0  6a00                  PUSH 0x0
00601cf2  8d45cc                LEA EAX,[EBP + -0x34]
00601cf5  50                    PUSH EAX
00601cf6  68ac749900            PUSH 0x9974ac
00601cfb  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601cfe  51                    PUSH ECX
00601cff  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601d02  e8f96c1100            CALL 0x00718a00
00601d07  0fb6d0                MOVZX EDX,AL
00601d0a  89951cffffff          MOV dword ptr [EBP + 0xffffff1c],EDX
00601d10  8b851cffffff          MOV EAX,dword ptr [EBP + 0xffffff1c]
00601d16  83e001                AND EAX,0x1
00601d19  c1e004                SHL EAX,0x4
00601d1c  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601d22  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00601d28  83e2ef                AND EDX,0xffffffef
00601d2b  0bd0                  OR EDX,EAX
00601d2d  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601d33  8990c8020000          MOV dword ptr [EAX + 0x2c8],EDX
00601d39  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601d3f  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00601d45  c1ea06                SHR EDX,0x6
00601d48  83e201                AND EDX,0x1
00601d4b  899518ffffff          MOV dword ptr [EBP + 0xffffff18],EDX
00601d51  0fb68518ffffff        MOVZX EAX,byte ptr [EBP + 0xffffff18]
00601d58  50                    PUSH EAX
00601d59  8d4dcc                LEA ECX,[EBP + -0x34]
00601d5c  51                    PUSH ECX
00601d5d  689c749900            PUSH 0x99749c
00601d62  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00601d65  52                    PUSH EDX
00601d66  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601d69  e8926c1100            CALL 0x00718a00
00601d6e  0fb6c0                MOVZX EAX,AL
00601d71  898514ffffff          MOV dword ptr [EBP + 0xffffff14],EAX
00601d77  8b8d14ffffff          MOV ECX,dword ptr [EBP + 0xffffff14]
00601d7d  83e101                AND ECX,0x1
00601d80  c1e106                SHL ECX,0x6
00601d83  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601d89  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00601d8f  83e0bf                AND EAX,0xffffffbf
00601d92  0bc1                  OR EAX,ECX
00601d94  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601d9a  8981c8020000          MOV dword ptr [ECX + 0x2c8],EAX
00601da0  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601da6  8a82a4020000          MOV AL,byte ptr [EDX + 0x2a4]
00601dac  888513ffffff          MOV byte ptr [EBP + 0xffffff13],AL
00601db2  0fb68d13ffffff        MOVZX ECX,byte ptr [EBP + 0xffffff13]
00601db9  51                    PUSH ECX
00601dba  8d55cc                LEA EDX,[EBP + -0x34]
00601dbd  52                    PUSH EDX
00601dbe  688c749900            PUSH 0x99748c
00601dc3  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601dc6  50                    PUSH EAX
00601dc7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601dca  e8316c1100            CALL 0x00718a00
00601dcf  8845ab                MOV byte ptr [EBP + -0x55],AL
00601dd2  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00601dd6  7529                  JNZ 0x00601e01
00601dd8  0fb64dab              MOVZX ECX,byte ptr [EBP + -0x55]
00601ddc  51                    PUSH ECX
00601ddd  8d55cc                LEA EDX,[EBP + -0x34]
00601de0  52                    PUSH EDX
00601de1  6880749900            PUSH 0x997480
00601de6  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601de9  50                    PUSH EAX
00601dea  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601ded  e80e6c1100            CALL 0x00718a00
00601df2  8845ab                MOV byte ptr [EBP + -0x55],AL
00601df5  0fb64dab              MOVZX ECX,byte ptr [EBP + -0x55]
00601df9  85c9                  TEST ECX,ECX
00601dfb  7504                  JNZ 0x00601e01
00601dfd  c645ab01              MOV byte ptr [EBP + -0x55],0x1
00601e01  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601e07  8a45ab                MOV AL,byte ptr [EBP + -0x55]
00601e0a  8882a4020000          MOV byte ptr [EDX + 0x2a4],AL
00601e10  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601e16  8b510c                MOV EDX,dword ptr [ECX + 0xc]
00601e19  89950cffffff          MOV dword ptr [EBP + 0xffffff0c],EDX
00601e1f  a1d0b4a100            MOV EAX,[0x00a1b4d0]
00601e24  898508ffffff          MOV dword ptr [EBP + 0xffffff08],EAX
00601e2a  8b8d0cffffff          MOV ECX,dword ptr [EBP + 0xffffff0c]
00601e30  51                    PUSH ECX
00601e31  8b9508ffffff          MOV EDX,dword ptr [EBP + 0xffffff08]
00601e37  8b4a34                MOV ECX,dword ptr [EDX + 0x34]
00601e3a  e8c1330d00            CALL 0x006d5200
00601e3f  898504ffffff          MOV dword ptr [EBP + 0xffffff04],EAX
00601e45  8b8504ffffff          MOV EAX,dword ptr [EBP + 0xffffff04]
00601e4b  8a4818                MOV CL,byte ptr [EAX + 0x18]
00601e4e  888d03ffffff          MOV byte ptr [EBP + 0xffffff03],CL
00601e54  8a9503ffffff          MOV DL,byte ptr [EBP + 0xffffff03]
00601e5a  88951cfeffff          MOV byte ptr [EBP + 0xfffffe1c],DL
00601e60  80bd1cfeffff01        CMP byte ptr [EBP + 0xfffffe1c],0x1
00601e67  7405                  JZ 0x00601e6e
00601e69  e989000000            JMP 0x00601ef7
00601e6e  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601e74  8b480c                MOV ECX,dword ptr [EAX + 0xc]
00601e77  898dfcfeffff          MOV dword ptr [EBP + 0xfffffefc],ECX
00601e7d  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
00601e83  8995f8feffff          MOV dword ptr [EBP + 0xfffffef8],EDX
00601e89  8b85fcfeffff          MOV EAX,dword ptr [EBP + 0xfffffefc]
00601e8f  50                    PUSH EAX
00601e90  8b8df8feffff          MOV ECX,dword ptr [EBP + 0xfffffef8]
00601e96  8b4934                MOV ECX,dword ptr [ECX + 0x34]
00601e99  e862330d00            CALL 0x006d5200
00601e9e  8985f4feffff          MOV dword ptr [EBP + 0xfffffef4],EAX
00601ea4  8b95f4feffff          MOV EDX,dword ptr [EBP + 0xfffffef4]
00601eaa  8a82af000000          MOV AL,byte ptr [EDX + 0xaf]
00601eb0  8885f3feffff          MOV byte ptr [EBP + 0xfffffef3],AL
00601eb6  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601ebc  8a95f3feffff          MOV DL,byte ptr [EBP + 0xfffffef3]
00601ec2  8891a5020000          MOV byte ptr [ECX + 0x2a5],DL
00601ec8  6a01                  PUSH 0x1
00601eca  8d45cc                LEA EAX,[EBP + -0x34]
00601ecd  50                    PUSH EAX
00601ece  6874749900            PUSH 0x997474
00601ed3  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601ed6  51                    PUSH ECX
00601ed7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601eda  e8216b1100            CALL 0x00718a00
00601edf  8885f2feffff          MOV byte ptr [EBP + 0xfffffef2],AL
00601ee5  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601eeb  8a85f2feffff          MOV AL,byte ptr [EBP + 0xfffffef2]
00601ef1  8882a6020000          MOV byte ptr [EDX + 0x2a6],AL
00601ef7  6a32                  PUSH 0x32
00601ef9  8d4dcc                LEA ECX,[EBP + -0x34]
00601efc  51                    PUSH ECX
00601efd  686c749900            PUSH 0x99746c
00601f02  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00601f05  52                    PUSH EDX
00601f06  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601f09  e8f26a1100            CALL 0x00718a00
00601f0e  0fb6c0                MOVZX EAX,AL
00601f11  8985ecfeffff          MOV dword ptr [EBP + 0xfffffeec],EAX
00601f17  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00601f1d  8b95ecfeffff          MOV EDX,dword ptr [EBP + 0xfffffeec]
00601f23  899198020000          MOV dword ptr [ECX + 0x298],EDX
00601f29  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601f2f  8b8898020000          MOV ECX,dword ptr [EAX + 0x298]
00601f35  898de8feffff          MOV dword ptr [EBP + 0xfffffee8],ECX
00601f3b  0fb695e8feffff        MOVZX EDX,byte ptr [EBP + 0xfffffee8]
00601f42  52                    PUSH EDX
00601f43  8d45cc                LEA EAX,[EBP + -0x34]
00601f46  50                    PUSH EAX
00601f47  6860749900            PUSH 0x997460
00601f4c  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00601f4f  51                    PUSH ECX
00601f50  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601f53  e8a86a1100            CALL 0x00718a00
00601f58  0fb6d0                MOVZX EDX,AL
00601f5b  8995e4feffff          MOV dword ptr [EBP + 0xfffffee4],EDX
00601f61  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601f67  8b8de4feffff          MOV ECX,dword ptr [EBP + 0xfffffee4]
00601f6d  89889c020000          MOV dword ptr [EAX + 0x29c],ECX
00601f73  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601f79  8b82c8020000          MOV EAX,dword ptr [EDX + 0x2c8]
00601f7f  c1e807                SHR EAX,0x7
00601f82  83e001                AND EAX,0x1
00601f85  8985e0feffff          MOV dword ptr [EBP + 0xfffffee0],EAX
00601f8b  0fb68de0feffff        MOVZX ECX,byte ptr [EBP + 0xfffffee0]
00601f92  51                    PUSH ECX
00601f93  8d55cc                LEA EDX,[EBP + -0x34]
00601f96  52                    PUSH EDX
00601f97  6858749900            PUSH 0x997458
00601f9c  8b450c                MOV EAX,dword ptr [EBP + 0xc]
00601f9f  50                    PUSH EAX
00601fa0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00601fa3  e8586a1100            CALL 0x00718a00
00601fa8  0fb6c8                MOVZX ECX,AL
00601fab  898ddcfeffff          MOV dword ptr [EBP + 0xfffffedc],ECX
00601fb1  8b95dcfeffff          MOV EDX,dword ptr [EBP + 0xfffffedc]
00601fb7  83e201                AND EDX,0x1
00601fba  c1e207                SHL EDX,0x7
00601fbd  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601fc3  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
00601fc9  81e17fffffff          AND ECX,0xffffff7f
00601fcf  0bca                  OR ECX,EDX
00601fd1  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00601fd7  898ac8020000          MOV dword ptr [EDX + 0x2c8],ECX
00601fdd  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00601fe3  8b88c8020000          MOV ECX,dword ptr [EAX + 0x2c8]
00601fe9  c1e908                SHR ECX,0x8
00601fec  83e101                AND ECX,0x1
00601fef  898dd8feffff          MOV dword ptr [EBP + 0xfffffed8],ECX
00601ff5  0fb695d8feffff        MOVZX EDX,byte ptr [EBP + 0xfffffed8]
00601ffc  52                    PUSH EDX
00601ffd  8d45cc                LEA EAX,[EBP + -0x34]
00602000  50                    PUSH EAX
00602001  684c749900            PUSH 0x99744c
00602006  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00602009  51                    PUSH ECX
0060200a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060200d  e8ee691100            CALL 0x00718a00
00602012  0fb6d0                MOVZX EDX,AL
00602015  8995d4feffff          MOV dword ptr [EBP + 0xfffffed4],EDX
0060201b  8b85d4feffff          MOV EAX,dword ptr [EBP + 0xfffffed4]
00602021  83e001                AND EAX,0x1
00602024  c1e008                SHL EAX,0x8
00602027  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060202d  8b91c8020000          MOV EDX,dword ptr [ECX + 0x2c8]
00602033  81e2fffeffff          AND EDX,0xfffffeff
00602039  0bd0                  OR EDX,EAX
0060203b  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00602041  8990c8020000          MOV dword ptr [EAX + 0x2c8],EDX
00602047  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060204d  8b91a0020000          MOV EDX,dword ptr [ECX + 0x2a0]
00602053  52                    PUSH EDX
00602054  8d45cc                LEA EAX,[EBP + -0x34]
00602057  50                    PUSH EAX
00602058  6844749900            PUSH 0x997444
0060205d  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
00602060  51                    PUSH ECX
00602061  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602064  e8976b1100            CALL 0x00718c00
00602069  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
0060206f  8982a0020000          MOV dword ptr [EDX + 0x2a0],EAX
00602075  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
0060207b  8b480c                MOV ECX,dword ptr [EAX + 0xc]
0060207e  898dd0feffff          MOV dword ptr [EBP + 0xfffffed0],ECX
00602084  8b15d0b4a100          MOV EDX,dword ptr [0x00a1b4d0]
0060208a  8995ccfeffff          MOV dword ptr [EBP + 0xfffffecc],EDX
00602090  8b85d0feffff          MOV EAX,dword ptr [EBP + 0xfffffed0]
00602096  50                    PUSH EAX
00602097  8b8dccfeffff          MOV ECX,dword ptr [EBP + 0xfffffecc]
0060209d  8b4934                MOV ECX,dword ptr [ECX + 0x34]
006020a0  e85b310d00            CALL 0x006d5200
006020a5  8985c8feffff          MOV dword ptr [EBP + 0xfffffec8],EAX
006020ab  8b95c8feffff          MOV EDX,dword ptr [EBP + 0xfffffec8]
006020b1  8b4270                MOV EAX,dword ptr [EDX + 0x70]
006020b4  8985c4feffff          MOV dword ptr [EBP + 0xfffffec4],EAX
006020ba  83bdc4feffff00        CMP dword ptr [EBP + 0xfffffec4],0x0
006020c1  7413                  JZ 0x006020d6
006020c3  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
006020c6  51                    PUSH ECX
006020c7  8b5508                MOV EDX,dword ptr [EBP + 0x8]
006020ca  52                    PUSH EDX
006020cb  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006020d1  e81a090000            CALL 0x006029f0
006020d6  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
006020dc  8b880c010000          MOV ECX,dword ptr [EAX + 0x10c]
006020e2  898dc0feffff          MOV dword ptr [EBP + 0xfffffec0],ECX
006020e8  0fb695c0feffff        MOVZX EDX,byte ptr [EBP + 0xfffffec0]
006020ef  52                    PUSH EDX
006020f0  8d45cc                LEA EAX,[EBP + -0x34]
006020f3  50                    PUSH EAX
006020f4  68f46d9900            PUSH 0x996df4
006020f9  8b4d0c                MOV ECX,dword ptr [EBP + 0xc]
006020fc  51                    PUSH ECX
006020fd  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602100  e8fb681100            CALL 0x00718a00
00602105  0fb6d0                MOVZX EDX,AL
00602108  8995bcfeffff          MOV dword ptr [EBP + 0xfffffebc],EDX
0060210e  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00602114  8b8dbcfeffff          MOV ECX,dword ptr [EBP + 0xfffffebc]
0060211a  89880c010000          MOV dword ptr [EAX + 0x10c],ECX
00602120  6834749900            PUSH 0x997434
00602125  8b550c                MOV EDX,dword ptr [EBP + 0xc]
00602128  52                    PUSH EDX
00602129  8d45d4                LEA EAX,[EBP + -0x2c]
0060212c  50                    PUSH EAX
0060212d  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602130  e82b661100            CALL 0x00718760
00602135  83f801                CMP EAX,0x1
00602138  0f8554070000          JNZ 0x00602892
0060213e  8d4dd4                LEA ECX,[EBP + -0x2c]
00602141  51                    PUSH ECX
00602142  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602145  e8c6661100            CALL 0x00718810
0060214a  85c0                  TEST EAX,EAX
0060214c  0f8640070000          JBE 0x00602892
00602152  8d55d4                LEA EDX,[EBP + -0x2c]
00602155  52                    PUSH EDX
00602156  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602159  e8b2661100            CALL 0x00718810
0060215e  8945ac                MOV dword ptr [EBP + -0x54],EAX
00602161  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
00602168  c745f000000000        MOV dword ptr [EBP + -0x10],0x0
0060216f  eb09                  JMP 0x0060217a
00602171  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00602174  83c001                ADD EAX,0x1
00602177  8945ec                MOV dword ptr [EBP + -0x14],EAX
0060217a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0060217d  3b4dac                CMP ECX,dword ptr [EBP + -0x54]
00602180  7d64                  JGE 0x006021e6
00602182  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00602185  52                    PUSH EDX
00602186  8d45d4                LEA EAX,[EBP + -0x2c]
00602189  50                    PUSH EAX
0060218a  8d4db0                LEA ECX,[EBP + -0x50]
0060218d  51                    PUSH ECX
0060218e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602191  e8fa661100            CALL 0x00718890
00602196  8d55b0                LEA EDX,[EBP + -0x50]
00602199  52                    PUSH EDX
0060219a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060219d  e83e651100            CALL 0x007186e0
006021a2  85c0                  TEST EAX,EAX
006021a4  743e                  JZ 0x006021e4
006021a6  6a00                  PUSH 0x0
006021a8  8d45cc                LEA EAX,[EBP + -0x34]
006021ab  50                    PUSH EAX
006021ac  6824749900            PUSH 0x997424
006021b1  8d4db0                LEA ECX,[EBP + -0x50]
006021b4  51                    PUSH ECX
006021b5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006021b8  e843691100            CALL 0x00718b00
006021bd  668945d0              MOV word ptr [EBP + -0x30],AX
006021c1  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006021c5  741d                  JZ 0x006021e4
006021c7  0fb755d0              MOVZX EDX,word ptr [EBP + -0x30]
006021cb  52                    PUSH EDX
006021cc  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006021d2  e8091d0000            CALL 0x00603ee0
006021d7  85c0                  TEST EAX,EAX
006021d9  7409                  JZ 0x006021e4
006021db  8b45f0                MOV EAX,dword ptr [EBP + -0x10]
006021de  83c001                ADD EAX,0x1
006021e1  8945f0                MOV dword ptr [EBP + -0x10],EAX
006021e4  eb8b                  JMP 0x00602171
006021e6  8b4df0                MOV ECX,dword ptr [EBP + -0x10]
006021e9  51                    PUSH ECX
006021ea  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006021f0  e84b1f0000            CALL 0x00604140
006021f5  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
006021fc  c745c400000000        MOV dword ptr [EBP + -0x3c],0x0
00602203  eb09                  JMP 0x0060220e
00602205  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
00602208  83c201                ADD EDX,0x1
0060220b  8955ec                MOV dword ptr [EBP + -0x14],EDX
0060220e  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00602211  3b45ac                CMP EAX,dword ptr [EBP + -0x54]
00602214  0f8d16030000          JGE 0x00602530
0060221a  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0060221d  51                    PUSH ECX
0060221e  8d55d4                LEA EDX,[EBP + -0x2c]
00602221  52                    PUSH EDX
00602222  8d45b0                LEA EAX,[EBP + -0x50]
00602225  50                    PUSH EAX
00602226  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602229  e862661100            CALL 0x00718890
0060222e  8d4db0                LEA ECX,[EBP + -0x50]
00602231  51                    PUSH ECX
00602232  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602235  e8a6641100            CALL 0x007186e0
0060223a  85c0                  TEST EAX,EAX
0060223c  0f84e9020000          JZ 0x0060252b
00602242  6a00                  PUSH 0x0
00602244  8d55cc                LEA EDX,[EBP + -0x34]
00602247  52                    PUSH EDX
00602248  6824749900            PUSH 0x997424
0060224d  8d45b0                LEA EAX,[EBP + -0x50]
00602250  50                    PUSH EAX
00602251  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602254  e8a7681100            CALL 0x00718b00
00602259  668945d0              MOV word ptr [EBP + -0x30],AX
0060225d  0fb74dd0              MOVZX ECX,word ptr [EBP + -0x30]
00602261  51                    PUSH ECX
00602262  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602268  e8731c0000            CALL 0x00603ee0
0060226d  85c0                  TEST EAX,EAX
0060226f  0f84b6020000          JZ 0x0060252b
00602275  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602279  7422                  JZ 0x0060229d
0060227b  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0060227e  52                    PUSH EDX
0060227f  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602285  e8d61f0000            CALL 0x00604260
0060228a  8985b8feffff          MOV dword ptr [EBP + 0xfffffeb8],EAX
00602290  8b85b8feffff          MOV EAX,dword ptr [EBP + 0xfffffeb8]
00602296  668b4dd0              MOV CX,word ptr [EBP + -0x30]
0060229a  668908                MOV word ptr [EAX],CX
0060229d  6a00                  PUSH 0x0
0060229f  8d55cc                LEA EDX,[EBP + -0x34]
006022a2  52                    PUSH EDX
006022a3  681c749900            PUSH 0x99741c
006022a8  8d45b0                LEA EAX,[EBP + -0x50]
006022ab  50                    PUSH EAX
006022ac  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006022af  e84c681100            CALL 0x00718b00
006022b4  668945d0              MOV word ptr [EBP + -0x30],AX
006022b8  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006022bc  7423                  JZ 0x006022e1
006022be  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
006022c1  51                    PUSH ECX
006022c2  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006022c8  e8931f0000            CALL 0x00604260
006022cd  8985b4feffff          MOV dword ptr [EBP + 0xfffffeb4],EAX
006022d3  8b95b4feffff          MOV EDX,dword ptr [EBP + 0xfffffeb4]
006022d9  668b45d0              MOV AX,word ptr [EBP + -0x30]
006022dd  66894202              MOV word ptr [EDX + 0x2],AX
006022e1  6a00                  PUSH 0x0
006022e3  8d4dcc                LEA ECX,[EBP + -0x34]
006022e6  51                    PUSH ECX
006022e7  6810749900            PUSH 0x997410
006022ec  8d55b0                LEA EDX,[EBP + -0x50]
006022ef  52                    PUSH EDX
006022f0  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006022f3  e808671100            CALL 0x00718a00
006022f8  8845bb                MOV byte ptr [EBP + -0x45],AL
006022fb  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006022ff  7421                  JZ 0x00602322
00602301  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
00602304  50                    PUSH EAX
00602305  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060230b  e8501f0000            CALL 0x00604260
00602310  8985b0feffff          MOV dword ptr [EBP + 0xfffffeb0],EAX
00602316  8b8db0feffff          MOV ECX,dword ptr [EBP + 0xfffffeb0]
0060231c  8a55bb                MOV DL,byte ptr [EBP + -0x45]
0060231f  885104                MOV byte ptr [ECX + 0x4],DL
00602322  6a00                  PUSH 0x0
00602324  8d45cc                LEA EAX,[EBP + -0x34]
00602327  50                    PUSH EAX
00602328  6804749900            PUSH 0x997404
0060232d  8d4db0                LEA ECX,[EBP + -0x50]
00602330  51                    PUSH ECX
00602331  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602334  e8c7671100            CALL 0x00718b00
00602339  668945d0              MOV word ptr [EBP + -0x30],AX
0060233d  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602341  7423                  JZ 0x00602366
00602343  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
00602346  52                    PUSH EDX
00602347  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060234d  e80e1f0000            CALL 0x00604260
00602352  8985acfeffff          MOV dword ptr [EBP + 0xfffffeac],EAX
00602358  8b85acfeffff          MOV EAX,dword ptr [EBP + 0xfffffeac]
0060235e  668b4dd0              MOV CX,word ptr [EBP + -0x30]
00602362  66894806              MOV word ptr [EAX + 0x6],CX
00602366  6a00                  PUSH 0x0
00602368  8d55cc                LEA EDX,[EBP + -0x34]
0060236b  52                    PUSH EDX
0060236c  68fc739900            PUSH 0x9973fc
00602371  8d45b0                LEA EAX,[EBP + -0x50]
00602374  50                    PUSH EAX
00602375  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602378  e883661100            CALL 0x00718a00
0060237d  8845bb                MOV byte ptr [EBP + -0x45],AL
00602380  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602384  7421                  JZ 0x006023a7
00602386  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00602389  51                    PUSH ECX
0060238a  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602390  e8cb1e0000            CALL 0x00604260
00602395  8985a8feffff          MOV dword ptr [EBP + 0xfffffea8],EAX
0060239b  8b95a8feffff          MOV EDX,dword ptr [EBP + 0xfffffea8]
006023a1  8a45bb                MOV AL,byte ptr [EBP + -0x45]
006023a4  884208                MOV byte ptr [EDX + 0x8],AL
006023a7  6a00                  PUSH 0x0
006023a9  8d4dcc                LEA ECX,[EBP + -0x34]
006023ac  51                    PUSH ECX
006023ad  68f0739900            PUSH 0x9973f0
006023b2  8d55b0                LEA EDX,[EBP + -0x50]
006023b5  52                    PUSH EDX
006023b6  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006023b9  e842661100            CALL 0x00718a00
006023be  8845bb                MOV byte ptr [EBP + -0x45],AL
006023c1  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006023c5  7421                  JZ 0x006023e8
006023c7  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
006023ca  50                    PUSH EAX
006023cb  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006023d1  e88a1e0000            CALL 0x00604260
006023d6  8985a4feffff          MOV dword ptr [EBP + 0xfffffea4],EAX
006023dc  8b8da4feffff          MOV ECX,dword ptr [EBP + 0xfffffea4]
006023e2  8a55bb                MOV DL,byte ptr [EBP + -0x45]
006023e5  885109                MOV byte ptr [ECX + 0x9],DL
006023e8  6a00                  PUSH 0x0
006023ea  8d45cc                LEA EAX,[EBP + -0x34]
006023ed  50                    PUSH EAX
006023ee  68e0739900            PUSH 0x9973e0
006023f3  8d4db0                LEA ECX,[EBP + -0x50]
006023f6  51                    PUSH ECX
006023f7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006023fa  e801661100            CALL 0x00718a00
006023ff  8845bb                MOV byte ptr [EBP + -0x45],AL
00602402  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602406  7421                  JZ 0x00602429
00602408  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
0060240b  52                    PUSH EDX
0060240c  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602412  e8491e0000            CALL 0x00604260
00602417  8985a0feffff          MOV dword ptr [EBP + 0xfffffea0],EAX
0060241d  8b85a0feffff          MOV EAX,dword ptr [EBP + 0xfffffea0]
00602423  8a4dbb                MOV CL,byte ptr [EBP + -0x45]
00602426  88480a                MOV byte ptr [EAX + 0xa],CL
00602429  6a01                  PUSH 0x1
0060242b  8d55cc                LEA EDX,[EBP + -0x34]
0060242e  52                    PUSH EDX
0060242f  68186d9900            PUSH 0x996d18
00602434  8d45b0                LEA EAX,[EBP + -0x50]
00602437  50                    PUSH EAX
00602438  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060243b  e8c0651100            CALL 0x00718a00
00602440  8845bb                MOV byte ptr [EBP + -0x45],AL
00602443  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00602446  51                    PUSH ECX
00602447  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060244d  e80e1e0000            CALL 0x00604260
00602452  89859cfeffff          MOV dword ptr [EBP + 0xfffffe9c],EAX
00602458  0fb655bb              MOVZX EDX,byte ptr [EBP + -0x45]
0060245c  8b859cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe9c]
00602462  89500c                MOV dword ptr [EAX + 0xc],EDX
00602465  68ff000000            PUSH 0xff
0060246a  8d4dcc                LEA ECX,[EBP + -0x34]
0060246d  51                    PUSH ECX
0060246e  68d4739900            PUSH 0x9973d4
00602473  8d55b0                LEA EDX,[EBP + -0x50]
00602476  52                    PUSH EDX
00602477  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060247a  e881651100            CALL 0x00718a00
0060247f  8845bb                MOV byte ptr [EBP + -0x45],AL
00602482  8b45c4                MOV EAX,dword ptr [EBP + -0x3c]
00602485  50                    PUSH EAX
00602486  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060248c  e8cf1d0000            CALL 0x00604260
00602491  898598feffff          MOV dword ptr [EBP + 0xfffffe98],EAX
00602497  8b8d98feffff          MOV ECX,dword ptr [EBP + 0xfffffe98]
0060249d  8a55bb                MOV DL,byte ptr [EBP + -0x45]
006024a0  885110                MOV byte ptr [ECX + 0x10],DL
006024a3  68ff000000            PUSH 0xff
006024a8  8d45cc                LEA EAX,[EBP + -0x34]
006024ab  50                    PUSH EAX
006024ac  68c8739900            PUSH 0x9973c8
006024b1  8d4db0                LEA ECX,[EBP + -0x50]
006024b4  51                    PUSH ECX
006024b5  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006024b8  e843651100            CALL 0x00718a00
006024bd  8845bb                MOV byte ptr [EBP + -0x45],AL
006024c0  8b55c4                MOV EDX,dword ptr [EBP + -0x3c]
006024c3  52                    PUSH EDX
006024c4  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006024ca  e8911d0000            CALL 0x00604260
006024cf  898594feffff          MOV dword ptr [EBP + 0xfffffe94],EAX
006024d5  8b8594feffff          MOV EAX,dword ptr [EBP + 0xfffffe94]
006024db  8a4dbb                MOV CL,byte ptr [EBP + -0x45]
006024de  884811                MOV byte ptr [EAX + 0x11],CL
006024e1  68ff000000            PUSH 0xff
006024e6  8d55cc                LEA EDX,[EBP + -0x34]
006024e9  52                    PUSH EDX
006024ea  68bc739900            PUSH 0x9973bc
006024ef  8d45b0                LEA EAX,[EBP + -0x50]
006024f2  50                    PUSH EAX
006024f3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006024f6  e805661100            CALL 0x00718b00
006024fb  668945d0              MOV word ptr [EBP + -0x30],AX
006024ff  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00602502  51                    PUSH ECX
00602503  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602509  e8521d0000            CALL 0x00604260
0060250e  898590feffff          MOV dword ptr [EBP + 0xfffffe90],EAX
00602514  8b9590feffff          MOV EDX,dword ptr [EBP + 0xfffffe90]
0060251a  668b45d0              MOV AX,word ptr [EBP + -0x30]
0060251e  66894212              MOV word ptr [EDX + 0x12],AX
00602522  8b4dc4                MOV ECX,dword ptr [EBP + -0x3c]
00602525  83c101                ADD ECX,0x1
00602528  894dc4                MOV dword ptr [EBP + -0x3c],ECX
0060252b  e9d5fcffff            JMP 0x00602205
00602530  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
00602536  8b8288020000          MOV EAX,dword ptr [EDX + 0x288]
0060253c  89858cfeffff          MOV dword ptr [EBP + 0xfffffe8c],EAX
00602542  8b4dac                MOV ECX,dword ptr [EBP + -0x54]
00602545  2b8d8cfeffff          SUB ECX,dword ptr [EBP + 0xfffffe8c]
0060254b  51                    PUSH ECX
0060254c  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602552  e8791c0000            CALL 0x006041d0
00602557  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
0060255e  c745b400000000        MOV dword ptr [EBP + -0x4c],0x0
00602565  eb09                  JMP 0x00602570
00602567  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0060256a  83c201                ADD EDX,0x1
0060256d  8955ec                MOV dword ptr [EBP + -0x14],EDX
00602570  8b45ec                MOV EAX,dword ptr [EBP + -0x14]
00602573  3b45ac                CMP EAX,dword ptr [EBP + -0x54]
00602576  0f8d16030000          JGE 0x00602892
0060257c  8b4dec                MOV ECX,dword ptr [EBP + -0x14]
0060257f  51                    PUSH ECX
00602580  8d55d4                LEA EDX,[EBP + -0x2c]
00602583  52                    PUSH EDX
00602584  8d45b0                LEA EAX,[EBP + -0x50]
00602587  50                    PUSH EAX
00602588  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060258b  e800631100            CALL 0x00718890
00602590  8d4db0                LEA ECX,[EBP + -0x50]
00602593  51                    PUSH ECX
00602594  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602597  e844611100            CALL 0x007186e0
0060259c  85c0                  TEST EAX,EAX
0060259e  0f84e9020000          JZ 0x0060288d
006025a4  6a00                  PUSH 0x0
006025a6  8d55cc                LEA EDX,[EBP + -0x34]
006025a9  52                    PUSH EDX
006025aa  6824749900            PUSH 0x997424
006025af  8d45b0                LEA EAX,[EBP + -0x50]
006025b2  50                    PUSH EAX
006025b3  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006025b6  e845651100            CALL 0x00718b00
006025bb  668945d0              MOV word ptr [EBP + -0x30],AX
006025bf  0fb74dd0              MOVZX ECX,word ptr [EBP + -0x30]
006025c3  51                    PUSH ECX
006025c4  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006025ca  e811190000            CALL 0x00603ee0
006025cf  85c0                  TEST EAX,EAX
006025d1  0f85b6020000          JNZ 0x0060288d
006025d7  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006025db  7422                  JZ 0x006025ff
006025dd  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
006025e0  52                    PUSH EDX
006025e1  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006025e7  e8b41c0000            CALL 0x006042a0
006025ec  898588feffff          MOV dword ptr [EBP + 0xfffffe88],EAX
006025f2  8b8588feffff          MOV EAX,dword ptr [EBP + 0xfffffe88]
006025f8  668b4dd0              MOV CX,word ptr [EBP + -0x30]
006025fc  668908                MOV word ptr [EAX],CX
006025ff  6a00                  PUSH 0x0
00602601  8d55cc                LEA EDX,[EBP + -0x34]
00602604  52                    PUSH EDX
00602605  681c749900            PUSH 0x99741c
0060260a  8d45b0                LEA EAX,[EBP + -0x50]
0060260d  50                    PUSH EAX
0060260e  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602611  e8ea641100            CALL 0x00718b00
00602616  668945d0              MOV word ptr [EBP + -0x30],AX
0060261a  837dcc00              CMP dword ptr [EBP + -0x34],0x0
0060261e  7423                  JZ 0x00602643
00602620  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
00602623  51                    PUSH ECX
00602624  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060262a  e8711c0000            CALL 0x006042a0
0060262f  898584feffff          MOV dword ptr [EBP + 0xfffffe84],EAX
00602635  8b9584feffff          MOV EDX,dword ptr [EBP + 0xfffffe84]
0060263b  668b45d0              MOV AX,word ptr [EBP + -0x30]
0060263f  66894202              MOV word ptr [EDX + 0x2],AX
00602643  6a00                  PUSH 0x0
00602645  8d4dcc                LEA ECX,[EBP + -0x34]
00602648  51                    PUSH ECX
00602649  6810749900            PUSH 0x997410
0060264e  8d55b0                LEA EDX,[EBP + -0x50]
00602651  52                    PUSH EDX
00602652  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602655  e8a6631100            CALL 0x00718a00
0060265a  8845bb                MOV byte ptr [EBP + -0x45],AL
0060265d  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602661  7421                  JZ 0x00602684
00602663  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
00602666  50                    PUSH EAX
00602667  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060266d  e82e1c0000            CALL 0x006042a0
00602672  898580feffff          MOV dword ptr [EBP + 0xfffffe80],EAX
00602678  8b8d80feffff          MOV ECX,dword ptr [EBP + 0xfffffe80]
0060267e  8a55bb                MOV DL,byte ptr [EBP + -0x45]
00602681  885104                MOV byte ptr [ECX + 0x4],DL
00602684  6a00                  PUSH 0x0
00602686  8d45cc                LEA EAX,[EBP + -0x34]
00602689  50                    PUSH EAX
0060268a  6804749900            PUSH 0x997404
0060268f  8d4db0                LEA ECX,[EBP + -0x50]
00602692  51                    PUSH ECX
00602693  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602696  e865641100            CALL 0x00718b00
0060269b  668945d0              MOV word ptr [EBP + -0x30],AX
0060269f  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006026a3  7423                  JZ 0x006026c8
006026a5  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
006026a8  52                    PUSH EDX
006026a9  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006026af  e8ec1b0000            CALL 0x006042a0
006026b4  89857cfeffff          MOV dword ptr [EBP + 0xfffffe7c],EAX
006026ba  8b857cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe7c]
006026c0  668b4dd0              MOV CX,word ptr [EBP + -0x30]
006026c4  66894806              MOV word ptr [EAX + 0x6],CX
006026c8  6a00                  PUSH 0x0
006026ca  8d55cc                LEA EDX,[EBP + -0x34]
006026cd  52                    PUSH EDX
006026ce  68fc739900            PUSH 0x9973fc
006026d3  8d45b0                LEA EAX,[EBP + -0x50]
006026d6  50                    PUSH EAX
006026d7  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006026da  e821631100            CALL 0x00718a00
006026df  8845bb                MOV byte ptr [EBP + -0x45],AL
006026e2  837dcc00              CMP dword ptr [EBP + -0x34],0x0
006026e6  7421                  JZ 0x00602709
006026e8  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006026eb  51                    PUSH ECX
006026ec  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006026f2  e8a91b0000            CALL 0x006042a0
006026f7  898578feffff          MOV dword ptr [EBP + 0xfffffe78],EAX
006026fd  8b9578feffff          MOV EDX,dword ptr [EBP + 0xfffffe78]
00602703  8a45bb                MOV AL,byte ptr [EBP + -0x45]
00602706  884208                MOV byte ptr [EDX + 0x8],AL
00602709  6a00                  PUSH 0x0
0060270b  8d4dcc                LEA ECX,[EBP + -0x34]
0060270e  51                    PUSH ECX
0060270f  68f0739900            PUSH 0x9973f0
00602714  8d55b0                LEA EDX,[EBP + -0x50]
00602717  52                    PUSH EDX
00602718  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060271b  e8e0621100            CALL 0x00718a00
00602720  8845bb                MOV byte ptr [EBP + -0x45],AL
00602723  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602727  7421                  JZ 0x0060274a
00602729  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
0060272c  50                    PUSH EAX
0060272d  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602733  e8681b0000            CALL 0x006042a0
00602738  898574feffff          MOV dword ptr [EBP + 0xfffffe74],EAX
0060273e  8b8d74feffff          MOV ECX,dword ptr [EBP + 0xfffffe74]
00602744  8a55bb                MOV DL,byte ptr [EBP + -0x45]
00602747  885109                MOV byte ptr [ECX + 0x9],DL
0060274a  6a00                  PUSH 0x0
0060274c  8d45cc                LEA EAX,[EBP + -0x34]
0060274f  50                    PUSH EAX
00602750  68e0739900            PUSH 0x9973e0
00602755  8d4db0                LEA ECX,[EBP + -0x50]
00602758  51                    PUSH ECX
00602759  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060275c  e89f621100            CALL 0x00718a00
00602761  8845bb                MOV byte ptr [EBP + -0x45],AL
00602764  837dcc00              CMP dword ptr [EBP + -0x34],0x0
00602768  7421                  JZ 0x0060278b
0060276a  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
0060276d  52                    PUSH EDX
0060276e  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
00602774  e8271b0000            CALL 0x006042a0
00602779  898570feffff          MOV dword ptr [EBP + 0xfffffe70],EAX
0060277f  8b8570feffff          MOV EAX,dword ptr [EBP + 0xfffffe70]
00602785  8a4dbb                MOV CL,byte ptr [EBP + -0x45]
00602788  88480a                MOV byte ptr [EAX + 0xa],CL
0060278b  6a00                  PUSH 0x0
0060278d  8d55cc                LEA EDX,[EBP + -0x34]
00602790  52                    PUSH EDX
00602791  68186d9900            PUSH 0x996d18
00602796  8d45b0                LEA EAX,[EBP + -0x50]
00602799  50                    PUSH EAX
0060279a  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060279d  e85e621100            CALL 0x00718a00
006027a2  8845bb                MOV byte ptr [EBP + -0x45],AL
006027a5  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
006027a8  51                    PUSH ECX
006027a9  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006027af  e8ec1a0000            CALL 0x006042a0
006027b4  89856cfeffff          MOV dword ptr [EBP + 0xfffffe6c],EAX
006027ba  0fb655bb              MOVZX EDX,byte ptr [EBP + -0x45]
006027be  8b856cfeffff          MOV EAX,dword ptr [EBP + 0xfffffe6c]
006027c4  89500c                MOV dword ptr [EAX + 0xc],EDX
006027c7  68ff000000            PUSH 0xff
006027cc  8d4dcc                LEA ECX,[EBP + -0x34]
006027cf  51                    PUSH ECX
006027d0  68d4739900            PUSH 0x9973d4
006027d5  8d55b0                LEA EDX,[EBP + -0x50]
006027d8  52                    PUSH EDX
006027d9  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
006027dc  e81f621100            CALL 0x00718a00
006027e1  8845bb                MOV byte ptr [EBP + -0x45],AL
006027e4  8b45b4                MOV EAX,dword ptr [EBP + -0x4c]
006027e7  50                    PUSH EAX
006027e8  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006027ee  e8ad1a0000            CALL 0x006042a0
006027f3  898568feffff          MOV dword ptr [EBP + 0xfffffe68],EAX
006027f9  8b8d68feffff          MOV ECX,dword ptr [EBP + 0xfffffe68]
006027ff  8a55bb                MOV DL,byte ptr [EBP + -0x45]
00602802  885110                MOV byte ptr [ECX + 0x10],DL
00602805  68ff000000            PUSH 0xff
0060280a  8d45cc                LEA EAX,[EBP + -0x34]
0060280d  50                    PUSH EAX
0060280e  68c8739900            PUSH 0x9973c8
00602813  8d4db0                LEA ECX,[EBP + -0x50]
00602816  51                    PUSH ECX
00602817  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
0060281a  e8e1611100            CALL 0x00718a00
0060281f  8845bb                MOV byte ptr [EBP + -0x45],AL
00602822  8b55b4                MOV EDX,dword ptr [EBP + -0x4c]
00602825  52                    PUSH EDX
00602826  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060282c  e86f1a0000            CALL 0x006042a0
00602831  898564feffff          MOV dword ptr [EBP + 0xfffffe64],EAX
00602837  8b8564feffff          MOV EAX,dword ptr [EBP + 0xfffffe64]
0060283d  8a4dbb                MOV CL,byte ptr [EBP + -0x45]
00602840  884811                MOV byte ptr [EAX + 0x11],CL
00602843  68ff000000            PUSH 0xff
00602848  8d55cc                LEA EDX,[EBP + -0x34]
0060284b  52                    PUSH EDX
0060284c  68bc739900            PUSH 0x9973bc
00602851  8d45b0                LEA EAX,[EBP + -0x50]
00602854  50                    PUSH EAX
00602855  8b4d08                MOV ECX,dword ptr [EBP + 0x8]
00602858  e8a3621100            CALL 0x00718b00
0060285d  668945d0              MOV word ptr [EBP + -0x30],AX
00602861  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
00602864  51                    PUSH ECX
00602865  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060286b  e8301a0000            CALL 0x006042a0
00602870  898560feffff          MOV dword ptr [EBP + 0xfffffe60],EAX
00602876  8b9560feffff          MOV EDX,dword ptr [EBP + 0xfffffe60]
0060287c  668b45d0              MOV AX,word ptr [EBP + -0x30]
00602880  66894212              MOV word ptr [EDX + 0x12],AX
00602884  8b4db4                MOV ECX,dword ptr [EBP + -0x4c]
00602887  83c101                ADD ECX,0x1
0060288a  894db4                MOV dword ptr [EBP + -0x4c],ECX
0060288d  e9d5fcffff            JMP 0x00602567
00602892  6a01                  PUSH 0x1
00602894  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060289a  e831150000            CALL 0x00603dd0
0060289f  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006028a5  e8b60f0000            CALL 0x00603860
006028aa  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
006028b0  8b8288020000          MOV EAX,dword ptr [EDX + 0x288]
006028b6  89855cfeffff          MOV dword ptr [EBP + 0xfffffe5c],EAX
006028bc  83bd5cfeffff00        CMP dword ptr [EBP + 0xfffffe5c],0x0
006028c3  7528                  JNZ 0x006028ed
006028c5  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006028cb  8b918c020000          MOV EDX,dword ptr [ECX + 0x28c]
006028d1  899558feffff          MOV dword ptr [EBP + 0xfffffe58],EDX
006028d7  83bd58feffff00        CMP dword ptr [EBP + 0xfffffe58],0x0
006028de  750d                  JNZ 0x006028ed
006028e0  6a01                  PUSH 0x1
006028e2  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006028e8  e813440000            CALL 0x00606d00
006028ed  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006028f3  e8c81d0000            CALL 0x006046c0
006028f8  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006028fe  898184020000          MOV dword ptr [ECX + 0x284],EAX
00602904  8b9540feffff          MOV EDX,dword ptr [EBP + 0xfffffe40]
0060290a  8b4214                MOV EAX,dword ptr [EDX + 0x14]
0060290d  898554feffff          MOV dword ptr [EBP + 0xfffffe54],EAX
00602913  8b0da4b4a100          MOV ECX,dword ptr [0x00a1b4a4]
00602919  8b5108                MOV EDX,dword ptr [ECX + 0x8]
0060291c  899550feffff          MOV dword ptr [EBP + 0xfffffe50],EDX
00602922  8b8554feffff          MOV EAX,dword ptr [EBP + 0xfffffe54]
00602928  50                    PUSH EAX
00602929  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060292f  83c128                ADD ECX,0x28
00602932  51                    PUSH ECX
00602933  8b8d50feffff          MOV ECX,dword ptr [EBP + 0xfffffe50]
00602939  e85294f1ff            CALL 0x0051bd90
0060293e  8bc8                  MOV ECX,EAX
00602940  e82bc0f5ff            CALL 0x0055e970
00602945  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
0060294b  e8e0440d00            CALL 0x006d6e30
00602950  89854cfeffff          MOV dword ptr [EBP + 0xfffffe4c],EAX
00602956  8b954cfeffff          MOV EDX,dword ptr [EBP + 0xfffffe4c]
0060295c  8a8289000000          MOV AL,byte ptr [EDX + 0x89]
00602962  88854bfeffff          MOV byte ptr [EBP + 0xfffffe4b],AL
00602968  0fb68d4bfeffff        MOVZX ECX,byte ptr [EBP + 0xfffffe4b]
0060296f  894dc0                MOV dword ptr [EBP + -0x40],ECX
00602972  d9e8                  FLD1
00602974  d95dbc                FSTP float ptr [EBP + -0x44]
00602977  c745c800000000        MOV dword ptr [EBP + -0x38],0x0
0060297e  c745ec00000000        MOV dword ptr [EBP + -0x14],0x0
00602985  eb09                  JMP 0x00602990
00602987  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
0060298a  83c201                ADD EDX,0x1
0060298d  8955ec                MOV dword ptr [EBP + -0x14],EDX
00602990  8b8540feffff          MOV EAX,dword ptr [EBP + 0xfffffe40]
00602996  8b888c020000          MOV ECX,dword ptr [EAX + 0x28c]
0060299c  898d44feffff          MOV dword ptr [EBP + 0xfffffe44],ECX
006029a2  8b55ec                MOV EDX,dword ptr [EBP + -0x14]
006029a5  3b9544feffff          CMP EDX,dword ptr [EBP + 0xfffffe44]
006029ab  7d02                  JGE 0x006029af
006029ad  ebd8                  JMP 0x00602987
006029af  8b45c0                MOV EAX,dword ptr [EBP + -0x40]
006029b2  0345c8                ADD EAX,dword ptr [EBP + -0x38]
006029b5  8945c0                MOV dword ptr [EBP + -0x40],EAX
006029b8  db45c0                FILD dword ptr [EBP + -0x40]
006029bb  d84dbc                FMUL float ptr [EBP + -0x44]
006029be  e89d9e3100            CALL 0x0091c860
006029c3  8945c0                MOV dword ptr [EBP + -0x40],EAX
006029c6  8b8d40feffff          MOV ECX,dword ptr [EBP + 0xfffffe40]
006029cc  8b55c0                MOV EDX,dword ptr [EBP + -0x40]
006029cf  8991d0020000          MOV dword ptr [ECX + 0x2d0],EDX
006029d5  8b4df4                MOV ECX,dword ptr [EBP + -0xc]
006029d8  64890d00000000        MOV dword ptr FS:[0x0],ECX
006029df  59                    POP ECX
006029e0  8b4de8                MOV ECX,dword ptr [EBP + -0x18]
006029e3  33cd                  XOR ECX,EBP
006029e5  e8046d3100            CALL 0x009196ee
006029ea  8be5                  MOV ESP,EBP
006029ec  5d                    POP EBP
006029ed  c20800                RET 0x8
