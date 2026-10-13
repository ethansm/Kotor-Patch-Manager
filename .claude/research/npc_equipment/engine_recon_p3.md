# engine_recon_p3: equipped HEAD item on modeltype F/L vs B (Steam vanilla)

Binary: `swkotor2.exe.unpatched_vanilla` (sha256 6a522e71...ffef), x86 PE, Ghidra 12.1.4, read-only.
All addresses are Steam vanilla image addresses.
Labels: CONFIRMED = Ghidra output at the cited address. INFERRED = from call-site pattern, callee body not decompiled. NOT FOUND = searched, absent. UNKNOWN = could not determine.

## 1. Function containing 0x0085B703 and its callers

- 0x85B703 is inside `FUN_0085a9e0` (entry 0x85a9e0, 75 basic blocks, 3931 bytes, 3 ranges 0x85a9e0-0x85ab64, 0x85ab67-0x85addb, 0x85adde-0x85b93e). CONFIRMED by `resolve_containing_procedure`.
- 0x85B703 is an xref of the string "GoggleHook" at 0x9a43e0. CONFIRMED by `xrefs`.
- Sole resolved caller of `FUN_0085a9e0`: `FUN_0085a6c0` (0x85a6c0, 247 bytes). CONFIRMED by `procedure_callers`.
- Sole resolved caller of `FUN_0085a6c0`: 0x76d200. CONFIRMED.
- `FUN_0085a9e0` is called from `FUN_0085a6c0` with type 3 only: `FUN_0085a9e0(3, rec)`. CONFIRMED by pseudocode.

### Gate (CONFIRMED in code, letter mapping UNKNOWN)

`FUN_0085a6c0(rec)`:
- `row = FUN_0071afa0(rec+0x18, DAT_00a0fde0)`, then `type = FUN_0085c150(row)`.
- `type == 3` calls `FUN_0085a9e0(3, rec)`.
- Otherwise it runs body vtable +0x7C(0xfe) (clear head slot), sets `in_ECX+0x1a = rec+0x1a`, and returns 1. No GoggleHook call on this branch.

`FUN_0085c150` (0x85c150) return values, from its pseudocode:
- `FUN_00734190(0x53 'S')` found: returns 0xb.
- Otherwise a string set at DAT_00991b40 is matched with `FUN_00734030`: match returns 3.
- Otherwise 'F' (0x46) or 'L' (0x4c) found: returns 2.
- None found: returns 0xffffff0b.
- The decompiled signature is `void` and does not read its argument, so the exact cell it classifies is UNKNOWN.
- `DAT_00991b40`: `list_strings` returns no Ghidra string, so the letter behind value 3 is UNKNOWN. The brief's "B" label is not confirmed by the binary.
- `DAT_00a0fde0` holds 0xFFFFFFFF in the file image (`read_bytes` 0xa0fde0). It is a runtime column id, so its column name is UNKNOWN.

## 2. F/L path

- F/L (values 2 or 0xb) never reach `FUN_0085a9e0`. CONFIRMED by the `FUN_0085a6c0` gate above.
- Head item model attach for F/L via `FUN_0085a9e0`: NOT FOUND.
- Head slot clear for F/L: CONFIRMED (`FUN_0085a6c0` else branch, vtable +0x7C(0xfe)).
- `FUN_0085a1f0` (0x85a1f0, sole caller 0x76d200) dispatches on `FUN_0085c150`:
  - 3: `FUN_0085cc00(3, rec)` (B body builder).
  - 2 or 0xb: `FUN_0085b940(type, rec)` (F/L path). CONFIRMED by `FUN_0085a1f0` pseudocode.
  - `FUN_0085b940` body was NOT decompiled in this pass. Its F/L head-item behavior is UNKNOWN.
- Head-variant swap for F/L: NOT FOUND on resolved call paths.
  - The name builder (`"_HEAD"`, `"0_HEAD"`, `"%03u"` of rec+0x1a) sits in the non-3 branch of `FUN_0085a9e0`, which no resolved caller reaches.
  - Indirect callers of `FUN_0085a9e0` are UNKNOWN.
- Head placement at `headhook` for F/L: NOT FOUND. "HeadHook" has one xref (0x85d1ba, in B builder `FUN_0085cc00`). `FUN_0085b940` has no direct xref to either hook string.

## 3. B path (for contrast, CONFIRMED)

`FUN_0085cc00` (0x85cc00, ranges 0x85cc00-0x85cd4c, 0x85cd4f-0x85d254). Its error literal is "CSWCCreatureAppearance::CreateBTypeBody(): Failed to load model '%s'." (0x9a4438). Sole caller 0x85a1f0 (type 3).
- Head part node 0xfe: `local_38 = vtable+0x98(0xfe,1)`.
- Body part 0xff loaded: vtable+0x78(out, 0xff, type).
- Head node reparented: `local_38 +0x50(body node 0xff, "HeadHook", 0)`, call site 0x85d1ba (xref to "HeadHook" 0x9a4428).
- Item head model reparented in `FUN_0085a9e0` (type 3): item node 0xff `+0x50(body node 0xfe, "GoggleHook", 0)`, call site 0x85b703.
- Resulting chain (INFERRED from call order): body 0xff --HeadHook--> head 0xfe --GoggleHook--> helmet item node.

## 4. Other callees and vtable slots

- `FUN_0085a1f0` callees: 0x576610, 0x71afa0, 0x71b3f0, 0x71b8b0, 0x733540, 0x733570, 0x733780, 0x734030, 0x76d160, 0x76d590, 0x77d800, 0x7ed830, 0x85b940, 0x85c150, 0x85cc00.
- `FUN_00868610` (0x868610, 197 bytes, one range). Callees 0x73f530, 0x8617e0.
  - Its item node `+0x50(body node, "GoggleHook", 0)` at 0x868685, gated on a flag at this+0x44.
  - Caller 0x869490: `procedure_callers` returns none. Trigger UNKNOWN.
- Vtable slots (arg shapes CONFIRMED from call sites, semantics INFERRED):
  - +0x78(out, slot, type): load model into slot.
  - +0x7C(slot): clear slot.
  - +0x98(slot, 1): get slot node.
  - +0x50(parent, name, 0): attach under named hook.
  - +0xAC(slot, mode, 1, name): set variant or texture, "DEFAULT" when no name.
  - Vtable targets were not decompiled, so exact semantics are UNKNOWN.

## 5. String literals

| Address | String | Evidence |
|---|---|---|
| 0x9a43e0 | GoggleHook | xrefs: 0x85b703 (FUN_0085a9e0), 0x868685 (FUN_00868610) |
| 0x9a43f4 | 0_HEAD | search_strings |
| 0x9a43fc | _HEAD | search_strings |
| 0x9a4404 | ALTTEXTURE | search_strings |
| 0x9a4428 | HeadHook | xref: 0x85d1ba (FUN_0085cc00) |
| 0x9a4438 | CSWCCreatureAppearance::CreateBTypeBody(): Failed to load model '%s'. | search_strings |
| 0x99ef86 | T@HEADHOOK | string CONFIRMED, no xrefs, role UNKNOWN |
| 0x991b40 | (DAT_00991b40, classifier set) | no Ghidra string, UNKNOWN |

- "helm" (case-insensitive literal): NOT FOUND.
- "%03u": not searched in this pass.

## Answers

1. `FUN_0085a9e0` contains 0x85B703. Caller `FUN_0085a6c0` (0x85a6c0), whose only caller is 0x76d200. Gate is classifier `FUN_0085c150` returning 3. The letter for value 3 is UNKNOWN; the code is CONFIRMED.
2. F/L: head item not attached via `FUN_0085a9e0`. Head slot cleared (CONFIRMED). `FUN_0085b940` F/L body path UNKNOWN. Head-variant swap NOT FOUND on resolved paths. Headhook placement for F/L NOT FOUND. For B, head placed at body HeadHook (CONFIRMED).
3. Strings as in the table above.

## Open

- Decompile `FUN_0085b940` (0x85b940): the F/L body path.
- Resolve indirect callers of `FUN_0085a9e0` and `FUN_0085b940` (vtable or thunk). Ghidra shows none.
- Resolve the caller of `FUN_00868610` (0x869490): none resolved.
- Map classifier value 3 to its letter (DAT_00991b40 content).
- Name the 2da columns behind DAT_00a0fde0 and rec+0x18.
- Sibling `engine_recon_p2.md` head was not read: this session has no file-read tool.
