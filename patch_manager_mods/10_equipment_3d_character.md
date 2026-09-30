# Mod Idea 5: Equipment screen live 3D character (+ flourish animation)

**Status (2026-09-30): Phase 1 static research DONE (GATE A passed). Phase 2: user first picked R, then switched to **A (view RIGHT)** after the stage-2 screenshot. 0.1.0-probe installed 2026-09-30 01:00 at stage 2 (static creature render confirmed working).** Predecessor: `01_inventory_3d_viewport.md` (Mod 1). Source of record will be `kotor-patch-manager/Patches/EquipCharPreview/`, mirrored in `equipment_3d_character_src/`. Raw research: `equipment_3d_character_src/research/R*`. DB: `functions`/`class_methods`/`lessons` 127-130 (scope `mod5-equip3d`).

## User request (2026-09-30)
Show a live render of the character wearing the currently chosen equipment on the equipment screen, and occasionally mix a flourish animation into the idle. Style reference: Mod 1 (drag rotate, wheel zoom, curved arc covers).

## GATE A: what we know

### The preview creature can be built by us (R2) — sequence, all Steam addresses
Decoded from `SetStats 0084e6f0` (raw disassembly `R2f`, `0084f2c6..0084f482`) and the character-page ctor `0084c3a0`:
1. **Once, in the panel ctor** (character page: after `StopLoadFromLayout`): `FnNew(0x470)` + `007645f0` (fastcall ECX=block) = a `CSWCCreature` (vtable `0x0099ee14`). The character page keeps it at `this+0x4b6c`.
2. **Source creature** = the client creature `real` (party member). The equip panel keeps it at **`panel+0x68`** (SetCharacter arg = `GetMember(0)` = a client-creature POINTER, not an object id: `007e5d80 -> 007e5da0(0) -> 0073f550(handle)`).
3. **Build/refresh** (`ECX` shown first):
   - copy the 15 dwords (0x3c bytes) at `real+0x224` (the appearance object `[0x89]`), set `copy+0x10 = 0x7f000000`;
   - `preview->vt[0xec](real->vt[0x148](buf, 0))` = set the tag string (the later camera-hook choice compares it with `t3m4`/`hk47`/`3cfd`/`zaalbar`); `vt[0x148]` = `00765000` builds a CExoString copy into `buf`;
   - if `*(int*)0x009f42b4 != 0`: `0076d200(ECX=preview; copy, 3, 1, 0)` (RET 0x10) builds body + head; **flag bit1 = body (0085a1f0 -> 0085b940/0085cc00), bit2 = head 0xfe, bit4 = tail 0xfd, bit8 = wings 0xfc — no weapons**;
   - `0085d430(ECX=preview+0x224; word real[0xc4]+0x80)` (alignment-dependent look);
   - `preview->vt[0x94](Scene* = view+0x74)`, `vt[0x8c](vec 0,0,0)` position, `vt[0x88](vec 1,0,0)` facing;
   - `0077ec20(ECX=preview; view, real, 0, 1)` (RET 0x10) = scene setup: camera hook per tag, `cam->vt[0x74](rig,hook,1)`, `cam->vt[0x44](22.726)`, `cam->vt[0x80](0x3ea)` behaviour block (+0x44 = `preview->vt[0xd8]()` height).
4. **Free**: `(*preview->vt[0])(1)`; the character page dtor `0084df70` does exactly that as its first act.
5. **Armour/helmet/hands** enter through the appearance copy (row `+0x18`, head `+0x1a`, `+0x14`, `+0x10`...), so a preview built from the real creature's *current* appearance shows the chosen armour. **Weapons in hands are not part of this path** (open item below).

### Equipment changes are applied to the REAL creature at once — but not by the hooks the earlier notes chose (Opus review)
`EquipItem 008b0320` has no confirm step. It calls `FUN_00566130(item,slot,1,1,0,0,flag==0,0)` on the server creature (`FUN_0077d800`); `00566130`/`00567490` are **server-creature methods used by every creature** and may only **queue** an action (`0053f7f0`/`00540ca0`, action 8/0xb). Hooking them fires before the change lands. **Decision: poll** the 0x3c bytes at `real+0x224` from the panel tick and rebuild via `0076d200` when they change (and on `panel+0x68` change).

### Animation facts (R4/R5)
- `Gob::PlayAnimation 004e3270` = `thiscall(Gob*, const char* name, float speed, uint flags, float start)`. `speed>0` loops, `<0` plays once for the anim length, flags `2` keep time (the idle call is `(name, 1.0, 2, 0)`). Name = raw char* looked up in the model's anim list, then its supermodel's. Events `start<name>`/`done<name>` go through `vt[0x1b4]`. Existence probe `vt[0x1c](name,&len)` returns -1.0 when missing.
- **The idle is NOT played on the creature by name-from-alignment as the earlier note said**: `0077ec20` plays the alignment name (`evil`/`good`/`align%d`) on scene model 0 = the **light rig** (`charrec_light` has those anims); the creature idle (`evil`/`neutral`/`good`, or `pause1` for t3m4/hk47/3cfd) is played on the part Gobs `preview->vt[8](0xff)` (body) and `(0xfe)` (head).
- R5 (`scripts/list_creature_anims.py`, `R5_creature_anim_inventory_*.txt`): all humanoid bodies (PC, Bastila, Carth, Mission, Zaalbar...) share the `s_male01` supermodel chain with **all 14 candidate flourishes** (`pausesh scanning salute victory bow greeting taunt touchheart rolleyes shrug nodyes point itemequip equip`, lengths 1.0-5.1 s; every one `fireforget=1 hideequippeditems=0 stationary=1`); `powerup` is missing from the chain. T3-M4 has only victory/greeting/taunt/equip; HK-47 none. **Plan: the INI holds a preferred list; the DLL filters it at run time with the vt[0x1c] probe** (no static per-body table).

### Panel facts (R1)
- Ctor `008a92d0`: `StopLoadFromLayout` call at **`008aa170`**; hook **H-E1** at **`008aa16a`**, original bytes `8B 8D 28 FE FF FF` (`MOV ECX,[EBP-0x1d8]`, panel), parameter `[ebp-0x1d8]`. No 3D view exists; `3D_MODEL` must be bound by us as in Mod 1 H1.
- Update (vtable slot 0x34) **`008aba50`**, prologue `55 8B EC 83 EC 08`, ECX = panel: hook **H-E3** (per-frame tick, Mod 1 H3 analogue).
- Dtor (slot 0) `008aa6f0` (44 bytes, scalar deleting dtor); OnPanelRemoved `008abd30` (slot 0x4c), OnPanelAdded `008abb30` (0x48). The panel is **re-constructed several times per session** (Mod 1 log shows new panel pointers), so the creature must be freed with the panel: hook **H-E4** on `008aa6f0` (verify prologue bytes before use) or free the previous one on the next H-E1.
- Overlaps with other live hooks must still be checked (±8 bytes) before the probe build: load-hang-probe, force-power-hotbar, buff-duration-hud, inventory-3d-viewport (H1 0x8A69AD, H2 0x8A8172, H3 0x8A7350 are all in the inventory panel, well away).

### Framing / rig (R6)
No rig copy needed: reuse **`charrec_light`** and the engine's own `0077ec20` (it picks the camera hook and the height-driven behaviour block, so no runtime bbox fit or per-creature table). The character page shows it in `LBL_3DCHAR [166,134,230,360]` (aspect 0.639); any view close to that aspect is framed correctly. Spin/drag: **`vt[0x88]` facing vector**, not Gob orientation (engine rotate handlers do this) — the Mod 1 Gob quaternion technique does not apply.

## Open items (carry into the probe)
1. **Weapons in hands** are not in the appearance copy; `0085b940` clears part ids `0xfb` and `0xf7` (hand attach points) — likely where hand models live. Needs R3b: find the writers of parts `0xfb/0xf7` (weapon model attach). v1 may ship armour/head/hands + flourish without weapons.
2. Whether the preview idle keeps animating without extra ticks in our scene (character page works with none).
3. Flourish completion: the anim list pops itself after `speed<0`; confirm under live test that the looping idle resumes (else re-issue the idle). Watch objects 0xff/0xfe ids do not collide (they are per-creature part ids, not global).
4. `CExoString` handling of `vt[0x148]` output (dtor `00733780`).
5. Per-frame cost of a second animated creature + rig particles.

## Layout candidates (Phase 2) — `scripts/equip3d_layout_mockup.py`, PNGs in `equipment_3d_character_src/mockups/`
Equip page live `equip_p.gui`: LB_ITEMS `[161,134,236,364]`, slot/description block `407..639` (LB_DESC `[407,134,232,336]` plus slot buttons/labels), free gutters `34..160` and `640..766`; LBL_BAR1-5, LBL_TITLE, LBL_SLOTNAME, PREV/NEXTNPC stay vanilla (BAR5 carries the portrait notch).
- **A (view right, mirror of Mod 1 layout V):** list + block shifted -113 -> LB_ITEMS `48..284`, block `294..526`, view `[536,134,216,336]` (1037x896 px, aspect 1.157 = the character page's physical aspect), arc covers `LBL_BEVEL/2 [532,130/436,224,38]`.
- **R (view left):** view `[48,134,216,336]`, list + block +113 -> list `274..510`, block `520..752`.
- **G (gutter only):** nothing moves, view `[650,134,102,336]` (aspect 0.546 px, cramped).
Constraints kept: 14-unit edge gaps, 10-unit panel gaps, BAR5 vanilla. Generator interplay (`generate_ultrawide_gui_overrides.py` records `equip_p` sha; its own tab-alignment check uses `equip_p LB_DESC right edge` = 723 in 4:3-px terms): apply the layout with a post-step script (`add_equip_3dview_control.py`, to be written) and `--rollback` before re-running the generator.

## Decisions log
- 2026-09-30 user picked **layout R** (view left `[48,134,216,336]`, list + block +113). The R5 inventory now covers the KOTOR 2 party rows only (appearance.2da keeps K1 leftovers: Bastila/Carth/Cand/Jolee/Juhani/Mission/Zaalbar; the first R5 run listed them by mistake).
- Idea parked for a later mod (user brainstorm): a persistent companion influence/affinity display. Vanilla has only the `statussummary_p` popup rows `LBL_INFLUENCE_RECV/LOST`; where the engine stores the per-companion value is NOT researched. Candidate homes: Character-page strip, party-select row. No free slot exists on the equip page in layout R.
- 2026-09-30 01:02 stage-2 live test: the preview creature renders correctly (PC with current gear, alignment mist rig). **Layout R was wrong**: the list moved right and overlapped the vanilla portrait-notch bar `LBL_BAR5 [407,476,227,32]` (list is 364 tall, ends y 498 > 476). Switched to **layout A** (list 48..284 ends left of the notch, block and view end at y 470 above it); same side as the inventory view.
