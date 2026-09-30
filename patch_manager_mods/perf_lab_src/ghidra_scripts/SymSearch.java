import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
// SymSearch.java substr... : list symbols whose name contains any substring (case-insensitive)
public class SymSearch extends GhidraScript {
  public void run() throws Exception {
    SymbolIterator it = currentProgram.getSymbolTable().getAllSymbols(true);
    while (it.hasNext()) {
      Symbol s = it.next();
      String n = s.getName(true).toLowerCase();
      for (String a : getScriptArgs()) if (n.contains(a.toLowerCase())) { println("SYM "+s.getAddress()+" "+s.getName(true)); break; }
    }
  }
}
