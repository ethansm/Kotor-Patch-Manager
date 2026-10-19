# h6a: vanilla K2 supernode simulator sweep

Source: `.claude/research/pykotor_writer/h2_sn_sim.py` (copied to scratch, not edited). Driver: scratch `h6a/run_h6a.py`. Read-only over `scratchpad/k2mdl/` (vanilla K2 MDL+MDX).

Checks per model: sim supernode per node index vs file u16 at node header +2 (`padding0`), and predicted total vs file total at offset 56. Supermodel = `<name>.mdl` in the same dir, name read from offset 148 (32 bytes, NUL-terminated). NULL/empty = out of scope.

## Counts

| Bucket | Count |
|---|---|
| MDL files scanned | 3237 |
| Out of scope (supermodel NULL/empty) | 2835 |
| In scope (has supermodel name) | 402 |
| Checked (child + supermodel loaded, sim ran) | 398 |
| Exact match (all node sn and total) | 398 |
| Mismatched | 0 |
| Supermodel file missing | 2 |
| Load errors (header, child or supermodel) | 2 |
| Simulator exceptions | 0 |

## Tag frequency over all checked models

| Tag | Models |
|---|---|
| root_name | 398 |
| unmatched_name | 388 |
| sup_has_super | 385 |
| dup_siblings | 8 |

## Supermodel missing

| Model | Named supermodel |
|---|---|
| h_m_hi01 | cspar2 |
| n_djedi_h | s_male02h |

## Load errors and simulator exceptions

- n_darthmalak02 (load_error): super N_DarthMalak: This operation would exceed the streams boundaries.
- n_darthmalak03 (load_error): super N_DarthMalak: This operation would exceed the streams boundaries.

## Known expected cases

- s_female02 (sup s_female01): status=exact, file total 423, pred 423, mismatch nodes 0/103
- s_female01 (sup s_male02): status=exact, file total 319, pred 319, mismatch nodes 0/106

Both known cases PASS in this sweep. The `h2_findings.md` note (S_Female02->S_Female01 total 423 vs 210; S_Female01->S_Male02 9 nodes) does not reproduce with the current `h2_sn_sim.py` on the vanilla extraction.

## Caveats

- Negative control: on a 60-model sample (2870 nodes) a sim shifted by +1 mismatches every node, and 2558 of those nodes have file sn != node index, so the exact-match result is not vacuous.
- `root_name` fires on all 398 checked models (supermodel root is always a different name), so it does not discriminate. `unmatched_name` (388 models) means most matches exercise the sim's new-supernode allocation path, not name matching.
- Errors n_darthmalak02 and n_darthmalak03 are not checked: pykotor fails to read the supermodel `n_darthmalak.mdl` (stream boundary), so this is a reader issue in the supermodel, not a sim result.
- Supermodel-missing h_m_hi01 (cspar2) and n_djedi_h (s_male02h) have no file in the vanilla extraction.

