#!/usr/bin/env python3
"""Deterministic lore harvester (no model tokens). Fandom/StrategyWiki/Commons MediaWiki APIs -> local text bundles.

Usage: python3 harvest.py [--out DIR] [--only FAMILY]
Writes DIR/pages/<slug>.txt (plain text, capped), DIR/bundles/<family>.txt, DIR/index.tsv, DIR/commons_decals.tsv.
Fandom HTML is Cloudflare-blocked, the api.php endpoint is not. TextExtracts is not installed there, so we use action=parse.
"""
import argparse, html, json, re, sys, time, urllib.parse, urllib.request
from html.parser import HTMLParser
from pathlib import Path

UA = "Mozilla/5.0 (KOTOR2 texture research; contact ethansm161@gmail.com)"
FANDOM = "https://starwars.fandom.com/api.php"
STRATEGY = "https://strategywiki.org/w/api.php"
COMMONS = "https://commons.wikimedia.org/w/api.php"
CAP = 7000          # chars kept per page
PAUSE = 0.25

FAMILIES = {
 "republic": ["Telos Security Force", "Republic crest", "Telosian Restoration Project", "Republic Military", "Republic Navy", "Republic soldier Jedi Civil War", "Dol Grenn", "Republic officer uniform"],
 "telos": ["Telos IV", "Citadel Station", "Telos Restoration Zone", "Telosian", "Dobo Brothers Emporium", "Czerka Offices Telos", "Telos Ithorian"],
 "czerka": ["Czerka Arms", "Czerka Corporation uniform", "Czerka Security", "Peragus Mining Facility", "Peragus II", "Czerka Dantooine Khoonda", "Czerka mercenary"],
 "exchange": ["The Exchange crime syndicate", "Exchange enforcer", "Serroco", "Zhug Brothers", "Red Eclipse", "Black Vulkars", "Vogga the Hutt", "Hutt cartel", "Ubese bounty hunter", "Smugglers' Moon Nar Shaddaa", "Refugee Landing Nar Shaddaa", "Jekk'jekk Tarr"],
 "onderon": ["Onderon", "Iziz", "Queen Talia", "Vaklu", "Onderon soldier", "Beast Rider Onderon", "Dxun", "Onderon royal guard"],
 "mandalorian": ["Mandalorian armor", "Mandalorian Neo-Crusaders", "Mandalore the Preserver", "Mandalorian clan", "Jaig eyes", "Mandalorian helmet", "Mandalorian kama", "Mandalorian Dxun camp"],
 "sith_jedi": ["Sith assassin", "Sith trooper", "Sith Academy Korriban", "Handmaiden Telos", "Jedi robes", "Jedi Order symbol", "Sith Empire symbol", "Sith Triumvirate", "Dark Jedi Apprentice", "Atris Jedi Enclave Telos"],
 "workers": ["Peragus miner", "Khoonda", "Dantooine militia", "Dantooine settler farmer", "droid technician mechanic", "Ebon Hawk"],
 "alien": ["Twi'lek", "Rodian", "Duros", "Bith", "Gand", "Weequay", "Trandoshan", "Sullustan", "Ithorian", "Devaronian", "Zabrak", "Aqualish", "Gamorrean", "Zabrak tattoos", "Twi'lek tattoos"],
 "traditions": ["Jal Shey", "Luka Sene", "Order of Shasa", "Matukai", "Jedi Covenant", "Krath", "Keetael", "Cinnagaran", "Echani", "GenoHaradan", "Iridonian", "Massassi", "Verpine", "Bonadan", "Eriadun"],
}
FANDOM_CATEGORIES = {   # whole category -> family "symbols"
 "symbols": ["Category:Symbols", "Category:Insignia", "Category:Mandalorian_symbols", "Category:Swoop_gangs"],
}
STRATEGY_PREFIX = "Star Wars Knights of the Old Republic II: The Sith Lords/"
STRATEGY_KEEP = re.compile(r"armor|armour|robe|clothing|shield|belt|gloves|headgear|implant|item", re.I)


def get(base, params):
    url = base + "?" + urllib.parse.urlencode(params)
    req = urllib.request.Request(url, headers={"User-Agent": UA})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=30) as r:
                return json.load(r)
        except Exception as e:  # noqa
            time.sleep(1.5 * (attempt + 1))
            err = e
    sys.stderr.write(f"FAIL {url[:120]} {err}\n")
    return {}


class Strip(HTMLParser):
    SKIP = {"script", "style", "sup", "table_nav"}
    def __init__(self):
        super().__init__(); self.out = []; self.skip = 0
    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        if tag in ("script", "style") or "reference" in (a.get("class") or "") or "navbox" in (a.get("class") or "") or "toc" == (a.get("id") or ""):
            self.skip += 1
        elif tag in ("p", "li", "tr", "br", "h2", "h3", "h4", "div"):
            self.out.append("\n")
        if tag in ("td", "th"):
            self.out.append(" | ")
    def handle_endtag(self, tag):
        if self.skip and tag in ("script", "style", "sup", "div", "ol", "span"):
            self.skip -= 1 if tag in ("script", "style") or self.skip else 0
    def handle_data(self, d):
        if not self.skip:
            self.out.append(d)


def to_text(htm):
    p = Strip(); p.feed(htm)
    t = html.unescape("".join(p.out))
    t = re.sub(r"\[\d+\]|\[edit\]", "", t)
    t = re.sub(r"[ \t]+", " ", t)
    t = re.sub(r"\n\s*\n+", "\n", t)
    return t.strip()


def slug(s):
    return re.sub(r"[^a-z0-9]+", "_", s.lower()).strip("_")[:70]


def fandom_page(title):
    d = get(FANDOM, {"action": "parse", "page": title, "prop": "text|categories", "redirects": 1, "format": "json", "disablelimitreport": 1})
    if "parse" not in d:
        return None
    p = d["parse"]
    return p["title"], to_text(p["text"]["*"]), [c["*"] for c in p.get("categories", []) if "hidden" not in c][:12]


def fandom_search(q, n=2):
    d = get(FANDOM, {"action": "query", "list": "search", "srsearch": q, "srlimit": n, "format": "json"})
    return [h["title"] for h in d.get("query", {}).get("search", [])]


def category_members(cat, limit=500):
    d = get(FANDOM, {"action": "query", "list": "categorymembers", "cmtitle": cat, "cmlimit": limit, "cmnamespace": 0, "format": "json"})
    return [m["title"] for m in d.get("query", {}).get("categorymembers", [])]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(Path(__file__).resolve().parent.parent / "harvest"))
    ap.add_argument("--only")
    a = ap.parse_args()
    out = Path(a.out); (out / "pages").mkdir(parents=True, exist_ok=True); (out / "bundles").mkdir(exist_ok=True)
    index = []
    seen = {}
    plan = {f: list(qs) for f, qs in FAMILIES.items()}
    for fam, cats in FANDOM_CATEGORIES.items():
        titles = []
        for c in cats:
            titles += category_members(c); time.sleep(PAUSE)
        plan[fam] = ["T:" + t for t in dict.fromkeys(titles)]
    for fam, items in plan.items():
        if a.only and fam != a.only:
            continue
        bundle = []
        for it in items:
            if it.startswith("T:"):
                titles = [it[2:]]
            else:
                titles = fandom_search(it, 2); time.sleep(PAUSE)
            for t in titles:
                if t in seen:
                    bundle.append(seen[t]); continue
                r = fandom_page(t); time.sleep(PAUSE)
                if not r:
                    continue
                title, text, cats = r
                if len(text) < 200:
                    continue
                rec = f"=== {title} [fandom:{fam}] cats={';'.join(cats[:6])}\n{text[:CAP]}\n"
                (out / "pages" / f"{slug(title)}.txt").write_text(rec)
                seen[t] = rec; seen[title] = rec
                bundle.append(rec)
                index.append(f"{fam}\t{title}\t{len(text)}\tfandom\t{it}")
        # dedupe within bundle
        uniq = list(dict.fromkeys(bundle))
        (out / "bundles" / f"{fam}.txt").write_text("\n".join(uniq))
        print(fam, len(uniq), "pages", sum(len(x) for x in uniq) // 1000, "KB", flush=True)
    # StrategyWiki armor/robe pages
    if not a.only or a.only == "strategy":
        d = get(STRATEGY, {"action": "query", "list": "allpages", "apprefix": STRATEGY_PREFIX.replace(" ", "_"), "aplimit": 200, "format": "json"})
        titles = [p["title"] for p in d.get("query", {}).get("allpages", []) if STRATEGY_KEEP.search(p["title"])]
        sb = []
        for t in titles:
            dd = get(STRATEGY, {"action": "parse", "page": t, "prop": "text", "format": "json", "disablelimitreport": 1}); time.sleep(PAUSE)
            if "parse" in dd:
                txt = to_text(dd["parse"]["text"]["*"])
                sb.append(f"=== {t} [strategywiki]\n{txt[:12000]}\n")
                index.append(f"strategy\t{t}\t{len(txt)}\tstrategywiki\t")
        (out / "bundles" / "strategy_items.txt").write_text("\n".join(sb))
        print("strategy", len(sb), "pages", flush=True)
    # Commons decal candidates (manifest only, with licence strings)
    if not a.only or a.only == "commons":
        rows = ["file\tlicense\tartist\turl"]
        for cat in ("Category:Star_Wars_logos_and_symbols", "Category:Star_Wars_emblems", "Category:Star_Wars_flags"):
            d = get(COMMONS, {"action": "query", "generator": "categorymembers", "gcmtitle": cat, "gcmtype": "file", "gcmlimit": 100,
                              "prop": "imageinfo", "iiprop": "url|extmetadata", "format": "json"}); time.sleep(PAUSE)
            for p in d.get("query", {}).get("pages", {}).values():
                ii = (p.get("imageinfo") or [{}])[0]; m = ii.get("extmetadata", {})
                rows.append("\t".join([p["title"], m.get("LicenseShortName", {}).get("value", "?"),
                                       re.sub(r"<[^>]+>|\s+", " ", m.get("Artist", {}).get("value", "?"))[:60], ii.get("url", "")]))
        (out / "commons_decals.tsv").write_text("\n".join(rows) + "\n")
        print("commons", len(rows) - 1, "files", flush=True)
    (out / "index.tsv").write_text("family\ttitle\tchars\tsource\tquery\n" + "\n".join(index) + "\n")


if __name__ == "__main__":
    main()
