// @category KOTOR
// args: (callers <entryHex>) | (vtable <addrHex> <nslots>) | (refs <hex>)  -- repeatable via triples? one per postScript
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class CallersAndVtableArgs extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        if (a[0].equals("callers") || a[0].equals("refs")) {
            Address t = toAddr(Long.parseLong(a[1],16));
            for (Reference r : getReferencesTo(t)) {
                Function f = getFunctionContaining(r.getFromAddress());
                println("REF " + r.getFromAddress() + " " + r.getReferenceType() + " in " + (f==null?"<none>":f.getEntryPoint()+" "+f.getName()));
            }
        } else if (a[0].equals("vtable")) {
            long base = Long.parseLong(a[1],16); int n = Integer.parseInt(a[2]);
            for (int i=0;i<n;i++) {
                Address s = toAddr(base+4L*i);
                long v = currentProgram.getMemory().getInt(s) & 0xffffffffL;
                Function f = getFunctionAt(toAddr(v));
                println("SLOT " + i + " " + String.format("%08x",v) + " " + (f==null?"":f.getName()+" size="+f.getBody().getNumAddresses()));
            }
        }
    }
}
