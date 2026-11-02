# Research Note: Rich Menu Typography Spacing & Vertical Overflow Dynamics

## Overview
Investigation into why rich description boxes (`RichDescBox.h`, `RichTextClassifier.h`) generated variable-length vertical blank spaces and premature scrollbars across UI panels in KOTOR II.

## 1. Structural Root Causes of Spacing Inflation

### A. Engine Listbox Inter-Item Padding (`CSWGuiListBox::OrganizeUnequal`)
- **Location**: `swkotor2.exe:0x0041F170` (`CSWGuiListBox::OrganizeUnequal`).
- **Mechanism**:
  - `0x0041f3b5`: `movzx ecx, BYTE PTR [listbox + 0x2D0]` (`m_nPadding`).
  - `0x0041f414`: Row 0 placed at `Y = padding`.
  - `0x0041f53e..0x0041f551`: Row 1 placed at `Y = row[0].y + row[0].h + padding`.
  - `0x0041f598..0x0041f5a5`: Successive rows `i` placed at `Y = row[i-1].y + row[i-1].h + padding`.
  - `0x0041f638`: `cmp edx, DWORD PTR [listbox + 0x2A8]` compares `row.bottom` against `clientH`.
  - `0x0041f649`: If `row.bottom > clientH`, sets `bottomItem = i - 1`.
  - `0x0041f71d..0x0041f73e`: If `totalItems - visibleCount + 1 > 1`, activates vertical scrollbar via `0x0041C760`.
- **Finding**: With multiple pooled rows (Title, Subtitle, Rules, Body), listbox inherently injects `(outCount) * m_nPadding` pixels of vertical separation.

### B. Double-Newline Injections (`\n\n`) in Body
- **Location**: `RichTextClassifier.h:551`.
- **Mechanism**: `itemprops::appendTrunc(out->body, sizeof(out->body), rl.blankBefore ? "\n\n" : "\n")`.
- **Impact**: In `measureTextHeight`, each empty line added `halfLine = lineHeight / 2` (6–8px), while the game engine's text renderer rendered each empty line as a full line height (~14px). Across 2–3 paragraphs in an item description, this inflated vertical height by 28–42px.

### C. Untrimmed Whitespace and Carriage Returns
- **Location**: `RichTextClassifier.h:417-418`.
- **Mechanism**: Copying raw `[lineStart, lineEnd)` instead of trimmed `[ts, te)` preserved trailing spaces and `\r`.
- **Impact**: `measureTextHeight` integer ceiling division `(plen + charsPerLine - 1) / charsPerLine` rounded up to an entire extra line height (12–16px) for just 1–2 trailing whitespace characters.

### D. Hardcoded Row Extra Padding
- **Location**: `RichDescBox.h:405, 410`.
- **Mechanism**: `setupTextRow(sec.rules, getRulesFont(), kTealText, 2)` and `setupTextRow(sec.body, getBodyFont(), kTealText, 2)` added hardcoded `extraPad = 2` to both rows (+4px).

### E. Inverted Height Clamping Gate
- **Location**: `RichDescBox.h:426`.
- **Mechanism**: `if (sumHeights <= lbH && sumHeights > maxVisible && outCount > 0)`.
- **Impact**: The clamp only activated if `sumHeights <= lbH`. As soon as any overestimation pushed total height over `lbH` by even 1px, clamping disabled entirely, triggering premature scrollbars.

---

## 2. Tight Spacing Profile (Current Baseline)
To eliminate variable empty spaces and ensure compact, deterministic rendering:
1. **Paragraphs**: Body paragraphs separated by single `\n` instead of `\n\n`.
2. **Whitespace Trimming**: Strictly copy trimmed bounds `[ts, te)` to prevent ceiling-division line wrap inflation.
3. **Extra Padding**: Set `extraPad = 0` on Rules and Body text rows.
4. **General Clamping**: Clamp whenever `sumHeights > maxVisible && outCount > 0`, shaving excess pixels from bottom rows to prevent scrollbar activation.

---

## 3. Roadmap for Future Strategic Spacing Additions
When selectively restoring breathing room in future iterations:
- **Controlled Section Spacing**: Instead of inserting `\n\n` into the text string, adjust `extraPad` per section (e.g., +2px between Title/Subtitle or between Rules and Body) where bounded and predictable.
- **Inter-Item Listbox Padding Awareness**: Measure `*(uint8_t*)(listbox + 0x2D0)` or account for `(outCount) * padding` in `sumHeights` calculations before clamping.
- **Configurable Spacing Mode**: Allow users/themes to select between "Tight" (current baseline) and "Comfortable" (strategic padding) in `RichMenuTypographyPage.h`.
