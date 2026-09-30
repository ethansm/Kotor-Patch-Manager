import sqlite3,sys
DB='/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db'
def add_names(rows,src='level-up-P1-decompile-confirmed'):
    d=sqlite3.connect(DB)
    for a,n,c,e in rows:
        d.execute("insert or replace into name_map values(?,?,?,?,?,'2026-09-30')",('0x%08x'%a,n,src,c,e))
    d.commit()
def add_lesson(scope,topic,note):
    d=sqlite3.connect(DB); d.execute("insert into lessons(scope,topic,note,date) values(?,?,?,'2026-09-30')",(scope,topic,note)); d.commit()
