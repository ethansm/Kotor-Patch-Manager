# T4 TSL 2DA schema recon (read-only)
Result: PARTIAL. Only the UTC Appearance_Type width is primary-sourced. 2DA headers are community-sourced.

## Q1 appearance.2da
- S1 (K1 order, in page order): label, string_ref, race, walkdist, rundist, driveanimwalk, driveanimrun, racetex, modeltype, normalhead, backuphead, model*, tex*, texaevil, skin, headtexve, headtexe, headtexg, headtexvg, envmap, bloodcolr, weaponscale, wing_tail_scale, moverate, driveaccl, drivemaxspeed, hitradius, perspace, creperspace, cameraspace, height, targetheight, abortonparry, racialtype, haslegs, hasarms, portrait, footstepsound, footstepvolume, sizecategory, armor_sound, combat_sound, helmet_scale_m, helmet_scale_f, perceptiondist, footsteptype, soundapptype, headtrack, head_arc_h, head_arc_v, headbone, hitdist, prefatckdist, groundtilt, body_bag, freelookeffect, cameraheightoffset, deathvfx, deathvfxnode, fadedelayondeath, destroyobjectdelay, disableinjuredanim. Status: UNVERIFIED (one community page).
- TSL diffs per S1: driveanimrun_pc and driveanimrun_xbox replace driveanimrun; tex*evil replaces texaevil; equipslotlocked added. Status: UNVERIFIED.
- Letter range per S1: model*/tex* run a-j (K1) and a-n (TSL). CONFLICT: repo doc 34 line 74 says modela..modeli.
- Body model: modeltype "B" value = Body-model, which uses normalhead/backuphead (S1). UNVERIFIED.
- Race texture: racetex (S1; S2 K1 context). UNVERIFIED.
- Head model: normalhead, a heads.2da row ID (S1, S2, S3 agree). UNVERIFIED (community only).
- Backup head: backuphead, fallback (S1). UNVERIFIED.
- Alt texture: tex*evil (TSL) vs texaevil (K1) (S1). UNVERIFIED.

## Q2 heads.2da
- Columns in order (S1): head, alttexture (TSL only), headtexvvve (as printed; spelling unconfirmed), headtexvve, headtexve, headtexe, headtexg, headtexvg. headtexg and headtexvg are unused (S1). Status: UNVERIFIED.
- Model name: `head` column (S3: "it's the head column"). alttexture TSL-only (S3). UNVERIFIED.
- Appearance rows reach heads via normalhead (Q1).

## Q3 UTC
- Field "Appearance_Type", UInt16, appearance.2da row index. VERIFIED: PyKotor utc.py dismantle_utc set_uint16("Appearance_Type") (S5).
- Field "PortraitId", UInt16 (S5 write path). VERIFIED.
- Head value: no UTC head field in PyKotor read path (S5 WebFetch extract, not grepped). PARTIAL.
- Repo doc 34 line 77 names "Appearance_Head": NOT FOUND in S5. UNVERIFIED.

## Q4 save persistence
- UNVERIFIED: no source reached. Repo doc 34 line 274 already lists this as open (A1 f).

## Local ground truth
- Live override line 1 (head -1): "2DA V2.b". Header row is line 2, not read (head -1 limit).
- Live override is a modded snapshot (doc 34 line 267), not vanilla. Local header UNVERIFIED.

## Blocked sources
- moddingwiki.com: DNS fail. moddingwiki.shoutwiki.com: Anubis block. rakata utc.html: 404. moddb tutorial: 403.

## Sources
- S1 https://deadlystream.com/topic/4725-detailed-information-of-2da-files/
- S2 https://deadlystream.com/topic/4791-creating-new-npcs-via-appearence2da/
- S3 https://deadlystream.com/topic/6376-appearance2da-and-heads2da-editingmodding-question/
- S5 https://raw.githubusercontent.com/OpenKotOR/PyKotor/master/Libraries/PyKotor/src/pykotor/resource/generics/utc.py
