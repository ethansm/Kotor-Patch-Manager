# R13c / c22 Card: In-World Left-Hand Holster Placement & kWorldKnowsHand Activation

**Date:** 2026-10-16  
**Status:** Recon complete, ready for worker implementation.  
**Sources:**  
- `.claude/research/holster_edit/r6_hand_hook_design.md` (detours H8/H9 at `0x763B49` / `0x763E7B`)  
- `.claude/research/holster_edit/prb1_hand_log.md` (live validation: 11 gobs, R7/L4, 100% consistent)  
- `patch_manager_mods/holster_src/Holster.cpp` (`kWorldKnowsHand`, `effLeft`, `previewPlace`, `HolsterPlace`, `s_cache`)  
- `patch_manager_mods/holster_src/HolsterHand.h` (`HandRing`, `SeenSet`)  
- `patch_manager_mods/holster_src/HolsterTune.h` (`resolveFull`, `mirrorLeft`, `mirrorRot`, `mirrorOff`, `mirrorBone`)  

---

## 1. Executive Summary & Context

- **Review F1 Problem:** In the world, both hands share the `bone_name` attach node (e.g. `pelvis_g` under `bone_test=1`), so `HolsterPlace` could not distinguish whether a weapon Gob belonged to the right or left hand. Consequently, `kWorldKnowsHand` was set to `false`, `effLeft` forced all preview weapons to right-hand values, and `HolsterPlace` hardcoded `ent->left = false`.
- **R6 / PRB1 Breakthrough:** R6 designed entry-point detours at the engine's in-world attach call site `FUN_007637E0`:
  - `0x00763B49` (`OnHandAttachR`) captures `(HandR, gob)`.
  - `0x00763E7B` (`OnHandAttachL`) captures `(HandL, gob)`.
- **PRB1 Live Result:** Frozen log `holster_log.txt` (1495 lines) proved 100% mapping consistency across 11 distinct weapon gobs (7 R, 4 L). Every placed Gob was registered in `g_handRing` *before* `HolsterPlace` processed it.
- **Card c22 Mission:** Turn on `kWorldKnowsHand = true`, wire `hs::handLookup(g_handRing, gob)` into `HolsterPlace`'s cache resolution (`s_cache`), fix the `g_onAttach` gating defect in `handAttach`, and enable full mirrored/custom left-hand placement in world and editor.

---

## 2. Investigation Findings

### 2.1 Where to Record the Hand per Gob
- **Hooks:** Detour hooks H8 and H9 in `patch_manager_mods/holster_src/kotor2-steam-aspyr.hooks.toml`:
  - Right hand: `0x00763B49` (`OnHandAttachR`), stolen bytes `8B 42 7C FF D0` (`MOV EAX,[EDX+0x7C]; CALL EAX`), resume `0x00763B4E`.
    - Stack/regs: `[ebp-0x2c]` = weapon Gob, `[ebp-0x40]` = item object, `ecx` = body anim object.
  - Left hand: `0x00763E7B` (`OnHandAttachL`), stolen bytes `8B 42 7C FF D0`, resume `0x00763E80`.
    - Stack/regs: `[ebp-0x2c]` = weapon Gob, `[ebp-0x30]` = item object, `ecx` = body anim object.
- **Storage:** `g_handRing` (`hs::HandRing`, `HolsterHand.h`), 16-slot circular buffer of `(uintptr_t gob, int hand)`.
- **CRITICAL DEFECT TO FIX in `Holster.cpp:448`:**
  ```cpp
  static void handAttach(int hand, void* gob, void* item, void* body) {
      if (!g_onAttach) return; // <-- DEFECT: Gated on probe && ch_attach!
      saferead::beginScope();
      const uintptr_t g = reinterpret_cast<uintptr_t>(gob);
      (void)hs::handRecord(g_handRing, g, hand);
      ...
  ```
  If `probe=0` or `ch_attach=0` in `holster.ini`, `g_handRing` is NEVER populated!
  **Fix:** `(void)hs::handRecord(g_handRing, g, hand);` must execute unconditionally on line 1 of `handAttach`. Only the logging code (`HANDATTACH`) is gated by `if (!g_onAttach) return;`.

### 2.2 How HolsterPlace Reads It
- **Location:** `HolsterPlace(void* gob, void* beh)` (`Holster.cpp:716`).
- **Cache miss path (`!ent`):** Line 758 currently hardcodes:
  ```cpp
  ent->left = false; // <-- HARDCODED RIGHT HAND
  ```
  **Fix:** Replace with:
  ```cpp
  const int hand = hs::handLookup(g_handRing, g);
  ent->left = kWorldKnowsHand && (hand == hs::HandL);
  ```
- **Opportunistic Cache Refresh:** If `ent` is already cached with `!ent->left` (e.g. if created before attach, or edge-case reuse), check `g_handRing`:
  ```cpp
  if (kWorldKnowsHand && !ent->left) {
      const int hand = hs::handLookup(g_handRing, g);
      if (hand == hs::HandL) {
          ent->left = true;
          ent->seq = -1; // force hs::resolveFull re-run
      }
  }
  ```
- **How `resolveFull` uses `ent->left`:**
  - `hs::resolveFull(s_tune, ent->mname, cat, ent->body, ent->left, false, ent->r);` (`HolsterTune.h:287-338`).
  - When `ent->left == true`, it evaluates:
    `left-model > model (mirrored) > left-type > type (mirrored) > global (mirrored)`.
  - Body delta X is negated on the left (`mirrorOff(d)`).
  - Rotations are reflected across sagittal plane (`mirrorRot`: `rot[1] = -rot[1]; rot[2] = -rot[2];`).
  - Bones swap `r_thigh` (4) <-> `l_thigh` (5) via `mirrorBone`.
- **The PRB1 `PLACEREFUSE reason=nottarget` Mystery Solved:**
  - In PRB1, all 4 left-hand gobs had `PLACEREFUSE reason=nottarget` (e.g. `gob=b17c570 n=35 node=... target=0`).
  - **Cause:** `holster.ini` had `bone_test_left=0`. `applyBoneTest()` only patched `rhand` -> `pelvis_g` and left `lhand` pointing at engine literal `"lhand"`. Thus `nodeIsTarget` (`_stricmp(ni.name, g_boneName) == 0`) returned `false` for left weapons!
  - `kWorldKnowsHand` was not responsible for the `nottarget` refusal. Setting `bone_test_left=1` in `holster.ini` patches `lhand` -> `pelvis_g`, making `nodeIsTarget` return `true` for left weapons. Once `target=1`, `ent->left` correctly directs placement to the left-hand rule chain.

### 2.3 What Flips `kWorldKnowsHand`
- `Holster.cpp:630`: Change `static const bool kWorldKnowsHand = false;` to:
  ```cpp
  static const bool kWorldKnowsHand = true;
  ```
- `effLeft(bool left)` (`Holster.cpp:631-636`):
  ```cpp
  static bool effLeft(bool left) {
      return kWorldKnowsHand && left;
  }
  ```
  When `kWorldKnowsHand` is `true`, `effLeft(left)` returns `left` directly.
- **Unlocks Across the Subsystem:**
  1. `previewPlace`: `left = effLeft(e.left)` preserves `left = true` for offhand preview weapons.
  2. `apiBeginEdit`: `left = effLeft(...)` preserves `left = true` when editing offhand preview weapons.
  3. `apiSetScope`: `edReseed` preserves `left`.
  4. `edWrite`: Writes left-hand keys (`place_*_l_*` and `[l_]<type>`) to `holster.ini`.
  5. `HolsterPlace`: In-world left weapon placement activates.

### 2.4 Files c22 Touches
1. `patch_manager_mods/holster_src/Holster.cpp` (core logic changes)
2. `patch_manager_mods/holster_src/HolsterHand.h` (optional: increase `kCap` from 16 to 32 or keep at 16)
3. `patch_manager_mods/holster_src/holster.ini` (documentation / defaults for `bone_test_left=1`)
4. `patch_manager_mods/holster_src/holster_hand_test.cpp` (unit tests for hand ring and left resolution)
5. `patch_manager_mods/PLAY_TEST_QUEUE.md` (play test row for live dual-wielding placement)

---

## 3. Test Vectors & Matrix

### 3.1 Host Unit Test Matrix (`holster_hand_test.cpp` & `holster_preview_test.cpp`)
| Vector ID | Scenario | Input / State | Expected Output |
|---|---|---|---|
| **T-HAND-1** | Interleaved R/L attaches | 7 R gobs, 4 L gobs (PRB1 trace) | Every gob returns exact recorded hand; no cross-talk |
| **T-HAND-2** | Hand swap | Gob 0x1000 attached R, then attached L | `handLookup` updates in place to `HandL`, count remains 1 |
| **T-HAND-3** | Unknown Gob | Gob never passed to `OnHandAttach` | `handLookup` returns `HandNone` (0) |
| **T-HAND-4** | effLeft with kWorldKnowsHand=true | `effLeft(true)`, `effLeft(false)` | Returns `true` and `false` respectively |
| **T-HAND-5** | Mirrored Placement | Right global rot=(10, 20, 30), off=(0.2, 0.3, 0.4) | Left resolves to rot=(10, -20, -30), off=(-0.2, 0.3, 0.4) |
| **T-HAND-6** | Explicit Left Override | Specific `l_w_SonicPstl_002` or `l_blaster_pistol` set | Left resolves to explicit values without mirroring |
| **T-HAND-7** | Thigh Bone Mirror | Bone set to `r_thigh` (4) on right | Left resolves to `l_thigh` (5) |

### 3.2 In-Game Live Verification Vectors (`PLAY_TEST_QUEUE.md`)
- **Setup:** PC or party member dual-wielding (e.g. two blaster pistols or two vibroblades).
- **Config:** `holster.ini` with `probe=1`, `bone_test=1`, `bone_test_left=1`, `place=1`, `ch_attach=1`, `ch_place=1`.
- **Expected Logs:**
  1. `HANDATTACH hand=R gob=<G1> ...` and `HANDATTACH hand=L gob=<G2> ...`
  2. `HANDMAP g=<G1> hand=R ...` and `HANDMAP g=<G2> hand=L ...`
  3. `PLACENODE ... name=pelvis_g target=1 ... left=0` (G1) and `... left=1` (G2)
  4. `PLACE ... gob=<G1> ... left=0` and `PLACE ... gob=<G2> ... left=1`
  5. Both weapons visible on character's hips/back with mirrored symmetry.

---

## 4. Paste-Ready c22 Worker Brief

```markdown
### Card c22 Brief: In-World Left-Hand Holster Placement & kWorldKnowsHand Activation

#### Contract
- Role: worker
- Write Scope:
  - `patch_manager_mods/holster_src/Holster.cpp`
  - `patch_manager_mods/holster_src/HolsterHand.h`
  - `patch_manager_mods/holster_src/holster.ini`
  - `patch_manager_mods/holster_src/holster_hand_test.cpp`
  - `patch_manager_mods/PLAY_TEST_QUEUE.md`

#### Pillar 1: Exact Code Changes
1. `patch_manager_mods/holster_src/Holster.cpp`:
   - Line 447-452 (`handAttach`): Move `hs::handRecord(g_handRing, g, hand);` outside and before `if (!g_onAttach) return;`.
     ```cpp
     static void handAttach(int hand, void* gob, void* item, void* body) {
         const uintptr_t g = reinterpret_cast<uintptr_t>(gob);
         (void)hs::handRecord(g_handRing, g, hand);
         if (!g_onAttach) return;
         saferead::beginScope();
         ...
     ```
   - Line 630: Flip `static const bool kWorldKnowsHand = true;`.
   - Line 631-636 (`effLeft`): Simplify to `return kWorldKnowsHand && left;`.
   - Line 758 (in `HolsterPlace` cache fill `!ent`):
     ```cpp
     const int hand = hs::handLookup(g_handRing, g);
     ent->left = kWorldKnowsHand && (hand == hs::HandL);
     ```
   - Line 745 (in `HolsterPlace` cache hit): Add opportunistic refresh:
     ```cpp
     if (kWorldKnowsHand && ent && !ent->left) {
         if (hs::handLookup(g_handRing, g) == hs::HandL) {
             ent->left = true;
             ent->seq = -1;
         }
     }
     ```
   - Line 778 (`PLACENODE` log) & Line 812 (`PLACE` log): Add `left=%d` to format and arguments.
2. `patch_manager_mods/holster_src/holster.ini`:
   - Document `bone_test_left=1` for testing in-world off-hand weapons on `bone_name`.
3. `patch_manager_mods/holster_src/holster_hand_test.cpp`:
   - Add unit tests for `effLeft` and multi-gob R/L interleaved lookup.

#### Pillar 2: Edge Cases & Test Matrix
- Unknown gob -> `handLookup` returns `HandNone` -> `left = false`.
- Hand swap -> `OnHandAttach` overwrites slot in place.
- Disabled `probe` / `ch_attach` -> Hand recording still functions.
- Dual-wielding -> Both weapons placed on target bone with mirrored coordinates.

#### Pillar 3: Tooling & Verification Commands
- Unit tests: `python3 scripts/run_tests.py holster` (all 11 suites must PASS).
- Single suite: `python3 scripts/run_tests.py holster_hand`.
- Gate check: `pre_install_gate.sh --mod holster_src --skip-default`.
- Prepare: `python3 scripts/kpm_hotswap.py --mod holster --prepare`.
- Full gate: `pre_install_gate.sh --mod holster_src`.

#### Pillar 4: Documentation & Contract
- Output 5-line WORKER contract upon completion:
  CREATED: ...
  MODIFIED: patch_manager_mods/holster_src/Holster.cpp, ...
  TESTS: python3 scripts/run_tests.py holster (PASS 11/11)
  NOTES: kWorldKnowsHand activated, in-world left hand placed via g_handRing
  STATUS: PASS
```
