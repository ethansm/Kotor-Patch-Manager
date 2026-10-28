# INGEST I2 cards (2026-10-27)

- Built: `ingest_names.py cards --in rea_findings/20261027_ingest-modcode.tsv --out-dir .claude/research/ingest/i2_cards --seed 1027`
- Scored: `ingest_names.py score --key key.tsv --verdicts 'verdicts_*.tsv' --out promote.tsv` (`verdicts_r2_*` = retry, sorts last and wins)
- Pass 1 (`verdicts_[1-8].tsv`): decoys 10/17 rejected, 7 `unsure`, 0 accepted.
- Retry chunks 3/5/6/7 with generic role calibration: 14/17 = 82% PASS; 2 decoys accepted (C31 GenderReq2Val, C34 kAddrResrefInit).
- `promote.tsv` 62 rows. Promoted only on the retry (weaker, review first at I4): C25 AddrShadowDist, C36 KeyboardDeviceGlobal,
  C49 SceneAddModel, C54 GenderReq2Val, C60 AddrPathN, C63 AddrCutFlag, C72 G_GlFinishFlag, C73 kAddrResrefInit,
  C79 AddrSoftShadows, C81 PreviewModelInstName, C84 GuiSceneCtor.
- Held back: FnPartyTable, AddrPathC60FC, SendContainerClose, HudTargetInvoke, TexMgr::Tick, SceneRemoveModel, AddrPathC6118.
- Lesson: function decoys are weak (2/4); bare-constant names (AddrPath*) carry no role, so decoys on them are unfalsifiable.
