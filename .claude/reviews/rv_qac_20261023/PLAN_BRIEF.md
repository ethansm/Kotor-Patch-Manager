# rv-qac plan-spec brief (Sonnet planners)

Input: `patch_manager_mods/47_queue_hotbar_review.md` (adjudicated issues, Opus decisions are final) and the validated detail in
`.claude/reviews/rv_qac_20261023/v01-v04.md` (V-ids). Engine facts: `.claude/reviews/rv_qac_20261023/haiku/BRIEF_COMMON.md`.

You write an EXECUTOR-READY implementation spec for ONE phase (named in your prompt). The executor will run after a context
clear and must not need to rediscover anything. Read the code you change (targeted `sed -n`/`rg -n`, never whole files).

Spec file format (markdown, write ONLY your output file):
```
# <Phase id> spec: <title>
Mod(s) + version bump: <id> <old> -> <new> (manifest.toml, ini header, version log string)
## Changes (one subsection per issue id)
### <RQn/RHn/RLn>: <title>
- Files/anchors: path:line (function name) for each edit
- Before (<=8 lines, exact current code) / After (exact new code, complete enough to paste; new pure helpers in full)
- Edge cases handled: pause/unpause, leader switch, combat edge, save/load, null/unreadable pointers, tick wrap
- Logging: exact new/changed log line formats (trace vs default level)
## Tests
- Unit: test function name, file, exact vectors (inputs -> expected), where it is called from main
- PLAY rows for patch_manager_mods/PLAY_TEST_QUEUE.md: id, steps, pass signal (log marker)
## Tooling
- exact commands: build/test (`python3 scripts/run_tests.py <scoped name>`), `python3 scripts/compat_check.py <files>` if _shared_src touched,
  `scripts/pre_install_gate.sh --mod <x>_src`
## Risks / ordering
- conflicts with other phases' edits (same functions), and the order to apply within the phase
```
Rules: keep fixes minimal and in the style of the surrounding code (comment density, naming). Pure decision logic goes in the
*Logic.h header with unit tests; engine glue in the .cpp. No new engine addresses unless already in GameAddr.h / the repo
(cite where). If an Opus fix in doc 47 is wrong or unsafe after reading the code, say so in a "Deviation" bullet with the reason
and your replacement. Keep the spec under ~350 lines.
Find the test runner names: `python3 scripts/run_tests.py --list 2>/dev/null | rg -i '<mod>'` (or rg the runner).
Reply with exactly the 5-line RECON block (OUTPUT, EVIDENCE, FINDING, OPEN, VERDICT); no prose. Read-only except your output file;
no builds, no git.
