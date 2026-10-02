// Buff Duration HUD Phase 3, R7 (2026-09-29 planning session, reused from
// R1's portrait-offset search): search every instruction in .text for a
// scalar operand equal to 0x706c (CSWGuiMainInterface's already-computed
// action-bar fill-ratio field, written by vtbl[14]/FUN_0074c320 -- see
// lessons id 25/functions table). Looking for the READER of this field,
// which is very likely inside or adjacent to the actual (non-virtual)
// Draw() function -- R7's target drawing primitive.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Portrait Slot Refs

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSetView;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.scalar.Scalar;

import java.util.LinkedHashMap;
import java.util.Map;

public class FindPortraitSlotRefs extends GhidraScript {

    private static final long[] TARGETS = { 0xab0L, 0x968L };

    @Override
    public void run() throws Exception {
        MemoryBlock text = null;
        for (MemoryBlock b : currentProgram.getMemory().getBlocks()) {
            if (b.getName().equals(".text")) { text = b; break; }
        }
        if (text == null) { println("no .text block"); return; }

        Listing listing = currentProgram.getListing();
        InstructionIterator it = listing.getInstructions(text.getStart(), true);
        Map<String, Integer> hitsByFunc = new LinkedHashMap<>();
        int total = 0;
        while (it.hasNext()) {
            Instruction insn = it.next();
            if (insn.getAddress().compareTo(text.getEnd()) > 0) break;
            int n = insn.getNumOperands();
            for (int i = 0; i < n; i++) {
                Object[] objs = insn.getOpObjects(i);
                for (Object o : objs) {
                    if (o instanceof Scalar) {
                        Scalar s = (Scalar) o;
                        for (long TARGET : TARGETS) {
                            if (s.getUnsignedValue() == TARGET || s.getSignedValue() == TARGET) {
                                Function fn = getFunctionContaining(insn.getAddress());
                                String key = fn == null ? "???" : (fn.getName() + " @ " + fn.getEntryPoint());
                                println("HIT 0x" + Long.toHexString(TARGET) + "  " + insn.getAddress() + "  " + insn.toString() + "   in " + key);
                                hitsByFunc.merge(key, 1, Integer::sum);
                                total++;
                            }
                        }
                    }
                }
            }
        }
        println("=== summary: " + total + " hits across " + hitsByFunc.size() + " functions ===");
        for (Map.Entry<String, Integer> e : hitsByFunc.entrySet()) {
            println("  " + e.getValue() + "x  " + e.getKey());
        }
    }
}
