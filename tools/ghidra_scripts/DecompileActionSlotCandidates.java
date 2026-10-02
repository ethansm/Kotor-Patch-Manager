// Buff Duration HUD Item 5 / Phase A.5: decompile the mid-size candidate
// functions found by FindActionSlotArrayRefs.java to identify which one
// dispatches to the CSWGuiMainInterfaceAction slot's Show/Update/Draw/
// HitCheckMouse/SetIcon virtuals (per GOG's method table), and to find
// the actual dynamic assignment/SetIcon call site.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile ActionSlot Candidates

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileActionSlotCandidates extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "0074ad90",
        "0074c320",
        "0074cc90",
        "0074cfe0",
        "00751750"
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
