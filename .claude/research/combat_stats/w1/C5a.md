# C5a: companion influence and exact alignment (Steam swkotor2.exe)

Recon only. Addresses are Steam. Names come from `patch_manager_mods/steam_db_batches/B0/final_rows.tsv` (GOG-derived, "confirmed") unless marked hypothesis.

| stat | what it means for the player | formula / parts | computed at (addr) | storage (struct+offset) | shown in vanilla UI? | how a patch could read it | confidence |
|---|---|---|---|---|---|---|---|
| Influence[i], i=0..11 | PC's sway over party NPC i's alignment | raw int32; 50 = neutral; -1 = unset (replaced by 2DA BaseInfluence on first HandleAlignmentInfluence); 0..100 enforced on set paths | GetInfluence 0x005fa9c0 (returns 0 if i<0 or i>11); SetInfluence 0x005fa9f0 | CSWPartyTable `this`+0x11c+i*4 | Only feedback text ("influence gained/lost", codes 9/10 via 0x0075cec0). No reader found that draws a bar | script `GetInfluence(i)` (id 795); or read this+0x11c+i*4 once table ptr known | high (offset, range); med (table ptr) |
| Set/ModifyInfluence rule | script 796/797 change Influence[i] | Set: clamp(v,0,100). Modify: cur=(cur==-1?50:cur); new=clamp(cur+d,0,100). If changed: feedback 9 (up) or 10 (down); if NPC managed: call HandleAlignmentInfluence | ExecuteCommandInfluence 0x00687230 (cases 0x31c/0x31d) | same as above | feedback only | script `SetInfluence`/`ModifyInfluence` | high |
| BaseInfluence (2DA) | starting influence per NPC | read column "BaseInfluence" from the influence.2da row matched by label; written back via SetInfluence when stored value is -1 | HandleAlignmentInfluence 0x0056ae40 (string @00995100) | 2DA object at CTwoDimArrays+0x138, set by Load2DArrays_Influence 0x006ed110 (resref "Influence") | No | load the 2DA via game accessor; no copy in repo | med |
| GoodEvil (exact alignment) | creature's exact good/evil value, 0=evil, 100=good | int16, clamped 0..100 | writers: ModifyAlignment 0x006ad400; ReadStatsFromGff 0x006afed0; HandleAlignmentInfluence 0x0056ae40 (writes at lines 335, 358) | creature `+0x1198` -> CSWSCreatureStats `+0x18a`; GFF field "GoodEvil" saved as CHAR (SaveStats 0x006b3d10) | Not verified. A bar is likely GUI-resource drawn; no code in dump reads +0x18a in ShowPartySelect 0x007cb920 | script `GetGoodEvilValue(oCreature)` (nwscript 125, -1 if invalid); or creature->+0x1198->+0x18a (int16) | high (offset, range); low (UI bar) |
| BaseCNPCAlignment | NPC's own base before influence | int16 stats+0x18c, clamped 0..100 in ModifyAlignment; GFF default = current GoodEvil when absent | ReadStatsFromGff 0x006afed0 (default -1 -> copy GoodEvil); SaveStats 0x006b3d10 | stats+0x18c; GFF "BaseCNPCAlignment" (CHAR) | No | stats+0x18c | high |
| Alignment recompute (NPC) | pulls an NPC's GoodEvil toward a PC-driven target | hypothesis: pc=(G_PC_Align_Val-50)/50; infn=(clamp(Influence+3*PCstats+0xf8,0,100)-50)/50; t=pc*infn; target = t>=0 ? base+t*(100-base) : (1+t)*base; GoodEvil=clamp(round(target),0,100) | 0x0056ae40 (Ghidra float compares obscure the sign test) | writes stats+0x18a | No | not patchable directly; trigger via ModifyAlignment | med (structure), low (exact sign/round) |
| G_PC_Align_Val | PC-side term of the influence formula | global variable read via CServerExoApp::GetGlobalVariableTable 0x0051c890 | 0x0056ae40 (decomp lines 91-96) | global-variable table, not the creature | No | read via global-var table by name | low-med (writer not traced) |
| Party save/load | persistence of influence | list "PT_INFLUENCE" of 12 structs, each dword "PT_NPC_INFLUENCE"; load copies min(count,12) with default 0xffffffff | SaveTableInfo 0x005fb1a0 (always writes 12); LoadTableInfo 0x005fbf60 | party table GFF | No | n/a | high |
| Script alignment API | designer-facing alignment | GetGoodEvilValue 125 (0..100); GetAlignmentGoodEvil 127 (ALIGNMENT_* enum); SetGoodEvilValue 750; AdjustAlignment 201 (shift capped at 50 per comment) | nwscript.nss lines 2674, 2682, 5486, 3065 | n/a | n/a | script calls | high (doc), med (code) |

## Evidence
- 0x005fa9c0 GetInfluence: `if ((param_2 < 0) || (0xb < param_2)) uVar1 = 0; else uVar1 = *(param_1 + 0x11c + param_2 * 4);`
- 0x005fa9f0 SetInfluence: stores `*(param_1+0x11c+param_2*4)=param_3`; if param_3==0 calls FUN_004010a0(0x25) (side effect unknown).
- 0x00687230 dispatch: 0x31b->GetInfluence, 0x31c->SetInfluence (clamp 0..100), 0x31d->ModifyInfluence; ids match nwscript 795/796/797 (`nwscript.nss:5719,5728,5738`).
- 0x006ed110 Load2DArrays_Influence: `FUN_00710840("Influence")`; result stored at `local_20+0x138`.
- 0x0056ae40 NPC index chain (tag strings, then nwscript NPC_* constants): 0 atton, 1 baodur, 2 canderous, 4 handmaiden, 6 kreia, 7 mira, 9 visasmarr, 10 hanharr, 11 disciple. Indices 3, 5, 8 unresolved.
- 0x0056ae40 `*(int*)(param_1+0x1198)+0x18a = sVar2` (lines 335, 358): NPC GoodEvil write-back. Creature->stats pointer is +0x1198; stats->owner is +0x24 (ModifyAlignment uses `*(param_1+0x24)`).
- 0x006ad400 ModifyAlignment: `*(short*)(param_1+0x18a)` and `+0x18c` both shifted by delta, clamped to 100 (lower clamp to 0 at line ~41-52); gated by `*(*(param_1+0x24)+0x1200)==0 && +0x11ac==0`.
- 0x006afed0 ReadStatsFromGff: `FUN_00718a00(param_3,"GoodEvil",...)` clamp 100; `"BaseCNPCAlignment"` default -1 -> copy of GoodEvil.
- 0x006b3d10 SaveStats: `FUN_007199d0(param_3,(char)+0x18a,"GoodEvil")`; `FUN_00719a40(...,(char)+0x18c,"BaseCNPCAlignment")`.
- 0x005fbf60 LoadTableInfo: `FUN_00718c80(...,"PT_NPC_INFLUENCE",...,0xffffffff)` into `param_1[i+0x47]` (0x47*4 = 0x11c).
- 0x0060b360 AddCNPCRequirements (CSWSItem): `FUN_005fa9c0(1)` (hard-coded index 1).
- nwscript.nss:2664-2674 GetGoodEvilValue "(100=good, 0=evil)"; :5716-5719 GetInfluence "not in party -> 0; Ambivalent -> -1".

## Open questions
- `0x0056ae40` entry guard `FUN_00734f60("kreia")`: polarity unclear, so Kreia's handling is unconfirmed.
- Table-pointer source: `0x0051c8d0` returns `*(obj+4)+0x1f0b4`, which is taken to be CSWPartyTable. Steam DB name "GetSwoopUpgradeRowProvider" looks wrong (review, med).
- GetInfluence doc says -1 means Ambivalent; code returns raw stored value, where -1 is the unset sentinel. Reconcile before relying on -1.
- Index 3, 5, 8 NPC names not resolved (strings 00995154/00995140/00995130 not in strings.tsv).
- Creature-template path (0x0076aed0) reads GoodEvil from `*(param_1+0x310)+0x80`, a different struct, unresolved. The GUI bar and the writer of G_PC_Align_Val are unverified. No influence.2da data in repo.
