// Finds callers of the first and last tiny console-command wrapper
// functions (0x983ea0 and 0x984500), to locate the master console-init
// routine, then dumps its disassembly to find where the last wrapper
// call happens and what follows.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Console Init Caller

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class FindConsoleInitCaller extends GhidraScript {

    @Override
    public void run() throws Exception {
        long[] wrappers = new long[]{0x00983ea0L, 0x00984500L, 0x00983ed0L, 0x00984440L};
        for (long w : wrappers) {
            Address wa = toAddr(w);
            println("=== callers of wrapper @ " + wa + " ===");
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(wa);
            while (refs.hasNext()) {
                Reference r = refs.next();
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                println("  called from " + from + " in fn=" + (fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none"));
            }
        }
    }
}
