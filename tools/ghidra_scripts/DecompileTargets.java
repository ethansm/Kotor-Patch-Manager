// Decompiles and disassembles a fixed list of candidate functions/addresses
// found while porting K2AspyrLoadingScreenLineFix from GOG to Steam Aspyr.
// Also lists any functions whose name contains a given substring (to catch
// RTTI-derived class method names).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Targets

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class DecompileTargets extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "008850b0",
        "0061bfe0",
        "0061a210",
        "00523870",
        "00621ba0",
        "00620250"
    };

    private static final String NAME_SUBSTR = "LoadScreen";

    @Override
    public void run() throws Exception {
        println("=== Functions with name containing '" + NAME_SUBSTR + "' ===");
        FunctionManager fm = currentProgram.getFunctionManager();
        for (Function fn : fm.getFunctions(true)) {
            if (fn.getName().contains(NAME_SUBSTR)) {
                println("  " + fn.getName() + " @ " + fn.getEntryPoint());
            }
        }

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGET_ENTRIES) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function defined at this exact address)");
                    fn = getFunctionContaining(addr);
                    if (fn != null) {
                        println("  containing function: " + fn.getName() + " @ " + fn.getEntryPoint());
                    } else {
                        continue;
                    }
                }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println("  -- Decompiled C --");
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed: " + (res != null ? res.getErrorMessage() : "null") + ")");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
