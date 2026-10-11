# B3 compare: entry 0x00775700 (FUN_00775700)

## Agreements
- entry: 0x00775700 in all three files.
- first_12_bytes_at_entry: 558bec83ec0c894df4a1a4b4 in all three.
- ret_immediate: 0x1c in all three.
- stack_arg_count: 7 in all three.
- decompilation_excerpt: identical 10-line body in all three (gate on FUN_0073f810, call to FUN_0087a5c0).

## Disagreements
- calling_convention (string differs): evidence-card says UNSURE; free-form and checklist say UNKNOWN. All three say the convention is not established. Only wording differs.
- callees (formatting only): evidence-card 0x0073f810, 0x0087a5c0; free-form and checklist 0x73f810, 0x87a5c0. Same addresses.
- callers (formatting only): evidence-card 0x007791a0, 0x00899270; free-form and checklist 0x7791a0, 0x899270. Same addresses.
- Substantive conflicts: none.

## Tool calls behind each disagreement
Files do not record per-claim provenance. The mapping below is inferred from the data each call returns.
- calling_convention, evidence-card: procedure_pseudo_code {procedure: 0x00775700}; procedure_assembly {procedure: 0x00775700}
- calling_convention, free-form: procedure_pseudo_code {procedure: 0x00775700}; procedure_assembly {procedure: 0x00775700}
- calling_convention, checklist: procedure_pseudo_code {procedure: 0x00775700}; read_bytes {address: 0x00775700, length: 82}
- callees, each file: procedure_callees {procedure: 0x00775700}
- callers, each file: procedure_callers {procedure: 0x00775700}
