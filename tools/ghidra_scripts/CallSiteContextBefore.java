// @category KOTOR
// Args: <target hex> <n instrs before>. For every call to target prints containing function and the n preceding instructions (to see what is pushed / loaded into ECX).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
public class CallSiteContextBefore extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    int n = Integer.parseInt(a[1]);
    for (Reference r : getReferencesTo(toAddr(Long.parseLong(a[0],16)))) {
      if (!r.getReferenceType().isCall()) continue;
      Function f = getFunctionContaining(r.getFromAddress());
      println("CALL " + r.getFromAddress() + " in " + (f==null?"-":f.getEntryPoint().toString()));
      Instruction i = getInstructionAt(r.getFromAddress());
      for (int k = 0; k < n && i != null; k++) { i = i.getPrevious(); if (i != null) println("    " + i.getAddress() + " " + i); }
    }
  }
}
