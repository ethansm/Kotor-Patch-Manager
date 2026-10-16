# kvoice: offline voice clip toolchain (doc 41, phase V-B)

Builds and checks KOTOR II voice clips (`.wav` in the 58-byte VO container + `.lip`) on the host. **Structure only: nothing here proves the engine plays a file** (that is live tests E1-E8, phase V-C). No auto-install into the game; the stager refuses any path inside the game dir.

## Usage
```
python3 patch_manager_mods/kvoice/kvoice.py probe-header <clip.wav>          # container, frames, duration, LUFS/TP
python3 patch_manager_mods/kvoice/kvoice.py clip <src_audio> --stage <dir> --resref kpv001 --source-id <id> \
        [--lip-tsv rhubarb.tsv | --no-lip] [--container vo58|bare]
python3 patch_manager_mods/kvoice/kvoice.py verify <stage_dir> [--shippable]
python3 patch_manager_mods/kvoice/kvoice.py ledger <stage_dir>
```
Stage layout: `<stage>/Override/<resref>.wav|.lip`, `manifest.json` (sha256), `ledger.json`.

## Modules
| file | job |
|---|---|
| `kvoice_container.py` | constant 58-byte header, `detect/strip/wrap`, MPEG-1 L3 32 kHz mono 48 kbps frame walker (216 B/frame) |
| `kvoice_audio.py` | ffmpeg decode, EBU R128 measure, libmp3lame encode |
| `kvoice_norm.py` | target -18.5 LUFS +/-0.3, true peak <= -1.0 dBTP, 3 passes, 150 ms lead-in, refuses silence |
| `kvoice_lip.py` | `.lip` write/check, Rhubarb A-H,X -> 16 KOTOR shapes (table derived, not engine-attested) |
| `kvoice_ledger.py` | `k1_port` ledger rows (kind `audio`); forces personal-only/local-only for `vanilla_*` parents |
| `kvoice_pack.py` / `kvoice_verify.py` | stage writer; rules R1-R9 |
| `kvoice.py` | CLI |

## Rules R1-R9 (verify)
R1 RIFF@0 · R2 no RIFF@0x1D6 · R3 header == `build_header()` · R4 frame chain reaches EOF at 32k/mono/48k · R5 size = 58+216*frames · R6 ffmpeg decodes · R7 `.lip` valid · R8 lip length within 36 ms of decoded duration · R9 stage: `kpv` resref `[a-z0-9_]{1,16}`, wav+lip paired, sha matches manifest, ledger row per file, no `local_only` in a shippable stage.

## Behaviour notes
- **Peak-limited clips**: vanilla VO is peaky (e.g. -19.9 LUFS / -0.5 dBTP). The -1.0 dBTP ceiling is hard, so such a clip lands quieter than target (up to 3 LU under) and the norm report has `tp_limited: true`; anything quieter or over the ceiling is refused.
- Output duration = source + 0.15 s lead-in, rounded up to a whole 1152-sample frame (up to ~36 ms).
- `clip` requires `--source-id` (the CLI never guesses provenance). Vanilla-derived audio is personal-use only (`vanilla_k2` in `k1_port/sources.json`).
- `--container bare` emits a bare MP3 for the E2 matrix; `verify` flags it as R1 by design.

## Tests
`python3 scripts/run_tests.py -k kvoice` (synthetic fixtures; vanilla controls SKIP when the game is absent; `KVOICE_VANILLA_VO` / `KVOICE_VANILLA_VO58` select a control clip).

## Next
V-C live probe (E1/E2/E3/E4/E7/E8): needs user GO, a forged save, and the D0 live-run pause lifted. See doc 41 section 6.

## Live lookup observer and Juhani tests
`python3 scripts/override_watch.py --match <regex> [--log F]` logs every engine OPEN in override/StreamVoice/StreamSounds/lips (inotify, no hook). Juhani dialog/lip/bark live tests: `V-D_JUHANI_RUNBOOK.md` (doc 38 §21).
