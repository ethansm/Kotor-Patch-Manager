// Buff Duration HUD UX-redesign session, U1 step 0: find the concrete
// vtable address of the icon-widget sub-object embedded at
// slotPtr+0xab0 / slotPtr+0x968 inside CSWGuiMainInterface's portrait
// slot struct. The vtable pointer is almost certainly written inside
// CSWGuiMainInterface's own constructor (FUN_00747210, size 13027).
// Scan that function's disassembly for any instruction writing a
// constant/address to a [reg + 0xab0] or [reg + 0x968]-style operand
// (a vtable-pointer store looks like MOV [reg+off], imm32 where imm32
// is itself a data address holding function pointers).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Icon Widget Vtable Write

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.scalar.Scalar;

public class FindIconWidgetVtableWrite extends GhidraScript {

    private static final String CTOR_ADDR = "00747210";
    private static final long[] TARGET_OFFSETS = { 0xab0L, 0x968L };

    @Override
    public void run() throws Exception {
        Address ctorAddr = currentProgram.getAddressFactory().getAddress("0x" + CTOR_ADDR);
        Function fn = getFunctionAt(ctorAddr);
        if (fn == null) {
            println("No function at " + CTOR_ADDR);
            return;
        }
        println("Scanning " + fn.getName() + " @ " + fn.getEntryPoint()
            + " size=" + fn.getBody().getNumAddresses());

        AddressSetView body = fn.getBody();
        InstructionIterator it = currentProgram.getListing().getInstructions(body, true);
        int hits = 0;
        while (it.hasNext()) {
            Instruction insn = it.next();
            int numOps = insn.getNumOperands();
            for (int op = 0; op < numOps; op++) {
                Object[] objs = insn.getOpObjects(op);
                for (Object o : objs) {
                    if (o instanceof Scalar) {
                        long val = ((Scalar) o).getUnsignedValue();
                        for (long target : TARGET_OFFSETS) {
                            if (val == target) {
                                hits++;
                                println("");
                                println("HIT (offset 0x" + Long.toHexString(target) + ") @ "
                                    + insn.getAddress() + " : " + insn.toString());
                                // print a window of context: 4 instructions before, 6 after
                                printContext(insn, 4, 6);
                            }
                        }
                    }
                }
            }
        }
        println("");
        println("Total hits: " + hits);
    }

    private void printContext(Instruction center, int before, int after) throws Exception {
        Instruction cur = center;
        Instruction[] prevArr = new Instruction[before];
        Instruction walker = center;
        for (int i = before - 1; i >= 0; i--) {
            walker = walker.getPrevious();
            prevArr[i] = walker;
        }
        for (int i = 0; i < before; i++) {
            if (prevArr[i] != null) {
                println("    " + prevArr[i].getAddress() + " : " + prevArr[i].toString());
            }
        }
        println(">>> " + center.getAddress() + " : " + center.toString());
        walker = center;
        for (int i = 0; i < after; i++) {
            walker = walker.getNext();
            if (walker == null) break;
            println("    " + walker.getAddress() + " : " + walker.toString());
        }
    }
}
