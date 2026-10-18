# KotorPatcher Runtime Module

KotorPatcher is the runtime module that loads into the KotOR game process to apply patches dynamically. It reads patch configurations, loads patch binaries, and modifies game code in-memory using various hooking techniques.

One source tree builds both targets: `KotorPatcher.dll` for the Windows game (whether run natively or under Wine/Proton), and `KotorPatcher.so` for KOTOR II's native Linux build. The engine under `src/core` is shared, and everything OS-specific sits behind the platform seam in `include/platform.h`. See `docs/NATIVE_LINUX.md` for how the `.so` gets loaded.

## Architecture Overview

The patcher operates in three phases:

1. **Initialization** (DLL_PROCESS_ATTACH on Windows, library constructor on Linux): Parse `patch_config.toml`, load patch binaries, apply hooks
2. **Runtime**: Patches execute as the game runs, intercepting and modifying game behavior
3. **Cleanup** (DLL_PROCESS_DETACH on Windows, library destructor on Linux): Unload patch binaries and free allocated memory

## Project Structure

### Core Components

Headers live under `include/`, the shared engine under `src/core/`, and the entry points and platform backends directly under `src/`.

**src/dllmain.cpp**: Windows DLL entry point that handles initialization and cleanup lifecycle events.

**src/soentry.cpp**: Linux entry point. Constructor and destructor attributes stand in for DLL_PROCESS_ATTACH/DETACH, handing off to the same shared engine.

**platform.h / src/platform_win32.cpp / src/platform_posix.cpp**: The platform seam. Declares the OS operations the engine needs (memory allocation and protection, module loading, symbol lookup, logging) with one backend per OS, so `src/core` contains no `#ifdef`s.

**patcher.h / src/core/patcher.cpp**: Core patching engine containing hook application logic, patch binary loading, and configuration management. Defines the `PatchInfo` structure that represents a single hook configuration.

**config_reader.h / src/core/config_reader.cpp**: TOML parser that reads `patch_config.toml` and converts it into `PatchInfo` structures. Uses the tomlplusplus library for parsing.

**trampoline.h / src/core/trampoline.cpp**: Low-level memory patching utilities for writing JMP/CALL instructions, verifying bytes, managing memory protection, and writing NOP instructions.

**kpatch_api.h**: The plain-C ABI between the patcher and the patch modules: `KPatchApi` and the optional `KPatchInit` export. The canonical copy is `Patches/Common/kpatch_api.h`, which the patches include; `include/kpatch_api.h` is a one-line forwarder to it. See [Patch registry and KPatchInit](#patch-registry-and-kpatchinit-rfc-180).

**registry.h / src/core/registry.cpp**: The patch registry. Holds the named, versioned interfaces that the core and the patch modules provide to each other, and runs each module's `KPatchInit` once.

**tests/**: Host tests for the engine and the registry, with plain-C stand-in patch modules under `tests/fixtures/`. See `tests/README.md`.

### Wrapper System

The wrapper system generates runtime x86 assembly code to intercept game functions, preserve CPU state, extract parameters, and call patch functions.

**wrappers/wrapper_base.h**: Abstract base class defining the wrapper generator interface. Provides platform-agnostic API for creating hook wrappers.

**wrappers/wrapper_x86.h / src/core/wrapper_x86.cpp**: x86 32-bit implementation of the wrapper generator. Emits machine code at runtime to save/restore registers, extract parameters from CPU state, and manage the transition between game code and patch code. The emitted code is the same on both targets since every supported game build is 32-bit x86; only the memory allocation behind it differs, which the platform seam handles.

**wrapper_context.h**: Defines `PatchContext` structure containing saved CPU state (registers, flags, stack pointer). Provides helper methods for accessing parameters and modifying return values.

## Hook Types

KotorPatcher supports four runtime hook types that are applied when the game launches. A fifth hook type (STATIC) is applied at install-time by KPatchCore and does not require runtime processing.

### DETOUR Hooks

Full-featured hooks with automatic state management. Requires a patch DLL with an exported function.

- Writes 5-byte JMP instruction at hook address
- Generates wrapper stub that saves all CPU state (PUSHAD/PUSHFD)
- Extracts parameters from registers or stack based on configuration
- Calls patch function with extracted parameters (cdecl convention)
- Restores CPU state (with optional register exclusions)
- Re-executes original bytes (stolen bytes) or skips them based on configuration
- Jumps back to continue game execution

**Use when**: Complex logic needed, calling game functions, accessing game state, parameter extraction required.

### SIMPLE Hooks

Direct byte replacement in memory. No DLL loading or wrapper generation.

- Verifies original bytes match expected values
- Overwrites with replacement bytes of equal length
- No state preservation or function calls

**Use when**: Changing constants, NOPing instructions, simple instruction replacements.

### REPLACE Hooks

Allocates executable memory for custom assembly code that executes in place of original instructions.

- Verifies original bytes (minimum 5 bytes)
- Allocates executable memory for replacement code
- Writes replacement bytes followed by JMP back to game
- Writes JMP at hook address to allocated memory
- NOPs remaining bytes

**Use when**: Need more complex logic than SIMPLE hooks but don't require DLL infrastructure.

### DLL_ONLY

Loads a patch DLL without applying any hooks. Used for patches that hook via their DllMain.

### STATIC (Not Handled by KotorPatcher)

STATIC hooks are applied at install-time by KPatchCore, not at runtime. They modify the executable file directly before the game launches. These hooks are excluded from `patch_config.toml` and never reach the runtime patcher. Use STATIC hooks for PE header modifications (e.g., 4GB patch) or other changes that must occur before the executable loads into memory.

## Key Classes and Structures

### PatchInfo

Represents a single hook configuration. Contains:

- **Hook location**: `hookAddress` specifies where to patch
- **Hook type**: `type` determines patching strategy (DETOUR/SIMPLE/REPLACE/DLL_ONLY)
- **Patch DLL**: `dllPath` and `functionName` for DETOUR hooks
- **Byte arrays**: `originalBytes` (for verification and execution), `replacementBytes` (for SIMPLE/REPLACE)
- **State management**: `preserveRegisters`, `preserveFlags`, `excludeFromRestore`
- **Parameters**: `parameters` array defining how to extract values from CPU state
- **Behavior flags**: `skipOriginalBytes` determines whether to re-execute stolen bytes

### ParameterInfo

Defines how to extract a parameter for a DETOUR hook function:

- **source**: Register name, a register with an offset, or constant, from the table below
- **type**: Data type, from the table under *What `type` decides*

Parameters are read out of the saved CPU state, then placed where the calling convention wants them: pushed in reverse order on x86 (cdecl), loaded into the argument registers on x86_64 (System V).

The two generators read different sources, and a hook naming one its generator cannot read does not install. `Parameter.IsValidFor` in KPatchCore holds the same table and refuses the install before anything is written to the game.

| Source | x86 | x86_64 | Notes |
| --- | --- | --- | --- |
| `eax` `ebx` `ecx` `edx` `esi` `edi` `ebp` | yes | yes | On x86_64 these name the low half of the 64-bit register |
| `rax` `rbx` `rcx` `rdx` `rsi` `rdi` `rbp` | no | yes | |
| `r8`..`r15`, `r8d`..`r15d` | no | yes | |
| `esp+N` `esp-N`, `ebp-N`, `esi+N` | yes | yes | Passes the *address* `N` bytes from the register's value, not what is stored there. Any register in the rows above takes an offset. `N` is written as a constant's digits are, so `ebp-0x10` works |
| `rsp+N` `rsp-N`, `r15+N` | no | yes | `esp+N` is accepted here too, so a hook ported off the Windows build needs no edit |
| `esp` `rsp` on their own | no | no | The wrapper keeps no saved copy of the stack pointer. Use `esp+0` for the game's stack |
| `[eax]`, `[esp+8]`, `[rdi+0x10]` | yes | yes | Reads *through* the register. See below |
| `const:<value>` | yes | yes | A literal, read from nowhere. See below |

Source names are matched case-insensitively.

### Constants

`source = "const:0xBC"` hands the patch function a literal instead of reading a register. It exists so one exported function can serve several game builds that differ only by an offset or a count, rather than needing an exported function per build.

The value is unsigned, written in decimal or with a `0x` prefix. A leading `-` is refused, which keeps the range check against the declared type a straight comparison: a negative constant is written as its bit pattern instead, so `-1` as an `sbyte` is `const:0xFF`. A leading zero is not read as octal, so `const:010` is ten.

### Dereferences

`source = "[esi+0x10]"` hands the patch function what the slot holds rather than its address. Any source naming a register takes brackets, with an optional signed offset written as a constant's digits are, and the declared type gives the width of the load.

Without brackets, `esi` passes the pointer itself, which is what a hook wanting the object rather than one of its fields means, and `esi+0x10` passes the field's address, for a hook that writes to it. Before this a bracketed source matched no form and was refused, so a hook wanting a field took the pointer and dereferenced it on the other side (issue #156).

`[esp+N]` is the form with no useful counterpart: `esp+N` yields the slot's address, so a hook wanting an argument the game pushed brackets it, and that is also how to ask for one at a narrow width.

### What `type` decides

The declared type says how much of the source reaches the patch function. Every type used to emit the same load, so `byte` and `short` described something the wrapper was not doing.

| Type | x86 | x86_64 |
| --- | --- | --- |
| `byte` | `MOVZX` from the low 8 bits | `MOVZX` from the low 8 bits |
| `short` | `MOVZX` from the low 16 bits | `MOVZX` from the low 16 bits |
| `sbyte` | `MOVSX` from the low 8 bits | `MOVSX` from the low 8 bits |
| `sshort` | `MOVSX` from the low 16 bits | `MOVSX` from the low 16 bits |
| `int`, `uint` | the whole 32-bit register | the low 32 bits, upper half cleared |
| `pointer` | the whole 32-bit register | all 64 bits |
| `int64`, `uint64` | refused | all 64 bits |
| `float` | pushed as 4 raw bytes | `MOVD` into an XMM register |
| `double` | refused | `MOVQ` into an XMM register |

`byte` and `short` zero-extend; `sbyte` and `sshort` sign-extend. Taking the full width instead of a narrow type would not work either way, because the bits above a narrow value are whatever the engine last left in that register.

`int` and `uint` emit the same instruction. The difference lives in the patch function's own declaration.

`int64`, `uint64` and `double` are x86_64 only, a 32-bit target having neither a register to read one out of nor a single stack slot to pass it in. Declaring `int` for a 64-bit value truncates it, which is worth knowing because that combination used to work by accident: the generator ignored the declaration and loaded all 64 bits regardless.

Two combinations are refused rather than guessed at:

- **A narrow type or a float on an address.** `esp+8` and `esi+4` yield an *address*, which is pointer-width whatever is there. Bracket it, as `[esp+8]`, to read what is there instead.
- **A float constant.** There is no syntax for a float literal, and an integer parse would not produce the bit pattern the hook meant.

### WrapperConfig

Configuration passed to wrapper generator:

- **patchFunction**: Address of patch function to call
- **hookAddress**: Game code address being hooked
- **originalBytes**: Stolen bytes to re-execute after patch
- **parameters**: Parameter extraction configuration
- **State preservation options**: Control register/flag saving
- **skipOriginalBytes**: Skip stolen byte execution

### HookType Enum

Defines the four hook types (DETOUR, SIMPLE, REPLACE, DLL_ONLY).

### ParameterType Enum

Defines supported parameter types (INT, UINT, POINTER, FLOAT, BYTE, SHORT).

## Core Functions

### Initialization

**InitializePatcher()**: Called on DLL_PROCESS_ATTACH. Initializes wrapper generator, starts the registry, loads patch_config.toml, sets KOTOR_VERSION_SHA environment variable, and applies all patches. Each patch module's `KPatchInit` runs as the module is loaded.

**CleanupPatcher()**: Called on DLL_PROCESS_DETACH. Closes the registry, frees wrapper stubs, deallocates REPLACE hook memory, unloads patch DLLs, then resets the registry. The order is spelled out under [Lifetime and cleanup order](#lifetime-and-cleanup-order).

### Patch Application

**ApplyPatches()**: Iterates through loaded patches and applies each one.

**ApplyPatch()**: Applies a single patch based on its type. Routes to type-specific handlers.

**ApplySimpleHook()**: Verifies bytes and performs direct memory replacement.

**ApplyReplaceHook()**: Allocates executable memory, writes replacement code, adds return JMP, and patches hook address.

### DETOUR Hook Application

For DETOUR hooks, `ApplyPatch()`:

1. Loads patch DLL via LoadLibraryA
2. Runs the module's `KPatchInit` export, if it has one (once per module)
3. Gets function address via GetProcAddress
4. Detects and skips hot-patch stub (0xCC byte) if present
5. Verifies original bytes
6. Generates wrapper via wrapper generator
7. Writes JMP to wrapper at hook address
8. NOPs remaining bytes

### Config Parsing

**Config::ParseConfig()**: Parses patch_config.toml and populates vector of PatchInfo structures. Extracts target_version_sha and validates hook configurations.

**ParseHexAddress()**: Converts hex strings ("0x401234") to DWORD addresses.

**ParseByteArray()**: Converts TOML arrays of integers or hex strings into byte vectors.

### Trampoline Utilities

**Trampoline::WriteJump()**: Writes 5-byte relative JMP (E9 xx xx xx xx) to specified address.

**Trampoline::WriteCall()**: Writes 5-byte relative CALL (E8 xx xx xx xx) to specified address.

**Trampoline::VerifyBytes()**: Compares bytes at address with expected values before patching.

**Trampoline::UnprotectMemory()**: Changes memory protection to PAGE_EXECUTE_READWRITE.

**Trampoline::ProtectMemory()**: Restores original memory protection.

**Trampoline::WriteNoOps()**: Writes NOP instructions (0x90) to fill unused bytes.

### Wrapper Generation

**WrapperGenerator_x86_Win32::GenerateWrapper()**: Routes to GenerateDetourWrapper().

**GenerateDetourWrapper()**: Generates x86 machine code for DETOUR wrapper:

1. Allocates executable memory
2. Emits PUSHAD (save registers) and PUSHFD (save flags)
3. Saves ESP to EBX for later restoration
4. Extracts and pushes parameters in reverse order
5. Calls patch function with relative CALL
6. Cleans up parameters (cdecl caller cleanup)
7. Restores ESP from EBX
8. Emits POPFD and POPAD (or selective register restoration)
9. Re-executes original bytes or skips them based on configuration
10. Jumps back to game code

**ExtractAndPushParameter()**: Generates x86 code to extract a parameter from saved CPU state or stack and push it for the patch function. Handles register sources (eax, ebx, etc.), a register with an offset as an address (esp+4, esi+0x10, etc.), dereferences of either, and constants.

**EmitBytes/EmitByte/EmitDword**: Helper functions to write raw bytes into code buffer.

**CalculateRelativeOffset()**: Calculates 32-bit relative offset for JMP/CALL instructions.

## Patch registry and KPatchInit (RFC #180)

Patch modules are separate binaries, possibly built by different compilers, and they share one process with the patcher. They cannot share a C++ ABI, and on Linux and macOS they cannot see each other's symbols (see [No symbol lookup between patches](#no-symbol-lookup-between-patches)). The registry is the supported way for one patch to use another patch's code. It implements the proposal in LaneDibello/Kotor-Patch-Manager#180.

The mechanism:

- A patch may export a function named `KPatchInit`. The patcher calls it once, right after loading the module, and passes a `KPatchApi` with two calls: `provide(name, version, iface)` publishes a table of function pointers under a name, and `require(name, version)` returns the table another provider published, or `NULL`.

A patch with no `KPatchInit` export is unaffected. No existing patch needs to change. For how a patch author uses this, see `docs/PatchRegistry.md`.

### The header

`Patches/Common/kpatch_api.h` is the contract. It is pure C: it compiles as C11 and as C++17, needs only `<stddef.h>` and `<stdint.h>`, and does not include `windows.h`. The patcher includes it through `include/kpatch_api.h` instead of adding `Patches/Common` to its include path, because that directory also holds a `Platform.h` that collides case-insensitively with this directory's `platform.h`.

### When KPatchInit runs

`ApplyPatch()` loads patch modules through `LoadPatchModule()`, which calls `Platform::LoadModule`, records the handle for cleanup, and then calls `Registry::OnModuleLoaded(handle, id)`. That function looks the export up with `Platform::GetSymbol(handle, "KPatchInit")` and calls it.

- **Right after the load, before the hook symbol lookup.** For a DETOUR patch `KPatchInit` has run by the time `functionName` is resolved, so a module can set itself up before its first hook is applied.
- **Once per distinct module handle.** A config that lists one DLL for twenty hooks loads it twenty times and initialises it once. The handle is remembered whether or not the module has the export, so a module without one is looked up once.
- **Even if a later step fails.** If the hook function is missing, the original bytes do not match, or the wrapper cannot be generated, `KPatchInit` has already run and anything it provided stays registered until cleanup. When `ApplyPatches()` aborts it logs one `[KotorPatcher] Apply aborted: N module(s) initialised` line.
- **Only for modules that are loaded.** DETOUR and DLL_ONLY patches load a module. SIMPLE and REPLACE patches load nothing, so there is nothing to initialise.
- **A missing export is normal.** It is not logged and is not an error.
- **Attribution.** The registry records which patch provided each interface, using the `id` from `patch_config.toml`, or the DLL's file name without directory or extension if the entry has no `id`. The id appears in registry log lines.
- **Order.** Modules are initialised in config order. That order is not a contract.

### What KPatchInit may do

`KPatchInit` is DllMain-class code. Where it runs depends on the platform and on how the game starts:

- Windows, normal start: inside the patcher's `DllMain` (`DLL_PROCESS_ATTACH`), so under the loader lock.
- Windows, SteamStub: when the hook sites are still encrypted, the apply is deferred to a worker thread that waits for the stub to decrypt `.text`. `KPatchInit` then runs on that worker, which does not hold the loader lock but runs concurrently with the game's own startup.
- Linux and macOS: inside the library constructor, before the game's `main()`.

A patch must therefore treat it as if the loader lock is held:

- It may store the `KPatchApi` pointer, call `provide` and `require`, and make cheap registration calls on an interface it gets.
- It must not load libraries (`LoadLibrary`, `dlopen`), wait on other threads or events, start a thread and wait for it, or do file or network I/O.
- It must not assume another patch has been initialised. A patch that needs another patch's interface should call `require` lazily at first use, not in `KPatchInit`, and must handle `NULL` because the provider may not be installed.

The registry's mutex is not held while `KPatchInit` runs, so the function may call `provide` and `require` freely (they re-enter the registry).

### Threading

Every registry call is safe from any thread. `provide` and `require` take one mutex for a map lookup or insert, and `require` is cheap enough to call on a first-use path, but callers are expected to cache the result.

### Lifetime and cleanup order

The `KPatchApi` pointer and every interface pointer `require` returns stay valid until `CleanupPatcher()` begins. `CleanupPatcher()` then runs in this order:

1. `Registry::Close()`. From here `require` returns `NULL` and `provide` is rejected, so no patch can fetch an interface whose provider is about to be unloaded.
2. Free the wrapper stubs and REPLACE code buffers, and unload the patch modules.
3. `Registry::Reset()`: forget every provided interface and every module handle seen. This comes last because the OS may hand the same handle value to a different module later, and that module has to get its own `KPatchInit`.

A patch must never call **another** patch's interface from its DLL detach path (or a static destructor): the provider may already be unloaded.

The registry is heap-allocated and intentionally never deleted. A function-local static would be destroyed at process exit, while a patch thread may still be calling in: on Linux `exit()` runs static destructors with other threads alive, and on Windows `ExitProcess` kills threads that may hold the mutex. After `Close()` the calls become harmless no-ops (`require` returns `NULL`).

### ABI rules

- **Pure C, fixed-width types.** Interface structs hold only `uint32_t`, `int32_t`, `uint64_t`, `const char*` and function pointers. No `bool`, enums, `double`, `std::string`, exceptions or C++ objects cross the boundary.
- **`struct_size` first.** The first field of every interface struct is `uint32_t struct_size`, set by the provider to `sizeof` its struct.
- **Append-only within a major version.** Fields are added at the end and never removed, reordered or resized. A consumer that wants a field newer than its oldest supported provider checks `struct_size` before reading it.
- **The version is the major version.** `require("x", 1)` matches only an interface provided as `("x", 1)`. A breaking change is a new major, and a provider may provide `("x", 1)` and `("x", 2)` side by side.
- **First provider wins.** A second `provide` of the same name and major is rejected (a non-zero return, plus a log line naming both providers). The first pointer keeps being served. `provide` also rejects a null or empty name and a null interface.
- **`pack(8)`.** The structs sit inside `#pragma pack(push, 8)`, because `Common.h` opens `pack(push, 4)` and every compiler has to agree on the layout. The header carries `static_assert` checks of the sizes and offsets.
- **`KPATCH_CALL`.** Every function pointer in the ABI uses `KPATCH_CALL`: `__cdecl` for MSVC and MinGW, `__attribute__((cdecl))` for GCC on i386, and empty elsewhere (x86_64 has one convention). On x86-32 Windows compilers disagree about the default convention, so it is pinned.
- **Lifetime of an interface.** A provider's interface table must stay valid until the patcher cleans up (static storage is the usual choice).

### No symbol lookup between patches

Patch modules are opened with `dlopen(path, RTLD_NOW | RTLD_LOCAL)` on Linux and macOS. `RTLD_LOCAL` keeps a module's exported symbols out of the global scope, so one patch cannot find another patch's functions with `dlsym(RTLD_DEFAULT, ...)`, and an undefined symbol in one patch is not satisfied by another that happens to be loaded. Linking one patch against another's `.so` through `DT_NEEDED` would make the dynamic loader own the dependency, with its own load and unload order, and is not supported. The registry is the supported way to share code between patches. The patcher itself offers no symbol API to patches: everything a patch needs from the core comes through `KPatchApi` and the interfaces it provides.

## Wrapper Code Generation Details

The generated DETOUR wrapper follows this structure:

```
1. PUSHAD              ; Save all registers (32 bytes)
2. PUSHFD              ; Save flags (4 bytes)
3. MOV EBX, ESP        ; Save stack pointer for restoration
4. [Extract params]    ; Read from saved state, push in reverse order
5. CALL patch_func     ; Call the patch function
6. ADD ESP, N          ; Clean up parameters (cdecl)
7. MOV ESP, EBX        ; Restore stack pointer
8. POPFD               ; Restore flags
9. POPAD or selective  ; Restore registers (respect excludeFromRestore)
10. [Original bytes]   ; Re-execute stolen instructions (unless skipped)
11. JMP return_addr    ; Jump back to game code
```

Register exclusion allows patches to modify specific registers (e.g., changing EAX to modify return value) by selectively skipping restoration for excluded registers.

## Debug Logging

KotorPatcher logs initialization, patch application, and errors through the platform seam's log operation (`Platform::Log`): `OutputDebugStringA()` on Windows, stderr on Linux. Neither writes to disk by default. All log messages are prefixed with component names:

- `[KotorPatcher]`: Main patcher operations
- `[Config]`: Configuration parsing
- `[Trampoline]`: Memory patching operations
- `[Wrapper]`: Wrapper generation

## Viewing Debug Logs

On Windows, use Sysinternals DebugView to capture debug output:

1. Download DebugView from Microsoft Sysinternals
2. Run as Administrator
3. Enable "Capture Global Win32" in the Capture menu
4. Filter for "KotorPatcher" to see only patcher messages
5. Use Ctrl+X to clear the log buffer

On Linux, the log goes to stderr, which Steam redirects away. Set `KPATCH_LOG` to a
file path to capture a run, for example `KPATCH_LOG=/tmp/kp.log %command%` in the
Steam launch options. The file is truncated per launch.

Debug logs show patch loading, hook application, wrapper generation, and any errors encountered during runtime.

## Memory Management

Allocation and module loading go through the platform seam: VirtualAlloc/LoadLibraryA on Windows, mmap/dlopen on Linux.

**Wrapper stubs**: Allocated as readable, writable and executable, tracked in m_allocatedWrappers, freed on cleanup.

**REPLACE code buffers**: Allocated the same way, tracked in g_allocatedCodeBuffers, freed on cleanup.

**Patch binaries**: Loaded by name, tracked in g_loadedPatches, unloaded on cleanup.

All allocations are cleaned up in CleanupPatcher() to prevent memory leaks.

## Environment Variables

KotorPatcher sets the `KOTOR_VERSION_SHA` environment variable based on target_version_sha from patch_config.toml. This allows patch binaries to query the game version and adjust behavior accordingly.

KotorPatcher reads `KPATCH_LOG` on Linux. When set to a file path, log output is written there in addition to stderr. Unset, nothing is written to disk.

## Technical Constraints

**Platform**: x86 32-bit only (KotOR is a 32-bit game). Both targets are 32-bit x86: the Windows PE game and KOTOR II's native Linux build.

**Calling convention**: Patch functions must use __cdecl convention (caller cleans stack).

**Function exports**: DETOUR patch functions must be exported as `extern "C"` to prevent name mangling.

**Instruction boundaries**: Original bytes must align with x86 instruction boundaries (minimum 5 bytes for DETOUR/REPLACE to fit JMP instruction).

**Memory protection**: All memory patching temporarily makes the target writable and executable, then restores the original protection (VirtualProtect on Windows, mprotect on Linux).

**Hot-patch stubs**: Automatically detects and skips Visual Studio hot-patch stubs (0xCC byte before function entry).

## Error Handling

All patch operations verify original bytes before modification to detect version mismatches. Failures log detailed error messages via OutputDebugStringA and return false to abort initialization. If any patch fails to apply, the entire initialization fails to prevent partial/corrupted game state.
