// Byte-diff-cluster proximity check for AdditionalConsoleCommands' 2
// confirmed Steam addresses (0x0091d52d hook1, 0x004752d8 hook2) against
// the known unidentified mystery hand-patch clusters, using the correct
// per-section VA->file-offset math (not a flat VA-ImageBase subtraction).
//
// @category KOTOR
// @menupath Tools.KOTOR.Proximity Check Console Hooks

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class ProximityCheckConsoleHooks extends GhidraScript {

    @Override
    public void run() throws Exception {
        long[] hooks = new long[]{0x0091d52dL, 0x004752d8L};
        String[] names = new String[]{"hook1 (InitializeAdditionalCommands)", "hook2 (console-history-save flag)"};
        // Known mystery clusters (VA), from prior sessions.
        long[] clusters = new long[]{
            0x0070d530L, // audio ducking hand-patch
            0x009889baL, // shader-string cluster (gamma/color correction)
            0x0098ec2aL,
            0x0098f522L,
            0x0098fad2L,
            0x009f5993L,
        };

        for (int h = 0; h < hooks.length; h++) {
            println("=== " + names[h] + " @ 0x" + Long.toHexString(hooks[h]) + " ===");
            for (long c : clusters) {
                long dist = Math.abs(hooks[h] - c);
                println(String.format("  vs cluster 0x%08x: direct VA distance = %,d bytes (%.2f KB)",
                    c, dist, dist / 1024.0));
            }
            println();
        }
    }
}
