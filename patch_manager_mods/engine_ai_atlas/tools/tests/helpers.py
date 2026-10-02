"""Shared test helpers: fixture root, tiny DB builder, stdout capture."""
import contextlib
import io
import os
import sqlite3
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOOLS = os.path.dirname(HERE)
if TOOLS not in sys.path:
    sys.path.insert(0, TOOLS)

FIXROOT = os.path.join(HERE, "fixtures", "root")
STEAM = "6a522e71631dcee93467bd2010f3b23d9145326e1e2e89305f13ab104dbbffef"
OTHER = "4ab72fc1ab082f427e008cda32fc5602d27b4e12fef48c4a1a2c6f7b2f36fb5a"
REAL_ROOT = os.path.expanduser("~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/")
REAL_DB = os.path.join(os.path.dirname(TOOLS), "..", "ghidra_knowledge", "kotor2_ghidra_knowledge.db")


def build_db(path, with_struct_fields=False):
    c = sqlite3.connect(path)
    c.executescript("""
    create table name_map(steam_addr text primary key, name text, source text, confidence text, evidence text, date text);
    create table functions(binary_hash text, address text, symbol text, size integer, purpose text, notes text, discovered_date text);
    create table class_methods(binary_hash text, class_name text, vtable_index integer, address text, size integer, symbol text, purpose text, notes text);
    create table candidates(id text primary key, title text, subsystem text, hypothesis text, evidence text, expected_win text, risk text, status text, decision_date text);
    create table lessons(id integer primary key autoincrement, scope text, topic text, note text, date text);
    """)
    c.executemany("insert into name_map values(?,?,?,?,?,?)", [
        ("0x00401000", "Foo::Alpha", "engine-ai-atlas-AI1", "high", "ev", "2026-10-02"),
        ("0x00401100", "Foo::Beta", "gog_cluster_delta_map.py", "medium", "ev", "2026-10-02"),
        ("0x00401200", "Foo::Gamma2", "x", "high", "ev", "2026-10-02"),
        ("0x00402100", "Other::Thing", "x", "low", "ev", "2026-10-02"),
        ("0x00403000", "NoNamespaceName", "x", "low", "ev", "2026-10-02"),
    ])
    c.executemany("insert into functions values(?,?,?,?,?,?,?)", [
        (STEAM, "00402000", "SteamFn", 100, "purpose a", "notes a", "d"),
        (STEAM, "00401000", "SteamFn0", None, "multi\nline\ttext", None, "d"),
        (OTHER, "00409999", "OtherBinaryFn", 1, "p", "n", "d"),
    ])
    c.executemany("insert into class_methods values(?,?,?,?,?,?,?,?)", [
        (STEAM, "Foo", 28, "00401400", 10, "FUN_00401400", "p", "n"),
        (STEAM, "Baz", 29, "00401500", 10, "FUN_00401500", "p", "n"),
        (STEAM, "Foo", 3, "00401600", 10, "FUN_00401600", "p", "n"),
    ])
    c.executemany("insert into candidates values(?,?,?,?,?,?,?,?,?)", [
        ("Z9", "late id first", "sub", "hyp", "evi", "win", "risk", "open", None),
        ("A1", "early id second", "sub", "hyp", "evi", "win", "risk", "closed", None),
    ])
    c.executemany("insert into lessons(id,scope,topic,note,date) values(?,?,?,?,?)", [
        (100, "s", "old", "old note", "d"), (150, "s", "t150", "n150", "d"), (200, "s", "t200", "n200", "d")])
    if with_struct_fields:
        c.execute("create table struct_fields(class text, offset text, size int, type text, name text, confidence text, evidence text, readers text, writers text, source text, date text)")
        c.executemany("insert into struct_fields(class, offset) values(?,?)", [("Foo", "0x524"), ("Foo", "0x570"), ("Bar", "0x8")])
    c.commit()
    c.close()


def run_main(mod, argv):
    """Run mod.main(argv); returns (rc, stdout, stderr)."""
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = mod.main(argv)
    return rc or 0, out.getvalue(), err.getvalue()
