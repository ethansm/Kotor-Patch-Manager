// Phase A.6 sub-question B, continued. FUN_0051d2f0 decompile confirmed:
// CItemPropertyApplierRemover::vftable @ 0x0099247c (base, written first)
// CSWSItemPropertyHandler::vftable    @ 0x00992468 (derived, final -- this
// is the live vtable pointer the owning object actually carries at +0x5c)
// This mirrors CSWSEffectListHandler exactly (base 0x9924a4, derived/final
// 0x992490, whose slots 0/1/2/3 = dtor/Init/Apply/Remove per the DB).
// Dump slots 0-3 of 0x00992468, then decompile slot1 (Init-equivalent)
// and slot2 (Apply-equivalent) in full, looking for calls to
// CGameEffect::SetObjectID (0x005e4f00) / SetString (0x005e4f60) with
// an item-handle-shaped argument.
//
// @category KOTOR
// @menupath Tools.KOTOR.Dump ItemProperty Vtable And Apply

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.mem.Memory;

public class DumpItemPropertyVtableAndApply extends GhidraScript {

    @Override
    public void run() throws Exception {
        Address vftableAddr = currentProgram.getAddressFactory().getAddress("0x00992468");
        Memory mem = currentProgram.getMemory();

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        println("=== CSWSItemPropertyHandler::vftable @ " + vftableAddr + " slots 0-5 ===");
        Address[] slotAddrs = new Address[6];
        for (int slot = 0; slot < 6; slot++) {
            Address cur = vftableAddr.add(slot * 4L);
            int ptr;
            try {
                ptr = mem.getInt(cur);
            } catch (Exception e) {
                println("  [" + slot + "] " + cur + " -- read failed");
                continue;
            }
            Address target = toAddr(ptr & 0xffffffffL);
            slotAddrs[slot] = target;
            Function fn = getFunctionAt(target);
            println("  [" + slot + "] " + cur + " -> " + target + (fn != null ? " (" + fn.getName() + ", size=" + fn.getBody().getNumAddresses() + ")" : " (NOT A FUNCTION)"));
        }

        for (int slot : new int[]{1, 2}) {
            Address target = slotAddrs[slot];
            if (target == null) continue;
            Function fn = getFunctionAt(target);
            if (fn == null) { println("slot " + slot + " -- no function, skipping decompile"); continue; }
            println("");
            println("---- DECOMPILE slot " + slot + ": " + fn.getName() + " @ " + target + " size=" + fn.getBody().getNumAddresses() + " ----");
            DecompileResults res = decomp.decompileFunction(fn, 60, getMonitor());
            if (res != null && res.decompileCompleted()) {
                println(res.getDecompiledFunction().getC());
            } else {
                println("  (decompile failed / timed out)");
            }
        }

        decomp.dispose();
    }
}
