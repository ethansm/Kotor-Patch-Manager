# voice_probe_src/nss: KPV probe scripts

Purpose: NSS triggers for the voice test ladder (`.claude/research/voice_audio/w3/b8.md`, TEST LADDER). Each script sends one `kpv:` line to the PC with `SendMessageToPC`, then runs one probe. Not compiled yet.

## Scripts
- `kpv_e1.nss`: `PlaySound("kpve1a")`, loose-Override route (E1).
- `kpv_e2a.nss`: `PlaySound("kpve2a")`, container a, RIFF PCM (E2).
- `kpv_e2b.nss`: `PlaySound("kpve2b")`, container b, 58-byte fake header + MP3 (E2).
- `kpv_e2c.nss`: `PlaySound("kpve2c")`, container c, bare .mp3 (E2).
- `kpv_e2d.nss`: `PlaySound("kpve2d")`, container d, 470-byte prefix + RIFF PCM (E2).
- `kpv_e4.nss`: `AssignCommand` + `ActionSpeakStringByStrRef` on the nearest living creature, strref 36878 (E4).
- `kpv_e7a.nss`: 3x `PlaySound("kpve1a")` in one call (E7).
- `kpv_e7b.nss`: 8x `PlaySound("kpve1a")` in one call (E7).
- `kpv_e8.nss`: "KpvE8Mixed" at 0 s, "kpve8_sixteenchr" (16 chars) at 3 s, "kpve8_seventeench" (17 chars) at 6 s (E8).

## Open
- Does PlaySound animate lips? E3 (lip) and E6 get NO script yet; decide after the E1 result (may need a dialog route).
- `kpv_e4.nss` uses TLK strref 36878 ("Let's move together again.", SoundResRef p_bastila_rprty). The wav is not at the StreamVoice or StreamSounds top level: confirm where the resref lives (BIF or subdir) before the live run, else pick another row.

## Added 2026-10-16 (live E1/E2 session)
- `kpv_c1.nss`: runscript control (credits + overhead text). `runscript` itself never ran any script, so it is only useful on a working trigger.
- `hb/k_hen_heartbt01.nss`: TEST override of the vanilla henchman heartbeat, the only trigger that worked (fires every ~6 s per companion, hot-swappable). It applies to EVERY save while staged, is not removed by `rm kpv*`, and puts a file in `Override`: retired by the no-loose-files rule (doc 44). Do not stage it again; kept as the record of how E1/E2 were run.
