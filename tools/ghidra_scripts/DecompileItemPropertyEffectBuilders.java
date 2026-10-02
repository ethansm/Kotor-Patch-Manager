// Phase A.6 sub-question B, final step. Cross-ref found 5 strong
// candidates in the item-property-handler address range that call
// CGameEffect::SetObjectID (item back-reference?) or ::SetExpiryTime
// (duration -- directly relevant regardless): FUN_005a7610, FUN_005a8860,
// FUN_005a9520 (SetObjectID) and FUN_005a4bb0, FUN_005a4d70
// (SetExpiryTime). Decompile all 5 in full.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile ItemProperty Effect Builders

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileItemPropertyEffectBuilders extends GhidraScript {

    @Override
    public void run() throws Exception {
        String[] addrs = {"0x005a7610", "0x005a8860", "0x005a9520", "0x005a4bb0", "0x005a4d70"};
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String a : addrs) {
                Address entry = currentProgram.getAddressFactory().getAddress(a);
                Function fn = getFunctionAt(entry);
                println("");
                if (fn == null) { println("---- " + a + " -- NO FUNCTION HERE ----"); continue; }
                println("---- DECOMPILE " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses() + " params=" + fn.getParameterCount() + " ----");
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
