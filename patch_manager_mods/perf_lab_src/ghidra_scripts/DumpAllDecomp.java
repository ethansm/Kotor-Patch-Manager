// Decompiles (and disassembles) every non-thunk function into <outdir>/decomp/<bucket>/<addr>.c and .asm,
// bucket = high address byte ("00".."ff" of the 32-bit VA >> 16 & 0xff). Header line per .c file:
//   // FN <addr> <name> size=<n> ret=<N> callers=<a,b,..> callees=<a,b,..>
// Headless args: outdir [startIdx endIdx]  (index into the filtered, address-ordered function list; resumable:
// files that already exist are skipped). Failures are appended to <outdir>/decomp_failed.txt.
// Doc 10 Phase 2 (dump once, analyse the text in parallel without Ghidra).
// @category KOTOR

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.io.*;
import java.util.*;

public class DumpAllDecomp extends GhidraScript {
    static String hex(Function f) { return String.format("%08x", f.getEntryPoint().getOffset()); }

    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        File root = new File(a[0]);
        List<Function> fns = new ArrayList<>();
        for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
            if (f.isThunk() || f.isExternal() || f.getName().startsWith("Unwind@")) continue;
            fns.add(f);
        }
        int lo = a.length > 2 ? Integer.parseInt(a[1]) : 0;
        int hi = a.length > 2 ? Math.min(Integer.parseInt(a[2]), fns.size()) : fns.size();
        println("DumpAllDecomp: " + fns.size() + " functions, range " + lo + ".." + hi);
        DecompInterface d = new DecompInterface();
        d.openProgram(currentProgram);
        Listing l = currentProgram.getListing();
        long t0 = System.currentTimeMillis();
        int done = 0, failed = 0, skipped = 0;
        try (PrintWriter fail = new PrintWriter(new FileWriter(new File(root, "decomp_failed.txt"), true))) {
            for (int i = lo; i < hi; i++) {
                if (monitor.isCancelled()) break;
                Function f = fns.get(i);
                String h = hex(f);
                File dir = new File(root, "decomp/" + h.substring(2, 4));
                dir.mkdirs();
                File cf = new File(dir, h + ".c");
                if (cf.exists()) { skipped++; continue; }
                // ret N = max RET immediate (stack purge)
                int retn = -1;
                StringBuilder asm = new StringBuilder();
                InstructionIterator it = l.getInstructions(f.getBody(), true);
                while (it.hasNext()) {
                    Instruction ins = it.next();
                    if (ins.getMnemonicString().equals("RET")) {
                        int r = 0;
                        if (ins.getNumOperands() > 0) try { r = (int) ins.getScalar(0).getUnsignedValue(); } catch (Exception e) { }
                        retn = Math.max(retn, r);
                    }
                    StringBuilder b = new StringBuilder();
                    try { for (byte x : ins.getBytes()) b.append(String.format("%02x", x & 0xff)); } catch (Exception e) { }
                    asm.append(ins.getAddress()).append("  ").append(String.format("%-22s", b)).append(ins).append('\n');
                }
                StringBuilder callers = new StringBuilder(), callees = new StringBuilder();
                for (Function c : f.getCallingFunctions(monitor)) callers.append(callers.length() > 0 ? "," : "").append(hex(c));
                for (Function c : f.getCalledFunctions(monitor)) callees.append(callees.length() > 0 ? "," : "").append(hex(c));
                String hdr = "// FN " + h + " " + f.getName(true) + " size=" + f.getBody().getNumAddresses() + " ret=" + retn
                    + " callers=" + callers + " callees=" + callees + "\n";
                String c;
                try {
                    DecompileResults r = d.decompileFunction(f, 60, monitor);
                    c = r.decompileCompleted() ? r.getDecompiledFunction().getC() : null;
                    if (c == null) { failed++; fail.println(h + "\t" + f.getName() + "\t" + r.getErrorMessage().replace('\n', ' ')); fail.flush(); c = "// DECOMPILE FAILED: " + r.getErrorMessage() + "\n"; }
                } catch (Exception e) {
                    failed++; fail.println(h + "\t" + f.getName() + "\texception " + e); fail.flush();
                    c = "// DECOMPILE FAILED: " + e + "\n";
                    d.dispose(); d = new DecompInterface(); d.openProgram(currentProgram);
                }
                try (Writer w = new FileWriter(new File(dir, h + ".asm"))) { w.write("; FN " + h + " " + f.getName(true) + "\n"); w.write(asm.toString()); }
                try (Writer w = new FileWriter(cf)) { w.write(hdr); w.write(c); }
                done++;
                if (done % 500 == 0) println("  " + i + "/" + hi + " done=" + done + " failed=" + failed + " " + (System.currentTimeMillis() - t0) / 1000 + "s");
            }
        }
        d.dispose();
        println("DumpAllDecomp: done=" + done + " failed=" + failed + " skipped(existing)=" + skipped + " in " + (System.currentTimeMillis() - t0) / 1000 + "s");
    }
}
