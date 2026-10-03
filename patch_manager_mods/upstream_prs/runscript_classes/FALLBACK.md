# runscript / empty `classes` table: decision and documented fallback

## Decision (user, 2026-10-03)
Issue only: report the bug and let Lane choose between DB rows and a code fallback, because it touches the GOG DB and
the 840f515 design. The issue draft is `../drafts/issue_gog_db_missing_names.md`. It is NOT filed yet; posting needs
explicit approval. This file documents the code fallback so it can be implemented later if Lane prefers it.

## Bug (verified on disk 10-03)
- `classes` has 0 rows in every K2 DB (GOG, upstream Steam, branch Steam 4324098). K1 1.03 has 977.
- `CExoString::InitializeOffsets()` (CExoString.cpp ~58-67) calls `GameVersion::GetClassSize("CExoString")` last in
  its `try`. With no row it throws: `classSize` stays -1 and `offsetsInitialized` stays false, so every later
  construction retries the failing lookup.
- The three allocating constructors do `objectPtr = (classSize > 0) ? malloc(classSize) : nullptr;`, so the object
  is null and the engine constructor is skipped.
- AdditionalConsoleCommands `runscript` (`CExoString scriptFile(script)`, then `vm->RunScript(&scriptFile, ...)`)
  passes a null string. Steam's RunScript guards against null, so nothing runs and nothing crashes.
- Real size = 8 on K1 (DB row) and K2. The Steam ctor 0x733540 writes two dwords; 0x733570 stores the pointer and
  length+1. The layout is {char* CStr @0, uint Length @4}.

## Fallback implementation (for later)
Smallest change, `Patches/Common/GameAPI/CExoString.cpp`, in `InitializeOffsets()`. Resolve the size outside the
offsets `try` so that a missing `classes` row neither aborts the offsets nor leaves the class uninitialised:

```cpp
    try {
        offsetCStr = GameVersion::GetOffset("CExoString", "CStr");
        offsetLength = GameVersion::GetOffset("CExoString", "Length");
        offsetsInitialized = true;
    }
    catch (const GameVersionException& e) {
        debugLog("[CExoString] ERROR: %s\n", e.what());
    }

    // CExoString is { char* CStr; uint Length; } on every supported build. Fall back to that size when
    // the address DB has no classes row (the K2 databases ship with an empty classes table).
    try {
        classSize = GameVersion::GetClassSize("CExoString");
    }
    catch (const GameVersionException&) {
        classSize = 8;
    }
```

Notes for the implementer:
- Check that the K2 DBs carry the `CExoString` `CStr`/`Length` offsets. If they don't, the first `try` fails as
  well and needs the same fallback (0 and 4).
- Other wrappers that switched to `GetClassSize` in 840f515 (CSWGuiBorder, CSWGuiButton, CSWGuiButtonToggle,
  CSWGuiControl, CSWGuiEditBox, ...) have the same K2 failure. A generic option is
  `GameVersion::GetClassSizeOr(name, fallback)`. That is a larger, design-level change, which is why it's Lane's call.
- Alternative DB fix: `classes` rows `CExoString` 8 and `ConsoleFunc` 84 (as in K1) in both K2 DBs. The Steam half
  could ride the Steam DB PR; the GOG half is an upstream DB edit.
- Test: on Steam under Proton, `runscript <a known script>` should then run (use a script with a visible effect).
