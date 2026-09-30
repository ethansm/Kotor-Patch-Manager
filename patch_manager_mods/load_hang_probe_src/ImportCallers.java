// @category KOTOR
// Lists every call site of the named imports (args = import names; default = the pointer-probe /
// exception-handling set). Resolves each external symbol, its thunks and IAT pointer refs, groups
// by containing function. Load-hang follow-up (09_load_hang_investigation.md section 9).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
import java.util.*;
public class ImportCallers extends GhidraScript {
  static final String[] DEFAULT = { "IsBadReadPtr", "IsBadWritePtr", "IsBadCodePtr", "IsBadStringPtrA", "IsBadStringPtrW",
      "IsBadHugeReadPtr", "SetUnhandledExceptionFilter", "AddVectoredExceptionHandler", "RaiseException",
      "UnhandledExceptionFilter", "VirtualQuery", "PeekMessageA", "PeekMessageW", "DispatchMessageA", "GetMessageA",
      "__except_handler3", "_except_handler3", "_except_handler4", "__except_handler4", "__SEH_prolog4", "__SEH_prolog", "_EH_prolog" };
  public void run() throws Exception {
    String[] names = getScriptArgs().length > 0 ? getScriptArgs() : DEFAULT;
    SymbolTable st = currentProgram.getSymbolTable();
    for (String n : names) {
      Set<Address> targets = new LinkedHashSet<>();
      for (Symbol s : st.getSymbols(n)) targets.add(s.getAddress());
      for (Symbol s : st.getExternalSymbols(n)) targets.add(s.getAddress());
      // thunks + IAT slots that point at the external
      for (Address t : new ArrayList<>(targets)) {
        for (Reference r : getReferencesTo(t)) {
          targets.add(r.getFromAddress());
          Function tf = getFunctionContaining(r.getFromAddress());
          if (tf != null && tf.isThunk()) targets.add(tf.getEntryPoint());
        }
      }
      Map<String, List<String>> byFn = new TreeMap<>();
      for (Address t : targets) {
        for (Reference r : getReferencesTo(t)) {
          if (!r.getReferenceType().isCall() && !r.getReferenceType().isData() && !r.getReferenceType().isJump()) continue;
          Function f = getFunctionContaining(r.getFromAddress());
          if (f != null && f.isThunk()) continue;
          String key = f == null ? "?" : f.getEntryPoint() + " " + f.getName() + " sz=" + f.getBody().getNumAddresses();
          byFn.computeIfAbsent(key, k -> new ArrayList<>()).add(r.getFromAddress() + "(" + r.getReferenceType() + ")");
        }
      }
      println("== " + n + ": " + targets.size() + " target addrs, " + byFn.size() + " calling functions");
      int i = 0;
      for (Map.Entry<String, List<String>> e : byFn.entrySet()) {
        if (++i > 150) { println("  ... truncated"); break; }
        println("  " + e.getKey() + "  <- " + String.join(",", e.getValue()));
      }
    }
  }
}
