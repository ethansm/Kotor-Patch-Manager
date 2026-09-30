import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.mem.*;
public class FindStrRefs extends GhidraScript {
  public void run() throws Exception {
    for (String str : getScriptArgs()) {
      byte[] pat = (str + "\0").getBytes();
      Address a = currentProgram.getMinAddress();
      while (true) {
        a = currentProgram.getMemory().findBytes(a, pat, null, true, monitor);
        if (a == null) break;
        println("STR '" + str + "' at " + a);
        for (Reference r : getReferencesTo(a)) {
          Function f = getFunctionContaining(r.getFromAddress());
          println("  REF " + r.getFromAddress() + " in " + (f == null ? "?" : f.getEntryPoint().toString()));
        }
        a = a.add(1);
      }
    }
  }
}
