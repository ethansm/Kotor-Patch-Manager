# Reduce: code,other

Verdicts: none
Cross-file findings: 0


## Clusters

- animLen-contract-change: animLen was changed to return true with a 2.0s fallback for names missing from the table, but its only caller in playFlourish still treats false as the not-in-table case, so that branch is now dead and the table-miss behaviour changed silently. (2 findings)

## Notes

No finding in this bundle is rated high or critical, so no verdicts were issued and no medium or low finding was re-verified, as the rules require. The code-001 and other-001 items are the most important to re-check in a later pass. The wheel-sync ordering item (9dd171cb3452) and the destroyCreature-before-rebuild item (6d18e325826a) are medium and unverified here. Cross-file analysis was not run, because the budget was almost exhausted and no high or critical finding justified reading across files. The bundle's domain header lists "code,other", so the domain field echoes that. The packet metadata describes the code domain as "all", which is inconsistent with the header and was not used.
