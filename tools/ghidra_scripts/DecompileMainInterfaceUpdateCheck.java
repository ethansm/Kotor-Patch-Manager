// Buff Duration HUD Phase 3 continued: re-verify whether Steam 0x0074b3f0
// (recorded as CSWGuiMainInterface::Update, vtable slot 13) is actually
// CSWGuiMainInterface::Draw. Byte-delta math (0x0074ee90 - 0x0074b3f0 =
// 15008 = GOG UpdatePortraits - GOG Draw) suggested Draw, not Update.
// Decompile the function itself plus its known vtable slot 14 entry
// (whatever that currently resolves to) for cross-comparison.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile MainInterface Update Check

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileMainInterfaceUpdateCheck extends GhidraScript {

    private static final String[] TARGETS = {
        "0074b3f0",  // recorded as CSWGuiMainInterface::Update (vtable slot 13)
        "0074ee90"   // confirmed UpdatePortraits, called from 0074b3f0
    };

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
