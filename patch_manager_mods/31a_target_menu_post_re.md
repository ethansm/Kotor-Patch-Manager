# 31a: TARGET_MENU_POST candidate = 0x00743850 (static RE)

Status: 2026-10-04. Static only, no Ghidra run. Steam Aspyr swkotor2.exe sha 6A522E71 (vanilla dump; live = vanilla + 4GB flag).
Citations: `D` = `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp`, so `D/74/00743540.asm:234` is line 234 of that file. `CG` = `.../steam_6a522e71/callgraph.tsv`.
Names (name_map, medium conf): 0x743540 = CSWGuiTargetActionMenu::Update/PopulateMenus, 0x7433B0 = ::HitCheckMouse, 0x743280 = ::Draw, 0x74CFE0 = CSWGuiMainInterface::UpdateTargetMenu. 0x743850 and 0x741660 are unnamed.

## Q1. Is 0x743850 called on every PopulateMenus path, after all 0x741660 calls?

YES. Evidence:
- PopulateMenus has exactly one `RET`: `D/74/00743540.asm:238` (`RET 0xc`). Every path reaches it through `0x74382b`, the loop-exit target (`00743540.asm:62`, `JGE 0x0074382b`), which runs `CALL 0x00742a00` (`:228`), then `CALL 0x00743850` (`:234`), then the epilogue (`:235-238`).
- 0x741660 is called from PopulateMenus at `00743540.asm:106`, `:174` and `:222`. These are three mutually exclusive branches inside loop 2 (`:56-224`, group index `[EBP-4]` = 0..2, back-edge `:224 JMP 0x007435f0`):
  - selected entry found (`:106`, then `:148-149` set found flag and jump to `0x74372f`, `:152 JNZ 0x743826` skips the rest);
  - not found and count > 0 (`:174`, reached via the `JLE` at `:159`);
  - count <= 0 (`:222`).
  So every group does exactly one 0x741660 call per PopulateMenus, and all of them precede `0x74382b`.
- Between the last 0x741660 call and 0x743850, only 0x742a00 runs (`:228`). It only READS the arrow flags: `D/74/00742a00.asm:111-124` loads `[ctl+0x3E8]` and `[ctl+0x5B8]` and masks bit 0. No instruction in `D/74/*.asm` stores to `+0x3e8/+0x5b8` except 0x741660 (`rg -l "0x3e8|0x5b8" D/74` gives only 007414d0 (read), 00741940 (clears bit 2 in the ctor), 00741660 (the writer), 00742a00 (read), 0074cc90 (read, other array)).
- Call setup at `00743540.asm:225-234`:
  - `:225-226` `MOV EDX,[EBP+0x10]; PUSH EDX` (target, pushed first, so higher address);
  - `:230-231` `MOV ECX,[EBP+8]; PUSH ECX` (leader);
  - `:233` `MOV ECX,[EBP-0x30]` (menu pointer saved from the original ECX at `:6`);
  - `:234` `CALL 0x00743850`.
  Callee view: `ecx` = menu (hud + 0xCC), `[esp+4]` = leader (PopulateMenus arg1), `[esp+8]` = target (PopulateMenus arg3, `[EBP+0x10]`). This agrees with `patch_manager_mods/hud_host_src/kotor2-steam-aspyr.hooks.toml:4-5` (leader at `[esp+4]`, target at `[esp+0xC]` at the 0x743540 prologue). 0x743850 uses the target as `[EBP+0xC]` (`D/74/00743850.asm:6`, vtable calls `:82-85`); the leader slot `[EBP+8]` is not read (`00743850.c:6` declares `param_2` undefined4 and the body never uses it; UNVERIFIED by full asm read, but no `[EBP + 0x8]` in the grep of CALL/offset lines).
- RET n: `D/74/00743850.asm:603` `c20800` = `RET 0x8` (also `00743850.c:1` `ret=8`). Matches the two stack args.
- Callers: SOLE caller. `CG:73054` (`00743540 00743842 00743850 CALL`); `00743850.c:1` `callers=00743540`; `rg -w "743850|0x00743850|FUN_00743850"` over all of `ghidra_dumps` hits only 00743540.{c,asm} and 00743850.{c,asm}. Independent byte scan of `swkotor2_vanilla_6a522e71.exe`: the only `E8/E9` rel32 targeting 0x743850 is at 0x743842, and no absolute dword `0x00743850` exists anywhere in the image (no vtable or data reference).
- Early exit inside 0x743850: `00743850.asm:6-7` `CMP [EBP+0xC],0 ; JZ 0x744239` (target null) goes to the tail at `:596-600`, which clears `menu+0x1BAC` bit 0 and then returns (`:601-603`). The hook still fires on entry in that case; a POST write is harmless there.
- PopulateMenus cadence: `D/74/0074cfe0.c:58` calls `FUN_00743540(iVar3,local_10,local_18)` whenever a leader object exists (`iVar3 != 0`, `:18-31`). `0074cfe0` is called with arg 1 on every update at `D/74/0074b3f0.c:83` (after the `0x7f000000` no-leader early return at `:66-71`), and with arg 0 from `0074da40.c:97`. So PopulateMenus, hence 0x741660, runs every frame while a leader exists. (The other 0x741660 calls, in `0074cfe0.asm` at 0x74d24e/0x74d33c/0x74d3e0, use `hud+0x733C`, a different array, not `menu+0x54`.)

## Q2. Detour validity at 0x743850

- Bytes (`D/74/00743850.asm:2-4`, confirmed against the exe file bytes at 0x743850: `55 8b ec 81 ec e4 00 00 00 89 8d ...`):
  - `00743850  55            PUSH EBP` (1 byte)
  - `00743851  8bec          MOV EBP,ESP` (2 bytes)
  - `00743853  81ece4000000  SUB ESP,0xe4` (6 bytes)
  Total 9 bytes = 3 whole instructions; next instruction starts at 0x743859 (`:5`). No relative branch/call in the 9 bytes, so they are position independent. 0x743850 is 16-byte aligned (`0x...0`).
- `original_bytes = [0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xE4, 0x00, 0x00, 0x00]`.
- KPM acceptance of non-5/6-byte stolen spans: Detour minimum is 5 (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/src/KPatchCore/Models/Hook.cs:136`, error text "at least 5 bytes"; `src/KotorPatcher/src/core/patcher.cpp:550-552` NOPs the remainder when `originalBytes.size() > 5`). No upper bound found. Existing 9-byte detours in this repo (`rg original_bytes patch_manager_mods/*_src/*.hooks.toml`; 9-byte entries exist): `companion_ai_src/kotor2-steam-aspyr.hooks.toml:281` (0x005C2438), 0x005BC5F4 and 0x005D6A60 (same file), `dialogue_letterbox_fix_probe_src/kotor2-steam-aspyr.hooks.toml:80-83` (0x008BB538). Other lengths in use: 5, 6, 7, 8, 10, 12, 13, 15, 22.
- Unowned: `rg -n "7616592|0x00743850|743850" "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II/patch_config.toml"` returned no match (exit 1; 7616592 = 0x743850). `rg -n "0x00743850|743850" patch_manager_mods/*_src` found nothing. `python3 scripts/check_hook_spans.py` (source 215 spans / installed 249 spans) reports FAIL=5 WARN=3, but none mention 0x0074[34]xxx (the 5 FAILs are action-hotbar vs save-events overlaps at 0x0052FD1E, 0x00534527, 0x0053C5AA, 0x008529E0, 0x008FACA0). The only owner nearby is hud-host's PRE detour at 0x743540 (`hud_host_src/kotor2-steam-aspyr.hooks.toml:7-10`, 6 bytes `55 8B EC 83 EC 38`), 0x310 bytes before; no span overlap.
- Entry state at the detour: prologue not yet run (no frame). `ecx` = menu; `[esp]` = return address (0x743847), `[esp+4]` = leader, `[esp+8]` = target. Hook parameters: `ecx` pointer, `[esp+4]` pointer, `[esp+8]` pointer.

## Q3. Which bits 0x741660 writes at ctl+0x3E8 / ctl+0x5B8

Signature: `__thiscall FUN_00741660(ctl, int p2, uint p3)`, `RET 0x8` (`D/74/00741660.asm:88`); p2 = `[EBP+8]` (`:15`), p3 = `[EBP+0xC]` (`:59`). `00741660.c:42-44` agrees.

Writes (the flag writes are unconditional; only the value depends on p3):
- `ctl+0x3E8` (= `ctl+0x3A0` + 0x48, the UP arrow button flags word): `00741660.asm:56-66`: `flags = (flags & 0xFFFFFFFD) | ((p3 & 1) << 1)`. Clears bit 1, then sets it iff `p3 & 1`. All other bits preserved.
- `ctl+0x5B8` (= `ctl+0x570` + 0x48, the DOWN arrow flags word): `00741660.asm:68-78`, identical formula.
- Also written (not the arrow flags): float `[ctl+0x94]` (`:44`) and `[ctl+0x10C]` (`:54`) = `1.0` if p2 == 0 else `[0x98C014]`; byte `[ctl+0x74D]` = (p2 != 0) (`:79-82`). Meaning of these three: UNVERIFIED.
- Control layout (so `ctl+0x3A0` = UP, `+0x570` = DOWN): `D/74/00741940.c:38-48` (`BTN_%s%d` at +0, `LBL_%s%d` at +0x1D0, `BTN_%sUP%d` at +0x3A0, `BTN_%sDOWN%d` at +0x570), group built by 0x741420 with four `CSWGuiButton` ctor calls (`D/74/00741420.c`, `D/41/0041a510.c:14` sets `CSWGuiButton::vftable`).

What p3 is at each PopulateMenus call site:
- Call 1 (`00743540.asm:100-106`) and call 2 (`:168-174`): `SETG CL` (`:99`, `:167`) `PUSH ECX` then `PUSH 0`, so p2 = 0, p3 = `(count > 1)`. Bit 1 = (count > 1).
- Call 3 (`:216-222`, count <= 0): `PUSH 0 ; PUSH 1`, so p2 = 1, p3 = 0. Bit 1 = 0.
Net: after PopulateMenus, a group's arrow flag bit 1 is set iff that group has more than one entry; UP and DOWN always carry the same bit.

Nothing after the last 0x741660 call rewrites them: 0x742a00 only reads (`00742a00.asm:111-124`), and a store scan of `00743850.asm` (every `MOV/AND/OR/ADD/SUB/FSTP` with a non-EBP memory destination) shows stores to the menu only at `+0x1648`, `+0x164C`, `+0x1654` and `+0x1BAC` (e.g. `00743850.asm:13`, `:45`, `:55`, `:73`, `:152`, `:600`). So the arrow flags written by 0x741660 are intact when 0x743850 is entered, and 0x743850 never touches them.

Other readers of these flags (side effects of a bit-1 clear):
- Bit 0 (hot/hilight) of `+0x3E8/+0x5B8/+0x218/+0x48` is read by `00742a00.asm:111-124` and `0074cc90.c:25-30` (the latter on the `hud+0x733C` array). A bit-1 clear does not touch it.
- Bit 1: drawn-or-not and hit-testable-or-not (Q4).

## Q4. Visibility bit vs HitCheckMouse, and the mouse wheel

Hit test path:
- `0x7433B0` (HitCheckMouse): gate on `menu+0x1BAC` bit 0 (`D/74/007433b0.asm:7-10`; that bit is set by 0x743850 at `00743850.asm:10-13` when a target exists and cleared at the null-target tail `:596-600`). It then runs vtable+0x44 on `menu+0x1668` (name label, `:23-28`) and `menu+0x1A40` (health bar, `:39-44`), then for groups 0..2 with count > 0 (loops `:51-69`, `:72-91`) calls `CALL 0x00741590` (`:89`) with `ecx = menu+0x54+i*0x750` (`:86-88`), and finally stores `bit1 = (hit != 0)` into `menu+0x1BAC` (`:102-112`).
- `0x741590` (per-group hit test, `D/74/00741590.asm`): tries vtable+0x44 on, in order, `ctl+0x3A0` UP (`:11-16`), `ctl+0x570` DOWN (`:27-32`), `ctl+0x1D0` label (`:43-48`), `ctl` itself (`:59-63`).
- vtable+0x44 of `CSWGuiButton` (vftable 0x987A1C, `ghidra_dumps/steam_6a522e71/upstream_server/k2_win_steam_aspyr_swkotor2.exe/swkotor2_aspyr.exe_symbols.tsv` `0x00987a1c vftable CSWGuiButton`) is `0x00418D20` (read from the vanilla exe image; `CSWGuiControl`'s own vftable 0x9877D4 slot 0x44 is also 0x418D20). This slot value was read with a PE parse, not from a Ghidra dump (so "verified by byte read", not by Ghidra xref).
- `0x418D20` (`D/41/00418d20.asm`): `[this+0x48] >> 1 & 1` must be 1 (`:7-10`, else return 0) AND `[this+0x48] >> 5 & 1` must be 0 (`:12-15`), then a rect test. So the visible bit tested is bit 1 (mask 0x2) of the control flags, the same bit 0x741660 writes. Clearing bit 1 on `ctl+0x3E8`/`+0x5B8` makes UP/DOWN return 0 from the hit test (arrow clicks disabled). Setting bit 5 (0x20) would disable clicks while still drawing (not needed).
- Draw uses the same bit: `D/74/007414d0.asm:19-23` (`[ctl+0x3E8] >> 1 & 1` gates the UP draw call) and `:34-37` (`[ctl+0x5B8]`, DOWN). 0x7414D0 is called three times from `CSWGuiTargetActionMenu::Draw` 0x743280 (`CG:73039`, loop `local_8 < 3` in `00743280.c`). So one bit-1 clear = arrow not drawn and not clickable.

Mouse wheel / carousel:
- Event callbacks are registered by 0x741940 (`D/74/00741940.c:68-93`, `FUN_00418af0(code, panel, handler)`), in four blocks (label, main button, UP, DOWN; each block ends with that control's id store at `+0x22C`, `+0x5C`, `+0x3FC`, `+0x5CC`). Event codes 500 (0x1F4) and 0x1F5 are registered on all four controls (`:71-72`, `:76-77`, `:83-84`, `:89-90`) with handlers `local_1b4` and `local_1b8`; for TARGET (`param_4 != 0`, the menu) those are `FUN_00751d90` and `FUN_00751e40` (`:62-66`). The UP arrow click code 0x27 -> `local_1b4` (`:81`), the DOWN arrow click 0x27 -> `local_1b8` (`:87`).
- Handlers: `D/75/00751d90.c:5-18` -> `FUN_00744410` (previous carousel entry, step -1 with wrap, `00744410.c`), `00751e40.c` -> `FUN_00744260` (next). Neither the handlers nor `00744410.c` / `00744260.c` reference `0x3E8` / `1000` / `0x5B8` / `0x3A0` / `0x570` (`rg` clean). They only use `menu+4+i*0xC` (count), `menu+i*0xC` (array), `menu+0x24+...`, `menu+0x1BAA`. So the carousel step logic does NOT depend on the arrow controls' flags.
- UNVERIFIED: that event codes 500/0x1F5 are mouse-wheel up/down, and whether the input dispatcher only delivers them to a control that passes the hit test (the dispatcher that invokes the 0x418AF0-registered callbacks was not traced). If delivery is hit-test gated, the wheel still works over the group main button and label (their bit 1 is untouched), but not while the pointer is over a hidden arrow.

## Q5. Recommendation

At POST (entry of 0x743850, `ecx` = menu), for each group i = 0, 1, 2 clear bit 1 of both arrow flag words:

| i | UP flags `menu + 0x43C + i*0x750` | DOWN flags `menu + 0x60C + i*0x750` |
|---|---|---|
| 0 | `menu + 0x043C` | `menu + 0x060C` |
| 1 | `menu + 0x0B8C` | `menu + 0x0D5C` |
| 2 | `menu + 0x12DC` | `menu + 0x14AC` |

Derivation: 0x43C = 0x54 + 0x3E8; 0x60C = 0x54 + 0x5B8; group stride 0x750 (`00743540.asm:102-105`: `IMUL EDX,EDX,0x750 ; LEA ECX,[EAX+EDX+0x54]`).

Operation: `*(uint32_t*)p &= ~0x2u;` on each of the 6 words (32-bit read-modify-write, same as the engine's `00741660.asm:62-66`). Leave bit 0 (hot), bit 2 (0x4, cleared once in the ctor at `00741940.c:68,74,80,86`) and bit 5 alone. Conditions: none needed (idempotent). The engine re-derives bit 1 every frame in PopulateMenus before the POST point, so the write must be repeated every frame at POST, which it is, because PopulateMenus runs every frame while a leader exists (Q1). Skip when `ecx == 0`.

Draw ordering: UpdateTargetMenu (`0x74cfe0`, `0074b3f0.c:83`) runs before Draw `FUN_00743280` (`0074b3f0.c:290`) in the same function, and the POST write happens before PopulateMenus returns, so the cleared bit is what Draw (0x7414D0) and HitCheckMouse (0x7433B0) see that frame.

Notes:
- Groups with count == 0 already have bit 1 = 0 (call 3, p3 = 0); the write is a no-op there. Groups with count == 1 also already have bit 1 = 0 (p3 = count > 1).
- 0x741660 also writes `ctl+0x94`, `ctl+0x10C`, `ctl+0x74D`; this plan does not touch them.
- Wheel dependence on hit-test gating is UNVERIFIED (Q4).

## Verdict

VERDICT: SUITABLE

Hook spec:
- address: `0x00743850` (CSWGuiTargetActionMenu POST, runs before the menu is drawn; unnamed in the DB)
- type: detour, `original_bytes = [0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xE4, 0x00, 0x00, 0x00]` (9 bytes, 3 whole instructions, no rel branch), `skip_original_bytes = false`
- menu pointer: register `ecx` (menu = hud + 0xCC, thiscall `this`). Other params: `[esp+4]` leader pointer, `[esp+8]` target pointer (may be null). The callee does `RET 0x8`.

Write spec:
- per group i = 0..2: `*(uint32_t*)(menu + 0x43C + i*0x750) &= ~0x2u;` and `*(uint32_t*)(menu + 0x60C + i*0x750) &= ~0x2u;`
- effect: UP/DOWN arrows are not drawn (0x7414D0) and not hit-testable (0x418D20 via 0x741590 / 0x7433B0).
