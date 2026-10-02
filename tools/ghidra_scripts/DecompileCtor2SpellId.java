// @category KOTOR
// @menupath Tools.KOTOR.Decompile Ctor2 SpellId Check (item 3, hop 2)
// Item 3: does CGameEffect::Constructor_2 (copy-ctor, 0x005e4ac0) copy
// offset +0x1c (CGameEffect_SpellIdOffset) from its source effect?
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileCtor2SpellId extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            Address addr = currentProgram.getAddressFactory().getAddress("0x005e4ac0");
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
