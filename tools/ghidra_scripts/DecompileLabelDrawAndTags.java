// Buff Duration HUD UX-redesign continuation (U1/U5/U3 research).
// Decompiles:
//  - CSWGuiLabel::Draw (00419880) -- resolves U1(a) vs (b)
//  - CSWGuiProgressBar::Draw (0041bb20) -- R7 fallback lead, cheap bonus
//  - CSWGuiBorder::Load's 3 sub-widget setters (00414680/00414760/00414840) -- U5 step 2
//  - Adjust candidate (00745560) and DrawEffects_2 candidate (00745cc0) -- U3
//  - CSWGuiImage slots 3/4 (004168b0/00416720) -- U5 step 3 continuation
// Also dumps the raw string bytes at the two unresolved DAT_ addresses referenced
// by CSWGuiBorder::Load next to the already-decoded "CORNER" tag -- U5 step 1.
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Label Draw And Tags

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;

public class DecompileLabelDrawAndTags extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "00419880",  // CSWGuiLabel::Draw
        "0041bb20",  // CSWGuiProgressBar::Draw
        "00414680",  // CSWGuiBorder::Load setter #1
        "00414760",  // CSWGuiBorder::Load setter #2
        "00414840",  // CSWGuiBorder::Load setter #3
        "00745560",  // Adjust candidate (U3)
        "00745cc0",  // DrawEffects_2 candidate (U3)
        "004168b0",  // CSWGuiImage slot 3
        "00416720"   // CSWGuiImage slot 4
    };

    private static final String[] STRING_ADDRS = {
        "0098762c",
        "00987624"
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
        println("=== String dumps (U5 step 1) ===");
        for (String hex : STRING_ADDRS) {
            Address addr = currentProgram.getAddressFactory().getAddress("0x" + hex);
            // The DAT_ symbol is a pointer to the string in some cases; try both
            // direct-bytes-at-address and pointer-dereference interpretations.
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
