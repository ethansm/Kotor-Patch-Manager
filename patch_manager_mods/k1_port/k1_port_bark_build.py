"""Stage the Juhani bark (soundset -> SSF -> TLK -> wav) test and the UTC variants. Local only: K1 audio stays out of the repo.

Run with the k1port venv python (pykotor):

    ~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I patch_manager_mods/k1_port/k1_port_bark_build.py \
        --out ~/Games/KOTOR2_Mod_Workspace/k1_port_stage/char_juhani_audio

Why no TLK edit: the K2 dialog.tlk still holds K1's Juhani rows 36914..36942 (same strrefs, same text, SoundResRef
`p_juhani_*`) and no K2 file carries those names. So the K1 SSF `p_juhani` works unchanged in K2.

Writes under <out>/:
  override_bark/soundset.2da    live K2 table + one appended row (label kvj_juhani, resref kvjssf, gender 1, strref -1)
  override_bark/kvjssf.ssf      the K1 `p_juhani` SSF bytes (28 strrefs 36914..36942, same 172-byte V1.1 layout as K2)
  override_bark/p_juhani_slct1.wav   control: RIFF PCM16 mono 22050 tone, 440 Hz (E1 format; proves SSF -> TLK -> Override)
  override_bark/p_juhani_slct2.wav   K1 original (SFX layout: MP3 + RIFF at byte 470) = E2d
  override_bark/p_juhani_slct3.wav   K1 original (same layout); also the candidate for a StreamSounds/ copy
  utc/dialog/k1p_juhani.utc     installed UTC with Conversation = kvjdlg01 (Soundset unchanged)
  utc/bark/k1p_juhani.utc       Soundset = the new row (Conversation unchanged)
  utc/both/k1p_juhani.utc       both
The base UTC is the staged char_juhani/override/k1p_juhani.utc (so the appearance row stays 818). Install ONE UTC
as override/k1p_juhani.utc per test. Watch with scripts/override_watch.py --match 'p_juhani|kvj'.
"""
import argparse
import logging
import hashlib
import math
import os
import struct
import subprocess
import sys
from pathlib import Path

logging.disable(logging.CRITICAL)  # pykotor logs heavily while loading an Installation
HERE = Path(__file__).resolve().parent
DEFAULT_K1 = "~/.local/share/Steam/steamapps/common/swkotor"
DEFAULT_K2 = "~/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
STAGE = Path("~/Games/KOTOR2_Mod_Workspace/k1_port_stage").expanduser()
BASE_UTC = STAGE / "char_juhani" / "override" / "k1p_juhani.utc"
NEW_LABEL, NEW_RESREF, SSF_NAME = "kvj_juhani", "kvjssf", "kvjssf"
DLG = "kvjdlg01"
TONE_HZ, RATE = 440, 22050


class BuildError(Exception):
    pass


def tone_wav(seconds=1.5, hz=TONE_HZ, rate=RATE) -> bytes:
    """RIFF PCM16 mono tone with a fade in/out (no clicks), the E1 format."""
    n = int(seconds * rate)
    fade = int(0.02 * rate)
    pcm = bytearray()
    for i in range(n):
        env = min(1.0, i / fade, (n - 1 - i) / fade)
        pcm += struct.pack("<h", int(12000 * env * math.sin(2 * math.pi * hz * i / rate)))
    hdr = b"RIFF" + struct.pack("<I", 36 + len(pcm)) + b"WAVEfmt " + struct.pack("<IHHIIHH", 16, 1, 1, rate, rate * 2, 2, 16)
    return hdr + b"data" + struct.pack("<I", len(pcm)) + bytes(pcm)


def find_ci(base: Path, rel: str) -> Path:
    cur = base
    for part in rel.split("/"):
        hit = next((p for p in cur.iterdir() if p.name.lower() == part.lower()), None) if cur.is_dir() else None
        if hit is None:
            raise BuildError(f"{rel}: '{part}' not found under {cur}")
        cur = hit
    return cur


def append_soundset_row(raw: bytes, label=NEW_LABEL, resref=NEW_RESREF, gender="1"):
    """Return (new 2da bytes, new row index). Other rows are untouched; unknown columns stay empty."""
    from pykotor.resource.formats.twoda import bytes_2da, read_2da
    t = read_2da(raw)
    row = t.get_height()
    cells = {"label": label, "resref": resref, "strref": "-1", "gender": gender}
    t.add_row(str(row), {h: cells.get(h, "") for h in t.get_headers()})
    return bytes_2da(t), row


def patch_utc(raw: bytes, *, conversation=None, soundset=None) -> bytes:
    from pykotor.common.misc import ResRef
    from pykotor.resource.generics.utc import bytes_utc, read_utc
    u = read_utc(raw)
    if conversation is not None:
        u.conversation = ResRef(conversation)
    if soundset is not None:
        u.soundset_id = soundset
    return bytes_utc(u)


def sha(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()[:12]


def build(k1, k2, out: Path, *, adopt=True) -> dict:
    from pykotor.extract.installation import Installation, SearchLocation
    from pykotor.resource.generics.utc import read_utc
    from pykotor.resource.type import ResourceType
    k1, k2 = Path(k1).expanduser(), Path(k2).expanduser()
    live = k2 / "override" / "soundset.2da"
    if not live.is_file():
        raise BuildError(f"missing live file: {live}")
    if not BASE_UTC.is_file():
        raise BuildError(f"missing base UTC: {BASE_UTC}")
    ssf = Installation(str(k1)).resource("p_juhani", ResourceType.SSF, [SearchLocation.CHITIN, SearchLocation.OVERRIDE])
    if ssf is None or len(ssf.data) != 172 or ssf.data[:8] != b"SSF V1.1":
        raise BuildError("K1 p_juhani.ssf not found or not a 172-byte SSF V1.1")
    strefs = struct.unpack_from("<28I", ssf.data, 12)
    if strefs[0] != 36914 or strefs[27] != 36942:
        raise BuildError(f"unexpected SSF strrefs {strefs[0]}..{strefs[27]}")
    table, row = append_soundset_row(live.read_bytes())
    bark = out / "override_bark"
    utcd = out / "utc"
    bark.mkdir(parents=True, exist_ok=True)
    utcd.mkdir(parents=True, exist_ok=True)
    k1_made = []
    files = {bark / "soundset.2da": table, bark / f"{SSF_NAME}.ssf": bytes(ssf.data),
             bark / "p_juhani_slct1.wav": tone_wav()}
    for n in (2, 3):
        files[bark / f"p_juhani_slct{n}.wav"] = find_ci(k1, f"streamsounds/P_JUHANI_SLCT{n}.wav").read_bytes()
        k1_made.append(bark / f"p_juhani_slct{n}.wav")
    k1_made.append(bark / f"{SSF_NAME}.ssf")
    base = BASE_UTC.read_bytes()
    for name, kw in (("dialog", dict(conversation=DLG)), ("bark", dict(soundset=row)),
                     ("both", dict(conversation=DLG, soundset=row))):
        (utcd / name).mkdir(exist_ok=True)  # the probe installs by file name, so each variant is k1p_juhani.utc
        files[utcd / name / "k1p_juhani.utc"] = patch_utc(base, **kw)
    for p, b in files.items():
        p.write_bytes(b)
    if adopt:
        # The tone is synthesised here (no third-party content); everything else read from the K1 install is vanilla_k1.
        for f, source_id in [(f, "vanilla_k1") for f in k1_made] + [(bark / "p_juhani_slct1.wav", "workspace_generated")]:
            r = subprocess.run([sys.executable, str(HERE / "k1_port_prune.py"), "adopt", "--file", str(f), "--kind",
                                "other", "--source-id", source_id], capture_output=True, text=True,
                               env=dict(os.environ, K1PORT_LEDGER=str(STAGE / "ledger.json")))
            if r.returncode != 0:
                raise BuildError(f"ledger adopt failed for {f}: {r.stderr.strip() or r.stdout.strip()}")
    u0 = read_utc(base)
    return {"soundset_row": row, "base_soundset_sha": sha(live.read_bytes()), "base_utc_conversation": str(u0.conversation),
            "base_utc_soundset": u0.soundset_id, "files": {str(p): sha(b) for p, b in files.items()}}


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--k1", default=DEFAULT_K1)
    ap.add_argument("--k2", default=DEFAULT_K2, help="live K2 install (read only; its override/soundset.2da is the base)")
    ap.add_argument("--out", required=True)
    ap.add_argument("--no-adopt", action="store_true")
    a = ap.parse_args(argv)
    try:
        res = build(a.k1, a.k2, Path(a.out).expanduser(), adopt=not a.no_adopt)
    except (BuildError, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    print(f"soundset row {res['soundset_row']} (base soundset.2da sha {res['base_soundset_sha']}); base UTC "
          f"conversation {res['base_utc_conversation']!r}, soundset {res['base_utc_soundset']}")
    for p, h in res["files"].items():
        print(f"  {h}  {p}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
