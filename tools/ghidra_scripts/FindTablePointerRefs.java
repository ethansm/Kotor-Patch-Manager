// The console-command table (0x9866fc-0x986854, 87 entries) has zero
// direct Ghidra references to its start address, meaning code doesn't
// embed 0x9866fc as an immediate operand directly. Scan all of memory
// for raw occurrences of the 4-byte LE value 0x009866FC (a pointer-to-
// the-table stored in some other global), and also disassemble any
// instruction operand matches.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Table Pointer Refs

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;

public class FindTablePointerRefs extends GhidraScript {

    @Override
    public void run() throws Exception {
        byte[] needle = new byte[]{(byte)0xFC, (byte)0x66, (byte)0x98, 0x00}; // 0x009866FC little-endian
        Address addr = currentProgram.getMinAddress();
        int count = 0;
        while (count < 100) {
            Address hit = find(addr, needle);
            if (hit == null) break;
            Function fn = getFunctionContaining(hit);
            Instruction insn = getInstructionContaining(hit);
            println("hit @ " + hit + "  fn=" + (fn != null ? fn.getName() + "@" + fn.getEntryPoint() : "none")
                + "  insn=" + (insn != null ? insn.getAddress() + ": " + insn.toString() : "none (data)"));
            count++;
            addr = hit.add(1);
        }
        println("total: " + count);
    }
}
