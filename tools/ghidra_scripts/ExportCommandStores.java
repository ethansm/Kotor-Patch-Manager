// Read-only: exports function-pointer table stores (NWScript command tables, B15 of the Steam address-DB pipeline).
// For every function with at least MIN_STORES instructions `MOV [reg(+disp)], imm` whose imm is a function entry,
// writes one row per such store to <outdir>/cmdstores.tsv: fn  insn  base  disp  target  (hex, disp decimal-free hex).
// Headless arg 0: output directory.
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.*;
import ghidra.program.model.scalar.Scalar;
import java.io.*;
import java.util.*;

public class ExportCommandStores extends GhidraScript {
    private static final int MIN_STORES = 50;

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        File dir = new File(args.length > 0 ? args[0] : ".");
        dir.mkdirs();
        Listing listing = currentProgram.getListing();
        FunctionManager fm = currentProgram.getFunctionManager();
        int nFn = 0, nRows = 0;
        try (PrintWriter w = new PrintWriter(new FileWriter(new File(dir, "cmdstores.tsv")))) {
            w.println("fn\tinsn\tbase\tdisp\ttarget");
            for (Function fn : fm.getFunctions(true)) {
                monitor.checkCancelled();
                List<String> rows = new ArrayList<>();
                InstructionIterator it = listing.getInstructions(fn.getBody(), true);
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    if (!ins.getMnemonicString().equals("MOV") || ins.getNumOperands() != 2) continue;
                    Object[] dst = ins.getOpObjects(0);
                    Object[] src = ins.getOpObjects(1);
                    if (src.length != 1 || !(src[0] instanceof Scalar)) continue;
                    Register base = null;
                    long disp = 0;
                    int nReg = 0;
                    for (Object o : dst) {
                        if (o instanceof Register) { base = (Register) o; nReg++; }
                        else if (o instanceof Scalar) disp = ((Scalar) o).getUnsignedValue();
                    }
                    if (base == null || nReg != 1) continue;           // [reg] or [reg+disp] only
                    long v = ((Scalar) src[0]).getUnsignedValue();
                    Address t;
                    try { t = toAddr(v); } catch (Exception e) { continue; }
                    Function tf = fm.getFunctionAt(t);
                    if (tf == null) continue;
                    rows.add(String.format("0x%08x\t0x%08x\t%s\t0x%x\t0x%08x", fn.getEntryPoint().getOffset(),
                        ins.getAddress().getOffset(), base.getName(), disp, v));
                }
                if (rows.size() >= MIN_STORES) {
                    nFn++;
                    for (String r : rows) { w.println(r); nRows++; }
                }
            }
        }
        println(String.format("ExportCommandStores: %d functions, %d stores -> %s", nFn, nRows, dir));
    }
}
