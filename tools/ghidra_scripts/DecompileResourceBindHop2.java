// Follow-on to DecompileResourceBindAndLabelInit.java:
//  - FUN_0047d070 -- one hop past FUN_0047eb60, likely the real resref/tag
//    resolve-to-handle call (item 1)
//  - FUN_0047cd50 -- the null/default-tag sibling call (item 1, context)
//  - string dump at DAT_009876d0 -- the profiler-scope tag string paired
//    with "BORDER" in FUN_004198d0, identifying self+0xd8 vs self+0x60 (item 2)
//  - FUN_00710980 / FUN_00710a70 / FUN_00710a10 / FUN_00710ca0 -- the
//    GFF-tag-resolution helper family feeding FUN_0047eb60/FUN_00416090
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Resource Bind Hop2

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;

public class DecompileResourceBindHop2 extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "0047d070",
        "0047cd50",
        "00710980",
        "00710a70",
        "00710a10",
        "00710ca0"
    };

    private static final String[] STRING_ADDRS = {
        "009876d0"
    };

    private void dumpString(Address addr) {
        try {
            Memory mem = currentProgram.getMemory();
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 64; i++) {
                byte b = mem.getByte(addr.add(i));
                if (b == 0) break;
                sb.append((char) (b & 0xFF));
            }
            println("  string@" + addr + " = \"" + sb.toString() + "\"");
        } catch (Exception e) {
            println("  string@" + addr + " -- read failed: " + e.getMessage());
        }
    }

    @Override
    public void run() throws Exception {
        println("=== String dumps ===");
        for (String hex : STRING_ADDRS) {
            Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
            dumpString(addr);
            try {
                Memory mem = currentProgram.getMemory();
                int ptrVal = mem.getInt(addr);
                Address deref = currentProgram.getAddressFactory().getAddress(
                    "0x" + Integer.toHexString(ptrVal));
                println("  (as pointer -> " + deref + ")");
                dumpString(deref);
            } catch (Exception e) {
                println("  (pointer-dereference attempt failed: " + e.getMessage() + ")");
            }
        }

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (String hex : DECOMPILE_TARGETS) {
                Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
                Function fn = getFunctionAt(addr);
                println("");
                println("==== TARGET " + hex + " ====");
                if (fn == null) {
                    println("  (no function defined at this exact address)");
                    fn = getFunctionContaining(addr);
                    if (fn != null) {
                        println("  containing function: " + fn.getName() + " @ " + fn.getEntryPoint());
                    } else {
                        continue;
                    }
                }
                println("  Function: " + fn.getName() + " @ " + fn.getEntryPoint()
                    + " size=" + fn.getBody().getNumAddresses());

                DecompileResults res = decomp.decompileFunction(fn, 30, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println("  -- Decompiled C --");
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed: " + (res != null ? res.getErrorMessage() : "null") + ")");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
