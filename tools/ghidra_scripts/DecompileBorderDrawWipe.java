// @category KOTOR
// @menupath Tools.KOTOR.Decompile Border Draw for Wipe Mechanism (item 4)
// Determines whether CSWGuiBorder::Draw's FillCenter/FillTile dispatch
// reads a LIVE dimension field each call (allowing a simple shrink-in-place
// wipe) or needs external rect parameters (needing a separate draw call).
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileBorderDrawWipe extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            String[] targets = {"00415300", "00415d20"}; // Draw, FillCenter
            for (String hex : targets) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) { println("  (no function)"); continue; }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());
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
