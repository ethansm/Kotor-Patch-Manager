// @category KOTOR
// @menupath Tools.KOTOR.Dump 00569320 Full Disasm
// Item 1 hop 4: full raw disassembly of FUN_00569320 (CSWSObject heartbeat)
// to trace what [EBP-0x28] holds right before CALL 0x0051c350 at 00569522
// -- that becomes FUN_0051c350's own `this` (ECX), which it then
// dereferences at +0x4 to get FUN_00538e00's `this`.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class DumpFunctionDisasm_00569320 extends GhidraScript {
    private static final String ENTRY_HEX = "00569320";

    @Override
    public void run() throws Exception {
        Address entry = currentProgram.getAddressFactory().getAddress("0x" + ENTRY_HEX);
        Function fn = getFunctionAt(entry);
        if (fn == null) { println("No function at " + ENTRY_HEX); return; }
        println("Function: " + fn.getName() + " @ " + fn.getEntryPoint() + " end=" + fn.getBody().getMaxAddress());
        Listing listing = currentProgram.getListing();
        InstructionIterator it = listing.getInstructions(fn.getBody(), true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            println(insn.getAddress() + "  " + insn.toString());
        }
    }
}
