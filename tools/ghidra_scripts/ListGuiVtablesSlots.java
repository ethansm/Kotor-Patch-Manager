// @category KOTOR
// Args: <namespace prefix e.g. CSWGui> <nslots>. For every vftable whose class name starts with prefix, prints address and slot targets; also xref count of vtable address with containing functions (ctors).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
public class ListGuiVtablesSlots extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        int n = Integer.parseInt(a[1]);
        SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
        while (it.hasNext()) { Symbol y = it.next();
            if (!y.getName().equals("vftable")) continue;
            String cls = y.getParentNamespace().getName(true);
            if (!cls.contains(a[0])) continue;
            Address base = y.getAddress();
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < n; i++) { long v = currentProgram.getMemory().getInt(base.add(i*4)) & 0xffffffffL; sb.append(i+"="+Long.toHexString(v)+" "); }
            println("VT " + cls + " @ " + base + " : " + sb);
            StringBuilder rb = new StringBuilder();
            for (Reference r : getReferencesTo(base)) { Function f = getFunctionContaining(r.getFromAddress()); rb.append((f==null?"-":f.getEntryPoint().toString())+"("+r.getFromAddress()+") "); }
            println("   REFS " + rb);
        }
    }
}
