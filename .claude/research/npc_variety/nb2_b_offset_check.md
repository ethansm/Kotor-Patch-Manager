# NB2 B offset check: DiversityAnimEngine.h uncommitted diff

Date: 2026-10-18. Read-only recon. Scope: every offset, slot, and calling-convention constant the diff adds (`git diff -- patch_manager_mods/npc_diversity_src/DiversityAnimEngine.h`), plus the pre-existing Play path the header uses.
Sources: `nb2_anim_driver_recon.md`, `nb2_play_semantics_recon.md`, `nb2_recon.md`, ledgers `20261018_nb2-anim-driver.tsv`, `20261018_nb2-play-semantics.tsv`, `20261016_nb2-recon.tsv`, equip3d `research/R9_head_item_2026-10-02.txt`, `EquipCharPreview.cpp`, `Preview3dProvider.cpp`.

## Constant check

| Diff constant | Value in diff | Fact | Evidence | Verdict |
|---|---|---|---|---|
| kOffGobCount | 0xE0 on body | Gob+0xE0 = entry count | play-semantics ledger row 39 (Gob+0xDC list, +0xE0 count); nb2_recon.md L68; 20261016 ledger row 35 | CONFIRMED |
| kOffClientAnim | 0x48, read u16 | client+0x48 u16 = current anim id | anim-driver ledger rows 3 and 12: `*(short*)(param_1+0x12)` with int-typed base = +0x48 | CONFIRMED |
| kClientSetAnimSlot | 0x9C/4 = 39; SetIdle reads vt + 39*4 = 0x9C | vt+0x9C = 0x007ed9a0 | anim-driver ledger row 9 (dword index 39 = a0d97e00) | CONFIRMED |
| FnSetAnim | `void __thiscall(uintptr_t, short, unsigned, int)`, args (client, 10000, 0u, 0) | thiscall(creature, short, dword, int), RET 12 | anim-driver ledger row 2: `__thiscall(int*, short, undefined4, int)`, header ret=12 | CONFIRMED |
| SetIdle id | literal 10000 | default idle 10000 | ledger rows 6 and 8 (setter fallback L216/L498); ctor default 10000 (nb2_recon) | PARTIAL, see M1 |
| kBodyPlaySlot | 6, byte offset 0x18 | body vt slot 6 = Gob::PlayAnimation 0x004E3270 | equip3d R9 L14 `vt[0x18] = PlayAnimation 004e3270 thiscall ret=16`; EquipCharPreview.cpp:65 and Preview3dProvider.cpp:65 PartPlaySlot 0x18 | PARTIAL, see O1 |
| Play signature (pre-existing, unchanged by diff) | `void __thiscall(uintptr_t, const char*, float, unsigned, float)`, 16 bytes | thiscall(name, float speed, unsigned flags, float 0.0f), RET 0x10 | play-semantics ledger row 2 and recon Q1 (RET 0x10 at 0x004E3A15); anim-driver ledger row 21 | CONFIRMED |
| kHolderGetSlot / kPartBody | vt+8 (slot 2) on holder; 0xFF | holder vt[2](0xFF) = body Gob | equip3d and Preview3dProvider CreatureHolderOff 0x68, `hv[2]` with id 0xff (partGob); not in NB2 ledgers (nb2_recon.md L118: "not verified here") | source-level only |
| kOffClientHolder | 0x68 | creature+0x68 = holder | anim-driver recon L19 (holder = creature+0x68); R4a L598/L654 reads creature+0x68 as holder | CONFIRMED (holder class UNKNOWN) |

## Mismatches

M1. SetIdle hard-codes 10000, but the engine's own idle re-issue takes its id from `FUN_007ed980(0,0)`, which is undecoded (anim-driver recon Unknowns #3). Ledger row 13 is labelled "confirmed" for "re-issuing default", but that label does not carry the id. The 10000 matches the setter's fallback (ledger rows 6 and 8), not a decoded FUN_007ed980 return. Recon Q4 "(10000, 0, 0)" inherits the same gap. Effect: value is plausible, not proven as the engine's idle id.

M2. Header comment line 8 of `DiversityAnimEngine.h` reads "body Gob = holder = [client+0x68]". Code (line 111) sets `c->body = holder->vt[2](holder, 0xFF)`. The holder is not the Gob. The anim-driver recon also treats creature+0x68 as a separate holder object (class UNKNOWN, vt+0x54). The code is correct (both kOffGobCount and Play use body), but the comment invites applying +0xE0 to the holder. Pre-existing text, not in the diff.

M3 (doc drift only). `nb2_recon.md` L15 and L73 and 20261016 ledger row 36 say client +0x48 has no known writer. The 10-18 anim-driver ledger rows 3 and 12 found writers (0x007ed9a0, 0x00777e30). The diff's "current animation id" wording matches the 10-18 finding.

## Open

O1. Play-semantics recon OPEN #3: "Vtable slot 6 = 0x004E3270 not checked here." Only equip3d R9 supports it. One REA `read_bytes` on the Gob vtable, dword index 6, would close it.

O2. Holder class and its vt+0x54 are UNKNOWN (anim-driver Q1, Unknowns #1). SetIdle's visible effect runs through that call, so the idle write is not confirmed end to end.

O3. Holder chain (creature+0x68, then vt[2](0xFF)) is confirmed only at source level in equip3d and preview code. The NB2 recon says it was not verified there.

## Verdict

PARTIAL. No numeric mismatch: 0xE0, 0x48, slot 39 (=0x9C), 12-byte stack args, and RET 16 all match the facts and ledgers. Open items: M1 (idle id source), M2 (comment), O1 (slot 6 link).
