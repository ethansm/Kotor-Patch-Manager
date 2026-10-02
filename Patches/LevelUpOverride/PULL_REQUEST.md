# Pull Request: Level-Up Override

### PR Title
`feat(progression): scripted per-companion level-up progression paths (#83, #31)`

### Commit Message
```text
feat(progression): scripted per-companion level-up paths

Hooks character level-up dispatch to enforce scripted companion feat,
power, and attribute progression defined via INI configuration.

- Patches/LevelUpOverride/manifest.toml: patch manifest (v1.0.0)
- Patches/LevelUpOverride/kotor2-steam-aspyr.hooks.toml: hook definitions
- Patches/LevelUpOverride/LevelUpOverride.cpp: LevelUp hook, path parser
- Patches/LevelUpOverride/levelup_paths.ini: example companion paths
- Patches/LevelUpOverride/README.md: configuration documentation
```

### PR Description Body
```markdown
### Summary of Changes
Hooks creature level-up logic to allow modders and players to define custom, per-character auto-level progression paths in an external INI file.

### Motivation & Background
Vanilla auto-leveling in KOTOR II relies on generic class 2DA tables (`featgain.2da`, `classpowergain.2da`) that grant suboptimal feats to companions (e.g. Droids taking useless weapon proficiencies). This patch provides fine-grained control over companion builds as requested in **#83 (Make hard-coded class and party abilities read from a 2DA/config)**.

### Technical Implementation
- **Hook Site (`0x006B9870`):** Intercepts `CSWSCreatureStats::LevelUp`.
- **Path Resolution:** Identifies companion by tag/NPC ID and looks up corresponding level table in `levelup_paths.ini`. Injects desired feats, Force powers, skills, and attribute allocations, logging transitions to `levelup_override_log.txt`.

### Testing & Verification
- [x] Tested auto-leveling across levels 1–20 on Atton and Bao-Dur.
- [x] Verified dry-run / log-only mode works safely without altering game memory.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #83, #31
```
