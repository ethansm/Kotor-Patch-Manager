import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.scalar.*;
// Lists every instruction (in .text) with an operand scalar equal to any hex arg: ScalarSearch.java 1388 ...
public class ScalarSearch extends GhidraScript {
  public void run() throws Exception {
    for (String a : getScriptArgs()) {
      long v = Long.parseLong(a,16);
      println("=== SCALAR " + a);
      InstructionIterator it = currentProgram.getListing().getInstructions(true);
      while (it.hasNext()) {
        Instruction i = it.next();
        for (int op = 0; op < i.getNumOperands(); op++) {
          Scalar s = i.getScalar(op);
          if (s != null && s.getUnsignedValue() == v) {
            Function f = getFunctionContaining(i.getAddress());
            println(i.getAddress()+" "+(f==null?"-":f.getName())+" | "+i);
            break;
          }
        }
      }
    }
  }
}
