// Buff Duration HUD Item 5 / Phase A.5: among the functions that touch
// the CSWGuiMainInterfaceAction slot array (hudThis+0x733c, stride
// 0x750), find which one is SetIcon by looking for calls to the already
// -confirmed icon/resource-binding chain (CSWGuiBorder_LoadResource
// 0x0047eb60, CSWGuiImage_SetImage 0x00416090, CSWGuiImage_Load
// 0x00416720) from within the candidate functions found by
// FindActionSlotArrayRefs.java.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find ActionSetIcon Candidate

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.InstructionIterator;
import ghidra.program.model.symbol.Reference;

public class FindActionSetIconCandidate extends GhidraScript {

    static final long[] CANDIDATES = {
        0x007469a0L, 0x00747210L, 0x0074ad90L, 0x0074c320L,
        0x0074cc90L, 0x0074cfe0L, 0x00751750L
    };

    static final long[] ICON_CHAIN = {
        0x0047eb60L, // CSWGuiBorder_LoadResource
        0x00416090L, // CSWGuiImage_SetImage
        0x00416720L, // CSWGuiImage_Load
        0x00414840L, // CSWGuiBorder_Load_FillSetter
    };

    @Override
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        for (long c : CANDIDATES) {
            Address entry = toAddr(c);
            Function f = fm.getFunctionAt(entry);
            if (f == null) {
                println("FUN_" + Long.toHexString(c) + " not found as function");
                continue;
            }
            println("=== FUN_" + Long.toHexString(c) + " body=" + f.getBody().getNumAddresses() + " bytes ===");
            InstructionIterator it = currentProgram.getListing().getInstructions(f.getBody(), true);
            while (it.hasNext() && !monitor.isCancelled()) {
                Instruction instr = it.next();
                if (instr.getFlowType().isCall()) {
                    Reference[] refs = instr.getReferencesFrom();
                    for (Reference r : refs) {
                        long target = r.getToAddress().getOffset();
                        for (long ic : ICON_CHAIN) {
                            if (target == ic) {
                                println("  CALLS icon-chain fn 0x" + Long.toHexString(ic)
                                        + " at " + instr.getAddress());
                            }
                        }
                    }
                }
            }
        }
    }
}
