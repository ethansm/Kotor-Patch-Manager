// Buff Duration HUD UX-redesign U1 step 0: full decompile of
// FUN_007457e0 (Target 2's confirmed render hook point) to see the
// exact addressing pattern for its two virtual calls on the icon
// sub-object at slotPtr+0xab0/+0x968 -- need to know whether that
// offset holds an EMBEDDED object (vtable ptr directly there) or a
// POINTER to a heap-allocated object (double dereference), since that
// changes how to find its concrete vtable address.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Icon Row Draw

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileIconRowDraw extends GhidraScript {

    private static final String TARGET = "007457e0";

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address addr = currentProgram.getAddressFactory().getAddress("0x" + TARGET);
            Function fn = getFunctionAt(addr);
            if (fn == null) {
                println("no function at " + TARGET);
                return;
            }
            println("Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                + " size=" + fn.getBody().getNumAddresses());
            DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
            if (res != null && res.decompileCompleted()) {
                println("-- Decompiled C --");
                println(res.getDecompiledFunction().getC());
            } else {
                println("decompile failed: " + (res != null ? res.getErrorMessage() : "null"));
            }
        } finally {
            decomp.dispose();
        }
    }
}
