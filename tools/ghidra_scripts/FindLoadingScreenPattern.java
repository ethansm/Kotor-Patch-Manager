// Searches the Steam Aspyr swkotor2.exe for the GOG loading-screen-sizing
// byte pattern (K2AspyrLoadingScreenLineFix hook), to find its Steam-build
// equivalent address for porting. Tries the full 30-byte GOG original_bytes
// sequence first, then a shorter 8-byte distinctive sub-sequence if that
// misses. For every hit, dumps the containing function, disassembly context,
// and decompiled C so the match can be judged by hand.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Loading Screen Pattern

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.listing.Listing;

public class FindLoadingScreenPattern extends GhidraScript {

    private static final String FULL_PATTERN =
        "b8 01 00 00 00 " +
        "83 e0 01 " +
        "8b 8d 54 ff ff ff " +
        "8b 51 48 " +
        "83 e2 fe " +
        "0b d0 " +
        "8b 85 54 ff ff ff " +
        "89 50 48";

    private static final String SHORT_PATTERN =
        "8b 51 48 83 e2 fe 0b d0";

    @Override
    public void run() throws Exception {
        Address start = currentProgram.getMinAddress();

        println("=== Searching for FULL 30-byte GOG pattern ===");
        Address[] fullHits = findBytes(start, FULL_PATTERN, 50);
        report(fullHits, 30);

        println("");
        println("=== Searching for SHORT 8-byte distinctive sub-sequence ===");
        Address[] shortHits = findBytes(start, SHORT_PATTERN, 50);
        report(shortHits, 8);
    }

    private void report(Address[] hits, int patternLen) throws Exception {
        println("Found " + hits.length + " hit(s).");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (Address hit : hits) {
                println("---- HIT at " + hit + " ----");
                Function fn = getFunctionContaining(hit);
                if (fn == null) {
                    println("  (not inside any defined function)");
                } else {
                    println("  Containing function: " + fn.getName() + " @ " + fn.getEntryPoint()
                        + " (size " + fn.getBody().getNumAddresses() + " bytes)");
                }

                println("  -- Disassembly context --");
                Listing listing = currentProgram.getListing();
                Address ctxStart = hit.subtract(16);
                if (ctxStart == null || ctxStart.getOffset() < 0) {
                    ctxStart = hit;
                }
                InstructionIterator it = listing.getInstructions(ctxStart, true);
                Address end = hit.add(patternLen + 16);
                while (it.hasNext()) {
                    Instruction insn = it.next();
                    if (insn.getAddress().compareTo(end) > 0) break;
                    String marker = (insn.getAddress().compareTo(hit) >= 0
                        && insn.getAddress().compareTo(hit.add(patternLen - 1)) <= 0) ? " <== IN PATTERN" : "";
                    println("    " + insn.getAddress() + ": " + insn.toString() + marker);
                }

                if (fn != null) {
                    println("  -- Decompiled C --");
                    DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                    if (res != null && res.decompileCompleted()) {
                        println(res.getDecompiledFunction().getC());
                    } else {
                        println("  (decompilation failed: " + (res != null ? res.getErrorMessage() : "null result") + ")");
                    }
                }
                println("");
            }
        } finally {
            decomp.dispose();
        }
    }
}
