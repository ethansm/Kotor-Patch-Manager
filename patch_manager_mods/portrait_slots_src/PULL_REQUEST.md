# Pull Request: Portrait Slots (service provider)

DRAFT, not sent. Full PR text is written when the upstream PR is prepared (service-provider category: doc 21, KPM branch feat/library-patches).

### GOG support callout (required text, rule of 2026-10-03)
Every issue, PR or RFC #180 comment that mentions the shared service providers (portrait-slots, `k2.portraitslots`) or GOG support for portrait-slots / buff-duration-hud / shield-bar must say plainly:

> GOG support is **static-verified only** (no GOG install here: hook bytes and addresses checked against the GOG exe in Ghidra, see doc 21b). **GOG contributor testing is required** before it is considered supported.

What a GOG tester should check and send:
1. Install through KPM: portrait-slots shows under "Service providers" and is auto-added; `patch_config.toml` lists it first; the patcher log has every hook `Applied`.
2. HUD: buff/debuff icon columns with draining durations on the party portraits; shield bar beside the health bar when a shield is up.
3. In-game menu: companion portrait gauges/bars and buff icon columns on every menu tab; nothing drawn outside the panels.
4. Portrait hover tooltip shows `Shield: cur/max` while shielded.
5. Logs: `kpatch.log`, `buffhud_log.txt`, `shieldprobe_log.txt`, the patcher log, plus a screenshot of HUD and one menu tab.
