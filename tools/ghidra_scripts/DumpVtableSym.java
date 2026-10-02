// @category KOTOR
// Args: <symbol name, e.g. CSWGuiInGameMenu::vftable> <nslots>. Prints each slot's target function.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
public class DumpVtableSym extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        int n = Integer.parseInt(a[1]);
        String cls = a[0].contains("::") ? a[0].substring(0, a[0].indexOf("::")) : a[0];
        java.util.List<Symbol> found = new java.util.ArrayList<>();
        SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
        while (it.hasNext()) { Symbol y = it.next(); if (y.getName().equals("vftable") && y.getParentNamespace().getName().equals(cls)) found.add(y); }
        if (found.isEmpty()) println("NOT FOUND " + a[0]);
        for (Symbol s : found) {
            Address base = s.getAddress();
            println("VTABLE " + a[0] + " @ " + base);
            for (int i = 0; i < n; i++) {
                int v = currentProgram.getMemory().getInt(base.add(i * 4));
                Address t = toAddr(v & 0xffffffffL);
                Function f = getFunctionAt(t);
                println("  slot " + i + " -> " + t + (f != null ? " size=" + f.getBody().getNumAddresses() : ""));
            }
        }
    }
}
