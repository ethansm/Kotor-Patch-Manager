# K2ClassMenuClothes: Steam Aspyr port spec (task R-E, read-only research)

Target: KOTOR II Steam Aspyr `swkotor2.exe` sha256 `6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF` (base 0x400000; the scratch vanilla exe hashes to it).
Source: `tools/kpm-pr-cmc/Patches/K2ClassMenuClothes` @ upstream master 6374f73 (manifest 1.0.0, GOG `777BEE23...`).
Deliverable beside this file: `kotor2-steam-aspyr.hooks.toml.draft`. Helper scripts (all read-only; need `K2_STEAM_EXE`): `hookcheck.py`, `cmc_check.py`, `fn_check.py`, `xref.py`, `c2da_anchor.py`, `branch_into.py`, `gen_draft.py`.

## 1. Address table (hooks first, then constants)

Hooks: `original_bytes` are byte-identical on GOG and Steam (read from both exes). "ctx hits" = hits in Steam `.text` of original_bytes + the next 8 bytes (all 1). "bytes-only" = hits of original_bytes alone.

| # | export | GOG VA | Steam VA | len | original_bytes | bytes-only | ctx hits | containing fn (GOG -> Steam) |
|---|---|---|---|---|---|---|---|---|
| 0 | BeginClassMenuAppearance | 0x005DE18A | **0x008F9DCA** | 10 | `8D 45 88 50 8B 8D 5C FF FF FF` | 1 | 1 | `CSWGuiClassSelection` ctor 0x5DD670 -> 0x8F92B0 (sz 3843, ret 8, 909 insns) +0xB1A |
| 1 | OverrideClassMenuModel | 0x0048E574 | **0x0085E4E4** | 7 | `8D 45 E0 50 8D 4D 8C` | 3 | 1 | `CSWCCreatureAppearance::GetBTypeModelAndTextureResRef` 0x48E3B0 -> 0x85E320 (sz 813, ret 12, 222 insns) +0x1C4 |
| 2 | OverrideClassMenuTexture | 0x0048E691 | **0x0085E601** | 7 | `C6 45 FC 00 8D 4D B4` | 32 | 1 | same fn +0x2E1 |
| 3 | EndClassMenuAppearance | 0x005DE199 | **0x008F9DD9** | 10 | `8B 4D 80 51 8B 95 5C FF FF FF` | 1 | 1 | ctor +0xB29 |

Steam ctx bytes (the 8 after original_bytes): #0 `E8 27 34 E7 FF 8B 4D 80`, #1 `E8 80 50 ED FF C6 45 FC`, #2 `E8 73 51 ED FF EB 14 68`, #3 `8B 02 8B 8D 5C FF FF FF`. All four hooks are plain detours on instruction boundaries (checked: start on a decoded insn, end on a decoded insn, no branch lands inside, on both exes; no two hooks overlap). The function size/ret/insn-count triples are identical on GOG and Steam (server `fp.tsv`).

Constants (all code addresses; no vtables, globals or IAT). Steam VA = same size, same `ret N`, same insn count (`fp.tsv`), same prologue bytes, 16-byte aligned; anchors in section 3.

| field | GOG | Steam | sz / ret / insns | anchor (section 3) |
|---|---|---|---|---|
| ExoStringDefaultConstructorAddress | 0x004EAED0 | **0x00733540** | 33 / 0 / 12 | A2, A3 |
| ExoStringConstructorAddress (char*) | 0x00605680 | **0x00733570** | 126 / 4 / 44 | A1, A3 |
| ExoStringDestructorAddress | 0x00605890 | **0x00733780** | 60 / 0 / 21 | A1, A3 |
| C2DAConstructorAddress (ResRef by value, int) | 0x006207E0 | **0x0071AD40** | 258 / 20 / 64 | A2 (111 name-matched call sites) |
| C2DADestructorAddress | 0x00620920 | **0x0071AE80** | 107 / 0 / 31 | A2 (ctor/Load/dtor triplets) |
| C2DALoadAddress | 0x00621E70 | **0x0071C3D0** | 2991 / 0 / 764 | A2 |
| C2DAGetStringAddress (row, col, out) | 0x00620C70 | **0x0071B1D0** | 267 / 12 / 90 | A1, A2 |

Hard-coded struct facts (unchanged): `SlotOffset 0x34` = `[ebp-0x34]` slot loop variable in the class-selection ctor; `ModelArgumentOffset 0x0C` / `TextureArgumentOffset 0x10` = `[ebp+0xC]` / `[ebp+0x10]` output ResRef pointers of the resolver; C2DA object = 0x54 bytes (`push 0x54` / `new`); ResRef 16 bytes; ExoString `{char*, DWORD}`.

## 2. Frame slots and registers the C++ uses (GOG vs Steam disassembly)

- **Hooks 0 and 3** pass `ebp` (class selection ctor frame); C++ reads `*(int*)(ebp - 0x34)`. Both exes: `mov dword [ebp-0x34],0` loop init (GOG 0x5DDA9D / Steam 0x8F96DD), loop body `mov ecx,[ebp-0x34]; imul ecx,ecx,0x26C` at 0x5DE1E0 / 0x8F9E20 (bytes `69 C9 6C 02 00 00` on both; class-slot stride 0x26C identical). Same loop counter, so slot 0 triggers `LoadClassMenu2DA()`. Hook 0 sits inside `if ([0x9F22B4]!=0 -> Steam 0x9F42B4)` just before the `call` that creates the preview (GOG 0x419070 / Steam 0x76D200); hook 3 is the first insn after that call. Instruction sequence around both is the same byte-for-byte except rel32/abs32 operands.
- **Hooks 1 and 2** pass `ebp` (resolver frame, `ret 0xC`, 3 stack args). C++ reads `*(void**)(ebp+0x0C)` = model out ResRef and `*(void**)(ebp+0x10)` = texture out ResRef, then `memcpy`s 16 bytes. Both exes: `mov ecx,[ebp+0xc]; call <ResRef copy>` (GOG 0x48E562 / Steam 0x85E4D2) for the model, and `mov ecx,[ebp+0x10]; call <copy>` (0x48E689 / 0x85E5F9) for the texture; `cmp [ebp+0x10],0` guard at 0x48E56A / 0x85E4DA. Callees are rel32-different but fingerprint-equal in `fp.tsv` (GOG 0x61A710 sz 235/ret 8/80 insns = Steam 0x710DC0; GOG 0x61A5F0 sz 98/ret 8/35 insns = Steam 0x710CA0), called with the same push order.
- Hook 1 follows `lea eax,[ebp-0x20]; push eax; lea ecx,[ebp-0x74]` which feeds the ExoString ctor call (`0x605680` / `0x733570`); hook 2 runs after the texture ResRef copy. Same on Steam.

## 3. Anchors (cited evidence, not mapper votes)

- **A1 (hook-site calls):** `GetBTypeModelAndTextureResRef` calls ExoString(char*) ctor at +0x1CB (GOG 0x48E57B -> 0x605680; Steam 0x85E4EB -> 0x733570), `C2DA::GetCExoStringEntry` at +0x20B (GOG 0x48E5BB -> 0x620C70; Steam 0x85E52B -> 0x71B1D0, args `WORD [ecx+0x18]`, `&ExoString`, `&ExoString`; `ret 12`), and ExoString dtor at +0x21A (0x48E5CA -> 0x605890; 0x85E53A -> 0x733780). Same call order and argument pushes on both.
- **A2 (call topology, `c2da_anchor.py`):** the 2DA loader pattern `new(0x54); push ResRef("name"); C2DA ctor; Load; GetString...; dtor` exists on both. 111 call sites of the C2DA ctor are paired by the unique 2DA name string they push (`Appearance`: GOG 0x480C6F / Steam 0x6E7CCF; `CLASSES`: 0x50C586 / 0x69FF46; `BindableKeys`: 0x483619 / 0x6EA679; ...). In the `Appearance` loader the next calls are Load (GOG 0x480CB7 -> 0x621E70 / Steam 0x6E7D17 -> 0x71C3D0) and ExoString ctor (0x480CCF / 0x6E7D2F). Triplet ctor/Load/dtor at GOG 0x5E3FD8/0x5E3FE7/0x5E3FFA = Steam 0x8FFB38/0x8FFB47/0x8FFB5A (dtor 0x620920 vs 0x71AE80). Exact `E8` xref counts in `.text`: ctor 145 vs 145, Load 145 vs 145, GetString 220 vs 220, dtor 6 vs 6. The triplet lives in one function on both (GOG 0x5E3F90 / Steam 0x8FFAF0, sz 502, ret 8, 130 insns).
- **A3 (ExoString trio):** prologue bytes equal (`55 8B EC 51 89 4D FC 8B 45 FC` default ctor; `55 8B EC 83 EC 08 89 4D F8 83` ctor; `...8B` dtor), `ret` at the same offset (+0x20 / +0x7B ret 4 / +0x3B), same sizes and insn counts. The Steam C2DA ctor (0x71AD40) lists 0x733540 among its callees as the GOG ctor lists 0x4EAED0 (`fp.tsv`).
- Mapper pitfall: the old function-mapper left C2DA ctor unmapped; the neighbour-delta guess (0x71AD40) is now confirmed by A2. The Steam server label of 0x71AE80 (`~CMFCFilterChunkValueImpl`) and 0x71C3D0 (`C2DA::LoadText`) are not reliable names; identity rests on A2.

## 4. C++ switch design (EKC pattern, minimal diff, compile-checked)

`ClassMenuModels.cpp` takes 7 `constexpr` addresses and builds `const auto` function pointers at namespace scope. Replace the 7 constants by a struct plus two tables and select once. `exports.def` is unchanged (same 4 exports); GOG values byte-identical. KotorPatcher calls `SetEnv("KOTOR_VERSION_SHA")` (`src/KotorPatcher/src/core/patcher.cpp:122`) before `ApplyPatches()`/`LoadModule` (lines 148/226), so the variable exists when the DLL's namespace-scope initialisers run. Unknown or missing sha = GOG (same behaviour as today). No `addresses.db`, no `GameAPI`.

```cpp
// Replaces the 7 "Native KotOR II function addresses" constants.
struct NativeAddresses {
    std::uintptr_t c2daConstructor, c2daDestructor, c2daLoad, c2daGetString;
    std::uintptr_t exoStringDefaultConstructor, exoStringConstructor, exoStringDestructor;
};
constexpr NativeAddresses GogAddresses = {
    0x006207E0, 0x00620920, 0x00621E70, 0x00620C70, 0x004EAED0, 0x00605680, 0x00605890 };
constexpr NativeAddresses SteamAddresses = {
    0x0071AD40, 0x0071AE80, 0x0071C3D0, 0x0071B1D0, 0x00733540, 0x00733570, 0x00733780 };
constexpr char SteamSha[] = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF";

const NativeAddresses& SelectAddresses() {   // KotorPatcher exports the target sha before loading patch DLLs
    char sha[128] = {};
    const DWORD length = GetEnvironmentVariableA("KOTOR_VERSION_SHA", sha, sizeof(sha));
    const bool steam = length == sizeof(SteamSha) - 1 && _stricmp(sha, SteamSha) == 0;
    return steam ? SteamAddresses : GogAddresses;
}
const NativeAddresses& Addresses = SelectAddresses();
// and: reinterpret_cast<C2DAConstructor>(Addresses.c2daConstructor) etc. for the 7 pointers
```
`<cstring>` is already included (`_stricmp` via `<windows.h>`/`<cstring>`). Diff size about 45 lines in one file. Compile check (scratch copy, `i686-w64-mingw32-g++ -std=c++17 -Wall -Wextra -O2 -shared`): original and patched both 0 warnings. Not run (no Wine/game run).

Alternative if upstream prefers `GameVersion::GetVersionSha()`: needs `addresses.db`, links Common/GameAPI; rejected for the same reasons as EKC (a missing DB would silently fall back to GOG addresses on Steam).

## 5. Files to change in the PR (`Patches/K2ClassMenuClothes/` only)

1. New `kotor2-steam-aspyr.hooks.toml` = the draft (rename `.draft`). It is the GOG `hooks.toml` with only `target_versions` and the four `address` values changed (verified by `gen_draft.py`: every other field equal; every original_bytes equals the Steam exe at the new address).
2. `ClassMenuModels.cpp`: section 4.
3. `manifest.toml`: `version = "1.1.0"` (minor bump, as K2AspyrMapAspectFix 1.2.0 / #292) and add
   `kotor2_steam_aspyr = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"` under `[patch.supported_versions]`.
4. Optional, not required: merged Steam ports name the GOG file `kotor2-gog-aspyr.hooks.toml`; this patch keeps `hooks.toml`. `*hooks.toml` is globbed by `create-patch.py` (line 436) and `validate-patches.py`, selection is by `target_versions`, so leaving `hooks.toml` as is keeps the diff minimal.
5. `additional/classmenu.2da` unchanged (data only).

## 6. Risks

1. Not run in game. Only static evidence. First live check: new game, class menu; each of the six previews should show the `classmenu.2da` model/texture (vanilla shows the stock outfits).
2. Static-init-time selection: the function pointers are fixed at DLL load. If `KOTOR_VERSION_SHA` were unset on Steam (SetEnv failure only logs a warning) the patch would call GOG addresses and crash at the first class-menu open. A lazy selector does not change that; a fail-closed variant (no hooks active) is impossible because the hooks are applied by the patcher.
3. C2DA/ExoString identity relies on A1-A3 (call-site pairing, equal fingerprints), not on the Steam DB; the DB has no C2DA rows. Do not add DB rows from this spec without a model-verified pass.
4. Steam `0x71AE80`/`0x71C3D0` have misleading server labels (see section 3), so any later tooling that trusts names would be wrong; addresses are right.
5. Hook 0 is conditional on a video/GUI global (`[0x9F42B4]`); same as GOG (`0x9F22B4`), so the override only runs when that flag is set, on both builds.
6. Overlap with local mods: none of the four Steam VAs is hooked by any file under `patch_manager_mods/` (rg check). Linux/macOS Steam builds have different shas; out of scope.
7. `validate-patches.py` requires the manifest sha to appear in an address DB; `kotor2_steam_aspyr.db` exists in `AddressDatabases/` at master.
