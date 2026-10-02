// Buff Duration HUD Item 5 / Phase A.5: RTTI search for
// CSWGuiMainInterfaceAction came back 0 hits (panel-embedded, no RTTI,
// same class as CSWGuiMainInterfaceChar/Status). Falling back to
// positional clustering: the action-bar slot array is already known
// from 07_force_power_hotbar.md's Lesson 1 as `hudThis + 0x733c`,
// stride `0x750`. This script scans all instructions for an immediate
// operand equal to 0x733c (29500 decimal) to find where the main
// interface computes a slot pointer, so we can trace forward from there
// to the vtable calls (Show/Update/Draw/HitCheckMouse/SetIcon) GOG's
// method table already names.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find ActionSlotArray Refs

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.scalar.Scalar;

public class FindActionSlotArrayRefs extends GhidraScript {

    static final long TARGET_OFFSET = 0x733cL;
    static final long STRIDE = 0x750L;

    @Override
    public void run() throws Exception {
        Listing listing = currentProgram.getListing();
        InstructionIterator it = listing.getInstructions(true);
        int count = 0;
        while (it.hasNext() && !monitor.isCancelled()) {
            Instruction instr = it.next();
            int numOps = instr.getNumOperands();
            for (int i = 0; i < numOps; i++) {
                Object[] opObjs = instr.getOpObjects(i);
                for (Object o : opObjs) {
                    if (o instanceof Scalar) {
                        long v = ((Scalar) o).getSignedValue();
                        if (v == TARGET_OFFSET || v == (TARGET_OFFSET + STRIDE)
                                || v == (TARGET_OFFSET + 2 * STRIDE)
                                || v == (TARGET_OFFSET + 3 * STRIDE)) {
                            Address a = instr.getAddress();
                            Function f = getFunctionContaining(a);
                            String fname = (f != null) ? f.getName() + "@" + f.getEntryPoint() : "???";
                            println("  " + a + "  [" + fname + "]  " + instr.toString());
                            count++;
                        }
                    }
                }
            }
        }
        println("Total hits: " + count);
    }
}
