---
name: openkotor-ghidra-server
description: >-
  Query and extract reverse-engineering intelligence autonomously from the official OpenKotOR shared Ghidra server (ghidra.openkotor.com:13100, repository Odyssey) covering TSL (KotOR 2), K1, and other BioWare engines across Steam, GOG, Linux, Mac, Xbox, and CD binaries.
  Use when asked to check the Ghidra server, list or browse server programs, export upstream symbols/types,
  run an export script remotely, compare server labels with our address DB (audit), or when a local
  query_ghidra.py lookup misses and the upstream GOG/Linux/Mac names might help.
---

# OpenKotOR Ghidra Server Autonomous Skill

Use this skill to interact with the upstream OpenKotOR Ghidra Server (`ghidra.openkotor.com:13100`, repository `Odyssey`). It provides safe, credential-isolated, headless access to cross-platform binary databases and reverse engineering knowledge.

## Credential Isolation Policy (Strict Rule)
- The server credentials are held in a mode-0600 vault: `~/.config/ghidra/.server_credentials`.
- **NEVER** run `cat`, `view_file`, `head`, `tail`, `grep`, or `echo` on `~/.config/ghidra/.server_credentials` or any file matching `*credential*` or `*secret*`.
- **ALWAYS** interact with the Ghidra Server via `python3 scripts/ghidra_server_api.py` or `scripts/export_server_knowledge.sh`, which consume credentials in memory without echoing to logs or transcripts.

---

## 0. Instant Local Knowledge Lookup (Sub-Millisecond, Zero Network)
Before querying the remote server, query the local synchronized knowledge base using `query_ghidra.py`:
```bash
python3 scripts/query_ghidra.py symbol <name> [--json]
python3 scripts/query_ghidra.py address <0xADDR>
python3 scripts/query_ghidra.py struct <class_name>
```
- Backed by local SQLite `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` (server functions, symbols, struct fields).
- Returns instantly without consuming network bandwidth or token overhead.
- Steam exe facts per address ($0, local dumps): `python3 patch_manager_mods/ghidra_knowledge/tools/static_facts.py addr <hex>`; named pseudo-C `named_decomp.py`, class pages `fn_doc.py`, KDB purposes `persist_purposes.py`, ledger `rea_ledger.py lint` (order and flags: `ghidra_knowledge/RE_AGENT_GUIDE.md` §2.1). Use the server only when these miss.

---

## 1. Quick Verification & Status
Verify that the local credential vault is initialized and ready for autonomous queries:
```bash
python3 scripts/ghidra_server_api.py status
```
Expected output:
```json
{
  "vault_file": "/home/ethansm/.config/ghidra/.server_credentials",
  "vault_exists": true,
  "server": "ghidra.openkotor.com",
  "port": 13100,
  "repo": "Odyssey",
  "user": "Ethansm",
  "authenticated_ready": true
}
```

---

## 2. Browsing Remote Server Repositories
List root repository directories (`/JE`, `/K1`, `/TSL`, `/Libraries`, etc.):
```bash
python3 scripts/ghidra_server_api.py list-tree /
```

List binaries and versions inside `/TSL` (Knights of the Old Republic II):
```bash
python3 scripts/ghidra_server_api.py list-tree /TSL
```
Key binaries available on the server:
- `k2_win_steam_aspyr_swkotor2.exe` (Steam Windows Aspyr executable)
- `k2_win_gog_aspyr_swkotor2.exe` (GOG Windows Aspyr executable, most names mapped)
- `k2_linux_swkotor2.elf` (Linux ELF - useful for DWARF types and assertion strings)
- `k2_mac_KOTOR2_x64` / `k2_mac_KOTOR2_x86` (Mac Mach-O - demangled symbol tables)
- `k2_win_CD_1.0_swkotor2.exe` / `k2_win_CD_1.0b_swkotor2.exe` (Retail 2004 4-CD release)
- `k2_xbox_default.xbe` (Original Xbox release)

List binaries in `/K1` (Knights of the Old Republic I):
```bash
python3 scripts/ghidra_server_api.py list-tree /K1
```

---

## 3. Headless Export & Symbol Extraction
Export all functions, namespaces, prototypes, calling conventions, and labels from any remote program to TSV:
```bash
python3 scripts/ghidra_server_api.py export <program_name> [--folder /TSL] [--out <output_dir>]
```

Examples:
- Export GOG symbols:
  ```bash
  python3 scripts/ghidra_server_api.py export k2_win_gog_aspyr_swkotor2.exe --folder /TSL
  ```
- Export Steam symbols:
  ```bash
  python3 scripts/ghidra_server_api.py export k2_win_steam_aspyr_swkotor2.exe --folder /TSL
  ```

Other headless exports (both need `--out`):
```bash
python3 scripts/ghidra_server_api.py export-types <program> --folder /TSL --out <output_dir>
python3 scripts/ghidra_server_api.py export-script <program> <ExportScript.java> --folder /TSL --out <output_dir>
```
`export-script` only runs scripts from the KPM `ghidra_scripts` folder whose names start with `Export`.

Output files land in `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/upstream_server/`:
- `<program>_functions.tsv` (addr, size, retn, name, namespace, callconv, prototype, source, thunk)
- `<program>_symbols.tsv` (addr, name, namespace, symbol_type, source)

---

## 4. Cross-Referencing & Audit
Compare exported server knowledge against our local SQLite databases (`kotor2_steam_aspyr.db` and `kotor2_ghidra_knowledge.db`):
```bash
python3 scripts/ghidra_server_api.py audit
```
This produces:
- Exact and normalized match percentages across validated functions.
- Discrepancy / naming conflict reports.
- Catalog of functions unnamed on the server that are verified in our database.
- Subsystems available in upstream GOG/Linux ready for translation unit delta mapping.

---

## 5. Trust Model & Validation Standards
1. **Never trust server labels as ground truth:**
   - Steam binary on the server contains thousands of automated Function ID (FID) template hashes (`CPaneContainerManager`, `SchedulerProxy`, `ios_base`).
   - Use server entries strictly as **hypotheses and hints**.
2. **Admission rules and the SqliteTools integrity gate** for `kotor2_steam_aspyr.db` are in CLAUDE.md
   ("Address Database & R&D Development Standards"). A server label alone never admits a row.
