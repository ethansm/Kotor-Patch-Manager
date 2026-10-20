# R3 review: unarmed animations while holstered (read-only, static, $0)

Reviewer: Sonnet 5.5, 10-20. Sources read: `scripts/disasm_va.py steam 0x863650` (stops at 0x86367C, so the full listing is the Ghidra dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/86/00863650.asm|.c`, abbreviated `D86` below), `decomp/77/007759b0.c`, `decomp/77/00776fe0.c`, `scripts/disasm_va.py steam 0x7EF3E0 --len 120`, `.claude/research/holster_c6_unarmed_anim.md`, `holster_c7/{c_readers_combat,d_resolver_caller}.md`, `kotor2_logs.py check holster_log.txt` (10-16 ANIMRES summary only, no raw log), `Holster.cpp`, hooks.toml, `BUILD_AND_INSTALL.md`.

## 0. Resolution of the T3a/T3b/T3c inconsistency (answer first)

The resolver prologue and spill, from D86 asm lines 1-17 (all verified by me):

| VA | bytes | insn | role |
|---|---|---|---|
| 0x863650 | 55 8B EC 83 EC 60 | push ebp; mov ebp,esp; sub esp,0x60 | H4 steals these 6 B |
| 0x863656 | 89 4D CC | mov [ebp-0x34],ecx | ECX = anim base (creature+0x68), spilled |
| 0x863659/5D | 66 8B 45 08 / 66 89 45 F4 | id u16 from [ebp+8] copied to [ebp-0xC] (local_10, the RETURN value) |
| 0x863664/6A | 8A 91 C6 00 00 00 / 88 55 F3 | gate = [base+0xC6] spilled to [ebp-0xD] |
| 0x863670 | 8A 88 C5 00 00 00 | mov cl,[eax+0xC5] (wield byte) |
| 0x863676 | 88 4D FF | mov [ebp-1],cl (style spill) |
| 0x863679 | 8B 55 CC | mov edx,[ebp-0x34] |
| 0x86367C | 8B 82 D4 00 00 00 | mov eax,[edx+0xD4] (READ of base+0xD4) |
| 0x863682 | 89 45 F8 | mov [ebp-8],eax (iVar2) |
| 0x863685 | 0F B6 4D F3 | movzx ecx,byte [ebp-0xD] (ECX reloaded: CL is DEAD from here) |
| 0x863689/8C | 83 F9 01 / 0F 85 A7 0D 00 00 | cmp ecx,1; jnz 0x864439 (gate != 1 = constant table) |

So: T3a is right (86367C is a READ of +0xD4 placed after the spill; a 6 B hook there can rewrite [EBP-1]). T3c's "UNVERIFIED: switch reads [EBP-1], not CL" is now VERIFIED: every style switch is `MOVZX reg,byte [EBP-1]` (asm 0x863736, 0x86375E, 0x863786, 0x8637AE, 0x8637C2, 0x86380C, 0x863837, 0x86385F, 0x863887, 0x86389B, 0x863CA0, 0x863EDD, 0x863F76, 0x864042, 0x8641EE, 0x86425E, 0x8643FB, 0x864632). Nothing reads CL after 0x863685. The prior C6 "hook 0x863670, set CL" design is superseded (CL would also work at 0x863676 but that insn is 3 B).

### (a) Which insn reads what, to pick the row
- Gate: `movzx ecx,[ebp-0xD]` at 0x863685, `cmp ecx,1` 0x863689. Gate == 1 -> wield table; else -> constant table (0x864439..), which is wield-independent except id 0x2806 (asm 0x864632 reads [ebp-1]; T3a's "all 19 reads are below 0x864439" is wrong in one place: I count 18 reads, one of them at 0x864632 in the constant-table region).
- Idle 0x2711/0x27A4: case at 0x863CA0 `movzx edx,[ebp-1]; sub eax,1; cmp 8; jmp [ecx*4+0x864C24]`. Style 8: `cmp [ebp-8],0` (0x863D0D): d4==0 -> row 0xF9, else 0x217. Styles 1..7,9 -> 0x5C,0x85,0xAE,0xD7,0xDF,0xED,0xF5,0xF5. Style 0 has no case: id stays 0x2711.
- Walk 0x2712/0x2713 (0x863736...): style 2/3/4 AND `[base+0xE8] != 0` -> 0x152/0x155/0x153; style 7 or 9 -> 0x154; else row 0.
- Run 0x2714: `base+0xCC == 0` then style 2/3/4 AND e8 -> 0x159/0x157/3; style 7/9 -> 0x156; else row 2; `+0xCC != 0` -> row 0.
- Second read: case 0x283E at 0x863D39/0x863D3C re-reads `[eax+0xC5]` straight from memory into [ebp-0x1D] (asm 0x863D42), switch -> 0x7C,0xA5,0xCE,0xDC,0xEA,0xF2, style 8 -> 0x118 / 0x20E. A [EBP-1] write does NOT reach it. (Constant table 0x283E -> 0x118 fixed, D86 .c:706.)

### (b) Patch bytes at 0x86367C
- Instruction `8B 82 D4 00 00 00`, 6 bytes, starts at an instruction boundary (0x863679 insn ends there), ends at 0x863682 (boundary). No relative branch inside. No branch target in 0x86367D..0x863681 (rg of the asm for 0x0086367x/0x0086368[0-2] targets: none). `check_hook_bytes.py --mod holster` currently FAIL=0 (existing hooks), the new row must be re-run.
- >= 5 B, so an ordinary KPM detour fits (E9 rel32 + 1 NOP). No 2-byte in-place trick exists: a pure in-place edit (e.g. `B1 08` = mov cl,8 at 0x863670) would force style 8 for EVERY creature, so a conditional needs the DLL.
- At the hook: EDX = anim base, EBP = resolver frame, [ebp-1] = style, [ebp-0xD] = gate, [ebp+8] = u16 id (upper 16 bits of that dword are caller garbage, mask them), [ebp-0x34] = anim base. Flags and EAX are dead after the replayed insn (next is cmp); the wrapper (PUSHAD+PUSHFD+FXSAVE) saves everything anyway.
- Overlap check: H4 occupies 0x863650-0x863655 only. No other hooks toml touches 0x8636xx (rg over `*.toml`/`*.h`). Param sources `ebp`, `[ebp-N]`, `[ebp+N]` are in use elsewhere (script_events_src hooks.toml:17, portrait_slots_src hooks.toml:17,51).

### (c) What the resolver decides
It maps an Animations.2da ROW (u16) for ~hundreds of ids (switch 0x2710..0x2AFA, vtable slot 0xE0 of 7 CSWCAnimBase-family classes; `d_resolver_caller.md` Q1). Wield-dependent in the gate==1 table: idle/ready 0x2711/0x27A4, walk 0x2712/0x2713, run 0x2714, flourish-in 0x27AD, flourish-out 0x27AE, 0x2806 (both tables), 0x283C, 0x283D, 0x283E (direct read), 0x28BD. Attacks, hit reactions, force powers are NOT resolved by the style here (T3b right: those read +0xC5 elsewhere). Whole-resolver ownership is unproven for the 0x2710 "pause" id: it is wield-independent (row 6, or 0x100 gate != 1).

### (d) Out-of-combat idle (T3b OPEN 1): ANSWERED, and it changes the plan
`FUN_007759b0` (decomp 77/007759b0.c; REA ledger B13a-q3 confirms; `query_ghidra.py address 0x7759B0` row; the name_map label "GetCameraFollowPart" is wrong, ledger behaviour row is right) returns **0x2711 only if `[creature+0x138] != 0`**, else 0x2710, or 0x276C. `DriveAnimationUpdate` 0x776FE0 uses it (decomp 77/00776fe0.c:112 and :127). `+0x138` is exactly the combat flag the holster draw gate already reads (`Creature_ClientCombat` = 0x138, K2Addr.h:400, Holster.cpp:1171). Therefore a HOLSTERED creature (combat flag 0, hip pose written) asks for 0x2710 / 0x276C, which map to rows 6 / 8 (wield table) or 0x100 (constant table): wield-independent. **The idle stance while holstered is already the plain idle.** Forcing style 8 changes nothing for idle. The ready-stance (0x2711) is only requested while drawn.

The ANIMRES summary from the 10-16 session agrees: top ids are `0x2710 x32, 0x2712, 0x2713, 0x2714` with no 0x2711 in the top 10.

So the visible "armed stance with empty hands" while holstered can only come from locomotion (and transitions), and only where the wield table has non-default rows: **styles 2,3,4 with +0xE8 != 0 (dual/two-weapon classes), and styles 7,9 (right weaponwield 5/6 = rifle/heavy type: rows 0x154 walk, 0x156 run, per wave2_3.md:442/457 the `walkrf`/`runrf` clips hold the arms)**. Styles 0,1,5,6 (single saber/sword, stun baton etc.) already walk/run with row 0 / row 2 = the unarmed clips. Style 8 walk/run = row 0 / row 2 (`cmp` chain falls through) = same clips; verified from D86 .c 60-105.

### (e) 0x7EF412 caching (T3a OPEN): ANSWERED
`0x7EF3E0` (asm via disasm_va) is the `Animation*` predicate for the Walking column: `if ([this+0x68]) { id = vfn[0xE0](id) }` (0x7EF40C mov eax,[edx+0xE0]; 0x7EF412 call eax), stores the row back into the stack arg ([ebp+8]) and returns `FUN_0071B8B0(row, DAT_00A0FE30=Walking, &out)`-derived bool (0x7EF445..0x7EF454). No cache: row is recomputed per call, nothing is stored on the object, result is a bool. Nine sibling predicates at 0x7EF2F2/382/4A2/576/650/6D4/822/8B2/942 have the identical shape. Consequence: a holstered override is applied consistently to predicate and playback alike; the one risk is a one-frame inconsistency at the holster flip (predicate sees new row, the already-playing clip is old). Rows 0x154/0x156 vs 0 may differ in their Walking/Running flag columns (2DA not in repo, UNVERIFIED) so a flip while moving could restart the clip once. Harmless, covered by the play row.

## 1. Verified / corrected claims

| claim | verdict | anchor |
|---|---|---|
| T3a: 0x86367C is a 6 B READ of +0xD4 after the spill; hook there can write [EBP-1] | VERIFIED | D86 asm 0x863676-0x863682 |
| T3a: [EBP-1] has 19 reads all below 0x864439 | CORRECTED: 18 reads; one (0x864632, id 0x2806) is in the gate != 1 region | D86 asm 0x864632 |
| T3a: second read 0x863D3C (case 0x283E) not covered by [EBP-1] hook | VERIFIED | D86 asm 0x863D3C, .c:544 |
| T3a: wield 8 idle -> 0xF9 / 0x217; walk/run of 8 -> row 0 / 2 | VERIFIED | D86 .c:24-105 |
| T3a: override must also cover 0x27A4 (and 0x2713 alias) | VERIFIED (case labels share the arms) | D86 .c:24-25, 60-61 |
| T3a: caller 0x7EF412 caching UNKNOWN | RESOLVED: no cache, predicate recomputes per call | disasm 0x7EF3E0..0x7EF457 |
| T3b: "idle 0x2711 -> unarmed row, intended" and T3b OPEN 1 | CORRECTED: holstered creatures never request 0x2711 (combat flag 0 -> 0x2710/0x276C), idle already wield-independent | 77/007759b0.c, 77/00776fe0.c:112 |
| T3b: walk for ss/ds/st keyframes equal plain walk; only rifle differs | VERIFIED vs code: only styles 2,3,4 (+e8) and 7,9 select non-default walk/run rows | D86 .c:60-105 |
| T3b: direct +0xC5 write breaks attacks/flourish/ranged | VERIFIED | c_readers_combat.md:10-31 (melee 0x79CA3A, ranged 0x79CAEC, flourish gate 0x79F520/0x66B9CD) |
| T3b: id list conflict {2711,2712,2714} vs {2711,27a4} vs 5 ids | RESOLVED: filter = {2711,27A4,2712,2713,2714} | D86 .c case labels |
| T3c: "UNVERIFIED switch reads [EBP-1] not CL" | VERIFIED [EBP-1] | asm list above |
| T3c: H4 steals 0x863650-0x863655 only, no overlap | VERIFIED | hooks.toml:62-66 |
| T3c: B (tick write +0xC5) is SWAP but hits every reader | VERIFIED but REJECTED (see sec 2) | c_readers_combat.md:23-31, a_update_weapon_class.md:41 |
| T3c: A needs new export = REINSTALL | VERIFIED | BUILD_AND_INSTALL.md sec 4 table |
| C6 doc: "unarmed = 0" guess | REFUTED earlier (0 = left-only); unarmed = 8 | 0x76D590 decomp; live ANIMRES style=8 for several creatures |
| handoff: gate C6 must be 1 for the wield table | VERIFIED; live 10-16 ANIMRES shows gate 2 for 3 of 8 creatures (b31fe64, b1da308, b1da628 all style 8/gate 2): override is a no-op for gate-2 creatures | kotor2_logs.py check holster_log.txt ANIMRES line |
| T3c: holstered set "includes NPCs/enemies" | VERIFIED by code: HolsterPlace gate runs per weapon Gob of any creature | Holster.cpp:1167-1177 |
| prior: ledger 0x7645F0 writes +0xC5 | REFUTED already (word at +0x314) | T3a sec 6 |
| name_map label for 0x7759B0 "GetCameraFollowPart" | WRONG for behaviour (returns idle anim id); REA ledger row is right | query_ghidra.py address 0x7759B0 |

## 2. Decisions

**D1 (A vs B vs third): choose A, a gated detour at 0x86367C writing only the [EBP-1] byte, but NOT as the first step.** B is rejected (any tick write of +0xC5 is lost on every 0x76E130 slot 0x10/0x20 call and, while set, makes attacks use the unarmed melee family, kills flourish and ranged anims: c_readers_combat.md:23-25). Third option C (reuse the existing H4 entry hook to write +0xC5 at entry and restore on the next call) is rejected: no post-resolver hook exists, so +0xC5 stays 8 across frames in which attack code can read it. Third option D ("A plus tiny gate function") IS the recommended shape: the hook handler is the gate.

**D2: the first step is a PROBE, and it is a SWAP.** Because holstered idle is already wield-independent, A may turn out to change nothing for the party (all party styles 1/5/6/0, or gate 2). A one-DLL probe answers that before paying a reinstall. Probe needs no new hook: extend the existing H4 `HolsterAnimResolve` ANIMRES line (Holster.cpp:440-467, DLL-only change, SWAP) with the holster flag from the placement set (see steps 1-3). No attack/flourish hooks are needed since A never writes +0xC5 (T3b's attack/flourish probe lines are dropped).

**D3: filter ids** = {0x2711, 0x27A4, 0x2712, 0x2713, 0x2714}. 0x2711/0x27A4 are kept only as a cheap defence (any non-idle caller requesting the ready idle while the creature is holstered and combat flag 0 should get the unarmed row); they will not be hit by the normal idle chooser. Not touched: 0x2710 family, 0x27AD/0x27AE (flourish gate reads the real byte and returns on 1/8 anyway), 0x2806, 0x283C-0x283E, 0x28BD.

**D4: write only when it changes the result**: gate byte `[ebp-0xD] == 1` AND `[ebp-1]` in {2,3,4,7,9} (styles with non-default walk/run/idle rows; for 0,1,5,6 writing 8 is a no-op for walk/run and 0 is a pass-through). Style 8 stays 8.

**D5: second read 0x863D3C is out of scope** (case 0x283E is a ready-idle variant; the holstered creature does not ask for it, and its constant-table value is fixed). Log it in the probe by adding 0x283C..0x283E, 0x2806, 0x28BD to the interesting-id list, so an unexpected holstered request shows. If one appears, a second tiny detour at 0x863D3C (`8A 88 C5 00 00 00`, 6 B, same shape) can write CL; not planned now.

**D6: replace H4 rather than add a second resolver hook**, at the reinstall step. Every resolver call passes 0x86367C (no early return before it), so one handler at 0x86367C can both log (style before/after, id, gate, d4, e8; same params as H4) and override. One wrapper cost per resolver call, the same as today (the resolver runs for every animation of every creature every frame; wrapper cost UNMEASURED, so the handler's first 6 lines must be pure integer compares: id filter, then gate, then style set, then the set lookup).

**D7: scope = every creature in the holstered set**, not party only, because placement (and therefore the visible holster) already applies to any weapon Gob. The set stores `{owner, animBase, lastSeenTick}` and is filled by the existing H7 `HolsterPlace` drawgate branch (Holster.cpp:1167-1177): add when `hip` is true, remove when false. The handler verifies at call time (never trusts the set alone): entry stamp within 500 ms, `*(owner+0x68) == self`, `owner+0x138 == 0` (readable), else no write. Failure mode = armed stance (vanilla), never a crash.

**D8: kill switch**: `holster.ini unarmed_anim=1` (default 1 once proven; default 0 for the first reinstall build), live-reloaded like the other tune keys; 0 -> handler only logs.

**Answers to the OPEN items**:
- T3a: caller 0x7EF412 caching -> none (sec 0e). Server idle -> untouched (+0x504). Indirect callers of 0x76D590 remain unverified but irrelevant to A (A never writes +0xC5).
- T3b 1 (idle id out of combat) -> 0x2710/0x276C, wield independent (sec 0d). 2 (row names) -> rows are numbers; names need the 2DA (not in repo): UNVERIFIED, not needed. 3 (id 0x2718, ~1900 hits) -> resolver row 0x53 for all styles (D86 .c:113-115), wield independent; meaning still unknown, irrelevant. 4 (id list) -> D3. 5 (0x863D3C) -> D5. 6 (what sets gate 1) -> still OPEN; the probe logs gate per creature (live 10-16 shows both 1 and 2 on style-8 creatures). 7 (0x76E130 on combat start/swap) -> irrelevant to A. 8 (first attack frame before hip-to-hand swap) -> irrelevant to A; attacks read the real byte.
- T3c 10: wield switch reads [EBP-1] -> yes. Gate C6 flag -> open (probe). Walk row for 8 -> row 0 (walk), row 2 (run); 0x154 is the rifle row, not the default (T3c said "default row 0x154": wrong).

## 3. Implementation steps (ordered)

Rule for every address: no raw hex into any `raw_addresses.toml` (none exists for holster; if one is added) or `GameAddr.h`/`K2Addr.h` before `python3 scripts/query_ghidra.py address <hex>`. Pre-checked: `address 0x863650` = resolver (confirmed); `address 0x7759B0` = idle-id chooser (confirmed); `address 0x86367C` returns "No results" because it is mid-function (a hook site, not a symbol): verify it with `check_hook_bytes.py`, not query_ghidra.

PHASE P (probe, SWAP, game closed or not, no reinstall):
1. S: new pure header `patch_manager_mods/holster_src/HolsterAnimStyle.h` + colocated `holster_animstyle_test.cpp` (add a `[holster_animstyle_test]` table to test.toml only if flags differ; default is `-I../_shared_src -Werror`). Contents: `idInFilter(id)`, `styleHasOwnLocoRow(style)` ({2,3,4,7,9}), `decide(id, gate, style, inSet, ownerCombatZero)` -> new style or unchanged, and `HolsterSet` (fixed 64 entries `{owner, base, stamp}`; add/remove/find/expire; wrap-safe tick compare, same convention as `holdActive`). Tests: each id x gate {0,1,2} x style {0..9} x {inSet, notInSet, combat!=0, stale stamp}; set overflow (65th entry refused, never overwrites); tick wrap; remove then find.
2. S: `Holster.cpp` HolsterPlace drawgate branch (after `hs::gateUpdate`, ~line 1177): `set.touch(owner, base=*(owner+0x68), now)` when `hip`, `set.remove(owner)` when not. `base` read through `rdU32Ok(kSitePlace, owner, 0x68, &base)`. No new hook, no new export.
3. S: `HolsterAnimResolve` (Holster.cpp:440-467): add `hol=%d` (set lookup of `self`) to the ANIMRES line and to the first-sighting key `(self,id,style,hol)`; widen `animInteresting` to include 0x283C..0x283E, 0x2806, 0x28BD; also log `comb=` (`owner+0x138` through the set entry's owner) so "holstered with combat flag != 0" would show. Keep the per-id cap (4 / 20 verbose). Update `manifest.toml` version patch bump (0.4.1-dev; versions stay 0.x.y per memory rule) and the description sentence.
4. S: gate: `python3 scripts/run_tests.py holster` (scoped), `scripts/compat_check.py patch_manager_mods/holster_src/Holster.cpp` (only if `_shared_src` touched), then `kotor2-kpm-install` skill flow: `kpm_hotswap.py --mod holster --prepare`, fresh `scripts/pre_install_gate.sh --mod holster_src` GATE PASS, `--apply` only on user GO with game closed. Verdict must read SWAP (BUILD_AND_INSTALL.md sec 4: no hooks/exports/imports/config change).
5. S: add the play row (HW7a-P below) to `patch_manager_mods/PLAY_TEST_QUEUE.md` (live-only checks never block).
DECISION GATE after the probe: if no holstered creature shows gate=1 with style in {2,3,4,7,9} and `id` in {2712,2713,2714}, STOP: option A would be a no-op for the tested party; record it in doc 42 and close HW7.

PHASE A (only if the gate passes; REINSTALL):
6. S: `kotor2-steam-aspyr.hooks.toml`: replace H4 (0x00863650) by H4b:
   `address = 0x0086367C`, `type = "detour"`, `function = "HolsterAnimStyle"`, `original_bytes = [0x8B,0x82,0xD4,0x00,0x00,0x00]`, `skip_original_bytes = false`, `exclude_from_restore = []`, params: `source = "ebp"` (pointer), `source = "[ebp-0x34]"` (pointer, anim base), `source = "[ebp+8]"` (uint, mask 0xFFFF in C++). Update the header comment (the old "H4 ... 6 bytes PUSH EBP" text). `exports.def`: replace `HolsterAnimResolve` by `HolsterAnimStyle`. Run `python3 scripts/check_hook_bytes.py --mod holster` (instruction boundary, no rel branch in the stolen 6 B, bytes equal the vanilla exe) and `scripts/check_hook_spans.py`. Packaging check (embedded hooks vs source) FAIL if the .kpatch is stale.
7. M: `Holster.cpp`: handler `extern "C" void __cdecl HolsterAnimStyle(void* ebp, void* self, uint32_t idRaw)`: (1) `maybeReload()` first (it lived in the old handler), (2) log branch exactly as old ANIMRES but with `style_in`, `style_out`, (3) override branch: `id = idRaw & 0xFFFF`; `if (!g_unarmedAnim || !idInFilter(id)) return`; gate = `*(u8*)(ebp-0xD)`, style = `*(u8*)(ebp-1)` read via `saferead`; `decide(...)`; on a change store one byte `*(volatile uint8_t*)(ebp-1) = 8` via the module's `probeWrite`/checked writer (T3c: `writeU32Safe` exists, no byte variant: add `writeU8Safe` next to it, Holster.cpp:549-553; guard `ebp` non-null and 4-aligned, since the frame is the game's own stack and a bad EBP must not fault). Log once per `(self,id,style_in)` change, rate-limited by the existing `takeLine`.
8. S: `holster.ini` + `HolsterTune`/ini parse: key `unarmed_anim` (0/1, live reload, default 0 in the first reinstall build, 1 after the play rows pass) and its commented line in holster.ini. Add to `HolsterEdit.h` page only if the user wants a menu row (the mod-config value test says on/off = install state, so NO menu row).
9. S: add `Holster_AnimResolve`/new site constant `AnimStyleSite = 0x0086367C` to `_shared_src/GameAddr.h` (after `query_ghidra.py address 0x863650` as above, comment "mid-function hook site, not a symbol") and pin it in `common_contract_test.cpp`; re-run `run_tests.py common_contract` and `compat_check.py` since `_shared_src` changed.
10. M: update `holster_src/test.toml` if the new test needs flags; run `run_tests.py holster`, `pre_install_gate.sh --mod holster_src` GATE PASS (expect verdict REINSTALL: `kpm_hotswap.py --prepare` refuses to hot-swap; do not force).
11. S: install via `KPATCH_NO_LAUNCH=1 .../KPatchLauncher swkotor2.exe --patches <dir> <all live ids>` on user GO, game closed, rollback noted in the queue row. Only after step 4 probe evidence.
12. S: docs: doc 42 HW7 status line, `holster` manifest description (0.5.0-dev: minor = new behaviour), `.claude/handoffs/holstered_weapons.handoff.md` via `plan_manager.py mark-done`.

## 4. Install type (with proof)

- PHASE P (probe): **hot-swap**. Only DLL code changes: same hooks, same exports (`exports.def` unchanged), no new imports (no new Win32 call; `writeU8Safe` is not used in the probe), same `patch_config.toml`. BUILD_AND_INSTALL.md section 4 table row 1: "Only DLL code changed ... Hot-swap (kpm_hotswap.py)". Use the gate: `pre_install_gate.sh --mod holster_src` fresh GATE PASS after `--prepare` before `--apply` (Verify-After-Fix invariant).
- PHASE A (override): **full reinstall**. Row 2 "Hooks added, removed or changed (verdict REINSTALL) -> Full reinstall" and row 3 "DLL exports changed ... Full reinstall (--apply refuses these on PE checks)". The new detour 0x86367C and the export `HolsterAnimStyle` trigger both. T3c's classification stands. Game closed, user GO required.
- B (rejected) would have been SWAP; that is the only thing it has going for it.
- Alternative to avoid a reinstall: none found. An in-place `code_patches.toml` byte patch (the bone_test pattern, `holster_src/code_patches.toml`) needs a fixed conditional-free replacement; here the condition (holstered?) lives in the DLL, so a detour/cave is mandatory.

## 5. Play rows (add to PLAY_TEST_QUEUE.md, status `pending`)

| id | steps | pass signal |
|---|---|---|
| HW7a-P (probe, SWAP build) | holster.ini `probe=1 ch_anim=1 log_verbose=0`. Load a save with 3 party members (Atton heavy blaster/pistols, Kreia sword if present). Holster (default; weapons on hip/back), stand 10 s, walk 10 s, run 10 s, enter combat, one attack, leave combat, wait for holster, walk again. | `kotor2_logs.py check holster_log.txt` ANIMRES lines: (1) holstered creatures (`hol=1`) show only ids 2710/276C/2712/2713/2714, never 2711; (2) at least one holstered member with `gate=1` and `style` in {2,3,4,7,9} for 2712/2714 (A would change something) OR the table shows none (then STOP, close HW7 as no-op); (3) `comb=0` on every `hol=1` line. |
| HW7a-A (override, reinstall build) | same save, `unarmed_anim=1`. Holster Atton (heavy blaster), walk and run 10 s each; draw by F8 hold, walk; enter combat, shoot; re-holster. | Atton walks/runs with arms down while holstered (no rifle arm hold), ANIMRES `style_in=7/9 style_out=8` only while `hol=1`, and in combat `style_in==style_out` (no override); his ranged attack animation plays and the blaster fires normally. |
| HW7b (flip while moving) | run while holstered, press F8 draw hold mid-run, then re-holster mid-walk. | No crash; at most one clip restart at each flip; no T-pose/stuck pose; log has no `PLACE refuse` burst. |
| HW7c (attacks/flourish unchanged) | holstered party, start combat on a dummy, attack with melee and ranged members, trigger flourish. | Attack, ranged and flourish animations play exactly as with `unarmed_anim=0` (the hook never touches +0xC5). |
| HW7d (kill switch) | set `unarmed_anim=0` live (no restart). | Within 0.5 s walking again shows the armed rows (`style_out==style_in` in log). |
| HW7e (save/load, area change) | save while holstered, load, then change area and return. | No crash; set repopulates within a few frames; no stale override on drawn creatures (log shows `hol=0` on drawn). |

## 6. Risks

- **Crash**: handler writes one byte into the resolver's own stack frame. Mitigations: ebp null/alignment guard, `saferead` for every game read, no game call from the handler, writes only for gate==1 and style in {2,3,4,7,9}. A wrong param mapping (`[ebp-0x34]` vs `ebp`) would write the wrong stack byte: step 6 test must assert on a host fake frame, and HW7a-A checks `style_out` in the log. FXSAVE cost per resolver call is UNMEASURED; resolver ~3900 calls in 26 s of load (Run 1); the handler's first test is an integer id compare.
- **Hook interplay**: replacing H4 changes `maybeReload()` timing (it runs from the resolver; keep it as the first statement). robustness/perf-lab have no hook in 0x8636xx; rg-verified, re-run `check_hook_spans.py`.
- **Save/load**: nothing persisted (no game state write; client anim base only). The holstered set is volatile; entries expire in 500 ms; area change/recycled pointers are rejected by the three-way verification (stamp, `owner+0x68 == self`, `owner+0x138 == 0`).
- **Other creatures**: enemies/NPCs with a drawgate-placed weapon get the unarmed locomotion rows while out of combat; if a hostile aggros the combat flag becomes nonzero at once and the handler stops overriding even before the placement set updates. Creatures with gate 2 (3/8 in the 10-16 log) are untouched. Creatures with a resolver that is not CSWCAnimBase-family never reach this code.
- **Restore on unload**: A holds no state in the game: removing the hook restores the 6 stolen bytes; a set entry is DLL memory. B would have left +0xC5 = 8 until the next equip change (reason B is rejected).
- **Residual armed stance**: 0x283E direct read (D5), 0x27AD/0x27AE flourish and 0x2806/0x28BD transitions keep armed rows. If HW7a shows such ids with `hol=1`, add a second detour at 0x863D3C or extend the filter; do not widen blindly.
- **Naming/doc hygiene**: ledger labels "creature+0xD4/+0xC6" are anim-base offsets (T3a label fix stands); the name_map label for 0x7759B0 is misleading; the master-plan handoff line 113 (hook 0x863670/CL) and the handoff list {2711,2712,2714} are stale and should be edited to match D3/D6.
- **Value**: most single-weapon styles already walk and idle unarmed. The change helps rifle/heavy and dual-wield carriers only. This is the honest scope and the reason the probe goes first.
