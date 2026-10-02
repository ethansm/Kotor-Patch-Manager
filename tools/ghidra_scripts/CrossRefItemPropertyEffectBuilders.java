// Phase A.6 sub-question B, continued. FUN_005aa490 (CSWSItemPropertyHandler
// Init) populated a 0x46=70-entry Apply-dispatch table with handler
// functions spanning roughly 0x5aa000-0x5b3000. CGameEffect::SetObjectID
// (0x005e4f00) has 26 known callers in the broad 0x52c000-0x6e2000 range
// (never individually identified) -- check whether any of THOSE callers
// falls inside the item-property-handler address range, which would mean
// an item-property Apply handler directly sets an ObjectID param on a
// CGameEffect (the item back-reference this session is looking for).
// Also check CGameEffect::Constructor (0x005e4920) and ::SetString
// (0x005e4f60) callers the same way. Any hit in [0x5a0000,0x5c0000) gets
// a full decompile.
//
// @category KOTOR
// @menupath Tools.KOTOR.CrossRef ItemProperty Effect Builders

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

import java.util.LinkedHashSet;
import java.util.Set;

public class CrossRefItemPropertyEffectBuilders extends GhidraScript {

    private void checkTarget(String label, String addrStr, Address rangeLo, Address rangeHi, Set<Address> hits) throws Exception {
        Address target = currentProgram.getAddressFactory().getAddress(addrStr);
        ReferenceIterator refs = currentProgram.getReferenceManager().getReferencesTo(target);
        println("=== callers of " + label + " (" + addrStr + ") ===");
        int n = 0;
        while (refs.hasNext()) {
            Reference r = refs.next();
            Function fn = getFunctionContaining(r.getFromAddress());
            n++;
            if (fn == null) continue;
            Address e = fn.getEntryPoint();
            boolean inRange = e.compareTo(rangeLo) >= 0 && e.compareTo(rangeHi) < 0;
            println("  from " + r.getFromAddress() + " in " + fn.getName() + " @ " + e + (inRange ? "   <-- IN ITEM-PROPERTY-HANDLER RANGE" : ""));
            if (inRange) hits.add(e);
        }
        println("  total callers: " + n);
        println("");
    }

    @Override
    public void run() throws Exception {
        Address rangeLo = currentProgram.getAddressFactory().getAddress("0x005a0000");
        Address rangeHi = currentProgram.getAddressFactory().getAddress("0x005c0000");
        Set<Address> hits = new LinkedHashSet<>();

        checkTarget("CGameEffect::Constructor", "0x005e4920", rangeLo, rangeHi, hits);
        checkTarget("CGameEffect::SetObjectID", "0x005e4f00", rangeLo, rangeHi, hits);
        checkTarget("CGameEffect::SetString", "0x005e4f60", rangeLo, rangeHi, hits);
        checkTarget("CGameEffect::SetInteger", "0x005e4e80", rangeLo, rangeHi, hits);
        checkTarget("CGameEffect::SetExpiryTime", "0x005e4f80", rangeLo, rangeHi, hits);

        println("=== " + hits.size() + " distinct item-property-handler functions call one of the above ===");
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        try {
            for (Address entry : hits) {
                Function fn = getFunctionAt(entry);
                println("");
                println("---- DECOMPILE " + fn.getName() + " @ " + entry + " size=" + fn.getBody().getNumAddresses() + " ----");
                DecompileResults res = decomp.decompileFunction(fn, 45, getMonitor());
                if (res != null && res.decompileCompleted()) {
                    println(res.getDecompiledFunction().getC());
                } else {
                    println("  (decompile failed)");
                }
            }
        } finally {
            decomp.dispose();
        }
    }
}
