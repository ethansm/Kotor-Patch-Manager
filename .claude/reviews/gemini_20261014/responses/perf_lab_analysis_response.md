### Findings

1. **`patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java` — `run**`

* **Anchor:** `for (Address t : new ArrayList<>(targets)) {`

* **What is wrong and trigger:** `new ArrayList<>(targets)` snapshots `targets` when it contains only the single `EXTERNAL` symbol address (`s.getAddress()`). In a PE binary in Ghidra, `s.getAddress()` is referenced by the IAT data slot in `.idata` (where `getFunctionContaining(r.getFromAddress())` is `null`), and the import thunk function in `.text` (`JMP dword ptr [IAT]`) references the IAT slot rather than `s.getAddress()`. Because the first loop iterates only over the initial 1-element snapshot and never expands references to the newly added IAT slot addresses, thunk functions are never added to `targets`. The second loop then skips the thunk itself (`if (f != null && f.isThunk()) continue;`), causing `ExportImportXrefs` to miss 100% of import call sites that call through a compiler thunk.


* **Suggested fix:** Expand `targets` iteratively across both hops (external symbol $\to$ IAT slot $\to$ thunk entry point):
```java
Queue<Address> q = new ArrayDeque<>(targets);
while (!q.isEmpty()) {
    Address t = q.poll();
    for (Reference r : getReferencesTo(t)) {
        Address from = r.getFromAddress();
        Data d = currentProgram.getListing().getDataAt(from);
        if (d != null && targets.add(from)) q.add(from);
        Function tf = getFunctionContaining(from);
        if (tf != null && tf.isThunk() && targets.add(tf.getEntryPoint())) q.add(tf.getEntryPoint());
    }
}

```


* **Confidence:** certain


2. **`patch_manager_mods/perf_lab_src/ghidra_scripts/ExportCallGraph.java` — `run**`

* **Anchor:** `if (tg != null && tg.isExternal()) { outs.add(new String[] { tg.getName(), "IMPORT" }); continue; }`

* **What is wrong and trigger:** The outer loop skips all thunk functions (`if (f.isThunk()) continue;`), so thunks never emit their own outgoing edges. When a non-thunk function `f` calls or tail-jumps to an **internal** thunk `g` (where `tg = g.getThunkedFunction(true)` is non-null and `!tg.isExternal()`, such as `operator_new_arr` at `0x91D288` thunking to `0x919723`), `ExportCallGraph` does not resolve `g` to `tg` and instead emits `hx(g.getEntryPoint())`. Consequently, `tg` never receives those edges in `callgraph.tsv` or `callers_count.tsv`, and the thunk `g` appears as a leaf callee with no outgoing edge to `tg`.


* **Suggested fix:** When `g.isThunk()` and `tg != null && !tg.isExternal()`, resolve `g = tg` before emitting the `CALL` / `TAILJMP` edge.


* **Confidence:** certain


3. **`patch_manager_mods/perf_lab_src/analyze_runs.py` — `parse_perflab_pair`, `parse_textlog_pair`, and `render**`

* **Anchor:** `if nb and (not ob or tdiff(nb[-1].tick, ob[-1].tick) < 0x80000000 and nb[-1].tick != ob[-1].tick):`

* **What is wrong and trigger:**
1. When a game directory contains earlier launches logged to the legacy/fallback files (`perflab_log.txt`, `robustnessfixes_log.txt`, etc.) and a later launch logged to the KpLog core files (`perf-lab_log.txt`, `k2-robustness-fixes_log.txt`, etc.), `parse_perflab_pair` and `parse_textlog_pair` discard the entire list of attaches/blocks from the older file instead of merging them across files. Meanwhile, `perflab_metrics.txt` still contains all launches, so every earlier launch in `perflab_metrics.txt` loses its attach metadata and mod log blocks.


2. If the system or Wine server restarted between runs so that `GetTickCount()` reset to a smaller value in the newer file, `tdiff(nb[-1].tick, ob[-1].tick) < 0x80000000` evaluates to `False` and selects the stale file.


3. In `render()`, the `## Sources` table only renders `infos[n]` for `ALL_FILES` (the legacy file names). When a run uses the KpLog core filenames (`perf-lab_log.txt`, `k2-robustness-fixes_log.txt`, etc.) and the legacy files are absent, `## Sources` reports `ABSENT` for logs that were actually read and matched.




* **Suggested fix:** Merge blocks/attaches from both files (deduplicating by `(tick, pid)` or choosing per-launch during `build_launches`), and include present `LOG_ALIASES` files in the `## Sources` table.


* **Confidence:** certain


4. **`patch_manager_mods/perf_lab_src/analyze_runs.py` — `build_launches**`

* **Anchor:** `elif prev is not None:`

* **What is wrong and trigger:**
1. When a metrics launch has no matched attach in `perflab_log.txt` (e.g., when `perflab_log.txt` is absent as in `test_only_metrics_present`, or when an offset skips earlier attaches), every launch after the first sets `x.start = prev.hi + 1` (1 ms after the previous launch ended) instead of deriving `x.start` from `x.blocks[0].tick`. This inflates the launch's reported duration (`tdiff(x.hi, x.start) / 1000.0`) with all idle time between game sessions and shifts `x.wall_s` backward by `tdiff(b.tick, prev.hi + 1)`.


2. If `prev` is an attach-only stub (`prev.hi = prev.attach["tick"] + 60000`) and the next launch has no attach and finishes within 60 s of the stub's attach (`x.blocks[-1].tick < prev.hi + 1`), `x.lo` (`prev.hi + 1`) exceeds `x.hi`, causing `find(tick)` to fail for all blocks of `x` (matching them to the stub instead) and wrapping `tdiff(x.hi, x.start)` to ~49.7 days.


3. In the attach-matching pass (`cand = [...]`), there is no upper bound on `tdiff(first, a["tick"])` (other than `< 0x80000000`), so a metrics launch whose own attach line is missing will steal an earlier crashed stub's attach line instead of leaving the earlier attach as a stub.




* **Suggested fix:** Bound attach-to-first-block distance (e.g., `tdiff(first, a["tick"]) <= 120000`), and when `x.attach` is `None`, set `x.start = max(prev.hi + 1, x.blocks[0].tick - 120000)` if `prev is not None` (and cap a stub's `hi` at the next launch's start tick minus 1).


* **Confidence:** certain


5. **`patch_manager_mods/perf_lab_src/resolve_sites.py` — `mblocks`, `loads`, and `kv**`

* **Anchor:** `v = [int(x) for x in l.split()[1:]]; cur['rt'][v[0]] = dict(zip(RTF, v[1:]))`

* **What is wrong and trigger:**
1. In perf-lab 0.5.0+, `RT` lines can include `redemB=<n>` or `RT stat redemB=<n>`. `mblocks` runs `[int(x) for x in l.split()[1:]]`, which crashes with `ValueError: invalid literal for int() with base 10` on `'redemB=...'` or `'stat'`.


2. Any blank line inside a `#METRICS`..`#END` block hits `elif l[:1] != '#': cur['lines'].setdefault(l.split()[0], kv(l))` (since `"" != "#"`) and crashes with `IndexError: list index out of range`.


3. `mblocks` does not split launches at `#METRICS 1`: `resolve_sites.py metrics FILE A B` looks up `next(b for b in bs if b['n'] == int(x))`, which always picks block `n` from Launch 1, and `loads(bs)` pairs a hung `LOAD_UP` at the end of one launch with a `LOAD_DOWN` in a later launch, producing negative counter deltas across processes.




* **Suggested fix:** Ignore non-integer tokens after column 12 on `RT` lines (and skip `RT stat`), guard empty lines (`if not l.strip(): continue`), and reset `up = None` on `#METRICS 1`.


* **Confidence:** certain


6. **`patch_manager_mods/perf_lab_src/ghidra_scripts/NewNullCheck.java` — `run**`

* **Anchor:** `for (int k = 0; q != null && k < 8; k++, q = q.getNext()) {`

* **What is wrong and trigger:**
1. The forward instruction walk breaks on `isCall()` and (when `getNumOperands() < 2`) `isJump()`, but never breaks on `RET` (`q.getFlowType().isTerminal()`) or when `q` leaves the current function body, nor when the call site itself is an unconditional `JMP` thunk (`r.getReferenceType().isJump()`, e.g., `operator_new_arr` at `0x91D288`). When a small wrapper function calls `operator_new` and immediately returns (`ADD ESP, 4; RET`), `q.getNext()` walks past `RET` into the next function in `.text`.


2. Only `MOV` removes overwritten registers from `held` (`if (held.contains(o1)) held.add(o0); else held.remove(o0);`). Any other instruction that overwrites `EAX` (e.g., `XOR EAX, EAX`, `LEA EAX, [...]`, `POP EAX`) leaves `"EAX"` in `held`, so a subsequent `TEST EAX, EAX` on the new value falsely reports `checked = "yes"`.




* **Suggested fix:** Break the forward walk when `q.getFlowType().isTerminal() || q.getFlowType().isJump() || (f != null && !f.getBody().contains(q.getAddress()))`, skip sites where `r.getReferenceType().isJump()`, and remove `o0` from `held` whenever a non-test instruction writes to `o0`.


* **Confidence:** certain


7. **`patch_manager_mods/perf_lab_src/persist_findings.py` — `<module>**`

* **Anchor:** `u=sys.argv[1]; ver=sys.argv[2].split(',')`

* **What is wrong and trigger:**
1. If the second CLI argument is empty (`""`, when no rows in the batch are Opus-verified) or has a trailing comma (`"7054ce,"`), `ver` contains `''`. Because `'' in ev.lower()` is `True` for every string in Python, `v = any(x in ev.lower() for x in ver)` evaluates to `True` on every row, upgrading all rows in the batch (including `low` confidence rows) to `high` confidence and labelling them `"Opus-verified"` in `kotor2_ghidra_knowledge.db`.


2. When `v` is `False`, `conf='high' if v else ('low' if conf.startswith('low') else 'medium')` downgrades any row whose markdown table confidence was already `high` to `medium`.




* **Suggested fix:** Filter empty tokens (`ver = [x.strip().lower() for x in sys.argv[2].split(',') if x.strip()]`) and preserve existing `'high'` confidence when `conf.startswith('high')`.


* **Confidence:** certain


8. **`patch_manager_mods/perf_lab_src/hookbytes.py` — `<module>` and `insns**`

* **Anchor:** `rel=any(t.split()[0] in ('CALL','JMP') and '0x' in t and '[' not in t or t.startswith('J') for t in txt)`

* **What is wrong and trigger:**
1. Because `and` binds tighter than `or` in Python, `t.startswith('J')` evaluates to `True` for every `JMP` instruction, completely short-circuiting `t.split()[0] in ('CALL','JMP') and '0x' in t and '[' not in t` for `JMP`. Indirect jumps (`JMP dword ptr [0x...]`, `JMP EAX`) are therefore falsely flagged as `REL-BRANCH-IN-STOLEN`.


2. `while len(got)<2*max(5,len(exp.replace(' ',''))//2): got+=ins[k][1]; txt.append(ins[k][2]); k+=1` does not check `k < len(ins)` and raises `IndexError` if a hook site is within 4 bytes of the end of a function.


3. `fns` is loaded from `steam_functions.tsv` (which includes thunks and `Unwind@...` exported by `ExportAllFunctions.java`), whereas `DumpAllDecomp.java` skips thunks, externals, and `Unwind@...` when generating `<addr>.asm`. Passing a thunk address (or an address after a thunk inside a split function) raises `FileNotFoundError`.




* **Suggested fix:** Parenthesize the condition (`(t.split()[0] in ('CALL','JMP') and '0x' in t and '[' not in t) or (t.startswith('J') and not t.startswith('JMP'))`), guard `k < len(ins)` in the `while` loop, and skip thunk/`Unwind@` rows when loading `steam_functions.tsv`.


* **Confidence:** certain


9. **`patch_manager_mods/perf_lab_src/analyze_runs.py` — `analyze**`

* **Anchor:** `reds = [b.rt_redem[TPC_RESTYPE] for b in x.blocks if TPC_RESTYPE in b.rt_redem]`

* **What is wrong and trigger:** `parse_metrics` explicitly supports standalone `RT stat redemB=<n>` lines by storing `blk.rt_redem["stat"] = red` (comment: ``RT stat redemB=..` tolerated`). However, `analyze(x)` only looks up `b.rt_redem[TPC_RESTYPE]` (`3007`) and never checks `b.rt_redem.get("stat")`, so `RT stat redemB=<n>` lines are silently dropped and `a["redemB"]` evaluates to `None`.


* **Suggested fix:** Fall back to `b.rt_redem.get("stat")` when `TPC_RESTYPE` is not in `b.rt_redem`:
```python
reds = [b.rt_redem.get(TPC_RESTYPE, b.rt_redem.get("stat")) for b in x.blocks if TPC_RESTYPE in b.rt_redem or "stat" in b.rt_redem]

```


* **Confidence:** certain


10. **`patch_manager_mods/perf_lab_src/analyze_runs.py` — `Launch._mk` and `Launch.flags**`

* **Anchor:** `for b in (down, up):`

* **What is wrong and trigger:**
1. In `Launch._mk`, the fallback loop `for b in (down, up):` that inspects `for key in ("mod", "area"):` only breaks out of the inner `key` loop when a match is found. If both `down` and `up` carry `hdr["mod"]` or `hdr["area"]`, the loop continues to `up` and overwrites `L["module"]` with the `LOAD_UP` module instead of keeping `LOAD_DOWN`'s module.


2. In `Launch.flags()`, `self.attach["kv"]` excludes only `("exe", "scan")` and omits `"tick"` (which is present on all 0.5.1+ KpLog attach lines), causing `tick=<n>` to be rendered as a config flag in `- config/header flags:`.




* **Suggested fix:** Stop the outer `for b in (down, up):` loop once `L["module"] is not None`, and exclude `"tick"` from `self.attach["kv"]` in `Launch.flags()`.


* **Confidence:** certain


11. **`patch_manager_mods/perf_lab_src/analyze_runs.py` — `render_launch**`

* **Anchor:** `ref = sorted(set(ref)) if "REFUSED" in rb.loaded or ref else []`

* **What is wrong and trigger:** When `robustnessfixes_log.txt`'s `loaded:` line itself contains `"REFUSED"` and there are no separate `REFUSED` lines in `rb.lines`, `ref` is `[]`, so `(" / ".join(ref) if ref else "none")` renders `REFUSED none`, hiding the refusal on the `loaded:` line.


* **Suggested fix:** Include `rb.loaded` in `ref` when `"REFUSED" in rb.loaded` and `ref` is otherwise empty.


* **Confidence:** certain



---

### Files or Areas Not Assessed

* **`patch_manager_mods/perf_lab_src/fixtures/game/*`, `patch_manager_mods/perf_lab_src/fixtures/game050/*`, and `patch_manager_mods/perf_lab_src/fixtures/gen_fixtures.py`:** Omitted from the bundle (`manifest-omit, 0 bytes` in `<directory_tree>` and `4d_perf_lab_analysis.manifest.tsv`).


* **`scripts/kplog_parse.py`:** Imported by `analyze_runs.py` from `../../scripts` (`import kplog_parse`), outside `perf_lab_src/` and not included in the bundle.


* **`ghidra_knowledge/kotor2_ghidra_knowledge.db` and `steam_functions.tsv`:** External SQLite database and TSV dump referenced by `persist_findings.py`, `resolve_sites.py`, and `hookbytes.py`, not included in the bundle.