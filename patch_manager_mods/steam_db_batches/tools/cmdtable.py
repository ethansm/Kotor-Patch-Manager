#!/usr/bin/env python3
"""cmdtable.py - decode the NWScript command tables (GOG and Steam) into a B<n> candidate source (B15, ExecuteCommand*).

Both exes fill a 0x36D-slot handler table: `InitCommands` stores `MOV [reg + 4*i], handler` for 774 slots and then
calls a second function that stores the 103 SWMG minigame slots. Slot i handles the i-th function declared in
nwscript.nss. Input: `SERVER/<program>/cmdstores.tsv` from the read-only `ExportCommandStores.java` (columns
fn insn base disp target). Within the listed init functions, in the given order then instruction order, the last
store to a slot wins (the runtime value; Steam slot 156 is stored twice).

Two separate facts per GOG `ExecuteCommand*` row (G = its GOG address):
- counterpart: the Steam handler whose slot set equals G's slot set (engine data, independent of any label);
- name: for `ExecuteCommand<X>` with <X> in nwscript.nss, whether slot(<X>) is in G's slot set (`name_slot` =
  yes / no / n/a). `no` = the GOG label contradicts the dispatch table (a label suspect for the gate).

Writes (never a database):
- B<n>/cmdtable.tsv          `index nwscript gog_handler steam_handler`
- B<n>/cmdtable_rows.tsv     per GOG row: `gog name gog_slots steam_counterpart match name_slot name_steam`
                             (name_steam = Steam handler of slot(<X>), the name-correct address)
- B<n>/extra_candidates.tsv  `gog steam source` for `batch.py candidates --extra-candidates`
- B<n>/cmdtable_report.md
"""
import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bdlib as bd  # noqa: E402
from batch import write_tsv  # noqa: E402

SLOTS = 0x36D
GOG_FNS = "0x77b530,0x7f9140"
STEAM_FNS = "0x665f50,0x6f5b80"
DEFAULT_NSS = Path.home() / "Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla/nwscript.nss"
NSS_TYPES = "int|float|void|object|string|location|vector|effect|event|talent|action|itemproperty"


def nss_functions(path: Path) -> list[str]:
    """Function names in declaration order (= command index)."""
    src = path.read_text(encoding="latin-1")
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    src = re.sub(r"//[^\n]*", "", src)
    return re.findall(r"^\s*(?:%s)\s+(\w+)\s*\(" % NSS_TYPES, src, re.M)


def decode(stores: Path, fns: list[int], slots: int = SLOTS) -> tuple[dict[int, int], list]:
    """-> (slot -> handler, overwrites [(slot, old, new, insn)])."""
    by_fn = defaultdict(list)
    for r in bd.iter_tsv(stores):
        by_fn[bd.parse_addr(r["fn"])].append((bd.parse_addr(r["insn"]), int(r["disp"], 16), bd.parse_addr(r["target"])))
    table, over = {}, []
    for f in fns:
        if f not in by_fn:
            raise SystemExit("%s has no stores for init function 0x%x" % (stores, f))
        for insn, disp, tgt in sorted(by_fn[f]):
            if disp % 4 or disp // 4 >= slots:
                continue
            i = disp // 4
            if i in table and table[i] != tgt:
                over.append((i, table[i], tgt, insn))
            table[i] = tgt
    return table, over


def slot_sets(table: dict[int, int]) -> dict[int, frozenset]:
    out = defaultdict(set)
    for i, h in table.items():
        out[h].add(i)
    return {h: frozenset(s) for h, s in out.items()}


def fmt_slots(s) -> str:
    return ",".join(str(i) for i in sorted(s))


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("batch")
    ap.add_argument("--gog-fns", default=GOG_FNS, help="GOG init functions, in call order")
    ap.add_argument("--steam-fns", default=STEAM_FNS, help="Steam init functions, in call order")
    ap.add_argument("--nss", default=str(DEFAULT_NSS))
    ap.add_argument("--prefix", default="ExecuteCommand", help="GOG name prefix before the nwscript name")
    a = ap.parse_args(argv)
    paths = bd.get_paths()
    bdir = paths.batches / a.batch
    bdir.mkdir(parents=True, exist_ok=True)

    names = nss_functions(Path(a.nss))
    if len(names) != SLOTS or len(set(names)) != SLOTS:
        raise SystemExit("nwscript.nss: %d functions (%d unique), expected %d" % (len(names), len(set(names)), SLOTS))
    index_of = {n: i for i, n in enumerate(names)}
    parse = lambda s: [int(x, 16) for x in s.split(",") if x]  # noqa: E731
    gt, gover = decode(paths.server / paths.gog_program / "cmdstores.tsv", parse(a.gog_fns))
    st, sover = decode(paths.server / paths.steam_program / "cmdstores.tsv", parse(a.steam_fns))
    gsets, ssets = slot_sets(gt), slot_sets(st)
    steam_by_set = defaultdict(list)
    for h, s in ssets.items():
        steam_by_set[s].append(h)
    write_tsv(bdir / "cmdtable.tsv", ["index", "nwscript", "gog_handler", "steam_handler"],
              [{"index": i, "nwscript": names[i], "gog_handler": bd.hx8(gt[i]) if i in gt else "",
                "steam_handler": bd.hx8(st[i]) if i in st else ""} for i in range(SLOTS)])

    # a handler stored first and then overwritten at the same slot on both sides (dead code in both exes) pairs too
    dead = {}
    for (gi, gold, _gn, _ga) in gover:
        for (si, sold, _sn, _sa) in sover:
            if gi == si and gold not in gsets and sold not in ssets:
                dead[gold] = (sold, gi)
    rows, extra = [], []
    for g in bd.load_gog_functions(paths.dump):
        if not g.func.startswith(a.prefix) or g.func == a.prefix:
            continue
        gs = gsets.get(g.addr, frozenset())
        cps = steam_by_set.get(gs, []) if gs else []
        match = "none" if not gs else ("exact" if len(cps) == 1 else ("ambiguous" if cps else "no_equal_set"))
        x = re.sub(r"_\d+$", "", g.func[len(a.prefix):])
        xi = index_of.get(x)
        name_slot = "n/a" if xi is None or not gs else ("yes" if xi in gs else "no")
        if not gs and g.addr in dead:
            cps, match = [dead[g.addr][0]], "overwritten_slot_%d" % dead[g.addr][1]
        rows.append({"gog": bd.hx8(g.addr), "name": g.name, "gog_slots": fmt_slots(gs),
                     "steam_counterpart": ",".join(bd.hx8(c) for c in cps), "match": match, "name_slot": name_slot,
                     "name_steam": bd.hx8(st[xi]) if xi is not None and xi in st else ""})
        if match == "exact":
            extra.append({"gog": bd.hx8(g.addr), "steam": bd.hx8(cps[0]), "source": "cmdtable"})
        elif match.startswith("overwritten"):
            extra.append({"gog": bd.hx8(g.addr), "steam": bd.hx8(cps[0]), "source": "cmdtable_overwritten"})
        elif gs:                                      # partial: every Steam handler of G's slots
            for h in sorted({st[i] for i in gs if i in st}):
                extra.append({"gog": bd.hx8(g.addr), "steam": bd.hx8(h), "source": "cmdtable_partial"})
    write_tsv(bdir / "cmdtable_rows.tsv", ["gog", "name", "gog_slots", "steam_counterpart", "match", "name_slot",
                                           "name_steam"], rows)
    write_tsv(bdir / "extra_candidates.tsv", ["gog", "steam", "source"], extra)
    # GOG-side card fact (no Steam address, so decoy cards stay blind)
    facts = []
    for h, sset in sorted(gsets.items()):
        cmds = ", ".join("%d %s" % (i, names[i]) for i in sorted(sset))
        facts.append({"gog": bd.hx8(h), "fact": "the GOG command table dispatches nwscript.nss function(s) [%s] to this "
                      "GOG function (InitCommands slot = nwscript declaration index)" % cmds})
    for gold, (_sold, i) in sorted(dead.items()):
        facts.append({"gog": bd.hx8(gold), "fact": "the GOG command table stores this GOG function at slot %d (%s) and "
                      "then overwrites that slot with another handler (dead code)" % (i, names[i])})
    write_tsv(bdir / "card_facts.tsv", ["gog", "fact"], facts)

    cnt = defaultdict(int)
    for r in rows:
        cnt["match:" + r["match"]] += 1
        cnt["name:" + r["name_slot"]] += 1
    gnames = {g.addr: g.name for g in bd.load_gog_functions(paths.dump)}
    unnamed = sorted(h for h in gsets if not gnames.get(h, "").split("::")[-1].startswith(a.prefix))
    rep = ["# %s command tables (GOG %s, Steam %s)" % (a.batch, a.gog_fns, a.steam_fns), "",
           "- slots %d; filled GOG %d / Steam %d; distinct handlers GOG %d / Steam %d" % (
               SLOTS, len(gt), len(st), len(gsets), len(ssets)),
           "- slot sets equal on both sides: %d of %d GOG handlers" % (
               sum(1 for s in gsets.values() if len(steam_by_set.get(s, [])) == 1), len(gsets)),
           "- GOG `%s*` rows %d: %s" % (a.prefix, len(rows), ", ".join("%s %d" % kv for kv in sorted(cnt.items()))), "",
           "## Slots stored twice (the later store is the runtime handler)", ""] + \
          ["- %s slot %d %s: 0x%08x overwritten by 0x%08x at 0x%08x" % (side, i, names[i], o, n, at)
           for side, ov in (("GOG", gover), ("Steam", sover)) for i, o, n, at in ov] + \
          ["", "## GOG label contradicts the dispatch table (name_slot = no)", ""] + \
          ["- %s %s slots {%s}; slot of the name = %d -> name-correct Steam %s" % (
              r["gog"], r["name"], r["gog_slots"], index_of[re.sub(r"_\d+$", "", r["name"].split("::")[-1][len(a.prefix):])],
              r["name_steam"]) for r in rows if r["name_slot"] == "no"] + \
          ["", "## GOG rows without an exact counterpart", ""] + \
          ["- %s %s slots {%s} -> %s (%s)" % (r["gog"], r["name"], r["gog_slots"], r["steam_counterpart"] or "-", r["match"])
           for r in rows if r["match"] != "exact"] + \
          ["", "## GOG table handlers with no `%s*` name in the GOG DB (%d)" % (a.prefix, len(unnamed)), ""] + \
          ["- 0x%08x %s slots {%s}" % (h, gnames.get(h, "(no DB row)"), fmt_slots(gsets[h])) for h in unnamed]
    (bdir / "cmdtable_report.md").write_text("\n".join(rep) + "\n", encoding="utf-8")
    print("\n".join(rep[2:5]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
