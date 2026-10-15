# SH-T21

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-09 | PARTIAL | patch_manager_mods/_shared_src/textfit_test.cpp:52; patch_manager_mods/_shared_src/TextFit.h:31,42,43,61 | yes | Anchor exact. Off=7 is 'e', not a space. Off-on-space, negative off, >120 word absent in 30-70. Trailing-space trim not targeted. |

- textfit_test.cpp:52 is the exact anchor line; its F10 call passes off=7, and "Droid Deflector Mark I"[7] is 'e' (spaces sit at indices 5, 15, 20).
- textfit_test.cpp:44-53: marqueeText offsets exercised are 0, 3, 100 (clamped to maxOff 9) and 7; none lands on a space.
- TextFit.h:43 skips spaces after off; no input in 30-70 triggers it, so off-on-space is untested in range.
- TextFit.h:42 clamps negative off to 0; no call in 30-70 passes a negative offset.
- TextFit.h:61 caps word length at 120 and line 60 consumes the rest of the word, so chars past 120 are dropped; test inputs at lines 58, 66, 70 use words of at most 8 chars.
- TextFit.h:31 strips trailing spaces after the cut loop; no fitText or marquee input in 30-70 is typed with a trailing space.
- Expected outputs at textfit_test.cpp:36, 49, 53 could still run the trim internally if prefix plus "..." fits; that depends on glyph widths (kFont10TexW/kFontTexW, unread), so UNSURE.
- Not read: textfit_test.cpp:1-29 and 71+, TextFit.h:1-19 and 75+. Gap claims hold for 30-70 only.
- Anchor's "function main" not verified; the enclosing function opens above line 30 (unread).
- fix_sound yes: the four added cases are test-only; expected strings still need a font-width check before commit.
