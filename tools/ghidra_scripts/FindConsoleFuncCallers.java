// Finds every caller of the 3 known ConsoleFunc constructor addresses,
// groups by containing function, to locate the game's own built-in
// console-command registration routine (the function that calls these
// constructors many times in a row) -- that's the real anchor for
// AdditionalConsoleCommands hook1, per its README ("we patch in shortly
// after the existing console commands are initialized").
//
// @category KOTOR
// @menupath Tools.KOTOR.Find ConsoleFunc Callers

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

import java.util.*;

public class FindConsoleFuncCallers extends GhidraScript {

    @Override
    public void run() throws Exception {
        long[] ctors = new long[]{4675536L, 4675696L, 4675856L}; // NoParam, String, Int
        Map<String, Integer> counts = new LinkedHashMap<>();
        Map<String, Address> fnEntry = new LinkedHashMap<>();

        for (long c : ctors) {
            Address ctorAddr = toAddr(c);
            println("=== xrefs to ctor @ " + ctorAddr + " ===");
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(ctorAddr);
            int n = 0;
            while (refs.hasNext()) {
                Reference r = refs.next();
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                String key = fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none@" + from;
                counts.merge(key, 1, Integer::sum);
                if (fn != null) fnEntry.put(key, fn.getEntryPoint());
                n++;
            }
            println("  total xrefs: " + n);
        }

        println();
        println("=== functions calling ConsoleFunc ctors, by call count ===");
        counts.entrySet().stream()
            .sorted((a, b) -> b.getValue() - a.getValue())
            .forEach(e -> println("  " + e.getValue() + "x  " + e.getKey()));
    }
}
