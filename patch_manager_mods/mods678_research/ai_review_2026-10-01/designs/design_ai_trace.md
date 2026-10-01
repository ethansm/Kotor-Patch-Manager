# AI trace layer (KPM DLL hooks): design

Date: 2026-10-01. Target: Steam Aspyr `swkotor2.exe` 6A522E71, and the LAA build 4AB72FC1 (every window below is byte-identical in both).
This is a static design only. Nothing was built, installed or run in the game. Ghidra was not run; all evidence comes from the existing dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/` (`.asm` checked for every hook) and the knowledge DB.

## 0. Summary

- **Command indices** come from the prototype order in `nwscript.nss`. The file has 877 prototypes, matching the 0x36D-entry table. The order-to-index mapping is verified against four known commands in InitializeCommands `0x665F50`.

  | command | index | table offset | handler |
  |---|---|---|---|
  | `PrintString` | 1 | +0x4 | `0x68F260` |
  | `SendMessageToPC` | 374 | +0x5D8 | `0x690A90` |
  | `AurPostString` | 582 | +0x918 | `0x6F6200` (filled by the second init function `0x6F5B80`) |
  | `ShipBuild` | 761 | +0xBE4 | `0x692C40` |

- **ShipBuild returns its value through one local.** The handler stores a constant in `[EBP-4]` and pushes it with `StackPushInteger 0x6FD9C0`. The hook sits after that store and overwrites `[EBP-4]` with 0, but only for traced scripts.
- **Script name and OBJECT_SELF in any handler.**
  - `VM = [0xA1B4A8]`, `VMI = [VM+0x1C]`, `depth = [VMI+0x28]`.
  - The name is a CExoString at `VMI+0x44+depth*0x28`.
  - OBJECT_SELF is `[VMI+0x18C+depth*4]`, mirrored at `[impl+8]` with `impl = [VMI+0x3D4]` (the handler's ECX).
  - The ExecuteScript event id ("RunScriptVar") is the global `[0xA7E10C]`.
- **A global ShipBuild flip would also be behaviour-safe, but this design still scopes it.**
  - A read-only scan covered every NCS the install can load: 34,927 instances, 4,883 names, from chitin, modules and Override.
  - It found 518 `ShipBuild` calls, and all of them are `if(!ShipBuild())`.
  - The guarded blocks contain only `PrintString` (495), only `AurPostString` (5, which is a pop-only no-op in retail), or nothing (18). No guarded block contains a JSR.
  - Scoping is therefore a volume and cost choice, and a guard against future script mods. Correctness does not depend on it.
- **Hooks: 4 VM command sites plus 5 engine log points (9 windows).** All pass `check_windows.py` (copied to the scratchpad and extended) and match both executables. None touch perf-lab's VM hooks or any live, ranged-ai or inventory window.
- **Recommendation: a new patch id `companion-ai`, not `ranged-ai`.** The decisive reason: the planned FM1 fix needs the same address (`0x586F43`) as the FM1 log point, so the trace and the future engine AI fixes must share one DLL (section 9).

## 1. Command indices and handlers (evidence)

The index is the 0-based position in `nwscript.nss`, after stripping comments, counting `type Name(` prototypes. The count is 877, matching `CMP dword ptr [EBP+0x8],0x36d` at `0x668FD9`.

The mapping was verified against commands whose handlers were already identified (U1, map OK):

| command | index | slot = idx*4 | table write (asm, `0x665F50`) | known handler |
|---|---|---|---|---|
| AssignCommand | 6 | 0x18 | `006663e9 c74118f0d96600 MOV dword ptr [ECX + 0x18],0x66d9f0` | 0x66D9F0 (U1 map OK) |
| DelayCommand | 7 | 0x1C | `006665e0 c7421c70036700 MOV dword ptr [EDX + 0x1c],0x670370` | 0x670370 (U1 map OK) |
| ActionDoCommand | 294 | 0x498 | `00666025 c78298040000d0996600 MOV dword ptr [EDX + 0x498],0x6699d0` | 0x6699D0 (U1) |
| GetRunScriptVar | 565 | 0x8D4 | `00667cd2 c782d4080000b07f6800 MOV dword ptr [EDX + 0x8d4],0x687fb0` | body is `MOV EAX,[0x00a7e10c]` |

The four requested commands:

| command | idx | slot | table write | handler (name_map, mapper "medium") |
|---|---|---|---|---|
| PrintString | 1 | 0x4 | `006685ba c7420460f26800 MOV dword ptr [EDX + 0x4],0x68f260` | 0x68F260 `Global::ExecuteCommandPrintString` |
| SendMessageToPC | 374 | 0x5D8 | `00668733 c782d8050000900a6900 MOV dword ptr [EDX + 0x5d8],0x690a90` | 0x690A90 `Global::ExecuteCommandSendMessageToPC` |
| AurPostString | 582 | 0x918 | `006f5b8d c7811809000000626f00 MOV dword ptr [ECX + 0x918],0x6f6200` (in `0x6F5B80` InitializeSWMGCommands, called at `00668fc1 CALL 0x006f5b80`) | 0x6F6200 `Global::ExecuteCommandAurPostString` |
| ShipBuild | 761 | 0xBE4 | `00668b87 c781e40b0000402c6900 MOV dword ptr [ECX + 0xbe4],0x692c40` | 0x692C40 `Global::ExecuteCommandShipBuild` |

The print family clusters as expected: PrintFloat idx 2 at 0x68EEF0, PrintInteger idx 4 at 0x68F0C0, PrintObject idx 5 at 0x68F200, PrintVector idx 141 at 0x68F3D0. Each of the four handlers fills exactly one slot and has no direct callers in the dump, so a hook in a handler fires only for its own command.

**Dispatch and return convention.**
- `ExecuteCommand 0x668FD0` makes the call `table[id](id, argc)` with `ECX = impl`, `RET 8`:
  ```
  00668ff1 MOV EAX,[EBP+0xc] / PUSH EAX / MOV ECX,[EBP+8] / PUSH ECX / ... MOV ECX,[EBP-8] / MOV EAX,[EAX+EDX*4] / CALL EAX
  ```
- A handler pops its arguments and pushes its return value through the global VM:
  - `MOV ECX,[0x00a1b4a8]`, then one of: `CALL 0x6FD9C0` push int, `0x6FDA70` pop string, `0x6FD9A0` pop int, `0x6FD9E0` pop float, `0x6FDAF0` pop object (each a thin wrapper that forwards to `[VM+0x1C]`).
  - The handler returns 0 on success. The error codes are `0xFFFFF82F` (pop failed), `0xFFFFF830` (push failed) and `0xFFFFF82E` (NULL slot).
- A negative result aborts the script: `0070029e CMP [EBP-0x70],0 / 007002a2 JGE 0x7002ac / 007002a4 MOV EAX,[EBP-0x70] / JMP 0x701e58`. So no hook may ever make a handler fail.

**ShipBuild `0x692C40`** (`thiscall(impl)`, stack `(id, argc)`, `RET 8`):
```
00692c49  c745fc01000000        MOV dword ptr [EBP + -0x4],0x1      ; the value ShipBuild() returns
00692c50  c70514e1a70001000000  MOV dword ptr [0x00a7e114],0x1      ; write-only global (no reader anywhere in the dump)
00692c5a  8b45fc                MOV EAX,dword ptr [EBP + -0x4]
00692c5d  50                    PUSH EAX
00692c5e  8b0da8b4a100          MOV ECX,dword ptr [0x00a1b4a8]
00692c64  e857ad0600            CALL 0x006fd9c0                      ; StackPushInteger(value)
```
So the hook at `0x692C50` overwrites `[EBP-4]` before `0x692C5A` reads it.

**PrintString `0x68F260`.**
- It pops the string into a CExoString at `[EBP-0x18]` (char* at `-0x18`, length at `-0x14`).
- When `argc >= 2` it also pops an int into `[EBP-0x10]`. Otherwise it stores 0 there at `0x68F301`. All 512 compiled PrintString calls in the install use argc = 1, so `0x68F301` runs on every call.
- It then formats `"PRINTSTRING: %s\n"` (string `0x998D18`) through `CExoString::Format 0x734270` into the engine debug object `[[0xA1B488]+8]`. It writes that out through `0x73A5C0`/`0x73A510` only when the object's sink `[+8]` is non-NULL. Whether a sink is open in retail is not known (open question 3).
- The success tail starts at `0068f39f MOV [EBP-0x24],0` and runs the CExoString dtor, then `RET 8`.

**AurPostString `0x6F6200`** pops string `[EBP-0x20]`, int x `[EBP-0x10]`, int y `[EBP-0x14]` and float life `[EBP-0x18]`, then returns 0 at `006f62f2 MOV [EBP-0x34],0` without drawing anything. It is a retail stub.
- k_ai_master calls it **unconditionally** in 9 places, for example the ON_PATH_BLOCKED event 1012: `AurPostString(GN_ReturnDebugName(oBlocker),0,15,2.0)`. The capture therefore yields data even with the flip off.
- The KPM `EnableScriptAurPostString` community patch is KotOR 1 only (GOG 1.03 hashes, hook `0x5CB41C`), so nothing competes for this site.

**SendMessageToPC `0x690A90`.**
- It requires argc == 2, pops the object `[EBP-0x10]` and the string `[EBP-0x1C]`, and resolves the creature with `CServerExoApp::GetCreatureByGameObjectID 0x51C100`.
- It builds a 0x34-byte message and calls `SendFeedbackMessage 0x584DA0(0xCC, msg)`. The success tail is `00690bc8 MOV [EBP-0x38],0`.
- **No AI-set script calls it:** there are 0 calls across the 17 traced scripts, and only 37 calls in 13 NCS overall. Its hook is therefore optional and is left out of v1.

## 2. VM state: running script name and OBJECT_SELF

| what | how to read it inside any command handler | evidence |
|---|---|---|
| CVirtualMachine | `VM = *(int*)0xA1B4A8` | every handler: `MOV ECX,[0x00a1b4a8]; CALL 0x6fd9c0/...` |
| CVirtualMachineInternal | `VMI = [VM+0x1C]` | `006fdafe 8b491c MOV ECX,[ECX+0x1c]; CALL 0x00703070` (pop object wrapper); U1 `0x6FD922` |
| depth (0..7, -1 idle) | `[VMI+0x28]` | ReadScriptFile `00701feb CMP [EAX+0x28],0x8`; ctor `param_1[10]=-1` |
| **script name** | CExoString at `VMI + 0x44 + depth*0x28`; `char* = [that]` (slot = `VMI+0x2C+depth*0x28`, name = slot+0x18) | ReadScriptFile `00702092 MOV EAX,[EDX+0x28] / 00702095 IMUL EAX,EAX,0x28 / 0070209b LEA ECX,[ECX+EAX+0x44] / 0070209f CALL 0x007337c0` (CExoString::operator=(name)); situations: SetUpScriptSituation `00702a8b LEA ECX,[EAX+EDX+0x44] / CALL 0x7337c0` (copies `sit+0x18`); RunScriptFile error report reads it: `0070290e LEA ECX,[EAX+EDX+0x44] / CALL 0x734010` (CStr) |
| **OBJECT_SELF** | `[VMI + 0x18C + depth*4]`, mirrored to `[impl+8]`, impl = `[VMI+0x3D4]` (= ECX at handler entry; saved in each handler's frame) | RunScript `0070235e MOV EAX,[EBP+0xc]` (2nd arg = object id) `/ 00702361 MOV [EDX+ECX*4+0x18c],EAX` ... `007023c1 MOV ECX,[EDX+ECX*4+0x18c] / 007023c8 MOV [EAX+0x8],ECX` (EAX = `[VMI+0x3d4]`); RunScriptSituation 0x702590 does the same; restored for the parent at script exit |
| `[impl+4]` | `[VMI+0x16C+depth*4]` = RunScript 3rd arg | `0070234e MOV [ECX+EAX*4+0x16c],EDX` (EDX=`[EBP+0x10]`), `007023ac MOV [ECX+0x4],EDX` |
| event id (RunScriptVar) | global `*(int*)0xA7E10C` | ExecuteScript `0x679050`: `DAT_00a7e10c = local_20` then `RunScript 0x6FD8D0(name, oid, 1)` (synchronous, depth+1); GetRunScriptVar `0x687FB0`: `00687fb7 MOV EAX,[0x00a7e10c]` |

Notes:
- **Nesting.**
  - Companion event scripts `k_hen_heartbt01`, `k_hen_combend01` and the other `k_hen_*01` scripts are one-line `ExecuteScript("k_ai_master", OBJECT_SELF, <event>)`. k_ai_master therefore runs at depth+1, with the k_ai_master name in its own slot and the correct event in `[0xA7E10C]`.
  - Delayed, assigned and ActionDo commands created inside k_ai_master keep the name "k_ai_master": StackPopCommand copies it into the situation, and SetUpScriptSituation writes it back to the slot.
  - So the per-depth name check scopes correctly in all these cases.
- **Object pointer for the tag and party filters.**
  - Call `CServerExoApp::GetGameObject 0x51C0A0` as `__thiscall(ECX = [[0xA1B4A4]+8], id)`, `RET 4`. It returns NULL for stale or invalid ids. Asm: `006d77ca MOV EDX,[0x00a1b4a4] / MOV EAX,[EDX+8] / ... MOV ECX,[EBP-0x288] / CALL 0x0051c0a0`; inside: `0051c0ac MOV ECX,[EAX+0x4]`, `0051c0db RET 0x4`.
  - **The DB `functions` row says "cdecl(id)"; that is wrong.**
- **Fields read from the object.** The type byte is at `[obj+8]` (5 = creature, as in ranged-ai). The tag is a CExoString at `obj+0x18` for creatures; GetTag handler `0x689560`: `FUN_007337c0(local_18 + 6)` for the non-special types. Party is `[cr+0x11AC]` and IsPC is `[cr+0x10E8]`.
- **Name.** `GetName` is a localized-string lookup, so it is not cheap. The tag is used instead. k_ai_master's own strings already embed `GetName(o) + "_" + ObjectToString(o)` (GN_ReturnDebugName).

## 3. Hooks

All hooks are cdecl detours, following the ranged-ai style. Parameters come from registers, `ebp` included, so the handler reads frame slots itself. Every handler starts with `saferead::beginScope()` and wraps its whole body in `if (!g_cfg.enable) return 0;`.

### 3a. VM command sites (gated by `scripts=1`)

| id | address / len | stolen bytes | fn | params | consumed | handler does |
|---|---|---|---|---|---|---|
| **T1** ShipBuild value | `0x692C50` / 10 | `C7 05 14 E1 A7 00 01 00 00 00` (`MOV [0xA7E114],1`) | 0x692C40 | `ebp` | no; `exclude_from_restore = []` | If the current script is traced and OBJECT_SELF passes the filter, write `*(int*)(ebp-4) = 0` (ShipBuild() = FALSE) and count a flip; otherwise leave vanilla 1. Fail-safe: any read failure leaves 1. The stolen store runs after the handler and only touches the dead global. |
| **T2** PrintString capture | `0x68F301` / 7 | `C7 45 F0 00 00 00 00` (`MOV [EBP-0x10],0`, EAX-free) | 0x68F260 | `ebp` | yes, exit `0x68F39F`; `exclude_from_restore = ["eax"]` | Text = `*(char**)(ebp-0x18)` (len `[ebp-0x14]`). If it is from a traced script (or `prints_all_scripts=1`) and the object passes, write a `P` line. Return 1 only when `engine_print=0` and the line was traced: that skips the engine's `PRINTSTRING:` format and sink write and jumps to the vanilla success tail (`[EBP-0x24]=0`, string dtor, `RET 8`; same frame and SEH state as `0x68F301`, no pushes pending). Default `engine_print=1` returns 0, the vanilla path. |
| **T3** AurPostString capture | `0x6F62F2` / 7 | `C7 45 CC 00 00 00 00` (`MOV [EBP-0x34],0`) | 0x6F6200 | `ebp` | no; `[]` | Text `*(char**)(ebp-0x20)`, x `[ebp-0x10]`, y `[ebp-0x14]`, life `*(float*)(ebp-0x18)`. If traced, write an `A` line. Reached only after all four pops succeeded (JNZ target from `0x6F62D5`). |
| T4 SendMessageToPC (optional; omit in v1) | `0x690BC8` / 7 | `C7 45 C8 00 00 00 00` | 0x690A90 | `ebp` | no | Text `*(char**)(ebp-0x1C)`, target `[ebp-0x10]`; writes an `M` line. Zero AI-set callers, so it is not shipped in v1. The window is validated in case it is wanted later. |

T1 pseudo-code:
```cpp
struct Ctx { int depth; const char* script; int self; int ev; };
static bool vmCtx(Ctx& c) {                       // all reads via saferead (cached region => ~1 VirtualQuery per scope)
    int vm = rd<int>(0, 0xA1B4A8), vmi = vm ? rd<int>(vm, 0x1C) : 0;
    c.depth = vmi ? rd<int>(vmi, 0x28, -1) : -1;
    if (c.depth < 0 || c.depth > 7) return false;
    int strp = rd<int>(vmi, 0x44 + c.depth * 0x28);
    c.script = strp && probe(strp, 1) ? (const char*)strp : "";   // copy at most 32 chars, stop at NUL
    c.self = rd<int>(vmi, 0x18C + c.depth * 4, 0x7F000000);
    c.ev = rd<int>(0, 0xA7E10C);
    return c.script[0] != 0;
}
extern "C" void __cdecl AiTraceShipBuild(int ebp) {
    saferead::beginScope(); loadIni(false);
    if (!g_cfg.enable || !g_cfg.scripts || g_cfg.shipScope == ScopeOff) return;
    Ctx c; if (!vmCtx(c)) return;
    if (g_cfg.shipScope == ScopeAi && !inTraceSet(c.script)) return;
    if (!objPasses(c.self)) return;                     // party_only / tags / log_all (cached 1 s per id)
    *reinterpret_cast<int*>(ebp - 4) = 0;  ++g_st.flips;
}
```

`inTraceSet`:
- It is a case-insensitive compare against the INI list. A trailing `*` acts as a prefix wildcard, and the `k_hen_*dlg` form (one `*` in the middle) is a prefix plus suffix match.
- The default list has 17 names: `k_ai_master,k_hen_*dlg,a_atkonend,a_force_combat,k_combat_rnd`. `k_hen_*dlg` matches exactly the 13 vanilla `k_hen_<name>dlg` scripts: atton, baodur, discip, g0t0, hanharr, hk47, hndmaid, kreia, mand, mira, remote, t3m4, visas.

`objPasses(id)`:
- It keeps a 32-entry id cache of `{type, party, tag[33], tick}`, refreshed after 1 s, and resolves misses via `0x51C0A0`.
- The rule is `log_all || (party_only && type==5 && party) || tagListed(tag)`.

### 3b. Engine log points (gated by `engine_points=1`; the party filter applies to the subject creature)

| id | address / len | stolen bytes | fn | params | consumed | what it measures |
|---|---|---|---|---|---|---|
| **E1** A1 attack entry | `0x6D740F` / 10 | `C7 80 70 05 00 00 B8 0B 00 00` (`MOV [EAX+0x570],0xBB8`; EAX = attacker `[ebp-0x49C]`) | 0x6D7390 | `eax` | no; `[]` (EAX is restored before the stolen bytes, so the stolen bytes may use it) | Every entry of AIActionPhysicalAttacks: the prologue up to here is straight-line, with no branch, CALL or RET before `0x6D740F`. Per-creature slot (16, keyed by `[eax+4]`): `++perSec`. **Burst** tracking uses QPC µs: entries less than 2 ms apart count as one AIUpdate's RunActions loop (that loop is capped at 1000 µs). It also records `f570` (the value before the reset = fallback ms left). |
| E1b A1 approach exit | `0x6D8BE2` / 5 | `A1 B8 44 99 00` (`MOV EAX,[0x9944B8]`, return 2) | 0x6D7390 | `ebp` | no; `[]` | Counts approach-branch exits per creature (attacker `[ebp-0x49C]`). Snapshots stance `[ebp-0x174]` (after ranged-ai E-H3 has mapped 16 to 11), back-off `[ebp-0x168]`, controlled id `[ebp-0x16C]`, ranged `[ebp-0x164]`, dist² `[ebp-0xBC]`, max `[ebp-0x2C]` and los `[ebp-0xD0]` into the creature slot. This is only a snapshot; E1's summary prints it. Also reached from the back-off and Force-jump exits, so the snapshot carries the back-off flag. |
| **E2** A2 fallback fires | `0x569A08` / 6 | `8B 8D 6C FE FF FF` (`MOV ECX,[EBP-0x194]`) | 0x569320 | `ebp` | no; `[]` | Reached only when `+0x520==1`, `+0x570<=0` and vtbl+0x94 (dead) returned 0, i.e. exactly when slot 6 is about to run (`0x569A67 CALL RunScript`). Logs one line per fire: creature `[ebp-0x194]`, slot-6 script name `*(char**)(cr+0x2A0)`, round active `[[cr+0x10DC]+0xA84]`, and the creature's current E1 state. |
| **E3** P1 perception (block 2) | `0x56BDBD` / 8 | `6A 01 8B 8D 2C FF FF FF` (`PUSH 1; MOV ECX,[EBP-0xD4]`; the next insn `CALL 0x5D0F30` is not stolen) | 0x56B8A0 | `ebp` | no; `[]` | Each `UpdatePerception(1)` from SpawnInHeartbeatPerception block 2. Creature `[ebp-0xD4]`, elapsed `[ebp-0x28]` (ms since the never-written stamp `+0x3A0/+0x3A4`), flag `+0x38C` (0 means the JZ at `0x56BD40` jumps straight here), IsPC `+0x10E8`, interval `+0x39C`, party `+0x11AC`. Aggregated: per second, total calls, distinct creatures (64-slot id set), the per-creature maximum, and the share with elapsed ≥ 4000. Plus the first `p1_detail_lines` per-creature detail lines per session (stamp day/ms) as evidence that the stamp stays (0,0). **Party filter does not apply** here: P1 is about non-party creatures, and `log_all` is not needed. |
| **E4** FM1 leader bump clears queue | `0x586F43` / 5 | `6A 01 8B 4D 08` (`PUSH 1; MOV ECX,[EBP+8]`; the next insn `CALL 0x541080` ClearAllActions is not stolen) | 0x586B70 | `ebp` | no; `[]` | Reached only when no type-1 or 0x11 action was found (`0x586F3D JNZ` not taken). Bumpee `[ebp+8]`, mover `[ebp-0x158]` (= `this`). Logs bumpee id/tag/party, mover id/tag, combat `+0x520`, attempted target `+0x550`, round active, queue count `[[bumpee+0x100]+8]` (CExoLinkedList internal count; **speculative**, so log it raw) and the last examined action type `[ebp-0x78]` (valid only if the count is > 0). Rare event, so every one is logged. |

E1 evaluation runs once per second per creature, inside the first E1 call after the second boundary:
```cpp
spin = (maxBurst >= a1_burst_threshold || perSec >= a1_rate_threshold);
roundActive = rd<int>(rd<int>(cr,0x10DC), 0xA84);   // StartCombatRound 0x58E873 writes 1, EndCombatRound 0x58E9D3 writes 0
if (spin && !roundActive) -> "E1 SPIN" line (first second) / counter update; on clear -> "E1 END ... after N s"
if (engine_verbose) -> "E1 RATE" line for every party attacker every second
```
- In the normal case an attack returns 1 (in progress), so RunActions stops and the burst is 1.
- In the A1 spin `[0x13,0xC]` the creature re-enters until the 1000 µs budget is spent, giving a burst of tens.
- `perSec` alone is ambiguous, because a normal attacker at level 4 runs once per AIUpdate tick. **maxBurst is the discriminator.**

**Hot-path discipline.**
- Problem: SafeRead's `probeRead` calls `VirtualQuery`, which is costly under Wine, and the region cache is reset in every scope. E1 can fire thousands of times a second in a spin, and E3 roughly enemies × AI ticks per second.
- In E1 and E3, read the fixed fields of the creature pointer directly. The engine dereferences that same pointer in the next instruction (E1: `[EAX+0x570]` store; E3: `ECX` for the `CALL`).
- Use `saferead` only for secondary pointers (stats, combat round, tag string) and only in the 1 Hz evaluation or when a log line is emitted. This follows the 2026-10-01 Buff HUD IsBadReadPtr fault-storm lesson.

### 3c. Hook manifest (excerpt, `companion-ai`, `kotor2-steam-aspyr.hooks.toml`)
```toml
# T1 ShipBuild value: ExecuteCommandShipBuild 0x692C40. [EBP-4] holds the pushed value (0x692C5A MOV EAX,[EBP-4]).
[[hooks]]
address = 0x00692C50
type = "detour"
function = "AiTraceShipBuild"
original_bytes = [0xC7, 0x05, 0x14, 0xE1, 0xA7, 0x00, 0x01, 0x00, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"

# T2 PrintString capture: ExecuteCommandPrintString 0x68F260, argc<2 path. Return 1 => 0x68F39F (skip engine print).
[[hooks]]
address = 0x0068F301
type = "detour"
function = "AiTracePrintString"
original_bytes = [0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00]
consumed_exit_address = 0x0068F39F
exclude_from_restore = ["eax"]
[[hooks.parameters]]
source = "ebp"
type = "int"

# T3 AurPostString capture: ExecuteCommandAurPostString 0x6F6200 (retail stub), after the 4 pops.
[[hooks]]
address = 0x006F62F2
type = "detour"
function = "AiTraceAurPostString"
original_bytes = [0xC7, 0x45, 0xCC, 0x00, 0x00, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"

# E1 A1 attack entry: AIActionPhysicalAttacks 0x6D7390, the +0x570 fallback reset (EAX = attacker).
[[hooks]]
address = 0x006D740F
type = "detour"
function = "AiTraceAttackEntry"
original_bytes = [0xC7, 0x80, 0x70, 0x05, 0x00, 0x00, 0xB8, 0x0B, 0x00, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "eax"
type = "int"

# E1b A1 approach-branch exit (return 2): snapshot only.
[[hooks]]
address = 0x006D8BE2
type = "detour"
function = "AiTraceAttackApproachExit"
original_bytes = [0xA1, 0xB8, 0x44, 0x99, 0x00]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"

# E2 A2 slot-6 fallback fires: CSWSCreature::AIUpdate 0x569320.
[[hooks]]
address = 0x00569A08
type = "detour"
function = "AiTraceFallbackFires"
original_bytes = [0x8B, 0x8D, 0x6C, 0xFE, 0xFF, 0xFF]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"

# E3 P1 block-2 UpdatePerception(1): SpawnInHeartbeatPerception 0x56B8A0.
[[hooks]]
address = 0x0056BDBD
type = "detour"
function = "AiTracePerceptionMode1"
original_bytes = [0x6A, 0x01, 0x8B, 0x8D, 0x2C, 0xFF, 0xFF, 0xFF]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"

# E4 FM1 BumpFriends 0x586B70: leader bump about to ClearAllActions(1) on the bumpee.
[[hooks]]
address = 0x00586F43
type = "detour"
function = "AiTraceBumpClear"
original_bytes = [0x6A, 0x01, 0x8B, 0x4D, 0x08]
skip_original_bytes = false
exclude_from_restore = []
[[hooks.parameters]]
source = "ebp"
type = "int"
```
The `manifest.toml` lists both hashes (6A522E71…, 4AB72FC1…) in the same way as `ranged_ai_src/manifest.toml`. The `exports.def` lists the 8 functions.

## 4. check_windows result

**How it was run.**
- `check_windows.py` was copied to `scratchpad/check_windows_ai_trace.py`; the repo copy is untouched.
- The `WINDOWS` table was replaced with the windows below.
- The 7 ranged-ai 0.2.0 windows were added as extra live hooks: `0x6D83EA+12`, `0x6D7C79+6`, `0x6D850F+6`, `0x77AA5E+7`, `0x77ACBC+9`, `0x77D8D4+6`, `0x74DBDD+7`. They are not in the 09-30g TSV, but `ranged_ai.ini` and `ranged_ai_log.txt` (Oct 1 18:36) are in the game folder.
- The EAX warning was applied to the consumed windows.
- The X windows are future companion-ai fixes, validated so the shared-DLL plan is sound. They are not part of the trace. E4 and X1 are alternatives at the same address, so they cannot both be installed.

```
[OK  ] T1 ShipBuild value: 0x00692c50 len 10  bytes = [0xC7, 0x05, 0x14, 0xE1, 0xA7, 0x00, 0x01, 0x00, 0x00, 0x00]
        00692c50  c70514e1a70001000000   MOV dword ptr [0x00a7e114],0x1
[OK  ] T2 PrintString capture: 0x0068f301 len 7  bytes = [0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00]
        0068f301  c745f000000000         MOV dword ptr [EBP + -0x10],0x0
[OK  ] T3 AurPostString capture: 0x006f62f2 len 7  bytes = [0xC7, 0x45, 0xCC, 0x00, 0x00, 0x00, 0x00]
        006f62f2  c745cc00000000         MOV dword ptr [EBP + -0x34],0x0
[OK  ] T4 SendMessageToPC (optional): 0x00690bc8 len 7  bytes = [0xC7, 0x45, 0xC8, 0x00, 0x00, 0x00, 0x00]
        00690bc8  c745c800000000         MOV dword ptr [EBP + -0x38],0x0
[OK  ] E1 A1 attack entry (+0x570 reset): 0x006d740f len 10  bytes = [0xC7, 0x80, 0x70, 0x05, 0x00, 0x00, 0xB8, 0x0B, 0x00, 0x00]
        006d740f  c78070050000b80b0000   MOV dword ptr [EAX + 0x570],0xbb8
[OK  ] E1b A1 approach-branch exit: 0x006d8be2 len 5  bytes = [0xA1, 0xB8, 0x44, 0x99, 0x00]
        006d8be2  a1b8449900             MOV EAX,[0x009944b8]
[OK  ] E2 A2 slot-6 fallback fires: 0x00569a08 len 6  bytes = [0x8B, 0x8D, 0x6C, 0xFE, 0xFF, 0xFF]
        00569a08  8b8d6cfeffff           MOV ECX,dword ptr [EBP + 0xfffffe6c]
[OK  ] E3 P1 block-2 UpdatePerception(1): 0x0056bdbd len 8  bytes = [0x6A, 0x01, 0x8B, 0x8D, 0x2C, 0xFF, 0xFF, 0xFF]
        0056bdbd  6a01                   PUSH 0x1
        0056bdbf  8b8d2cffffff           MOV ECX,dword ptr [EBP + 0xffffff2c]
[OK  ] E4 FM1 bump ClearAllActions: 0x00586f43 len 5  bytes = [0x6A, 0x01, 0x8B, 0x4D, 0x08]
        00586f43  6a01                   PUSH 0x1
        00586f45  8b4d08                 MOV ECX,dword ptr [EBP + 0x8]
[OK  ] X1 FM1 fix variant (skip-only, incl. CALL): 0x00586f43 len 10 (skip-only)  bytes = [0x6A, 0x01, 0x8B, 0x4D, 0x08, 0xE8, 0x33, 0xA1, 0xFB, 0xFF]
[OK  ] X2 A1 fix (review option 1): 0x006d84fc len 6  bytes = [0x89, 0x8D, 0x7C, 0xFC, 0xFF, 0xFF]
[OK  ] X3 P1 fix (store stamp): 0x0056bde3 len 6  bytes = [0x89, 0x90, 0x9C, 0x03, 0x00, 0x00]

ALL OK
```
The scratchpad copy of `verify_windows_both_exes.py` reports `ALL MATCH`: every window is identical in `swkotor2.exe` (LAA) and `swkotor2.exe.unpatched_vanilla`.

The windows are also clear of perf-lab's VM hooks: `0x668FD0`, `0x6699D0`, `0x669A32`, `0x670370`, `0x6FFA76`, `0x7022A0`, and the AI hooks `0x51D790`, `0x51DC70`, `0x51DC86`.

## 5. INI spec: `<game dir>/ai_trace.ini` (section `[AITrace]`)

The INI path is made absolute with `GetCurrentDirectoryA`, as in ranged-ai. The INI is re-read at most once per 1000 ms (GetTickCount gate) at the start of any handler, and a config line is logged when it changes.

| key | default | meaning |
|---|---|---|
| `enable` | 1 | master switch; 0 = every handler returns immediately (vanilla behaviour, ShipBuild stays TRUE) |
| `scripts` | 1 | T1 flip + T2/T3 capture |
| `engine_points` | 1 | E1-E4 |
| `trace_scripts` | `k_ai_master,k_hen_*dlg,a_atkonend,a_force_combat,k_combat_rnd` | names (case-insensitive, `*` wildcard) whose ShipBuild is flipped and whose prints are logged |
| `shipbuild_scope` | `ai` | `ai` = only `trace_scripts`; `all` = every script (proven print-only, section 8; still filtered by object); `off` = never flip (capture unguarded prints only) |
| `prints_all_scripts` | 0 | 1 = also log PrintString/AurPostString from non-traced scripts (no flip for them) |
| `party_only` | 1 | object filter: party members (`+0x11AC`) pass |
| `tags` | (empty) | comma list of creature tags that pass in addition (e.g. enemy boss tags) |
| `log_all` | 0 | 1 = every OBJECT_SELF / creature passes (enemies run k_ai_master too: noisy) |
| `engine_print` | 1 | 0 = traced PrintStrings skip the engine's own `PRINTSTRING:` path (T2 consumed exit) |
| `log_aur` | 1 | T3 lines |
| `dedupe_ms` | 1000 | identical consecutive line (same object + text) within this window becomes an `R` repeat count |
| `max_lines_per_sec` | 200 | per-second cap; dropped lines counted, then one marker line |
| `max_file_mb` | 64 | file cap; one marker line, then writes stop (counters continue); raising it via hot reload resumes |
| `flush_ms` | 250 | buffered FILE (64 KB `setvbuf`), flushed at most this often and on every marker line or DLL detach |
| `a1_burst_threshold` | 4 | attack entries within one AIUpdate burst that count as a spin |
| `a1_rate_threshold` | 400 | attack entries per second that count as a spin |
| `engine_verbose` | 0 | 1 = per-second E1 rate line for every party attacker; E3 line every second (otherwise E3 every `summary_sec`) |
| `summary_sec` | 10 | `S` counter summary period |
| `p1_detail_lines` | 20 | per-session budget of E3 per-creature detail lines |

The log is truncated per session: `DllMain(PROCESS_ATTACH)` opens `ai_trace.txt` with `"w"` and keeps it open.

## 6. Log line formats (`<game dir>/ai_trace.txt`)

The timestamp `t` is `GetTickCount()` ms, the same clock as `ranged_ai_log.txt`, so the two files merge by `t`. `oid` is 8 hex digits, `tag` falls back to `-`, and `ev` is `[0xA7E10C]`.
```
# ai-trace 0.1.0 session 2026-10-01 21:14:03 pid=1234 cwd=Z:\...\Knights of the Old Republic II ini=...\ai_trace.ini
# config enable=1 scripts=1 engine_points=1 scope=ai trace_scripts=... party_only=1 tags= log_all=0 engine_print=1 caps=200/s,64MB
t=5123456 P 7f0001a2 p_atton k_ai_master ev=1003 d=1 | GENERIC DEBUG *************** START DETERMINE COMBAT ROUND Atton_2130706850
t=5123457 A 7f0001a2 p_atton k_ai_master ev=1012 d=1 | Mandalorian Soldier_2130707011 @(0,15) life=2.0
t=5124460 R 7f0001a2 p_atton k_ai_master | previous line repeated 57x in 1000 ms
t=5130000 E1 SPIN cr=7f0001a2 p_atton party=1 stance=11 entries/s=912 maxBurst=31 round=0 combat=1 f570=2991 tgt=7f000233 exit{stance=11 backoff=0 ctrl=7f0001a0 ranged=0 dist2=31.40 max=2.25 los=1}
t=5148000 E1 END cr=7f0001a2 p_atton spin lasted 18.0 s
t=5131000 E2 FALLBACK cr=7f0001a2 p_atton party=1 combat=1 round=0 script=k_hen_combend01 a1spin=0
t=5140000 E3 P1 calls/s=1180 creatures=19 maxPerCreature=62 elapsed>=4000:100%
t=5140001 E3 P1 detail cr=7f000233 n_mandsold elapsed=5140001 stamp=(0,0) interval=4000 f38c=1 isPC=0 party=0
t=5150000 E4 BUMP bumpee=7f0001a2 p_atton party=1 mover=7f0001a0 p_exile combat=0 tgt=7f000000 round=0 queue=1 lastType=0x3d
t=5150500 S lines=1234 dropped=0 deduped=310 flips=402 prints=388 aur=46 e1=15123 e1spins=1 e2=3 e3=11800 e4=2 cap=0
-- ai_trace: 214 lines dropped by the per-second cap (200/s) at t=5160000 --
-- ai_trace: file cap 64 MB reached at t=...; further lines dropped (raise max_file_mb to resume) --
```
`d` is the VM depth.
- **k_ai_master** usually runs at d=1, under the k_hen_* wrapper.
- **The `k_hen_*dlg` scripts** run at d=0, because they are the OnDialogue slot.

## 7. Expected volume and cost

**Volume.**
- The vanilla AI includes have 170 live (uncommented) debug prints: k_ai_master main 13, k_inc_generic 132, k_inc_gensupport 25. The figure of about 230 counts commented-out ones too.
- They all JSR into one GN_MyPrintString or Db_MyPrintString body, which is why the compiled k_ai_master has only 2 PrintString ACTIONs. So the volume is per call.
- One combat round's `GN_DetermineCombatRound` passes through roughly 15-40 prints. The heaviest functions are DefaultAIRoutine 13, TalentMasterRoutine 13, GetAttackTalentCode 12, DetermineAttackTarget 11 (which loops over enemies) and DetermineCombatRound 6.
- In combat, each companion runs a DCR on every end-of-round (3 s, or ≤ 3 s through A2) and on OnAttacked/OnDamaged: about 5-25 lines/s per companion, so **10-50 lines/s for a two-companion party**. With lines around 110 B that is **1-5.5 KB/s**, or 4-20 MB per hour of continuous combat.
- Out of combat: heartbeat events give about 0-1 line/s per companion.
- AurPostString: the k_ai_master path-blocked event posts 2 lines per OnBlocked, and FM5 says OnBlocked can fire once per frame. That is up to 2×FPS lines/s per blocked follower before dedupe, and 1 `R` line/s after it. That burst is itself the FM5 signal.
- `log_all=1` with about 10 enemies multiplies the volume by roughly 5-6, so it reaches the 200/s cap.
- Engine points emit at most a few lines per second, plus spin/fallback/bump events.

**Cost.**
- **T1:** one context read plus 17 short `stricmp`, about 1 µs per ShipBuild call.
- **T2/T3:** an `fwrite` into a 64 KB buffer, about 1-2 µs. The engine's own PrintString path adds a `vsnprintf` per traced print (removable with `engine_print=0`).
- **Script side:** each flipped print adds about 6 NCS instructions (NOTI/JZ, CPTOPSP, ACTION). The instruction cap is 0x20000, so this is irrelevant. The strings are already built by the caller in retail today (the concatenation happens before the JSR), so the flip adds no string work.
- **E1:** one direct read plus a slot lookup plus `QueryPerformanceCounter`, about 0.2-0.5 µs per entry. A normal attacker enters once per AI tick. In a spin, entries are bounded by the engine's own 1 ms budget, and each engine pass (LOS raycast plus two AddActionToFront) costs 20-50 µs, so the hook adds about 1-2 % to an already-broken path.
- **E3:** about 0.2 µs per call. At the P1 bug rate of roughly enemies × ticks (for example 20×60 = 1200/s), that is about 0.25 ms of CPU per second.
- **E2/E4:** rare.
- **Total:** under 0.5 ms of CPU per second in heavy combat. The per-line `fopen`/`fclose` used by ranged-ai is not used here, because at 50 lines/s it would cost milliseconds under Wine.

## 8. ShipBuild grep: other callers and why scoping is safe

**Source corpus (`mod8_kite_script/vanilla_all`, the 638 sources shipped in Scripts.bif).** ShipBuild appears only in include helpers, and every use is `if(!ShipBuild()) { print }`:

| file:line | function | guarded body |
|---|---|---|
| `k_inc_debug.nss:41` | `Db_MyPrintString` | `PrintString(sString)` |
| `k_inc_debug.nss:100` | `Db_PostString` | `AurPostString(sString,x,y,fShow)` |
| `k_inc_gensupport.nss:3900` | `GN_MyPrintString` | `PrintString(sString)` |
| `k_inc_gensupport.nss:3913` | `GN_MyPrintShoutString` | `//PrintString(sString);` (empty) |
| `k_inc_walkways.nss:710` | `WK_MyPrintString` | `PrintString(sString)` |
| `k_inc_force.nss:6288` | `SP_MyPrintString` | `sString = "SPELL GENERIC DEBUG STRING: " + sString; PrintString(sString);` |

There are no cheats, debug dialogue or debug items. `GN_PostString` and `GN_PostShoutString` are fully commented out. `SP_MyPostString` calls AurPostString unguarded.

**Compiled corpus.** Most compiled scripts ship without source, so the bytecode was checked too.
- Scanner: `scratchpad/scan_ncs.py`, with output in `scratchpad/ncs_scan.json`.
- Corpus: every NCS reachable through pykotor `Installation` (chitin BIFs, every module RIM/MOD/ERF, Override; the game folder was opened read-only): **34,927 instances, 4,883 distinct names**.
- Method: every `ACTION 761` (0x2F9) was decoded together with the instructions that follow it.

Results:

| result | count |
|---|---|
| ShipBuild calls | 518, in 91 names (498 instances incl. duplicates across modules); every call is argc 0 |
| guard form | 518 / 518 are `ACTION 761; NOTI; JZ` = `if(!ShipBuild())`. None is stored to a variable, none is `if(ShipBuild())` |
| guarded block contents | `PrintString` only: 495; `AurPostString` only: 5; nothing: 18 |
| JSR inside any guarded block | 0, so no user-function calls are hidden behind a guard |
| ACTION argc (whole install) | PrintString (1, 512 calls), AurPostString (4, 10,069 calls in 3,392 NCS), SendMessageToPC (2, 37 calls in 13 NCS) |
| AI set | `k_ai_master` (chitin and the modded Override copy): ShipBuild 3, PrintString 2 (both guarded), AurPostString 9 (none guarded). `a_atkonend`, `a_force_combat`, `k_combat_rnd`: 1 / 1 / 7. Each of the 13 `k_hen_*dlg`: 2 / 1 / 7. SendMessageToPC: 0 |

**Conclusion.**
- With ShipBuild returning FALSE, the only extra work any loadable script does is call PrintString or AurPostString.
  - PrintString formats a line into the engine debug sink.
  - AurPostString pops its arguments and returns 0 (retail stub).
- The handler's only other side effect is the global `[0xA7E114]=1`, which is written on every call in retail anyway and has no readers.
- So even `shipbuild_scope=all` changes no game behaviour in this install.
- Scoping to `trace_scripts` plus the object filter is still the default, for three reasons:
  1. It bounds volume and cost: every ambient, walkway and spell script would otherwise print.
  2. It keeps the engine debug sink quiet if one is open.
  3. It protects against future script mods that might gate real behaviour on ShipBuild. **Re-run the scan after installing script mods before using `all`.**

## 9. Where it lives: recommend a new patch id `companion-ai`

| | add to `ranged-ai` | new `companion-ai` (recommended) |
|---|---|---|
| scope | ranged-ai is the stance-16 "Ranged (Kiting)" feature and is under user play-test now | general party-AI diagnostics now, and the engine AI fixes from the 10-01 review next (A1 `0x6D84FC`, P1 `0x56BDE3`, FM1 `0x586F43`, C1 byte patches, FM4 one byte), none of them stance-16 specific |
| shared windows | none | **FM1 log (E4) and the FM1 fix use the same address `0x586F43`**. KPM allows one hook per address, so the log point and the fix must be one handler in one DLL. The P1 fix (`0x56BDE3`) and the A1 fix (`0x6D84FC`) also want E3/E1's counters (fix-on/off A/B in one log) |
| rollback | a trace bug would force a rollback of the feature under test | independent install/uninstall; `enable=0` is vanilla |
| conflicts | n/a | none: no window overlap with ranged-ai (the shared function `0x6D7390` uses distinct sites) |

The design takes the reference DLL's style: absolute INI path, GetTickCount reload gate, SafeRead, the cdecl handler table, both exe hashes. It leaves out the per-line `fopen`.

**Note for the FM1 fix.** The review's "consumed hook at `0x586F43`, exit `0x586F4D`" is unsafe with this 5-byte window: the stolen `PUSH 1` runs before the consumed jump, and the skipped `CALL ClearAllActions` (`RET 4`) would have popped it. That leaves 4 bytes on the stack until the epilogue.
- Use **X1** instead: the 10-byte skip-only window `0x586F43` (`PUSH 1; MOV ECX,[EBP+8]; CALL 0x541080`, validated above).
- The handler then calls `ClearAllActions` itself when it does not apply the fix: `0x541080`, `__thiscall(bumpee, 1)`.
- Execution resumes at `0x586F4D`.
- That handler absorbs E4's logging.

## 10. Risks and open questions

1. **A failing handler kills the script.** Every T hook sits after the pops succeeded and changes only a local that is pushed later (T1) or nothing (T2/T3). T2's consumed exit goes to the vanilla success tail with the same frame and SEH state. A crash inside our handler (bad read) would still crash the game. Mitigations: SafeRead on every VM pointer, a depth range check, and `char*` copies bounded to 32/512 bytes.
2. **Reentrancy.** Commands, AI updates and our hooks all run on the game's main thread. The file writer is single-threaded and needs no locks. The only DLL call made from inside a handler is `0x51C0A0` (a pure lookup); E-hooks call nothing.
3. **Engine debug sink.** `PrintString` writes `PRINTSTRING: …` to `[[0xA1B488]+8]` when its `[+8]` sink is non-NULL. It is unknown whether Aspyr retail has one open (no log file is known). If it does, the flip also adds lines there. Use `engine_print=0` to suppress them. Check at first run: look for a new or growing engine log next to the exe.
4. **ExecuteScript event id is one global.** A nested ExecuteScript inside k_ai_master overwrites `[0xA7E10C]`, so `ev` can be stale after one. A direct OnDialogue run (the k_hen_*dlg scripts) shows whatever the last ExecuteScript set. It is a best-effort label only.
5. **Volume estimate is static.** Measure it from the `S` lines in the first session. If `dropped` grows, raise `max_lines_per_sec` or lower `dedupe_ms`.
6. **Speculative offsets, to confirm from the first log.**
   - The bumpee queue count `[[bumpee+0x100]+8]` (CExoLinkedList internal +8, also speculative in U1/H2).
   - Combat-round `+0xA84` active is written by Start/EndCombatRound (asm `0x58E873` = 1, `0x58E9D3` = 0); its reader semantics are from the review.
   - `[ebp-0x78]` in E4 is valid only when the queue was non-empty.
7. **A1 burst window.** 2 ms is greater than RunActions' 1000 µs cap and less than one frame at 240 FPS (4.2 ms). At a frame rate above 500 FPS, two normal entries could fall into one burst; the threshold of 4 keeps that harmless.
8. **The Override `k_ai_master` is a modded build** (Mines Overhaul and/or Mod 8 kite dispatch, 125,197 B vs 121,637 B). The scan shows the same ShipBuild/print structure. Mod 8's kite routine is compiled into k_ai_master itself, so no extra name is needed.
9. **Name/tag cache staleness.** Ids are re-resolved through `0x51C0A0` after 1 s, never by caching a raw pointer across calls.
10. **A future AurPostString on-screen mod for K2** would compete for `0x6F62F2` only if it chose that site. The KotOR 1 one does not apply to K2.
11. **DB corrections to persist** (read-only here). Both 0x51C0A0 facts are confirmed in the asm.
    - **0x51C0A0.** `functions` describes 0x51C0A0 as "cdecl(id)"; it is really `__thiscall(ECX=[[0xA1B4A4]+8], id) RET 4`. Add `name_map` row: `0x51C0A0 CServerExoApp::GetGameObject(id)`.
    - **Handler names.** Upgrade the confidence of the four handler names to high (verified by slot asm).
    - **New VM facts.** Add these as lessons or rows:
      - AurPostString is a retail stub filled by `0x6F5B80`.
      - ShipBuild's value is in `[EBP-4]`.
      - `[0xA7E114]` is write-only.
      - `[0xA7E10C]` = RunScriptVar.
      - name and OBJECT_SELF per depth are at `VMI+0x44+d*0x28` and `VMI+0x18C+d*4`.
      - the NCS ShipBuild scan result.

## 11. Files (scratchpad)

- `design_ai_trace.md`: this file.
- `check_windows_ai_trace.py` and `check_windows_ai_trace_output.txt`: the validator copy and its output.
- `verify_windows_both_exes_ai_trace.py`: the both-exe byte check (ALL MATCH).
- `scan_ncs.py`, `argc_scan.py`, `ncs_scan.json`: the read-only NCS scan of every loadable script (re-runnable with the repo `.venv`).
