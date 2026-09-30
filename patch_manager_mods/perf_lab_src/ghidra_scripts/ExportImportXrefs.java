// Exports every call/reference site of every imported function: <outdir>/imports.tsv
// (import, library, calling_fn, fn_name, site, reftype, insn). Resolves the external symbol, its IAT slot and
// thunks the same way as KPM ImportCallers.java, but for all imports at once. Headless arg: outdir. Doc 10 Phase 2.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.util.*;

public class ExportImportXrefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        File root = new File(getScriptArgs()[0]);
        SymbolTable st = currentProgram.getSymbolTable();
        int n = 0, imps = 0;
        try (PrintWriter w = new PrintWriter(new BufferedWriter(new FileWriter(new File(root, "imports.tsv"))))) {
            w.println("import\tlibrary\tcalling_fn\tfn_name\tsite\treftype\tinsn");
            for (Symbol s : st.getExternalSymbols()) {
                imps++;
                String lib = s.getParentNamespace() == null ? "" : s.getParentNamespace().getName();
                Set<Address> targets = new LinkedHashSet<>();
                targets.add(s.getAddress());
                for (Address t : new ArrayList<>(targets)) {
                    for (Reference r : getReferencesTo(t)) {
                        targets.add(r.getFromAddress());
                        Function tf = getFunctionContaining(r.getFromAddress());
                        if (tf != null && tf.isThunk()) targets.add(tf.getEntryPoint());
                    }
                }
                Set<String> seen = new HashSet<>();
                for (Address t : targets) {
                    for (Reference r : getReferencesTo(t)) {
                        RefType rt = r.getReferenceType();
                        if (!rt.isCall() && !rt.isData() && !rt.isJump()) continue;
                        Function f = getFunctionContaining(r.getFromAddress());
                        if (f != null && f.isThunk()) continue;
                        if (f == null && !rt.isCall()) continue; // IAT slot itself
                        if (!seen.add(r.getFromAddress().toString())) continue;
                        Instruction i = getInstructionAt(r.getFromAddress());
                        w.println(s.getName() + "\t" + lib + "\t" + (f == null ? "-" : String.format("%08x", f.getEntryPoint().getOffset()))
                            + "\t" + (f == null ? "-" : f.getName(true)) + "\t" + String.format("%08x", r.getFromAddress().getOffset())
                            + "\t" + rt + "\t" + (i == null ? "" : i.toString()));
                        n++;
                    }
                }
            }
        }
        println("ExportImportXrefs: " + imps + " imports, " + n + " sites");
    }
}
