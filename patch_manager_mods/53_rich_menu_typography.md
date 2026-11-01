# Doc 53: Rich Menu Typography & Multi-Font UI Overhaul (`rich-menu-typography`)

Goal: Replace vanilla KotOR II's monotone, single-font, single-color menu text with the rich hierarchy established in `quick_loot_src` (`Cream` titles and values, `TealDim` property keys and subtitles, `TealText` lore prose, `Amber`/`Red` stat deltas, mixed `dialogfont16x16` / `dialogfont10x10` / `fnt_d16x16` typography, and 2px `TealFrame` divider bars) across all 53 vanilla menus and scrollable description boxes.

---

## 1. Validated Engine Architecture & Constraints (Steam `6A522E71`, GOG `915200`)

### 1.1 Why Vanilla Menus Are Flat (Verified Static RE)
1. **No Inline Color Tags in Vanilla**: `CAurGUIStringInternal::Draw_2` (`0x0047B500`, GOG `0x008D1E10`) does not parse `<cRGB>` tags (`dialog.tlk` has 0 instances across 136,540 entries). It packs a single RGBA dword (`0xAABBGGRR`) from `this + 0x30..0x38` into `[EBP - 0x68]` at `0x0047BEF2`.
2. **Per-Character `glColor4ubv` Call Site Exists at `0x0047C06F`**: Inside the `glBegin(GL_QUADS)` loop (`0x0047B9E0..0x0047CC6E`), `Draw_2` executes `glColor4ubv` (`CALL dword ptr [0x009862E0]`) **per printable character** at `0x0047C06F`, passing `&[EBP - 0xF4]` with `[EBP - 0x10]` holding `const char* curChar` and `[EBP - 0x04]` holding `int lineIdx`.
3. **Forced `'b'` Suffix & High-DPI `d2x` Texture Binding**:
   - `CSWGuiManager::ResolveFontName` (`0x00413CE0`, GOG `0x00910160`) overwrites `[EBP - 2] = 2` at `0x00413D35` and appends `'b'` to every font except `"fnt_console"`. Callers of `guikit::setFont` MUST pass unsuffixed names (`"dialogfont16x16"`, `"dialogfont10x10"`, `"fnt_d16x16"`).
   - `LoadAlternateFonts` (`0x00479B60`) matches `_strstr` against `"dialogfont16x16b"`, `"dialogfont10x10b"`, and `"fnt_d16x16b"`, binding `d2xfont16x16b` or `d2xfnt_d16x16b` into `this + 0x1C` at 3840x1600 (`scalar = 2.0`).
4. **Native Variable-Height Multi-Control Stacking in `CSWGuiListBox`**:
   - Every scrollable description pane (`LB_DESCRIPTION` / `LB_DESC` in Inventory, Equip, Store, Workbench, Abilities, Journal, Options) is a `CSWGuiListBox` (size `0x2F0`) populated via `CSWGuiListBox::AddControls_2` (`0x0041FD90`).
   - When the 5th parameter `bVariableItemHeights == 0`, `CSWGuiListBox::OrganizeControls` (`0x0041EA50`) forces all child controls to the maximum `m_nItemHeight` (`+0x2C4`).
   - When `bVariableItemHeights == 1` (sets bit 8 `0x100` of `listbox + 0x2CC`), `OrganizeControls` dispatches at `0x0041EADF` to `CSWGuiListBox::OrganizeUnequal` (`0x0041F170`), which stacks multiple `CSWGuiLabel` controls vertically using each control's own `extent.h` (`+0x10`)!
5. **K5 Vertical-Centering Vanishing Trap**:
   - In `Draw_2` (`0x0047B6CB` / `0x0047B7CB`), vertical center (`0x10`) or bottom (`0x20`) alignment drops single-line text when `box_height < font_line_height` (`32 px` for `16x16`, `38 px` for `fnt_d16x16` at 3840x1600). All dynamically stacked description labels MUST use top-left alignment (`align = 9`, `guikit::AlignTopLeft`).

---

## 2. Master Plan Phasing (7-Column Schema)

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **RMT0** | Extract shared item/rules parser `_shared_src/ItemProps.h` + pure `RichTextClassifier.h` (`LineSpan`, `Key: Value`, `Header:`, `+Bonus`/`-Penalty`, `{tag}` strip) + colocated `rich_text_classifier_test.cpp` | - | - | DONE | cec2a5bf | Extracted shared ItemProps.h + RichTextClassifier.h + rich_text_classifier_test.cpp (46/46 PASS, Flash review APPROVE) |
| **RMT1** | Engine inline multi-color hook in `CAurGUIStringInternal::Draw_2` (`0x0047C06F`, 6-byte call-site detour) with per-line span cache and INI gate `InlineColoring=1` + contract test `rich_menu_typography_contract_test.cpp` | RMT0 | - | DONE | 71c72ce5 | Draw_2 0x0047C06F 6B call-site detour + INI gate InlineColoring=1 + contract test PASS |
| **RMT2** | Central multi-font description builder (`RichDescBox.h`) hooking `CSWGuiListBox::AddControls_2` (`0x0041FD90`, 6B `55 8B EC 83 EC 14`) with `bAutoDelete=0, bVariableItemHeights=1` across all 28 description listboxes (Inventory, Equip, Store, Upgrade, Crafting, Abilities, Journal, Options) | RMT1 | - | DONE | d0cd3423 | RichDescBox.h + AddControls_2 0x0041FD90 6B hook + rich_desc_box_test.cpp (PASS, Flash review APPROVE) |
| **RMT3** | Static menu header & stat label hierarchy pass (`CSWGuiInGameCharacter::SetStats` `0x0084E6F0` and panel title/caption styling across primary menus) | RMT2 | - | TODO | - | `python3 scripts/run_tests.py rich_menu_typography` |
| **RMT4** | Offline pixel mockup & frame-by-frame simulation (`scripts/rich_menu_mockup.py`) rendering Inventory, Abilities, Journal, and Character panes with `FontWidths.inc` metrics | RMT3 | user review | TODO | - | user visual review of generated PNGs |
| **RMT5** | Shared floating hover card (`_shared_src/ItemHoverCard.h`) ported from `quick_loot_src` to `CSWGuiInGameInventory`, `CSWGuiInGameEquip`, `CSWGuiStore`, and `CSWGuiContainer` | RMT0, RMT2 | - | TODO | - | `python3 scripts/run_tests.py item_hover_card` |
| **RMT6** | Stage & live calibration (`0.1.0-dev`): two-stage gate, `--prepare`, full gate, user GO for `--apply`, live log verification via `kotor2_logs.py check` | RMT4, RMT5 | user GO install | TODO | - | `python3 scripts/kpm_status.py` |
| **RMTR** | Release log tightening (`0.2.0`): gate per-line/per-item diagnostics behind `Verbose=0`, deduplicate state logs, verify `<50` startup lines | RMT6 | user GO install | TODO | - | `python3 scripts/kotor2_logs.py check` |

---

## 3. Pillar 1: Exact Code Changes, Struct Offsets, and Inlined Snippets (Zero-Read Spec)

### 3.1 Target Files & Directory Layout
- `patch_manager_mods/_shared_src/RichTextClassifier.h` (NEW — pure C++17 header, zero Win32/GL deps, host-testable)
- `patch_manager_mods/_shared_src/RichDescBox.h` (NEW — multi-label variable-height `CSWGuiListBox` injector using `GuiKit.h`)
- `patch_manager_mods/rich_menu_typography_src/RichMenuTypography.cpp` (NEW — KPM DLL entry, hooks, and panel dispatchers)
- `patch_manager_mods/rich_menu_typography_src/kotor2-steam-aspyr.hooks.toml` (NEW — KPM hook manifest)
- `patch_manager_mods/rich_menu_typography_src/tests/rich_text_classifier_test.cpp` (NEW — unit & boundary tests)
- `patch_manager_mods/rich_menu_typography_src/tests/rich_menu_typography_contract_test.cpp` (NEW — hook & address contract tests)

### 3.2 Canonical Color Palette & Font Constants (Inlined from `QuickLoot.cpp` & `SetLabelBonusPenaltyColour`)
```cpp
namespace rmt {
    // Float RGB vectors for CSWGuiText::TextColor (0x00417140)
    constexpr float kCream[3]      = { 0.8000f, 0.8000f, 0.6980f }; // Titles, primary values, section headers
    constexpr float kTealDim[3]    = { 0.0663f, 0.4537f, 0.3569f }; // Property labels ("Damage:"), subtitles, footers
    constexpr float kTealText[3]   = { 0.1020f, 0.6980f, 0.5490f }; // Flavor / lore prose body
    constexpr float kTealFrame[3]  = { 0.0510f, 0.3490f, 0.2710f }; // 2px divider bars ("uibit_fill_16g")
    constexpr float kBonusAmber[3] = { 0.9500f, 0.6600f, 0.3000f }; // Positive modifiers (+N), matches 0x00A7FE48
    constexpr float kPenaltyRed[3] = { 0.7400f, 0.1100f, 0.0000f }; // Negative modifiers (-N), matches 0x00A7FDD0

    // Packed 0xAABBGGRR bytes for glColor4ubv at 0x0047C06F (caller alpha byte preserved in bits 24..31)
    constexpr uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        return static_cast<uint32_t>(r) | (static_cast<uint32_t>(g) << 8) |
               (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(a) << 24);
    }
    constexpr uint32_t kRgbCream      = 0x00B2CCCCu; // R=204, G=204, B=178
    constexpr uint32_t kRgbTealDim    = 0x005B7411u; // R=17,  G=116, B=91
    constexpr uint32_t kRgbTealText   = 0x008CB21Au; // R=26,  G=178, B=140
    constexpr uint32_t kRgbBonusAmber = 0x004CA8F2u; // R=242, G=168, B=76
    constexpr uint32_t kRgbPenaltyRed = 0x00001CBDu; // R=189, G=28,  B=0

    // Font ResRefs (NEVER pass trailing 'b'; 0x00413CE0 appends 'b' automatically)
    constexpr const char* kFontTitle = "dialogfont16x16"; // Resolves to dialogfont16x16b -> d2xfont16x16b
    constexpr const char* kFontSub   = "dialogfont10x10"; // Resolves to dialogfont10x10b -> d2xfont16x16b
    constexpr const char* kFontDesc  = "fnt_d16x16";      // Resolves to fnt_d16x16b      -> d2xfnt_d16x16b
}
```

### 3.3 Inlined Struct Offsets (`CAurGUIStringInternal`, `CSWGuiLabel`, `CSWGuiListBox`)
```cpp
// CAurGUIStringInternal (vtable = 0x0098C0CC)
// +0x04: int m_nXOffset       | +0x08: int m_nYOffset
// +0x0C: int m_nBoxWidth      | +0x10: int m_nBoxHeight
// +0x14: const char* m_pszStr | +0x18: CAurFont* m_pFont
// +0x1C: CAurTexture* m_pD2x  | +0x20: CAurTexture* m_pD3x
// +0x30: float m_fColorR      | +0x34: float m_fColorG      | +0x38: float m_fColorB
// +0x3C: int* m_pLineWidths   | +0x40: int m_nLineWidths
// +0x48: int* m_pLineLengths  | +0x4C: int m_nLineLengths   (negative length = hyphenated wrap)
// +0x54: float m_fTextScale   | +0x5A: uint8_t m_nAlign     | +0x5C: float m_fResScalar

// CSWGuiLabel (size = 0x148, vtable = 0x009878BC)
// +0x04..+0x10: int extent[4] (x, y, w, h)
// +0x38: CSWGuiPanel* m_pParentPanel
// +0x48: uint32_t m_nFlags (0x02 = Visible, 0x20 = ClickThrough)
// +0x60: CSWGuiBorder (fill resref via SetFill 0x00414EF0, tint RGB at label + 0x88)
// +0xD8: CSWGuiText   (rendererCAurGUIStringInternal* at label + 0xEC; vtable[2] = GetFormattedTextHeight)
// +0xF0: CSWGuiTextParams (this for SetText 0x00416E30, SetFont 0x00416DE0, SetAlign 0x00416FA0, SetColor 0x00417140)

// CSWGuiListBox (size = 0x2F0, vtable = 0x00987D3C)
// +0x118: int m_nScrollBarWidth (typically 16 GUI units)
// +0x29C..+0x2A8: int clientRect[4] (x, y, w, h; clientWidth at +0x2A4)
// +0x2AC: CSWGuiControl** m_pItems | +0x2B0: int m_nItemCount | +0x2B4: int m_nItemAlloc
// +0x2C4: int m_nItemHeight        | +0x2CC: uint32_t m_nFlags (bit 8 = 0x100 = OrganizeUnequal variable heights)
// +0x2D0: uint8_t m_nPadding       | +0x2DC: CSWGuiControl* m_pProtoItem
```

### 3.4 Hook Site 1: Per-Glyph Inline Color Override in `CAurGUIStringInternal::Draw_2` (`0x0047C06F`)
- **Target Address (Steam):** `0x0047C06F` (inside `CAurGUIStringInternal::Draw_2` at `0x0047B500`)
- **Stolen Bytes (6 bytes):** `FF 15 E0 62 98 00` (`call dword ptr ds:[0x009862e0]` = `glColor4ubv`)
- **Jump Target Safety:** Verified 0 internal jump targets in `0x0047C05F..0x0047C074`. **CRITICAL:** Do NOT exceed 6 bytes starting at `0x0047C06F` because `0x0047C05D` (`EB 16`) jumps directly to `0x0047C075` (`D9 45 D8`).
- **Frame Register Context at `0x0047C06F`:**
  - `EBP` = `Draw_2` stack frame pointer
  - `[EBP - 0x168]` = `CAurGUIStringInternal* self`
  - `[EBP - 0x10]` = `const char* curChar` (points into `[self + 0x14]`)
  - `[EBP - 0x04]` = `int lineIdx` (`0 .. [self + 0x4C] - 1`)
  - `[EBP - 0x68]` = `uint32_t defaultPackedRGBA` (`0xAABBGGRR`, alpha in high byte `[EBP - 0x58]`)
  - `[EBP - 0xF4]` = `uint32_t activePackedRGBA` (whose address `&[EBP - 0xF4]` was just pushed to `[ESP]` by `50 PUSH EAX` at `0x0047C06E`)
- **Trampoline Mechanics:**
  ```cpp
  // Called from naked stub at 0x0047C06F before invoking *(void(__stdcall**)(const uint32_t*))0x009862E0
  extern "C" void __fastcall RmtOverrideGlyphColor(uintptr_t ebp) noexcept {
      auto* self       = *reinterpret_cast<const uint8_t* const*>(ebp - 0x168);
      auto* curChar    = *reinterpret_cast<const char* const*>(ebp - 0x10);
      int   lineIdx    = *reinterpret_cast<const int*>(ebp - 0x04);
      uint32_t baseCol = *reinterpret_cast<const uint32_t*>(ebp - 0x68);
      uint32_t* outCol = reinterpret_cast<uint32_t*>(ebp - 0xF4);
      *outCol = rmt::resolveGlyphColor(self, curChar, lineIdx, baseCol);
  }
  ```
- **Span Classification Rules (`rmt::resolveGlyphColor` in `RichTextClassifier.h`):**
  - Only activates on labels whose base color is the default teal (`R < 0.20f && G > 0.55f && B > 0.40f`), preserving explicit red/amber/gold/white labels untouched.
  - **Rule 1 (`Header:` Line):** A line whose trimmed content ends with `':'` (e.g. `"Attributes Required:"`, `"Special:"`, `"Dantooine:"`) -> entire line colored `kRgbCream` (`(baseCol & 0xFF000000u) | kRgbCream`).
  - **Rule 2 (`Key: Value` Line):** A line containing `": "` within the first 32 characters where the prefix before `':'` has no sentence punctuation (`'.'`, `'!'`, `'?'`) -> characters `0 .. colonPos` colored `kRgbTealDim`; characters after `": "` colored `kRgbCream`, with signed numeric tokens `+N` / `+N%` / `+N-M` colored `kRgbBonusAmber` and `-N` (when preceded by space or `'('`, not hyphenated words or ranges like `2-12`) colored `kRgbPenaltyRed`.
  - **Rule 3 (Prose Body):** Lines without a `Header:` or `Key: Value` pattern retain `baseCol` (`kRgbTealText`).

### 3.5 Hook Site 2: Central Multi-Font Description Builder at `CSWGuiListBox::AddControls_2` (`0x0041FD90`)
- **Target Address (Steam):** `0x0041FD90` (`CSWGuiListBox::AddControls_2`, `__thiscall`, `RET 0x14`)
- **Stolen Bytes (6 bytes):** `55 8B EC 83 EC 14` (`push ebp; mov ebp, esp; sub esp, 0x14`)
- **Why One Hook Covers All 28 Description Panes:**
  All 28 description updaters in `swkotor2.exe` — including Inventory (`0x008A8394`), Equip (`0x008ABA11`), Store (`0x008B7B5D`), Upgrade Mod (`0x008C884D`), Upgrade Item (`0x008C7F1F`), Abilities/Force Powers (`0x008A54BA`), Journal (`0x007FC736`), Workbench Crafting (`0x008D641E`), Lab Station (`0x008DAFB1`, `0x008DDD7F`), and Options (`0x008A2430`, `0x007D3170`, `0x007D4570`) — write the complete description string into a single `CSWGuiLabel` (`vtable == 0x009878BC`) via `TextSet` (`0x00416E30`) and then invoke `CSWGuiListBox::AddControls_2(listbox, &pLabel, 1, 0, 0, 0)`.
- **Interception Filter & Multi-Control Section Stack (`RichDescBox.h`):**
  When `AddControls_2` is called with `count == 1 && bAutoDelete == 0 && bVariableItemHeights == 0` and `*reinterpret_cast<uintptr_t*>(pControls[0]) == 0x009878BC` (`CSWGuiLabel`), `RichDescBox::populate` reads `*(const char**)((uint8_t*)pControls[0] + 0xF0)`, splits the text into up to 6 pooled `CSWGuiLabel*` controls owned per `CSWGuiListBox*`, and forwards to the original `AddControls_2` trampoline with `bAutoDelete = 0` and `bVariableItemHeights = 1`:
  ```cpp
  // Signature: void __thiscall AddControls_2(void* listbox, void** pControls, int count,
  //                                          int bAutoDelete, int bRestoreExtents, int bVariableItemHeights)
  using FnAddControls2 = void(__thiscall*)(void*, void**, int, int, int, int);
  // CRITICAL: bAutoDelete MUST be 0 (bit 1 of listbox+0x2CC = 0) so ClearItems (0x0041D290)
  // only clears pointers and does NOT call delete on pooled labels!
  origAddControls2(listbox, controls, count, /*bAutoDelete=*/0, /*bRestoreExtents=*/0, /*bVariableItemHeights=*/1);
  ```
  - **Row 0 (Header / Title, when present):** `kFontTitle` (`"dialogfont16x16"`), `kCream`, `align = 9` (`TopLeft`), `extent.h = formattedHeight + 4`.
  - **Row 1 (Subtitle / Category / Planet):** `kFontSub` (`"dialogfont10x10"`), `kTealDim`, `align = 9`, `extent.h = formattedHeight + 4`.
  - **Row 2 (Top Divider Bar):** 2px tall `CSWGuiLabel` (`extent.h = 2`), empty text, fill `"uibit_fill_16g"`, border/fill tint `kTealFrame`.
  - **Row 3 (Rules / Properties Block):** `kFontSub` (`"dialogfont10x10"`), base color `kTealText` (inline-colored per span by Hook Site 1 into `kTealDim` keys + `kCream` values + `kBonusAmber`/`kPenaltyRed` modifiers), `align = 9`, `extent.h = formattedHeight + 6`.
  - **Row 4 (Bottom Divider Bar, when both rules and prose exist):** 2px tall `CSWGuiLabel` (`extent.h = 2`), fill `"uibit_fill_16g"`, tint `kTealFrame`.
  - **Row 5 (Lore / Flavor Prose Body):** `kFontDesc` (`"fnt_d16x16"`) or `kFontSub` (`"dialogfont10x10"`, configurable via `BodyFont` INI), `kTealText`, `align = 9`, `extent.h = formattedHeight + 8`.

---

## 4. Pillar 2: Edge Cases, Invariants & Unit Test Matrix

### 4.1 Critical Edge Cases & Safety Guards
1. **Jump Target at `0x0047C075`:** The `Draw_2` hook at `0x0047C06F` MUST be exactly 6 bytes (`FF 15 E0 62 98 00`). Any 7+ byte patch overwrites `0x0047C075`, breaking the `EB 16` short jump from `0x0047C05D`. Enforced by static assertion in `rich_menu_typography_contract_test.cpp`.
2. **`K5` Vertical-Centering Vanishing Prevention:** Every `CSWGuiLabel` pushed into `AddControls_2(..., bVariableItemHeights=1)` MUST set `guikit::setAlign(lbl, 9)` (`TopLeft`). Never use `17` or `18` (`Center`) on 2px divider labels or short single-line labels.
3. **`CSWGuiListBox::ClearItems` Ownership (`bAutoDelete = 0`):** In `AddControls_2` (`0x0041FDA8`), parameter 3 (`[EBP + 0x10]`) sets bit 1 (`0x2`) of `listbox + 0x2CC`. In `ClearItems` (`0x0041D29C`), if bit 1 is `1`, it calls `vtable[0]` (`delete`) on every item. Callers MUST pass `bAutoDelete = 0` so `ClearItems` never deletes the panel's native label or our pooled section labels.
4. **Dice Range vs. Negative Penalty Disambiguation:** In `RichTextClassifier.h`, `"Damage: 2-12"` or `"1-10 Physical"` contains `'-'` between digits. The classifier MUST only color `-N` as `kRgbPenaltyRed` when `'-'` is preceded by whitespace, `'('`, or `':'` (e.g. `"Attack Modifier: -2"`, `"Defense: -4 (Makashi)"`), NEVER inside `"2-12"` or hyphenated words (`"Light-Scan"`).
5. **Wrapped Continuation Lines of `Key: Value` Rules:** In `CAurGUIStringInternal`, a long `Key: Value` line that wraps across 2 visual lines has `'\n'` only at the paragraph break, while `m_pLineLengths` (`[this + 0x48]`) splits it into visual line `i` and `i + 1`. `RichTextClassifier.h` scans backward from `curChar` to the nearest `'\n'` (or start of `m_pszString` `[this + 0x14]`), so continuation lines after `": "` automatically inherit the `Value` (`kRgbCream`) span state!
6. **Pause / Unpause, Resolution Change, and Save / Load:** Pooled section labels are keyed by `CSWGuiListBox*` and validated against `listbox->m_pParentPanel` (`+0x38`); if a panel is reconstructed on resolution change, stale label pointers are discarded safely.

### 4.2 Unit Test Matrix (`rich_text_classifier_test.cpp` & `rich_desc_box_test.cpp`)
| Test Vector | Input String / Scenario | Expected Output |
|---|---|---|
| `TV1_HeaderLine` | `"Attributes Required:\n"` | All chars on line -> `kRgbCream`, alpha preserved from `baseCol` |
| `TV2_KeyValueBasic` | `"Damage: Energy, 4-16\n"` | `"Damage:"` -> `kRgbTealDim`; `" Energy, 4-16"` -> `kRgbCream` (`"-16"` NOT red) |
| `TV3_KeyValueBonusPenalty` | `"Attack Modifier: +3 (-2 Offhand)\n"` | `"Attack Modifier:"` -> `kRgbTealDim`; `"+3"` -> `kRgbBonusAmber`; `"(-2"` -> `'-2'` in `kRgbPenaltyRed` |
| `TV4_WrappedContinuation` | `"Immunity: Mind-Affecting, Poison,\n  Critical Hits"` (soft wrap) | Line 2 has no `'\n'` before it; scans back to `"Immunity: "` -> colored `kRgbCream` |
| `TV5_ProseParagraph` | `"This blaster pistol was common during the Mandalorian Wars. It fires bolts: fast and accurate."` | Sentence punctuation `'.'` before `':'` or `':'` past char 32 -> entire paragraph `kRgbTealText` |
| `TV6_NonTealPassthrough` | Base color = `{0.95, 0.66, 0.30}` (amber stat label) | Classifier returns `baseCol` unchanged |
| `TV7_SectionSplitItem` | Full `GetPropertyDescription` string (props + `\n\n` + lore) | `RichDescBox` splits into Rules block + 2px Divider + Lore body block; strips `{tag}` |
| `TV8_SectionSplitJournal` | `"Nar Shaddaa:\nRefugee Sector troubles..."` | Splits `"Nar Shaddaa"` into Subtitle/Title + 2px Divider + Body |
| `TV9_HookBytesContract` | `0x0047C06F` (6B), `0x0041FD90` (6B) | Exact byte match against Steam exe `6A522E71`; 0 overlap with `patch_config.toml` |

---

## 5. Pillar 3: Tooling, Verification & Gate Commands

1. **Host Unit & Contract Tests (Scoped, < 5s):**
   ```bash
   PYTHONUNBUFFERED=1 python3 -u scripts/run_tests.py rich_text_classifier < /dev/null 2>&1
   PYTHONUNBUFFERED=1 python3 -u scripts/run_tests.py rich_menu_typography < /dev/null 2>&1
   ```
2. **Shared Header Compatibility Check (after touching `_shared_src/`):**
   ```bash
   PYTHONUNBUFFERED=1 python3 -u scripts/compat_check.py patch_manager_mods/_shared_src/RichTextClassifier.h patch_manager_mods/_shared_src/RichDescBox.h < /dev/null 2>&1
   ```
3. **Offline Mockup & Layout Verifier:**
   ```bash
   PYTHONUNBUFFERED=1 python3 -u scripts/rich_menu_mockup.py < /dev/null 2>&1
   ```
4. **Two-Stage Pre-Install Gate & Hot-Swap Flow:**
   ```bash
   ./scripts/pre_install_gate.sh --skip-default --mod rich_menu_typography_src
   PYTHONUNBUFFERED=1 python3 -u scripts/kpm_hotswap.py --prepare rich-menu-typography < /dev/null 2>&1
   ./scripts/pre_install_gate.sh --mod rich_menu_typography_src
   # Ask user for GO before --apply (or hot-swap if hooks already live)
   ```

---

## 6. Pillar 4: Documentation, Subagent Matrix & Contract Compliance

### 6.1 Subagent Delegation Matrix
| Batch | Role | Scope (`--scope`) | Verification Command |
|---|---|---|---|
| **RMT0** | `worker` | `patch_manager_mods/_shared_src/RichTextClassifier.h,patch_manager_mods/rich_menu_typography_src/tests/*` | `python3 scripts/run_tests.py rich_text_classifier` |
| **RMT1** | `worker` | `patch_manager_mods/rich_menu_typography_src/*` | `python3 scripts/run_tests.py rich_menu_typography` |
| **RMT2** | `worker` | `patch_manager_mods/_shared_src/RichDescBox.h,patch_manager_mods/rich_menu_typography_src/*` | `python3 scripts/run_tests.py rich_desc_box` |
| **RMT3** | `worker` | `patch_manager_mods/rich_menu_typography_src/*` | `python3 scripts/run_tests.py rich_menu_typography` |
| **RMT4** | `worker` | `scripts/rich_menu_mockup.py` | `python3 scripts/rich_menu_mockup.py` |

### 6.2 Progress & Handoff Commands
- Mark batch completion:
  ```bash
  python3 scripts/plan_manager.py mark-done <RMT0..RMTR> --state DONE --commit <sha> --notes "<summary>"
  ```
- Scoped context handoff & resume:
  ```bash
  python3 scripts/plan_manager.py handoff 53
  python3 scripts/plan_manager.py resume 53
  ```
