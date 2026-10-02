import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
public class RefsTo extends GhidraScript {
  public void run() throws Exception {
    for (String a : getScriptArgs()) {
      Address ad = toAddr(Long.parseLong(a,16));
      println("=== REFS to " + a);
      for (Reference r : getReferencesTo(ad)) {
        Function f = getFunctionContaining(r.getFromAddress());
        Instruction i = getInstructionAt(r.getFromAddress());
        println(r.getFromAddress()+" "+r.getReferenceType()+" "+(f==null?"-":f.getName()+"@"+f.getEntryPoint())+" | "+(i==null?"":i.toString()));
      }
    }
  }
}
