# K2TextureMemoryRelease

Frees the CPU-side copy of every TPC texture once it has been uploaded to OpenGL, as the engine already does for TGA textures. This gives back memory in a 32-bit process that is usually close to its limit on long sessions and heavy texture packs. See Testing for a measurement.

Supported build: Steam (Aspyr), `kotor2_steam_aspyr`. Developed and played under Proton on Linux.

## Cause

`CAurTextureBasic::BuildAndStoreTexture` (`0x004260E0`) builds a texture, uploads it with `glImage`, and then frees the in-memory image so only the GL copy remains. It only does that for non-TPC textures:

```
0x004261CD  call [vtable+0x30]          ; IsTPCLoaded, AL = 1 for a TPC texture
0x004261CF  movzx ecx, al
0x004261D2  test  ecx, ecx
0x004261D4  jnz   0x004261E6            ; TPC: skip the free
0x004261D6  ...   call [vtable+0xB4]    ; FreeImage
0x004261E6  mov esp, ebp / pop ebp / ret
```

A TPC texture uses the resource manager's buffer in place (`tex+0x44`) and keeps it, so after the upload the pixels live in memory twice: once in OpenGL and once in the texture object. In my measurement that came to about 195 MB of pinned TPC pixel data.

## Fix

One `replace` hook at `0x004261CF` over the seven bytes `movzx / test / jnz`. The new code:

```
movzx ecx, al
test  ecx, ecx
jz    free                      ; not TPC: free, as before
mov   edx, [ebp-4]              ; this
cmp   dword ptr [edx+0x34], 0
jz    free                      ; TPC without a texture controller: free
push  0x004261E6                ; TPC with a controller: skip the free, as before
ret
free:                           ; the hook jumps back to 0x004261D6, which calls FreeImage
```

So TPC textures are freed after upload, except ones that have a texture controller (`tex+0x34 != 0`). Those are animated or procedural textures, and the controller functions (`0x48FF30`, `0x491C40`, `0x492310`, `0x492460`, `0x4907F0`, `0x490710`) read the image data every frame, so they keep it exactly as vanilla does.

There is no DLL. The hook is a raw `replace` block, and the original bytes are verified before it is applied.

## Why freeing is safe

I checked the code paths statically in the Steam exe:

* `EAX`, `ECX`, `EDX` and the flags are dead at both `0x004261D6` and `0x004261E6`, and nothing else branches into the seven replaced bytes.
* `FreeImage` (`0x00425160`) is safe to call on any texture: it returns at once when `tex+0x44` is already 0.
* Every path that needs the pixels again either skips the upload when `tex+0x44 == 0` (the GL texture is left alone, it never goes black) or goes through `Reset`, then `ConstructImage`, which asks the resource manager for the data again. That covers requeue, quality or downsample changes, and window or GL context re-creation (`0x409750` to `0x476BE0` to `0x426C30` to `0x427DE0`), which already frees and reloads the retained TPC buffer in vanilla.
* The only readers of the image after upload are the `GetImage` family: the four upload routines and the six texture-controller functions. Screenshots and save thumbnails use `glReadPixels`, and the exe does not import `glGetTexImage`.

## Testing

* Steam (Aspyr) `6A522E71...`, Proton on Linux. GOG, Windows native and Linux native are not covered.
* The same change, written as a DllMain code cave instead of a hook, has been on in my full install since 2026-10-15 across many play sessions. One session logged 2038 TPC textures freed and 17 kept because they had a controller, with no warnings in the log.
* The hook version in this patch is a different mechanism for the same bytes. I have played it over several long sessions on top of my full install. The longest one I have logs for ran about 90 minutes, with roughly 49 area loads and transitions and 34 combats, and had no crash, hang or load stall. I have one report of a room in Nar Shaddaa (Dock apartments) whose walls lost their textures. It also happens with the patch disabled, so I do not attribute it to this patch.
* Process memory (MangoHud PMEM), measured once each on the same save and spot after a fresh game load: 1.0 GiB with the patch disabled, 435 MiB with it enabled. That is about 0.6 GiB less, which is larger than the roughly 195 MB of pinned TPC pixel data. I have not explained the difference yet, and one reading per state is not a benchmark.

## Configuration

None. Uninstall the patch to turn it off. If another mod patches `0x004261CF`, the hook refuses to apply because the original bytes no longer match.
