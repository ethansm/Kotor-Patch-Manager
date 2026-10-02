// Lists defined strings matching load/loadscreen-related keywords, and for
// each, the functions that reference it. Used to locate the Steam-build
// equivalent of GOG's loading-screen panel-sizing code when a direct byte
// pattern search misses.
//
// @category KOTOR
// @menupath Tools.KOTOR.Find Load Strings

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.DataType;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;

public class FindLoadStrings extends GhidraScript {

    private static final String[] KEYWORDS = {
        "load", "Load", "LOAD"
    };

    @Override
    public void run() throws Exception {
        DataIterator it = currentProgram.getListing().getDefinedData(currentProgram.getMinAddress(), true);
        int matched = 0;
        while (it.hasNext()) {
            Data d = it.next();
            DataType dt = d.getDataType();
            String tn = dt.getName().toLowerCase();
            if (!tn.contains("string") && !tn.contains("char")) continue;
            Object val = d.getValue();
            if (val == null) continue;
            String s = val.toString();
            if (s.length() < 3 || s.length() > 80) continue;

            boolean hit = false;
            for (String kw : KEYWORDS) {
                if (s.contains(kw)) { hit = true; break; }
            }
            if (!hit) continue;

            matched++;
            println("STRING @ " + d.getAddress() + " : \"" + s + "\"");
            Reference[] refs = getReferencesTo(d.getAddress());
            int refCount = 0;
            for (Reference r : refs) {
                Address from = r.getFromAddress();
                Function fn = getFunctionContaining(from);
                println("    ref from " + from + (fn != null ? " in " + fn.getName() + " @ " + fn.getEntryPoint() : " (no function)"));
                refCount++;
                if (refCount > 15) { println("    ... (more refs truncated)"); break; }
            }
            if (matched > 200) { println("... (too many string matches, truncating)"); break; }
        }
        println("Total matching strings: " + matched);
    }
}
