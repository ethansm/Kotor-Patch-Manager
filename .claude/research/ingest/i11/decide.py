"""Per shortlist row: Steam callers (fp graph) mapped to GOG via trusted_map; which GOG candidate has that caller."""
import os, sys
sys.path.insert(0, "/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ghidra_knowledge/tools")
import ingest_names as I
srv = os.path.expanduser(I.SERVER_DEFAULT)
steam, sg = I._load_fp(os.path.join(srv, I.FP_STEAM, "fp.tsv"))
gog, gg = I._load_fp(os.path.join(srv, I.FP_GOG, "fp.tsv"))
g2s, s2g = I.trusted_map(sg, gg, I.KDB_DEFAULT)
sc, gc = I._caller_index(steam), I._caller_index(gog)
for r in I.read_tsv(sys.argv[1]):
    group = [int(r["steam"], 16)] + [int(x, 16) for x in r["steam_peers"].split(",") if x]
    cands = [int(x, 16) for x in r["gog_addrs"].split(",")]
    names = r["name"].split(",")
    print("ROW", r["steam"], "peers", r["steam_peers"])
    for g, n in zip(cands, names):
        print("   GOG 0x%08x %-45s callers=%s callees=%d" % (g, n, ",".join("0x%x%s" % (c, "->S0x%x" % g2s[c] if c in g2s else "") for c in sorted(gc.get(g, ()))), len(I._callee_set(gog[g])) if g in gog else -1))
    for s in group:
        out = []
        for c in sorted(sc.get(s, ())):
            m = s2g.get(c)
            hit = [("0x%x" % g) for g in cands if m is not None and m in gc.get(g, ())]
            out.append("0x%x%s%s" % (c, "->G0x%x" % m if m else "", "=>" + "/".join(hit) if hit else ""))
        # callee side: GOG candidate callees mapped to Steam callees
        hits = []
        for g in cands:
            if g in gog and s in steam:
                o = sum(g2s.get(x) in I._callee_set(steam[s]) for x in I._callee_set(gog[g]))
                hits.append(o)
        print("   STEAM 0x%08x callers=%s calleehits=%s" % (s, " ".join(out), hits))
