# RFC: Shared Engine Service Providers & Hook Multiplexing Framework (KotOR 2)

DRAFT, not sent. Posting anything upstream needs the user's explicit approval in chat.

## Problem

The installer rejects two patches that hook the same address. Mods that need the same engine event (party portraits, HUD tick, GUI input, save/load, NPC spawn) therefore cannot be installed together, and each new feature mod re-derives the same hook sites.

## Proposal

A patch with `kind = "provider"` owns the shared hooks and registers versioned interfaces (`name@major`) through the core `KPatchApi::provide`. Consumers list the provider in `requires`, call `KPatchApi::require` lazily, and stay inert on NULL. The installer derives providers from `requires` (never user-selected) and installs them first. KPM already supports this under the older name `kind = "library"`; this fork has renamed it with no alias.

## Working providers (this fork)

| Provider id | Interface |
|---|---|
| portrait-slots | `k2.portraitslots@1` |
| gui-input | `k2.guiinput@1` |
| hud-host | `k2.hud@1` |
| save-events | `k2.saveevents@1` |
| npc-world | `k2.npcworld@1` |

## Asks

1. Rename the manifest kind from `library` to `provider` upstream (this fork carries no `library` alias; only fork-authored manifests ever used it).
2. Rename the launcher section "Shared libraries" to "Service providers" and the CLI line to `auto-added provider <id> (required by ...)`.
3. Document the provider contract in `docs/PatchRegistry.md`: provides, does not act; consumers inert without it; major-version rules; ids name the shared thing.

## Compatibility

`KotorPatcher.dll` never reads `kind`, so no native change. Manifests with `kind = "library"` are rejected after the rename; only this fork's five providers used it and they are already updated.
