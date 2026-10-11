## Specialty: build-config (fixer)
- Files: manifest.toml, test.toml, smokes.toml, exports.def, include paths.
- Replace machine-specific absolute paths with the variable pattern already used in sibling entries ($KPM_DIR, $REPO, relative paths). Do not add new variables the runner does not expand (run_tests.py expands ${VAR:-default}, $VAR, ~ with REPO, KPM_DIR, KOTOR2_ROOT defined).
- Dependency or version changes in manifest.toml are policy: report not-fixable unless the finding states the exact intended value.
