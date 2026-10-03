Title: `GameAPI: Match wrapper signatures to the engine`
Branch: `ethansm:gameapi/match-engine-signatures` (4 commits, c9d1aba..f19550c)

### Problem
Seven GameAPI wrappers call engine functions with a different signature from the one the engine uses. In each case the
callee pops more stack than the wrapper pushes, or reads `this` from a wrapper that never passes one, so a call would
leave the stack unbalanced or dereference a garbage ECX. Nothing in the tree calls these methods today, so this is
latent, but any new patch that uses them would crash or corrupt state.

| Wrapper | Before | Engine (K1 1.03, K2 GOG, K2 Steam) |
| --- | --- | --- |
| `CClientExoApp::DisplayMainMenu` | thiscall, 0 args | `ret 4`, the arg is never read |
| `CClientExoApp::ExitProgram` | stdcall, no `this` | thiscall thunk, `mov ecx,[ecx+4]` |
| `CClientExoApp::GetPausedByCombat` | stdcall, no `this` | thiscall thunk, `mov ecx,[ecx+4]` |
| `CSWCObject::GetPortrait` | 1 arg | `ret 8`: out resref + size selector byte (1-5 = T/S/M/L/H suffix) |
| `CSWCPlaceable::ActionMenuSecurity` | 1 arg | `ret 8`, reads neither arg |
| `Gob::EnableRenderBumpedOut` | 0 args | `ret 8`: texture name + float |
| `Gob::GetPartLocalPosition` | 2 args | `ret 12`: part name + the two out pointers |

### Approach
I checked each function's `ret N`, its argument reads and its callers in the disassembly of all three Windows builds
whose address DB carries the name: K1 GOG 1.03, K2 GOG and K2 Steam. Only rows where all three agree are changed. Each
wrapper gets the engine's typedef. Arguments the engine ignores are passed as 0, and `GetPortrait`'s new size argument
defaults to 0, which is what vanilla callers pass. No caller in `Patches/**` needs changing.

- `Patches/Common/GameAPI/CClientExoApp.h`, `CClientExoApp.cpp`
- `Patches/Common/GameAPI/CSWCObject.h`, `CSWCObject.cpp`
- `Patches/Common/GameAPI/CSWCPlaceable.h`, `CSWCPlaceable.cpp`
- `Patches/Common/GameAPI/Gob.h`, `Gob.cpp`

### Not changed here (K1 and K2 differ)
These wrappers match K1 but not K2. Their only in-tree callers are K1 patches, so fixing them needs a title switch
(`GameVersion::GetTitle()`). Happy to send that as a follow-up if you want the pattern:
- `CSWGuiPanel::StartLoadFromLayout` (K2 pops 12) and `InitControl` (K2 pops 16)
- `CGuiInGame::ShowPartySelection` (K2 pops 20) and `ShowUpgradeScreen` (K2 pops 24, with a by-value `CExoString`)
- The `Destructor` wrappers of `CRes`, `CResGFF` and `CSWGuiBorder`. On K2 the DB name `Destructor` is the
  scalar-deleting destructor (`ret 4` with a delete flag), so the wrapper's call plus `free()` skews the stack and can
  double free.
- `CGuiInGame::ShutDownGlobal`: both K2 rows point at a deleting destructor, not at ShutDownGlobal. This is a DB row problem.

Some DB `param_size` values also disagree with the code: GOG `ShowUpgradeScreen` should be 24 not 20, GOG/K1
`ActionMenuSecurity` should be 8 not 0/4, and K1 `DisplayMainMenu` should be 4 not 0.

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors; 3 existing version-format warnings)
- [x] `python3 publish-patches.py --target windows_x86` (25/25 packaged)
- [x] No in-tree callers of the changed methods
- No runtime test: nothing calls these wrappers yet.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
