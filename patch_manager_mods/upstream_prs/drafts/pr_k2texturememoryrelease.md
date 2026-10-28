# PR draft: K2TextureMemoryRelease (NOT OPENED)

Target: `LaneDibello/Kotor-Patch-Manager` `master`
Head: `ethansm:patch_add/k2texturememoryrelease` (local worktree `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-texmem`, based on fork base 55725f7 because the fork token lacks the `workflow` scope)
Title: `K2TextureMemoryRelease: Free TPC texture memory after the GL upload`

Pre-checks done 2026-10-28 against upstream `eaa7bda`:
- `tools/validate-patches.py` from upstream master: 68 patches, 0 errors, no warning for this patch.
- `create-patch.py` packages it (hook-only, 1573 B `.kpatch`).
- No other patch or database row mentions 0x004261CF or BuildAndStoreTexture.
- The diff against the base adds only `Patches/K2TextureMemoryRelease/` (manifest, hooks, README).

## Body

Adds `K2TextureMemoryRelease` for the Steam (Aspyr) build of KOTOR 2.

### Vanilla behaviour

`CAurTextureBasic::BuildAndStoreTexture` (`0x004260E0`) uploads a texture to OpenGL and then frees the in-memory image, but only for non-TPC textures. A TPC texture uses the resource manager's buffer in place (`tex+0x44`) and keeps it, so every uploaded TPC texture stays in memory next to its GL copy. I measured about 195 MB of pinned TPC pixel data. In a 32-bit process that is a lot of the address space on long sessions and with texture packs.

### The change

One `replace` hook at `0x004261CF` over `movzx ecx, al / test ecx, ecx / jnz 0x004261E6`. TPC textures are now freed after the upload, except ones with a texture controller (`[tex+0x34] != 0`). The controller functions read the image every frame, so those keep it exactly as vanilla does. There is no DLL, no log and no config. The hook verifies the original bytes before it writes.

### Why freeing is safe

I checked the paths statically in the Steam exe. The README has the full list. In short:

* `EAX`, `ECX`, `EDX` and the flags are dead at both jump targets, and nothing else branches into the replaced bytes.
* `FreeImage` (`0x00425160`) returns at once when `tex+0x44` is already 0.
* Every path that needs the pixels again either skips the upload when `tex+0x44 == 0` or goes through `Reset`, then `ConstructImage`, which asks the resource manager again. That covers requeue, quality changes and window or GL context re-creation.
* The only readers of the image after the upload are the `GetImage` family: the four upload routines and the six texture-controller functions.

### Tested

* Steam (Aspyr) `6A522E71...`, Proton on Linux. GOG, Windows native and Linux native are not covered; I have no way to test them.
* The same change as a DllMain code cave has been in my full install since 2026-10-15 across many sessions. One session logged 2038 TPC textures freed and 17 kept because they had a controller, with no warnings.
* This hook version: several long sessions on top of my full install. The longest one I have logs for ran about 90 minutes, with roughly 49 area loads and transitions and 34 combats. No crash, hang or load stall, and no black or missing textures.
* `tools/validate-patches.py` and `create-patch.py` pass locally.
* I have not measured the end-to-end change in process memory with this exact build. The 195 MB figure is the size of the pinned TPC data without the patch, so please read it as an upper bound on the saving.

Screenshots: none, there is no visual change.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
