// Decompiles the 9 functions whose single-occurrence movzx-byte-from-.data
// read (out of FindConsoleHistoryFlag2's 65 .data-targeting hits) makes
// them the most plausible candidates for AdditionalConsoleCommands hook2
// (console-history-save flag) -- a flag checked in exactly one place is a
// much better match for a single boolean "should I save history" guard
// than an address referenced from many unrelated functions.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Hook2 Candidates

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileHook2Candidates extends GhidraScript {

    private static final String[] TARGETS = {
        "004010e0", // reads [00a13c20]
        "004204f0", // reads [00a1ba48]
        "00426270", // reads [00a1bb9c]
        "00445310", // reads [00a309bc]
        "004752a0", // reads [00a30828]
        "0047ea60", // reads [00a32a40]
        "004b07d0", // reads [009f6584] and [009f5b2c]
        "007b12c0", // reads [00a7fcc8]
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("==== " + hex + " ====");
                if (fn == null) {
                    println("  (no function at this address)");
                    continue;
                }
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
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
