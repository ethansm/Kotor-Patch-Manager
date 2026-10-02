// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
public class Callers extends GhidraScript {
  public void run() throws Exception {
    for (String a : getScriptArgs()) {
      println("== refs to "+a);
      for (Reference r : getReferencesTo(toAddr(Long.parseLong(a,16)))) {
        Function f = getFunctionContaining(r.getFromAddress());
        println("  from "+r.getFromAddress()+" in "+(f==null?"?":f.getName()+"@"+f.getEntryPoint()+" sz="+f.getBody().getNumAddresses()));
      }
    }
  }
}
