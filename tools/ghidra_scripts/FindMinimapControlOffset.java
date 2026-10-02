// Finds the real Steam-build offset where CSWGuiMainInterface's constructor
// stores the BTN_MINIMAP control pointer (analogous to how LBL_Map was
// found at CSWGuiInGameMap+0x68), then searches the whole binary for other
// code referencing that same offset (candidates for the minimap
// resize/update method containing K2AspyrMapAspectFix's hook3).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Minimap Control Offset

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class FindMinimapControlOffset extends GhidraScript {

    @Override
    public void run() throws Exception {
        println("=== Disasm context around BTN_MINIMAP ref (0x00747f3f) in CSWGuiMainInterface ctor ===");
        Listing listing = currentProgram.getListing();
        Address ctxStart = currentProgram.getAddressFactory().getAddress("0x00747ee0");
        Address ctxEnd = currentProgram.getAddressFactory().getAddress("0x00747fb0");
        InstructionIterator it = listing.getInstructions(ctxStart, true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            if (insn.getAddress().compareTo(ctxEnd) > 0) break;
            byte[] bytes = insn.getBytes();
            StringBuilder hex = new StringBuilder();
            for (byte b : bytes) hex.append(String.format("%02x ", b & 0xff));
            println("  " + insn.getAddress() + "  " + String.format("%-30s", hex.toString()) + insn.toString());
        }

        println("");
        println("=== Same window around LBL_MAP ref (0x00747eba) for comparison ===");
        ctxStart = currentProgram.getAddressFactory().getAddress("0x00747e80");
        ctxEnd = currentProgram.getAddressFactory().getAddress("0x00747f10");
        it = listing.getInstructions(ctxStart, true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            if (insn.getAddress().compareTo(ctxEnd) > 0) break;
            byte[] bytes = insn.getBytes();
            StringBuilder hex = new StringBuilder();
            for (byte b : bytes) hex.append(String.format("%02x ", b & 0xff));
            println("  " + insn.getAddress() + "  " + String.format("%-30s", hex.toString()) + insn.toString());
        }
    }
}
