// Buff Duration HUD "Resource-Bind Completion + Icon/Effect Correspondence" session.
// Decompiles:
//  - FUN_0047eb60 -- CSWGuiBorder::Load's resource-acquire call (item 1)
//  - FUN_00416090 -- CSWGuiImage::Load's "IMAGE"-tag binder call (item 1)
//  - CSWGuiLabel's own (non-inherited) vtable-override slots other than
//    dtor/SetRect/Draw: 004198d0(18), 00419a10(34), 00419a60(35),
//    00419840(38), 004197d0(39) -- looking for Load/Initialize (item 2)
//  - FUN_007450e0 -- re-decompiled in full for the STORAGE OFFSET of each
//    looked-up GUI tag (LBL_CHAR%d/PB_VIT%d/PB_FORCE%d/LBL_LEVELUP%d/
//    LBL_BACK%d/LBL_DEBILITATED%d/LBL_DISABLED%d/BTN_CHAR%d), to
//    cross-reference against Adjust's 8 array slots (item 3)
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile Resource Bind And Label Init

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DecompileResourceBindAndLabelInit extends GhidraScript {

    private static final String[] DECOMPILE_TARGETS = {
        "0047eb60",  // item 1: CSWGuiBorder::Load resource-acquire call
        "00416090",  // item 1: CSWGuiImage::Load "IMAGE"-tag binder
        "004198d0",  // item 2: CSWGuiLabel own vtable slot 18
        "00419a10",  // item 2: CSWGuiLabel own vtable slot 34
        "00419a60",  // item 2: CSWGuiLabel own vtable slot 35
        "00419840",  // item 2: CSWGuiLabel own vtable slot 38
        "004197d0",  // item 2: CSWGuiLabel own vtable slot 39
        "007450e0"   // item 3: full re-decompile for tag storage offsets
    };

    @Override
    public void run() throws Exception {
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
