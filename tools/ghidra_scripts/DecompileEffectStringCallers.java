// Buff Duration HUD Item 5, continued (user chose to keep chasing this
// lead): decompile the two CGameEffect::SetString (0x005e4f60) callers
// that invoke it more than once each -- FUN_0065a170 and FUN_00671e20 --
// to see whether an item-property-triggered effect stores a resref or
// other item-identifying string on itself.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile EffectString Callers

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileEffectStringCallers extends GhidraScript {

    private static final String[] TARGET_ENTRIES = {
        "0065a170",
        "00671e20"
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGET_ENTRIES) {
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

                DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
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
