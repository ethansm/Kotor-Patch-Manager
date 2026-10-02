// Generic reusable caller-finder -- edit TARGET_ADDR (a single function
// entry-point hex address). Finds every unique containing function that
// calls it (via getReferencesTo on the function's entry point), then
// decompiles each unique caller so the call-site context (which
// args/indices are passed, how the return value is used) can be read
// without a second script run. Unlike FindConsoleFuncCallers.java (which
// only counts/groups by caller name), this one also dumps full decompiled
// C for each caller -- more useful when there are few callers and the
// point is call-site *content*, not just call-site *count*.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Callers And Decompile

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

import java.util.*;

public class FindCallersAndDecompile extends GhidraScript {

    private static final String TARGET_ADDR = "00594740";
    private static final int MAX_CALLERS = 5;

    @Override
    public void run() throws Exception {
        Address target = currentProgram.getAddressFactory().getAddress("0x" + TARGET_ADDR);
        println("=== xrefs to " + target + " ===");

        Set<Address> callerEntries = new LinkedHashSet<>();
        ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(target);
        int total = 0;
        while (refs.hasNext()) {
            Reference r = refs.next();
            Address from = r.getFromAddress();
            Function fn = getFunctionContaining(from);
            if (fn != null) {
                callerEntries.add(fn.getEntryPoint());
            } else {
                println("  (xref from " + from + " has no containing function)");
            }
            total++;
        }
        println("total xrefs: " + total + "  unique callers: " + callerEntries.size());
        println();

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            int shown = 0;
            for (Address entry : callerEntries) {
                if (shown >= MAX_CALLERS) {
                    println("... (capped at " + MAX_CALLERS + " callers, " + (callerEntries.size() - shown) + " more not shown)");
                    break;
                }
                Function fn = getFunctionAt(entry);
                println("==== caller: " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses() + " ====");
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }
                println("");
                shown++;
            }
        } finally {
            decomp.dispose();
        }
    }
}
