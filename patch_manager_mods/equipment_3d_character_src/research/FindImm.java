import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.scalar.Scalar;
public class FindImm extends GhidraScript {
  public void run() throws Exception {
    java.util.Set<Long> want = new java.util.HashSet<>();
    for (String a : getScriptArgs()) want.add(Long.decode(a) & 0xffffffffL);
    Listing L = currentProgram.getListing();
    for (Instruction in : L.getInstructions(true)) {
      if (!in.getMnemonicString().equals("PUSH") && !in.getMnemonicString().equals("MOV") && !in.getMnemonicString().equals("CMP")) continue;
      for (int i = 0; i < in.getNumOperands(); i++) {
        Scalar sc = in.getScalar(i);
        if (sc != null && want.contains(sc.getUnsignedValue() & 0xffffffffL) || (sc != null && sc.getValue() < 0 && want.contains(sc.getValue() & 0xffffffffL))) {
          Function f = getFunctionContaining(in.getAddress());
          println("IMM " + in.getAddress() + " " + in + " in " + (f == null ? "?" : f.getEntryPoint() + " " + f.getName()));
        }
      }
    }
  }
}
