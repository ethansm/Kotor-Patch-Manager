// Buff Duration HUD UX-redesign U1 step 0: decompile CSWGuiBorder's Draw
// (Steam 0x00415080, vftable slot 4, matches GOG Draw's implied size 640
// almost exactly) and FillTile (Steam 0x00415300, vftable slot 3, matches
// GOG FillTile's implied size 1888 almost exactly) to find the internal
// non-virtual call to FillCenter (GOG-confirmed real function, not in the
// vtable -- must be called directly by one of these).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Border Draw FillTile

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileBorderDrawFillTile extends GhidraScript {

    private static final String[] TARGETS = { "00415080", "00415300" };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function)");
                    continue;
                }
                println("  " + fn.getName() + " @ " + fn.getEntryPoint() + " size=" + fn.getBody().getNumAddresses());
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  decompile failed: " + (res != null ? res.getErrorMessage() : "null"));
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
