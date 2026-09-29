// @category KOTOR
// Args: tag strings. For each exact-match defined string (ASCII, NUL-terminated) anywhere in memory,
// list referencing instructions + containing function; decompile each distinct containing function once.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;
import java.util.*;
public class FindTagXrefsDecompile extends GhidraScript {
    public void run() throws Exception {
        Memory mem = currentProgram.getMemory();
        Set<Function> toDecomp = new LinkedHashSet<>();
        for (String tag : getScriptArgs()) {
            byte[] pat = (tag + "\0").getBytes("US-ASCII");
            Address start = currentProgram.getMinAddress();
            while (true) {
                Address hit = mem.findBytes(start, pat, null, true, monitor);
                if (hit == null) break;
                // must be start of string (previous byte NUL or non-printable)
                byte prev = 0; try { prev = mem.getByte(hit.subtract(1)); } catch (Exception e) {}
                if (prev == 0) {
                    println("STRING '" + tag + "' @ " + hit);
                    for (Reference r : getReferencesTo(hit)) {
                        Function f = getFunctionContaining(r.getFromAddress());
                        println("   ref from " + r.getFromAddress() + " in " + (f == null ? "?" : f.getName() + "@" + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses()));
                        if (f != null) toDecomp.add(f);
                    }
                }
                start = hit.add(1);
            }
        }
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        for (Function f : toDecomp) {
            println("=== DECOMP " + f.getName() + " @ " + f.getEntryPoint());
            DecompileResults r = d.decompileFunction(f, 120, monitor);
            println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(fail)");
        }
    }
}
