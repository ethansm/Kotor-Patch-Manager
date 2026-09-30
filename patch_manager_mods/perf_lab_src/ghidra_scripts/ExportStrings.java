// Exports every defined string with its referencing functions: <outdir>/strings.tsv
// (addr, string (escaped), ref_count, referencing functions as addr:name;...). Strings are the naming anchors
// (Grass_*, restype extensions, error messages). Headless arg: outdir. Doc 10 Phase 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportStrings extends GhidraScript {
    @Override
    public void run() throws Exception {
        File root = new File(getScriptArgs()[0]);
        int n = 0;
        try (PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(root, "strings.tsv"))))) {
            w.println("addr\tstring\tref_count\tfunctions");
            DataIterator it = currentProgram.getListing().getDefinedData(true);
            while (it.hasNext()) {
                Data d = it.next();
                if (!d.hasStringValue()) continue;
                Object v = d.getValue();
                if (v == null) continue;
                String s = v.toString().replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n").replace("\r", "\\r");
                Set<String> fs = new LinkedHashSet<>();
                int refs = 0;
                for (Reference r : getReferencesTo(d.getAddress())) {
                    refs++;
                    Function f = getFunctionContaining(r.getFromAddress());
                    fs.add(f == null ? "?" + r.getFromAddress() : String.format("%08x", f.getEntryPoint().getOffset()) + ":" + f.getName(true));
                }
                w.println(String.format("%08x", d.getAddress().getOffset()) + "\t" + s + "\t" + refs + "\t" + String.join(";", fs));
                n++;
            }
        }
        println("ExportStrings: " + n + " strings");
    }
}
