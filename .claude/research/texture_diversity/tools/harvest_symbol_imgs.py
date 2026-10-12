#!/usr/bin/env python3
"""One emblem image per Category:Symbols/Insignia page (best file-name match), 256 px, plus 4x3 contact sheets for Haiku viewers."""
import re, sys, time, urllib.request, difflib
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from harvest import get, FANDOM, UA, PAUSE, slug  # noqa
H = Path(__file__).resolve().parent.parent / "harvest"
OUT = H / "symimg"; OUT.mkdir(exist_ok=True)
BAD = re.compile(r"tab-|premium|canon|legends|databank|template|wiki-|favicon|cover|mini|swcustom|starwars-|fortnite|logo|icon|^file:o\d|gotolegends", re.I)
titles = [l.split("\t")[1] for l in (H/"index.tsv").read_text().splitlines()[1:] if l.startswith("symbols\t")]
rows = ["page\tfile\tpng"]
for t in titles:
    d = get(FANDOM, {"action":"query","titles":t,"prop":"images","imlimit":40,"redirects":1,"format":"json"}); time.sleep(0.2)
    files = [i["title"] for p in d.get("query",{}).get("pages",{}).values() for i in p.get("images",[]) if not BAD.search(i["title"])]
    if not files: continue
    best = max(files, key=lambda f: difflib.SequenceMatcher(None, t.lower(), f[5:].rsplit(".",1)[0].lower()).ratio())
    if difflib.SequenceMatcher(None, t.lower(), best[5:].rsplit(".",1)[0].lower()).ratio() < 0.35: continue
    ii = get(FANDOM, {"action":"query","titles":best,"prop":"imageinfo","iiprop":"url","iiurlwidth":256,"format":"json"}); time.sleep(0.2)
    info = (list(ii.get("query",{}).get("pages",{}).values())[0].get("imageinfo") or [{}])[0]
    url = info.get("thumburl") or info.get("url")
    if not url: continue
    p = OUT / (slug(t) + ".png")
    try:
        p.write_bytes(urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent":UA}), timeout=30).read())
        rows.append(f"{t}\t{best}\t{p.name}")
    except Exception as e: print("fail", t, e)
(OUT/"index.tsv").write_text("\n".join(rows)+"\n"); print(len(rows)-1, "images of", len(titles))
