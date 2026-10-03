// Exports functions and symbols to TSV for comparative auditing.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;

public class ExportProgramSymbols extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        String outDir = args.length > 0 ? args[0] : ".";
        File dir = new File(outDir);
        dir.mkdirs();

        String progName = currentProgram.getName();
        File fnOut = new File(dir, progName + "_functions.tsv");
        File symOut = new File(dir, progName + "_symbols.tsv");

        println("Exporting functions from " + progName + "...");
        int fnCount = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(fnOut))) {
            w.println("addr\tsize\tretn\tname\tnamespace\tcallconv\tprototype\tsource\tthunk");
            Listing l = currentProgram.getListing();
            for (Function fn : currentProgram.getFunctionManager().getFunctions(true)) {
                long size = fn.getBody().getNumAddresses();
                int retn = -1;
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
                String callconv = fn.getCallingConventionName() != null ? fn.getCallingConventionName() : "";
                String proto = fn.getPrototypeString(false, false);
                w.println(String.format("0x%08x", fn.getEntryPoint().getOffset()) + "\t"
                    + size + "\t" + retn + "\t" + fn.getName() + "\t" + ns + "\t"
                    + callconv + "\t" + proto + "\t" + fn.getSymbol().getSource() + "\t" + fn.isThunk());
                fnCount++;
            }
        }
        println("Exported " + fnCount + " functions to " + fnOut);

        println("Exporting global symbols from " + progName + "...");
        int symCount = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(symOut))) {
            w.println("addr\tname\tnamespace\tsymbol_type\tsource");
            for (Symbol s : currentProgram.getSymbolTable().getAllSymbols(true)) {
                if (s.getSymbolType() == SymbolType.FUNCTION) continue;
                String ns = s.getParentNamespace().isGlobal() ? "" : s.getParentNamespace().getName();
                w.println(String.format("0x%08x", s.getAddress().getOffset()) + "\t"
                    + s.getName() + "\t" + ns + "\t" + s.getSymbolType().toString() + "\t" + s.getSource());
                symCount++;
            }
        }
        println("Exported " + symCount + " symbols to " + symOut);
    }
}
