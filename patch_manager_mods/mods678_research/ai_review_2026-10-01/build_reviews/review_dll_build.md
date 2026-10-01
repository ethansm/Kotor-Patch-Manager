# Pre-install review: companion-ai 0.1.0 + ranged-ai 0.3.0 + check_windows.py (commit 2e596e9)

Independent, read-only. Sources: `patch_manager_mods/companion_ai_src/`, `ranged_ai_src/` (diff 3dc974e..2e596e9), `mods678_research/check_windows.py` (diff f4d2aa1..2e596e9), the full Ghidra dump `.asm`, `KotorPatcher/src/core/wrapper_x86.cpp` + `patcher.cpp` + `config_reader.cpp`, the live `patch_config.toml` (25 ids, 131 hooks), both game executables, and the built kpatches in `kotor-patch-manager/Patches/{CompanionAI,RangedAI}/`.

Nothing was edited, built, installed or launched. The host test was not compiled or run.

## Verdict

**No BLOCKERs found.** I would install both patches as built. SF1 is a two-line parser fix and is worth doing first, but it does not block: the shipped INI uses digits everywhere.

Framework facts re-confirmed from `wrapper_x86.cpp`:
- The handler runs first, then registers/flags are restored, then the stolen bytes run, then the consumed test.
- `preserve_registers`/`preserve_flags` default to true (`patcher.h:64-65`).
- There is no auto-exclusion of EAX for consumed hooks, so the toml must list it. All 5 consumed hooks in scope do.
- The stolen bytes are copied verbatim, with no rel32 relocation. Only skip-only windows may contain a rel CALL. FM1 is the only such window, and it is skip-only.

## Findings

| id | sev | where | summary |
|---|---|---|---|
| SF1 | SHOULD-FIX | `CompanionAiLogic.h:157-158` | `a1=off` / `fm4=false` / `c1=no` silently become the built-in default, which is ON for a1, a1_retarget, fm1, fm4, c1, a4 and trace. A word-valued "off" turns a fix on. |
| SF2 | SHOULD-FIX | `check_windows.py:116-155` `eax_state` | The EAX liveness walk is not sound. It ignores implicit EAX readers, treats an indirect JMP as fall-through, and misclassifies `XOR EAX,EAX` as a read. It can say "dead" when EAX is live. |
| SF3 | SHOULD-FIX | `companion_ai_logic_test.cpp` | The host test covers the parser, gate and throttles, but none of the behaviour-changing handler decisions (A1 gate, A4 return mapping, C1 value, P1 p1=0 reset, FM4 pairing, hot-reload semantics). |
| N1 | NIT | `CompanionAI.cpp:519, 670, 723/734` | E2, FM1 and A4 lines go through `eline`: forced `fflush`, past the per-second cap, with no per-subject throttle. With `log_all=1`, E2 fires about every 3 s per idle combatant. |
| N2 | NIT | hooks.toml FM3 `0x51DBE7`, P1i/P1s | Hooks whose flag is 0 still pay the full wrapper (PUSHAD + FXSAVE/FXRSTOR) every scheduler-loop object or AI tick. FM3 is log-only, so consider leaving it out of the shipped toml. |
| N3 | NIT | `CompanionAI.cpp:721-728` | With `a4=0`, a NULL spell row faults inside CompanionAI.dll (vanilla-parity by design), so crash logs will name the DLL. Note it in CRASH_INVESTIGATION. |
| N4 | NIT | `CompanionAI.cpp:531` / `629` | With p1=1, E3 reads `[ebp-0x28]` after P1i overwrote it (0xFA0/0), so the "elapsed>=4000" share is meaningless. P1s also stamps creatures on the `+0x38C==0` and IsPC paths (harmless). |
| N5 | NIT | `CompanionAI.cpp:578` | The A1 retarget range is `[ebp-0x2C]` = `0x571570(target, flag, 0)`. The vanilla PC tail uses `0x571570(selfId, 0, 1)`. A design choice; document it. |
| N6 | NIT | `CompanionAI.cpp:314-321` | The 16-slot `g_tr` is shared by E1, E1b and A1. With `tags=`/`log_all=1`, big fights evict party tracks, which resets the A1 throttle and the would-spin counts. |
| N7 | NIT | `RangedAI.cpp:300-305` | The kite_approach in-range test ignores area. A garbage max (<=1) counts as out of range, so the creature approaches with move range <=1. These are edge cases. |
| N8 | NIT | `CompanionAI.cpp` globals | `g_cfg`, `g_tr`, `g_obj` and `g_snap` are unsynchronized (only the log is locked). Correct only if every hooked function runs on the main thread. Confirm SaveCreature does: add `GetCurrentThreadId()` to the FM4 line. |
| N9 | NIT | `check_windows.py` LIVE TSV | The `live_hooks_2026-09-30g.tsv` snapshot lacks ranged-ai, character-effects-panel (3 hooks) and inventory-sort-filter `0x8A8478`. Harmless now that the live toml is also read, but the fallback when the toml is absent is stale and silent. |
| N10 | NIT | `check_windows.py` `toml_crosscheck` | It does not check parameter sources, does not check that non-consumed hooks do not exclude eax, and does not check stack depth at the consumed exit. |
| N11 | NIT | `companion_ai.ini` | The shipped defaults turn on a1, a1_retarget, fm1, fm4 and c1 at first boot. Design §2/§10 recommended a log-only Stage 0, and README A1 is "not yet checked at runtime". This is your call: hot reload makes rollback a one-line edit. |
| N12 | NIT | A1 scope | A1 also ends scripted `ActionAttack` on doors/placeables for a non-controlled Ranged/Stationary party member that is out of range (vanilla spins there forever). This is a behaviour change at quest scripts, though it is arguably a fix. |
| N13 | NIT | kpatch provenance | The kpatch DLLs (CompanionAI md5 `733ab1b1f5e173f857266183b8069f02`, RangedAI `a20170c6823d721b212e146b9c54ea21`) postdate the sources by 1 min. Their exports and version strings match, and they import only KERNEL32 + msvcrt. A byte-level rebuild check was not done. |

## Details

### SF1: word-valued booleans fall back to "on"
```cpp
long getInt(const char* k, long def) const { return parseInt(get(k), def); }      // parseInt returns def when no digits
bool getBool(const char* k, bool def) const { return getInt(k, def ? 1 : 0) != 0; }
```
- `a1=off`, `a1=false` and `fm1=no` all parse as "no digits", which returns the default.
- The defaults are ON for a1/a1_retarget/fm1/c1/a4/trace (fm4 is int, default 1).
- This defeats "all flags 0 = vanilla" for anyone who types words. `shipbuild_scope=none` likewise becomes `ai`.

**Fix:**
- In `getBool`, accept `on/off/true/false/yes/no` (case-insensitive).
- For any other non-numeric value, keep the previous config value, or use 0 for fix flags, and emit `# config: bad value key=...`.
- Add these cases to `testIni`.

### SF2: `check_windows.py` EAX liveness walk
`eax_state` declares EAX dead "only if on EVERY path EAX is fully written". Three gaps, all found by reading the code:
1. **Implicit readers are invisible.** The read test only looks at explicit operands, so these pass as "transparent":
   - `CDQ`, `CWDE`, `CBW`;
   - one-operand `MUL`/`IMUL`/`DIV`/`IDIV`;
   - `STOS`/`SCAS`, `XCHG`, `CMPXCHG`, `SAHF`, `PUSHAD`.

   A `CALL` is always treated as a clobber, but MSVC helpers such as `__chkstk`, `_alloca_probe` and `__SEH_prolog` take EAX as input. Result: false "dead".
2. **Indirect jumps are mis-walked.** `JMP dword ptr [..]` (switch tables) and `JMP reg` do not match `(J\w+)\s+0x`. The walk falls through to the next linear instruction and never visits the real successors.
3. **`XOR EAX,EAX` is reported as a read.** `rd_ = re.search(EAX, src)` fires before the XOR special case, contradicting the docstring. This direction is only conservative.

**Not blocking this build.** I re-walked every consumed exit by hand from the asm:

| hook | resume | consumed exit |
|---|---|---|
| T2 | `0x68F308`: `CMP; JNZ` → `LEA ECX; CALL` (dead) | `0x68F39F`: `MOV [ebp-0x24],0; ...; CALL` (dead) |
| A1 | `0x6D8548`: `CMP; JZ` → `MOV EAX,[EDX]` or `CALL` (dead) | `0x6D7D2F`: `CMP [ebp-0xE8]; JZ` → `PUSH 1; CALL` or `MOV EAX,[0xA1B4A4]` (dead) |
| A4 | `0x6D9E39`: `CMP; JZ; MOV ECX; CALL` (dead) | `0x6D9EA4`: `PUSH 1; PUSH 1; MOV EDX; MOV EAX,[EDX+0x48]` (dead) |

**Fix:**
- Treat any mnemonic outside a small known set as "unknown". That fails the check rather than passing it.
- Model the implicit-operand opcodes above as reads.
- Treat indirect JMP as "unknown".
- Treat `XOR/SUB EAX,EAX` as writes.
- Optionally add an ESP-depth check: count PUSH/POP/`RET n` between the hook site and the exit within the function.

### SF3: host test coverage
What matters most is the behaviour-changing decisions. They live in `CompanionAI.cpp`, not in the header:
- the A1 gate: eff 11/12, backoff/force-jump, controlled id, raw-stance mask, party, retarget mask, `mx > 0.5`, rethink write;
- the A4 return mapping: row NULL/unreadable → 1 with the local = 0; v==0 → 1; v!=0 → 0;
- the C1 value: party/cap → -1 or 0;
- the P1 `p1=0` stamp reset;
- FM4 snap/restore pairing across `fm4`/`log` hot flips;
- `loadIni` semantics: missing section keeps the config; missing file keeps the last config.

**Fix:**
- Lift these into pure functions in `CompanionAiLogic.h`, for example `a1Decide(...)`, `a4Decide(row, readOk, v)`, `c1Value(...)`, `p1ParityReset(...)`, and test them.
- Add a test that the toml function names equal the exports in `exports.def`. I checked this by objdump: all 17 present.

### N1-N13
See the table. Concrete fixes:
- **N1:** give FM1 and E2 a `LineThrottle` per creature (as A1/A3 have) and write them with `tline`. Keep `eline` for fix decisions.
- **N2:** drop FM3 from the shipped toml until needed.
- **N8:** log the thread id once per FM4 and T1 line for one session.

## Verified OK, per hook

All 19 companion-ai and 7 ranged-ai `original_bytes` were checked three ways:
- They equal the `.asm`, on exact instruction boundaries (own script over the dump).
- They are identical in `swkotor2.exe` (LAA 4AB72FC1) and `swkotor2.exe.unpatched_vanilla` (6A522E71). The two exes differ in `.text` only at `0x70E130` (3 bytes), outside every function hooked or called here.
- No branch targets the interior of any window. No RET is in a window.
- Overlap: none with the 131 live hooks (live `patch_config.toml` read directly, not only the TSV), `k2-activation-fix 0x40D69E`, the `k2-equip-icon-fix` CALL sites, character-effects-panel, or perf-lab `0x51D790/0x51DC70/0x51DC86/0x55AB30/0x55AC26/0x544210/0x544640`.
  - Nearest neighbours: FM3 is 0x89 bytes before perf-lab `0x51DC70`; A1 is 0x29 bytes after E-H3.

Frame slots in `AIActionPhysicalAttacks 0x6D7390` were checked by a CFG walk: no path reaches E-H3 `0x6D850F`, A1 `0x6D853E` or E1b `0x6D8BE2` without passing the writers of:
- `[ebp-0x2C]` max (`0x6D79CA`);
- `[ebp-0xBC]` dist² (`0x6D7E5F`, a dot product);
- `[ebp-0x164]` ranged (`0x6D836B`);
- `[ebp-0xD0]` LOS (`0x6D79E6`);
- `[ebp-0x38]` area;
- `[ebp-0x168]` backoff (`0x6D8371`);
- `[ebp-0x174]` stance;
- `[ebp-0x178]` p (for A1 and E1b).

Engine calls: conventions checked against the RET n in the asm. The compiled call sites were checked in the DLL disassembly: ECX = this, the args at `(%esp)`, and `sub esp` after each callee-pop.

| call | convention |
|---|---|
| `0x57C990` | thiscall(cr; float, int, int), RET 0xC. The float goes through `fstps (%esp)`, matching vanilla `0x6D7D1A PUSH ECX; FSTP [ESP]`. Order (range, exclude=`[ebp-0x14]`, INVALID) matches `0x6D7CFB..0x6D7D24`. |
| `0x541080` | thiscall(obj; 1), RET 4 |
| `0x51C0A0` | thiscall(server; id), RET 4 |
| `0x77D800` | thiscall, RET; a pure getter |
| `0x73FB90` | RET |
| `0x7E5DA0` | RET 4; bounds-checked |
| `0x57D370` | thiscall(cr; INVALID, 0), RET 8; same order as `0x6DAB36` |
| `0x5453C0` | RET |
| `0x56E1E0` | RET |

Per hook:

- **T1 `0x692C50`:** `[ebp-4]` is the pushed value (`0x692C5A MOV EAX,[EBP-4]`). The stolen store hits a write-only global. The return code is unchanged, so it is never negative. VM offsets match `0x6FFFD4..0x6FFFF6` (`[vmi+0x28]` depth, `[vmi+depth*4+0x18C]` self).
- **T2 `0x68F301`:** reached only on the argc<2 path (`0x68F2CC JL`). The string is at `[ebp-0x18]`/`[ebp-0x14]`. Return 1 goes to `0x68F39F`: same function, same ESP, same SEH state (`[ebp-4]`=0 there, set to -1 at the exit), string destructed, VM return 0. EAX is dead at both ends. It returns 0 unless `engine_print=0`.
- **T3 `0x6F62F2`:** pops at `[ebp-0x20]` (string), `[ebp-0x10]`/`[ebp-0x14]` (ints) and `[ebp-0x18]` (float). The function is a retail stub, so logging only.
- **E1 `0x6D740F`:** EAX = attacker (`0x6D7409`). It reads `+0x570` before the stolen reset. Early-outs before any work when the trace is off or the creature is non-party with no tags.
- **E1b `0x6D8BE2`:** all six entries are inside the approach branch, after every slot it reads is written. The stolen `MOV EAX,[0x9944B8]` runs after it.
- **E2 `0x569A08`:** `[ebp-0x194]` = creature. The slot-6 CExoString is at `+0x270+6*8 = +0x2A0` (`0x569A2A..0x569A38`).
- **E3 `0x56BDBD`:** `[ebp-0xD4]` = creature (this for `0x5D0F30`). The stolen PUSH runs on the restored stack.
- **A1 `0x6D853E`:** the slots match the asm.
  - **a1=0:** a pure getter and reads only; the stolen store runs, then resume at `0x6D8548`.
  - **a1=1:** writes `[ebp-0xE8]` (new id or INVALID), then goes to `0x6D7D2F`. Same function and ESP; the x87 stack is empty. The tail needs only EBP/`[ebp+8]`/`[ebp-0x49C]`.
  - **No retarget ping-pong:** `0x6DAC70` sets `+0x52C=1`, so the next approach pass ends at `0x6D83F6` before A1. `+0x52C` is reset per new attack by `0x5689DF`.
  - **Gates:** a NULL controlled creature returns vanilla (parity with the vanilla crash).
  - **Stance gate:** eff 11/12 (after E-H3), with the raw stance mask.
  - **Party gate:** applied as specified.
- **P1i `0x56BDB4`:** reached only for non-party, non-IsPC creatures in mode 1 with `+0x38C!=0`. The stolen CMP re-sets the flags for the JC.
  - **p1=0 parity:** the stamp pair `+0x3A0/+0x3A4` is written only by the creature ctor `0x561F30` (zeroes) among CSWSCreature code. The other writers (`0x5F4C00`, `0x5F2FE0`) sit in vtable `0x996AA4`, not CSWSCreature `0x994F5C`. The reset branch is therefore dead in vanilla. Not serialized by SaveCreature.
- **P1s `0x56BDE3`:** returns at once with p1=0. The pairing matches the push order at `0x56BCF9`/`0x56BD06`.
- **FM1 `0x586F43` (skip-only, 10 bytes):** the handler makes the identical `ClearAllActions(bumpee=[ebp+8], 1)` call, and nothing after `0x586F4D` depends on the skipped call's registers.
  - **Queue walk:** matches `0x521060`/`0x7A1720`/`0x73A050`: `[q+0x100]` is the list, `[list]` the head node, node `+4` next, `+8` data, and the word `[data]` the type.
  - **Keep-set types:** confirmed in dispatcher `0x5D56B0`: 0x3D → `0x5BF7F0` FollowLeader, 0x40 → `0x5BCDC0`, 0x46 → `0x5BF940`.
  - **Push-aside order:** `0x5644B0` → `0x540CA0` AddActionToFront, so the push-aside goes in front.
- **FM4 snap `0x57451F` / restore `0x575183`:**
  - **Snap:** EAX = creature (`0x574519`).
  - **Restore:** `[ebp-0x1D4]` = creature. It runs after `SetPartyMemberFlag(flag,1)` re-created FollowInfo and `0x5751B0` wrote it, so the save bytes stay vanilla.
  - **Copy safety:** FollowInfo is `new(0x3C)` with a POD ctor and is freed by plain `operator delete`, so the memcpy is safe.
  - **Failure paths:** SaveCreature has a single RET. A missed restore is cleared by the next snap. The pair is keyed by creature, and hot flips are handled.
- **A4 `0x6D9E2A` (skip-only, 15 bytes, consumed to `0x6D9EA4`):** the handler writes `[ebp-0x15C]` itself. EAX and EDX are dead at both ends.
  - **a4=1:** NULL or unreadable row → local 0 → `0x6D9EA4`, the vanilla JZ target. v==0 → `0x6D9EA4`; v!=0 → resume CMP/JZ (falls through).
  - **Non-null rows:** identical to vanilla for both flag values.
- **C1 `0x5868B9`/`0x5868F1`/`0x586929` (skip-only):** the handler writes `[ebp-0x10]` = 0 (vanilla) or -1. The count `[ebp-0x14]` is stale in vanilla too, and `0x53F4A0` past the end returns 0. Bounded, with a 256 cap.
- **A3 `0x6D78B7` (off):** `[ebp-0x30]` is valid on all three entry paths. `controlledId` mirrors the `0x6D8515..0x6D856B` chain with probes.
- **FM3 `0x51DBE7` (off):** `[ebp-0x28]` obj, `[ebp-0x3C]` sentinel (`0x51DBE4`), `[ebp-0x34]` level, `[ebp-0x38]` first flag. With the flag off it returns before `tick()`.
- **ranged-ai E-H3 `0x6D850F` (0.3.0):** hooks.toml unchanged.
  - **Range test:** `kite_approach` reads `[ebp-0x164]`/`[ebp-0x2C]`/`[ebp-0xBC]`, all written on every path. The in-range formula equals vanilla `0x6D8297..0x6D82CB`: `(max + 0.1)^2` with `[0x987DF0]`=0.1, and `dist2 <= lim`.
  - **kite_approach=0:** byte-for-byte the 0.2.0 mapping; only a throttled log line is added.
  - **Interplay with A1:** out of range or melee keeps eff=16, so A1 returns 0 and the vanilla Aggressive approach runs. In range with a ranged weapon gives eff=11; A1 then ends or retargets only when the approach branch is still reached (LOS or a non-creature target).

**Hot reload 1→0:** every fix re-reads `g_cfg` per call (INI checked at most once a second), and each `flag=0` path is the vanilla write/call. No state outlives a flag flip except the P1 stamps, which P1i zeroes, and the FM4 snapshot, which is restored only if `fm4==1` at restore time.

**Crash safety:**
- Secondary pointers go through VirtualQuery-based SafeRead.
- Direct reads are only of `this`/locals the engine itself dereferences at that instruction.
- No C++ exceptions are used.
- No handler returns a negative VM result.

**Perf:**
- Off-path cost is GetTickCount + `tick()` (two compares).
- E1, E3 and P1i do no file I/O on the hot path. The trace log is 64 KB buffered and flushed every 250 ms. Fix events are flushed immediately: at most a few per second, except as in N1.
- The INI is stat'ed at most once a second.

**`check_windows.py` changes:**
- What works:
  - The new live-toml overlap.
  - The pairwise overlap.
  - The TOML cross-check (address, length, bytes, skip, exit, eax exclusion).
  - The consumed-hook EAX-in-stolen-bytes test.
- No regressions: the old windows all pass, and the old M8-only "stolen bytes touch EAX" warning was correctly narrowed to consumed hooks, since non-consumed hooks restore EAX.
- Limits are in SF2, N9 and N10.
