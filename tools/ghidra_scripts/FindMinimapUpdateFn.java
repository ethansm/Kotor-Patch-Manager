// Finds every reference to CSWGuiMainInterface+0x5d5c (the confirmed Steam
// offset of the BTN_MINIMAP control pointer, found in the constructor) OUTSIDE
// the constructor itself, to locate the runtime minimap update/resize method
// containing K2AspyrMapAspectFix's hook3. Also tests the uniform-shift
// hypothesis (GOG's MiniMapScaledWidthOffset/HeightOffset + 0x2a8) by
// searching for those candidate offsets directly.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Minimap Update Fn

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class FindMinimapUpdateFn extends GhidraScript {

    private static final String OFF_5D5C = "5c 5d 00 00";   // confirmed MiniMapControlOffset (Steam)
    private static final String OFF_5FFC = "fc 5f 00 00";   // hypothesis: MiniMapScaledWidthOffset + 0x2a8
    private static final String OFF_6000 = "00 60 00 00";   // hypothesis: MiniMapScaledHeightOffset + 0x2a8

    @Override
    public void run() throws Exception {
        Address start = currentProgram.getMinAddress();
        reportAndDecompile("0x5d5c (MiniMapControlOffset, confirmed)", findBytes(start, OFF_5D5C, 100));
        reportAndDecompile("0x5ffc (hypothesis: MiniMapScaledWidthOffset+0x2a8)", findBytes(start, OFF_5FFC, 100));
        reportAndDecompile("0x6000 (hypothesis: MiniMapScaledHeightOffset+0x2a8)", findBytes(start, OFF_6000, 100));
    }

    private void reportAndDecompile(String label, Address[] hits) throws Exception {
        println("=== " + label + ": " + hits.length + " hit(s) ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (Address hit : hits) {
                Function fn = getFunctionContaining(hit);
                println("  @ " + hit + "  fn=" + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses() : "none"));
                if (fn != null && fn.getEntryPoint().getOffset() != 0x00747210L) {
                    println("  ***** NON-CONSTRUCTOR HIT - dumping this function *****");
                    DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                    if (res != null && res.decompileCompleted()) {
                        println(res.getDecompiledFunction().getC());
                    }
                    println("  -- disasm --");
                    Listing listing = currentProgram.getListing();
                    InstructionIterator it = listing.getInstructions(fn.getBody(), true);
                    while (it.hasNext()) {
                        Instruction insn = it.next();
                        byte[] bytes = insn.getBytes();
                        StringBuilder hex = new StringBuilder();
                        for (byte b : bytes) hex.append(String.format("%02x ", b & 0xff));
                        println("    " + insn.getAddress() + "  " + String.format("%-30s", hex.toString()) + insn.toString());
                    }
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
