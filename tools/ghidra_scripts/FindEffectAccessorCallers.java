// R3/R5: find Steam callers of the already-confirmed non-virtual CGameEffect
// accessors (CopyEffect 005e4fd0, SetExpiryTime 005e4f80, GetExpiryTime 005e4fa0).
// These are strong anchor candidates for CSWSObject::ApplyEffect/UpdateEffectList
// since GOG's ApplyEffect/UpdateEffectList almost certainly call the equivalent
// CGameEffect copy/expiry-set accessors directly (non-virtual call, real xref).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Effect Accessor Callers

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;

import java.util.LinkedHashSet;
import java.util.Set;

public class FindEffectAccessorCallers extends GhidraScript {

    private static final String[] TARGETS = {
        "005e4fd0",  // CopyEffect
        "005e4f80",  // SetExpiryTime
        "005e4fa0",  // GetExpiryTime
        "005e5510"   // LoadGameEffect
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                println("");
                println("==== CALLERS OF " + hex + " ====");
                ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(addr);
                Set<Address> callerEntries = new LinkedHashSet<>();
                while (refs.hasNext()) {
                    Reference r = refs.next();
                    Address from = r.getFromAddress();
                    Function fn = getFunctionContaining(from);
                    if (fn != null) {
                        callerEntries.add(fn.getEntryPoint());
                    } else {
                        println("  (xref from " + from + " -- no containing function)");
                    }
                }
                println("  " + callerEntries.size() + " unique caller function(s)");
                for (Address entry : callerEntries) {
                    Function fn = getFunctionAt(entry);
                    println("  caller: " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses());
                }
            }

            // Decompile every unique caller found across all 4 targets, capped.
            Set<Address> allCallers = new LinkedHashSet<>();
            for (String hex : TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(addr);
                while (refs.hasNext()) {
                    Reference r = refs.next();
                    Function fn = getFunctionContaining(r.getFromAddress());
                    if (fn != null) allCallers.add(fn.getEntryPoint());
                }
            }
            println("");
            println("=== DECOMPILING " + allCallers.size() + " UNIQUE CALLER(S) ===");
            int count = 0;
            for (Address entry : allCallers) {
                if (count++ > 15) {
                    println("(cap reached, stopping)");
                    break;
                }
                Function fn = getFunctionAt(entry);
                println("");
                println("---- CALLER " + fn.getName() + " @ " + entry + " ----");
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
