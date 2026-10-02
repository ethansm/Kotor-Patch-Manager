// Shield Bar research: find functions that call CGameEffect::SetInteger (0x005e4e80) with a constant index
// and also test an effect type tag == <type>. args: <typeHex> <setIntIndex>. Prints decompile of matches.
// @category KOTOR
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.*;
import java.util.*;
public class FindEffectIntConsumers extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        String[] targets = {"005e4e80", "005e4e60"};
        Set<Address> fns = new TreeSet<>();
        for (String t : targets) for (Reference r : getReferencesTo(toAddr(Long.parseLong(t,16)))) {
            Function f = getFunctionContaining(r.getFromAddress()); if (f != null) fns.add(f.getEntryPoint());
        }
        println("candidate functions: " + fns.size());
        DecompInterface d = new DecompInterface(); d.openProgram(currentProgram);
        String needle1 = a.length > 0 ? a[0] : "== 2";
        String needle2 = a.length > 1 ? a[1] : "FUN_005e4e80(2,";
        for (Address e : fns) {
            Function f = getFunctionAt(e);
            DecompileResults r = d.decompileFunction(f, 60, monitor);
            if (!r.decompileCompleted()) continue;
            String c = r.getDecompiledFunction().getC();
            if (c.contains(needle1) && c.contains(needle2))
                println("MATCH " + e + " size=" + f.getBody().getNumAddresses());
        }
    }
}
