// Dumps full disassembly (address, bytes, mnemonic) for a given function
// entry point. Used to pin down exact instruction boundaries/bytes for
// porting K2AspyrLoadingScreenLineFix.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump Function Disasm

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.mem.Memory;

public class DumpFunctionDisasm_0074ee90 extends GhidraScript {

    private static final String ENTRY_HEX = "0074ee90";

    @Override
    public void run() throws Exception {
        Address entry = currentProgram.getAddressFactory().getAddress("0x" + ENTRY_HEX);
        Function fn = getFunctionAt(entry);
        if (fn == null) {
            println("No function at " + ENTRY_HEX);
            return;
        }
        println("Function: " + fn.getName() + " @ " + fn.getEntryPoint() + " end=" + fn.getBody().getMaxAddress());

        Listing listing = currentProgram.getListing();
        InstructionIterator it = listing.getInstructions(fn.getBody(), true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            byte[] bytes = insn.getBytes();
            StringBuilder hex = new StringBuilder();
            for (byte b : bytes) hex.append(String.format("%02x ", b & 0xff));
            println(insn.getAddress() + "  " + String.format("%-30s", hex.toString()) + insn.toString());
        }
    }
}
