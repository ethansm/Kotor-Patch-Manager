#!/usr/bin/env python3
"""
Ghidra Server Autonomous API Client
Provides programmatic, autonomous access to OpenKotOR Ghidra Server (ghidra.openkotor.com:13100)
without exposing plaintext credentials in conversation transcripts, process arguments, or logs.
"""

import os
import sys
import json
import subprocess
import argparse
from pathlib import Path

GHIDRA_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC")
DEFAULT_SERVER = "ghidra.openkotor.com"
DEFAULT_PORT = 13100
DEFAULT_REPO = "Odyssey"
DEFAULT_USER = "Ethansm"
CRED_FILE = Path(os.path.expanduser("~/.config/ghidra/.server_credentials"))
SCRIPT_PATH = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts")
# Repo-local exporters (searched after SCRIPT_PATH), so new Export*.java scripts never touch the KPM clone.
LOCAL_SCRIPT_PATH = Path(__file__).resolve().parent / "ghidra_scripts"
DUMP_DIR = Path("/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server")

def load_credentials():
    """
    Loads server credentials securely from environment, protected vault file, or desktop keyring.
    Never prints or echoes the password.
    """
    creds = {
        "server": os.environ.get("GHIDRA_SERVER", DEFAULT_SERVER),
        "port": int(os.environ.get("GHIDRA_PORT", DEFAULT_PORT)),
        "repo": os.environ.get("GHIDRA_REPO", DEFAULT_REPO),
        "user": os.environ.get("GHIDRA_USER", DEFAULT_USER),
        "password": os.environ.get("GHIDRA_PASS")
    }

    if not creds["password"] and CRED_FILE.exists():
        try:
            with open(CRED_FILE, "r") as f:
                for line in f:
                    line = line.strip()
                    if line.startswith("export "):
                        line = line[7:]
                    if "=" in line and not line.startswith("#"):
                        k, v = line.split("=", 1)
                        v = v.strip("\"' \t")
                        if k == "GHIDRA_PASS": creds["password"] = v
                        elif k == "GHIDRA_USER": creds["user"] = v
                        elif k == "GHIDRA_SERVER": creds["server"] = v
                        elif k == "GHIDRA_PORT": creds["port"] = int(v)
                        elif k == "GHIDRA_REPO": creds["repo"] = v
        except Exception:
            pass

    if not creds["password"]:
        # Try secret-tool lookup
        try:
            res = subprocess.run(
                ["secret-tool", "lookup", "server", creds["server"], "user", creds["user"]],
                capture_output=True, text=True, timeout=3
            )
            if res.returncode == 0 and res.stdout.strip():
                creds["password"] = res.stdout.strip()
        except Exception:
            pass

    return creds

def list_remote_tree(folder="/"):
    """
    Fast remote repository directory listing via jshell.
    Returns subfolders and items without running a full headless analysis.
    """
    creds = load_credentials()
    if not creds["password"]:
        return {"error": "Missing credentials. Run scripts/setup_ghidra_vault.sh first."}

    template = """
import ghidra.framework.client.*;
import ghidra.framework.remote.*;
import java.util.*;

try {
    ClientUtil.setClientAuthenticator(new PasswordClientAuthenticator("__USER__", System.getenv("GHIDRA_PASS")));
    RepositoryServerAdapter rsa = ClientUtil.getRepositoryServer("__SERVER__", __PORT__, true);
    rsa.connect();
    RepositoryAdapter ra = rsa.getRepository("__REPO__");
    ra.connect();

    String targetFolder = "__FOLDER__";
    String[] subfolders = ra.getSubfolderList(targetFolder);
    RepositoryItem[] items = ra.getItemList(targetFolder);

    System.out.println("JSON_START");
    StringBuilder sb = new StringBuilder();
    sb.append("{\\"subfolders\\":[");
    for (int i = 0; i < subfolders.length; i++) {
        sb.append("\\"").append(subfolders[i]).append("\\"");
        if (i < subfolders.length - 1) sb.append(",");
    }
    sb.append("],\\"items\\":[");
    for (int i = 0; i < items.length; i++) {
        RepositoryItem it = items[i];
        sb.append("{\\"name\\":\\"").append(it.getName())
          .append("\\",\\"version\\":").append(it.getVersion())
          .append(",\\"type\\":\\"").append(it.getContentType()).append("\\"}");
        if (i < items.length - 1) sb.append(",");
    }
    sb.append("]}");
    System.out.println(sb.toString());
    System.out.println("JSON_END");
} catch (Exception e) {
    System.err.println("API_ERROR: " + e.getMessage());
    System.exit(1);
}
/exit
"""
    target_folder = folder if folder.startswith("/") else "/" + folder
    jshell_code = (template
        .replace("__USER__", creds["user"])
        .replace("__SERVER__", creds["server"])
        .replace("__PORT__", str(creds["port"]))
        .replace("__REPO__", creds["repo"])
        .replace("__FOLDER__", target_folder))
    env = os.environ.copy()
    env["GHIDRA_PASS"] = creds["password"]
    
    jshell_runner = GHIDRA_DIR / "support" / "jshellRun"
    p = subprocess.Popen([str(jshell_runner)], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env, text=True)
    out, err = p.communicate(input=jshell_code, timeout=60)
    
    if "JSON_START" in out and "JSON_END" in out:
        json_str = out.split("JSON_START")[1].split("JSON_END")[0].strip()
        try:
            return json.loads(json_str)
        except Exception as e:
            return {"raw_output": json_str, "parse_error": str(e)}
    return {"error": "Failed to list remote folder", "stderr": err}

def export_program_symbols(program_name, folder="TSL", output_dir=None, script="ExportProgramSymbols.java"):
    """
    Runs a read-only export script (-readOnly -noanalysis) headlessly against a server program
    without exposing the password. The script gets the output dir as its only argument.
    """
    creds = load_credentials()
    if not creds["password"]:
        return {"error": "Missing credentials. Run scripts/setup_ghidra_vault.sh first."}

    out_path = Path(output_dir) if output_dir else DUMP_DIR
    out_path.mkdir(parents=True, exist_ok=True)

    headless_bin = GHIDRA_DIR / "support" / "analyzeHeadless"
    repo_url = f"ghidra://{creds['server']}:{creds['port']}/{creds['repo']}/{folder.strip('/')}"

    cmd = [
        str(headless_bin),
        repo_url,
        "-process", program_name,
        "-readOnly", "-noanalysis",
        "-connect", creds["user"], "-p",
        "-scriptPath", f"{SCRIPT_PATH};{LOCAL_SCRIPT_PATH}",
        "-postScript", script, str(out_path)
    ]

    # Pipe password via stdin
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout, stderr = p.communicate(input=creds["password"] + "\n", timeout=600)

    # Filter out password prompts from logs before returning
    clean_stdout = "\n".join(l for l in stdout.splitlines() if "Password for" not in l and "***" not in l)

    return {
        "returncode": p.returncode,
        "program": program_name,
        "output_dir": str(out_path),
        "stdout_summary": clean_stdout[-1500:] if len(clean_stdout) > 1500 else clean_stdout
    }

def main(argv=None):
    parser = argparse.ArgumentParser(description="Ghidra Server Autonomous API")
    subparsers = parser.add_subparsers(dest="command")

    # list-tree
    p_list = subparsers.add_parser("list-tree", help="List remote folders and files")
    p_list.add_argument("folder", nargs="?", default="/", help="Folder path (e.g., '/', '/TSL')")

    # export
    p_export = subparsers.add_parser("export", help="Export program symbols headlessly")
    p_export.add_argument("program", help="Program filename (e.g. 'swkotor2_aspyr.exe')")
    p_export.add_argument("--folder", default="TSL", help="Folder path (default: 'TSL')")
    p_export.add_argument("--out", default=str(DUMP_DIR), help="Output directory")

    # export-script: any read-only export script from SCRIPT_PATH (e.g. ExportFingerprints.java, ExportDataTypes.java)
    p_xs = subparsers.add_parser("export-script", help="Run a read-only export script on a server program")
    p_xs.add_argument("program", help="Program filename (e.g. 'k2_win_gog_aspyr_swkotor2.exe')")
    p_xs.add_argument("script", help="Script in the KPM ghidra_scripts folder (must start with 'Export')")
    p_xs.add_argument("--folder", default="TSL", help="Folder path (default: 'TSL')")
    p_xs.add_argument("--out", required=True, help="Output directory")

    # export-types: shorthand for export-script PROGRAM ExportDataTypes.java
    p_xt = subparsers.add_parser("export-types", help="Export data types (structs/unions/enums) of a server program")
    p_xt.add_argument("program")
    p_xt.add_argument("--folder", default="TSL")
    p_xt.add_argument("--out", required=True)

    # audit
    p_audit = subparsers.add_parser("audit", help="Run local vs server comparison audit")

    # status
    p_status = subparsers.add_parser("status", help="Check credential vault configuration")

    args = parser.parse_args(argv)

    if args.command == "status":
        creds = load_credentials()
        has_pass = bool(creds["password"])
        print(json.dumps({
            "vault_file": str(CRED_FILE),
            "vault_exists": CRED_FILE.exists(),
            "server": creds["server"],
            "port": creds["port"],
            "repo": creds["repo"],
            "user": creds["user"],
            "authenticated_ready": has_pass
        }, indent=2))
        return 0

    if args.command == "list-tree":
        res = list_remote_tree(args.folder)
        print(json.dumps(res, indent=2))
        return 0

    if args.command == "export":
        res = export_program_symbols(args.program, folder=args.folder, output_dir=args.out)
        print(json.dumps(res, indent=2))
        return 0

    if args.command in ("export-script", "export-types"):
        script = "ExportDataTypes.java" if args.command == "export-types" else args.script
        # Only read-only exporters: the run is -readOnly anyway, this keeps arbitrary scripts out.
        if not (script.startswith("Export") and script.endswith(".java") and "/" not in script):
            print(json.dumps({"error": f"refusing script {script!r}: must be an Export*.java in {SCRIPT_PATH} or {LOCAL_SCRIPT_PATH}"}))
            return 2
        res = export_program_symbols(args.program, folder=args.folder, output_dir=args.out, script=script)
        print(json.dumps(res, indent=2))
        return 0 if res.get("returncode") == 0 else 1

    if args.command == "audit":
        import audit_server_knowledge
        return audit_server_knowledge.main()

    parser.print_help()
    return 0

if __name__ == "__main__":
    sys.exit(main())
