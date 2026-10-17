# 44 — In-Memory Asset VFS & Modular NPC Character Library Plan

**Status:** ACTIVE PLAN  
**Scope:** Architecture and implementation plan for 100% in-memory asset delivery (zero disk files) inside a singular `.dll` / `.kpatch`, plus a modular NPC character library interface (`k2.characterlib@1`) for cross-mod sharing.  
**Related Docs:** [34 (NPC Diversity)](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/34_npc_diversity_dispatcher.md), [38 (K1 Asset Import)](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/38_k1_to_k2_asset_import.md), [40 (NPC Behaviour Variety)](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/40_npc_behaviour_variety.md), [41 (Custom Voice & Events)](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/41_custom_voice_and_events.md), [43 (Asset Import Programme)](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/43_asset_import_and_npc_programme.md), and [Juhani RFC Draft](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/new_issue_drafts/issue_rfc_k1_to_k2_asset_porting.md).

---

## 1. Motivation & Core Constraints

Kotor Patch Manager (KPM) mods prioritize **non-destructive, zero-disk-residue installation**:
1. **Zero Loose File Footprint:** Traditional mods dump dozens of `.mdl`, `.mdx`, `.tpc`, `.lip`, and `.wav` files across `Override/`, `StreamVoice/`, and `StreamSounds/`. When a mod is removed, these orphaned files remain, frequently causing conflicts or corrupting uninstaller states.
2. **Zero 2DA Overwrite Risk:** Physical `appearance.2da`, `heads.2da`, and `soundset.2da` files in `Override/` overwrite each other, causing incompatibilities between character mods.
3. **Singular Distribution Container:** Modders and users want an entire character (3D geometry, textures, animations, voice lines, dialogues, spawn templates, and soundsets) packaged completely within a single `.dll` / `.kpatch` archive.
4. **Community Sharing Library:** Individual ported characters should act as modular DLL service providers that any quest, recruitment, or world overhaul mod can plug into without duplicating assets.

---

## 2. Technical Architecture: In-Memory VFS Interception

KotOR II divides resource lookups into three distinct runtime paths. The `asset-vfs` provider intercepts all three without colliding with `perf-lab` (`0x00711C20` / `0x00712F30`) or `k2-mp3-playback-fix` (`0x0070CDCB` / `0x0070CF34`):

```
                      [ Singular .dll / .kpatch Package ]
                      (Embedded PE .rdata / Static Buffers)
                                        │
      ┌─────────────────────────────────┼─────────────────────────────────┐
      ▼                                 ▼                                 ▼
[ Layer 1: CExoResMan ]          [ Layer 2: CRes2DA ]        [ Layer 3: Miles Streaming ]
(MDL, MDX, TPC, LIP, DLG,        (appearance, heads,         (Dialog VO & TLK/SSF Barks)
 UTC, NCS, SSF, PlaySound WAV)    soundset, portraits)                    │
      │                                 │                                 ▼
      ▼                                 ▼                    Detour InitializeSource
Detour GetKeyEntry Miss          Detour OnResourceServiced     (0x0070CB76 -> 0x0070D939)
  (0x0071244E -> 0x00712462)       (0x00729FD0)              + _AIL_set_file_callbacks@16
Detour ServiceFromImage                 │                                 │
  (0x00713E83 -> 0x00713FA3)            ▼                                 ▼
      │                          Splices virtual V2.b rows   Feeds in-memory audio slice
      ▼                          into live 2DA buffer        (58B VO MP3 / 470B SFX WAV)
Serves 16B-prefixed buffer       Zero Override/*.2da files   Zero StreamVoice/ files
```

### Hook Collision & Shared Interface Rule
- **One Detour Per Address:** KPM refuses duplicate hook addresses at install time (`HookValidator.cs`) and aborts at runtime on overlapping byte windows (`patcher.cpp`). Whenever two mods need the same engine site, move the hook behind a shared KPM provider interface (like `k2.npcworld@1`, `k2.assetvfs@1`, `k2.characterlib@1`).
- **Avoided Collisions:**
  - `CExoResMan::Demand` (`0x00711C20`) and `CExoResMan::Malloc` (`0x00712F30`) are owned by `perf-lab`. `asset-vfs` does **not** hook `0x00711C20` (`CExoResMan::Demand`), `0x007111E0` (`CRes::Demand`), or `0x00711ED0` (`CExoResMan::Exists`).
  - `CExoStreamingSoundSourceInternal::InitializeSource` (`0x0070CAA0`, GOG `0x0063BA40`) has live hooks from `k2-mp3-playback-fix` at `0x0070CDCB` (10 bytes) and `0x0070CF34` (6 bytes). `asset-vfs` hooks earlier at `0x0070CB76` (13 bytes, after `_fclose` of `this+0x13C`): non-VFS streams return `0` and flow untouched into `0x0070CDCB`/`0x0070CF34`, while in-memory VFS streams return `1` and jump to `0x0070D939` (`_AIL_open_stream@12`), bypassing disk `fopen` and `k2-mp3-playback-fix` completely.

### Layer 1: BioWare Odyssey Resource Manager (`CExoResMan`)
- **Hook Sites (Steam Aspyr & GOG Aspyr):**
  - `CExoResMan::GetKeyEntry` (`0x00712360`, GOG `0x0061BCC0`) — hooked at miss fallback `0x0071244E` (`consumed_exit_address = 0x00712462`).
  - `CExoResMan::ServiceFromImage` (`0x00713E80`, GOG `0x0061D7E0`) — hooked at `0x00713E83` (`consumed_exit_address = 0x00713FA3`).
- **Mechanism:**
  - Because `0x0071244E` is only reached when a resource is absent from Override, MOD/SAV, RIM, and BIF tables, 100% of vanilla resource lookups incur zero VFS overhead.
  - On a VFS match, `VFS_OnGetKeyEntryMiss` sets `*ppTable = &fakeTable` (`tableType = 4`) and `*ppEntry = &rec->keyEntry` (`packedId = 0x40FC0000 | idx`, Source Type 1 = `Image`) and returns `1`.
  - When `CExoResMan::Demand` or async `Update` services the resource, Source Type 1 dispatches to `CExoResMan::ServiceFromImage` (`0x00713E83`). `VFS_OnServiceFromImage` copies the payload into a 16-byte-prefixed mutable buffer (satisfying `CResMDL`'s `m_pResource - 6` header write at `0x0093D110` and `CRes2DA`'s `m_pResource - 8` without calling `CExoResMan::Malloc`), sets `pRes->m_nSize` (`+0x18`), `pRes->m_pResource` (`+0x10`), and `pRes->m_status |= 4` (`+0x0C`), and invokes `pRes->OnResourceServiced()` (`vtbl[4]`).
  - When `CExoResMan::FreeResourceData` (`0x007120C0`) later runs, Source Type 1 (`(m_nPackedId >> 30) == 1`) causes the engine to skip `_free(m_pResource)` and simply clear `m_pResource` / `m_nSize`.
- **Layer 1 Audio Contract (Live Tests E1/E2b/E2c):**
  - Script `PlaySound("...")` resolves `ResTypeWav = 4` through Layer 1 (`FlagResMan = 0`) and **requires bare RIFF WAVE PCM**. Bare `.mp3` is never queried by `PlaySound` (E2c), and 58-byte VO MP3 containers play silently on Layer 1 (E2b). Both `VfsRegistry` and `k1_port_vfs_pack.py` reject `.mp3` and 58-byte/470-byte wrapped containers when `flags == FlagResMan`.

### Layer 2: Virtual 2DA Row Interception & Binary Splicing (`CRes2DA`)
- **Hook Site:** `CRes2DA::OnResourceServiced` (`0x00729FD0`).
- **Target Tables:** `appearance.2da`, `heads.2da`, `soundset.2da`, `portraits.2da`.
- **Mechanism:**
  - When the engine loads a target `.2da` from `Override/` or `2da.bif`, `VFS_On2DaServiced` intercepts the buffer before `CRes2DA::OnResourceServiced` sets `m_p2DAHeader`/`m_p2DAData`.
  - `TwoDaSplicer` splices registered virtual rows (with optional `template_row` inheritance) into a newly allocated `2DA V2.b\n` buffer via `gameaddr::ResManFreeData` + `gameaddr::ResManMalloc`.
  - Handles `CSoundSetList` (`0x0085EF00`), `C2DA::GetINTEntry`, and `C2DA::GetCExoStringEntry` natively with zero `Override/*.2da` files.

### Layer 3: Miles AIL Streaming Audio Interception
- **Hook Site:** `CExoStreamingSoundSourceInternal::InitializeSource` (Steam `0x0070CAA0` at `0x0070CB76` -> `0x0070D939`; GOG `0x0063BA40`) + `Mss32.dll` `_AIL_set_file_callbacks@16`.
- **Layer 3 Audio Contract (Live Test E4):**
  - Dialog VO (`StreamVoice/`) and TLK/SSF barks (`StreamSounds/`, e.g. `p_juhani_slct1`) use `FlagMilesStream = 1` and route through `InitializeSource` (`0x0070CAA0`).
  - `inspectAudioContainer` strips the 58-byte StreamVoice header (`riffSz == 50` -> `.mp3` slice) or 470-byte (`0x1D6`) StreamSounds prefix (`-> .wav` RIFF slice), rewrites `this + 0x28` to `#*VFS_<idx>.mp3`/`.wav`, and jumps to `0x0070D939` where `_AIL_open_stream@12` streams directly from `MilesCursorTable` in memory.

---

## 3. Modular Character Library Interface (`k2.characterlib@1`)

Built on top of `kpatch_api.h` and integrated with `npc-world` (`NpcWorld.h`):

```c
/* CharacterProvider.h -- ABI for modular character providers */
#ifndef K2_CHARACTER_PROVIDER_H
#define K2_CHARACTER_PROVIDER_H

#include "kpatch_api.h"
#include <stdint.h>

#define K2_CHARLIB_IFACE   "k2.characterlib"
#define K2_CHARLIB_VERSION 1

#pragma pack(push, 8)

typedef struct K2CharacterDefinition {
    uint32_t struct_size;
    const char* character_id;       /* "juhani", "carth", etc. */
    const char* display_name;       /* "Juhani" */
    const char* default_tag;        /* "k1p_juhani" */
    const char* default_utc;        /* "k1p_juhani" */
    const char* default_dlg;        /* "kvjdlg01" */
    uint32_t appearance_id;         /* Virtual 2DA appearance row (e.g. 818) */
    uint32_t head_id;               /* Virtual 2DA heads row (e.g. 250) */
    uint32_t soundset_id;           /* Virtual soundset row (e.g. 70) */
} K2CharacterDefinition;

typedef struct K2CharacterLibApi {
    uint32_t struct_size;
    /* Register a character definition into the shared library */
    int32_t (*RegisterCharacter)(const K2CharacterDefinition* def);
    /* Query if a character provider is active */
    int32_t (*HasCharacter)(const char* character_id);
    /* Spawn a registered character into the active area through npc-world */
    uint32_t (*Spawn)(const char* character_id, const char* area_resref, 
                      float x, float y, float z, float facingRadians);
} K2CharacterLibApi;

#pragma pack(pop)

#endif /* K2_CHARACTER_PROVIDER_H */
```

---

## 4. Master Plan & Roadmap

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **VFS-R1** | Reverse-engineer `CExoResMan::Demand` & `Exists` calling conventions on Steam Aspyr | - | - | DONE | `2026-10-16` | Document signatures & register in Ghidra DB |
| **VFS-R2** | Reverse-engineer `StreamingSource::OpenStream` (`0x0070caa0`) Miles memory playback | - | - | DONE | `2026-10-16` | Prototype memory stream handle detour |
| **VFS-C1** | Pure logic VFS registry + host unit tests (`run_tests.py vfs_registry`) | VFS-R1 | - | DONE | `2026-10-17` | VfsRegistry.h + vfs_registry_test.cpp (151/151 PASS) |
| **VFS-C2** | In-memory 2DA row injector hook (`appearance.2da`, `heads.2da`, `soundset.2da`) | VFS-C1 | - | DONE | | TwoDaSplicer.h + twoda_splicer_test.cpp (202/202 PASS) |
| **VFS-C3** | Character Provider API header (`CharacterProvider.h`) & `k2.characterlib@1` | VFS-C1 | - | DONE | | CharacterProvider.h + character_provider_test.cpp (127/127 PASS) |
| **VFS-P1** | Juhani asset packing tool: embed models, textures, lips, dialog, audio into DLL | VFS-C2 | - | DONE | | k1_port_vfs_pack.py + test_k1_port_vfs_pack.py (8/8 PASS) |
| **VFS-T1** | In-game verification of Juhani spawned in Telos `204tel` with 0 disk files | VFS-P1, VFS-R2 | user GO | TODO | | Verify clean exit (0 disk files in game dir) |

---

## 5. Verification & Safety Invariants

1. **Zero-Disk-Write Invariant:** Running `git status` or `ls` on the game's `Override/`, `StreamVoice/`, and `StreamSounds/` directories before launch and after exit MUST show identical directory state with 0 modified or new files.
2. **Passthrough Invariant:** Any resource or 2DA row not owned by an active character provider MUST pass cleanly to the engine's original handlers without latency or alteration.
3. **Crash Durability:** If a character asset is requested but corrupt or missing from memory, the VFS detour must fail gracefully (returning `NULL` / `FALSE`) rather than dereferencing invalid memory.
