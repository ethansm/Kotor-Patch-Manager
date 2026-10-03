import sys, pathlib
sys.path.insert(0,'/home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts')
import ghidra_server_api as g
g.SCRIPT_PATH = pathlib.Path('/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/64189a71-021c-4f66-8a9d-eebc5aa38044/scratchpad/gscripts')
prog, folder, out = sys.argv[1:4]
r = g.export_program_symbols(prog, folder=folder, output_dir=out, script="ExportDisasmList.java")
print(r.get('returncode'), r.get('stdout_summary','')[-800:])
