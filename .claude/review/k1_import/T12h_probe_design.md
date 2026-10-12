# T12h: smallest reversible in-game probe for one k1p_* NPC

Status: DESIGN ONLY (10-11). Nothing installed, no game file written, no git. Every number below was read from disk this session.
Scratch proof: the two generated files were built into the session scratchpad (`.../scratchpad/T12h/o2/`), not into the game dir.

## 1. Choice: candidate (a)

(a) one appended `appearance.2da` row 812 plus ONE `.utc` override. (b) is strictly larger: it needs the same 2DA row, plus a C++ edit, a regenerated `DiversityRows.h`, a rebuild, a gate run and a DLL hot-swap, and it touches the live dispatcher.
(a) touches no code. The live dispatcher (0.1.4-probe) cannot interfere with row 812: `ShouldSkipDescriptor` returns `NotCommonerRow` (812 is in none of the `IsGeneric*` tables), `FamilyOf(812)` is `None` so `PickTextureVar` writes nothing, and the census arrays are sized 1024.
Rollback is five file operations. The probe cannot affect any other NPC: `per_miner_002` is used by exactly one instance in all scanned modules (registry: `201tel`, tag `commoner`, "Taris Citizen", app 17, at x=55.44 y=-24.31).

## 2. Why this NPC and this save

- Target: `per_miner_002.utc` (appearance 17 = `N_CommM`, male, type B, body_variation 1, texture_variation 1, conversation `taris`). Row 17 is the exact vanilla analogue of `k1p_commm` (K1 `n_commm`), so a copy of row 17 is the smallest possible edit.
- Area: `201tel` (Citadel Station Dock Module) is the first Telos area after the Ebon Hawk lands. He is a standing background commoner; no quest needed. Bay Control is at x=-26 in the same module, he is about 80 m further along +x.
- Save: existing saves hold full creature snapshots (W4: a template edit never reaches a visited area). SAVEGAME.sav module lists, read this session:
  - slots 000000-000008 (QUICKSAVE, AUTOSAVE, Game2-Game7): contain `201tel` and the other Telos modules. UNUSABLE.
  - slot 000010 `Game9` ("Peragus - Hangar Bay", 10h43m, Atton + T3M4): modules 101per..106per, 151har..153har; NO `2xxtel`. USABLE, provided its next story step is the flight to Telos (user confirms).
  - slots 000002, 000006, 000009: Peragus, also unvisited, but earlier in the game.
- Texture rule used: type B builds the body texture as `texa` + two-digit `texture_variation` (doc 34 hook 1: texa "N_CommM", then the slot). So `texa=k1p_commm` with `TextureVar=4` requests `k1p_commm04`, the staged file. UNVERIFIED in-game; fallbacks in section 9.

## 3. Files

Create (stage dir, outside the repo): `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/probe_T12h/`
- `override/appearance.2da`: live 812-row table plus row 812. Row 812 is a copy of row 17 with `label=K1P_CommM_Probe` and `race=modela=modelb=texa=texb=k1p_commm`. Rows 0-811 equal the live file cell by cell (checked: 0 diffs, headers equal). Expected sha256: `c92a7e8610f372018c74fc2890087e725ac4bafadbd3f19eb03b0e055f398865`.
- `override/per_miner_002.utc`: the `201TEL.mod` copy of the template with `Appearance_Type=812`, `TextureVar=4`; every other field unchanged. Expected sha256: `fa8a8be2b3778659de9eebf09eb67f2f0d79ab2f6fbf137535c0ce59cef46c5d`.
- `probe_manifest.json`: written by the tool (section 6).

Copy in unchanged from the stage (ledger sha256 verified equal this session):
- `models/k1p_commm.mdl` b7cf35e0...cc37, `models/k1p_commm.mdx` 84acc62d...8655, `override/k1p_commm04.tpc` 11b67dbf...21a7 (vanilla K1, 512 px DXT1, class `vanilla`, no approval needed). The `_hd/` variant is NOT used (flagged `pending_author_approval`).

Modify (game dir, via the tool only): `override/appearance.2da` is REPLACED. Current owner: the npc-diversity whole-file snapshot (sha256 `43e312dd...8002`, identical to `patch_manager_mods/npc_diversity_src/override/appearance.2da`). No other repo mod ships this file; the pre-merge base is `override/_diversity_premerge_backup/appearance.2da.bak`. Do NOT touch `DiversityRows.h`, the dispatcher source, hooks, or `patch_config.toml`. Do not run `generate_diversity_2da.py --apply` while the probe is installed (its base check would fail on row 812).

No collisions: `override/` has no `k1p_*` and no `per_miner_002.utc` today.

## 4. Generation (read-only on the game dir; writes only to the stage)

Interim script (tested; save as `patch_manager_mods/k1_port/k1_port_probe_build.py` or fold into the future tool `build` subcommand):

```
import sys
from pathlib import Path
from pykotor.resource.formats.erf import read_erf
from pykotor.resource.formats.twoda import bytes_2da, read_2da
from pykotor.resource.generics.utc import bytes_utc, read_utc
from pykotor.resource.type import ResourceType

G = Path.home() / ".local/share/Steam/steamapps/common/Knights of the Old Republic II"
OUT = Path(sys.argv[1])
OUT.mkdir(parents=True, exist_ok=True)
t = read_2da(str(G / "override/appearance.2da"))
assert t.get_height() == 812, t.get_height()
t.add_row("812", {c: t.get_cell(17, c) for c in t.get_headers() if c != "label"})
t.set_label(812, "K1P_CommM_Probe")
for c in ("race", "modela", "modelb", "texa", "texb"):
    t.set_cell(812, c, "k1p_commm")
(OUT / "appearance.2da").write_bytes(bytes_2da(t))
erf = read_erf(str(G / "modules/201TEL.mod"))
raw = next(r.data for r in erf if r.resref.get().lower() == "per_miner_002" and r.restype == ResourceType.UTC)
u = read_utc(raw)
assert u.appearance_id == 17
u.appearance_id, u.texture_variation = 812, 4
(OUT / "per_miner_002.utc").write_bytes(bytes_utc(u))
```

Command: `.venv/bin/python -I <script> ~/Games/KOTOR2_Mod_Workspace/k1_port_stage/probe_T12h/override`
The `assert height == 812` fails closed if another session changed the live table (then rebuild from the new base and re-record the sha).

## 5. Pre-checks (all must pass before the install step)

1. `python3 patch_manager_mods/k1_port/k1_port_validate.py <stage>/models/k1p_commm.mdl --mdx <stage>/models/k1p_commm.mdx --k1p` : exit 0, "0 finding(s)" (run this session: PASS).
2. `python3 patch_manager_mods/k1_port/k1_port_prune.py gate --strict <stage>/models/k1p_commm.mdl <stage>/models/k1p_commm.mdx <stage>/override/k1p_commm04.tpc <stage>/probe_T12h/override` : exit 0, blocked 0, warned 0 (run this session on the scratch build: PASS, 5 files).
3. `python3 scripts/run_tests.py k1_port` (scoped: model, textures, validate, prune suites).
4. Game closed: `pgrep -a swkotor2` prints nothing. `python3 scripts/kpm_status.py` md5s stable across two runs (no peer installing).
5. Live `override/appearance.2da` sha256 still `43e312dd...8002` (the tool refuses otherwise).

## 6. Install and rollback (future tool: `patch_manager_mods/k1_port/k1_port_probe.py`)

Subcommands: `build`, `install [--apply]`, `rollback [--apply]`, `verify`. Dry run by default; `--apply` needs the user's GO with the game closed.

install --apply:
1. Refuse if `swkotor2.exe` runs, if any target other than `override/appearance.2da` already exists in `override/`, or if the live `appearance.2da` sha differs from the manifest `base_sha256`.
2. Back up into `<game>/backups/k1p_probe_<YYYYMMDD_HHMM>/`: `override/appearance.2da`, plus the save slots `cloudsaves/62703561/000000 - QUICKSAVE` and `000001 - AUTOSAVE`. Reason: entering Telos triggers an autosave that overwrites the 16h36m AUTOSAVE. Game9 itself is never written unless the user saves.
3. Copy five files into `override/`: `appearance.2da` (replaced), `per_miner_002.utc`, `k1p_commm.mdl`, `k1p_commm.mdx`, `k1p_commm04.tpc`.
4. Write `<backup>/probe_manifest.json`: `{base_sha256, installed:[{path,sha256,size}], backups:[...], created}`; re-hash every installed file and fail loudly on any mismatch.

rollback --apply (idempotent):
1. Refuse if the game runs.
2. Verify each installed file still matches its manifest sha256 (a mismatch means a peer session edited it: stop and ask).
3. Restore `override/appearance.2da` from the backup (sha must equal `43e312dd...8002`); delete `per_miner_002.utc`, `k1p_commm.mdl`, `k1p_commm.mdx`, `k1p_commm04.tpc` from `override/`.
4. Saves: never touched automatically; print the `cp -a` command to restore the two backed-up slots (the user decides).
5. `verify`: no `k1p_*` and no `per_miner_002.utc` left in `override/`; `appearance.2da` sha equals the base.

Until the tool exists, the same steps are manual `cp`/`sha256sum`/`rm` with the explicit five-file list and one backup dir. Steam Cloud may sync the autosave change; the slot backup covers that.

## 7. What the log shows and how to check

- Command: `python3 scripts/kotor2_logs.py check npc-diversity_log.txt --new` (routes to `npc_div_log_check`; never cat the raw log).
- Expect for the probe NPC: a `TEX` line with `app=812 ... cur=k1p_commm` (hook 1 saw the new row's texa) and an `RMP` line `app=812>812 tv=4>4` with a non-changing `why` (`skip:*` or `nochg`). Locate it by position: `python3 scripts/npc_div_whatis.py 55.44 -24.31`.
- Any `RMP` line at that position with `app=812>other` or `tv=4>other` means dispatcher interference (FAIL, design assumption broken).
- The checker summary may not mention 812. If it is silent, run `whatis`; if still nothing, the area was loaded from a snapshot (wrong save) or the hook did not fire.
- A model or texture problem leaves NO dispatcher log line. Evidence for those is the user report (section 8) and, on a crash, the `kotor2-crash-triage` skill (filtered Proton log, never raw cat).

## 8. What the user does (about 5 minutes, one save load)

1. Close the game. Say GO for install. (Agent runs install --apply.)
2. Launch KOTOR 2, load slot `Game9` ("Peragus - Hangar Bay"). Confirm it is the last Peragus step, then take the Ebon Hawk to Telos. Do NOT save over Game9; decline any save prompt.
3. In the Citadel Station Dock Module, go to about x=55, y=-24 (the man named "Taris Citizen") and look at him from 2-3 m, from behind and from the side.
4. Report: (a) crash or hang on area load; (b) visible, standing upright (not T-posed, no stretched or collapsed limbs); (c) body/clothes texture normal, pink/white, or flat grey; (d) head present and attached; (e) idle or walk animation plays.
5. Quit the game. Say "done". (Agent checks the log, then offers rollback.)

## 9. Failure reading

| Symptom | Likely cause | Next probe |
|---|---|---|
| Crash or freeze loading `201tel` | loader rejects `k1p_commm.mdl` (K2 trimesh/skin layout) | rollback; keep the crash log for `kotor2-crash-triage`; compare against MDLOps (T7) |
| Visible, white/pink body | texture name mismatch (`k1p_commm04` not requested) | rebuild with `texa=k1p_commm04` and `TextureVar=0`, or blank `texa` so the model's embedded `k1p_commm04` is used |
| Visible, T-pose or sliding | supermodel/animation set (`S_Male02` K1 vs K2 anim names) | see T2b notes; change supermodel only after rollback |
| Original Taris Citizen unchanged | wrong save (snapshot) or override not taking precedence over the module | confirm the slot has no `201tel`; retry from another pre-Telos save |
| Body ok, head odd | `normalhead=9` copy of row 17 | vanilla head expected; report |

## 10. Checklist (8 steps)

1. Build the two files with the section 4 command (agent); record both sha256.
2. Run the five pre-checks in section 5 (agent); all must pass.
3. [USER] Close KOTOR 2 and reply GO for install.
4. Agent installs: `k1_port_probe.py install --apply` (interim: manual copy of the five files plus the backup dir); verify manifest sha256.
5. [USER] Launch, load `Game9`, fly to Telos, do not save, walk to x=55 y=-24 in the Dock Module.
6. [USER] Look at "Taris Citizen" from 3 sides, report (a)-(e) from section 8, quit the game.
7. Agent: `python3 scripts/kotor2_logs.py check npc-diversity_log.txt --new`, then `npc_div_whatis.py 55.44 -24.31`; record the result in doc 38 section 7 (T12 row).
8. [USER] Reply KEEP or ROLLBACK (default ROLLBACK); agent runs `k1_port_probe.py rollback --apply` then `verify`.

## 11. Open

- Texture naming rule (`texa` + two-digit TextureVar) comes from doc 34 hook 1 text, not a decompile; fallbacks are in section 9.
- Game9's next story step being the Telos flight is inferred from its module list and area name; the user must confirm.
- `per_miner_002.utc` was taken from `201TEL.mod` (conversation `taris`); `201TEL_s.rim` holds a different copy (`airport_tarsis`). The override replaces whichever copy loads, and sets the conversation to the `.mod` value.
