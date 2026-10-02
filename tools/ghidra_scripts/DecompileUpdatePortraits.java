// @category KOTOR
// @menupath Tools.KOTOR.Decompile UpdatePortraits (item 2)
// Item 2: multi-slot creature resolution. Full decompile of
// CSWGuiMainInterface::UpdatePortraits (0x0074ee90) to find where it
// computes the display slot index and calls FUN_0077d800
// (RosterHandleToCreature) per position, looking for a clean splice point.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileUpdatePortraits extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address addr = currentProgram.getAddressFactory().getAddress("0x0074ee90");
            Function fn = getFunctionAt(addr);
            println("Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                + " size=" + fn.getBody().getNumAddresses());
            DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
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
