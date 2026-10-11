# S1 setup recon: --client claude_code

| Item | Finding | Anchor |
|---|---|---|
| a | Writes `mcpServers.rea` in `$CLAUDE_CONFIG_DIR` or `~/` `.claude.json` | dist/application/SupportedClients.js:104; dist/application/SetupClientConfiguration.js:47 |
| a | Entry `{command, args, env?}`: `npx -y rea-agents@6.3.0 mcp` or CLI path plus `mcp` | dist/application/ClientConfigurationDocument.js:1105-1109; dist/application/SetupHost.js:15-23 |
| a | `env` holds runtime provider keys | dist/application/SetupHost.js:65-70; dist/ghidra/GhidraDoctor.js:13-18 |
| a | No write to `~/.claude/settings.json` | rg `settings.json`: only dist/application/SupportedClients.js:132 (Gemini) |
| b | Only `$CLAUDE_CONFIG_DIR` or `~/.claude`/`skills/reverse-engineer-anything/`; `~/.agents` skipped | dist/application/SetupSkill.js:14,16-24,30-36; dist/cli/setupCommands.js:124 |
| b | `SKILL.md` plus five `references/*.md` files | dist/application/SetupSkill.js:5-12 |
| c | `~/.claude.json.rea.backup`: only if file exists and entry not current; never overwritten | dist/application/SetupClientConfiguration.js:38-43,149-156 |
| c | Skill: `<file>.rea.backup` beside each changed existing file | dist/application/SetupSkill.js:86-89 |
| d | `--dry-run` returns `status: "planned"` before any write; plan code has no mkdir/write/copy/rm | dist/application/Setup.js:117-128; dist/application/SetupPlan.js:160-188 |
| e | Removes REA-owned `mcpServers.rea` (backup kept) and both skill dirs; `~/.rea` only with `--purge-data` | dist/application/Uninstall.js:141-170,228-229,83-84 |

OPEN:
- Provider keys present in `mcpServers.rea.env` (runtime).
- Dry-run stdout format: not traced.
