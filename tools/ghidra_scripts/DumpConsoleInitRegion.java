// Raw disassembly dump of the region that calls the console-command
// wrapper functions (~0x986700-0x986900), regardless of function
// boundaries, to find the master console-init sequence and what follows
// its last wrapper call.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump Console Init Region

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Function;

public class DumpConsoleInitRegion extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address start = toAddr(0x00986600L);
        Address end = toAddr(0x00986a00L);
        InstructionIterator it = currentProgram.getListing().getInstructions(start, true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            if (insn.getAddress().compareTo(end) > 0) break;
            Function fn = getFunctionContaining(insn.getAddress());
            String fnTag = fn != null ? " [" + fn.getName() + "]" : " [NOFUNC]";
            println(insn.getAddress() + ": " + insn.toString() + fnTag);
        }
    }
}
