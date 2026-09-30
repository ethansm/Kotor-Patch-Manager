// @category KOTOR
// args: <fieldOffsetHex> <fromHex> <toHex>  -> every MOV/CMP-free store "MOV [reg+off], x" in the range, with the function.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
public class FindFieldWrites extends GhidraScript {
  public void run() throws Exception {
    String[] a = getScriptArgs();
    long off = Long.parseLong(a[0],16), lo = Long.parseLong(a[1],16), hi = Long.parseLong(a[2],16);
    Listing l = currentProgram.getListing();
    InstructionIterator it = l.getInstructions(toAddr(lo), true);
    while (it.hasNext()) {
      Instruction ins = it.next();
      if (ins.getAddress().getOffset() >= hi) break;
      if (!ins.getMnemonicString().equals("MOV")) continue;
      String s = ins.toString();
      String dst = s.substring(4).split(",")[0];
      if (dst.contains("[") && dst.matches(".*\\+ 0x" + Long.toHexString(off) + "\\]")) {
        Function f = getFunctionContaining(ins.getAddress());
        println(ins.getAddress() + "  " + s + "  in " + (f==null?"?":f.getName()+"@"+f.getEntryPoint()));
      }
    }
  }
}
