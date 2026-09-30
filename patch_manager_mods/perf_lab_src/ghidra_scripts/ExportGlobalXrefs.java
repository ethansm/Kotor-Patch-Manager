// Exports readers/writers of engine globals: <outdir>/globals.tsv
// (global, target_addr, site, reftype, fn, fn_name, insn). Args: outdir addr[:len] ...  (hex; :len scans every
// byte of [addr, addr+len) so struct/array globals catch field references). Doc 10 Phase 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;

public class ExportGlobalXrefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File root = new File(a[0]);
        int n = 0;
        try (PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(root, "globals.tsv"))))) {
            w.println("global\ttarget_addr\tsite\treftype\tfn\tfn_name\tinsn");
            for (int k = 1; k < a.length; k++) {
                String[] p = a[k].split(":");
                long base = Long.parseLong(p[0].replace("0x", ""), 16);
                long len = p.length > 1 ? Long.decode(p[1]) : 1;
                for (long off = 0; off < len; off++) {
                    Address t = toAddr(base + off);
                    for (Reference r : getReferencesTo(t)) {
                        Function f = getFunctionContaining(r.getFromAddress());
                        Instruction i = getInstructionAt(r.getFromAddress());
                        w.println(p[0] + "\t" + String.format("%08x", t.getOffset()) + "\t" + String.format("%08x", r.getFromAddress().getOffset())
                            + "\t" + r.getReferenceType() + "\t" + (f == null ? "-" : String.format("%08x", f.getEntryPoint().getOffset()))
                            + "\t" + (f == null ? "-" : f.getName(true)) + "\t" + (i == null ? "" : i.toString()));
                        n++;
                    }
                }
            }
        }
        println("ExportGlobalXrefs: " + n + " refs");
    }
}
