// Searches the whole binary for the raw little-endian disp32 bytes of the
// minimap struct offsets referenced by K2AspyrMapAspectFix.cpp's
// preserveMiniMapAspect (0x5ab4, 0x5d54, 0x5d58), to find hook3's
// containing function directly rather than guessing which vtable method.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Minimap Offsets

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class FindMinimapOffsets extends GhidraScript {

    // disp32 LE encoding, preceded by wildcard modrm byte (2 bytes before) and
    // opcode (1 byte before) -- just search the raw disp32 itself, cheap and
    // very distinctive at 4 bytes matching a big/unusual offset.
    private static final String OFF_5AB4 = "b4 5a 00 00";
    private static final String OFF_5D54 = "54 5d 00 00";
    private static final String OFF_5D58 = "58 5d 00 00";

    @Override
    public void run() throws Exception {
        Address start = currentProgram.getMinAddress();
        report("0x5ab4 (MiniMapControlOffset)", findBytes(start, OFF_5AB4, 100));
        report("0x5d54 (MiniMapScaledWidthOffset)", findBytes(start, OFF_5D54, 100));
        report("0x5d58 (MiniMapScaledHeightOffset)", findBytes(start, OFF_5D58, 100));
    }

    private void report(String label, Address[] hits) throws Exception {
        println("=== " + label + ": " + hits.length + " hit(s) ===");
        for (Address hit : hits) {
            // the disp32 bytes start ~2-3 bytes after the instruction start; look back a little
            Address insnStart = hit.subtract(3);
            Function fn = getFunctionContaining(hit);
            Listing listing = currentProgram.getListing();
            Instruction insn = listing.getInstructionContaining(hit);
            String insnStr = (insn != null) ? insn.toString() : "(no instruction here)";
            println("  @ " + hit + "  insn=[" + insnStr + "]  fn="
                + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint() + " size=" + fn.getBody().getNumAddresses() : "none"));
        }
    }
}
