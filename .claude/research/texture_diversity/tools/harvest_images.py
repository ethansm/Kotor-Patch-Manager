#!/usr/bin/env python3
"""Download small reference thumbnails of emblem/logo/crest files from Fandom pages (LOCAL VIEWING ONLY, never composited).

Writes harvest/img/<slug>__<file>.png|jpg (400 px wide) and harvest/img/index.tsv (page, file, url, license-ish note).
Fandom images are fair-use franchise art: reference for describing shape/colour, never a decal source.
"""
import re, sys, time, urllib.request
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from harvest import get, FANDOM, UA, PAUSE, slug  # noqa

OUT = Path(__file__).resolve().parent.parent / "harvest" / "img"
PAGES = ["Republic crest", "Czerka Arms", "Exchange", "Telos Security Force", "Serroco", "Black Vulkars", "Red Eclipse", "Mandalorian armor",
         "Mythosaur", "Jaig eyes", "Clan Vizsla", "Sith Empire", "Jedi Order", "Onderon", "Khoonda", "Hutt Cartel", "Peragus Mining Facility",
         "Galactic Republic", "Sith trooper", "Ubese", "GenoHaradan", "Blazing Claw", "Mandalorian neo-Crusaders", "Mandalore the Preserver",
         "Telosian Restoration Project", "Republic Military", "Sith assassin", "Jedi Covenant", "Matukai", "Handmaiden", "Vogga the Hutt",
         "Nar Shaddaa", "Telos IV", "Citadel Station", "Iziz", "Queen Talia", "Vaklu", "Beast Rider", "Zhug Brothers", "Dxun"]
GOOD = re.compile(r"emblem|logo|symbol|crest|insignia|sigil|seal|badge|flag|clan|icon|uniform|armou?r|robe|tattoo|swoop gang|mark", re.I)
BAD = re.compile(r"tab-|premium|canon|legends|databank|template|wiki-|favicon|title|cover|mini|swcustom|starwars-|fortnite|rebels|tcw|tales|mandalorian-logo|adventures|shatterpoint|allstars|swicon|^file:o\d|gotolegends|tomlogo|ahsoka", re.I)
MAXPER = 3


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    rows = ["page\tfile\tpath\turl"]
    seen = set()
    for t in PAGES:
        d = get(FANDOM, {"action": "query", "titles": t, "prop": "images", "imlimit": 60, "redirects": 1, "format": "json"}); time.sleep(PAUSE)
        pg = list(d.get("query", {}).get("pages", {}).values())
        files = [i["title"] for p in pg for i in p.get("images", [])]
        pick = [f for f in files if GOOD.search(f) and not BAD.search(f) and f not in seen][:MAXPER]
        for f in pick:
            seen.add(f)
            ii = get(FANDOM, {"action": "query", "titles": f, "prop": "imageinfo", "iiprop": "url|size", "iiurlwidth": 400, "format": "json"}); time.sleep(PAUSE)
            info = (list(ii.get("query", {}).get("pages", {}).values())[0].get("imageinfo") or [{}])[0]
            url = info.get("thumburl") or info.get("url")
            if not url:
                continue
            ext = ".png" if ".png" in url.lower().split("?")[0] or ".svg" in url.lower().split("?")[0] else ".jpg"
            path = OUT / f"{slug(t)}__{slug(f[5:])}{ext}"
            try:
                req = urllib.request.Request(url, headers={"User-Agent": UA})
                path.write_bytes(urllib.request.urlopen(req, timeout=30).read())
                rows.append(f"{t}\t{f}\t{path.name}\t{url}")
                print("ok", path.name, flush=True)
            except Exception as e:  # noqa
                print("fail", f, e, flush=True)
    (OUT / "index.tsv").write_text("\n".join(rows) + "\n")


if __name__ == "__main__":
    main()
