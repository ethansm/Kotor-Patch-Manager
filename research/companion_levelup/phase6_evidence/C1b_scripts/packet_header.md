# C1b blind verification packet (R10 level-up UI identities, KOTOR II Steam/Aspyr swkotor2.exe)

You are a verifier (Haiku sieve, tier 1 of RE_AGENT_GUIDE.md). Each CARD has a CLAIM (an address with a proposed name, vftable
or hook-site description) and EVIDENCE for the address named in the claim. Roughly two thirds of the cards are deliberately
WRONG (wrong address, wrong name, wrong vftable, wrong exit, wrong callee). You are not told which. A wrong claim sits next to
evidence that contradicts it; look for the contradiction.

## Rules (from RE_AGENT_GUIDE.md sections 3 and 4)
1. Assume the claim is a NEIGHBOURING function/site with the same signature. You are FORBIDDEN to answer `match` on `ret N`,
   size, prologue or name alone. To answer `match` you MUST cite at least one CONTENT ANCHOR from the evidence shown: a string
   literal (address + text), a confirmed callee (address + name from the ANCHORS list), or a member offset / instruction
   with its asm line. If you cannot, answer `unsure`.
2. Do NOT do hex/decimal or stack math. TIER-0 FACTS are computed for you. Bare numbers in decompile are DECIMAL; the script
   has added hex and scaled byte offsets: `int *p; p + 100` = byte +0x190 (use the `{int*: byte +0x...}` note). Asm operands are always hex.
3. Thiscall thunks print without `__thiscall`; ECX passes through. Ghidra auto labels (FUN_*, FID_*) are not evidence.
4. A claimed NAME must be supported by what the code does, not by the claim's wording. If the code does the sibling thing
   (feats vs powers vs skills; ctor vs dtor; add-row vs sort; call to the feat test vs the spell test) answer `mismatch`.
5. For hook-site cards: check that the claimed address is an instruction START whose bytes equal the claimed bytes, that the
   register role is what the preceding instructions establish (quote them), and that the exit target is the claimed kind of
   instruction. A different address, bytes or exit than claimed = `mismatch`.
6. Absence is not proof. `unsure` is always allowed and is not penalised.

## Few-shot negatives
- Neighbour pair: 0x5644B0 and 0x564770 both were called AddMoveToPointAction with the same RET; 0x5644B0 calls AddActionToFront
  (005645e4 CALL 0x00540ca0), 0x564770 calls AddAction (005648a4 CALL 0x0053f7f0). The callee is the anchor.
- Bool arg + RET 4: `Stack Slot Match: TRUE (GOG 1 byte -> 1 slot; ASM RET 4)` is normal, not a mismatch.
- Decimal + scaled offset: `*(short *)(param_1 + 100)` with `int *param_1` is byte offset 0x190, asm `MOV word ptr [EDX + 0x190],AX`.

## Output: one JSON object per card, in this order (verdict LAST)
```json
{"card": "C000", "observed_prologue_or_bytes": "55 8b ec", "content_anchor": "PUSH 0x9965f8 'text' at 005cbcb5 | callee 0x008480B0 | asm line",
 "neighbour_decoy_considered": "<what else could this be, and why not>", "auto_label_ignored": true,
 "confidence_1_to_5": 4, "verdict": "match | mismatch | unsure", "reason": "<= 20 words"}
```
Known DB facts you may rely on: 0x8480B0 = CSWCLevelUpStats::CanChooseFeat, 0x847E50 = CalcNumberFeats, 0x847F90 =
ComputeNumberKnownSpellsLeft, 0x8489F0 = AddDefaultPackageFeats, 0x848AE0 = AddDefaultPackageSpells, 0x593450 = CExoArrayList<uint16>::Add,
0x733780 = CExoString::~CExoString, 0x73F550 = CClientExoApp::GetCreatureByGameObjectID.
Everything else in the CLAIMs is unverified. Answer every card; do not skip.
