// R3/R5 confirmation: UpdateEffectList (00544ad0 candidate) for the game-time
// source, HasSpellEffectApplied (005476c0 candidate) for spell-ID mapping.
// Also 00544640/00544790/00544990/00544a10 for the RemoveEffect family (context).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile R3 R5 Candidates

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileR3R5Candidates extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "00544ad0",  // UpdateEffectList candidate (R3)
        "005476c0",  // HasSpellEffectApplied candidate (R5)
        "00544640",  // RemoveEffect candidate (context)
        "00544790",  // RemoveEffectById candidate (context)
        "00544990",  // RemoveEffectByCreator candidate part A (context)
        "00544a10"   // RemoveEffectByCreator candidate part B (context)
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : DECOMPILE_TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function at this exact address)");
                    continue;
                }
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
