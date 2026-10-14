# R3 supermodel handling (u03, recon)

## How k1_port_model.py treats the supermodel
- Name is never renamed (patch_manager_mods/k1_port/k1_port_model.py:13-14). NONE values: "" and "NULL" (:67).
- Lookup is K1-side only: pack dir, then K1 install (load_supermodel, :521-540). Missing name gives warning "not found in the pack dir or the K1 install" (:534), status not_found.
- Manifest fields: supermodel, supermodel_status, supermodel_origin (:704-706).
- No K2 existence check appears in the matched code. A name found in K1 but absent in K2 converts with no K2 warning (inferred from the code; converter not run).
- Tests: make_pack(supermodel=...) (patch_manager_mods/k1_port/test_k1_port_model.py:163); missing supermodel gives not_found + warning, exit 0 (:387-392); empty supermodel round-trips (:708).

## Raw K1 header facts (byte walk, not pykotor)
- Supermodel name: file offset 0x94, 32 B, NUL-terminated (MDL-rel 0x88). Model name: file offset 0x14.
- Offset check: K2 Override P_CarthH.mdl gives S_Female02, matching the K1a/K2v review notes.
- K1 source (chitin BIF), model name -> supermodel:
  - p_juhanih: P_JuhaniH -> P_JuhaniBB
  - p_juhani: P_juhani -> NULL
  - p_carthba: P_CarthBA -> P_CarthBB
  - p_carthbbh: P_CarthBBH -> S_Male02H
- Staged k1p_* files carry the same supermodel string as their K1 source (no rewrite).

## Presence of each supermodel name
| Name | K1 chitin | K2 chitin | K2 Override | Stage | Verdict |
|---|---|---|---|---|---|
| P_JuhaniBB | yes | no | no | no | K1-only |
| P_CandBB | yes | no | no | no | K1-only |
| P_JoleeBB | yes | no | no | no | K1-only |
| S_Male02H | no | no | no | no | absent in K1 and K2 |
| P_CarthBB | yes | yes | yes | no | in K2 |
| P_BastilaBB | yes | yes | yes | no | in K2 |
| S_Female02 / S_Female03 / N_TwilekF | yes | yes | yes | no | in K2 |

- Grep for S_Male02H and the three K1-only names: no hits in K1 patch.erf, K1 modules/, K2 modules/. K1 Override/ is empty.

## Converted models (stage, 18 files)
Still reference a K1-only or absent supermodel (7):
- k1p_juhanih, k1p_juhaniba -> P_JuhaniBB (K1-only)
- k1p_candba, k1p_candh -> P_CandBB (K1-only)
- k1p_joleeba, k1p_joleeh -> P_JoleeBB (K1-only)
- k1p_carthbbh -> S_Male02H (absent in both installs)

Resolve in K2 (9): k1p_bastilaba, k1p_bastilabb02 (P_BastilaBB); k1p_carthba (P_CarthBB); k1p_candbb, k1p_joleebb, k1p_commm (S_Female02); k1p_juhanibb, k1p_lenab (S_Female03); k1p_lenah (N_TwilekF).

No supermodel (2): k1p_juhani, k1p_t3m3 (NULL).

## Chain note
- K1 P_JuhaniBB -> S_Female03 (p_juhanibb); P_CandBB -> S_Female02 (p_candbb); P_JoleeBB -> S_Female02 (p_joleebb).
- The second hop resolves in K2; the first hop (P_JuhaniBB, P_CandBB, P_JoleeBB) does not.

## Scope and method
- Read: patch_manager_mods/k1_port/k1_port_model.py:1-60 as briefed.
- Beyond the brief: the task-mandated rg returned lines outside 1-60 (k1_port_model.py:67, 513-545, 645, 704-706; test_k1_port_model.py:48, 112, 163, 387-392, 708). Those hits were seen only as rg output, not read as files.
- rg also matched .claude/review/k1_import/ docs. They are not used as evidence here.
- Raw walk script: scratchpad u03/raw_super.py. Read-only on K1/K2 chitin BIF and KEY, K2 Override, and the stage dir. Converter not run; no writes except this file and the reply file.
