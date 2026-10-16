# V-D runbook: Juhani dialog voice + lip + bark tests (doc 38 §21, doc 41 §6 E3/E4/E2d)

State (10-16): **everything staged offline, nothing installed.** Needs per test: explicit user GO, game closed, no save inside `204tel` while the rig is installed. K1 audio stays local (private use only, never in the repo).
Stage: `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/char_juhani_audio/{override,streamvoice,override_bark,utc/{dialog,bark,both}}` (rebuild: `k1_port_audio_test_build.py`, `k1_port_bark_build.py`, k1port venv).
Models: use whichever body variant the arm test settled on (below: `probe_mdlops_juhani3`).

## Common setup (each test)
```
K2="$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"; P=patch_manager_mods/k1_port/k1_port_probe.py
S=char_juhani/override; M=probe_mdlops_juhani3/models; A=char_juhani_audio
BASE="--base-sha256 44427a3575a2d05374ca301757c372b7b10f0e7ede8b582241d438835f488b91 --base heads.2da=e0f8381cd43fe07e36c5c1e9339cca56ceb5039666ea24142646fab9c493fb9d"
MODELS="$M/k1p_juhaniba.mdl $M/k1p_juhaniba.mdx $M/k1p_juhanibb.mdl $M/k1p_juhanibb.mdx $M/k1p_juhanih.mdl $M/k1p_juhanih.mdx"
python3 $P rollback --game-dir "$K2" --yes          # remove the current rig first (CR *.tga stay)
python3 scripts/override_watch.py --match 'p_juhani|kvj' --log /tmp/kvj_watch.log &   # BEFORE launching the game
```
After the test: `fg` the watcher + Ctrl-C for the summary. The watcher shows which paths the engine OPENed; a staged name with no OPEN was never looked up.

## T1: dialog, voice from Override, lip valid / none / corrupt (E3 + Override-voice)
```
python3 $P install --game-dir "$K2" $BASE --files $S/appearance.2da $S/heads.2da $A/utc/dialog/k1p_juhani.utc $MODELS \
  $A/override/kvjdlg01.dlg $A/override/kvjv0001.wav $A/override/kvjv0002.wav $A/override/kvjv0003.wav $A/override/kvjv0001.lip $A/override/kvjv0003.lip --yes
```
Play: forged save (kotor2-forced-state-testing) in `204tel`, click Juhani, press Continue twice. Report per line: subtitle shown? voice audible? mouth moves?
Pass table:
| line | files | question |
|---|---|---|
| 1 `kvjv0001` | wav + valid lip | voice plays from Override? lip used (watcher OPEN of `kvjv0001.lip`)? |
| 2 `kvjv0002` | wav, no lip | still plays without lip? |
| 3 `kvjv0003` | wav + corrupt lip (`BAD ` magic) | crash, silent, or tolerated? |
If the wav is OPENed but silent: repeat with `streamvoice/kvjv0001..3.wav` copied by `cp` into `"$K2/StreamVoice/"` (out-of-band, user GO), remove them afterwards by explicit name.
If no OPEN of any `kvjv*`: the dialog did not start (check `kotor2_logs.py check`, never raw logs) or the lookup path differs: dump the watcher summary.

## T2: bark through soundset -> SSF -> TLK -> wav (E4 + E2d)
Row 70 is appended to `soundset.2da` (a replace; base sha from `sha256sum "$K2/override/soundset.2da"`, build time value `fb818124...`).
```
SSH=$(sha256sum "$K2/override/soundset.2da" | cut -d' ' -f1)
python3 $P install --game-dir "$K2" $BASE --base soundset.2da=$SSH --files $S/appearance.2da $S/heads.2da $A/utc/bark/k1p_juhani.utc $MODELS \
  $A/override_bark/soundset.2da $A/override_bark/kvjssf.ssf $A/override_bark/p_juhani_slct1.wav $A/override_bark/p_juhani_slct2.wav $A/override_bark/p_juhani_slct3.wav --yes
```
Play: fresh visit or forged save (saves keep the UTC snapshot), click Juhani several times. Mapping of the SELECT slots (K2 TLK rows 36920..36922 keep K1 text "Your wish?", "Yes?", "What is it?"):
| file | content | proves |
|---|---|---|
| `p_juhani_slct1.wav` | 440 Hz tone (PCM) | the SSF -> TLK SoundResRef -> Override chain works at all (E4) |
| `p_juhani_slct2.wav` | K1 original, SFX layout (MP3 + RIFF@470) | E2d: does the SFX layout play from Override |
| `p_juhani_slct3.wav` | same | second sample; later a StreamSounds copy candidate |
Fail splits: no OPEN of any `p_juhani_slct*` = clicking an NPC does not bark (try combat/party route, a later test) or the soundset row was not applied (fresh spawn needed); OPEN but silent = layout/path issue (repeat in `"$K2/StreamSounds/"`).

## T3: both (only after T1 and T2 pass)
Install with `$A/utc/both/k1p_juhani.utc` plus the union of the T1 and T2 file lists. Batch 4 then trims the dialog to the final three lines.

## Cleanup (every test)
```
python3 $P rollback --game-dir "$K2" --yes
ls "$K2/override" | rg -i 'kvj|p_juhani'      # expect nothing
```
Then record the verdicts in doc 41 §6 (new rows E3 / E4 / E2d / Override-voice) and doc 38. Remove any manual `StreamVoice/kvj*` or `StreamSounds/p_juhani_*` copies by explicit name.
