// @category KOTOR
// Portrait hover-tooltip research: (1) find functions holding scalar operands 0x1AC29 (strref 109865 XP Needed) / 0x123C8 (74696 Level:) /
// 1060 / 1061 near them, decompile each unique container; (2) decompile the GOG-delta candidates 0x7449c0..0x744c20 region.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.app.decompiler.*;
import java.util.*;
public class FindPortraitTooltip extends GhidraScript {
    DecompInterface d;
    void dump(Function f) throws Exception {
        println("=== " + f.getName() + " @ " + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses());
        DecompileResults r = d.decompileFunction(f, 60, monitor);
        println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(fail)");
    }
    public void run() throws Exception {
        d = new DecompInterface(); d.openProgram(currentProgram);
        long[] keys = {0x1AC29L, 0x123C8L};
        Set<Long> seen = new LinkedHashSet<>();
        Listing l = currentProgram.getListing();
        for (long k : keys) {
            println("### scalar 0x" + Long.toHexString(k));
            for (Instruction ins : l.getInstructions(true)) {
                int n = ins.getNumOperands();
                for (int i = 0; i < n; i++) {
                    ghidra.program.model.scalar.Scalar s = ins.getScalar(i);
                    if (s != null && s.getUnsignedValue() == k) {
                        Function f = getFunctionContaining(ins.getAddress());
                        println("  hit " + ins.getAddress() + " " + ins + " in " + (f == null ? "none" : f.getEntryPoint().toString()));
                        if (f != null) seen.add(f.getEntryPoint().getOffset());
                    }
                }
            }
        }
        for (long a : seen) { Function f = getFunctionAt(toAddr(a)); if (f != null && f.getBody().getNumAddresses() < 6000) dump(f); }
        println("### region 0x7449c0..0x744c20");
        FunctionIterator it = l.getFunctions(toAddr(0x7449c0L), true);
        while (it.hasNext()) { Function f = it.next(); if (f.getEntryPoint().getOffset() >= 0x744c20L) break; if (!seen.contains(f.getEntryPoint().getOffset())) dump(f); }
    }
}
