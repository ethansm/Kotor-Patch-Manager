import sqlite3,sys,glob,csv
rows=[(r['class'],r['name']) for r in csv.DictReader(open('/home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/steam_db_pr3/verified.tsv'),delimiter='\t') if r['wrapper_issue']]
dbs=sorted(glob.glob('*.db'))
for cl,n in rows:
    print('==',cl,n)
    for d in dbs:
        c=sqlite3.connect('file:%s?mode=ro'%d,uri=True)
        try:
            r=c.execute("select address,calling_convention,param_size_bytes,notes from functions where class_name=? and function_name=?",(cl,n)).fetchall()
        except Exception as e:
            r=str(e)
        for x in (r if isinstance(r,list) else [r]):
            print('  ',d,hex(x[0]) if isinstance(x,tuple) else x,x[1:] if isinstance(x,tuple) else '')
