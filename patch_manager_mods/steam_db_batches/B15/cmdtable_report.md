# B15 command tables (GOG 0x77b530,0x7f9140, Steam 0x665f50,0x6f5b80)

- slots 877; filled GOG 877 / Steam 877; distinct handlers GOG 655 / Steam 655
- slot sets equal on both sides: 655 of 655 GOG handlers
- GOG `ExecuteCommand*` rows 655: match:exact 654, match:overwritten_slot_156 1, name:n/a 152, name:yes 503

## Slots stored twice (the later store is the runtime handler)

- GOG slot 156 EffectTemporaryForcePoints: 0x0078d6b0 overwritten by 0x0078c440 at 0x0077c1cd
- Steam slot 156 EffectTemporaryForcePoints: 0x006780d0 overwritten by 0x00676e60 at 0x00666bed

## GOG label contradicts the dispatch table (name_slot = no)


## GOG rows without an exact counterpart

- 0x0078d6b0 Global::ExecuteCommandEffectTemporaryFP slots {} -> 0x006780d0 (overwritten_slot_156)

## GOG table handlers with no `ExecuteCommand*` name in the GOG DB (1)

- 0x0044f600 CSWArea::GetIsIDInExcludeList slots {321,327,403}
