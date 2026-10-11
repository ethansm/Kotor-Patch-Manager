# Reduce: all

Verdicts: downgraded 1
Cross-file findings: 0

## Verdicts

| finding | status | sev | conf | reason |
|---|---|---|---|---|
| ba03658d665e | downgraded | medium | medium | Confirmed that animTable() returns nullptr with n=0 for rows 3, 538 and 539 (EquipCharPreview.cpp:592), and that animLen() now returns true with *len=2.0f on every path (lines 596-601). The caller at line 762 therefore never takes its 'not in table' branch, so unmapped flourishes now play with a 2.0 |

## Clusters

- destroy-before-gate: The character-change branch in the preview build path calls destroyCreature() before the allocation, readApp and gate checks, so any later failure or zero gate leaves the panel with no creature and no rebuild. (2 findings)
- unmapped-anim-fallback: animLen() was changed to return true with a 2.0s default for every name, which removes the caller's 'not in table' rejection path for rows with no static animation list. (1 findings)
- reloadplay-changed-flag: changed() sets the primed and last state before returning true, so the caller's recomputed 'changed' flag is always false when it reaches the log gate. (1 findings)
- party-leader-lookup: leaderCreature() now derives the party table from the party helper walk instead of the engine ClientPartyTable call, so its result may differ from the removed code path. (1 findings)
- tautological-or-unthrottled-tests: Test assertions are either always true (taking the address of a function and comparing with nullptr) or depend on file-stamp timing that the test does not control, so they cannot fail as written. (3 findings)
- writethrough-not-shared: ptWriteText keeps a hand-rolled MoveFileExA with MOVEFILE_WRITE_THROUGH because the shared atomicWrite has no write-through option, so the save path does not use the shared helper. (1 findings)

## Notes

Only one finding in the bundle was high or critical (ba03658d665e), so it is the only verdict. It was downgraded to medium because the old animLen behaviour is not in the snapshot. The remaining medium and low findings were not re-verified, as the rules require. Cluster membership groups findings by suspected root cause from the bundle text, not by reading their code. The 'domain' field echoes the bundle header value 'all'. No cross-file findings were produced because the reads were limited to the one verified finding.
