# HB-T12
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1c-03 | CONFIRMED | patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:708-712 | yes | Clear or refresh tip on empty name is sound; guitip empty handling unverified. |
| HB-1c-04 | PARTIAL | patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:507-516; patch_manager_mods/combat_hotbar_src/CombatLogic.h:460-466 | yes | Sig hashes count+keys only; needs collision or same-key rename to serve stale. |

- CombatHotbar.cpp:708-712: tip block runs only when !tipSet or tipKey != e.key; tipKey and tipSet are written unconditionally inside the block.
- CombatHotbar.cpp:709-710: name = g_names[g][ord[g][k]]; setNative runs only if name[0] is nonzero, so an empty name leaves the widget's previous native tip in place.
- CombatHotbar.cpp:705: widShow(b.bg,true) is the only widget call before the tip block; no tip clear appears in lines 695-715.
- CombatHotbar.cpp:708: tip is never re-set while e.key is unchanged, so a name refreshed later for the same key does not reach the tooltip.
- CombatHotbar.cpp:508-511: c clamped to 0..16; identity order idn[i]=i; early continue when g_nameValid and ns == g_nameSig.
- CombatLogic.h:460-466: orderSig is FNV-1a 32-bit over n, then each g.e[ord[i]].key; name bytes are not hashed.
- CombatHotbar.cpp:512-516: names zeroed; readExo <= 0 leaves an empty name, yet g_nameSig and g_nameValid are still set, so no retry until the key sig changes.
- OPEN (HB-1c-03): does guitip::setNative or any code outside the listed ranges clear b.bg.c's tooltip on an empty name?
- OPEN (HB-1c-04): GroupView key definition is outside range; if a key can keep its value while name bytes change, stale names are served.
