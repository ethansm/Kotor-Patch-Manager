// Phase A.6, sub-question B: does CItemPropertyApplierRemover/
// CSWSItemPropertyHandler (the +0x5c sibling of CSWSEffectListHandler,
// constructed alongside it in FUN_0051d2f0) call CGameEffect::SetObjectID
// (0x005e4f00) or ::SetString (0x005e4f60) with the granting item's own
// handle when building/applying an item-ability effect?
//
// Step 1: decompile FUN_0051d2f0 to find the vftable pointer(s) written
// at owning-object+0x5c (parallel to +0x58's CSWSEffectListHandler
// pattern: base vtable written first, then overwritten with derived).
// Step 2: walk that vtable's slots 0-3 (dtor/Init/Apply/Remove, same
// shape as CSWSEffectListHandler) to resolve real Steam addresses.
// Step 3: decompile slots 1 (Init) and 2 (Apply) fully, and grep the
// decompile text for calls to 005e4f00/005e4f60 or "SetObjectID"/
// "SetString"-shaped call patterns.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Item Property Handler

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.scalar.Scalar;

public class DecompileItemPropertyHandler extends GhidraScript {

    private DecompInterface decomp;

    private String decompileText(Address entry) {
        Function fn = getFunctionAt(entry);
        if (fn == null) return "  (no function at " + entry + ")";
        DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
        if (res == null || !res.decompileCompleted()) return "  (decompile failed for " + entry + ")";
        return res.getDecompiledFunction().getC();
    }

    private void dumpRawDisasm(Address entry, int maxInsns) {
        Function fn = getFunctionAt(entry);
        if (fn == null) { println("  (no function to disasm at " + entry + ")"); return; }
        InstructionIterator it = currentProgram.getListing().getInstructions(fn.getBody(), true);
        int n = 0;
        while (it.hasNext() && n < maxInsns) {
            Instruction ins = it.next();
            println("    " + ins.getAddress() + ": " + ins.toString());
            n++;
        }
    }

    @Override
    public void run() throws Exception {
        decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        Address ctorAddr = currentProgram.getAddressFactory().getAddress("0x0051d2f0");
        println("=== FUN_0051d2f0 (owning-object constructor) decompile ===");
        println(decompileText(ctorAddr));

        println("");
        println("=== FUN_0051d2f0 raw disassembly (looking for vtable stores at offset 0x5c) ===");
        dumpRawDisasm(ctorAddr, 400);

        decomp.dispose();
    }
}
