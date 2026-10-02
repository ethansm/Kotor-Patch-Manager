// @category KOTOR
// @menupath Tools.KOTOR.Decompile Spell ID Check (item 3)
// Item 3: cheap sanity check -- does CGameEffect+0x1c (CGameEffect_SpellIdOffset,
// found via HasSpellEffectApplied) really hold a spells.2da row-index-space
// value? Decompile FUN_0059de30 (compound-effect decomposition helper that
// calls ApplyEffect/0x00544210 repeatedly) looking for where +0x1c gets SET
// on a freshly-built effect.
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileSpellIdCheck extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            String[] targets = {"0059de30"};
            for (String hex : targets) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) { println("  (no function)"); continue; }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());
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
