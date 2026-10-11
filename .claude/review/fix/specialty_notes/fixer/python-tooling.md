## Specialty: python-tooling (fixer)
- stdlib only; keep the script's existing argparse, logging and path conventions (REPO = Path(__file__).resolve().parent.parent).
- Common fixes: unchecked subprocess returncode, bare `except Exception: pass`, unconditional destructive copy/overwrite, missing atomic write (write temp then os.replace), hard-coded home paths (use the existing env/default pattern in the file).
- Never add a write to the live game dir, the production ledger (.claude/master_ledger.jsonl) or any install path.
- Do not change CLI flags or output formats other scripts parse unless the finding is about exactly that.
