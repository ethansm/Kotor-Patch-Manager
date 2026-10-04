# Movie Patch (K2 half): Steam Aspyr port spec (task R-E, read-only research)

Target: KOTOR II Steam Aspyr `swkotor2.exe` sha256 `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (base 0x400000; scratch vanilla exe hashes to it).
Source: `tools/kpm-pr-movie/Patches/Movie Patch` @ upstream master 6374f73 (manifest 1.5.3; K1 GOG, K1 CD-crack, K2 GOG `777BEE23...`). Only the K2 path (2 detours in `kotor2_hooks.toml`, 2 exports) is ported; `hooks.toml` (K1, 9 hooks), the K1 exports and K1 constants stay untouched.
Deliverable beside this file: `kotor2-steam-aspyr.hooks.toml.draft`. Helpers (read-only; need `K2_STEAM_EXE`): `hookcheck.py`, `movie_check.py`, `iat_names.py`, `callnames.py`, `glob_refs.py`, `branch_into.py`, `gen_draft.py` (imports `../cmc_steam/xref.py`).

## 1. Address table

Whole movie module is a constant delta: GOG - Steam = **0x5131F0** (hook 0, hook 1, and the three callers of ShutDown all fit it; no other address here does).

| # | export | GOG VA | Steam VA | len | Steam original_bytes | uniq in .text |
|---|---|---|---|---|---|---|
| 0 | blitMovieMitchellNetravaliKotor2 | 0x00920AF1 (`RunMovieFrameLoop` 0x9208E0 +0x211) | **0x0040D901** (`FUN_0040D6F0` +0x211; sz 807, ret 4, 203 insns on both) | 30 | `50 8B 8D 20 FF FF FF 8B 51 48 83 C2 34 52 8B 85 20 FF FF FF 8B 48 4C 51 FF 15 A0 64 98 00` | bytes alone 1 hit; with 8 ctx bytes (`8B 95 20 FF FF FF 8B 42`) 1 hit |
| 1 | releaseMovieFilterKotor2 | 0x0091FD10 (`CExoMoviePlayerInternal::ShutDown` entry) | **0x0040CB20** (`FUN_0040CB20`; sz 220, ret 0, 50 insns on both) | 9 | `55 8B EC 81 EC 84 00 00 00` (identical to GOG) | bytes alone 13 hits, +8 ctx 3 hits, **+13 ctx = 22 bytes `55 8B EC 81 EC 84 00 00 00 89 8D 7C FF FF FF 8B 85 7C FF FF FF C7` 1 hit** |

Hook 0 bytes differ from GOG in exactly two bytes (indexes 26, 27: the IAT operand `BC 54` -> `A0 64`); all other 28 bytes identical, same `disp`/`imm` pattern. Hook 1 is byte-identical. Both are detours on instruction boundaries: hook 0 spans 9 whole instructions (ends 0x40D91F, right after `call [BinkBufferBlit]`), hook 1 spans 3 (`push ebp; mov ebp,esp; sub esp,0x84`); no branch lands inside either span (checked with `branch_into.py` on both exes). Hook 1's 9 bytes are not unique on their own; KPM applies by address and refuses on byte mismatch, and the identity is pinned by the function fingerprint plus the 22-byte pattern (and by the `ShutDown` callers, section 3).

Constants (the K2 `MovieFilterAddresses` struct in `better_movie_playback.h`; `Kotor2GogMovieFilterAddresses` = {0x9854BC, 0x9854C8, 0x9F22A4, 0x9F22A8}):

| field | GOG | Steam | meaning / anchor |
|---|---|---|---|
| binkBufferBlitIatAddress | 0x009854BC | **0x009864A0** | IAT slot of `_BinkBufferBlit@12` on both (name read from the import table, `iat_names.py`) |
| binkCopyToBufferIatAddress | 0x009854C8 | **0x00986494** | IAT slot of `_BinkCopyToBuffer@28` on both |
| screenWidthAddress | 0x009F22A4 | **0x009F42A4** | int, current width; anchor G1 |
| screenHeightAddress | 0x009F22A8 | **0x009F42A8** | int, current height; anchor G1 |

(The R-E brief listed 0x9864A0/0x986494 as GOG; they are the Steam slots. GOG slots are 0x9854BC/0x9854C8.) The Steam IAT is ordered differently (GOG ascending NextFrame, Wait, BufferBlit, GetRects, BufferUnlock, CopyToBuffer; Steam descending: CopyToBuffer 0x986494, BufferUnlock 0x986498, GetRects 0x98649C, BufferBlit 0x9864A0, Wait 0x9864A4, NextFrame 0x9864A8), which is why a delta guess would be wrong. The C++ reads the slot at run time (`*reinterpret_cast<Fn*>(slot)`), so the slot content is the real Bink function pointer on both.

Unchanged literals: `Kotor2MoviePointerFrameOffset 0xE0` (`[ebp-0xE0]` = movie object, both), `MovieBinkOffset 0x48`, `MovieBufferOffset 0x4C`, `MovieWindowOffset 0x50` (HWND; ShutDown calls `DestroyWindow([this+0x50])` on both), `BinkFrameBufferOffset 0x34` (`add edx,0x34`). Hook 1 `disp`/`imm` lists identical (0x24, 0x48, 0x4C, 0x50, 0x64, imm 0x84).

## 2. Frame and registers the C++ uses

- **Hook 0** passes `ebp` (pointer) and `eax` (uint). C++: `movie = *(DWORD*)(ebp - 0xE0)`, `rectCount = eax`. On both exes the frame loop is `mov ecx,[ebp-0xE0]; mov edx,[ecx+0x4C]; ...` and the stolen `push eax` pushes the return of the immediately preceding `call [BinkGetRects@8]` (GOG 0x920AEB -> slot 0x9854C0; Steam 0x40D8FB -> slot 0x98649C, `_BinkGetRects@8` both). So `eax` = rect count on both. The stolen sequence is `push eax; push [movie+0x48]+0x34; push [movie+0x4C]; call [BinkBufferBlit@12]` (stdcall, callee pops 12), i.e. stack-balanced, so `skip_original_bytes = true` plus the C++ fallback (`blitMovieOriginalImpl` calling the same slot) is correct. After the span the code reloads from `[ebp-0xE0]` (`BinkNextFrame`), so clobbering eax/ecx/edx is harmless on both.
- **Hook 1** has no parameters; runs at ShutDown entry (`ecx` unused). It only calls `releaseMovieContext()`.
- Call-by-name equivalence of the two containing functions (every `call [IAT]` in order, by import name): ShutDown = BinkGetSummary, BinkPause, BinkClose, BinkBufferClose, DestroyWindow (same on Steam); frame loop = BinkDoFrame, BinkBufferLock, BinkCopyToBuffer, BinkBufferUnlock, BinkGetRects, BinkBufferBlit, BinkNextFrame, BinkWait, Sleep, PeekMessageA, GetMessageA, TranslateMessage, DispatchMessageA, BinkGetSummary (same on Steam; `callnames.py`).

## 3. Anchors

- **G1 (globals):** `ChangeResolutionForMovie` GOG 0x40B960 = Steam 0x40BE50 (sz 230, ret 0, 60 insns on both): `mov ecx,[W]; mov [W+0x1C],ecx; mov edx,[H]; mov [H+0x1C],edx` then pushes `H`, `W`, `0x20`, `1`, `0` before a call, same order on both, so W = ...A4 (width), H = ...A8 (height). Absolute-operand xref counts in `.text`: W 193 vs 193, H 209 vs 209; `WinMain` references H then W in both (GOG 0x407AC6/0x407ACD, Steam 0x407FB4/0x407FBA).
- **I1 (IAT):** slot values are RVAs of hint/name entries; GOG read from the raw `.rdata` dump, Steam from the exe. Names above. Call sites: GOG `call [0x9854C8]` at 0x920ABE and `call [0x9854BC]` at 0x920B09; Steam `call [0x986494]` at 0x40D8CE and `call [0x9864A0]` at 0x40D919, each with the same push sequence before it.
- **M1 (module):** the three ShutDown callers (GOG 0x91FC9E, 0x9202BC, 0x920C1A; Steam 0x40CAAE, 0x40D0CC, 0x40DA2A) all differ by 0x5131F0, as do the hooks. The EKC spec independently pins the movie window proc (0x40C4A0) and `StartMovie` (0x40CC70) in this same module.

## 4. C++ switch design (EKC pattern; compile-checked)

Existing design: `MovieFilterAddresses` struct (header), `Kotor1MovieFilterAddresses`, `Kotor2GogMovieFilterAddresses`; `blitMovieOriginalImpl/blitMovieFilteredImpl/originalBinkBlit/renderFilteredFrame` already take `const MovieFilterAddresses&` and use `.binkBufferBlitIatAddress/.binkCopyToBufferIatAddress/.screenWidthAddress/.screenHeightAddress`. Only the K2 export picks the table, so the switch is local to `blitMovieMitchellNetravaliKotor2`; K1 export, `applyMovieAspectScale` (uses K1 `ScreenWidth/Height` globals) and `postMovieCleanup` are not touched. `exports.def` unchanged; both K2 hooks files call the same two exports. KotorPatcher calls `SetEnv("KOTOR_VERSION_SHA")` (`patcher.cpp:122`) before `ApplyPatches` (148)/`LoadModule` (226). Unknown/missing sha = GOG addresses (today's behaviour).

Header (`better_movie_playback.h`, after `Kotor2GogMovieFilterAddresses`):
```cpp
// Steam Aspyr (6A522E71...): same code and struct layout, IAT and globals moved.
constexpr MovieFilterAddresses Kotor2SteamMovieFilterAddresses = {
    0x009864A0, 0x00986494, 0x009F42A4, 0x009F42A8 };
```
`export.cpp` (add `#include <cstring>`; selector in the anonymous namespace; replace the two uses of `Kotor2GogMovieFilterAddresses` in the K2 export by one `addresses` local):
```cpp
constexpr char Kotor2SteamSha[] = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";
const MovieFilterAddresses& kotor2MovieFilterAddresses() {
    static const MovieFilterAddresses* selected = nullptr;
    if (!selected) {
        char sha[128] = {};
        const DWORD length = GetEnvironmentVariableA("KOTOR_VERSION_SHA", sha, sizeof(sha));
        const bool steam = length == sizeof(Kotor2SteamSha) - 1 && _stricmp(sha, Kotor2SteamSha) == 0;
        selected = steam ? &Kotor2SteamMovieFilterAddresses : &Kotor2GogMovieFilterAddresses;
    }
    return *selected;
}
// in blitMovieMitchellNetravaliKotor2: const MovieFilterAddresses& addresses = kotor2MovieFilterAddresses();
//   prepareMovieContext fail -> blitMovieOriginalImpl(movie, rectCount, addresses); else blitMovieFilteredImpl(movie, rectCount, addresses);
```
Scratch build of all 3 `.cpp` (original vs patched, `i686-w64-mingw32-g++ -std=c++17 -Wall -Wextra -O2 -shared -lgdi32 -luser32`): both link, 33 warnings each (pre-existing `-Wcast-function-type`), warning sets identical, none new. Diff about 49 lines in `export.cpp` and 17 in the header. Not run (no game).

## 5. Files to change in the PR (`Patches/Movie Patch/` only)

1. New `kotor2-steam-aspyr.hooks.toml` = the draft. It is `kotor2_hooks.toml` with only `target_versions`, the two `address` values and the 2 IAT bytes of hook 0 `original_bytes` changed (`gen_draft.py` asserts every other field equal and original_bytes == Steam exe). `*hooks.toml` is globbed by `create-patch.py`/`validate-patches.py`; selection is by `target_versions`, so the existing `hooks.toml` (K1) and `kotor2_hooks.toml` (K2 GOG) need no rename.
2. `better_movie_playback.h` and `export.cpp`: section 4.
3. `manifest.toml`: `version = "1.6.0"` and add `kotor2_steam_aspyr = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"`. Description/README text mentions of "K2" can stay.

## 6. Risks

1. Not run. First live test (user): intro logos, a cutscene, skip with Esc, alt-tab during a movie; the debug strings `[Movie Patch] Movie OpenGL context created.` / `Mitchell-Netravali frame presented successfully.` (OutputDebugStringA) should appear. Wrong IAT/global values would crash on the first movie frame, so these four constants are the part to re-verify.
2. Runs an OpenGL 2 filter on the movie thread under Proton (wgl via Wine's opengl32); the GOG path was only tested on Windows. Failure falls back to the original Bink blit (`blitMovieOriginalImpl`), which is safe only if the IAT slot is right.
3. Local mods near the area (no byte overlap): `activation_fix_src` replaces `call [PostMessageA]` at 0x40D69E and `load_hang_probe_src` detours 0x40D692/0x40D637, both inside the movie thread proc `FUN_0040D430` (sz 686, ends 0x40D6DE), a different function from `FUN_0040D6F0` (hook 0). EKC's movie window proc hook is 0x40C4A6 (different function). A `check_hook_spans.py` run is still advisable at install time.
4. Behaviour difference to document: on Steam this ports only the bicubic blit (no in-window movies, no aspect scale, no 640x480 override), as on GOG K2.
5. Hook 1's 9 original_bytes are not unique in `.text` (13 hits alone); a mis-targeted address would be refused only if the bytes differ, so the address is protected by the 22-byte pattern, the fingerprint and the callers (M1), not by the 9 bytes.
6. `KOTOR_VERSION_SHA` missing on Steam would select GOG addresses and crash at the first movie frame; KotorPatcher always sets it (a failed `SetEnv` only logs).
7. `validate-patches.py` requires the sha in an address DB; `AddressDatabases/kotor2_steam_aspyr.db` exists at master.
