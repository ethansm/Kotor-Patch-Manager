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
- The `kpatch.log` service: line format, config, hot reload, sinks, rotation,
  rate limiting, ring overflow, drop accounting, the session header, shutdown,
  and the real flush thread.

## Registry, module and log tests

These three are `any_*` tests like the engine ones, but they do not generate or
run machine code. They exercise the registry (`src/core/registry.cpp`) and the
log service (`src/core/log_service.cpp`), which are linked into every test
binary through `TEST_ENGINE` in the Makefile. Because the service and the
registry use threads, the test compile lines pass `-pthread`.

### any_registry

Links no module. It calls the registry directly, and a row that needs an init
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
that exists today. `fixtures/logger.c` logs through `kpatch.log` from its
`KPatchInit`; no unit test loads it, it is for a host smoke run that points a
built `KotorPatcher.so` at a temporary `patch_config.toml` and `kplog.ini`.

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

### any_log_service

Drives `src/core/log_service.cpp` through its own API. Every row starts a fresh
session in a scratch directory next to the test binary
(`build/tests/logsvc_tmp_<width>`, wiped per row). Most rows run with the flush
thread off, so the test calls the flush and config poll itself, and with a fake
clock, so timestamps and the rate-limit window are exact. These seams exist for
that:

- `LogService::Options{startThread=false}` passed to `Init()` leaves the thread
  out.
- `LogService::FlushNow()` and `PollConfigNow()` do what the thread does on its
  cycle.
- `LogService::SetClockForTest()` replaces the clock.
- `LogService::LockRingForTest()` and `UnlockRingForTest()` hold the ring lock
  from a helper thread, to show that `Shutdown()` stays bounded.

Deterministic rows: line format (including `f=-` before any tick, and the WARN
and ERR markers); the `NowUs`, `Frame` and api table; frame ownership; default
levels with no ini; the `enabled` switches; level by number and by name;
channels lists (`*`, `-x`, exclusions only); hot reload by content within the
same second, and removing the file; malformed lines ignored; a channel
registered after a reload; Register and Channel limits (63 and 64 character ids,
the 65th patch, the 257th channel); the truncation marker; null, empty and bad
input; the perpatch, merged and both sinks; byte-cap rotation, including
replacing an existing `.1` and writing a new header; launch rotation; the rate
limit and `DROPPED` lines; ring overflow with the WARN and ERR reserve (a
100,000-line burst must finish in under two seconds, and measures a few
milliseconds); `Mark` reaching every open file; the session header and the
later `# session info` block; an unwritable directory; shutdown and submit after
shutdown.

One deterministic row is multithreaded: eight threads submit 10,000 lines each
while another thread flushes in a loop. It checks that flushed plus dropped
lines add up to 80,000 and that each thread's lines stay in order.

#### Threaded rows and their timing

The rows named "real flush thread" use the real clock and the real thread, and
poll the disk. They are the only part of the suite that depends on timing, so
the limits are generous compared with what they measure on the machine they were
written on (an x86_64 Linux host, about five seconds for the whole binary):

- A WARN reaches the disk within 1500 ms, and under 1000 ms with `flush_ms`
  raised to 5000, which shows the wake-up and not the periodic flush did it
  (observed about 10 ms).
- An INFO line reaches the disk through the periodic flush within 1500 ms
  (observed about 500 ms). An ERR and a `Mark` arrive within 1500 ms (about
  10 ms).
- The thread picks up a newly written `kplog.ini` within 2500 ms (observed about
  1000 ms).
- `Shutdown()` with the ring lock held by another thread for 900 ms returns in
  at least 150 ms and under 600 ms (observed about 200 ms), logs `final flush
  skipped (lock busy)`, and a new session afterwards works.
- A superseded session's thread leaves a new session alone. This row sleeps 600
  to 700 ms to show that something does not happen, and runs 100 rapid
  Init, Shutdown, Init cycles.
- After `Shutdown()` further submits write nothing (checked after a 700 ms wait).

On a heavily loaded machine these are the rows to suspect first if a run fails
once and passes on the next.
