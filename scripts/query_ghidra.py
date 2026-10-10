#!/usr/bin/env python3
"""query_ghidra.py - Fast, deterministic CLI for kotor2_ghidra_knowledge.db

Provides instant lookup for symbols, addresses, and struct layouts
without writing inline SQLite scripts.

Exit codes:
  0 = Results found
  1 = No matches found
  2 = Usage or database connection error
"""

import argparse
import json
import os
import re
import sqlite3
import sys
from pathlib import Path

DEFAULT_DB_PATH = Path(__file__).resolve().parent.parent / "patch_manager_mods" / "ghidra_knowledge" / "kotor2_ghidra_knowledge.db"

def normalize_hex_address(addr_str: str) -> str:
    """Normalizes address strings like '0x0053f000', '53F000', '0x53f000' to '0x%08x' and integer."""
    clean = addr_str.strip().lower()
    if clean.startswith("0x"):
        clean = clean[2:]
    try:
        val = int(clean, 16)
        return f"0x{val:08x}", f"0x{val:x}", f"{val:x}", val
    except ValueError:
        return None, None, None, None

def connect_db(db_path: Path) -> sqlite3.Connection:
    if not db_path.exists():
        sys.stderr.write(f"Error: Database not found at {db_path}\n")
        sys.exit(2)
    try:
        con = sqlite3.connect(f"file:{db_path}?mode=ro", uri=True)
        con.row_factory = sqlite3.Row
        return con
    except sqlite3.Error as e:
        sys.stderr.write(f"Database connection error: {e}\n")
        sys.exit(2)

def query_symbol(con: sqlite3.Connection, query: str, limit: int = 25) -> list[dict]:
    results = []
    cur = con.cursor()
    
    # 1. Check server_functions
    rows = cur.execute(
        """SELECT 'server_function' as source_table, program, addr, name, namespace, prototype, size, trust 
           FROM server_functions 
           WHERE name = ? OR name LIKE ? 
           ORDER BY (name = ?) DESC, trust DESC LIMIT ?""",
        (query, f"%{query}%", query, limit)
    ).fetchall()
    for r in rows:
        results.append(dict(r))

    # 2. Check struct_fields (if queried by name or class)
    if len(results) < limit:
        remaining = limit - len(results)
        s_rows = cur.execute(
            """SELECT 'struct_field' as source_table, class, offset, size, type, name, confidence, readers, writers 
               FROM struct_fields 
               WHERE name = ? OR name LIKE ? OR class = ? LIMIT ?""",
            (query, f"%{query}%", query, remaining)
        ).fetchall()
        for r in s_rows:
            d = dict(r)
            d["offset_hex"] = f"0x{d['offset']:x}"
            results.append(d)

    # 3. Check steam_validated
    if len(results) < limit:
        remaining = limit - len(results)
        sv_rows = cur.execute(
            """SELECT 'steam_validated' as source_table, steam_addr, name, verdict, tier, gog_addr, batch 
               FROM steam_validated 
               WHERE name = ? OR name LIKE ? LIMIT ?""",
            (query, f"%{query}%", remaining)
        ).fetchall()
        for r in sv_rows:
            results.append(dict(r))

    return results

def query_address(con: sqlite3.Connection, raw_addr: str) -> list[dict]:
    formatted, short_hex, bare_hex, int_val = normalize_hex_address(raw_addr)
    if formatted is None:
        sys.stderr.write(f"Error: Invalid hex address format: {raw_addr}\n")
        sys.exit(2)

    results = []
    cur = con.cursor()

    # Search server_functions (matches '0x%08x', '0x%x', or exact string)
    rows = cur.execute(
        """SELECT 'server_function' as source_table, program, addr, name, namespace, prototype, size, retn, trust 
           FROM server_functions 
           WHERE addr = ? OR addr = ? OR addr = ?""",
        (formatted, short_hex, bare_hex)
    ).fetchall()
    for r in rows:
        results.append(dict(r))

    # Search steam_validated
    sv_rows = cur.execute(
        """SELECT 'steam_validated' as source_table, steam_addr, name, verdict, tier, gog_addr, batch 
           FROM steam_validated 
           WHERE steam_addr = ? OR steam_addr = ? OR steam_addr = ? OR gog_addr = ? OR gog_addr = ?""",
        (formatted, short_hex, bare_hex, formatted, short_hex)
    ).fetchall()
    for r in sv_rows:
        results.append(dict(r))

    # Search name_map
    nm_rows = cur.execute(
        """SELECT 'name_map' as source_table, steam_addr, name, source, confidence, evidence 
           FROM name_map 
           WHERE steam_addr = ? OR steam_addr = ? OR steam_addr = ?""",
        (formatted, short_hex, bare_hex)
    ).fetchall()
    for r in nm_rows:
        results.append(dict(r))

    return results

def query_struct(con: sqlite3.Connection, class_name: str, limit: int = 100) -> list[dict]:
    cur = con.cursor()
    rows = cur.execute(
        """SELECT class, offset, size, type, name, confidence, readers, writers 
           FROM struct_fields 
           WHERE class = ? OR class LIKE ? 
           ORDER BY offset ASC LIMIT ?""",
        (class_name, f"%{class_name}%", limit)
    ).fetchall()
    
    results = []
    for r in rows:
        d = dict(r)
        d["offset_hex"] = f"0x{d['offset']:04x}"
        results.append(d)
    return results

def format_cpp_struct(results: list[dict], class_name: str) -> str:
    lines = [f"// Auto-generated struct layout for {class_name}", f"struct {class_name} {{"]
    type_map = {
        "ptr": "void*",
        "int": "int32_t",
        "i32": "int32_t",
        "u32": "uint32_t",
        "word": "uint16_t",
        "byte": "uint8_t",
        "f32": "float",
        "id": "uint32_t",
    }
    for item in results:
        off = item.get("offset", 0)
        t_raw = str(item.get("type", "")).strip().lower()
        c_type = type_map.get(t_raw, "uint32_t")
        raw_name = item.get("name", f"field_0x{off:x}")
        clean_name = re.sub(r"[^a-zA-Z0-9_]", "_", raw_name).strip("_")
        if not clean_name or clean_name[0].isdigit():
            clean_name = f"field_{clean_name}"
        lines.append(f"    /* 0x{off:04x} */ {c_type:10s} {clean_name};")
    lines.append("};")
    return "\n".join(lines)

def get_stats(con: sqlite3.Connection) -> dict:
    cur = con.cursor()
    stats = {}
    tables = ["server_functions", "server_symbols", "server_types", "struct_fields", "steam_validated", "name_map"]
    for t in tables:
        try:
            cnt = cur.execute(f"SELECT COUNT(*) FROM {t}").fetchone()[0]
            stats[t] = cnt
        except sqlite3.Error:
            stats[t] = 0
    return stats

def main():
    parent_parser = argparse.ArgumentParser(add_help=False)
    parent_parser.add_argument("--db", type=Path, default=DEFAULT_DB_PATH, help="Path to kotor2_ghidra_knowledge.db")
    parent_parser.add_argument("--json", action="store_true", help="Output results in JSON format")
    parent_parser.add_argument("--limit", type=int, default=25, help="Maximum number of results to display")

    parser = argparse.ArgumentParser(description="Query KOTOR II Ghidra knowledge database", parents=[parent_parser])
    subparsers = parser.add_subparsers(dest="command", required=True)

    # Subcommand: symbol
    p_sym = subparsers.add_parser("symbol", parents=[parent_parser], help="Search by symbol or function name")
    p_sym.add_argument("query", help="Symbol name or substring")

    # Subcommand: address
    p_addr = subparsers.add_parser("address", parents=[parent_parser], help="Lookup by hex address (e.g. 0x0053f000)")
    p_addr.add_argument("address", help="Hex address")

    # Subcommand: struct
    p_struct = subparsers.add_parser("struct", parents=[parent_parser], help="Lookup struct fields by class name")
    p_struct.add_argument("class_name", help="Class name (e.g. CSWSCreature)")
    p_struct.add_argument("--cpp", action="store_true", help="Format struct fields as C++ struct declaration")

    # Subcommand: stats
    subparsers.add_parser("stats", parents=[parent_parser], help="Show knowledge database statistics")

    args = parser.parse_args()
    con = connect_db(args.db)

    results = []
    if args.command == "symbol":
        results = query_symbol(con, args.query, limit=args.limit)
    elif args.command == "address":
        results = query_address(con, args.address)
    elif args.command == "struct":
        results = query_struct(con, args.class_name, limit=args.limit)
    elif args.command == "stats":
        stats = get_stats(con)
        if args.json:
            print(json.dumps(stats, indent=2))
        else:
            print("Ghidra Knowledge Database Stats:")
            for k, v in stats.items():
                print(f"  {k:20s}: {v:6d} entries")
        sys.exit(0)

    if not results:
        if args.json:
            print(json.dumps({"status": "not_found", "matches": []}))
        else:
            print(f"No results found for query: {getattr(args, 'query', getattr(args, 'address', getattr(args, 'class_name', '')))}")
        sys.exit(1)

    if args.json:
        print(json.dumps(results, indent=2))
    elif args.command == "struct" and getattr(args, "cpp", False):
        print(format_cpp_struct(results, args.class_name))
    else:
        for i, item in enumerate(results, 1):
            source = item.get("source_table", args.command)
            if source == "server_function":
                print(f"[{item.get('program', 'k2')}] {item.get('addr')} | {item.get('name')} | trust: {item.get('trust')} | size: {item.get('size')}b")
                if item.get("prototype"):
                    print(f"       proto: {item.get('prototype')}")
            elif source == "struct_field" or args.command == "struct":
                print(f"  {item.get('offset_hex')} (+{item.get('offset'):4d}) | size: {str(item.get('size')):2s} | {item.get('type', ''):18s} | {item.get('class')}::{item.get('name')}")
            elif source == "steam_validated":
                print(f"[steam_val] Steam: {item.get('steam_addr')} -> GOG: {item.get('gog_addr')} | {item.get('name')} | verdict: {item.get('verdict')} (batch {item.get('batch')})")
            elif source == "name_map":
                print(f"[name_map] {item.get('steam_addr')} | {item.get('name')} | source: {item.get('source')} | conf: {item.get('confidence')}")
            else:
                print(f"  {item}")

    sys.exit(0)

if __name__ == "__main__":
    main()
