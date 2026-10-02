// @category KOTOR
// args: <startHex> <endHex> [maxSize]  -- lists every function in [start,end) and decompiles each
// (skips bodies larger than maxSize bytes, default 4000). Generic class-cluster survey.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.app.decompiler.*;
public class RangeDecompileArgs extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        long start = Long.parseLong(a[0], 16), end = Long.parseLong(a[1], 16);
        long max = a.length > 2 ? Long.parseLong(a[2]) : 4000;
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        FunctionIterator it = currentProgram.getListing().getFunctions(toAddr(start), true);
        while (it.hasNext()) {
            Function f = it.next();
            if (f.getEntryPoint().getOffset() >= end) break;
            long sz = f.getBody().getNumAddresses();
            println("=== " + f.getName() + " @ " + f.getEntryPoint() + " size=" + sz);
            if (sz > max) { println("(skipped, too large)"); continue; }
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            println(r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(fail)");
        }
    }
}
