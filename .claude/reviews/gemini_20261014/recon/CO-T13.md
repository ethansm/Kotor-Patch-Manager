# CO-T13
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2c-01 | PARTIAL | patch_manager_mods/companion_ai_src/CompanionAI.cpp:53; patch_manager_mods/companion_ai_src/test.toml:6 | yes | Mismatch and unresolved include confirmed; whether KPM provides _shared unverified. |

- patch_manager_mods/companion_ai_src/CompanionAI.cpp:53 - quote include `../_shared/SafeRead.h`; exact anchor match.
- patch_manager_mods/companion_ai_src/test.toml:6 - common_contract_test cxxflags add -I../_shared_src, -I../../research/companion_levelup/tools/parity/stub, -DGUIKIT_HOST, -Werror.
- patch_manager_mods/companion_ai_src/test.toml:2 - default cxxflags = ["-Werror"]; no include-path flags at top level.
- Resolution: quote includes search the including file's dir first, so the target is patch_manager_mods/_shared/SafeRead.h; -I../_shared_src maps `../_shared` to that same sibling (compile cwd assumed companion_ai_src).
- patch_manager_mods/_shared: absent (existence check only; not a symlink; 0 git-tracked files; not git-ignored).
- patch_manager_mods/_shared_src/SafeRead.h: exists (existence check only).
- research/companion_levelup/tools/parity/stub: exists; its sibling _shared/SafeRead.h is absent, so the second -I does not rescue the include.
- patch_manager_mods/companion_ai_src/manifest.toml:1-13: [patch] metadata only (id companion-ai, version 0.2.0-probe); no staging, copy or _shared directive.
- Fix: `../_shared_src/SafeRead.h` from CompanionAI.cpp resolves in-tree via the file-relative lookup; KPM staging effect unverified.
- OPEN (outside ranges, not read): does the KPM build create patch_manager_mods/_shared; does common_contract_test compile CompanionAI.cpp; the actual test compile cwd.
