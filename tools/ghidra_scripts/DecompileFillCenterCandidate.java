// Buff Duration HUD UX-redesign U1 step 0 / U4: decompile the two
// candidate direct-fill functions found inside CSWGuiBorder::Draw's
// center-fill branch (fillstyle 0 -> FUN_00415a60, fillstyle 1 ->
// FUN_00415d20), to confirm which is FillCenter (GOG-typed signature:
// height, width, x, y, alpha, color) and see the actual draw primitive
// it issues (confirms/denies U4: rectangular wipe shape).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile FillCenter Candidate

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileFillCenterCandidate extends GhidraScript {

    private static final String[] TARGETS = { "00415a60", "00415d20" };

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
