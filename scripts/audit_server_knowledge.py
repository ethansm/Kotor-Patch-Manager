#!/usr/bin/env python3
"""
Audit script: Compares upstream OpenKotOR Ghidra Server knowledge against:
1. Local Steam Database: AddressDatabases/kotor2_steam_aspyr.db (1,704 validated functions)
2. Local Ghidra Knowledge Base: patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db (4,057 name_map entries)
3. Local GOG Database: AddressDatabases/kotor2_gog_aspyr.db
"""

import os
import sys
import sqlite3
from pathlib import Path
from collections import defaultdict

UPSTREAM_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server")
LOCAL_STEAM_DB = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db")
LOCAL_GOG_DB = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_gog_aspyr.db")
KNOWLEDGE_DB = Path("/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db")

def parse_tsv(path):
    if path is None or not path.exists():
        return None
    rows = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        header = None
        for line in f:
            parts = line.strip().split("\t")
            if not header:
                header = [p.lower() for p in parts]
                continue
            if len(parts) >= len(header):
                row = dict(zip(header, parts[:len(header)]))
                rows.append(row)
    return rows

def load_local_db(db_path):
    conn = sqlite3.connect(db_path)
    cur = conn.cursor()
    functions = {}
    for row in cur.execute("SELECT class_name, function_name, address, notes, calling_convention, param_size_bytes FROM functions"):
        cname, fname, addr, notes, callconv, paramsize = row
        hex_addr = f"0x{addr:08x}".lower()
        functions[hex_addr] = {
            "class_name": cname,
            "function_name": fname,
            "full_name": f"{cname}::{fname}" if cname else fname,
            "address": addr,
            "notes": notes,
            "calling_convention": callconv,
            "param_size_bytes": paramsize
        }
    
    globals_dict = {}
    for row in cur.execute("SELECT pointer_name, address, notes FROM global_pointers"):
        pname, addr, notes = row
        hex_addr = f"0x{addr:08x}".lower()
        globals_dict[hex_addr] = {"name": pname, "address": addr, "notes": notes}
        
    offsets = {}
    for row in cur.execute("SELECT class_name, member_name, offset, notes FROM offsets"):
        cname, mname, off, notes = row
        offsets[f"{cname}::{mname}"] = {"class_name": cname, "member_name": mname, "offset": off, "notes": notes}
        
    return functions, globals_dict, offsets

def load_knowledge_db(db_path):
    conn = sqlite3.connect(db_path)
    cur = conn.cursor()
    name_map = {}
    for row in cur.execute("SELECT steam_addr, name, source, confidence, evidence FROM name_map"):
        saddr, name, src, conf, evid = row
        if not saddr: continue
        clean_addr = saddr.lower()
        if not clean_addr.startswith("0x"):
            clean_addr = "0x" + clean_addr
        name_map[clean_addr] = {"name": name, "source": src, "confidence": conf, "evidence": evid}
    return name_map

def main():
    print("=" * 70)
    print("  OpenKotOR Ghidra Server vs Local Knowledge Audit Report")
    print("=" * 70)
    
    # 1. Load Local State
    if not LOCAL_STEAM_DB.exists():
        print(f"ERROR: Local Steam DB not found at {LOCAL_STEAM_DB}")
        return 1
    
    local_steam_funcs, local_steam_globals, local_steam_offsets = load_local_db(LOCAL_STEAM_DB)
    local_gog_funcs, local_gog_globals, local_gog_offsets = load_local_db(LOCAL_GOG_DB) if LOCAL_GOG_DB.exists() else ({}, {}, {})
    knowledge_name_map = load_knowledge_db(KNOWLEDGE_DB) if KNOWLEDGE_DB.exists() else {}

    print(f"\n[1] Local Database & Knowledge Inventory:")
    print(f"  • Local Steam Aspyr DB (`kotor2_steam_aspyr.db`):")
    print(f"      - Validated Functions:      {len(local_steam_funcs):>6}")
    print(f"      - Validated Global Pointers:{len(local_steam_globals):>6}")
    print(f"      - Validated Struct Offsets: {len(local_steam_offsets):>6}")
    print(f"  • Local Knowledge Map (`kotor2_ghidra_knowledge.db`):")
    print(f"      - Total Mapped Symbols:     {len(knowledge_name_map):>6}")
    print(f"  • Local GOG Aspyr DB (`kotor2_gog_aspyr.db`):")
    print(f"      - Functions in DB:          {len(local_gog_funcs):>6}")

    # 2. Check Upstream Server TSV Exports
    steam_candidates = [
        UPSTREAM_DIR / "swkotor2_aspyr.exe_functions.tsv",
        UPSTREAM_DIR / "k2_win_steam_aspyr_swkotor2.exe_functions.tsv",
    ]
    steam_server_fn_file = next((f for f in steam_candidates if f.exists()), None)
    if not steam_server_fn_file:
        # Fallback to any functions.tsv containing steam or aspyr
        found = list(UPSTREAM_DIR.glob("*aspyr*functions.tsv")) + list(UPSTREAM_DIR.glob("*steam*functions.tsv"))
        if found:
            steam_server_fn_file = found[0]

    if not steam_server_fn_file:
        print(f"\n[!] Upstream server export files not found in {UPSTREAM_DIR}")
        print(f"    Expected: swkotor2_aspyr.exe_functions.tsv or k2_win_steam_aspyr_swkotor2.exe_functions.tsv")
        print(f"    Run `scripts/export_server_knowledge.sh` to fetch upstream symbols.")
        return 0

    base_stem = steam_server_fn_file.name.replace("_functions.tsv", "")
    steam_server_sym_file = UPSTREAM_DIR / f"{base_stem}_symbols.tsv"

    gog_candidates = [
        UPSTREAM_DIR / "k2_win_gog_aspyr_swkotor2.exe_functions.tsv",
        UPSTREAM_DIR / "swkotor2_gog.exe_functions.tsv",
    ]
    gog_server_fn_file = next((f for f in gog_candidates if f.exists()), None)
    gog_server_sym_file = UPSTREAM_DIR / f"{gog_server_fn_file.name.replace('_functions.tsv', '')}_symbols.tsv" if gog_server_fn_file else None

    print(f"\n[2] Upstream Server Inventory ({steam_server_fn_file.name}):")
    steam_server_fns = parse_tsv(steam_server_fn_file) or []
    steam_server_syms = parse_tsv(steam_server_sym_file) or []
    gog_server_fns = parse_tsv(gog_server_fn_file) or []
    gog_server_syms = parse_tsv(gog_server_sym_file) or []

    # Process Steam Server Functions
    # Named vs Unnamed (FUN_*)
    server_steam_named = {}
    server_steam_unnamed = {}
    for r in steam_server_fns:
        addr = r.get("addr", "").lower()
        name = r.get("name", "")
        ns = r.get("namespace", "")
        source = r.get("source", "")
        full = f"{ns}::{name}" if ns else name
        
        is_default = name.startswith("FUN_") or source == "DEFAULT"
        if is_default:
            server_steam_unnamed[addr] = r
        else:
            server_steam_named[addr] = full

    print(f"  • Upstream Steam Binary (`k2_win_steam_aspyr_swkotor2.exe`):")
    print(f"      - Total Function Listing:   {len(steam_server_fns):>6}")
    print(f"      - Named / Labeled Functions:{len(server_steam_named):>6}")
    print(f"      - Unnamed (FUN_*) Functions:{len(server_steam_unnamed):>6}")
    print(f"      - Other Labels/Symbols:     {len(steam_server_syms):>6}")

    if gog_server_fns:
        server_gog_named = {}
        for r in gog_server_fns:
            addr = r.get("addr", "").lower()
            name = r.get("name", "")
            ns = r.get("namespace", "")
            source = r.get("source", "")
            if not (name.startswith("FUN_") or source == "DEFAULT"):
                server_gog_named[addr] = f"{ns}::{name}" if ns else name
        print(f"  • Upstream GOG Binary (`k2_win_gog_aspyr_swkotor2.exe`):")
        print(f"      - Total Function Listing:   {len(gog_server_fns):>6}")
        print(f"      - Named / Labeled Functions:{len(server_gog_named):>6}")

    # 3. Comparative Analysis: Local Steam DB vs Server Steam Symbols
    print(f"\n[3] Local Steam DB (1,704 validated) vs Upstream Server Steam Parity:")
    matches = 0
    name_conflicts = []
    unnamed_on_server = []
    missing_on_server = []

    for addr, lfunc in local_steam_funcs.items():
        if addr in server_steam_named:
            sname = server_steam_named[addr]
            lname = lfunc["full_name"]
            # Normalize names
            norm_sname = sname.replace(" ", "").replace("__thiscall", "").replace("::", ".")
            norm_lname = lname.replace(" ", "").replace("__thiscall", "").replace("::", ".")
            if norm_sname == norm_lname or lname in sname or sname in lname:
                matches += 1
            else:
                name_conflicts.append((addr, lname, sname))
        elif addr in server_steam_unnamed:
            unnamed_on_server.append((addr, lfunc["full_name"]))
        else:
            missing_on_server.append((addr, lfunc["full_name"]))

    print(f"      - Exact/Normalized Matches: {matches:>6} / {len(local_steam_funcs)} ({matches*100/len(local_steam_funcs):.1f}%)")
    print(f"      - Unnamed on Upstream Server:{len(unnamed_on_server):>6} (Known in our DB, FUN_* upstream)")
    print(f"      - Name Conflicts / Variants:{len(name_conflicts):>6}")
    print(f"      - Missing from Server Table:{len(missing_on_server):>6}")

    if name_conflicts:
        print(f"\n    Sample Naming Conflicts (First 10):")
        for addr, lname, sname in name_conflicts[:10]:
            print(f"      {addr}: Local DB '{lname}' vs Server '{sname}'")

    # 4. What Upstream Server Has That Our Steam DB Doesn't
    server_exclusive = {}
    for addr, sname in server_steam_named.items():
        if addr not in local_steam_funcs:
            server_exclusive[addr] = sname

    print(f"\n[4] Knowledge Delta:")
    print(f"  • Upstream Named Symbols NOT yet in Local Steam DB: {len(server_exclusive):>5}")
    print(f"  • Local Validated Functions NOT yet Named Upstream:  {len(unnamed_on_server):>5}")

    # Breakdown of classes in upstream server exclusives
    class_tally = defaultdict(int)
    for addr, sname in server_exclusive.items():
        cname = sname.split("::")[0] if "::" in sname else "Global"
        class_tally[cname] += 1

    top_classes = sorted(class_tally.items(), key=lambda x: x[1], reverse=True)[:15]
    print(f"\n  Top Subsystems in Upstream Server Available for Import:")
    for cname, count in top_classes:
        in_local = sum(1 for f in local_steam_funcs.values() if f["class_name"] == cname)
        print(f"    - {cname:<35} Server: {count:>4} new | Local has: {in_local:>4}")

    print("\n" + "=" * 70)
    print("  Audit Summary Complete.")
    print("=" * 70)
    return 0

if __name__ == "__main__":
    sys.exit(main())
