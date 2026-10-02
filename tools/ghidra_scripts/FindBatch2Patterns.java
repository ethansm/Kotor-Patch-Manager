// Byte-pattern search for the Post-Combat Movement Fix and
// AdditionalConsoleCommands GOG hook sites, using find(Address,byte[])
// in a manual loop (NOT findBytes(), which is a regex API and silently
// mismatches plain hex -- see kotor2_ghidra_knowledge.db lessons table).
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Batch2 Patterns

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class FindBatch2Patterns extends GhidraScript {

    @Override
    public void run() throws Exception {
        // Post-Combat Movement Fix: GOG 0x0045BDA8, replace hook, 30 bytes
        reportAll("PostCombatMovementFix original_bytes (30 bytes)",
            new byte[]{
                (byte)0x8B,(byte)0x8D,(byte)0xA0,(byte)0xF9,(byte)0xFF,(byte)0xFF,
                (byte)0x8B,(byte)0x91,(byte)0xA0,0x02,0x00,0x00,
                (byte)0x8B,(byte)0x85,(byte)0xA0,(byte)0xF9,(byte)0xFF,(byte)0xFF,
                (byte)0x8B,(byte)0x88,(byte)0xA0,0x02,0x00,0x00,
                (byte)0x8B,0x12,
                (byte)0x8B,0x42,0x1C,
                (byte)0xFF,(byte)0xD0
            });

        // AdditionalConsoleCommands hook1: GOG 0x00925133, detour, 5 bytes
        // (push 0x00400000 -- watch for many hits, this is a short generic pattern)
        reportAll("AdditionalConsoleCommands hook1 (push 0x00400000, 5 bytes)",
            new byte[]{0x68,0x00,0x00,0x40,0x00});

        // AdditionalConsoleCommands hook2: GOG 0x00867248, simple, 7 bytes
        reportAll("AdditionalConsoleCommands hook2 (7 bytes)",
            new byte[]{0x0f,(byte)0xb6,0x15,(byte)0xd0,(byte)0x98,(byte)0xa2,0x00});
    }

    private void reportAll(String label, byte[] pattern) throws Exception {
        println("=== " + label + " ===");
        Address addr = currentProgram.getMinAddress();
        int count = 0;
        while (count < 300) {
            Address hit = find(addr, pattern);
            if (hit == null) break;
            Function fn = getFunctionContaining(hit);
            println("  hit @ " + hit + "  fn=" + (fn != null ? fn.getName() + " @ " + fn.getEntryPoint() : "none"));
            count++;
            addr = hit.add(1);
        }
        println("  total: " + count + " hit(s)" + (count >= 300 ? " (capped at 300)" : ""));
    }
}
