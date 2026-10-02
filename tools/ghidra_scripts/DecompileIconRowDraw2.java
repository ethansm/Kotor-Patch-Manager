// @category KOTOR
// @menupath Tools.KOTOR.Decompile Icon Row Draw 2 (item 4, hook 2)
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileIconRowDraw2 extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address addr = currentProgram.getAddressFactory().getAddress("0x007457e0");
            Function fn = getFunctionAt(addr);
            println("Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                + " size=" + fn.getBody().getNumAddresses());
            DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
            if (res != null && res.decompileCompleted()) {
                println(res.getDecompiledFunction().getC());
            } else {
                println("  (decompile failed)");
            }
        } finally {
            decomp.dispose();
        }
    }
}
