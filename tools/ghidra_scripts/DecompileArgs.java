// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.app.decompiler.*;
public class DecompileArgs extends GhidraScript {
    public void run() throws Exception {
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        for (String s : getScriptArgs()) {
            Function f = getFunctionContaining(toAddr(Long.parseLong(s,16)));
            if (f==null) { println("NOFUNC "+s); continue; }
            println("=== "+f.getName()+" @ "+f.getEntryPoint()+" size="+f.getBody().getNumAddresses());
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            println(r.decompileCompleted()? r.getDecompiledFunction().getC() : "(fail)");
        }
    }
}
