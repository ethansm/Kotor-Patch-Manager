// Buff Duration HUD Item 5 / Force Power Hotbar Step 0: FUN_0077e650
// dispatches per action-bar category index (0-5) to one of 4 distinct
// populator functions. Decompile all 4 to see how each fills the
// per-entry struct (stride 0x3c: +0x1c id-like field used with
// FUN_0073f4d0/FUN_0053dfb0, +0xc callable fn ptr, +0x30 type bitfield)
// -- this is the concrete answer to "how does the vanilla quickbar
// represent a spell vs an item internally."
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Category Populators

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileCategoryPopulators extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "00779510",
        "0077c510",
        "0077a9e0",
        "0077aeb0"
    };

    @Override
    public void run() throws Exception {
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
                    continue;
                }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
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
