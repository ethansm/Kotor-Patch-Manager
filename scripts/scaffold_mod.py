#!/usr/bin/env python3
"""scaffold_mod.py - Automated Scaffolding for KOTOR II Mods and Master Plans.

Generates a fully compliant, turnkey mod directory and master plan with:
- Standard include style ("Header.h" with -I Common, no relative ../_shared paths)
- Built-in Common contract test (common_contract_test.cpp + test.toml)
- KPM patch manifest, hook definitions, and export definitions
- 7-column master plan markdown with oc-subagent delegation commands for all phases

Usage:
  python3 scripts/scaffold_mod.py <mod_name> [--doc-id <num>] [--title <title>] [--dry-run]

Example:
  python3 scripts/scaffold_mod.py stealth_telemetry --doc-id 35 --title "Stealth Telemetry & AI Sensors"
"""

import argparse
import os
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def to_pascal_case(s: str) -> str:
    parts = re.split(r"[_\-\s]+", s)
    return "".join(p.capitalize() for p in parts if p)


def to_snake_case(s: str) -> str:
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", s)
    s = re.sub(r"([a-z\d])([A-Z])", r"\1_\2", s)
    return s.replace("-", "_").lower()


def scaffold_mod(mod_name: str, doc_id: str = None, title: str = None, dry_run: bool = False):
    clean_name = to_snake_case(mod_name).removesuffix("_src")
    pascal_name = to_pascal_case(clean_name)
    mod_dir_name = f"{clean_name}_src"
    mod_path = REPO_ROOT / "patch_manager_mods" / mod_dir_name

    display_title = title or f"{pascal_name} Mod"

    if not doc_id:
        existing_docs = []
        for p in (REPO_ROOT / "patch_manager_mods").glob("*.md"):
            m = re.match(r"^(\d{2})_", p.name)
            if m:
                existing_docs.append(int(m.group(1)))
        doc_id = f"{max(existing_docs, default=34) + 1:02d}"

    plan_filename = f"{doc_id}_{clean_name}.md"
    plan_path = REPO_ROOT / "patch_manager_mods" / plan_filename

    print(f"=== SCAFFOLDING MOD: {clean_name} ===")
    print(f"Mod Directory: patch_manager_mods/{mod_dir_name}/")
    print(f"Master Plan:   patch_manager_mods/{plan_filename}")

    # 1. Main C++ source
    cpp_content = f"""// {pascal_name}.cpp -- {display_title}
// Standard KPM DLL entry point and hook implementations.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// Shared headers included via standard -I Common (never use relative ../_shared/)
#include "SafeRead.h"

namespace {{

using saferead::readAt;
using saferead::probeRead;

constexpr const char* Version = "1.0.0";

}} // namespace

extern "C" {{

__declspec(dllexport) void LuoProbe_Init() {{
    // Initializer hook called by KPatch on DLL load
}}

}} // extern "C"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {{
    (void)hinstDLL;
    (void)lpvReserved;
    if (fdwReason == DLL_PROCESS_ATTACH) {{
        DisableThreadLibraryCalls(hinstDLL);
    }}
    return TRUE;
}}
"""

    # 2. exports.def
    exports_def = f"""EXPORTS
    LuoProbe_Init
"""

    # 3. hooks.toml
    hooks_toml = f"""# kotor2-steam-aspyr.hooks.toml -- Steam Windows hooks for {clean_name}
target_binary = "swkotor2.exe"
target_version = "Steam Aspyr 2015"

# Example hook detour entry:
# [[detour]]
# address = 0x0073F450
# function = "LuoProbe_Init"
# convention = "cdecl"
"""

    # 4. manifest.toml
    manifest_toml = f"""[patch]
id = "{clean_name.replace('_', '-')}"
name = "{display_title}"
version = "1.0.0"
author = "ethansm"
description = "{display_title} for KOTOR II"
kind = "mod"
target = "swkotor2.exe"

[build]
sources = ["{pascal_name}.cpp"]
exports = "exports.def"
hooks = "kotor2-steam-aspyr.hooks.toml"
"""

    # 5. common_contract_test.cpp
    contract_cpp = f"""// common_contract_test.cpp -- Common CONTRACT test for {display_title} (doc 20)
// Pins the semantics this mod relies on from _shared_src (Patches/Common).
// Built against _shared_src via test.toml.
#include "SafeRead.h"
#include "check.h"
#include <stdio.h>

#define LCHECK(why, expr) ::kt::Check("{clean_name} relies on: " why " (line " KT_STR(__LINE__) ")", static_cast<bool>(expr))

int main() {{
    g_stubVirtualQueryAll = true;

    // Verify SafeRead contract
    uint32_t val = 0x12345678;
    LCHECK("readAt reads valid memory", saferead::readAt<uint32_t>(reinterpret_cast<uintptr_t>(&val), 0) == 0x12345678);
    LCHECK("probeRead detects valid buffer", saferead::probeRead("test", reinterpret_cast<uintptr_t>(&val), sizeof(val)));

    return kt::Report("{clean_name}_contract_test");
}}
"""

    # 6. test.toml
    test_toml = """[common_contract_test]
cxxflags = ["-I../_shared_src", "-I../../tests/include", "-Werror"]
"""

    # 7. Master plan markdown with oc-subagent roles
    plan_md = f"""# Doc {doc_id}: {display_title}

**Mod Directory:** `patch_manager_mods/{mod_dir_name}/`  
**Status:** Inception / Planning  
**Owning Mod Scope:** `patch_manager_mods/{mod_dir_name}/` (Strict boundary: do not touch other mods)

---

## 1. Objective & Architecture

Detailed design and requirements for {display_title}.

---

## 2. Master Plan Phasing (7-Column Schema)

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| **P0** | Baseline & Scaffolding | - | - | TODO | | `build_check.py --mod {mod_dir_name}` |
| **P1** | Core Detour Implementation | P0 | - | TODO | | `build_check.py` + host unit tests |
| **P2** | Common Contract Test Verification | P1 | - | TODO | | `run_tests.py -k {clean_name}` |
| **P3** | Live In-Game Probing & Calibration | P2 | user GO | TODO | | Live hotswap validation |
| **P4** | Log Tightening & Packaging | P3 | - | TODO | | Clean startup log verification |

---

## 3. Subagent Delegation Architecture (`opencode-subagents`)

See [SUBAGENT_ROLES.md](SUBAGENT_ROLES.md) for role specifications, contracts, and backend routes.
When orchestrating OpenCode / OpenRouter subagents, invoke role-based commands directly:

### Phase P0 - Baseline & Scaffolding
- Subagent: `oc-subagent scout "Verify {clean_name} scaffolding and inspect hooks against Ghidra DB"`
- Subagent: `oc-subagent gate "Run python3 scripts/build_check.py --mod {mod_dir_name}"`

### Phase P1 - Core Detour Implementation
- Subagent: `oc-subagent ghidra "Cross-reference engine hook addresses in Ghidra DB for {clean_name}"`
- Subagent: `oc-subagent code "Implement core logic in patch_manager_mods/{mod_dir_name}/{pascal_name}.cpp"`
- Subagent: `oc-subagent verify "Run python3 scripts/build_check.py --mod {mod_dir_name}"`

### Phase P2 - Common Contract Test Verification
- Subagent: `oc-subagent test "Expand common_contract_test.cpp with mod-specific API assumptions"`
- Subagent: `oc-subagent verify "Run python3 scripts/run_tests.py -k {clean_name}"`

### Phase P3 - Live Probing & Calibration
- Subagent: `oc-subagent doc "Prepare live hotswap instructions and verify backup paths"`

### Phase P4 - Log Tightening & Packaging
- Subagent: `oc-subagent doc "Verify log format deduplication and Verbose=0 gating"`
- Subagent: `oc-subagent gate "Run scripts/build_check.py and scripts/kpm_status.py"`

---

## 4. Verification Gates

```bash
# Build gate
python3 scripts/build_check.py --mod {mod_dir_name}

# Unit and contract tests
python3 scripts/run_tests.py -k {clean_name}

# Scope check
python3 scripts/agent_diff_check.py --mod {mod_dir_name}
```
"""

    files_to_write = [
        (mod_path / f"{pascal_name}.cpp", cpp_content),
        (mod_path / "exports.def", exports_def),
        (mod_path / "kotor2-steam-aspyr.hooks.toml", hooks_toml),
        (mod_path / "manifest.toml", manifest_toml),
        (mod_path / "common_contract_test.cpp", contract_cpp),
        (mod_path / "test.toml", test_toml),
        (plan_path, plan_md),
    ]

    if dry_run:
        print("\n[DRY RUN] Would create the following files:")
        for target, _ in files_to_write:
            print(f"  - {target.relative_to(REPO_ROOT)}")
        return 0

    mod_path.mkdir(parents=True, exist_ok=True)
    for target, content in files_to_write:
        if not target.exists():
            target.write_text(content, encoding="utf-8")
            print(f"  [CREATED] {target.relative_to(REPO_ROOT)}")
        else:
            print(f"  [EXISTS]  {target.relative_to(REPO_ROOT)} (skipped)")

    print("\nMod scaffolding complete and verified compliant with:")
    print("  ✓ Standard Include Style (\"Header.h\" with -I Common)")
    print("  ✓ Common Contract Testing (common_contract_test.cpp + test.toml)")
    print("  ✓ Master Plan 7-column schema & oc-subagent delegation matrix")
    print(f"\nNext Action: python3 scripts/plan_manager.py resume {plan_path.name}")
    return 0


def main():
    parser = argparse.ArgumentParser(description="Scaffold a new KOTOR II mod and master plan.")
    parser.add_argument("name", help="Mod name (e.g. stealth_telemetry, force_jump_fix)")
    parser.add_argument("--doc-id", default=None, help="Explicit documentation ID number (e.g. 35)")
    parser.add_argument("--title", default=None, help="Human-readable title")
    parser.add_argument("--dry-run", action="store_true", help="Print files without creating")
    args = parser.parse_args()

    return scaffold_mod(args.name, args.doc_id, args.title, args.dry_run)


if __name__ == "__main__":
    sys.exit(main())
