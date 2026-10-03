# KMRP / KPM PR #286 reference notes (K1 bug descriptions; NOT K2 evidence)

Persisted 2026-10-02 so the session scratchpad can be discarded. Everything here describes **K1 (`swkotor.exe` 1.03 / Aspyr macOS x86_64)**; addresses are K1 addresses.
Use only as leads; K2 facts live in `engine_research_findings/V1-V6.md`. K1 -> K2 name bridge: `k1_k2_bridge.tsv` (`tools/k1_to_k2_bridge.py`).

## Sources (re-fetch commands)
- KMRP repo: https://github.com/RayesDiyab/KMRP-KOTOR-Modern-Restoration-Patch (`reverse-engineering/*.md`, `docs/font-scaling.md`, `docs/universal-resolution-math.md`).
  `gh api repos/RayesDiyab/KMRP-KOTOR-Modern-Restoration-Patch/contents/reverse-engineering --jq '.[].download_url'`, then `curl -sfL <url>`.
  Files used: `binary-inventory.md` (every patched K1 byte run; the bridge parses its section-4 table), `listbox-geometry.md`, `inventory-item-rows.md`, `map.md`, `map-markers.md`, `map-scaling.md`, `area-map-surface.md`, `message-popup.md`, `text-padding.md`, `movies.md`, `font.md`, `exe-patching.md`.
- KPM PR #286 "Widescreen patch and stray bug fixes" (merged): `gh pr diff 286 --repo LaneDibello/Kotor-Patch-Manager`; files `Patches/K1WidescreenPatch/KMRP-ENGINE-FIXES.md` (K1-K9 below) and `Patches/K1StrayBugFixes/README.md` (K1, K2, K3, K5, K6 as a standalone patch).
- K1 clean exe for the inventory = `swkotornopatch.exe` sha256 `761F9466...` = KPM `kotor1_0_3.db` "cd crack 103" (same address space; the bridge relies on that).

## The nine KMRP fixes (K1 mechanism as described there)
| Id | Bug | Mechanism / fix (K1) | K2 result (doc 16 / V files) |
|---|---|---|---|
| K1 | word-wrap hang | `WrapStrings` backs the cursor up one char on overflow, then checks progress against the string start instead of the line start; empty line appended per pass (OOM). Fix: test against line start `r15`, take the remainder whole. Two narrow-label blanking guards also removed. | V3: same bad compare (0x47B321) but a 100-line cap (0x47B0E8) => no hang. Killed. |
| K2 | listbox rows grow | `OrganizeControls` writes enlarged row height into a local `[rbp-0x34]` and reused row controls; next pass takes max row height as item height (`+0x368`). Powers/Feats 42->56->126 px. Fix: NOP two stores. | V2: cycle broken (ClearItems restores saved rects via vtbl+4 when flag bit 2). Killed. |
| K3 | leading newline in descriptions | description builder prefixes `\n` per property; trimmed in `CSWGuiTextParams::SetText`. | Not examined (K2 SetText = 0x416E30; cosmetic; open idea). |
| K4 | video mode follows widescreen target | macOS only. | n/a |
| K5 | single line taller than box vanishes | `Draw` drops lines from the top until the rest fits, including the last. Fix: keep the last line (4 replace hooks). | V3: same loops (0x47B6CB / 0x47B7CB); latent (only at >=5120 wide). L15. |
| K6 | wrap under-measure | advance truncated to int after `+0.25f` (avg 0.25 px/char); fix `+0.5f` (re-point one operand). | V3: confirmed, FADD double [0x98C1D8]=0.25 at 0x47B232 -> re-point to 0.5 @0x987F08. L4. |
| K7 | dialogue letterbox | bar = `(H - W/(7/3))/2`, 7/3 a float constant read by 6 sites; negative past 21:9; reply panel hard-coded 100 px; fix bar = H/6 via `c = 1.5*W/H` (one write), reply-panel immediates -> `bar - 36` (min 100), `LB_REPLIES` stretched to the panel in `CSWGuiDialogCinematic::SetExtent`. | V1: same formula (double 2.3333330154 @0x9A7D30, 7 FDIV sites); 3840x1600 -> -22 px. L1. |
| K8 | minimap zoom | HUD authors radar 120x120, map 512x512; enlarging only the radar shows 1.6x more area; fix replays a 120 px radar in the enlarged viewport (3 detours). Not a pan clamp. | V4: radar/map from GUI rects; live HUD vanilla; live aspect-fix hook corrects stretch. Killed. |
| K9 | Retina display modes | macOS only. | n/a |

**Phase 3 fetch (2026-10-03):** KMRP commit `156552f54d8b5650145b0aabb2b23d56cd72ec91` (committer date 2026-10-01): `movies.md`, `text-padding.md`, `map-markers.md`, `area-map-surface.md`, `font-atlases.md`, `button-focus-badge-geometry.md`, `texture-residency.md` saved to `~/Games/KOTOR2_Mod_Workspace/kmrp_docs/` (third-party text, outside the repo), plus `K3_leading_newline_from_K1StrayBugFixes_README.md` (K3 section of the local KPM clone). Re-fetch: `curl -sfL https://raw.githubusercontent.com/RayesDiyab/KMRP-KOTOR-Modern-Restoration-Patch/<sha>/reverse-engineering/<name>.md`. Briefs V8 (text/fonts), V9 (movies/maps), V10 (focus geometry/texture residency) in `engine_research_findings/V_BRIEFS.md`.

Other KMRP topics NOT yet looked at for K2 (before Phase 3) (possible later leads): `movies.md` (Bink video fullscreen size / aspect fit hook at K1 0x4057AC), `map-markers.md`, `area-map-surface.md`, `text-padding.md` (K1 0x415E0D SetText region), `font-atlases.md`, `button-focus-badge-geometry.md`, `texture-residency.md`.
Rule from KMRP's own docs: scaling factor `max(1.0, height/720)`; universal resolution math is in `docs/universal-resolution-math.md`.
