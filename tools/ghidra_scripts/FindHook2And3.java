// Confirms a clean hook2 anchor immediately after FUN_00894320's yScale
// computation, and searches CSWGuiMainInterface's 3 largest vtable methods
// for the minimap struct offsets (0x5ab4/0x5d54/0x5d58) referenced by
// K2AspyrMapAspectFix.cpp's preserveMiniMapAspect, to locate hook3.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Hook2 And 3

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class FindHook2And3 extends GhidraScript {

    private static final String[] MAIN_INTERFACE_TARGETS = {
        "0074b3f0",
        "0074c320",
        "007469f0"
    };

    @Override
    public void run() throws Exception {
        println("=== Hook2 candidate: instructions right after FUN_00894320's yScale compute ===");
        Address start = currentProgram.getAddressFactory().getAddress("0x00894432");
        Address end = currentProgram.getAddressFactory().getAddress("0x00894470");
        Listing listing = currentProgram.getListing();
        InstructionIterator it = listing.getInstructions(start, true);
        while (it.hasNext()) {
            Instruction insn = it.next();
            if (insn.getAddress().compareTo(end) > 0) break;
            byte[] bytes = insn.getBytes();
            StringBuilder hex = new StringBuilder();
            for (byte b : bytes) hex.append(String.format("%02x ", b & 0xff));
            println("  " + insn.getAddress() + "  " + String.format("%-30s", hex.toString()) + insn.toString());
        }

        println("");
        println("=== CSWGuiMainInterface candidates: searching for minimap offsets (0x5ab4/0x5d54/0x5d58) ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : MAIN_INTERFACE_TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("---- " + hex + " ----");
                if (fn == null) { println("  no function"); continue; }

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    String c = res.getDecompiledFunction().getC();
                    boolean has5ab4 = c.contains("0x5ab4") || c.contains("0x5AB4") || c.contains("23220");
                    boolean has5d54 = c.contains("0x5d54") || c.contains("0x5D54") || c.contains("23892");
                    boolean has5d58 = c.contains("0x5d58") || c.contains("0x5D58") || c.contains("23896");
                    boolean hasA8 = c.contains("-0xa8") || c.contains("0xffffff58");
                    println("  contains 0x5ab4-ish: " + has5ab4 + "  0x5d54-ish: " + has5d54
                        + "  0x5d58-ish: " + has5d58 + "  -0xa8-ish: " + hasA8);
                    if (has5ab4 || has5d54 || has5d58) {
                        println("  ***** STRONG CANDIDATE - dumping full decompile *****");
                        println(c);
                    } else {
                        println("  (no match, first 500 chars for manual check)");
                        println(c.substring(0, Math.min(500, c.length())));
                    }
                } else {
                    println("  (decompile failed)");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
