// @category KOTOR
// args: loHex-hiHex  -- decompile every function whose entry lies in [lo,hi], with xrefs-to list
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;
public class DecompileRange extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs()[0].split("-");
        long lo = Long.parseLong(a[0], 16), hi = Long.parseLong(a[1], 16);
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(toAddr(lo), true);
        while (it.hasNext()) {
            Function f = it.next();
            if (f.getEntryPoint().getOffset() > hi) break;
            println("=== " + f.getName() + " @ " + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses());
            StringBuilder sb = new StringBuilder("xrefs:");
            for (Reference r : getReferencesTo(f.getEntryPoint())) sb.append(" ").append(r.getFromAddress());
            println(sb.toString());
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(fail)");
        }
    }
}
