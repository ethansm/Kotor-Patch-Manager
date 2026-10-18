# Code generator tests

Run them with `make test` from `src/KotorPatcher`.

The engine writes machine code while the game is running: the trampoline works
out jump displacements, the platform backend hands out memory to put stubs in,
and the two DETOUR wrapper generators emit the code that calls a patch
function. These tests cover that code.

They are not unit tests in the usual sense. Each one builds a fake hook site in
a page of stub memory, points it at a wrapper the generator has just produced,
jumps into it with a known value in every register, and checks what comes out
the other end. Running the bytes is the only way to tell whether they are
right. Reading them back only proves they are the bytes we meant to write, not
that those bytes do what we wanted.

## Why they run on the host

What a generator emits depends on the instruction set, not on the operating
system. An x86_64 wrapper built for macOS is byte for byte the one built here,
so running it on Linux tests that build too. That is worth having, because
testing the macOS engine any other way means a virtual machine with a copy of
the game in it.

You need an x86 host that can build 32-bit binaries (`glibc-devel.i686` on
Fedora, `gcc-multilib` on Debian and Ubuntu). There is nothing here to run on
other architectures.

## Layout

- `any_*` is built at both widths, once against each wrapper generator.
- `t32_*` and `t64_*` are built against the generator for their instruction
  set.
- `fixtures/*.c` are stand-in patch modules, plain C, built by `make test` into
  `build/tests/fixtures/32/` and `build/tests/fixtures/64/` (see below).

A test is a program that prints its checks and exits non-zero if any of them
failed. `check.h` is the whole framework.

## What is covered

- Jump displacements, including the wrap-around cases at 32 bits that a naive
  range check gets wrong, and the exact boundary at 64 bits.
- Stub memory: writable, then sealed, then callable, and placed close enough to
  a given address that a relative jump reaches it.
- Wrappers: passing arguments, reading a parameter out of a register or off the
  stack, stack alignment at the call, the saved state surviving a patch
  function that destroys everything it is allowed to, the stolen bytes running,
  flags surviving to the resume point, the red zone, excluded registers, and
  both consumed-exit paths.
- Floating point: x87, MMX, SSE and MXCSR surviving a patch function that calls
  `fninit` and zeroes the XMM registers.
- The patch registry: provide and require, versions, duplicates, invalid
  arguments, growth of an interface struct, re-entrant provides, concurrent
  providers and consumers, and what happens after `Close()` and `Reset()`.
- `KPatchInit` against real modules: init once per handle, lazy require,
  fallback when a provider is absent, attribution, and the module state after a
  partial apply.

## Registry and module tests

These two are `any_*` tests like the engine ones, but they do not generate or
run machine code. They exercise the registry (`src/core/registry.cpp`), which is
linked into every test binary through `TEST_ENGINE` in the Makefile. Because the
registry uses threads, the test compile lines pass `-pthread`.

### any_registry

Links no fixture module (one row loads the system libm to check a module
without `KPatchInit`). It calls the registry directly, and a row that needs an init
function uses the test seam `Registry::RunInitForTest(id, fn)`, which runs `fn`
through the same path as a module's `KPatchInit` (same attribution, same
logging). Rows are isolated by `Registry::Reset()`, which is also what the
patcher calls after unloading modules.

Rows: provide then require returns the same pointer; a missing name returns
null; a second major version is provided side by side; a duplicate is rejected
and the first pointer is still served; a null name or interface is rejected
without a crash; struct_size growth (a consumer checks the size before reading a
new field, and an old provider takes the fallback); the `KPatchApi` fields; a
provide inside an init does not deadlock; eight threads of concurrent
provide and require (one winner per contested name); require and provide after
close; the dump lists each provider.

Where a row has to prove a log line was written, it redirects stderr to a temp
file around the call, because registry messages go through `Platform::Log`.

### any_module_init

Loads real shared objects through `Platform::LoadModule` and hands the handles
to `Registry::OnModuleLoaded`, so the `KPatchInit` lookup runs against plain-C
patch code. The test is built with `-DFIXTURE_DIR="<build>/tests/fixtures/<width>"`
and loads `<FIXTURE_DIR>/<name>.so`. Each row starts with `Registry::Reset()`
and unloads everything it loaded. `dlopen` of a path that is still loaded
returns the same handle with its old statics, which would hide a missing init,
so the init counts the fixtures report double as proof the unload really
happened.

Rows: one `.so` loaded 20 times is initialised once; a consumer loaded before
its provider still succeeds at its first call (lazy require); a consumer without
a provider takes its fallback; a module with no `KPatchInit` returns false and
logs nothing; the provider is attributed by patch id; after a reset a reload
inits again; after a simulated abort the interface of the module initialised
before the abort stays valid until close while the missing one is null, and a
module loaded after `Close()` is not initialised.

### Fixtures

`fixtures/provider.c` provides `test.counter` v1. `fixtures/consumer.c` requires
it lazily at first use and takes a fallback if it is missing, the pattern the
header documents. `fixtures/noinit.c` has no `KPatchInit`, like every patch
that exists today.

The Makefile builds each fixture twice, with
`gcc [-m32] -std=c11 -Wall -shared -fPIC -I../../Patches/Common`, once per
width, so the `.so` a test loads matches its own word size. Nothing is written
outside `build/`.

The fixtures include the canonical `Patches/Common/kpatch_api.h` directly, as
patch code does, so compiling them as C11 is also a check that the header is
valid C and that its `_Static_assert` layout checks hold at both widths. That
covers GCC, including `KPATCH_CALL` as `__attribute__((cdecl))` on i386. The
Makefile does not compile the header with MinGW or MSVC. A MinGW check by hand, run from `src/KotorPatcher`:

```sh
printf '#include "kpatch_api.h"\nint main(void){return 0;}' |
    i686-w64-mingw32-gcc-posix -std=c11 -fsyntax-only -I../../Patches/Common -x c -
```
