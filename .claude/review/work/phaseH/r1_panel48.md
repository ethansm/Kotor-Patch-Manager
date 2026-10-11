# r1_panel48 full findings (fallback copy; repo output file could not be overwritten)

Binary: swkotor2.exe.unpatched_vanilla (Steam path given by coordinator), sha256 6a522e71...bbffef, PE x86, ghidra 12.1.4.

## Q1. 0x0040F5A0 writes to [this+0x40..0x4F]
procedure_assembly 0x0040F5A0:
- 0x40F5AC-0x40F5B4: reads [this+0x48], tests bit 0x2 (SHR 1, AND 1, JZ). Read only.
- 0x40F5B9-0x40F5BC: CALL 0x007112B0 with [this+0x30]. Not analyzed.
- 0x40F5C4-0x40F5E2: vtable slot 0 call on [this+0x30] with push 1. Target unknown.
- 0x40F5F3: MOV [this+0x30], 0.
- 0x40F600-0x40F606: [this+0x48] = [this+0x48] AND 0xFFFFFFFD.
- 0x40F60C: CALL 0x0040FB70.
procedure_callees: 0x0040FB70, 0x007112B0.
Only write to the 0x40-0x4F range is +0x48 (bit clear). No recompute of +0x48 from children here. No float, pointer, word, or byte store to +0x48/+0x4A/+0x4B.

## Q1b. 0x0040FB70
procedure_assembly 0x0040FB70: walks array at [this+0x24] (count [this+0x28]); for each non-null child calls vtable slot 0x4C (0x40FBCA-0x40FBCD), then 0x418720 and 0x4186F0 (up to 4 iterations). No direct write to [this+0x48]. Writes via 0x4186F0 unknown.

## Q2. HUD ctor 0x00748480..0x00748530; global 0x00A1B9F0
xrefs 0x00A1B9F0: 0x40F78C, 0x40FA49, 0x40FAA6, 0x40FABE, 0x747963, 0x747AA3, 0x747B32, 0x747C33, 0x747E24, 0x74850D.
Not confirmed: instructions between 0x748502 and the CALL 0x40F5A0; access kind at 0x74850D; readers' roles. HUD ctor disassembly overflowed tool output and was not read. inspect_native_instruction failed ("No app is open").

## Q3. Flag bits
- 0x2: confirmed. Tested at 0x40F5AC-0x40F5B4; set means release child at +0x30; cleared at 0x40F600.
- 0x8, 0x200, 0x400: UNKNOWN. procedure_references incoming for 0x40F5A0 overflowed; full xref sweep not done.

## Q4. 0xFF8FB4xx origin
- 0x40F5A0's only +0x48 write is AND 0xFFFFFFFD (0x40F600). AND cannot set high bits; 0x47 becomes 0x45, not 0xFF8FB4xx. This function cannot produce the value.
- Float, sign-extended word, partial store, or struct copy elsewhere: not identified. UNKNOWN.
- Vanilla analysis cannot show KPM detours; hook-site effects at 0x40F5A0 or 0x748502 are unverified.

## Verdict
PARTIAL.
