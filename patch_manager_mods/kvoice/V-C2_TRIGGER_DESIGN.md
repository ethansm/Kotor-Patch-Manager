# V-C2 trigger design: run a script with no loose game files (doc 41)

> **BUILT 2026-10-17 as voice-probe 0.2.0 (NOT installed).** Code: `voice_probe_src/TriggerCore.h` (pure logic), wiring in `VoiceProbe.cpp`, tests `trigger_core_test.cpp` (45 checks) + `common_contract_test.cpp`. Manifest now `requires = ["npc-world"]`. Ini: `voice_probe.ini [trigger] key=<VK decimal> ctrl=1 shift=0 cooldown_ms=1000 script=<resref>` (key 0 or empty script = off). Log: `TRIGGER cfg ...` once, then `TRIGGER #n t=.. script=.. oid=.. called=.. ret=..` or `skip=<reason>` per press. R-T1/R-T2 answered by wave `w15-41-hotkey` u01: arg 3 = `bOidValid` (1 at 110/142 engine sites), the name is deep-copied (ReadScriptFile -> 0x7337C0), return 1 = ran. R-T4: `a_attspch1`/`a_attspch2` are NOT sound-only (they start Atton's `102Atton` dialog), so no default script is shipped; pick one per live run. Install = the combined VFS reinstall (needs GO).

Status 2026-10-17: design only. No code written, nothing installed or run live. Input: wave `w14-41-VC2` (u01-u03) plus static facts (`ghidra_knowledge/tools/static_facts.py addr`).

Rule (user, 2026-10-17): no loose game files, no overrides, no edits of game files. Only kpatches with hooks and redirects. Allowed exception: the existing ini config files (`voice_probe.ini`, `kplog.ini`). A `commands/*.toml` for `additional-console-commands` is a loose file, so it is out.

## 1. Recommendation

Trigger = a native hotkey inside `voice_probe_src`, polled from the npc-world tick, that CALLS `CVirtualMachine::RunScript` on a vanilla script that contains a `PlaySound` call. No new hook. No file in the game dir except the ini.

- Why: the console `runscript` route never ran a script (section 9 of `V-C2_HOOK_LOGGING_PLAN.md`); a heartbeat override is a loose file; RunScript's entry is owned by companion-ai, but calling a hooked function is fine.
- Gate: `voice_probe.ini` `[trigger] hotkey=0` by default; `script=<vanilla resref>` names the script. Off = zero calls.
- Fallback once the doc 44 VFS lands: the same hotkey runs a probe NCS (`kpv_e1` etc.) served from memory by the VFS, so custom clips need no `Override` file either.

## 2. Facts the design rests on

### 2.1 Tick and key polling (u02)
| Service | Interface | Anchor | Use |
|---|---|---|---|
| npc-world tick (LIVE 0.1.1-dev) | `k2.npcworld` v1, `K2NW_TICK = 8`, every server MainLoop tick, callback must be cheap | `_shared_src/NpcWorld.h:44`, register `:78`, `k2nw::Client::add :128`, `connect :140`, `poll :161` | per-frame callback with no new hook |
| hud-host (LIVE) | `k2.hud`, `K2HUD_FRAME = 0` per PopulateMenus | `_shared_src/HudHost.h:33-36` | only while the HUD draws; second choice |
| key state | `GetAsyncKeyState` directly (no detour), pattern in GuiKit | `_shared_src/GuiKit.h:694-707` | edge-detect one key (e.g. ctrl + F-key) |

- Caveat: with npc-world absent, every `k2nw` call is a no-op (`NpcWorld.h:105-106`); the trigger then silently does nothing. Log one WARN at init when `connect` fails.
- Main-thread question: the tick fires on the server MainLoop, which is the thread RunScript normally runs on. Confirm in the first live run (log the thread id once).

### 2.2 RunScript call recipe (u02 + static facts)
- `CVirtualMachine::RunScript` 0x6FD8D0: thiscall on `*(int*)G_VM`, `G_VM = 0x00A1B4A8` (`companion_ai_src/CompanionAI.cpp:69`). 96 B, `RET 12` = 3 stack args: `CExoString* script`, `int objectSelf`, `int flag` (companion-ai hook reads slots `[esp+4]`, `[esp+8]`; `CompanionAI.cpp:1363-1365`, `companion_ai_src/kotor2-steam-aspyr.hooks.toml:346`). The third arg's meaning is OPEN (R-T1 below).
- Owner: companion-ai detours the entry (`55 8B EC 83 EC 08`, 6 bytes). CALLING it runs through that probe, which filters by return address (`cai::rsSite`) and returns at once for our site. Safe.
- PC object id: `[[G_App]+4]` -> `FnClientList 0x0073FB90` -> `FnClientAt 0x007E5DA0 (x, idx)` -> `FnControlled 0x0077D800 (p)` -> creature; id = `*(int*)(creature + 0x04)` (`CompanionAI.cpp:70-75, :86-87, :350`). `G_App = 0x00A1B4A4`.
- CExoString as used there: `+0x0 char*`, `+0x4 int32 length` (8 bytes; `CompanionAI.cpp:113-121`). Build it on our stack with a static char buffer; the VM must not free it (OPEN R-T2).

### 2.3 Resman facts for the VFS session (u01, reused, not re-derived)
| Function | Convention | Known | Owner |
|---|---|---|---|
| `GetKeyEntry` 0x712360 | thiscall, `RET 16`, 264 B | resref vs u16 restype slot order NOT FOUND (Ghidra signatures disagree with 0x7122C0; asm read needed) | doc 44 VFS (hooks 0x71244E inside it) |
| `ServiceFromImage` 0x713E80 | thiscall, `RET 8`, 297 B | arg1 CRes*, arg2 async flag | doc 44 VFS (hooks 0x713E83) |
| `Exists` 0x711ED0 | `RET 12`, 83 B | calls GetKeyEntry | free (VFS does not hook it) |
| `Exists_2` 0x711F30 | `RET 12`, 150 B | nothing else recorded | free |

Sources: `.claude/waves/w14-41-VC2/u01.md` (rows cite `w14-juhani-bundle/a2_resource_hook.md:8,12,15`, `w13-vfs-collide/u02.md:12,28,33`, doc 44:46-52).

### 2.4 Vanilla scripts with a PlaySound call (u03)
Method (`.claude/waves/w14-41-VC2/u03.md`): `PlaySound` = routine 46 (`nwscript.nss:2306-2308`); pykotor `read_ncs`, hit = ACTION 46, resref = the CONSTS string right before it.

- `chitin.key` (612 global NCS): ZERO `PlaySound` calls (raw-byte cross-check `05 00 00 2E` also 0; control patterns hit 148/116 scripts).
- `Modules/` archives: 18 module-only scripts, all with a constant resref. A script in a module archive resolves only while that module is loaded, so the trigger must run in that module.

| Script | Resref | Module | Note |
|---|---|---|---|
| `a_attspch1`..`a_attspch6` | `evt_commopen` | 102per | Peragus, early save; best pick: a short UI-type sound |
| `a_shutdownff` | `evt_stasisoff` | 102per | same module; may also drop a forcefield (side effect) |
| `k_pervoice` | `101pervoice008` | 101per | voice-type clip; 2 differing md5 copies across archives |
| `a_bet3m4` | `MetalStrain` | 001ebo | Ebon Hawk |
| `a_createg0t0holo` | `v_imp_stun` (x14) | 351nar, 352nar | heavy side effects; avoid |

Pick: `a_attspch1` in 102per (no game state beyond the sound is assumed; read its other ACTIONs before the first live run, R-T4). Module copies are not checksum-verified against a pristine install (files dated 2026-09-24).

Consequence for the design: a vanilla script proves the trigger + `PlaySound` path (V-H0 control) only in 102per. Custom clips (E1/E2e/E4) still need the VFS-served NCS fallback.

## 3. Owned addresses (do not detour)
- `GetKeyEntry` 0x712360 and inside it 0x71244E, `ServiceFromImage` 0x713E80/0x713E83: doc 44 VFS.
- `Demand` 0x711C20, `Malloc` 0x712F30: perf-lab.
- `RunScript` 0x6FD8D0: companion-ai (call it, never hook it).
- Inside `InitializeSource` 0x70CAA0 (0x70CDCB, 0x70CF34): k2-mp3-playback-fix.
- Free and used by voice-probe: `ExecuteCommandPlaySound` 0x68EA70 (`RET 8`, live in 0.1.1).

## 4. What the VFS session must expose for V-H0..V-H4
1. Observe-only mode, ini-gated: one line per lookup with resref, restype, served (memory / disk / miss). This replaces the planned voice-probe H1 hook.
2. The GetKeyEntry slot order (u01 OPEN) once their asm read is done, written to doc 44, so the voice ladder can reuse it.
3. A way to serve an NCS and a WAV from memory by name (the fallback trigger target and the E1/E2e/E4 clips).

## 5. Open items
- R-T1: meaning of RunScript arg 3 (flag). Static: read the 7 companion-ai probed call sites' pushes (`static_facts.py addr` on a caller) before code.
- R-T2: does RunScript copy or keep the `CExoString*`? Read the first 96 B (static dump) before code.
- R-T4: list every ACTION in `a_attspch1` (pykotor `read_ncs`, read-only) to confirm it only plays the sound.
- R-T3: npc-world tick thread = RunScript's usual thread (first live run logs it).

## 6. Decision for the user
1. DONE 2026-10-17: the user chose "build now". Built as 0.2.0. Review fixes (Sonnet, APPROVE, 0 blocking): the key is read first and the other Win32 queries only while it is held; TRIGGER lines have their own 200-line cap; a skipped press (no PC) does not start the cooldown; presses are ignored until npc-world `AREA_STABLE` after each `AREA_LOAD`; `g_nw.poll()` in the PlaySound hook retries a late npc-world connect.
2. Open: GO for the full reinstall (bundle with the VFS install) + the ini lines, then play row VC2-T1.
