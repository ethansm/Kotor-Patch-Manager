import json,subprocess,sys,os,concurrent.futures as cf,pathlib,time
R=pathlib.Path("/home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/review")
OUT=pathlib.Path(sys.argv[1]); OUT.mkdir(exist_ok=True)
TREE=pathlib.Path.home()/".cache/kotor2_review/worktree"
CAP=float(sys.argv[2]); PER=0.12
batches=sys.argv[3].split(","); WAVE=4
variant=json.load(open(R/"canary.json")).get("variant","A")
iso=["--restricted","--safe-mode"] if variant=="A" else ["--restricted"]
schema=(R/"verify/adjudicate_schema.json").read_text(); sysp=(R/"verify/adjudicate_prompt.md").read_text()
def run(b):
    prompt=(R/f"work/adjudicate/{b}.md").read_text()
    cmd=["claude","-p","--model","claude-haiku-5-5",*iso,"--settings",str(R/"worker_settings.json"),"--tools","Read,Grep,Glob",
      "--permission-mode","dontAsk","--max-budget-usd",str(PER),"--json-schema",schema,"--strict-mcp-config","--mcp-config",'{"mcpServers":{}}',
      "--disable-slash-commands","--no-session-persistence","--output-format","stream-json","--verbose","--append-system-prompt",sysp]
    t=time.time(); p=subprocess.run(cmd,input=prompt,capture_output=True,text=True,cwd=TREE,timeout=900)
    (OUT/f"{b}.out").write_text(p.stdout); (OUT/f"{b}.err").write_text(p.stderr)
    res=None
    for ln in p.stdout.splitlines():
        try: e=json.loads(ln)
        except Exception: continue
        if e.get("type")=="result": res=e
    cost=(res or {}).get("total_cost_usd",0.0)
    so=(res or {}).get("structured_output")
    json.dump({"batch":b,"cost":cost,"rc":p.returncode,"sec":round(time.time()-t),"output":so,"subtype":(res or {}).get("subtype")},open(OUT/f"{b}.json","w"))
    return b,cost,p.returncode,so is not None
spent=0.0
for i in range(0,len(batches),WAVE):
    if spent>=CAP: print("cap reached, stopping at",spent); break
    w=batches[i:i+WAVE]
    with cf.ThreadPoolExecutor(WAVE) as ex:
        for b,c,rc,ok in ex.map(run,w):
            spent+=c; print(b,f"cost=${c:.4f}",f"rc={rc}","ok" if ok else "NO-OUTPUT",flush=True)
    print(f"spent so far ${spent:.4f}",flush=True)
print("total",round(spent,4))
