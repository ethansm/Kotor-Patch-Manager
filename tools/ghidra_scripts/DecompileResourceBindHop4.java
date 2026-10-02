// Final confirmation hop: FUN_004da3f0, the literal-resref-string load
// call reached from FUN_004da820's param_2 guard (item 1's terminal entry
// point candidate).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Resource Bind Hop4

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileResourceBindHop4 extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "004da3f0"
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : DECOMPILE_TARGETS) {
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
