# B17 Opus gate part a (21 rows)

All 21 Steam candidates have byte-identical fp shape to GOG (size, retn, n_insn, full disp+imm multisets, callee count): counterpart solid for every row. Decision rests on name fit + quoted anchor.

| # | GOG | Steam | verdict | anchor |
|---|---|---|---|---|
| 1 | 0x0047e830 LoadModel | 0x007fd5b0 | verified | 007fd5e1 PUSH 0xfc |
| 2 | 0x00522830 CSWGuiLabel scalar dtor | 0x007510b0 | verified | vftable=CSWGuiLabel, CALL 0x418640, free 0x9196fd |
| 3 | 0x00571320 SetNewOptions | 0x007dba10 | verified | 007dba70 [ECX+0x18b4]; GetClientOptions |
| 4 | 0x005977a0 BlackenedLabel dtor | 0x0074ac30 | label_suspect | 0074ac59 MOV [EAX],0x9878bc CSWGuiLabel::vftable -> CSWGuiLabel::~CSWGuiLabel |
| 5 | 0x0059f8a0 InGameEquip::Draw | 0x008aba50 | verified | 008aba94 FLD [EAX+0x50e4]; CSWGuiPanel::Draw |
| 6 | 0x005a71d0 Container::OnPanelAdded | 0x008b3630 | verified | 008b36a2 ADD ECX,0xb18; CALL OnPanelAdded 0x40f3c0 |
| 7 | 0x005c5330 CreateItem::InitializeStoreType | 0x008d4f60 | label_suspect | toggle +0x3ed8, ShowSell/ShowBuy -> OnControlXButton |
| 8 | 0x005c61f0 CreateItem::UpdateCreateFilters | 0x008d5e20 | label_suspect | 008d5ed5 PUSH 0x30c new item, CopyItem -> BuyItem |
| 9 | 0x005ca130 CreateMedicalItem::DeleteItemEntries | 0x008d9d60 | label_suspect | toggle +0x33f8, ShowSell/ShowBuy -> OnControlXButton |
| 10 | 0x005cf920 SwoopUpgrade::SetDescription | 0x008df550 | verified | 008df56f [EDX+0x4340] |
| 11 | 0x005f42f0 OnSelectFeatsButton | 0x00914e70 | verified | 00914eb8 PUSH 0x1c08 + FeatsCharGen ctor |
| 12 | 0x005fae20 RemoveChosenFeat | 0x0090bf90 | verified | 0090c0b0 [EDX+0x1bde]; CExoString dtor |
| 13 | 0x005fafb0 AddChosenFeat | 0x0090c120 | verified | 0090c1fb CALL 0x0090c120 self-call |
| 14 | 0x00608260 CExoTimers ctor | 0x00736130 | likely | 00736158 PUSH 0x20 (thin) |
| 15 | 0x00615ec0 UpdateMouseDelta | 0x0072f110 | verified | 0072f113 SUB ESP,0x78; +0x408/+0x40c |
| 16 | 0x0061a300 GetResRefStr | 0x007109b0 | verified | 007109dc IMUL EDX,EDX,0x11 |
| 17 | 0x00630410 GetTableIndex | 0x007269c0 | verified | 007269f0 AND EDX,0xffffff |
| 18 | 0x00645160 ExecuteCode_5 | 0x0044b210 | label_suspect | 0044b23f CALL 0x436200 (operator*) with 1/s -> Vector::operator/ |
| 19 | 0x00648370 ComputeHexPoints | 0x006fa580 | verified | 006fa583 SUB ESP,0x2c0; GetArea/ComputeHeight |
| 20 | 0x00714000 ComputeClippedLineSegment | 0x006518b0 | verified | 006518b3 SUB ESP,0x15c |
| 21 | 0x00823860 RenderObjectsVTCQuadUP | 0x00431140 | verified | 004312ca PUSH 0x8892 |

## Label rotation (systematic)
GOG CSWGuiCreateItem / CSWGuiCreateMedicalItem labels are rotated. Mac x86 (real names) + GOG callee names:
- GOG 0x005c5330 (108 B, toggles +0x3ed8, calls ShowSellGUI 0x5c69a0 / ShowBuyGUI 0x5c6c20 + PlayGuiSound) = CSWGuiCreateItem::OnControlXButton (Mac 0x001d4e00, 77 B, disp 0x1c/0x35a8/0x3e4c, callees ShowBuyGUI/ShowSellGUI/sound). GOG's own "OnControlXButton" 0x005c54b0 (3391 B, 'base_skill') is something else.
- GOG 0x005c61f0 = CSWGuiCreateItem::BuyItem (Mac 0x001d8ae4: GetItemBuyValue, new item, CopyItem).
- GOG 0x005ca130 = CSWGuiCreateMedicalItem::OnControlXButton (Mac 0x001da50e, disp 0x1c/0x2ae0/0x3384).
- GOG 0x005977a0 = CSWGuiLabel::~CSWGuiLabel (GOG has no CSWGuiLabel dtor label).
- GOG 0x00645160 = Vector::operator/(float) (neighbours Vector::operator+/-/* at 0x413280/0x413720/0x419020).

mac_contra / mac_rotation flags on rows 3,5,6,10,12,13,15,16,17,19,20,21: no body-level contradiction found; fp identical and anchors fit names.

sample_errors: 0 (part a has no `sample` section rows)

AUTO meaning checks: none in part a.
