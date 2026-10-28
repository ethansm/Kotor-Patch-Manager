# B18 Opus gate part a (lead, inline)
Checked every row against GOG/Steam fp.tsv (size/retn/n_insn/disp/imm identical on all 12) plus callers/callees mapped through trusted name_map pairs.
- fptwin_order pairs: StopAllOneShots 0x70bb00 (mapper pick 0x70b8f0 was wrong), AurEnableForceFury 0x478f50 (2/2 callers vs sibling 1/2), AurMultiTextureAvailable 0x484710 (4/4), InsertModel 0x48e130 (callee hit) hold.
- FrameBufferModifications -> likely: exact twin, but GLRender FBM labels rotated in B17; name stays a GOG hypothesis.
- RenderDrawableSquares stays unresolved: 0x435720 is the FBM twin (1885B), GOG RenderDrawableSquares is 54B.
sample_errors: 0
