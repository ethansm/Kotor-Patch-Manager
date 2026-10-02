// Verifies the corrected hook2 candidate (FUN_00895cc0, exact 12-byte GOG
// match at 0x00895e6d) and hook3 candidate (FUN_00750790, exact 6-byte GOG
// match at 0x007508aa) by full decompile+disasm, checking frame offsets
// against K2AspyrMapAspectFix.cpp's hardcoded constants.
//
// @category KOTOR
// @menupath Tools.KOTOR.Verify Hook2 And 3

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class VerifyHook2And3 extends GhidraScript {

    private static final String[] TARGETS = { "00895cc0", "00750790" };

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
                println("  size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }

                println("  -- disasm --");
                Listing listing = currentProgram.getListing();
                InstructionIterator it = listing.getInstructions(fn.getBody(), true);
                while (it.hasNext()) {
                    Instruction insn = it.next();
                    byte[] bytes = insn.getBytes();
                    StringBuilder hex2 = new StringBuilder();
                    for (byte b : bytes) hex2.append(String.format("%02x ", b & 0xff));
                    println("    " + insn.getAddress() + "  " + String.format("%-30s", hex2.toString()) + insn.toString());
                }
                println("");
            }
        } finally {
            decomp.dispose();
        }
    }
}
