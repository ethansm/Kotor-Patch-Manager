// @category KOTOR
// @menupath Tools.KOTOR.Decompile World Timer Singleton Hop 2
//
// Continues item 1 (R3 residual): FUN_00538e00's `this` (param_1) reads
// fields up to offset 0x10050 -- a huge (~64KB+) object, almost certainly
// a single game-wide server/world singleton, not a per-creature struct.
// The open question is whether CSWSObject+0x4 (the field passed as
// FUN_0051c350's/FUN_00538e00's `this`) is a per-object cached copy of ONE
// global singleton pointer (ideal -- render hook can read the same global
// directly with zero per-object context) or something else. Decompile:
//   - FUN_00569320 (CSWSObject's heartbeat) in full, to see param_1[1]'s
//     real declared shape/usage in context.
//   - FUN_0053e830 (likely CSWSObject::Constructor per classes table) to
//     find where offset +0x4 gets written and what it's copied FROM.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileWorldTimerSingletonHop2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            String[] targets = {"00569320", "0053e830"};
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
