// Phase A.6, sub-question A: find the GFF tag literally named "ICON"
// (baseitems.2da's own icon column is named "Icon") inside the item/.uti
// loader's tag-lookup calls -- same technique that already found
// CSWGuiImage_Load's "IMAGE" tag and CSWGuiBorder's "CORNER"/"EDGE"/
// "FILL" tags earlier in this project. Finds defined strings matching
// ICON (case-insensitive, short strings only to avoid prose matches),
// prints xrefs + containing function + a decompile of each containing
// function so the storage offset can be read directly.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Icon Tag Xrefs

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.DataType;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

import java.util.LinkedHashSet;
import java.util.Set;

public class FindIconTagXrefs extends GhidraScript {

    @Override
    public void run() throws Exception {
        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);

        DataIterator it = currentProgram.getListing().getDefinedData(currentProgram.getMinAddress(), true);
        Set<Address> containingFns = new LinkedHashSet<>();
        int matched = 0;
        while (it.hasNext()) {
            Data d = it.next();
            DataType dt = d.getDataType();
            String tn = dt.getName().toLowerCase();
            if (!tn.contains("string") && !tn.contains("char")) continue;
            Object val = d.getValue();
            if (val == null) continue;
            String s = val.toString();
            if (s.length() < 3 || s.length() > 20) continue;
            if (!s.toLowerCase().contains("icon")) continue;

            matched++;
            println("STRING @ " + d.getAddress() + " : \"" + s + "\"");
            Reference[] refs = getReferencesTo(d.getAddress());
            for (Reference r : refs) {
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                if (fn != null) {
                    println("    ref from " + from + " in " + fn.getName() + " @ " + fn.getEntryPoint());
                    containingFns.add(fn.getEntryPoint());
                } else {
                    println("    ref from " + from + " (no function)");
                }
            }
        }
        println("Total exact ICON string matches: " + matched);
        println("Distinct containing functions: " + containingFns.size());

        for (Address entry : containingFns) {
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
        decomp.dispose();
    }
}
