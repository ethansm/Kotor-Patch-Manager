"""Stage the Juhani voice/dialog test files (E3 lip lookup + Override-voice test). Local only: K1 audio never enters the repo.

Run with the k1port venv python (pykotor):

    ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_audio_test_build.py \
        --out ~/Games/KOTOR2_Mod_Workspace/k1_port_stage/char_juhani_audio

Writes <out>/override/ (kvjdlg01.dlg, kvjv0001..3.wav, kvjv0001.lip valid, kvjv0003.lip corrupt; kvjv0002 has no
lip) and <out>/streamvoice/ (the same three wavs, flat: `kvj` is not a special StreamVoice prefix, so the lookup is
STREAMVOICE\\<resref>). Install ONE of the two sets per test; override/ first, streamvoice/ only if the engine
opened the Override wav but stayed silent.

The dialog is 3 NPC lines (each with inline text and a VO resref) joined by "Continue" replies, so one click on
Juhani exercises: line 1 = wav + valid lip, line 2 = wav + no lip, line 3 = wav + corrupt lip. All three wavs are the
same K1 clip (NM35AAJUHA11395_, 58-byte VO container, about 2.5 s). Source files are read from the K1 install
(never written) and registered in the removal ledger as vanilla_k1 `other`.
Watch the lookups with scripts/override_watch.py --match kvj.
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
DEFAULT_K1 = "~/.local/share/Steam/steamapps/common/swkotor"
DEFAULT_STAGE = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage").expanduser()
SRC_WAV = "streamwaves/m35aa/juha11/NM35AAJUHA11395_.wav"
SRC_LIP = ("lips/localization.mod", "nm35aajuha11395_")
LINES = (
    ("kvjv0001", "Juhani test line one: wav and a valid lip."),
    ("kvjv0002", "Juhani test line two: wav, no lip file."),
    ("kvjv0003", "Juhani test line three: wav and a corrupt lip."),
)
DLG_NAME = "kvjdlg01"


class BuildError(Exception):
    pass


def find_ci(base: Path, rel: str) -> Path:
    cur = base
    for part in rel.split("/"):
        hit = next((p for p in cur.iterdir() if p.name.lower() == part.lower()), None) if cur.is_dir() else None
        if hit is None:
            raise BuildError(f"{rel}: '{part}' not found under {cur}")
        cur = hit
    return cur


def corrupt_lip(data: bytes) -> bytes:
    """Same length, wrong magic: a strict reader refuses it, a lax one may still play."""
    return b"BAD " + data[4:]


def build_dlg(lines=LINES):
    from pykotor.common.language import LocalizedString
    from pykotor.common.misc import ResRef
    from pykotor.resource.generics.dlg import DLG, DLGEntry, DLGLink, DLGReply
    d = DLG()
    prev = None
    for i, (vo, text) in enumerate(lines):
        e = DLGEntry()
        e.text = LocalizedString.from_english(text)
        e.vo_resref = ResRef(vo)
        e.speaker = ""
        e.delay = -1
        e.camera_id = None
        if prev is None:
            d.starters.append(DLGLink(e))
        else:
            r = DLGReply()
            r.text = LocalizedString.from_english("Continue.")
            prev.links.append(DLGLink(r))
            r.links.append(DLGLink(e))
        prev = e
    last = DLGReply()
    last.text = LocalizedString.from_english("End the test.")
    prev.links.append(DLGLink(last))
    return d


def build(k1, out: Path, *, adopt=True) -> dict:
    from pykotor.resource.formats.erf import read_erf
    from pykotor.resource.generics.dlg import bytes_dlg
    k1 = Path(k1).expanduser()
    wav = find_ci(k1, SRC_WAV).read_bytes()
    erf = read_erf(find_ci(k1, SRC_LIP[0]).read_bytes())
    hits = [r for r in erf if r.resref.get().lower() == SRC_LIP[1] and r.restype.extension.lower() == "lip"]
    if len(hits) != 1:
        raise BuildError(f"expected one {SRC_LIP[1]}.lip in {SRC_LIP[0]}, found {len(hits)}")
    lip = bytes(hits[0].data)
    files = {"kvjdlg01.dlg": bytes_dlg(build_dlg())}
    for vo, _ in LINES:
        files[f"{vo}.wav"] = wav
    files["kvjv0001.lip"] = lip
    files["kvjv0003.lip"] = corrupt_lip(lip)
    written = []
    for sub, names in (("override", list(files)), ("streamvoice", [n for n in files if n.endswith(".wav")])):
        (out / sub).mkdir(parents=True, exist_ok=True)
        for n in names:
            (out / sub / n).write_bytes(files[n])
            written.append(out / sub / n)
    if adopt:
        ledger = DEFAULT_STAGE / "ledger.json"
        for f in written:
            if f.suffix in (".wav", ".lip"):
                r = subprocess.run([sys.executable, str(HERE / "k1_port_prune.py"), "adopt", "--file", str(f), "--kind",
                                    "other", "--source-id", "vanilla_k1"], capture_output=True, text=True,
                                   env=dict(os.environ, K1PORT_LEDGER=str(ledger)))
                if r.returncode != 0:
                    raise BuildError(f"ledger adopt failed for {f}: {r.stderr.strip() or r.stdout.strip()}")
    return {"files": [str(p) for p in written], "wav_bytes": len(wav), "lip_bytes": len(lip)}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--k1", default=DEFAULT_K1)
    ap.add_argument("--out", required=True, help="stage dir (must be under the k1_port stage root for the ledger)")
    ap.add_argument("--no-adopt", action="store_true", help="skip ledger registration (tests)")
    a = ap.parse_args(argv)
    try:
        res = build(a.k1, Path(a.out).expanduser(), adopt=not a.no_adopt)
    except (BuildError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    print(f"wrote {len(res['files'])} files (wav {res['wav_bytes']} B, lip {res['lip_bytes']} B)")
    for f in res["files"]:
        print("  " + f)
    return 0


if __name__ == "__main__":
    sys.exit(main())
