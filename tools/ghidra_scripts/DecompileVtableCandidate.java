// Decompiles + disassembles a fixed candidate list -- generic reusable
// template (edit TARGETS[] per investigation). Current contents: Buff
// Duration HUD Phase 3, R1/R2 (2026-09-29 planning session) -- full
// re-decompile of FUN_0074b3f0 (CSWGuiMainInterface::Update, vtbl[13]),
// looking specifically for portrait-slot-to-owning-creature binding logic
// not previously read closely (Phase 2 only examined it for the
// FUN_00745770/icon-draw call chain). GOG names a separate 0-arg
// "UpdatePortraits" function (address 5375504) sitting well after Update
// (5364384) in GOG's source order, past a long run of action-bar-related
// helpers -- unclear yet whether it's a distinct Steam function or inlined
// into Update itself; this re-decompile checks Update's own body first
// since it's already known to directly touch all 4 portrait slots.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Vtable Candidate

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class DecompileVtableCandidate extends GhidraScript {

    private static final String[] TARGETS = {
        "0053e830"
    };

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("==== " + hex + " ====");
                if (fn == null) { println("  no function"); continue; }
                println("  Function: " + fn.getName() + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }

                println("  -- disasm --");
                Listing listing = currentProgram.getListing();
                InstructionIterator insnIt = listing.getInstructions(fn.getBody(), true);
                while (insnIt.hasNext()) {
                    Instruction insn = insnIt.next();
                    byte[] bytes = insn.getBytes();
                    StringBuilder hexStr = new StringBuilder();
                    for (byte b : bytes) hexStr.append(String.format("%02x ", b & 0xff));
                    println("    " + insn.getAddress() + "  " + String.format("%-30s", hexStr.toString()) + insn.toString());
                }
                println("");
            }
        } finally {
            decomp.dispose();
        }
    }
}
