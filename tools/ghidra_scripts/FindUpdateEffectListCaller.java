// R3: find caller(s) of confirmed UpdateEffectList (00544ad0) to see where
// the (day, timeOfDay) current-game-time pair passed as param_2/param_3
// comes from.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find UpdateEffectList Caller

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;

import java.util.LinkedHashSet;
import java.util.Set;

public class FindUpdateEffectListCaller extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address target = currentProgram.getAddressFactory().getAddress("0x00544ad0");
        ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(target);
        Set<Address> callers = new LinkedHashSet<>();
        println("=== Callers of UpdateEffectList (00544ad0) ===");
        while (refs.hasNext()) {
            Reference r = refs.next();
            Function fn = getFunctionContaining(r.getFromAddress());
            if (fn != null) {
                callers.add(fn.getEntryPoint());
                println("  xref from " + r.getFromAddress() + " in " + fn.getName() + " @ " + fn.getEntryPoint());
            } else {
                println("  xref from " + r.getFromAddress() + " -- no containing function");
            }
        }

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (Address entry : callers) {
                Function fn = getFunctionAt(entry);
                println("");
                println("---- CALLER " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses() + " ----");
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
