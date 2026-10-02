// @category KOTOR
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;
public class SymAndVtab extends GhidraScript {
  public void run() throws Exception {
    for (String a : getScriptArgs()) {
      long v = Long.parseLong(a,16);
      println("== vtable "+a);
      for (long o=-8;o<0;o+=4){ Address ad=toAddr(v+o); Symbol[] ss=currentProgram.getSymbolTable().getSymbols(ad); for(Symbol s:ss) println("  sym@"+ad+" "+s.getName(true)); }
      for (Symbol s: currentProgram.getSymbolTable().getSymbols(toAddr(v))) println("  sym@"+v+" "+s.getName(true));
      for (int i=0;i<14;i++){ Address ad=toAddr(v+4*i); long t=currentProgram.getMemory().getInt(ad)&0xffffffffL; Function f=getFunctionAt(toAddr(t)); println("  ["+i+"] -> "+Long.toHexString(t)+" "+(f==null?"":f.getName()+" sz="+f.getBody().getNumAddresses())); }
    }
  }
}
