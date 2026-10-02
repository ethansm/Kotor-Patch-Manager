// @category KOTOR
// @menupath Tools.KOTOR.Decompile Primary Ctor SpellId Check (item 3, hop 3)
// Item 3: does CGameEffect's PRIMARY constructor (0x005e4920, previously
// only POSITIONAL-MATCH, never decompiled) take a spellId-shaped explicit
// parameter that gets written to +0x1c?
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompilePrimaryCtorSpellId extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address addr = currentProgram.getAddressFactory().getAddress("0x005e4920");
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
