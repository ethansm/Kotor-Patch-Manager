#!/usr/bin/env python3
"""Pass 2: for every Fandom page already harvested, fetch only VISUAL sections (appearance, uniform, equipment, insignia...).

Reads harvest/index.tsv (family, title, ...), writes harvest/vis/<family>.txt (no per-page truncation, 4500 chars/section).
Also adds extra seed pages given below. Deterministic, no model tokens.
"""
import re, sys, time
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
from harvest import get, to_text, FANDOM, PAUSE  # noqa

OUT = Path(__file__).resolve().parent.parent / "harvest"
VIS = re.compile(r"appearance|uniform|equipment|dress|cloth|attire|armou?r|insignia|symbol|emblem|tattoo|visual|logo|design|marking|crest|colou?r|clan|rank|garb|robe|outfit|fashion|culture|body|decorat|ornament|jewel|identity|iconograph|heraldry|badge", re.I)
SKIP = re.compile(r"behind the scenes|sources|notes and references|external links|see also|appearances$|contents|gallery|trivia|bibliography", re.I)
EXTRA = {
 "czerka": ["Czerka Arms", "Czerka Arms/Legends", "Czerka Corporation/Legends"],
 "republic": ["Republic crest", "Republic crest/Legends", "Galactic Republic", "Galactic Republic/Legends", "Telos Security Force"],
 "exchange": ["Exchange", "Exchange/Legends", "Serroco/Legends", "Black Vulkars", "Red Eclipse"],
 "mandalorian": ["Mandalorian armor", "Mandalorian armor/Legends", "Jaig eyes", "Mandalorian neo-Crusaders", "Mandalorian neo-Crusaders/Legends"],
 "sith_jedi": ["Sith Empire", "Sith Empire/Legends", "Sith trooper", "Sith assassin", "Jedi robe", "Jedi Order", "Jedi Order/Legends", "Sith Academy"],
 "onderon": ["Onderon", "Onderon/Legends", "Iziz", "Iziz/Legends", "Onderon soldier/Legends", "Vaklu/Legends"],
}


def sections(title):
    d = get(FANDOM, {"action": "parse", "page": title, "prop": "sections", "redirects": 1, "format": "json"})
    return d.get("parse", {}).get("sections", []), d.get("parse", {}).get("title", title)


def section_text(title, idx):
    d = get(FANDOM, {"action": "parse", "page": title, "prop": "text", "section": idx, "redirects": 1, "format": "json", "disablelimitreport": 1})
    return to_text(d["parse"]["text"]["*"]) if "parse" in d else ""


def main():
    pages = {}
    for line in (OUT / "index.tsv").read_text().splitlines()[1:]:
        f = line.split("\t")
        if len(f) >= 4 and f[3] == "fandom":
            pages.setdefault(f[0], []).append(f[1])
    for fam, ts in EXTRA.items():
        pages.setdefault(fam, []).extend(ts)
    (OUT / "vis").mkdir(exist_ok=True)
    for fam, titles in pages.items():
        out = []
        for t in dict.fromkeys(titles):
            secs, real = sections(t); time.sleep(PAUSE)
            pick = [s for s in secs if s.get("toclevel", 1) <= 2 and VIS.search(s["line"]) and not SKIP.search(s["line"])]
            for s in pick[:6]:
                tx = section_text(t, s["index"]); time.sleep(PAUSE)
                if len(tx) > 150:
                    out.append(f"=== {real} :: {s['line']} [fandom:{fam}]\n{tx[:4500]}\n")
        (OUT / "vis" / f"{fam}.txt").write_text("\n".join(out))
        print(fam, len(out), "sections", sum(len(x) for x in out) // 1000, "KB", flush=True)


if __name__ == "__main__":
    main()
