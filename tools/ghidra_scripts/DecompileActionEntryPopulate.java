// Buff Duration HUD Item 5 / Force Power Hotbar Step 0: FUN_00751750
// revealed a per-slot "action entry" array (stride 0x3c) with a
// callable function pointer at +0xc, an object/spell-id-shaped field at
// +0x1c used with FUN_0073f4d0, and a type bitfield at +0x30 (bits 1-4,
// values 1-6 map to distinct STRREF constants). FUN_0074cfe0 populates
// this array per-slot via FUN_0077e650(slotIndex, arrayFieldPtr). This
// script decompiles FUN_0077e650 (the populator) and FUN_007469a0 (a
// small accessor referenced nearby) to find where the type
// discrimination (spell vs item) and icon source are set.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile ActionEntry Populate

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileActionEntryPopulate extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "0077e650",
        "007469a0",
        "0073f4d0"
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
