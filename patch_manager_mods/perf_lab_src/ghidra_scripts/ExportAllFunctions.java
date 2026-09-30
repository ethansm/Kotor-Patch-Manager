// Exports EVERY function (named or FUN_) to a TSV: addr, size, retN, name, namespace, source.
// Headless arg: output path. Used by the engine-optimization research (doc 10) for
// profiler-sample mapping and GOG->Steam name transfer (gap-sequence alignment).
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import java.io.*;

public class ExportAllFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File out = new File(a.length > 0 ? a[0] : "steam_functions.tsv");
        int n = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(out))) {
            w.println("addr\tsize\tretn\tname\tnamespace\tsource\tthunk");
            Listing l = currentProgram.getListing();
            for (Function fn : currentProgram.getFunctionManager().getFunctions(true)) {
                long size = fn.getBody().getNumAddresses();
                int retn = -1; // -1 = no RET found; 0 = plain ret
                InstructionIterator it = l.getInstructions(fn.getBody(), true);
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    if (ins.getMnemonicString().equals("RET")) {
                        int r = 0;
                        if (ins.getNumOperands() > 0) {
                            try { r = (int) ins.getScalar(0).getUnsignedValue(); } catch (Exception e) { r = 0; }
                        }
                        retn = Math.max(retn, r);
                    }
                }
                String ns = fn.getParentNamespace().isGlobal() ? "" : fn.getParentNamespace().getName();
                w.println(String.format("0x%08x", fn.getEntryPoint().getOffset()) + "\t" + size + "\t" + retn + "\t"
                    + fn.getName() + "\t" + ns + "\t" + fn.getSymbol().getSource() + "\t" + fn.isThunk());
                n++;
            }
        }
        println("ExportAllFunctions: " + n + " functions -> " + out);
    }
}
