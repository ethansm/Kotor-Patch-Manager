Domain checks for manifest, TOML, CMake, build and documentation-config claims. Apply the ones the claim touches.
- Enum-like values: is the value used (an enum-like string or number) one the consumer accepts? Compare with sibling manifests in other mod directories and with the code or schema that reads the field.
- Paths and names: do referenced files, directories, symbols and DLL names exist in the tree (Glob for them)? Are there machine-specific absolute or `$HOME` paths?
- Stale or missing entries: an entry that names a file that no longer exists, or a mod directory that lacks an entry its siblings have.
- Version fields: do version strings agree between the manifest, the source, and any changelog or doc in the same directory?
- Test configuration: does a test config reference the right test binary or source file, and is any listed test actually present?
- Compare with a sibling: when the claim says a value is wrong, find at least two sibling files of the same type and quote how they set the same field.
