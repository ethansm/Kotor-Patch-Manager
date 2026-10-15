1. **patch_manager_mods/_shared_src/hudkit_test.cpp — `main**`

* **Anchor:** `KT_CHECK(frame(false) == nullptr && frame(true) == nullptr && cancel(false) == nullptr && disc() == nullptr);`

* **What is wrong:** `hudkit::ensure()` is one of the two primary entry points of `HudKit.h` (`inline bool ensure() { return embtex::ensure(detail::kHudKitTextures, Count); }`), yet `hudkit_test.cpp` never calls `hudkit::ensure()` at all. Moreover, because `hudkit_test.cpp` calls `frame(false)`, `frame(true)`, `cancel(false)`, and `disc()` *first* while `guikit::g_gk.loadImage = nullptr` and then sets `guikit::g_gk.loadImage = fakeLoad` and expects `frame(false)` to succeed on the second call, it also hides the interaction with `guikit::imageFor` (where `GuiKit.h` only skips caching when `g_gk.loadImage == nullptr`, whereas a non-null `loadImage` returning `nullptr`—such as `cancel(true)`—permanently caches `nullptr` inside `guikit::imageFor`'s pool so `cancel(true)` can never recover even after `ensure()` succeeds). If `hudkit::ensure()` passed `0` instead of `Count` or returned `false`, `hudkit_test.cpp` could not fail.


* **Suggested fix:** Set up fake `embtex::g_et` callbacks in `hudkit_test.cpp`, call `KT_CHECK(hudkit::ensure());`, and verify that `embtex::slotCount() == hudkit::Count` and all five `kHudKitTextures` are registered and filled. Also test calling `cancel(true)` after the loader begins returning a non-null handle to catch the `guikit::imageFor` negative-caching bug.


* **Confidence:** certain


2. **patch_manager_mods/_shared_src/gui2d_test.cpp — `main` (and `installFakes`)**

* **Anchor:** `static const float kNoFill[4] = {}; g_g2d.clipNoFill = kNoFill;   // distinguishable from the engine constant`

* **What is wrong:** `gui2d_test.cpp` immediately overwrites all 8 function pointers and all 3 data members (`flushArgAddr`, `clipDepthAddr`, `clipNoFill`) of `gui2d::g_g2d` before running a single check, and never tests the `k2addr::UseTable` / `K2AddrTable` resolution paths in `Gui2D.h`:


1. `pushClip` has an explicit safety guard: `const void* c = color ? color : clipNoFill(); if (!c) return 0;` for when `g_g2d.clipNoFill == nullptr` and the active `K2AddrTable` has `ClipNoFillColor == 0` (or `k2addr::get() == nullptr`). Because `gui2d_test.cpp` permanently sets `g_g2d.clipNoFill = kNoFill`, this `if (!c) return 0;` branch is never executed; deleting it cannot make the test fail.


2. `flushArg()` and `clipDepthRaw()` have `k2addr::UseTable` branches that read `k2addr::addr(&K2AddrTable::FlushArg)` and `k2addr::addr(&K2AddrTable::ClipDepth)`, and `Engine`'s default lambdas (`begin`, `end`, `push`, `pop`, `flush`, `hudScale`, `panelRect`, `draw`) dispatch through `k2addr::fnptr` / `k2addr::off(&K2AddrTable::ImageVt_Draw)`. None of these default `K2AddrTable`-backed paths (including `ImageVt_Draw <= 0` or `k2addr::setForTest(nullptr)`) are ever exercised in `gui2d_test.cpp`.




* **Suggested fix:** Add a test section in `gui2d_test.cpp` that restores default `Engine{}` (with `g_g2d.clipNoFill = nullptr`, `flushArgAddr = k2addr::UseTable`, `clipDepthAddr = k2addr::UseTable`), verifies that `k2addr::setForTest(nullptr)` makes `pushClip`, `flushArg`, `clipDepthRaw`, and `drawImage` safely no-op/return `0`, and verifies that a custom `K2AddrTable` pointing `FlushArg`, `ClipDepth`, and `ImageVt_Draw` into the test arena is honored.


* **Confidence:** certain


3. **patch_manager_mods/_shared_src/embtex_test.cpp — `main` (and `EmbedTex.h` — `embtex::heal`)**

* **Anchor:** `for (int i = 0; i < 25; ++i) { simGlReset(b); heal(); }`

* **What is wrong:** `embtex_test.cpp` only tests the `MaxHeals` give-up message by running a loop of 25 consecutive resets (`for (int i = 0; i < 25; ++i) { simGlReset(b); heal(); }`) and asserting `countLines("tex emb_a: giving up after 20 failed refills") == 1` at the end of the loop. It never checks *when* that log line is emitted (`i == 19`, the 20th reset) or whether the 20th `detail::fill` actually succeeded! In `EmbedTex.h::heal()`, `++s.heals` is incremented *before* `detail::fill`, and `if (s.heals == MaxHeals) EMBTEX_LOG("tex %s: giving up after %d failed refills ... art missing, labels still work", ...)` runs immediately after `detail::fill` in the same iteration. Thus, on the 20th reset (`i == 19`), `detail::fill` **succeeds** (`b[Tex_Constructed] == 1`, `pixels != nullptr`), yet `heal()` immediately logs `"giving up after 20 failed refills (built=1 missing=0 pixels=0x...); art missing"`! Furthermore, `embtex_test.cpp` never tests `heal()`'s `w != s.w` rebuild condition (`else if (built && !px && (b[Tex_Missing] || w != s.w))` when `b[Tex_Missing] == 0` and `at<int>(b, Tex_W)` changed), nor `ensure()` re-called on a slot whose initial `newFn` returned `nullptr` (`if (k >= 0) { if (!slots()[k].basic) all = false; continue; }`), nor `newArrFn` returning `nullptr`.


* **Suggested fix:** Fix `EmbedTex.h::heal()` so the give-up log only fires when `detail::fill` fails or when a 21st reset is seen before an upload, and add checks in `embtex_test.cpp` for: (a) no give-up log after 20 *successful* refills until a 21st reset occurs, (b) `b[Tex_Missing] == 0 && at<int>(b, Tex_W) != s.w` triggering `resetFn` + refill, and (c) `newFn` / `newArrFn` returning `nullptr`.


* **Confidence:** certain


4. **patch_manager_mods/_shared_src/actqueue_test.cpp — `main**`

* **Anchor:** `// playerPaused() walks the fixed engine address AppGlobal (not mappable in the host arena), so it is not host-testable; QueueHud's tests keep the behaviour pinned.`

* **What is wrong:**
1. The comment in `actqueue_test.cpp` claims `QueueHud`'s tests keep `actq::playerPaused()` pinned, which is false (`QueueHud` has its own `readPaused()` and never calls `actq::playerPaused()`, as seen in bundle 2a/2b). Because `actq::playerPaused` reads hardcoded `gameaddr::AppGlobal` (`0x00A1B4A4`) instead of an injectable address or `k2addr::addr(&K2AddrTable::AppGlobal)`, `playerPaused` is completely untested across the entire test suite; mutating any of its offsets (`+8`, `+4`, `PauseInternal = 0x10078`) or bitmask (`& 2`) cannot fail any test.


2. In `actq::read()`, the truncation bounds `kMaxEntries` (16) and `kMaxRound` (4) are never tested in `actqueue_test.cpp` (the test only builds a 2-entry action queue and a 1-entry round list), nor is `s.rn[0].spellId` checked (`put<uint32_t>(rd1 + RnSpell, 3)` is written in `main()`, but `s.rn[0].spellId` is never asserted, while `curN + RnItem` and `curN + RnFeat` are never written or checked).




* **Suggested fix:** Allow `actq::playerPaused` to read `AppGlobal` through `k2addr` or an injectable `g_appGlobal` (like `Party.h` and `ItemRead.h` do) and test the 4-step pointer chain and bit 1 (`& 2`) in `actqueue_test.cpp`; also assert `KT_CHECK_EQ((int)s.rn[0].spellId, 3);` and test lists longer than `kMaxEntries` (16) and `kMaxRound` (4).


* **Confidence:** certain


5. **patch_manager_mods/_shared_src/filewatch_test.cpp — `test_poll` and `test_stamp**`

* **Anchor:** `KT_CHECK(changed(p, f.c_str(), 2000));      // missing -> present`

* **What is wrong:**
1. `filewatch::changed(ReloadPoll& p, const char* path, unsigned nowMs, bool force = false, Stamp* out = nullptr)` has an optional 5th parameter `Stamp* out` (`if (out) *out = s;`), which receives the newly read stamp whenever a check runs. Not a single call in `filewatch_test.cpp` ever passes a non-null `out` pointer. If `if (out) *out = s;` were deleted from `FileWatch.h`, `filewatch_test.cpp` could not fail.


2. In `test_stamp()`, when the host filesystem does not support nanosecond timestamps (or truncates them), the `if (n1.mtime % 1000000000LL != 123456789LL)` branch skips the only assertion that tests `Stamp::mtimeMs()` on an existing file (`KT_CHECK_EQ(n1.mtimeMs(), 1700000000123LL)`), whereas a whole-second check (`t1.mtimeMs() == 1700000000000LL`) works on every filesystem.




* **Suggested fix:** Pass a `Stamp out` to `changed(p, f.c_str(), 2000, false, &out)` in `test_poll()` and assert `KT_CHECK(out.exists && out.size == 3ull && out == p.last);`, and assert `KT_CHECK_EQ(t1.mtimeMs(), 1700000000000LL);` unconditionally in `test_stamp()`.


* **Confidence:** certain


6. **patch_manager_mods/_shared_src/guitip_test.cpp — `main**`

* **Anchor:** `if (!guikit::detail::rd32(mgr, Mgr_Over, &over) || !guikit::detail::rd32(mgr, Mgr_Owner, &owner) || !guikit::detail::rd<float>(mgr, Mgr_Timer, &m.timer) ||`

* **What is wrong:**
1. In `GuiTip.h::setNative`, `if (!guikit::detail::rd32(ctl, Ctl_TipText, &probe) || !guikit::detail::wr<int>(ctl, Ctl_TipStrRef, 0)) return false;` checks that both `ctl + Ctl_TipText` (`+0x2C`) and `ctl + Ctl_TipStrRef` (`+0x28`) are accessible before calling `exoDtor` / `exoCtor`. `guitip_test.cpp` only tests `setNative(0x10, "x")` where the entire pointer is unmapped (`0x10 < 0x10000`), and never tests a control straddling a page boundary or read-only memory where `Ctl_TipStrRef` (`+0x28`) is valid but `Ctl_TipText` (`+0x2C`) is unreadable.


2. Similarly, in `readMgr(&mg)`, `guitip_test.cpp` only tests `cell` holding `0` (`at<uint32_t>(cell, 0) = 0`) and `guiSoundMgrAddr = 0`, and never tests when `mgr` itself points to an unreadable address (e.g. `at<uint32_t>(cell, 0) = 0x20`) or when `mgr` is only partially readable (failing at `Mgr_Disabled = 0x4C`).


3. In `measure()`, calling `measure("abc", nullptr, &mx)` or `measure("abc", &l, nullptr)` (null out-parameter guards `if (lines) *lines = n; if (maxLen) *maxLen = mx;`) is never tested.




* **Suggested fix:** Add checks in `guitip_test.cpp` for `at<uint32_t>(cell, 0) = 0x20` (`!readMgr(&mg)`), `measure("abc", nullptr, &mx)`, and `measure("abc", &l, nullptr)`.


* **Confidence:** certain


7. **patch_manager_mods/_shared_src/item_read_test.cpp — `main**`

* **Anchor:** `if (!saferead::probeRead(SiteBase, reinterpret_cast<const void*>(e), EntryStride)) break;`

* **What is wrong:** `ItemRead.h::readGroup` stops at the first unreadable entry (`if (!saferead::probeRead(SiteBase, reinterpret_cast<const void*>(e), EntryStride)) break;`) and returns the number of entries read so far, and also handles a full 16-character non-NUL-terminated `Entry_Icon` (`char ib[17] = {0}; memcpy(ib, ..., 16);`). `item_read_test.cpp` never tests:


1. An unreadable entry array pointer (e.g., `base = 0x20`, or an entry array that crosses into an unmapped page after entry 0 so the loop breaks early and returns partial count 1).


2. A negative `count` (`count = -1`) or `count == MaxEntries` (256, the exact upper boundary vs `257`).


3. A full 16-byte non-NUL-terminated icon resref (it only tests a 9-byte NUL-terminated `"i_medpac\x01"`, so if `char ib[17] = {0}` were `char ib[16]` or copied 15 bytes instead of 16, `item_read_test.cpp` would not fail).




* **Suggested fix:** Add test cases in `item_read_test.cpp` for a 16-character icon `"0123456789abcdef"` (verifying `v[1].icon == "0123456789abcdef"`), `count = -1`, `count = 257`, and an unreadable `base` (`0x20`).


* **Confidence:** certain


8. **patch_manager_mods/_shared_src/party_test.cpp — `main**`

* **Anchor:** `static int fakeVital(uintptr_t fn, uintptr_t c, int arg) {`

* **What is wrong:**
1. `party_test.cpp` replaces `party::g_callVital` with `fakeVital` for every single `vitals()` and `members()` test and never exercises `party::callVitalSlot` (the default `g_callVital` implementation that casts `fn` to `short(__thiscall*)(uintptr_t, int)` and sign-extends the 16-bit `short` return value to `int`). If `callVitalSlot` had the wrong return type (e.g. `uint16_t` instead of signed `short`, which would turn negative HP `-5` into `+65531` and make `v.down` false in the real game!) or wrong calling convention, `party_test.cpp` could not fail!


2. As noted in bundle 3a, `party::vitals` hardcodes `SlotMaxHp = 0x98` and `SlotCurHp = 0x9C` instead of reading `K2AddrTable::Creature_VtMaxHp` and `Creature_VtCurHp` via `k2addr::off`, and `party_test.cpp`'s K2Addr section never tests `party::vitals` with `Creature_VtMaxHp = 0`, `Creature_VtCurHp = 0`, or `k2addr::setForTest(nullptr)`.




* **Suggested fix:** On `__i386__` (or with a host-compatible `__thiscall` stub returning `short` `-5` with dirty high bits in `EAX`, e.g., `0x1234FFFB`), call `party::callVitalSlot` directly in `party_test.cpp` and assert it returns `-5` (not `0xFFFB` or `0x1234FFFB`); also test `party::vitals` when `k2addr::setForTest(nullptr)` or `Creature_VtMaxHp == 0`.


* **Confidence:** certain


9. **patch_manager_mods/_shared_src/textfit_test.cpp — `main**`

* **Anchor:** `marqueeText(ctx, "Droid Deflector Mark I", 223.856125f, 7, out, sizeof out, &mo, F10);`

* **What is wrong:** `TextFit.h` has four edge-case branches that `textfit_test.cpp` never exercises:
1. In `fitText`: `while (len > 0 && name[len - 1] == ' ') --len;` strips trailing spaces before appending `"..."` when the cut point lands right after a space, and `while (len > 0 && textPx(...) + dots > maxPx)` can reduce `len` to `0` when `maxPx < dots` (producing `"..."`). Every `fitText` test in `textfit_test.cpp` cuts inside the middle of a word (`"Droid Deflector Mark..."`, `"Droid Deflec..."`, `"Droid Def..."`), never right after a space and never with `maxPx <= 0`.


2. In `marqueeText`: `if (off < 0) off = 0;` and `while (off > 0 && off < len && full[off] == ' ') ++off;` (skipping a space at `full[off]`) are never tested: the test passes `off = 7` on `"Droid Deflector Mark I"`, where index 7 is `'e'` (not `' '`, which is index 5!). Despite the comment `// F10 offset landing on a space`, `off = 7` does not land on a space at all!


3. In `wrapAppendW`: `if (wl > 120) wl = 120;` (clamping a single word longer than 120 chars) and a single first word wider than `maxPx` (`cl == 0` when `wide(cand) > maxPx`) are never tested.




* **Suggested fix:** In `textfit_test.cpp`, test `marqueeText` with `off = 5` (which is `' '` in `"Droid Deflector Mark I"`) and `off = -3`, test `fitText` with a `maxPx` that cuts right after `"Droid "` (verifying `"Droid..."`, not `"Droid ..."`), and test `wrapAppendW` with a word > 120 chars.


* **Confidence:** certain



---

### Files or Areas Not Assessed

* **`patch_manager_mods/_shared_src/GuiKit.h` and `patch_manager_mods/_shared_src/K2Addr.h`:** Omitted from `3c_shared_tests_b.txt` (`manifest-omit, 0 bytes` in `<directory_tree>` and `3c_shared_tests_b.manifest.tsv`), though their contents were visible in earlier session bundles.


* **`patch_manager_mods/_shared_src/HudKitTextures.inc`:** Omitted from `3c_shared_tests_b.txt` (`over max_file_tokens, 391334 bytes`).


* **`MemberFunctionThunk.h` and `VTableOverride.h`:** Listed under `Includes not resolvable inside the repo (files not provided)` for `member_function_thunk_test.cpp` and `vtable_override_test.cpp`. Because the headers under test were not provided in `<documents>`, we could not verify whether `member_function_thunk_test.cpp` or `vtable_override_test.cpp` miss branches in those two headers.