// @category KOTOR
// @menupath Tools.KOTOR.Decompile World Timer Singleton (item 1, R3 residual)
//
// Closes the R3 residual for Buff Duration HUD: FUN_0051c350(param_1[1])
// calls FUN_00538e00(...) to resolve the worldTimerObj `this` pointer that
// WorldTimer::GetCurrentTime (0x0051ad20) needs. Decompile FUN_00538e00 to
// determine whether it's a fixed global singleton lookup (ideal) or a
// handle/id-keyed lookup (extra design constraint). If it's a lookup,
// this script also dumps FUN_0051c350 itself (full) for context on where
// param_1[1] comes from, and follows one more hop if a plausible
// candidate shows up in the decompile output.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileWorldTimerSingleton extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            String[] targets = {"0051c350", "00538e00"};
            for (String hex : targets) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) { println("  (no function)"); continue; }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses()
                    + " params=" + fn.getParameterCount());
                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
