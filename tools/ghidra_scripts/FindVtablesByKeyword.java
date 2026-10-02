// FindVtablesByKeyword.java — @category KOTOR. args: keyword... ; lists symbols whose qualified name contains a keyword AND 'vftable'.
import ghidra.app.script.GhidraScript; import ghidra.program.model.symbol.*;
public class FindVtablesByKeyword extends GhidraScript { public void run() throws Exception {
  SymbolTable st = currentProgram.getSymbolTable();
  for (String kw : getScriptArgs()) { println("=== "+kw); SymbolIterator it = st.getAllSymbols(true);
    while (it.hasNext()) { Symbol s = it.next(); String qn = s.getName(true);
      if (qn.contains(kw) && qn.toLowerCase().contains("vftable")) println("  "+qn+" @ "+s.getAddress()); } } } }
