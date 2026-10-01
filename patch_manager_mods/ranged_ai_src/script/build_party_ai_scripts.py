#!/usr/bin/env python3
"""Build the KOTOR II party AI script set (17 override .ncs). Never installs, never writes the game folder.

  build_party_ai_scripts.py [--work DIR] [--baseline-only]     (default DIR = ~/Games/KOTOR2_Mod_Workspace/mod8_kite_script)

Design: patch_manager_mods/mods678_research/ai_review_2026-10-01/designs/design_ai_script_fixes.md.
Patch list: party_ai_patches.py (this folder); kite routine v2: kite_routine.nss; helper bodies: party_ai_fixes.nss.

Steps (each failure exits non-zero; nwnnsscomp exits 0 on errors, so only the output file counts):
  1. Inputs. DIR/vanilla(+_all) .nss and the recompile kit's includes must equal the Scripts.bif copies (read straight
     from chitin.key / data/scripts.bif, read-only). The BIF .ncs are written to DIR/vanilla_ncs/ with MD5SUMS.
  2. Vanilla proofs: the 17 sources (k_ai_master, 13 k_hen_<name>dlg, a_atkonend, a_force_combat, k_combat_rnd) plus
     k_hen_leadchng, k_oei_endconv, k_oei_spawn (proof only) rebuild byte-identical to the BIF .ncs with nwnnsscomp.
  3. Baseline: ONLY the two Mod 8 insertions + the v1 kite routine reproduce the live k_ai_master.ncs (md5 94db8304).
  4. Patch: every anchor of party_ai_patches.py occurs exactly once (per file). Sources go to DIR/src_party_ai/ (CRLF).
  5. Compile the 17 outputs to DIR/out_party_ai/ (k_ai_master against the patched includes, the 16 routers against the
     vanilla includes). Never pykotor's compile_nss (it swaps engine-call arguments).
  6. Static checks: routers (< 8 KB, call ExecuteScript, no combat engine calls), k_ai_master trace strings, and the
     medpac guard (no party path reaches tSpe11_05) on the generated source.
  7. scripts/check_ncs_arg_order.py --override-only on DIR/checkroot_party_ai/override/ (must flag 0).
  8. DIR/out_party_ai/manifest.json (name, md5, size, source md5s).

--baseline-only is the old build_kite_script.py behaviour: steps 1 (partly), 3, and writes DIR/out/k_ai_master.ncs
(the Mod 8-only build, md5 94db8304), DIR/src/k_inc_generic.nss and DIR/checkroot, then the arg-order check.
"""
import argparse, hashlib, json, os, re, shutil, struct, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, HERE)
import party_ai_patches as P  # noqa: E402

G = os.path.expanduser("~/.local/share/Steam/steamapps/common/Knights of the Old Republic II")
KIT = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh")
KIT_VANILLA = os.path.dirname(KIT) + "/vanilla"
VENV_PY = os.path.join(REPO, ".venv", "bin", "python")

LIVE_MD5 = "94db830439f1d33572ba911174e20a04"        # live override/k_ai_master.ncs (Mod 8 0.2.0 build)
V1_ROUTINE_MD5 = "66645afcf7c959b4a1448224158b0fdb"  # kite_routine.nss v1 (git dad84a5, the last v1 commit)
V1_ROUTINE_COMMIT = "dad84a5"

DLG = ["k_hen_%sdlg" % n for n in "atton baodur discip g0t0 hanharr hk47 hndmaid kreia mand mira remote t3m4 visas".split()]
WRAP = ["a_atkonend", "a_force_combat", "k_combat_rnd"]
MASTER = "k_ai_master"
OUTPUTS = [MASTER] + DLG + WRAP                      # the 17 deliverables
SPARE = ["k_hen_leadchng", "k_oei_endconv", "k_oei_spawn"]   # proof only (a future E5)
INCLUDES = ["k_inc_debug", "k_inc_drop", "k_inc_generic", "k_inc_gensupport", "k_inc_switch", "k_inc_utility",
            "k_inc_walkways", "nwscript"]

NSS, NCS = 2009, 2010


def md5b(b):
    return hashlib.md5(b).hexdigest()


def md5(p):
    return md5b(open(p, "rb").read())


def die(msg):
    print("FAIL:", msg)
    sys.exit(1)


def crlf(s):
    return P.crlf(s)


# ---------------------------------------------------------------------------------------------- Scripts.bif reader
class Bif:
    """Read-only chitin.key + Scripts.bif reader (no pykotor)."""

    def __init__(self, game=G):
        key = open(os.path.join(game, "chitin.key"), "rb").read()
        assert key[:8] == b"KEY V1  "
        nbif, nkey, ofile, okey = struct.unpack_from("<4I", key, 8)
        bifs = []
        for i in range(nbif):
            size, noff, nsize, drives = struct.unpack_from("<IIHH", key, ofile + 12 * i)
            bifs.append(key[noff:noff + nsize].rstrip(b"\0").decode().replace("\\", "/"))
        self.keys = {}
        for i in range(nkey):
            o = okey + 22 * i
            resref = key[o:o + 16].rstrip(b"\0").decode().lower()
            rtype, rid = struct.unpack_from("<HI", key, o + 16)
            self.keys[(resref, rtype)] = (bifs[rid >> 20], rid & 0xFFFFF)
        self.game = game
        self._bif = {}

    def get(self, resref, rtype):
        e = self.keys.get((resref.lower(), rtype))
        if not e:
            return None
        bifname, idx = e
        d = os.path.join(self.game, "data")
        base = os.path.basename(bifname).lower()
        p = next(os.path.join(d, f) for f in os.listdir(d) if f.lower() == base)
        if p not in self._bif:
            self._bif[p] = open(p, "rb").read()
        b = self._bif[p]
        assert b[:8] == b"BIFFV1  "
        nvar, nfix, ovar = struct.unpack_from("<3I", b, 8)
        for i in range(nvar):
            rid, off, size, typ = struct.unpack_from("<4I", b, ovar + 16 * i)
            if (rid & 0xFFFFF) == idx:
                return b[off:off + size]
        return None


# ---------------------------------------------------------------------------------------------- compile
def compile_ncs(out, *nss, cwd):
    if os.path.exists(out):
        os.remove(out)
    r = subprocess.run([KIT, out, *nss], cwd=cwd, capture_output=True, text=True)
    if not os.path.exists(out):   # nwnnsscomp exits 0 on errors: only the output file means success
        print(r.stdout[-3000:], r.stderr[-3000:])
        die(f"compile failed: {out}")
    return out


def arg_order_check(chk):
    r = subprocess.run([VENV_PY, os.path.join(REPO, "scripts", "check_ncs_arg_order.py"), chk, "--override-only"],
                       capture_output=True, text=True)
    out = (r.stdout + r.stderr).strip()
    print(out[-1500:])
    if r.returncode != 0 or not re.search(r"\b0 script\(s\) compiled with swapped engine-call arguments", out):
        die("check_ncs_arg_order.py did not report 0 swapped scripts")
    return out.splitlines()[-1]


def src_of(W, name):
    return os.path.join(W, "vanilla" if name == MASTER else "vanilla_all", name + ".nss")


# ---------------------------------------------------------------------------------------------- step 1
def step_inputs(W, bif):
    ncs_dir = os.path.join(W, "vanilla_ncs")
    os.makedirs(ncs_dir, exist_ok=True)
    sums = []
    for name in OUTPUTS + SPARE:
        data = bif.get(name, NCS)
        if data is None:
            die(f"{name}.ncs not in the BIFs")
        open(os.path.join(ncs_dir, name + ".ncs"), "wb").write(data)
        sums.append(f"{md5b(data)}  {name}.ncs")
        nss = bif.get(name, NSS)
        if nss is None or nss != open(src_of(W, name), "rb").read():
            die(f"{src_of(W, name)} differs from the Scripts.bif source")
    for inc in INCLUDES:
        b = bif.get(inc, NSS)
        for d in (os.path.join(W, "vanilla"), KIT_VANILLA):
            if b is None or b != open(os.path.join(d, inc + ".nss"), "rb").read():
                die(f"{d}/{inc}.nss differs from the Scripts.bif source")
    open(os.path.join(ncs_dir, "MD5SUMS"), "w").write("\n".join(sums) + "\n")
    print(f"[1] inputs: {len(OUTPUTS) + len(SPARE)} sources + {len(INCLUDES)} includes (workspace and kit) are "
          f"byte-identical to Scripts.bif; BIF .ncs written to {ncs_dir} (MD5SUMS)")


# ---------------------------------------------------------------------------------------------- step 2
def step_proofs(W):
    pdir = os.path.join(W, "proof_party_ai")
    shutil.rmtree(pdir, ignore_errors=True)
    os.makedirs(pdir)
    res = []
    for name in OUTPUTS + SPARE:
        out = compile_ncs(os.path.join(pdir, name + ".ncs"), src_of(W, name), cwd=W)
        ref = os.path.join(W, "vanilla_ncs", name + ".ncs")
        ok = open(out, "rb").read() == open(ref, "rb").read()
        res.append((name, ok, md5(ref)[:8], os.path.getsize(ref)))
        print(f"    proof {name:18s} {'PASS' if ok else 'FAIL'} {md5(ref)[:8]} {os.path.getsize(ref)}")
    if not all(ok for _, ok, _, _ in res):
        die("vanilla rebuild proof failed")
    print(f"[2] vanilla proofs: {len(res)}/{len(res)} PASS (byte-identical to the BIF .ncs)")
    return res


# ---------------------------------------------------------------------------------------------- step 3
def v1_routine(W):
    d = os.path.join(W, "baseline")
    os.makedirs(d, exist_ok=True)
    p = os.path.join(d, "kite_routine_v1.nss")
    if not os.path.exists(p):
        r = subprocess.run(["git", "-C", REPO, "show",
                            f"{V1_ROUTINE_COMMIT}:patch_manager_mods/ranged_ai_src/script/kite_routine.nss"],
                           capture_output=True)
        if r.returncode != 0:
            die("cannot recover kite_routine v1 from git " + V1_ROUTINE_COMMIT)
        open(p, "wb").write(r.stdout)
    if md5(p) != V1_ROUTINE_MD5:
        die(f"{p} md5 {md5(p)} != expected v1 {V1_ROUTINE_MD5}")
    return p


def step_baseline(W, write_legacy=False):
    rep = []
    gen = P.apply(open(os.path.join(W, "vanilla", "k_inc_generic.nss"), newline="").read(), P.MOD8_ONLY,
                  "k_inc_generic.nss", rep)
    gen = gen.rstrip("\r\n") + "\r\n\r\n" + crlf(open(v1_routine(W)).read())
    bdir = os.path.join(W, "baseline")
    mod = os.path.join(bdir, "k_inc_generic.nss")
    open(mod, "w", newline="").write(gen)
    old = os.path.join(W, "src", "k_inc_generic.nss")
    same_src = os.path.exists(old) and open(old, "rb").read() == gen.encode()
    out = compile_ncs(os.path.join(bdir, "k_ai_master.ncs"), os.path.join(W, "vanilla", "k_ai_master.nss"), mod, cwd=W)
    m = md5(out)
    live = os.path.join(G, "override", "k_ai_master.ncs")
    live_m = md5(live) if os.path.exists(live) else None
    print(f"[3] baseline: Mod 8 insertions only + kite routine v1 -> k_ai_master.ncs md5 {m} "
          f"({'==' if m == LIVE_MD5 else '!='} expected live {LIVE_MD5[:8]}; live override file: "
          f"{live_m[:8] if live_m else 'absent'}; generated include == previous src/k_inc_generic.nss: {same_src})")
    if m != LIVE_MD5:
        die("baseline does not reproduce the live k_ai_master.ncs")
    if write_legacy:   # build_kite_script.py behaviour: same paths as before
        os.makedirs(os.path.join(W, "src"), exist_ok=True)
        os.makedirs(os.path.join(W, "out"), exist_ok=True)
        shutil.copy(mod, os.path.join(W, "src", "k_inc_generic.nss"))
        shutil.copy(out, os.path.join(W, "out", "k_ai_master.ncs"))
        chk = os.path.join(W, "checkroot")
        shutil.rmtree(chk, ignore_errors=True)
        os.makedirs(os.path.join(chk, "override"))
        shutil.copy(out, os.path.join(chk, "override", "k_ai_master.ncs"))
        arg_order_check(chk)
        print(f"wrote {W}/out/k_ai_master.ncs ({os.path.getsize(out)} bytes, md5 {m})")
    return {"md5": m, "same_src": same_src, "anchors": rep}


# ---------------------------------------------------------------------------------------------- step 4
def step_patch(W):
    sdir = os.path.join(W, "src_party_ai")
    shutil.rmtree(sdir, ignore_errors=True)
    os.makedirs(sdir)
    rep = []
    V = os.path.join(W, "vanilla")
    gen = P.apply(open(os.path.join(V, "k_inc_generic.nss"), newline="").read(), P.PATCHES, "k_inc_generic.nss", rep)
    routine = open(os.path.join(HERE, "kite_routine.nss")).read()
    fixes = open(os.path.join(HERE, "party_ai_fixes.nss")).read()
    gen = gen.rstrip("\r\n") + "\r\n\r\n" + crlf(routine)
    gen = gen.rstrip("\r\n") + "\r\n\r\n" + crlf(fixes)
    sup = P.apply(open(os.path.join(V, "k_inc_gensupport.nss"), newline="").read(), P.PATCHES, "k_inc_gensupport.nss", rep)
    mas = P.apply(open(os.path.join(V, "k_ai_master.nss"), newline="").read(), P.PATCHES, "k_ai_master.nss", rep)
    for name, text in (("k_inc_generic.nss", gen), ("k_inc_gensupport.nss", sup), ("k_ai_master.nss", mas)):
        open(os.path.join(sdir, name), "w", newline="").write(text)
    for name in DLG:
        t = P.apply(open(src_of(W, name), newline="").read(), P.E2_DLG, name + ".nss", rep)
        open(os.path.join(sdir, name + ".nss"), "w", newline="").write(t)
    for fname, pl in P.E2_WRAP.items():
        t = P.apply(open(src_of(W, fname[:-4]), newline="").read(), pl, fname, rep)
        open(os.path.join(sdir, fname), "w", newline="").write(t)
    bad = [r for r in rep if r[3] != 1]
    # every patch id declared in the lists must have been applied (a typo in a file name would skip it silently)
    expect = len(P.PATCHES) + len(DLG) * len(P.E2_DLG) + sum(len(v) for v in P.E2_WRAP.values())
    if bad or len(rep) != expect:
        die(f"anchor report: {len(rep)} applied vs {expect} expected, bad: {bad}")
    for pid, scope, fname, n in rep:
        print(f"    anchor ok  {pid:14s} {scope:22s} {fname:22s} count={n}")
    print(f"[4] patch: {len(rep)} anchors, all count == 1; sources in {sdir}")
    return rep, sdir


# ---------------------------------------------------------------------------------------------- step 5
def step_compile(W, sdir):
    odir = os.path.join(W, "out_party_ai")
    shutil.rmtree(odir, ignore_errors=True)
    os.makedirs(odir)
    inc = [os.path.join(sdir, "k_inc_generic.nss"), os.path.join(sdir, "k_inc_gensupport.nss")]
    built = [compile_ncs(os.path.join(odir, MASTER + ".ncs"), os.path.join(sdir, MASTER + ".nss"), *inc, cwd=W)]
    for name in DLG + WRAP:   # routers: compiled against the VANILLA includes (they no longer call the combat AI)
        built.append(compile_ncs(os.path.join(odir, name + ".ncs"), os.path.join(sdir, name + ".nss"), cwd=W))
    for p in built:
        print(f"    built {os.path.basename(p):22s} {os.path.getsize(p):7d} bytes  md5 {md5(p)}")
    print(f"[5] compile: {len(built)} outputs in {odir}")
    return odir, built


# ---------------------------------------------------------------------------------------------- step 6
def nwscript_names(W):
    names = []
    for l in open(os.path.join(W, "vanilla", "nwscript.nss"), encoding="latin1"):
        m = re.match(r"^(void|int|float|string|object|vector|location|effect|event|talent|action)\s+(\w+)\s*\(", l)
        if m:
            names.append(m.group(2))
    assert names[8] == "ExecuteScript" and names[37] == "ActionAttack", "nwscript routine table mismatch"
    return names


def ncs_actions(path, names):
    logging_off()
    from pykotor.resource.formats.ncs import read_ncs
    n = read_ncs(path)
    return [names[i.args[0]] for i in n.instructions if i.ins_type.name == "ACTION"]


def logging_off():
    import logging
    logging.disable(logging.CRITICAL)


def medpac_guard_check(sdir):
    """Read the final generated k_inc_generic.nss: no party path reaches ActionUseTalentOnObject with tSpe11_05.
    tUse can only become the medpac talent through `tUse = tSpe11_05;`, so it is enough to prove that this is the only
    such assignment in the whole file, that it sits in the NPC `else` of an IsObjectPartyMember guard, and that the
    branch leading to it is closed for the party."""
    text = open(os.path.join(sdir, "k_inc_generic.nss"), newline="").read().replace("\r\n", "\n")
    lines = text.split("\n")
    start = next(i for i, l in enumerate(lines) if l.startswith("int GN_TalentMasterRoutine(") and not l.rstrip().endswith(";"))
    end = next(i for i in range(start, len(lines)) if lines[i].startswith("//:: Last Round Setup"))
    uses = [i for i, l in enumerate(lines) if "tSpe11_05" in l]
    outside = [i + 1 for i in uses if not (start <= i < end)]
    assigns = [i for i in uses if re.search(r"\btUse\s*=\s*tSpe11_05\b", lines[i])]
    # every other mention must be the declaration, a write to tSpe11_05 itself, a GetCreatureHasTalent test or a comment
    allowed = [r"^\s*talent tSpe11_01,", r"^\s*tSpe11_05 = ", r"GetCreatureHasTalent\(tSpe11_05\)", r"^\s*//"]
    odd = [i + 1 for i in uses if i not in assigns and not any(re.search(r, lines[i]) for r in allowed)]
    any_assign = odd
    inner = re.search(r"if\(IsObjectPartyMember\(OBJECT_SELF\)\)\s*\{[^{}]*bValid = FALSE;\s*\}\s*else\s*\{[^{}]*"
                      r"tUse = tSpe11_05;\s*\}", text)
    outer = re.search(r"else if\(!IsObjectPartyMember\(OBJECT_SELF\) &&\s*\(GetCreatureHasTalent\(tSpe11_05\) \|\|", text)
    if outside or len(assigns) != 1 or any_assign or not inner or not outer:
        print("tSpe11_05 lines:", [(i + 1, lines[i]) for i in uses])
        die("medpac guard check failed on the generated k_inc_generic.nss")
    # ANTIDOTE: the poison branch. TalentSpell(67) "Remove Poison Item" is the only item talent there; its use is behind a
    # party guard, and tSpe11_03 is bound to it nowhere else in that branch.
    pstart = next(i for i, l in enumerate(lines) if "nTalentConstant == GEN_TALENT_REMOVE_POISON" in l)
    pend = next(i for i in range(pstart + 1, len(lines)) if "nTalentConstant == GEN_TALENT_HEALING" in lines[i])
    ptxt = "\n".join(lines[pstart:pend])
    anti = (len(re.findall(r"TalentSpell\(67\)", text)) == 1 and ptxt.count("TalentSpell(67)") == 1 and
            len(re.findall(r"tUse\s*=\s*tSpe11_03\s*;", ptxt)) == 1 and
            re.search(r"else if\(!IsObjectPartyMember\(OBJECT_SELF\) && GetCreatureHasTalent\(tSpe11_03\) && "
                      r"oTarget == OBJECT_SELF\)\s*\{\s*bValid = TRUE;\s*tUse = tSpe11_03;\s*\}", ptxt))
    if not anti:
        print(ptxt)
        die("antidote guard check failed on the generated k_inc_generic.nss")
    ai = next(i for i in range(pstart, pend) if "tUse = tSpe11_03;" in lines[i] and "TalentSpell" not in lines[i])
    sel = next(i for i in uses if "GetCreatureHasTalent(tSpe11_05) ||" in lines[i])
    quote = {"selection branch (closed for the party)": lines[sel - 5:sel + 6],
             "the only tUse = tSpe11_05 (NPC else of a party guard)": lines[assigns[0] - 8:assigns[0] + 3],
             "poison: Remove Poison Item (closed for the party)": lines[ai - 7:ai + 3]}
    return quote, [i + 1 for i in uses], (start + 1, end + 1)


def step_static(W, sdir, odir, built):
    names = nwscript_names(W)
    combat = {"ActionAttack", "ActionUseTalentOnObject", "ActionCastSpellAtObject", "ActionMoveToObject",
              "ActionMoveToLocation", "ActionEquipItem", "ActionDoCommand"}
    rows = []
    for name in DLG + WRAP:
        p = os.path.join(odir, name + ".ncs")
        raw = open(p, "rb").read()
        acts = ncs_actions(p, names)
        ok = (len(raw) < 8192 and b"k_ai_master" in raw and "ExecuteScript" in acts and not (set(acts) & combat))
        rows.append((name, ok, len(raw), sorted(set(acts))))
        if not ok:
            die(f"router {name} failed the static check: size {len(raw)} actions {sorted(set(acts))}")
    # routing source check, on code only (// comments stripped): the one ExecuteScript call, no direct combat AI call
    def code(path):
        return "\n".join(re.sub(r"//.*", "", l) for l in open(path).read().splitlines())

    for name in DLG:
        t = code(os.path.join(sdir, name + ".nss"))
        if t.count('ExecuteScript("k_ai_master", OBJECT_SELF, 2004)') != 1 or "GN_RespondToShout" in t:
            die(f"{name}: routing source check failed")
    for name in WRAP:
        t = code(os.path.join(sdir, name + ".nss"))
        if t.count('ExecuteScript("k_ai_master", OBJECT_SELF, 3001)') != 1 or "GN_DetermineCombatRound" in t:
            die(f"{name}: routing source check failed")
    raw = open(os.path.join(odir, MASTER + ".ncs"), "rb").read()
    need = [b"Party E1:", b"Kite K2 cap", b"Kite K1 drop target", b"Party MEDPAC", b"Party H1:", b"Party E8:"]
    missing = [n for n in need if n not in raw]
    if missing:
        die(f"k_ai_master.ncs misses trace strings {missing}")
    quote, uses, (a, b) = medpac_guard_check(sdir)
    print(f"[6] static: {len(rows)} routers OK (< 8 KB, ExecuteScript, no combat calls); k_ai_master trace strings present; "
          f"medpac + antidote guards OK (tSpe11_05 used only on k_inc_generic.nss lines {uses}, inside GN_TalentMasterRoutine "
          f"{a}-{b})")
    return rows, quote, uses


# ---------------------------------------------------------------------------------------------- step 7/8
def step_manifest(W, sdir, odir, built, proofs, rep, argres):
    chk = os.path.join(W, "checkroot_party_ai")
    shutil.rmtree(chk, ignore_errors=True)
    os.makedirs(os.path.join(chk, "override"))
    for p in built:
        shutil.copy(p, os.path.join(chk, "override"))
    res = arg_order_check(chk)
    srcs = {n: md5(os.path.join(sdir, n)) for n in sorted(os.listdir(sdir))}
    van = {n: md5(os.path.join(W, "vanilla_ncs", n + ".ncs")) for n in OUTPUTS}
    entries = []
    for p in built:
        name = os.path.basename(p)[:-4]
        entries.append({
            "name": name + ".ncs", "md5": md5(p), "size": os.path.getsize(p),
            "vanilla_md5": van[name],
            "source": name + ".nss", "source_md5": srcs[name + ".nss"],
            "patched_with": ([pid for pid, _, f, _ in rep if f == name + ".nss"]),
            "includes_md5": ({"k_inc_generic.nss": srcs["k_inc_generic.nss"], "k_inc_gensupport.nss": srcs["k_inc_gensupport.nss"]}
                             if name == MASTER else "vanilla"),
        })
    man = {
        "built": time.strftime("%Y-%m-%d %H:%M:%S"),
        "tool": "patch_manager_mods/ranged_ai_src/script/build_party_ai_scripts.py",
        "compiler": "nwnnsscomp via " + KIT,
        "inputs": {"party_ai_patches.py": md5(os.path.join(HERE, "party_ai_patches.py")),
                   "kite_routine.nss": md5(os.path.join(HERE, "kite_routine.nss")),
                   "party_ai_fixes.nss": md5(os.path.join(HERE, "party_ai_fixes.nss")),
                   "vanilla_ncs_md5": van},
        "vanilla_proofs": [{"name": n, "pass": ok, "bif_md5_8": m, "bytes": sz} for n, ok, m, sz in proofs],
        "baseline_live_md5": LIVE_MD5,
        "arg_order": res,
        "anchors": len(rep),
        "files": entries,
    }
    mp = os.path.join(odir, "manifest.json")
    json.dump(man, open(mp, "w"), indent=2)
    print(f"[7] arg-order check: {res}")
    print(f"[8] manifest: {mp}")
    return man


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--work", default=os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/mod8_kite_script"))
    ap.add_argument("--baseline-only", action="store_true",
                    help="old build_kite_script.py behaviour: only the Mod 8 insertions -> DIR/out/k_ai_master.ncs")
    a = ap.parse_args()
    W = a.work
    if not os.path.exists(KIT):
        die("recompile kit missing: " + KIT)
    bif = Bif()
    step_inputs(W, bif)
    if a.baseline_only:
        # old toolchain proof (k_ai_master only) + baseline
        ref = os.path.join(W, "vanilla_ncs", MASTER + ".ncs")
        os.makedirs(os.path.join(W, "out"), exist_ok=True)
        vout = compile_ncs(os.path.join(W, "out", "k_ai_master_vanilla.ncs"), src_of(W, MASTER), cwd=W)
        if open(vout, "rb").read() != open(ref, "rb").read():
            die("toolchain proof failed")
        print(f"toolchain proof: vanilla rebuild byte-identical to BIF k_ai_master.ncs ({md5(ref)})")
        step_baseline(W, write_legacy=True)
        return
    proofs = step_proofs(W)
    step_baseline(W)
    rep, sdir = step_patch(W)
    odir, built = step_compile(W, sdir)
    rows, quote, uses = step_static(W, sdir, odir, built)
    man = step_manifest(W, sdir, odir, built, proofs, rep, None)
    print("\nMedpac guard (generated k_inc_generic.nss):")
    for k, v in quote.items():
        print(f"  -- {k}")
        for l in v:
            print("     " + l)
    print("\nmanifest summary:")
    for e in man["files"]:
        print(f"  {e['name']:24s} {e['size']:7d}  {e['md5']}")
    print("\nOK")


if __name__ == "__main__":
    main()
