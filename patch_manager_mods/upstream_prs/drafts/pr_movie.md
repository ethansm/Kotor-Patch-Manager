Title: `Movie Patch: Support the Steam Windows build`
Branch: `ethansm:port/movie-patch-k2-steam` (8cc2647, on master 6374f73). Open only after an in-game test on Steam.

### Problem
Movie Patch supports K1 and K2 GOG. For K2, the C++ hard-codes the GOG import-table slots of `BinkBufferBlit` and
`BinkCopyToBuffer` and the GOG screen width and height globals (`Kotor2GogMovieFilterAddresses`), so a Steam hooks
file on its own would not work.

### Approach
- I found both K2 detour sites in the Steam exe (`6A522E71...BBFFEF`):
  - the Bink blit call sequence at 0x40D901 (the same 30 bytes as GOG apart from the import-slot operand);
  - the movie player's shutdown entry at 0x40CB20 (byte-identical to GOG).
- The movie pointer is read from the same frame slot (`[ebp-0xE0]`) as on GOG.
- Import slots: identified by name from the Steam import table (`_BinkBufferBlit@12` at 0x9864A0,
  `_BinkCopyToBuffer@28` at 0x986494). Screen globals: the function that stores the movie resolution writes the
  width and height globals in the same order on both builds (Steam 0x9F42A4 and 0x9F42A8).
- A new `Kotor2SteamMovieFilterAddresses` table sits next to the GOG one. The K2 export picks one from
  `KOTOR_VERSION_SHA`, which KotorPatcher sets before loading patches. Any other build keeps the GOG table. The K1
  hooks and code paths are untouched.

Files:
- `Patches/Movie Patch/kotor2-steam-aspyr.hooks.toml` (new)
- `Patches/Movie Patch/better_movie_playback.h`: `Kotor2SteamMovieFilterAddresses`
- `Patches/Movie Patch/export.cpp`: the K2 export's table selector
- `Patches/Movie Patch/manifest.toml`: `kotor2_steam_aspyr` target, 1.6.0

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] `create-patch.py` builds; no new `-Wall -Wextra` warnings
- [x] K1 `hooks.toml` and K2 GOG `kotor2_hooks.toml` unchanged
- [ ] Tested on Steam under Proton: TBD

🤖 Generated with [Claude Code](https://claude.com/claude-code)
