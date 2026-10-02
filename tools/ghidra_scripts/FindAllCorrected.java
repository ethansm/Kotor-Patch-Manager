// findBytes() was discovered to be non-functional in this environment (0
// hits even for byte-confirmed patterns) -- find(Address, byte[]) works
// correctly instead. This redoes, with a proper findAll-via-find() loop,
// the searches that earlier wrongly concluded "0 hits" using the broken
// API: the 3 GOG hook exact-byte patterns, and the 3 GOG minimap offsets
// (0x5ab4/0x5d54/0x5d58), to see whether any of those earlier conclusions
// were wrong.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find All Corrected

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class FindAllCorrected extends GhidraScript {

    @Override
    public void run() throws Exception {
        // hook1 (already confirmed correct via manual read, sanity check the fixed API agrees)
        reportAll("hook1 exact bytes (C7 85 7C FF FF FF 00 00 00 00)",
            new byte[]{(byte)0xC7,(byte)0x85,0x7C,(byte)0xFF,(byte)0xFF,(byte)0xFF,0,0,0,0});

        // hook2 GOG exact bytes
        reportAll("hook2 GOG exact bytes (89 85 A0 FD FF FF 8B 85 A0 FD FF FF)",
            new byte[]{(byte)0x89,(byte)0x85,(byte)0xA0,(byte)0xFD,(byte)0xFF,(byte)0xFF,
                       (byte)0x8B,(byte)0x85,(byte)0xA0,(byte)0xFD,(byte)0xFF,(byte)0xFF});

        // hook3 GOG exact bytes
        reportAll("hook3 GOG exact bytes (89 45 8C 8B 45 8C)",
            new byte[]{(byte)0x89,0x45,(byte)0x8C,(byte)0x8B,0x45,(byte)0x8C});

        // minimap offsets, raw disp32 LE
        reportAll("0x5ab4 disp32", new byte[]{(byte)0xB4,0x5A,0,0});
        reportAll("0x5d54 disp32", new byte[]{0x54,0x5D,0,0});
        reportAll("0x5d58 disp32", new byte[]{0x58,0x5D,0,0});
        reportAll("0x5d5c disp32 (confirmed present via manual read)", new byte[]{0x5C,0x5D,0,0});
    }

    private void reportAll(String label, byte[] pattern) throws Exception {
        println("=== " + label + " ===");
        Address addr = currentProgram.getMinAddress();
        int count = 0;
        while (count < 200) {
            Address hit = find(addr, pattern);
            if (hit == null) break;
            Function fn = getFunctionContaining(hit);
            println("  hit @ " + hit + "  fn=" + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint() : "none"));
            count++;
            addr = hit.add(1);
        }
        println("  total: " + count + " hit(s)");
    }
}
