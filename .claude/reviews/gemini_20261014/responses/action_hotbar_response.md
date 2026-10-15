### Step 1: File Integrity Check

* **`1a_action_hotbar_src.txt`**:
* Exact last line: `</review_bundle>`

* Ends with `</review_bundle>`: Yes (not cut off).




* **`1b_action_hotbar_logic_test.txt`**:
* Exact last line: `</review_bundle>`

* Ends with `</review_bundle>`: Yes (not cut off).




* **`1d_action_hotbar_io_tests.txt`**:
* Exact last line: `</review_bundle>`

* Ends with `</review_bundle>`: Yes (not cut off).





---

### Step 2: Code Review by Bundle

## `1a_action_hotbar_src.txt`

### Findings

**1. Unbounded out-of-bounds read in `TwoDA::ParseImpl` when cell strings lack a NUL terminator**

* **File and function:** `patch_manager_mods/action_hotbar_src/TwoDA.h`, `hb::TwoDA::ParseImpl`

* **Anchor:** `row.push_back(std::string(d.c_str() + o));`

* **Problem:** `std::string(const char*)` scans until it encounters a `'\0'` byte. Although `std::string::c_str()` is guaranteed to have a NUL byte at `d[d.size()]` in C++11/17, if a malformed or truncated `override/spells.2da` has a cell offset `o` pointing into a data blob that does not contain a NUL byte before `d.size()`, `std::string(d.c_str() + o)` will sweep the entire remainder of the file buffer into every cell. Worse, if `nrows * cols.size()` is large (e.g., thousands of cells all pointing to offset 0 of a 64 KiB un-terminated blob), this amplifies into hundreds of megabytes of heap allocations and hangs/crashes the 32-bit game process.


* **Suggested fix:** Bound the cell string scan using `memchr` up to `d.size() - o`:
```cpp
const char* start = d.data() + o;
size_t maxLen = d.size() - o;
const void* nul = memchr(start, '\0', maxLen);
if (!nul) { error = "unterminated cell"; return false; }
row.push_back(std::string(start, static_cast<const char*>(nul) - start));

```


* **Confidence:** certain

**2. Unchecked `size_t` multiplication overflow in `TwoDA::ParseImpl` on 32-bit builds**

* **File and function:** `patch_manager_mods/action_hotbar_src/TwoDA.h`, `hb::TwoDA::ParseImpl`

* **Anchor:** `size_t cells = (size_t)nrows * cols.size();`

* **Problem:** The DLL is compiled as a 32-bit binary (`size_t` is 32-bit unsigned). `nrows` is bounded only by `100000`, and `cols.size()` is only bounded by the number of `\t` characters in the file. A crafted or corrupt `spells.2da` with e.g. `nrows = 100000` and `cols.size() = 42950` will overflow `nrows * cols.size()` (or `cells * 2` on the next line `if (p + cells * 2 + 2 > d.size())`) past $2^{32}$, bypassing the bounds check and causing an out-of-bounds read in `rd16(d, offs + (r * cols.size() + c) * 2)`.


* **Suggested fix:** Perform the size calculation in `uint64_t` and cap `cols.size()`:
```cpp
if (cols.size() > 1000) { error = "implausible column count"; return false; }
uint64_t cells = (uint64_t)nrows * (uint64_t)cols.size();
if (cells * 2 + 2 > d.size() - p) { error = "truncated offsets"; return false; }

```


* **Confidence:** certain

**3. Non-NUL-terminated or control-character creature tags corrupt INI syntax**

* **File and function:** `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`, `readCreatureTag`

* **Anchor:** `memcpy(out, reinterpret_cast<const void*>(static_cast<uintptr_t>(str)), len);`

* **Problem:** Unlike `saferead::readExo` (which strips trailing NULs and rejects non-printable characters outside `0x20..0x7E`), `readCreatureTag` copies `len` raw bytes directly from the engine's `CExoString` buffer. As documented in `SafeRead.h` and `ActionHotbar.cpp`, engine `CExoString` lengths often include the trailing NUL, or on a stale/garbage server creature pointer can contain `']'`, `'\n'`, or non-printable bytes. If `tag` contains `']'` or `'\n'`, `characterKeyFor` retains it and `Config::Serialize` writes `[Profile.<tag>]` with a broken section header that fails to round-trip on load.


* **Suggested fix:** Replace the manual read in `readCreatureTag` with `saferead::readExo(S_Misc, cr + Cr_Tag, out, cap, 64, true) > 0`, and additionally reject `']'` and `';'` in `characterKeyFor`.


* **Confidence:** certain

**4. Out-of-bounds read in `SafeRead.h::probeRead` / `readAt` when `base + off` wraps around 32-bit address space**

* **File and function:** `patch_manager_mods/_shared_src/SafeRead.h`, `saferead::readAt`

* **Anchor:** `if (base == 0 || !probeRead(site, reinterpret_cast<const void*>(base + off), sizeof(T))) return false;`

* **Problem:** If `base` is a corrupted high pointer (e.g., `0xFFFFFFF0`) and `off` is positive (e.g., `0x1198` for `Creature_Stats`), `base + off` wraps around 32-bit `uintptr_t` to `0x00001188`. While `0x00001188` is caught by `a < 0x10000`, if `base` is `0xFFFFF000` and `off` is `0x15414` (`Hud_ArrowCycle`), `base + off` wraps to `0x00014414` (`>= 0x10000`), which may lie inside a valid low heap/module mapping. `readAt` does not check for `base + off < base` (when `off > 0`) before passing the wrapped address to `probeRead`.


* **Suggested fix:** Check for pointer addition wrap-around in `readAt` before calling `probeRead`:
```cpp
uintptr_t addr = base + static_cast<intptr_t>(off);
if (base == 0 || (off > 0 && addr < base) || (off < 0 && addr > base)) return false;

```


* **Confidence:** certain

**5. `ensureWheelHook` permanently stops retrying `wheel_subscribe` if `g_guiInput` connects before the game HWND exists**

* **File and function:** `patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`, `ensureWheelHook`

* **Anchor:** `if (g_guiInput.connected) {`

* **Problem:** According to `GuiInput.h`, `WheelSubscribe` returns `<= 0` if the game window is not yet available. In `ensureWheelHook()`, when `g_guiInput.connected` is true, it attempts `g_guiInput.wheel_subscribe(hotbarWheelCb, nullptr)` and returns early whether `tok > 0` or `tok <= 0`. While it does retry on subsequent frames while `subscribed == false`, if `gui-input` is loaded and connected (`g_guiInput.connected == true`) but its `WheelSubscribe` fails (e.g., table full or HWND never found by `gui-input`), `ensureWheelHook` unconditionally `return`s inside `if (g_guiInput.connected)` and never falls back to `WheelHook::install(g_gameWnd, &wheelProc)`.


* **Suggested fix:** Fall back to `WheelHook::install` after a bounded number of failed `wheel_subscribe` attempts when `g_gameWnd` is valid.
* **Confidence:** likely

**6. `HotbarConfig.h::Parse` silently misparses item names that contain ` (` as part of the base name**

* **File and function:** `patch_manager_mods/action_hotbar_src/HotbarConfig.h`, `hb::normalizeItemName`

* **Anchor:** `size_t p = t.find(" (");`

* **Problem:** `normalizeItemName` truncates at the first `" ("` rather than only stripping a trailing stack count or disambiguating properly. If two distinct items share a prefix before `" ("`—for example, modded or localized items named `"Force Crystal (Red)"` and `"Force Crystal (Blue)"`—`normalizeItemName` collapses both to `"Force Crystal"`. Consequently, `hb::Same` treats them as identical, `resolve()` always matches whichever appears first in the HUD group, and the player cannot bind the second one.


* **Suggested fix:** Strip only the trailing `" (<digits>)"` stack count suffix (matching the exact logic of `StackCount`) instead of truncating at the first `" ("`.


* **Confidence:** likely

### Files or Areas Not Assessed

* **`patch_manager_mods/_shared_src/GuiKit.h` & `K2Addr.h**`: Omitted from the bundle (`manifest-omit, 0 bytes` in `<directory_tree>` and `.manifest.tsv`). Could not verify `guikit::*` implementations or `K2AddrTable` definitions.


* **`patch_manager_mods/action_hotbar_src/HotbarTextures.inc`**: Omitted (`over max_file_tokens`).


* **`kpatch_api.h` & `KpLog.h**`: Listed as external includes not provided in the repo.


* **`CHANGELOG.md`, `PULL_REQUEST.md`, `README.md`, `baseline_h3_build26.txt**`: Omitted from the bundle (`noise` / `manifest-omit`).



---

## `1b_action_hotbar_logic_test.txt`

### Findings

**1. Design doc vs. code discrepancy on Character Key format and hooks**

* **File and function:** `patch_manager_mods/07_force_power_hotbar.md` vs. `patch_manager_mods/action_hotbar_src/HotbarLogic.h`, `hb::characterKeyFor`

* **Anchor:** `if (tag && *tag) {`

* **Problem:** The "Current reference -> Layout and controls" section of `07_force_power_hotbar.md` states:
* *"Character key: `obj<hexid>` of the controlled creature; Exile = `objfffffffd`. Invalid id 0xFFFFFFFF/0 = unknown (saves refused) except a one-member party with 0xFFFFFFFF -> `objfffffffd`."*

* *"8 hooks (all Steam Aspyr; table in the README "Hooks"): 0x0072CB63 `CaptureHotbarKey`, 0x0074B3F0 `HotbarHudTick`, ... 0x008FACA0 `HotbarOnNewGame`."*


However, `characterKeyFor` in `HotbarLogic.h` now prioritizes creature tags (`"atton"`, `"kreia"`), then party slot fallbacks (`"slot1"`, `"slot2"`), and maps the full sentinel range `0xFFFFFFF0..0xFFFFFFFE` (even with `partyCount > 1` and `isMember0 == false`) to `"objfffffffd"`. Furthermore, the hooks were moved to `0x0072CB84` and delegated to `hud-host` / `save-events`. The "Current reference" section of the design doc is out of sync with the code.




* **Suggested fix:** Update `07_force_power_hotbar.md` under `### Layout and controls` to document tag/slot keying, the `0xFFFFFFF0..0xFFFFFFFE` Exile sentinel range, and the `hud-host`/`save-events` hook migration.


* **Confidence:** certain

**2. Missing test coverage in `hotbar_logic_test.cpp` for negative `off` in `clampOffset**`

* **File and function:** `patch_manager_mods/action_hotbar_src/HotbarLogic.h`, `hb::clampOffset` and `patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp`, `main`

* **Anchor:** `inline int clampOffset(int off, int maxOff) { return off > maxOff ? maxOff : off; }`

* **Problem:** `clampOffset` only clamps from above (`off > maxOff ? maxOff : off`) and does not clamp negative values to `0`, unlike `scrollClamp` right below it. If `ed.off` ever became negative, `size_t idx = (size_t)ed.off + i` in `edPicker` would wrap around to a huge `size_t`. Neither `clampOffset(-1, 5)` nor negative `cells`/`n` in `pickerMaxOff` are tested in `hotbar_logic_test.cpp`.


* **Suggested fix:** Change `clampOffset` to `return std::max(0, std::min(off, maxOff));` and add a check `KT_CHECK_EQ(clampOffset(-1, 3), 0);` in `hotbar_logic_test.cpp`.


* **Confidence:** certain

**3. Missing test case in `hotbar_logic_test.cpp` for `chooseTarget` integer overflow on large `cur`/`max` HP**

* **File and function:** `patch_manager_mods/action_hotbar_src/hotbar_logic_test.cpp`, `main`

* **Anchor:** `long long a = (long long)c.cur * m[best].max, b = (long long)m[best].cur * c.max;`

* **Problem:** `chooseTarget` accepts `int cur` and `int max` (whereas engine vitals are `short`, `Candidate` stores `int`). If a test or caller passes negative `cur` (e.g., a dead/dying creature whose `down` flag was not set by a custom caller, `cur = -5, max = 100`), `(long long)c.cur * 100 < (long long)thresholdPct * c.max` evaluates to true (`-500 < 5000`), selecting a creature with negative HP as a heal target if `c.down` was false. `hotbar_logic_test.cpp` only tests `cur = -5` when `max = 0`, never `cur < 0` with `max > 0` and `down = false`.


* **Suggested fix:** Guard `chooseTarget` with `if (!c.ok || c.down || !c.eligible || c.cur <= 0 || c.max <= 0) continue;` (since 0 or negative HP in KOTOR is unconscious/dead) and add a test case in `hotbar_logic_test.cpp` with `C(-5, 100, true, false, true)`.


* **Confidence:** likely

### Files or Areas Not Assessed

* None of the files in `1b_action_hotbar_logic_test.manifest.tsv` were omitted from `1b_action_hotbar_logic_test.txt`. However, `testdata/spells_fixture.2da` (loaded at runtime by `hotbar_logic_test.cpp`) is a binary file not embedded in the text bundle.



---

## `1d_action_hotbar_io_tests.txt`

### Findings

**1. `twoda_test.cpp` does not test missing NUL terminators in the 2DA data blob (and `TwoDA::ParseImpl` reads past the logical cell)**

* **File and function:** `patch_manager_mods/action_hotbar_src/twoda_test.cpp`, `main` (testing `TwoDA.h`, `hb::TwoDA::ParseImpl`)


* **Anchor:** `std::string probe = Build2da({"A"}, {{"Q"}}); probe.pop_back();`

* **Problem:** In `twoda_test.cpp`, the test intentionally strips the final NUL byte (`probe.pop_back()`) and asserts that `m.Load` succeeds because `std::string::c_str()` happens to provide an implicit NUL at `d[d.size()]`. However, `twoda_test.cpp` never tests a multi-cell 2DA where an earlier cell's NUL terminator is missing or corrupted, nor does `TwoDA::ParseImpl` validate that a NUL exists within the data blob. If cell 0 lacks a NUL and cell 1 has a valid NUL, cell 0 silently concatenates cell 1's contents; if no NUL exists in a large blob and many cell offsets point into it, `ParseImpl` copies the remainder of `d` for every cell.


* **Suggested fix:** Bound string construction in `TwoDA::ParseImpl` using `memchr` and update `twoda_test.cpp` to test that an unterminated cell is either rejected or bounded safely.


* **Confidence:** certain

**2. `twoda_test.cpp` does not test `nrows * cols.size()` overflow on 32-bit `size_t**`

* **File and function:** `patch_manager_mods/action_hotbar_src/twoda_test.cpp`, `main` (testing `TwoDA.h`, `hb::TwoDA::ParseImpl`)


* **Anchor:** `size_t cells = (size_t)nrows * cols.size();`

* **Problem:** `twoda_test.cpp` tests `nrows = 100001` ("implausible row count") and `nrows = 100000` with 2 columns ("bad row label"), but never tests a high column count combined with a valid `nrows` that overflows 32-bit `size_t` in `nrows * cols.size()` or `cells * 2` when compiled with `-m32`.


* **Suggested fix:** Fix the overflow in `TwoDA::ParseImpl` using `uint64_t` and add a column-count limit test in `twoda_test.cpp` (e.g., 1001 columns).


* **Confidence:** certain

**3. `keybif_test.cpp` misses several error branches in `ReadKeyBifResource**`

* **File and function:** `patch_manager_mods/action_hotbar_src/keybif_test.cpp`, `main` (testing `KeyBif.h`, `hb::ReadKeyBifResource`)


* **Anchor:** `ExpectFail("missing bif", bad + "/bad_res.key", bad, "ghost", "cannot open bif");`

* **Problem:** `keybif_test.cpp` does not exercise several malformed-file branches in `KeyBif.h`:
1. `"key too short"` (a KEY file `< 24` bytes).


2. `"bad bif name size"` (`nameSize == 0` or `nameSize > 260`) and `"empty bif name"` (a BIF name consisting solely of `'\0'` bytes).


3. `"bif name out of range"` (`nameOff + nameSize > key.size`).


4. `"bif too short"` (a BIF file `< 20` bytes).


5. `"resource index ... out of range"`, `"bif var table out of range"`, and `"resource id not in bif"` (when the var table fallback scan completes without finding `resIdx`).
If any of those checks in `KeyBif.h` were mutated or broken, `keybif_test.cpp` would still pass.




* **Suggested fix:** Add unit checks in `keybif_test.cpp` (using an in-memory/temp-dir builder similar to `tlk_test.cpp`) covering short KEY/BIF headers, zero/oversize/all-NUL BIF names, out-of-range `resIdx`, and missing resource IDs in the BIF var table.


* **Confidence:** certain

**4. `tlk_test.cpp` uses `system("rm -rf ...")` instead of `kt::TempDir` and leaks the temp directory on early return**

* **File and function:** `patch_manager_mods/action_hotbar_src/tlk_test.cpp`, `main`

* **Anchor:** `std::string cmd = "rm -rf '" + g_dir + "'";`

* **Problem:** `tlk_test.cpp` manually creates a temp directory with `mkdtemp` and cleans it up at the end of `main()` by shelling out to `system("rm -rf '<g_dir>'")`. If `$TMPDIR` contains a single quote `'`, the shell command breaks or executes unintended tokens; if the test aborts early, the directory is leaked. `TestHarness.h` already provides `kt::TempDir` with RAII recursive cleanup specifically for this purpose (as used in `config_roundtrip_test.cpp`).


* **Suggested fix:** Replace `mkdtemp` and `system("rm -rf ...")` in `tlk_test.cpp` with `kt::TempDir td;` and `g_dir = td.path();`.


* **Confidence:** certain

**5. `twoda_test.cpp` real-data TLK test step has zero assertions (`cannot fail`)**

* **File and function:** `patch_manager_mods/action_hotbar_src/twoda_test.cpp`, `main`

* **Anchor:** `for (int r : {8, 10}) { int n = atoi(t.Get(r, "name").c_str()), d = atoi(t.Get(r, "spelldesc").c_str()); printf("tlk name[%d]='%s'\n desc[%d]='%s'\n", n, k.Get(n).c_str(), d, k.Get(d).c_str()); }`

* **Problem:** When `argv[2]` (`dialog.tlk`) is passed to `twoda_test`, it opens the TLK and prints the resolved strings for rows 8 and 10, but performs no `KT_CHECK` assertions on `k.Open(argv[2])` or on the returned strings `k.Get(n)` / `k.Get(d)` being non-empty. Even if `argv[2]` fails to open or returns empty strings for valid strrefs, `twoda_test` still exits 0.


* **Suggested fix:** Assert `KT_CHECK(k.Open(argv[2]))` when `argc > 2`, and check `KT_CHECK(!k.Get(n).empty())` and `KT_CHECK(!k.Get(d).empty())` inside the loop.


* **Confidence:** certain

### Files or Areas Not Assessed

* **`patch_manager_mods/action_hotbar_src/common_contract_test.cpp`**: Omitted from the bundle (`manifest-omit, 0 bytes` in `<directory_tree>` and `.manifest.tsv`).


* **`patch_manager_mods/action_hotbar_src/HotbarTextures.inc`**: Omitted from the bundle (`over max_file_tokens`).


* **`KpLog.h`**: External header included by `hotbar_log_test.cpp`, not provided in the bundle.


* **Binary fixtures under `testdata/**` (`fixture.tlk`, `spells_fixture.2da`, and `keybif_fixture/**`): Omitted as binary files (`omitted: binary`), though their generator scripts (`make_tlk_fixture.py`, `make_spells_fixture.py`, `make_keybif_fixture.py`) were provided and reviewed.