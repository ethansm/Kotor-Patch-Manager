# NB2 recon: Gob::PlayAnimation (0x004E3270) flags, stacking, removal

Date 2026-10-18. Contract: recon. Binary: Steam swkotor2.exe (md5 6a522e71).
Method: static dumps only (`ghidra_dumps/steam_6a522e71/decomp/*.c|asm`, `callgraph.tsv`). No live REA session was opened: the brief limits REA to decompile/asm, and the dumps already held every listing needed. Ledger: `patch_manager_mods/ghidra_knowledge/rea_findings/20261018_nb2-play-semantics.tsv` (50 rows). Reused: 0x004E31C0 (`20261016_nb2-recon.tsv` row 35, confirmed).
Correction: `nb2_recon.md` line 69 calls entry+0x10 "remaining time". It is a playback position (refuted, table row 15).

## Q1. Flags (verdict PARTIAL: play-once confirmed, fade-to-previous refuted, 0x2 confirmed)
- 0x1 = play once (confirmed). Without 0x1 the position wraps at desc+0x50 (loop). With 0x1 it clamps at the length and the entry ends (0x004E1D40 L232-287). On end, weight entry+0x14 fades to 0 over entry+0x18 (desc+0x54 unless 0x2), then entry+0x20 latches and the entry is removed with a `done<name>` event (L304-319, L258-271).
- It does NOT fade into the previous entry (refuted). The only other start is a queued 0x20 entry in front of it (L297-302).
- 0x2 = skips the fade-in (confirmed). New-entry block (0x004E3270 L117-120): without 0x2 the weight starts at 0 and ramps over desc+0x54; with 0x2 it stays 1.0 and blend time 0 (ctor defaults, 0x004DCDD0 L6-14). A same-name re-Play skips this init.

## Q2. Stacking with an idle loop
- New plain entry (no 0x08/0x20): inserted in front of existing plain entries, behind leading 0x08/0x20 entries (L127-143). So "on top" only holds for plain entries.
- Flags 0 (or 0x1/0x2/0x4): REPLACE. The idle is unlinked and freed at Play time (`done<idle>`), then `start<new>` (L144-155). Hard cut. The new entry still ramps in from 0 over desc+0x54 against whatever base pose remains.
- Flags 0x10: BLEND then replace. The new entry starts at the idle's position (L157-159). When its weight passes 1.0 the idle is removed silently (0x004E1D40 L331-337).
- Flags 0x08: OVERLAY. Nothing removed, no start event, idle stays current (0x004E31C0 skips 0x08).
- Flags 0x20: QUEUE. Held (skipped by the advance loop, L229) until the entry behind it ends or it is back-most (L215-221). Never starts on an endless loop.
- A once-entry that finishes: fades out, latches, and is removed by FUN_004E1D40 (`FUN_00449170` unlink, `FUN_009196FD` free). The idle becomes current again only if it survived Play (the 0x08 path). Otherwise the list is empty and FUN_004E31C0 returns 0. Whether the driver re-adds an idle is OPEN.

## Q3. Entry flag bits (+0x8)
- 0x008 overlay (name probable): not current, not cancelled by a later Play (L144-145), no blend prune (L332). Set only by the caller (raw store L126).
- 0x020 queued (name probable): not advanced (L229). Cleared, with `start<name>`, when the entry behind it ends (L297-302) or when it is back-most (L215-221). Set by caller only.
- 0x200 keyed (probable meaning): duration +0x24, key arrays +0x28/+0x2C, count +0x30, all built by 0x004DCE30 (L13-24). Removal calls 0x004DCF00 first (L266). Play allocates only 0x24 bytes (L95), so 0x200 through Play writes past its block (probable overrun).
- 0x400 force end: Play sets 0x400|0x1 on a found same-name entry and returns (L61-65, asm 0x004E3489). 0x004E3110 ORs any bits onto the last entry (L8-11). The advance loop forces end on 0x400 (L293-295).

## Q4. Return and safety
- RET 0x10 at 0x004E3A15 (confirmed). Return is void (no EAX result in the decompile).
- NULL or empty name: clear-all branch (L32-49), not an error.
- Unknown name: early return at L93-94, nothing added, no null dereference (confirmed).
- Probable hazards: L85-92 loops forever if descriptor length is 0 and param5 > 0 (DLL passes 0.0, not hit). Speed is stored as int (L125): 0.5 becomes 0, and a once-entry at speed 0 never ends (L231, L273-292).

## Fact table
| addr | fact | conf | anchor |
|---|---|---|---|
| 0x004E3A15 | RET 0x10 (thiscall, 4 stack args) | confirmed | asm 0x004E3A15 |
| 0x0045A190 | sole caller; teardown calls Play(NULL,1.0f,0,0.0f) | confirmed | callgraph 0x0045A1F9; decomp L21 |
| 0x004E3270 | NULL/empty name clears all | confirmed | decomp L32-49 |
| 0x004E3270 | unknown name returns early | confirmed | decomp L93-94 |
| 0x004E3270 | flag 0x2 skips weight ramp | confirmed | decomp L117-120 |
| 0x004E3270 | speed stored as (int) | confirmed | decomp L125 |
| 0x004E3270 | flags stored raw in entry+0x8 | confirmed | asm 0x004E37F5 |
| 0x004E3270 | plain Play cancels entries behind (done, start) | confirmed | decomp L144-155 |
| 0x004E3270 | flag 0x10 keeps entries, syncs position | confirmed | decomp L157-159 |
| 0x004E3270 | flag 0x400 on found name forces end | confirmed | asm 0x004E3489; decomp L61-65 |
| 0x004E31C0 | current = first entry without 0x08/0x20 | confirmed | decomp L8-17 |
| 0x004E1D40 | flag 0x1 clear = loop; set = play once | confirmed | decomp L232-287 |
| 0x004E1D40 | finished entry removed via 0x00449170 + 0x009196FD | confirmed | decomp L258-271 |
| 0x004E1D40 | blend prune of entries behind unless 0x08 | confirmed | decomp L322-337 |
| 0x004E1D40 | entry+0x10 is position, not remaining time | refuted claim (nb2_recon L69) | decomp L231 |

## OPEN
1. Which path re-adds the idle after an empty list (candidates 0x004DCA80 / 0x004DCE30 via 0x004DC920; not decoded).
2. Values of DAT_00991398 (float), DAT_009913A0 (double), DAT_00986B38 (double), DAT_00986B50 (not read; needs one read_bytes).
3. Vtable slot 6 = 0x004E3270 not checked here.
4. Owner of the bit-0x1 setter 0x007B5EF0 unidentified. 0x0062BDE0 and 0x00605020 use a different list (+0x270), not the Gob list.
5. Mixer effect of overlay weights (0x004DFED0) not decoded.

## DLL implications (probable, untested live)
- Pass speed 1.0f (integer). Do not pass 0x200. Avoid 0x40, 0x80, 0x400 unless intended.
- To keep the idle loop, try flags 0x09 (overlay + once): idle stays current and the flourish is removed when it ends. Flags 0 or 0x10 remove the idle.
- Flag 0x02 gives instant weight 1.0 (no fade-in). Default ramps in over desc+0x54 seconds.
