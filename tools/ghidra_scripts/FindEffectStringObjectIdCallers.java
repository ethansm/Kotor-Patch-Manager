// Buff Duration HUD Item 5: check whether item-property application ever
// writes into a CGameEffect's generic String (+0x48) or ObjectID (+0x78)
// param arrays via CGameEffect::SetString (0x005e4f60) or SetObjectID
// (0x005e4f00) -- if item-property-granted effects retain a reference
// back to the granting item (a resref string, or an object handle),
// that's a cleaner icon-resolution path than chasing the quickbar's
// SetIcon. Lists all callers of both setters.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Effect String ObjectId Callers

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class FindEffectStringObjectIdCallers extends GhidraScript {

    static final long[] TARGETS = {
        0x005e4f60L, // CGameEffect::SetString
        0x005e4f00L, // CGameEffect::SetObjectID
    };

    @Override
    public void run() throws Exception {
        for (long t : TARGETS) {
            Address addr = toAddr(t);
            println("=== callers of 0x" + Long.toHexString(t) + " ===");
            ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(addr);
            int count = 0;
            while (refs.hasNext() && !monitor.isCancelled()) {
                Reference r = refs.next();
                if (r.getReferenceType().isCall()) {
                    Address from = r.getFromAddress();
                    Function f = getFunctionContaining(from);
                    String fname = (f != null) ? f.getName() + "@" + f.getEntryPoint() : "???";
                    println("  " + from + "  [" + fname + "]");
                    count++;
                }
            }
            println("  (" + count + " call hits)");
        }
    }
}
