# quick-loot PV1 UX review (render ql_1..6 + QuickLootLogic.h)

Measured from ql_fit_table.txt: the font runs ~20 px/char (626/31, 1247/62, 1126/57, 1604/80, 1030/52; 1-item peek 536/28 = 19 px).
So 420 px holds ~20 chars, 760 px holds ~37 chars. Current caps (Took 80, Peek 100 chars) need 1600-2000 px, i.e. 2-3x the label.
Only the 1-item peek fits. Every other test case overflows. Renders show the label rect (red outline) cut across the text,
and the engine would wrap it into 2-5 lines that grow down over the scene. Caveat: mock draws one line, the game wraps.

## 1 Fit
- MUST one line, one width, caps derived from 20 px/char: PeekW = ToastW = 1200 px (31% of 3840), ToastH stays 20.
  PeekMaxChars = ToastMaxChars = 56 (56 x 20.2 = 1131 px, 6% margin). Both labels now share a rect, so they centre identically.
- MUST per-entry cap: ToastNameMax 40 -> 20 chars. Cut = first 17 chars, trim trailing space/punctuation, append "..." ("Lightsaber Crystal..."). The " xN" suffix is added after the cut (entry <= 24).
- Worst case check: prefix 19 ("[F] Take all (25): ") + entry 24 + ", +24 more" 10 = 53 <= 56, so at least one name always shows.
- Keep the existing "fit as many as possible, then ', +K more'" loop in detail::fmtList; only constants and the cut change.
- SHOULD log the label's real getExtent after SetText on first show (PEEK-LABEL line) so the first play proves 20 px/char at 3840x1600. Fallback if it wraps: PeekMaxChars 48.
- COULD 2-line layout (title + items) only if the engine honours "\n" in labels. Unverified, and one line of 56 chars carries the same information. Skip for PV1.

## 2 Information design
- MUST put the action first, count in brackets, drop the double space (a double space renders as a large gap):
  `[F] Take all (4): Medpac x2, Blaster Pistol, +2 more`   (52 chars, 1050 px, fits)
  `[F] Take all (25): Medpac x2, Blaster Pistol, +23 more` (54 chars, fits)
  `[F] Take all (1): Medpac` -> for N=1 use `[F] Take: Medpac` (drop "all" and count).
  Reason: the key is the thing the eye needs; the count is secondary. Case is cosmetic, the font draws small caps.
- "[F]" is fine (no KOTOR convention; square brackets read as a key). Use qlogic key name from the ini, so a rebound key shows up.
- Container name: SHOULD NOT repeat it in position A (the name tag is right above). COULD in position B only: prefix "<name 14 chars>: " when the budget allows, else omit.
- Empty state: SHOULD keep "no label" for 0 items (fmtPeek already returns ""); never show "0 items". Also no peek for an already-emptied container.
- Credits: COULD show credits first as "250 credits" if the repository reports them as an item; unverified, check in the first probe log. Do not count them in (N)? Count them; they are in the take.

## 3 Placement A vs B
- Position A (name_bottom + 20): clears the health bar in ql_4_zoom (about 8 px gap). OK. Risk: it follows the name tag as the camera moves, and a 1200 px label near the screen edge clips. MUST clamp x to [24, screenW - 24 - w] for A.
- MUST latch the position: once a peek shows in A or B, keep it there until it closes. Re-evaluating every frame makes the line jump when the name tag appears or vanishes (selection flicker while walking). Switch A -> B only after the tag is gone >= 300 ms.
- Position B at 62% of 992 = 615 px: probably over the player character in third person. COULD move to 70% (above the bottom bar, below the PC) and judge in play. It is a one-constant change (PeekTopPercent).
- SHOULD add a backing panel: black, alpha 0.55, pad 10 px x / 4 px y, same rect as the label. Yellow (0.9,0.85,0.5) on no outline is weak on bright scenes (Tatooine sand, white Peragus corridors). Reuse the enemy-effects-strip Backing/pill option (EnemyStripLogic.h previewLayout `backing`, `pillPad`) rather than a new mechanism. If a panel is not possible in PV1, SHOULD at least darken nothing and accept it; the dark render looks fine.
- Colour: keep 0.9,0.85,0.5 (matches KOTOR yellow text). SHOULD render the "[F]" in white if the label supports colour runs (unlikely, so COULD).
- LBL_QUEUE1 name suggests the queue HUD template. SHOULD confirm queue-hud does not drive the same label at the same time (shared-label fight shows as flicker).

## 4 Took vs peek consistency
- MUST Took reuses the rect the peek had (latched A/B and same 1200 px width). Today ToastW 420 vs PeekW 760 centred on the same point means the text re-centres and jumps when F is pressed.
- Same grammar so the swap looks like one line changing state: `Took (4): Medpac x2, Blaster Pistol, +2 more` (same prefix budget, same cap 56).
- Click-take (no peek): container is the selected target, so it is position A; keep A and the same rect.
- Timing: 3 s is fine for <= 3 names. SHOULD 3 s + 0.5 s when the line has more than 3 names or "+K more" (3.5 s), cap 4 s. Do not extend further, it blocks the next peek.
- A new Took replaces the old one immediately (no queue); the peek of the next container may show once the Took expires, or sooner if its rect is the same (then the Took wins for its 3 s).

## 5 Annoyances in play
- SHOULD dwell debounce for auto-open: leader must stay within 3 m of the same container for >= 400 ms before OpenInventory. Walking past a row of lockers otherwise gives lid animation + sound per container.
- SHOULD rate limit: >= 1.5 s between auto-opens, only the nearest container, and no switch to a second container while a peek is active (hysteresis 3 m open / 5 m close is already good).
- SHOULD no auto-open in combat, dialogue, cutscene, or while the inventory/menu is open (check the combat flag the other mods use).
- SHOULD after F take (or click take) put the container in cooldown (exists: CooldownCap 64) so walking back does not re-open an emptied one; empty containers never auto-open.
- COULD suppress the close animation/sound on the walk-away close if the engine call allows; otherwise accept it.
- COULD config: peek_width_px, peek_max_chars, auto_open_dwell_ms in the ini for tuning after PV1 play.

## Verdict
Content is right but the layout does not fit: 4 of 5 renders overflow and would wrap. Fix constants (1200 px, 56 chars, 20-char entries), the new prefix, the latched shared rect and edge clamp, then play. Backing panel and debounce are SHOULD and can follow the first play.
