# V-C2 plan: native logging for the E1 silent result (doc 41 section 6)

Status: PLAN ONLY (2026-10-14). Nothing built, nothing installed. Parent: `patch_manager_mods/41_custom_voice_and_events.md` phase V-C. Runbook it extends: `patch_manager_mods/kvoice/V-C_RUNBOOK.md`.

## 0. Why, and what is already known
- E1 (`runscript kpv_e1`, loose `kpve1a.wav` in Override) played nothing. The console accepted the command without an error. No mod log can say whether the script ran or whether the engine asked for the wav, because the only native probe (`voice-probe`, hook on `ExecuteCommandPlaySound`) is NOT installed.
- Zero-install channels, running or available now:
  1. Host `inotify` watcher on the live `Override` dir (scratchpad `ovr_watch.py`, started 2026-10-14): logs each OPEN/ACCESS/CLOSE of a `kpv*` file. It shows whether the engine read `kpv_e1.ncs` and `kpve1a.wav`. Limit: sees file reads only, not the call, and a cached resource is not re-read.
  2. `strace -f -e trace=openat -p <pid>`: blocked by `ptrace_scope=1`; needs the user's OK and a sysctl change. Not planned.
- The native plan below is for when the watcher is not enough. Do the cheap steps first: read the watcher after the user re-runs `runscript kpv_c1` and `runscript kpv_e1`. If it already separates "script never ran" from "wav never requested", this plan may shrink to hook H2 only.

## 1. Constraints found in recon (these shape the design)
1. **Two of the five hooks I first listed are ALREADY DETOURED by live patches.** KPM cannot put a second detour on the same entry bytes.
   - `CExoResMan::Demand` 0x00711C20: `perf-lab` (`perf_lab_src/perflab/kotor2-steam-aspyr.hooks.toml:81`, also 0x00711D04). Takes a `CRes*`, not a name, so it is the wrong hook anyway.
   - `CVirtualMachine::RunScript` 0x006FD8D0: `companion-ai` (`companion_ai_src/kotor2-steam-aspyr.hooks.toml:346`, probe `PR RS`, filtered by return-address site, logs no script name).
   - Rule: do NOT add a third detour at either address. Use name-level lookups below, or a mid-function address.
2. **Name-level resource lookups are not hooked by anyone** (`rg` over every `*.hooks.toml` finds none of 0x712360, 0x711ED0, 0x711F30, 0x7125B0, 0x712470, 0x5D9BF0). They see the resref and the resource type, so one hook family covers BOTH questions: was `kpv_e1` (NCS) requested, and was `kpve1a` (WAV) requested.
3. `PlaySound` 0x68EA70 (`ExecuteCommandPlaySound`, thiscall `55 8B EC 6A FF`) is unhooked and already built in `voice_probe_src`. Its resref is never read from the stack (unsafe). The name-level hook supplies the resref instead.
4. Any new hook means a FULL KPM reinstall (new patch id; hot-swap cannot add hooks). The tool is `scripts/kpm_reinstall.py` (doc 24 H10: snapshot, flat-stage run, peer DLL restore, rollback). A sibling session is gating `npc_world`, `npc_world_probe`, `k1_spawn_probe` for an install: BUNDLE this probe into the same reinstall instead of doing two. Ask that session before preparing.

## 2. Hook set (priority order)
| ID | Function | Steam addr | Status | Purpose | Cost |
|---|---|---|---|---|---|
| H1 | `CExoResMan::GetKeyEntry` | 0x00712360 | verified name (B0); signature/prologue TO VERIFY (R1) | every name+type lookup: `kpv_e1` type NCS, `kpve1a` type WAV, `kpve2c` type MP3 if used | HOT: filter first |
| H1b | `CExoResMan::Exists` / `Exists_2` | 0x00711ED0 / 0x00711F30 | verified names; prologues TO VERIFY | same, for existence checks (a sound system may call Exists before Demand) | warm |
| H2 | `ExecuteCommandPlaySound` | 0x0068EA70 | prologue VERIFIED, already built | the `PlaySound` call happened; tick + order | cold |
| H3 | RunScript mid-function or `LoadScript` path | TBD (R2) | UNKNOWN | script name (`CExoString*` arg) without touching the occupied entry | warm |
| H4 | `AIActionPlaySound` | 0x005D9BF0 | name_map conf MEDIUM only; do not hook until R3 confirms | the queued action fires; message 9/7 to objects within 1000 units | cold |
| H5 | `ExecuteCommandSendMessageToPC` | Steam addr UNKNOWN (GOG 0x7A60E0) | needs mapping | whether the script reached the message call | cold |
| H6 | `CExoSoundSourceInternal` ctor / resource helper | Steam addr UNKNOWN (GOG 0x63D990, 0x63DC50) | needs mapping | the audio backend actually opened a source for a resource | warm |

Interpretation matrix once H1+H2 exist (the decision table for E1):
| NCS `kpv_e1` in H1 | `PLAYSOUND call` H2 | WAV `kpve1a` in H1 | Verdict |
|---|---|---|---|
| no | no | no | `runscript` does not run scripts: native hotkey or `commands/*.toml` route |
| yes | no | no | script loaded but `PlaySound` never executed: script/compile problem (check ncs, SendMessage H5) |
| yes | yes | no | engine called `PlaySound` but never looked up the name: wrong path (handler gated on a different 2DA/sound-set check) |
| yes | yes | yes, result miss | lookup happened, file not found in Override: wrong type/ext or dir scan order (H1 logs the result) |
| yes | yes | yes, result hit | lookup found the file but nothing played: container/format or voice-stream gate (go to E2 matrix) |

## 3. Exact code changes (Task units)
All under `patch_manager_mods/voice_probe_src/`. Keep the default `probe=0`; one lazy ini read (absolute path, already fixed).

### Task R (recon, read-only, `rea-recon` or Haiku `recon` contract; Write scope: `.claude/research/voice_probe_hooks/R*.md`)
- R1: for 0x00712360, 0x00711ED0, 0x00711F30, 0x007125B0 on the Steam exe: calling convention, stack args (which is the resref `CResRef*`/char[16], which is the restype u16), return type, first 6-8 bytes as whole instructions, `RET n`. Evidence = decompile + `read_function_instructions`.
- R2: `RunScript` 0x006FD8D0 (96 bytes in GOG): whole-instruction boundaries; pick a mid-function address AFTER the companion-ai 6-byte detour region (>= 0x006FD8D6) whose 5+ bytes are whole instructions and where the `CExoString*` script arg is still in a register or `[ebp+8]`. If none, record "no safe site" and rely on H1.
- R3: confirm `AIActionPlaySound` Steam address (GOG 0x6B5DE0 to Steam 0x5D9BF0) and `SendMessageToPC` Steam address from `AddressDatabases` (`query_ghidra.py symbol` returned only GOG for SendMessageToPC); locate the sound-source creation Steam address.
- R4: layouts: `CResRef` (16-byte char array, not NUL-terminated?) and `CExoString` (`{char* text; uint len}`?). State "unverified" if not shown by decompile.
- Anchor for small models: `RE_AGENT_GUIDE.md`; every claim needs an address and bytes. Then Opus runs `python3 scripts/query_ghidra.py address <hex>` on each before any hex enters a hooks toml or `raw_addresses.toml`.

### Task A: pure logic header (testable on host)
- New `VoiceProbeLogic.h` (no windows headers): `bool ResrefHasPrefix(const char (&)[16] resref, const char* prefix)` (case-insensitive; resref not NUL-terminated, max 16 bytes), `const char* RestypeName(uint16_t)` (Aurora/xoreos ids: WAV 4, NCS 2010, MP3 25; **confirm from the engine table in R1 before use**), `struct LineBudget { int perKey; int total; bool take(key); }`.
- New `voice_probe_logic_test.cpp` (colocated, doc 20 conventions): vectors = `kpve1a` (hit), `KPVE8MIXED` (hit, case), `kpve8_seventeench` (hit, 17 chars truncated at 16), `cr_xxx` (miss), empty, 16-byte no-NUL.

### Task B: hooks H1 (+H1b) in `VoiceProbe.cpp`
- Handler shape (cdecl, parameters taken from `[hooks.parameters]` per R1 result; example uses ecx + two stack args):
```cpp
extern "C" void __cdecl VoiceProbeKeyLookup(void* self, const char* resref, int restype, int ret)
{
    if (!g_probeState && !ProbeEnabled()) return;           // one flag; lazy ini read already exists
    char name[17] = {0};
    if (!SafeReadBytes(resref, name, 16)) return;           // saferead.h; never deref raw
    if (!voiceprobe::ResrefHasPrefix(name, g_prefix)) { InterlockedIncrement(&g_lookupsAll); return; }
    if (!g_budget.take(name)) return;                       // 20 lines per name, 400 total
    logf("LOOKUP #%ld tick=%lu name=%s type=%d(%s) ret=%08x", ++g_n, GetTickCount(), name, restype,
         voiceprobe::RestypeName((uint16_t)restype), ret);
}
```
- `voice_probe.ini` new keys: `prefix=kpv` (lookups logged only for names starting with this; empty = log nothing but the counter), `all=0` (1 logs every lookup, capped 2000 lines, for one-off use only), `probe=0` stays the master switch.
- Hot-path rules: first instruction path is `if (!enabled) return`; no allocation, no logf before the prefix test; the counter is `InterlockedIncrement`. GetKeyEntry runs thousands of times per second.
- Result capture: a detour at entry sees the request only. To log hit/miss, add a second hook at the function's return site (R1 gives the `RET` address) reading EAX; if it adds risk, log the request only for v0.2.0 and note it.
- H2 stays as is; add the same prefix-free tick so lines order against `LOOKUP` lines.
- H3 only if R2 yields a safe site; else skip and note it in the README.

### Task C: hooks toml, manifest, version
- `kotor2-steam-aspyr.hooks.toml`: add one `[[hooks]]` per H1/H1b (and H3/H4/H5/H6 only after R verifies), each `type = "detour"`, `original_bytes` = the R1-verified whole-instruction prologue, `skip_original_bytes = false`, `exclude_from_restore = []`, parameters per R1. No address enters the toml, `raw_addresses.toml` or code before `query_ghidra.py address`.
- Raw-address checker: new addresses in `.cpp` must be `gameaddr::` names or allowlisted; keep addresses OUT of C++ comments (the checker counts comments; this broke the gate once).
- Bump `VoiceProbe` to 0.2.0-probe; update `manifest.toml` conflicts: must list `perf-lab` and `companion-ai` as non-overlapping (no shared entry address) and confirm KPM does not flag same-function mid hooks.

### Task D: scripts and ladder
- Add `kpv_e1b.nss` (SendMessageToPC first, then `PlaySound`) only if a differential is needed; otherwise the existing 9 scripts + `kpv_c1` (credits control, already compiled) are the trigger set.
- Update `V-C_RUNBOOK.md` step 4 to read the new `LOOKUP` and `PLAYSOUND` lines through `kotor2_logs.py check voice_probe_log.txt --new` and the matrix above.

## 4. Edge cases and test matrix
- probe=0: zero log lines, near-zero cost (flag test only). probe=1 with a missing ini: stays 0.
- Area transition / save load (GetKeyEntry heavy): no growth beyond the line budget; log file capped (`kMaxLogLines` pattern from AreaHookProbe).
- Case: engine lowercases resrefs; `KpvE8Mixed` may reach the hook as `kpve8mixed` (E8 data point, not a bug).
- Over-long names: 17-char `kpve8_seventeench` is truncated to 16 bytes by the engine; the log shows the truncated key.
- Null/garbage `resref` pointer: `SafeReadBytes` fails, return; never crash the game.
- Multithreading: GetKeyEntry may be called from loader threads. Handler uses only Interlocked counters and a lock-free budget; the log writer already serialises (check `logf`).
- Hook coexistence: after install, `perf-lab` and `companion-ai` logs must show the same behaviour as before (compare `kotor2_logs.py check perf-lab_log.txt` and the companion-ai probe line counts).

## 5. Tooling and verification (commands, in order)
1. `python3 scripts/run_tests.py voice_probe` (the new logic test) and `-k kvoice` (9/9 expected).
2. `python3 scripts/build_check.py --mod voice_probe_src` (FAIL=0).
3. `python3 scripts/check_hook_bytes.py` (new hooks PASS; 0 FAIL).
4. `python3 scripts/check_raw_addresses.py --quiet` (0 FAIL).
5. `python3 scripts/compat_check.py patch_manager_mods/voice_probe_src/VoiceProbe.cpp` if any shared header is touched.
6. Agent work: `agent_contract.py snapshot` before, `agent_contract.py check --scope <globs> -` after, each Haiku unit under 60k tokens, one module per card.
7. Gate: `scripts/pre_install_gate.sh --mod voice_probe_src` must print `GATE PASS` (fresh, after the last edit).

## 6. Install and live steps (each needs the user's GO, game closed)
1. `python3 scripts/kpm_status.py`; confirm no peer installer is active (md5s stable across two runs).
2. Ask the sibling gate session whether a reinstall of `npc_world`, `npc_world_probe`, `k1_spawn_probe` is planned; if yes, one combined run.
3. `python3 scripts/kpm_reinstall.py` per `kotor2-kpm-install` skill (snapshot, flat-stage run, peer DLL restore, rollback command printed). Pass: `patch_config.toml` lists `voice-probe`, every pre-existing patch DLL md5 unchanged except the intended ones.
4. Set `<game>/voice_probe.ini` `probe=1`, `prefix=kpv`. Load `000011 - KPV E1 TEST`, run `runscript kpv_c1`, then `kpv_e1`, then `kpv_e2c`.
5. Read: `python3 scripts/kotor2_logs.py check voice_probe_log.txt --new` and the inotify watcher; fill the matrix in section 2.
6. Cleanup: `probe=0`; `rm -f "$OVR"/kpv* "$OVR"/Kpv*`; delete the test slot; uninstall the probe only if the user asks (rollback command from step 3).

## 7. Time and budget
- Recon R1-R4: 4 Haiku/rea cards, about 20 min wall, under 60k tokens each.
- Tasks A-C: 3 cards, about 40 min; Opus review of `VoiceProbe.cpp` and the toml about 15 min.
- Gate plus reinstall: about 15 min (bundled).
- Live read of E1 with logs: about 10 min.

## 8. Decision gates (ask the user before each)
1. Is the watcher result enough? (If it shows `kpv_e1.ncs` opened and no `kpve1a.wav` open, H2 + H1 are the minimum.)
2. GO for the combined full reinstall.
3. GO to hook mid-function RunScript (H3) at all: it sits next to a live companion-ai detour.

## 9. E1 outcome so far (2026-10-15) and plan change
- Runs on the live game: `runscript kpv_e1`, `kpv_e4` (no audio, no text), then after a FRESH game start (23:11, all `kpv*` files indexed) `runscript kpv_c1` (control: +1234 credits, overhead text): no credits, no text, no console error.
- inotify watcher on `Override` saw NO game open of `kpv_c1.ncs` (nor `kpv_e1.ncs`/`kpve1a.wav`). Reading: the console `runscript` never reached script loading. E1 is therefore INCONCLUSIVE about audio: the trigger never fired.
- Likely cause (unverified): `additional-console-commands` `runscript` returns silently when `CServerExoApp::GetInstance()` is null (`if (!server) return;`), or the Steam address set is wrong for that call. Untested.
- Consequence: before any audio logging pays off, the trigger route must change. Order of work becomes: (1) a trigger that provably runs a script (below), (2) the hooks in section 3.
- Trigger options, best first: (a) native hotkey in `voice_probe_src` that calls `RunScript` 0x6FD8D0 with the PC id (same reinstall as the logging hooks); (b) `commands/*.toml` native console command; (c) override a vanilla script that fires on its own in the forged save area (OnEnter/heartbeat), no native code, needs a verified script name and cleanup.
