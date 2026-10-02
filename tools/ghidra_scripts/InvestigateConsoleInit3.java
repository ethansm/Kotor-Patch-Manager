// Follow-on investigation for AdditionalConsoleCommands hook1, continuing
// from FindConsoleFuncCallers/FindConsoleInitCaller/FindTablePointerRefs.
// Two independent steps, both from the persisted next-steps list:
//   1. List all memory blocks and flag any with "CRT" in the name -- the
//      standard MSVC static-initializer array (.CRT$XCA/.CRT$XCU/.CRT$XCZ)
//      is normally merged into .rdata by the linker, so this usually comes
//      back empty, but it's the correct targeted check rather than
//      re-trusting the ad-hoc range heuristic from the previous session.
//   2. Re-find every caller of the 3 known ConsoleFunc ctors (should
//      reproduce the same ~35 wrapper functions at 0x983ea0-0x984500), then
//      decompile the first 3 of them to read their embedded command-name
//      string literal -- independent confirmation they're real ConsoleFunc
//      static initializers, not yet done in the prior session.
//
// @category KOTOR
// @menupath Tools.KOTOR.Investigate Console Init 3

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

import java.util.*;

public class InvestigateConsoleInit3 extends GhidraScript {

    @Override
    public void run() throws Exception {
        println("=== Step 1: memory blocks, flagging CRT-named ones ===");
        for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
            String name = block.getName();
            String flag = name.toUpperCase().contains("CRT") ? "  <-- CRT" : "";
            println("  " + name + "  " + block.getStart() + " - " + block.getEnd() + flag);
        }

        println();
        println("=== Step 2: callers of the 3 known ConsoleFunc ctors ===");
        long[] ctors = new long[]{4675536L, 4675696L, 4675856L}; // NoParam, String, Int
        Map<String, Address> fnEntry = new LinkedHashMap<>();

        for (long c : ctors) {
            Address ctorAddr = toAddr(c);
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(ctorAddr);
            while (refs.hasNext()) {
                Reference r = refs.next();
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                if (fn != null) {
                    fnEntry.put(fn.getEntryPoint().toString(), fn.getEntryPoint());
                }
            }
        }

        List<String> sortedKeys = new ArrayList<>(fnEntry.keySet());
        Collections.sort(sortedKeys);
        println("  total distinct wrapper functions found: " + sortedKeys.size());
        for (String k : sortedKeys) {
            println("    " + k);
        }

        println();
        println("=== Step 3: decompile first 3 wrapper functions for embedded strings ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            int n = 0;
            for (String k : sortedKeys) {
                if (n >= 3) break;
                Address entry = fnEntry.get(k);
                Function fn = getFunctionAt(entry);
                if (fn == null) continue;
                println("---- " + fn.getName() + " @ " + entry + " ----");
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed: " + (res != null ? res.getErrorMessage() : "null") + ")");
                }
                n++;
            }
        } finally {
            decomp.dispose();
        }
    }
}
