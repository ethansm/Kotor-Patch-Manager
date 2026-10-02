// @category KOTOR
// args: hexaddr,hexaddr,...  -- decompile each function containing the address, plus callers list (xrefs) of each entry
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;
public class DecompileList extends GhidraScript {
    public void run() throws Exception {
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        for (String s : getScriptArgs()[0].split(",")) {
            Function f = getFunctionContaining(toAddr(Long.parseLong(s, 16)));
            if (f == null) { println("=== no function at " + s); continue; }
            println("=== " + f.getName() + " @ " + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses());
            StringBuilder sb = new StringBuilder("xrefs:");
            for (Reference r : getReferencesTo(f.getEntryPoint())) sb.append(" ").append(r.getFromAddress()).append("(").append(r.getReferenceType()).append(")");
            println(sb.toString());
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(fail)");
        }
    }
}
