// @category KOTOR
// args: hex byte string(s) like "602b1d6fa0d5cf11". For each hit in memory: address, and refs (from, function).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
public class FindBytesRefs extends GhidraScript {
    public void run() throws Exception {
        for (String h : getScriptArgs()) {
            byte[] pat = new byte[h.length() / 2];
            for (int i = 0; i < pat.length; i++) pat[i] = (byte) Integer.parseInt(h.substring(2 * i, 2 * i + 2), 16);
            Address st = currentProgram.getMinAddress();
            while (true) {
                Address hit = currentProgram.getMemory().findBytes(st, pat, null, true, monitor);
                if (hit == null) break;
                println("HIT " + h + " @ " + hit);
                for (Reference r : getReferencesTo(hit)) { Function f = getFunctionContaining(r.getFromAddress()); println("   ref " + r.getFromAddress() + " in " + (f == null ? "?" : f.getName() + "@" + f.getEntryPoint())); }
                st = hit.add(1);
            }
        }
    }
}
