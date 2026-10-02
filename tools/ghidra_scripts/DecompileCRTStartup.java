// Full decompile of ___tmainCRTStartup to find the standard MSVC
// _initterm-style static-initializer loop (a strong candidate for where
// static ConsoleFunc objects, including built-in console commands, get
// constructed before WinMain runs).
//
// @category KOTOR
// @menupath Tools.KOTOR.Decompile CRT Startup

import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;
import ghidra.util.task.ConsoleTaskMonitor;

public class DecompileCRTStartup extends GhidraScript {

    @Override
    public void run() throws Exception {
        Function fn = getFunctionAt(toAddr(0x0091d424L));
        if (fn == null) {
            println("function not found at 0091d424");
            return;
        }
        println("Decompiling " + fn.getName() + " @ " + fn.getEntryPoint() + ", size=" + fn.getBody().getNumAddresses());

        DecompInterface decomp = new DecompInterface();
        decomp.openProgram(currentProgram);
        DecompileResults res = decomp.decompileFunction(fn, 120, new ConsoleTaskMonitor());
        if (res.decompileCompleted()) {
            println(res.getDecompiledFunction().getC());
        } else {
            println("decompile failed: " + res.getErrorMessage());
        }
    }
}
