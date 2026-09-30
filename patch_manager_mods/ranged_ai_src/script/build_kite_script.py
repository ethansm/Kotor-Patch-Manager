#!/usr/bin/env python3
"""Build k_ai_master.ncs with the Mod 8 "Ranged (Kiting)" dispatch (aistate 16). Never writes the game dir.

  build_kite_script.py [--work DIR]      (default DIR = ~/Games/KOTOR2_Mod_Workspace/mod8_kite_script)

1. Reads vanilla k_inc_generic.nss (DIR/vanilla, CRLF) and inserts, at asserted anchors:
   a prototype, the NPC_AISTYLE_PARTY_KITE constant, the dispatch branch after NPC_AISTYLE_PARTY_RANGED in
   GN_DetermineCombatRound, and kite_routine.nss (this folder) at the end of the file. Writes DIR/src/k_inc_generic.nss.
2. Proves the toolchain first: compiles UNMODIFIED k_ai_master.nss + vanilla k_inc_generic.nss and requires a
   byte-identical match with the BIF k_ai_master.ncs (DIR/vanilla/k_ai_master.ncs).
3. Compiles the modified build to DIR/out/k_ai_master.ncs (nwnnsscomp via the recompile kit; never pykotor's
   compile_nss, which swaps engine-call arguments) and runs scripts/check_ncs_arg_order.py --override-only on it.
Doc: patch_manager_mods/13_ranged_ai_spacing.md sec. 3.
"""
import argparse, hashlib, os, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
KIT = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/script_recompile_20260924/tools/compile.sh")
VENV_PY = os.path.join(REPO, ".venv", "bin", "python")


def md5(p):
    return hashlib.md5(open(p, "rb").read()).hexdigest()


def insert_once(text, anchor, new, after=True):
    n = text.count(anchor)
    if n != 1:
        sys.exit(f"anchor found {n} times (need 1): {anchor[:80]!r}")
    i = text.index(anchor) + (len(anchor) if after else 0)
    return text[:i] + new + text[i:]


def crlf(s):
    return s.replace("\r\n", "\n").replace("\n", "\r\n")


def compile_ncs(out, *nss, cwd):
    if os.path.exists(out):
        os.remove(out)
    r = subprocess.run([KIT, out, *nss], cwd=cwd, capture_output=True, text=True)
    if not os.path.exists(out):   # nwnnsscomp exits 0 on errors: only the output file means success
        print(r.stdout[-3000:], r.stderr[-3000:])
        sys.exit(f"compile failed: {out}")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--work", default=os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/mod8_kite_script"))
    a = ap.parse_args()
    W = a.work
    van = os.path.join(W, "vanilla")
    os.makedirs(os.path.join(W, "src"), exist_ok=True)
    os.makedirs(os.path.join(W, "out"), exist_ok=True)

    # 1. toolchain proof on the unmodified files
    ref = os.path.join(van, "k_ai_master.ncs")
    vout = compile_ncs(os.path.join(W, "out", "k_ai_master_vanilla.ncs"), os.path.join(van, "k_ai_master.nss"),
                       os.path.join(van, "k_inc_generic.nss"), cwd=W)
    if open(vout, "rb").read() != open(ref, "rb").read():
        sys.exit(f"toolchain proof FAILED: {md5(vout)} != BIF {md5(ref)}")
    print(f"toolchain proof: vanilla rebuild byte-identical to BIF k_ai_master.ncs ({md5(ref)})")

    # 2. modified include
    src = open(os.path.join(van, "k_inc_generic.nss"), newline="").read()
    src = insert_once(src, "int GN_CheckRangedPreferred();\r\n", crlf(
        "//Mod 8 (ranged-ai) - party stance 16 \"Ranged (Kiting)\": keeps its distance, repositions when the\n"
        "//shot is blocked, switches to melee if cornered. Body at the end of this file.\n"
        "int NPC_AISTYLE_PARTY_KITE = 16;\n"
        "int GN_RunKiteAIRoutine(object oIntruder = OBJECT_INVALID);\n"))
    ranged_branch = ("                else if(nNPC_AI == NPC_AISTYLE_PARTY_RANGED)\r\n"
                     "                {\r\n")
    i = src.index(ranged_branch)
    # end of the PARTY_RANGED branch = the first "                }\r\n" (16 spaces) after it
    close = "\r\n                }\r\n"
    j = src.index(close, i) + len(close)
    branch = crlf(
        "                else if (nNPC_AI == NPC_AISTYLE_PARTY_KITE)\n"
        "                {\n"
        "                    //Mod 8: same leash as Ranged Support, then the kite routine.\n"
        "                    if (GN_CheckRangeFromLeader(10.0f) == TRUE)\n"
        "                    {\n"
        "                        return;\n"
        "                    }\n"
        "                    GN_RunKiteAIRoutine(oIntruder);\n"
        "                    return;\n"
        "                }\n")
    if not src[j:].startswith("                else if (nNPC_AI == NPC_AISTYLE_PARTY_STATIONARY)"):
        sys.exit("dispatch anchor: PARTY_RANGED branch is not followed by PARTY_STATIONARY")
    src = src[:j] + branch + src[j:]
    routine = open(os.path.join(HERE, "kite_routine.nss")).read()
    src = src.rstrip("\r\n") + "\r\n\r\n" + crlf(routine)
    mod = os.path.join(W, "src", "k_inc_generic.nss")
    open(mod, "w", newline="").write(src)
    print("wrote", mod)

    # 3. modified build + arg-order check
    out = compile_ncs(os.path.join(W, "out", "k_ai_master.ncs"), os.path.join(van, "k_ai_master.nss"), mod, cwd=W)
    print(f"built {out} ({os.path.getsize(out)} bytes, md5 {md5(out)})")
    chk = os.path.join(W, "checkroot")
    shutil.rmtree(chk, ignore_errors=True)
    os.makedirs(os.path.join(chk, "override"))
    shutil.copy(out, os.path.join(chk, "override", "k_ai_master.ncs"))
    r = subprocess.run([VENV_PY, os.path.join(REPO, "scripts", "check_ncs_arg_order.py"), chk, "--override-only"],
                       capture_output=True, text=True)
    print(r.stdout.strip()[-1500:])
    if r.returncode != 0:
        sys.exit("check_ncs_arg_order.py reported a problem")


if __name__ == "__main__":
    main()
