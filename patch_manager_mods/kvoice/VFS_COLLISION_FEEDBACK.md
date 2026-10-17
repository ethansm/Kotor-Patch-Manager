# Feedback for the doc 44 (in-memory VFS) session: collisions and audio facts (2026-10-17)

From the doc 41 voice session. Sources: live E1/E2 play tests (2026-10-15/16), Haiku recon wave `w13-vfs-collide` (6 units, read-only; units ended PARTIAL, claims below name their evidence). Doc 44 was not edited.

## A. Hook collisions (must change doc 44)
1. **`CExoResMan::Demand` 0x00711C20 is TAKEN** by the live `perf-lab` (`perf_lab_src/perflab/kotor2-steam-aspyr.hooks.toml:81`, 6-byte detour; same function also has 0x00711D04, :94). KPM refuses two detours on the same address at install (`HookValidator.cs:154-175`, before any game write). Doc 44 Layer 1 lists Demand as an interception point: drop it.
2. **`CExoResMan::Malloc` 0x00712F30 is TAKEN** by `perf-lab` too. Calling it from the VFS is fine, but every VFS call is counted in `perf-lab` stats (harmless, note it).
3. **FREE (no hook in any steam hooks toml):** `GetKeyEntry` 0x00712360, `ServiceFromImage` 0x00713E80, `Exists` 0x00711ED0, `Exists_2` 0x00711F30, `StartStream` 0x0070C0C0. (u01: 5 of 6 doc-44 detours free; other hook sources not scanned.)
4. **Doc 44 contradicts itself on Demand:** the diagram says Demand 0x7111E0, the table says 0x00711C20 is both `Demand` and `DemandResMan`. DB: 0x00711C20 = `CExoResMan::Demand` (verified B0); 0x007111E0 = `CRes::Demand` (a different function, verified B0, not hooked by anyone). Pick one name per address.
5. **`OpenStream` 0x0070CAA0 is mislabelled.** Address DB (Steam): `CExoStreamingSoundSourceInternal::InitializeSource`, 3882 bytes. `k2-mp3-playback-fix` (LIVE) hooks INSIDE it at 0x0070CDCB (10 bytes) and 0x0070CF34 (6 bytes), reading ebp-relative locals (-0x13C, -0x12C, -0x158, -0x14) and doing its own `fopen`. A new entry detour on 0x70CAA0 would install (windows disjoint) but must keep the ebp frame, runs BEFORE the mp3 rewrite/preference logic, and a file-less in-memory stream makes the mp3 fix's `fopen` fail. `K2_music_fix` also hooks 0x0070E130 (in-place NOP, function bounds not checked). `0x0070B170` has no DB entry; `StartStream` 0x0070C0C0 is medium confidence only. Rework Layer 3: prefer hooking the file open/read below `InitializeSource` (CRT `fopen`/`ReadFile`, or Miles `_AIL_set_file_callbacks`, which doc 44 already mentions) over the function entry, and re-verify names with `query_ghidra.py address`.
6. **KPM rules to design around:** same address = install refused; overlapping byte windows from different addresses PASS install but the later one aborts at runtime and stops every later patch (`patcher.cpp:270-285, 410-416`), so check window overlap by hand; a detour at a different address in an already-detoured function is fine (perf-lab does it); any new hook or patch id = full reinstall; `provides`/`requires_interfaces` are informational only (not checked against the DLL); `conflicts` only checks the install set.
7. **Pre-existing, unrelated:** `npc-world` and `area-hook-probe` overlap at 0x55A460 and 0x533830 (probe not live).

## B. Audio facts from live tests (change the Layer 1 / Layer 3 contract)
1. **`PlaySound` (script) = Layer 1.** E1: a loose RIFF PCM16 mono 22050 Hz `.wav` in `Override` PLAYS. The engine opens `kpve1a.wav` by name (watcher saw OPEN each call).
2. **E2b:** a 58-byte VO container (RIFF size field 50) + MP3, named `.wav`, is OPENED and READ but SILENT through `PlaySound`. So on Layer 1 the VFS must serve real RIFF PCM; the container is not decoded on this path.
3. **E2c:** a bare `.mp3` was NEVER looked up (44 script runs). `PlaySound` asks for restype WAV only; `k2-mp3-playback-fix` does not make it reachable.
4. **Dialog VO and TLK barks = Layer 3.** E4 evidence: TLK row 36878 plays `StreamSounds/P_BASTILA_RPRTY.wav`, not in `chitin.key`, a 470-byte prefix + RIFF.
5. Untested: bare MP3 bytes served under a WAV name (E2e), the 470-prefix variant (E2d), case/length (E8), concurrency (E7).

## C. Packer / registry gaps (u05, from `k1_port_vfs_pack.py` and `VfsRegistry.h`)
1. `.mp3` maps to restype 4 and is stored RAW (no transcode, no RIFF wrap). With B2/B3 a Layer-1 MP3 asset will not play: transcode to RIFF PCM at pack time or lazily at serve time (embed MP3, decode on demand to keep the DLL small).
2. `foo.wav` and `foo.mp3` in the same flag class raise a duplicate-key error (key ignores extension). Fine, but document it.
3. The flag is set by directory (`streamvoice`/`streamsounds` or `--stream-files`) with no stated rule. Make it explicit: script-played SFX = FlagResMan, dialog/bark = FlagMilesStream.
4. `inspectAudioContainer`: 58-byte VO classified as `.mp3`, 470-byte as `.wav`, plain RIFF passes raw. Callers and `streamKind` values unchecked; `normalizeResRef` lines 65-99 and kvoice rule R9 unchecked.
5. Walking a directory does not resolve symlinks (`:188-191`), so `--stream-files` membership can miss symlinked files.

## D. Provider question (user, 2026-10-17)
Moving contended addresses behind a provider is the right long-term pattern (like `k2.npcworld@1`), but only the `Demand` collision needs it, and the VFS does not need `Demand` if it uses `GetKeyEntry` + `ServiceFromImage`. Needed API shape if it is ever built: observers (all called, cannot change the result, `perf-lab`) and resolvers (first claim wins, the VFS). Migrating `perf-lab` costs a full reinstall, a load-order dependency and an extra call on a hot path. Recommendation: do not migrate now; write the rule "one detour per address; shared interface when two mods need it" into doc 44.

## E. Open (not resolved by the wave)
- Is `perf-lab` pair 0x711C20 + 0x711D04 live in the game config (the Haiku could not read it)? `patch_config.toml` lists the id, not addresses; the id is live.
- Function bounds of 0x70CAA0 vs 0x70E130.
- Whether KPM `--apply` checks PE export/import changes (BUILD_AND_INSTALL.md:62; not in KPM src).
