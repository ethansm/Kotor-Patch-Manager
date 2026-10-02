// Phase B step 2: decompile CSWGuiBorder_Load_FillSetter (0x00414840) in
// full -- does it accept an arbitrary manufactured resref, or does hook 2
// need to replicate release-old + FUN_0047eb60 + store-new manually?
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile FillSetter

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileFillSetter extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String a : new String[]{"0x00414840", "0x0047eb60"}) {
                Address entry = currentProgram.getAddressFactory().getAddress(a);
                Function fn = getFunctionAt(entry);
                println("");
                if (fn == null) { println("---- " + a + " NO FUNCTION ----"); continue; }
                println("---- DECOMPILE " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses() + " ----");
                DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
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
