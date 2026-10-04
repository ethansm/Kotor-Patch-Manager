from hookcheck import *
H = [("BeginClassMenuAppearance",0x5DE18A,0x8F9DCA,"8D 45 88 50 8B 8D 5C FF FF FF"),
("OverrideClassMenuModel",0x48E574,0x85E4E4,"8D 45 E0 50 8D 4D 8C"),
("OverrideClassMenuTexture",0x48E691,0x85E601,"C6 45 FC 00 8D 4D B4"),
("EndClassMenuAppearance",0x5DE199,0x8F9DD9,"8B 4D 80 51 8B 95 5C FF FF FF")]
for n,g,s,b in H: check(n,g,s,bytes.fromhex(b))
