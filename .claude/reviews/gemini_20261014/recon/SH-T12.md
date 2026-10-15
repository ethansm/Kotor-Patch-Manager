# SH-T12
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3b-02 | UNSURE | patch_manager_mods/_shared_src/k2addr_test.cpp:25-60; k2addr_test.cpp:150-170; k2addr_test.cpp:189-190; K2Addr.h:106 | yes | Steam read ranges lack Border_FillStyle, SceneView, Menu_Char2/3; 61-149 unread. GOG SceneView omission is intentional. |

- k2addr_test.cpp:25 is the checkSteamFields signature; anchor is exact, not NOT-FOUND.
- k2addr_test.cpp:27-60 STEAM list: no Border_FillStyle, no SceneView, no Menu_Char2/3; no duplicate names in this span.
- k2addr_test.cpp:150-170 STEAM list tail: no Border_FillStyle, no SceneView, no Menu_Char2/3; no duplicate names.
- k2addr_test.cpp:171-172: #undef STEAM, then KT_CHECK_EQ(t.sha, kSteamSha); a whole-table sha may cover omitted fields (coverage unverified).
- k2addr_test.cpp:174-176: checkSteamTable calls checkSteamFields(kSteam) and checks kGog.sha; checkGogFields is not called in the read range.
- k2addr_test.cpp:189-190: comment says 21b leaves out KeyboardDeviceGlobal and the inventory-3D Scene/Gob/View/GuiGate block; those stay zero by design (GOG SceneView REFUTED as a defect).
- k2addr_test.cpp:191-230: GOG list has no Border_FillStyle in span; list may continue past 230 (unread), so GOG Border_FillStyle is UNSURE.
- K2Addr.h:106: Border_FillStyle is an int member; comment says bits 0-1 hold the style, so it is not an address and its kSteam value (0x34?) is unverifiable in range.
- k2addr_test.cpp:163-164: Steam list pins GobUpdateDependent and GobAttachByName, which are functions, not the SceneView block.
- Menu_Char2/3 duplicate: cannot be confirmed or refuted without lines 61-149; a same-value duplicate is harmless, a wrong-value one hides a gap.
